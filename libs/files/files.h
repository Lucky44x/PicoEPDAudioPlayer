#ifndef FILESH
#define FILESH

#include "stdio.h"
#include <stdlib.h>

#include "dr_wav.h"
extern "C" {
    #include "ff.h"
    #include "tf_card.h"
}

#define SONG_RECORD_SIZE 66
#define SONG_NAME_CODEPOINTS 22
typedef struct {
    uint8_t md5[16];
    uint16_t name[SONG_NAME_CODEPOINTS];
    uint32_t artist_id;
    uint32_t image_id;
} song_record_t;

#define ARTIST_RECORD_SIZE 50
#define ARTIST_NAME_CODEPOINTS 22
typedef struct {
    uint16_t name[ARTIST_NAME_CODEPOINTS];
    uint32_t image_id;
    uint32_t album_id;
} artist_record_t;

#define ALBUM_RECORD_SIZE 53
#define ALBUM_NAME_CODEPOINTS 22
typedef struct {
    uint16_t name[ALBUM_NAME_CODEPOINTS];
    uint32_t image_id;
    uint32_t song_count;
    uint32_t song_list_off;
} album_record_t;

class FileManager {
    public:
        FileManager();
        FRESULT init();
        FRESULT deinit();
        FRESULT read_song_index(uint32_t index, song_record_t *out);
        FRESULT read_artist_index(uint32_t index, artist_record_t *out);
        FRESULT read_album_index(uint32_t index, album_record_t *out);
        FRESULT read_image_index(uint32_t index, uint8_t *out);

        uint32_t read_song_count();
        uint32_t read_album_count();

        FRESULT open_song_file(uint32_t index);
        void close_song_file();

        FIL current_song_file;
    private:
        FATFS fs;
        FIL songDB;
        FIL artistDB;
        FIL albumDB;
        FIL imageDB;
        uint32_t song_count;
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
#endif