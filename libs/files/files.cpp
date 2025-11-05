#include "files.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"

static inline void le32(uint8_t* p, uint32_t v) { p[0]=uint8_t(v); p[1]=uint8_t(v>>8); p[2]=uint8_t(v>>16); p[3]=uint8_t(v>>24); }
static inline void le16(uint8_t* p, uint16_t v) { p[0]=uint8_t(v); p[1]=uint8_t(v>>8); }

//-- HELPERS --\\

static inline void md5_hex(const uint8_t md5[16], char out[33]) {
    static const char* hexd = "0123456789abcdef";
    for (int i = 0; i < 16; i++) {
        out[i*2+0] = hexd[(md5[i] >> 4) & 0xF];
        out[i*2+1] = hexd[(md5[i]) & 0xF];
    }
    out[32] = '\0';
}

static inline size_t cp_len_0term(const uint16_t* s, size_t max_cp) {
    size_t i = 0;
    while (i < max_cp && s[i] != 0x0000) ++i;
    return i;
}

static inline uint32_t read_u24le(const uint8_t* p) {
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16);
}

static inline FRESULT read_exact_at(FIL* f, FSIZE_t off, void* dst, UINT len) {
    FRESULT fr = f_lseek(f, off);
    if (fr != FR_OK) return fr;
    UINT br = 0;
    fr = f_read(f, dst, len, &br);
    if (fr != FR_OK) return fr;
    return (br == len) ? FR_OK : FR_INT_ERR;
}

static inline void parse_song_record(song_record_t* out, const uint8_t buf[SONG_RECORD_SIZE]) {
    //16 bytes MD5
    memcpy(out->md5, buf, 16);

    //44 Bytes name: 22 * 16-bit LE code points
    const uint8_t* p = buf + 16;
    for (int i = 0; i < SONG_NAME_CODEPOINTS; ++i) {
        out->name[i] = (uint16_t)(p[0] | (p[1] << 8));
        p += 2;
    }

    // 3 bytes artist, 3 bytes image
    out->artist_id = read_u24le(buf + 16 + 44);
    out->image_id = read_u24le(buf + 16 + 44 + 3);
}

static inline void parse_artist_record(artist_record_t* out, const uint8_t buf[ARTIST_RECORD_SIZE]) {
    const uint8_t* p = buf;
    for (int i = 0; i < ARTIST_NAME_CODEPOINTS; ++i) {
        out->name[i] = (uint16_t)(p[0] | (p[1] << 8));
        p += 2;
    }
    out->image_id = (uint32_t)buf[44] | ((uint32_t)buf[45] << 8) | ((uint32_t)buf[46] << 16);
    out->album_id = (uint32_t)buf[47] | ((uint32_t)buf[48] << 8) | ((uint32_t)buf[49] << 16);
}

static inline void parse_album_record(album_record_t* out, const uint8_t buf[ALBUM_RECORD_SIZE]) {
    const uint8_t* p = buf; // 44 bytes name = 22 * 16-bit LE
    for (int i = 0; i < ALBUM_NAME_CODEPOINTS; ++i) {
        out->name[i] = (uint16_t)(p[0] | (p[1] << 8));
        p += 2;
    }
    out->image_id      = (uint32_t)buf[44] | ((uint32_t)buf[45] << 8) | ((uint32_t)buf[46] << 16);
    out->song_count    = (uint32_t)buf[47] | ((uint32_t)buf[48] << 8) | ((uint32_t)buf[49] << 16);
    out->song_list_off = (uint32_t)buf[50] | ((uint32_t)buf[51] << 8) | ((uint32_t)buf[52] << 16);
}

static inline void md5_filename(const uint8_t md5[16], char out[33+4]) {
    static const char hexDigits[] = "0123456789abcdef";

    for (int i = 0; i < 16; i++) {
        out[i * 2]     = hexDigits[(md5[i] >> 4) & 0xF];
        out[i * 2 + 1] = hexDigits[md5[i] & 0xF];
    }
    // Append ".wav"
    out[32] = '.';
    out[33] = 'w';
    out[34] = 'a';
    out[35] = 'v';
    out[36] = '\0'; // Null-terminate
}

//-- Implementation --\\

FileManager::FileManager() {}

FRESULT FileManager::init() {
        //Setup SD-Config
    pico_fatfs_spi_config_t sdConfig = {
        .spi_inst = spi1,
        .clk_slow = 400000,
        .clk_fast = 12000000,
        .pin_miso = 12,
        .pin_cs = 13,
        .pin_sck = 10,
        .pin_mosi = 11,
        .pullup = true
    };

    gpio_init(13);
    gpio_set_dir(13, true);
    gpio_put(13, 1);
    sleep_ms(5);

    if(!pico_fatfs_set_config(&sdConfig)) {
        printf("SPI config failed\n");
        return FR_INVALID_PARAMETER;
    }

    FRESULT fr = f_mount(&fs, "", 1);
    if (fr != FR_OK) {
        printf("Failed to mount filesystem: %d\n", fr);
        return fr;
    }

    DIR dir;
    FILINFO fno;
    fr = f_opendir(&dir, "/");
    if (fr != FR_OK) {
        printf("Failed to open root directory: %d\n", fr);
        return fr;
    }

    

    fr = f_open(&songDB, "songs.db", FA_READ);
    if (fr != FR_OK) {
        printf("Failed to open songs.db: %d\n", fr);
        return fr;
    }
    //Caclulate Song-Count
    FSIZE_t fileSize = f_size(&songDB);
    song_count = (uint32_t)(fileSize / SONG_RECORD_SIZE);

    fr = f_open(&artistDB, "artists.db", FA_READ);
    if (fr != FR_OK) {
        printf("Failed to open artists.db: %d\n", fr);
        return fr;
    }

    fr = f_open(&albumDB, "albums.db", FA_READ);
    if (fr != FR_OK) {
        printf("Failed to open albums.db: %d\n", fr);
        return fr;
    } 

    // Get Album-Count
    uint8_t buf[3];
    FRESULT album_fr = read_exact_at(&albumDB, 0, buf, 3);
    if (album_fr != FR_OK) return fr;
    album_count = read_u24le(buf);

    fr = f_open(&imageDB, "images.db", FA_READ);
    if (fr != FR_OK) {
        printf("Failed to open images.db: %d\n", fr);
        return fr;
    }

    return FR_OK;
}

