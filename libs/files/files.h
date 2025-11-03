#ifndef FILESH
#define FILESH

#include "stdio.h"
#include <stdlib.h>

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

        FRESULT open_song_file(uint32_t index);
        void close_song_file();
    private:
        FATFS fs;
        FIL current_song_file;
        FIL songDB;
        FIL artistDB;
        FIL albumDB;
        FIL imageDB;
        uint32_t song_count;
};
#endif