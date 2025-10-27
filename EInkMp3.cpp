#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "epdDraw.h"
#include "fileManager.h"
#include <string.h>

extern "C" {
    #include "ff.h"
    #include "tf_card.h"
}

#define MENUS 3
static uint16_t menuSymbols[MENUS] = { 0x0041, 0x0050, 0x0053 };

size_t utf8_to_16arr(const char* utf8, uint16_t* out, size_t max_len) {
    size_t count = 0;

    while (*utf8 && count < max_len) {
        uint8_t c = *utf8++;

        if (c < 0x80) {
            out[count++] = c;
        }
        else if ((c & 0xE0) == 0xC0) {
            // 2-byte UTF-8
            uint8_t c2 = *utf8;
            if ((c2 & 0xC0) != 0x80) {
                out[count++] = 0xFFFD;
                continue;
            }
            utf8++;
            out[count++] = ((c & 0x1F) << 6) | (c2 & 0x3F);
        }
        else if ((c & 0xF0) == 0xE0) {
            // 3-byte UTF-8
            uint8_t c2 = utf8[0];
            uint8_t c3 = utf8[1];
            if ((c2 & 0xC0) != 0x80 || (c3 & 0xC0) != 0x80) {
                out[count++] = 0xFFFD;
                continue;
            }
            utf8 += 2;
            out[count++] = ((c & 0x0F) << 12) |
                           ((c2 & 0x3F) << 6) |
                           (c3 & 0x3F);
        }
        else {
            // 4-byte UTF-8 or invalid: replace with 0xFFFD
            out[count++] = 0xFFFD;
            if ((*utf8 & 0xC0) == 0x80) utf8++;
            if ((*utf8 & 0xC0) == 0x80) utf8++;
            if ((*utf8 & 0xC0) == 0x80) utf8++;
        }
    }

    return count;
}

void print_hex(const char* label, const uint8_t* data, size_t len) {
    printf("%s (%u bytes):", label, (unsigned)len);
    for (size_t i = 0; i < len; ++i) {
        if ((i % 16) == 0) printf("\n%04u: ", (unsigned)i);
        printf("%02X ", data[i]);
    }
    printf("\n");
}

EPDRenderer renderer;
FileManager fileManager = FileManager();

void drawSongInfo(SongRecord song) {
    size_t name_len = cp_len_0term(song.name, SONG_NAME_CODEPOINTS);
    renderer.DrawText(song.name, name_len, 126, 46, GRAY4, 2);

    ArtistRecord artist{};
    FRESULT fr = fileManager.read_artist_by_index(song.artist_id, &artist);
    if (fr != FR_OK) panic("Error while reading artist");

    name_len = cp_len_0term(artist.name, ARTIST_NAME_CODEPOINTS);
    renderer.DrawText(artist.name, name_len, 126, 60, GRAY4, 2);

    uint8_t imageData[3600];
    fr = fileManager.read_image_by_index(song.image_id, imageData);
    if (fr != FR_OK) panic("Error while reading artist");

    renderer.DrawBitmap(imageData, 0, 4, 120, 120);
}

void drawMenuCorner(int currentMenu) {
    for(int menu = 0; menu < MENUS; menu++) {
        int xCoord = 296 - (16 * menu);
        renderer.DrawRect(xCoord - 16, 0, xCoord, 16, BLACK, DOT_PIXEL_1X1, menu == currentMenu ? DRAW_FILL_FULL : DRAW_FILL_EMPTY);
        renderer.DrawChar(menuSymbols[menu], xCoord - 12, 0, menu == currentMenu ? WHITE : BLACK);
    }
}

void drawSongMenu(uint32_t songIndex) {
    SongRecord currentSong{};
    FRESULT fr = fileManager.read_song_by_index(songIndex, &currentSong);
    if (fr != FR_OK) panic("Could not load song %u", songIndex);
    drawSongInfo(currentSong);
    size_t name_len = 0;

    //Draw Song Above
    if (songIndex > 0) {
        fr = fileManager.read_song_by_index(songIndex - 1, &currentSong);
        name_len = cp_len_0term(currentSong.name, SONG_NAME_CODEPOINTS);
        renderer.DrawText(currentSong.name, name_len, 126, 18, GRAY3, 2);
    }

    //Draw Song Below
    if (songIndex < fileManager.getSongCount() - 1) {
        fr = fileManager.read_song_by_index(songIndex + 1, &currentSong);
        name_len = cp_len_0term(currentSong.name, SONG_NAME_CODEPOINTS);
        renderer.DrawText(currentSong.name, name_len, 126, 88, GRAY3, 2);
    }

    drawMenuCorner(2);
}

int main()
{
    stdio_init_all();
    sleep_ms(10000);

    FRESULT initResult = fileManager.init_file_system();
    if (initResult != FR_OK) {
        return 1;
    }

    //Setup Renderer
    renderer = EPDRenderer();
    renderer.Init(4, ROTATE_270, GRAY4);
    //Reset the Screen
    renderer.Clear(WHITE);

    drawSongMenu(8);
    renderer.RefreshScreen();

    //Do Music shit
    //AudioManager.play_song(0);

    while (true) {
        printf("Hello, world!\n");
        sleep_ms(1000);
    }
}