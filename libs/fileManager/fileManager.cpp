#include "fileManager.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"

// ---------- Helpers ---------- \\

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

static inline void parse_song_record(SongRecord* out, const uint8_t buf[SONG_RECORD_SIZE]) {
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

static inline void parse_artist_record(ArtistRecord* out, const uint8_t buf[ARTIST_RECORD_SIZE]) {
    const uint8_t* p = buf;
    for (int i = 0; i < ARTIST_NAME_CODEPOINTS; ++i) {
        out->name[i] = (uint16_t)(p[0] | (p[1] << 8));
        p += 2;
    }
    out->image_id = (uint32_t)buf[44] | ((uint32_t)buf[45] << 8) | ((uint32_t)buf[46] << 16);
    out->album_id = (uint32_t)buf[47] | ((uint32_t)buf[48] << 8) | ((uint32_t)buf[49] << 16);
}

static inline void parse_album_header(AlbumHeader* out, const uint8_t buf[ALBUM_HEADER_SIZE]) {
    const uint8_t* p = buf; // 44 bytes name = 22 * 16-bit LE
    for (int i = 0; i < ALBUM_NAME_CODEPOINTS; ++i) {
        out->name[i] = (uint16_t)(p[0] | (p[1] << 8));
        p += 2;
    }
    out->image_id      = (uint32_t)buf[44] | ((uint32_t)buf[45] << 8) | ((uint32_t)buf[46] << 16);
    out->song_count    = (uint32_t)buf[47] | ((uint32_t)buf[48] << 8) | ((uint32_t)buf[49] << 16);
    out->song_list_off = (uint32_t)buf[50] | ((uint32_t)buf[51] << 8) | ((uint32_t)buf[52] << 16);
}

static inline void md5ToFilename(const uint8_t md5[16], char out[33+4]) {
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

// ---------- FILE MANAGER ---------- \\

FILEMANAGER::FILEMANAGER() {}

FRESULT FILEMANAGER::init_file_system() {
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

    fr = f_open(&songsDB, "songs.db", FA_READ);
    if (fr != FR_OK) {
        printf("Failed to open songs.db: %d\n", fr);
        return fr;
    }
    //Caclulate Song-Count
    FSIZE_t fileSize = f_size(&songsDB);
    songCount = (uint32_t)(fileSize / SONG_RECORD_SIZE);

    fr = f_open(&artistsDB, "artists.db", FA_READ);
    if (fr != FR_OK) {
        printf("Failed to open artists.db: %d\n", fr);
        return fr;
    }

    fr = f_open(&albumsDB, "albums.db", FA_READ);
    if (fr != FR_OK) {
        printf("Failed to open albums.db: %d\n", fr);
        return fr;
    }

    fr = f_open(&imagesDB, "images.db", FA_READ);
    if (fr != FR_OK) {
        printf("Failed to open images.db: %d\n", fr);
        return fr;
    }

    return FR_OK;
}

FRESULT FILEMANAGER::deinit_file_system() {
    f_close(&songsDB);
    f_close(&artistsDB);
    f_close(&albumsDB);
    f_close(&imagesDB);
    return FR_OK;
}

FRESULT FILEMANAGER::read_song_by_index(uint32_t index, SongRecord* out) {
    //Seek to index * 66
    FSIZE_t off = (FSIZE_t)index * SONG_RECORD_SIZE;
    FRESULT fr = f_lseek(&songsDB, off);
    if (fr != FR_OK) return fr;

    //Read exactly one record
    UINT br = 0;
    uint8_t buf[SONG_RECORD_SIZE];
    fr = f_read(&songsDB, buf, SONG_RECORD_SIZE, &br);
    if (fr != FR_OK) return fr;
    if (br != SONG_RECORD_SIZE) return FR_INT_ERR;

    parse_song_record(out, buf);
    return FR_OK;
}

uint32_t FILEMANAGER::getSongCount() {
    return songCount;
}

FRESULT FILEMANAGER::read_artist_by_index(uint32_t index, ArtistRecord* out) {
    if (!out) return FR_INT_ERR;

    const FSIZE_t off = (FSIZE_t)index * ARTIST_RECORD_SIZE;
    if (off + ARTIST_RECORD_SIZE > f_size(&artistsDB)) return FR_INT_ERR;

    uint8_t buf[ARTIST_RECORD_SIZE];
    FRESULT fr = read_exact_at(&artistsDB, off, buf, ARTIST_RECORD_SIZE);
    if (fr != FR_OK) return fr;

    parse_artist_record(out, buf);
    return FR_OK;
}

FRESULT FILEMANAGER::read_album_by_index(uint32_t index, AlbumHeader* out) {
    if (!out) return FR_INT_ERR;

    const FSIZE_t off = (FSIZE_t)index * ALBUM_HEADER_SIZE;
    if (off + ALBUM_HEADER_SIZE> f_size(&albumsDB)) return FR_INT_ERR;

    uint8_t buf[ALBUM_HEADER_SIZE];
    FRESULT fr = read_exact_at(&albumsDB, off, buf, ALBUM_HEADER_SIZE);
    if (fr != FR_OK) return fr;

    parse_album_header(out, buf);
    return FR_OK; 
}

FRESULT FILEMANAGER::read_image_by_index(uint32_t index, uint8_t* out) {
    if (!out) return FR_INT_ERR;
    static const UINT IMAGE_BYTES = 3600u; // 120*120*2/8

    const FSIZE_t off = (FSIZE_t)index * IMAGE_BYTES;
    if (off + IMAGE_BYTES > f_size(&imagesDB)) return FR_INT_ERR;

    return read_exact_at(&imagesDB, off, out, IMAGE_BYTES);
}

FRESULT FILEMANAGER::read_song_file(uint32_t index) {
    SongRecord selectedSong;
    FRESULT fr = read_song_by_index(index, &selectedSong);
    if (fr != FR_OK) return fr;

    char fileName[37];
    md5ToFilename(selectedSong.md5, fileName);

    printf("Opening File: %s", fileName);
    return f_open(&currentSongFile, fileName, FA_READ);
}

void FILEMANAGER::close_song_file() {
    f_close(&currentSongFile);
} 

static inline void le32(uint8_t* p, uint32_t v) { p[0]=uint8_t(v); p[1]=uint8_t(v>>8); p[2]=uint8_t(v>>16); p[3]=uint8_t(v>>24); }
static inline void le16(uint8_t* p, uint16_t v) { p[0]=uint8_t(v); p[1]=uint8_t(v>>8); }

bool FILEMANAGER::wav_begin(const char* path, uint32_t sr, uint16_t ch, uint16_t bps) {
    if (wav_open_) return false;
    FRESULT fr = f_open(&wav_fil_, path, FA_CREATE_ALWAYS | FA_WRITE);
    if (fr != FR_OK) return false;

    wav_sr_ = sr; wav_ch_ = ch; wav_bps_ = bps; wav_data_bytes_ = 0;
    // 44‑byte WAV header with placeholder sizes
    uint8_t hdr[44] = {
        'R','I','F','F', 0,0,0,0,  'W','A','V','E',
        'f','m','t',' ', 16,0,0,0,  // PCM fmt chunk size
        1,0,                       // PCM
        0,0,                       // channels
        0,0,0,0,                   // sample rate
        0,0,0,0,                   // byte rate
        0,0,                       // block align
        0,0,                       // bits per sample
        'd','a','t','a', 0,0,0,0
    };
    le16(&hdr[22], ch);
    le32(&hdr[24], sr);
    uint32_t byte_rate = sr * ch * (bps/8);
    le32(&hdr[28], byte_rate);
    le16(&hdr[32], ch * (bps/8));
    le16(&hdr[34], bps);
    UINT bw=0;
    fr = f_write(&wav_fil_, hdr, sizeof(hdr), &bw);
    if (fr != FR_OK || bw != sizeof(hdr)) { f_close(&wav_fil_); return false; }
    wav_open_ = true;
    return true;
}

bool FILEMANAGER::wav_write_pcm_s16(const int16_t* interleaved, size_t frames) {
    if (!wav_open_ || wav_bps_ != 16) return false;
    // write directly from caller's buffer; no big allocations
    const UINT bytes = (UINT)(frames * wav_ch_ * 2);
    UINT bw=0;
    FRESULT fr = f_write(&wav_fil_, interleaved, bytes, &bw);
    if (fr != FR_OK || bw != bytes) return false;
    wav_data_bytes_ += bw;
    return true;
}

void FILEMANAGER::wav_end() {
    if (!wav_open_) return;
    // Patch sizes: RIFF size at offset 4, data size at offset 40
    uint8_t sz4[4];
    UINT bw=0;
    // data size
    le32(sz4, wav_data_bytes_);
    f_lseek(&wav_fil_, 40);
    f_write(&wav_fil_, sz4, 4, &bw);
    // riff size = 36 + data
    le32(sz4, 36u + wav_data_bytes_);
    f_lseek(&wav_fil_, 4);
    f_write(&wav_fil_, sz4, 4, &bw);
    f_close(&wav_fil_);
    wav_open_ = false;
}