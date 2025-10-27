#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <stdint.h>
#include "dr_wav.h"

extern "C" {
    #include "ff.h"
    #include "tf_card.h"
}

// songs.db
#define SONG_RECORD_SIZE 66
#define SONG_NAME_CODEPOINTS 22

typedef struct {
    uint8_t md5[16];
    uint16_t name[SONG_NAME_CODEPOINTS];
    uint32_t artist_id;
    uint32_t image_id;
} SongRecord;

// artists.db
#define ARTIST_RECORD_SIZE 50
#define ARTIST_NAME_CODEPOINTS 22

typedef struct {
    uint16_t name[ARTIST_NAME_CODEPOINTS];
    uint32_t image_id;
    uint32_t album_id;
} ArtistRecord;

// albums.db
#define ALBUM_HEADER_SIZE 53
#define ALBUM_NAME_CODEPOINTS 22

typedef struct {
    uint16_t name[ALBUM_NAME_CODEPOINTS];
    uint32_t image_id;
    uint32_t song_count;
    uint32_t song_list_off;
} AlbumHeader;

// ---------- Helpers ---------- \\

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

class FILEMANAGER {
    public:
    FILEMANAGER();
        FRESULT init_file_system();
        FRESULT deinit_file_system();
        FRESULT read_song_by_index (uint32_t index, SongRecord* out);
        FRESULT read_artist_by_index (uint32_t index, ArtistRecord* out);
        FRESULT read_album_by_index (uint32_t index, AlbumHeader* out);
        FRESULT read_image_by_index (uint32_t index, uint8_t* out);

        uint32_t getSongCount();

        FRESULT read_song_file (uint32_t index);
        void close_song_file();

        bool wav_begin(const char* path, uint32_t sample_rate, uint16_t channels, uint16_t bits_per_sample);
        bool wav_write_pcm_s16(const int16_t* interleaved, size_t frames);
        void wav_end();

        FIL currentSongFile;
    private:
        FATFS fs;
        FIL songsDB;
        FIL artistsDB;
        FIL albumsDB;
        FIL imagesDB;
        uint32_t songCount;

        FIL wav_fil_;
        bool wav_open_ = false;
        uint32_t wav_sr_ = 0, wav_ch_ = 0, wav_bps_ = 0;
        uint32_t wav_data_bytes_ = 0;
};

//DR_WAV specific implementations
static size_t wav_read(void* ud, void* out, size_t bytes_to_read) {
    FIL* fil = (FIL*)ud;
    UINT br = 0;
    FRESULT fr = f_read(fil, out, (UINT)bytes_to_read, &br);
    if (fr != FR_OK && br == 0) return 0;
    return (size_t)br;
}

static drwav_bool32 wav_seek(void *ud, int offset, drwav_seek_origin origin) {
    FIL* f = (FIL*)ud;
    FSIZE_t cur  = f_tell(f);
    FSIZE_t size = f_size(f);

    int64_t base =
        (origin == DRWAV_SEEK_SET)  ? 0 :
        (origin == DRWAV_SEEK_CUR)  ? (int64_t)cur :
                                    (int64_t)size; // if your dr_wav defines SEEK_END

    int64_t target = base + (int64_t)offset;
    if (target < 0 || target > (int64_t)size) return DRWAV_FALSE;

    return (f_lseek(f, (FSIZE_t)target) == FR_OK) ? DRWAV_TRUE : DRWAV_FALSE;
}

static drwav_bool32 wav_tell(void* pUserData, drwav_int64* pCursor) {
    FIL* f = (FIL*)pUserData;
    *pCursor = (drwav_int64)f_tell(f);
    return DRWAV_TRUE;
}

/*
//drmp3_specific implementations
static size_t mp3_read(void* pUserData, void* pBufferOut, size_t bytesToRead) {
    FIL* file = (FIL*)pUserData;
    UINT br = 0;
    FRESULT fr = f_read(file, pBufferOut, bytesToRead, &br);
    if (fr != FR_OK && br == 0) return 0;  // pass partial bytes through
    return (size_t)br;
}

static drmp3_bool32 mp3_seek(void* pUserData, int offset, drmp3_seek_origin origin) {
FIL* file = (FIL*)pUserData;

    // v0.7.x seek origin names are DRMP3_SEEK_SET / DRMP3_SEEK_CUR / DRMP3_SEEK_END
    drmp3_int64 cur  = (drmp3_int64)f_tell(file);
    drmp3_int64 size = (drmp3_int64)f_size(file);

    drmp3_int64 base =
        (origin == DRMP3_SEEK_SET) ? 0 :
        (origin == DRMP3_SEEK_CUR) ? cur :
                                     size;  // if you want to support SEEK_END

    drmp3_int64 target = base + (drmp3_int64)offset;

    // Reject out-of-range seeks so the decoder knows it hit a limit.
    if (target < 0 || target > size) return DRMP3_FALSE;

    return (f_lseek(file, (FSIZE_t)target) == FR_OK) ? DRMP3_TRUE : DRMP3_FALSE;
}

static drmp3_bool32 mp3_tell(void* pUserData, drmp3_int64* pCursor) {
    FIL* file = (FIL*)pUserData;
    *pCursor = (drmp3_int64)f_tell(file);
    return DRMP3_TRUE;
}
*/
#endif