FRESULT FileManager::deinit() {
    f_close(&songDB);
    f_close(&artistDB);
    f_close(&albumDB);
    f_close(&imageDB);
    return FR_OK;
}

FRESULT FileManager::read_song_index(uint32_t index, song_record_t* out) {
    //Seek to index * 66
    FSIZE_t off = (FSIZE_t)index * SONG_RECORD_SIZE;
    FRESULT fr = f_lseek(&songDB, off);
    if (fr != FR_OK) return fr;

    //Read exactly one record
    UINT br = 0;
    uint8_t buf[SONG_RECORD_SIZE];
    fr = f_read(&songDB, buf, SONG_RECORD_SIZE, &br);
    if (fr != FR_OK) return fr;
    if (br != SONG_RECORD_SIZE) return FR_INT_ERR;

    parse_song_record(out, buf);
    return FR_OK;
}

FRESULT FileManager::read_song_index_from_album(uint32_t index_in_album, album_record_t *album, uint32_t *out) {
    FSIZE_t off = (FSIZE_t)index_in_album * 3 + album->song_list_off;
    FRESULT fr = f_lseek(&albumDB, off);
    if (fr != FR_OK) return fr;

    UINT br;
    uint8_t buf[3];
    fr = f_read(&albumDB, buf, 3, &br);
    if (fr != FR_OK) return fr;
    if (br != 3) return FR_INT_ERR;

    *out = read_u24le(buf);
    return fr;
}

FRESULT FileManager::read_song_album_index(uint32_t index_in_album, album_record_t *album, song_record_t *out) {
    FSIZE_t off = (FSIZE_t)index_in_album * 3 + album->song_list_off;
    FRESULT fr = f_lseek(&albumDB, off);
    if (fr != FR_OK) return fr;

    UINT br;
    uint8_t buf[3];
    fr = f_read(&albumDB, buf, 3, &br);
    if (fr != FR_OK) return fr;
    if (br != 3) return FR_INT_ERR;

    uint32_t song_idx = read_u24le(buf);
    return read_song_index(song_idx, out);
}

uint32_t FileManager::read_song_count() {
    return song_count;
}

uint32_t FileManager::read_album_count() {
    return album_count;
}

FRESULT FileManager::read_artist_index(uint32_t index, artist_record_t* out) {
    if (!out) return FR_INT_ERR;

    const FSIZE_t off = (FSIZE_t)index * ARTIST_RECORD_SIZE;
    if (off + ARTIST_RECORD_SIZE > f_size(&artistDB)) return FR_INT_ERR;

    uint8_t buf[ARTIST_RECORD_SIZE];
    FRESULT fr = read_exact_at(&artistDB, off, buf, ARTIST_RECORD_SIZE);
    if (fr != FR_OK) return fr;

    parse_artist_record(out, buf);
    return FR_OK;
}

FRESULT FileManager::read_album_index(uint32_t index, album_record_t* out) {
    if (!out) return FR_INT_ERR;

    const FSIZE_t off = (FSIZE_t)index * ALBUM_RECORD_SIZE + 3;
    if (off + ALBUM_RECORD_SIZE> f_size(&albumDB)) return FR_INT_ERR;

    uint8_t buf[ALBUM_RECORD_SIZE];
    FRESULT fr = read_exact_at(&albumDB, off, buf, ALBUM_RECORD_SIZE);
    if (fr != FR_OK) return fr;

    parse_album_record(out, buf);
    return FR_OK; 
}

FRESULT FileManager::read_image_index(uint32_t index, uint8_t* out) {
    if (!out) return FR_INT_ERR;
    static const UINT IMAGE_BYTES = 3600u; // 120*120*2/8

    const FSIZE_t off = (FSIZE_t)index * IMAGE_BYTES;
    if (off + IMAGE_BYTES > f_size(&imageDB)) return FR_INT_ERR;

    return read_exact_at(&imageDB, off, out, IMAGE_BYTES);
}

FRESULT FileManager::open_song_file(uint32_t index) {
    song_record_t selectedSong;
    FRESULT fr = read_song_index(index, &selectedSong);
    if (fr != FR_OK) return fr;

    char fileName[37];
    md5_filename(selectedSong.md5, fileName);

    printf("Opening File: %s", fileName);
    return f_open(&current_song_file, fileName, FA_READ);
}

void FileManager::close_song_file() {
    f_close(&current_song_file);
}