#include "UIMenus.h"

static inline size_t cp_len_0term(const uint16_t* s, size_t max_cp) {
    size_t i = 0;
    while (i < max_cp && s[i] != 0x0000) ++i;
    return i;
}

static void print_utf16le(const uint16_t *name, size_t codepoints) {
    for (size_t i = 0; i < codepoints; ++i) {
        uint16_t cp = name[i];
        if (cp == 0x0000) break; // stop at null-padding

        if (cp < 0x80) {                     // 1-byte ASCII
            putchar((char)cp);
        } else if (cp < 0x800) {             // 2-byte UTF-8
            putchar(0xC0 | (cp >> 6));
            putchar(0x80 | (cp & 0x3F));
        } else {                             // 3-byte UTF-8
            putchar(0xE0 | (cp >> 12));
            putchar(0x80 | ((cp >> 6) & 0x3F));
            putchar(0x80 | (cp & 0x3F));
        }
    }
    putchar('\n');
}

//Main Manager
MainMenu::MainMenu(UIManager *parent, FileManager *fm) : UIMenu(parent, fm) {}

void MainMenu::start_menu() {
    printf("Starting Main-Menu\n");
}

void MainMenu::draw_menu(canvas_config_t *canvas) {
    canvas_clear(canvas, CANVAS_COLOR_GRAY_G1);

    uint32_t xCoord = 32;
    uint32_t yCoord = 16;

    uint16_t allArr[13] = { 'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd', ' ', '!' };
    canvas_draw_text(canvas, allArr, 13, xCoord, yCoord, CANVAS_COLOR_GRAY_G4, 2, 0);
    yCoord += 16;

    for (uint i = 0; i < 2; i++) {
        album_record_t curAlbum;
        if (fm->read_album_index(i, &curAlbum) ){
            printf("Could not read album %u\n", i);
            break;
        }
        size_t name_len = cp_len_0term(curAlbum.name, ALBUM_NAME_CODEPOINTS);
        printf("Len: %u\n", name_len);
        print_utf16le(curAlbum.name, name_len);
        canvas_draw_text(canvas, curAlbum.name, name_len, xCoord, yCoord, CANVAS_COLOR_GRAY_G4, 2, 0);
        yCoord += 16;
    }

    canvas_draw_rect(canvas, EPD_HEIGHT-32, 0, EPD_HEIGHT, EPD_WIDTH, CANVAS_COLOR_GRAY_G3, DOT_SIZE_1X1, DRAW_FILL_FULL);

    canvas_refresh_screen(canvas);
}

void MainMenu::update_menu() {

}

void MainMenu::button_input(uint8_t buttonCode) {

}

void MainMenu::close_menu() {

}