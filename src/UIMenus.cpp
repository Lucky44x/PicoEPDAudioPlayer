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

#pragma region No Filesystem
ErrorMenu::ErrorMenu(UIManager *parent, FileManager *fm) : UIMenu(parent), m_fm(fm) {}

void ErrorMenu::start_menu() {
    printf("ERROR Menu started");
}

void ErrorMenu::set_message_utf8(const char* msg_utf8) {
    if(!msg_utf8) { m_msgLen = 0; return; }
    m_msgLen = utf8_to_16arr(msg_utf8, m_message, kMaxMsg);
}

void ErrorMenu::set_message_cp(const uint16_t* msg_cp, size_t len) {
    if (!msg_cp) { m_msgLen = 0; return; }
    if (len > kMaxMsg) len = kMaxMsg;
    for(size_t i=0; i < len; i++) m_message[i] = msg_cp[i];
    m_msgLen = len;
}

void ErrorMenu::draw_menu(canvas_config_t *canvas) {
    canvas_set_colorscale(canvas, 2);
    canvas_update_color_depth(canvas);
    canvas_clear(canvas, CANVAS_COLOR_BW_WHITE);

    const uint16_t w = canvas->width;
    const uint16_t h = canvas->height;

    //Title
    static const uint16_t ERR[5] = { 'E', 'R', 'R', 'O', 'R' };
    canvas_draw_text(canvas, ERR, 5, 8, 8, CANVAS_COLOR_BW_BLACK, 2, 0);

    uint16_t x = 8;
    uint16_t y = 8 + 16 + 8;
    if (m_msgLen) {
        canvas_draw_text(canvas, m_message, m_msgLen, x, y, CANVAS_COLOR_BW_BLACK, 2, 0);
    } else {
        static const uint16_t DEF[22] = {
            'U','n','k','n','o','w','n',' ','e','r','r','o','r',' ','o','c','c','u','r','r','e','d'
        };
        canvas_draw_text(canvas, DEF, 22, x, y, CANVAS_COLOR_BW_BLACK, 2, 0);
    }

    if (m_fallback != nullptr) {
        static const uint16_t HINT[16] = {
          'P','r','e','s','s',' ','a','n','y',' ','b','u','t','t','o','n'
        };
        canvas_draw_text(canvas, HINT, 16, 8, h - 16 - 4, CANVAS_COLOR_BW_BLACK, 1, 0);
    } else {
        static const uint16_t HINT[14] = {
          'P','l','e','a','s','e',' ','R','e','s','t','a','r','t'
        };
        canvas_draw_text(canvas, HINT, 14, 8, h - 16 - 4, CANVAS_COLOR_BW_BLACK, 1, 0);
    }

    canvas_refresh_screen(canvas);
}

void ErrorMenu::update_menu() {

}

void ErrorMenu::button_input(uint8_t buttonCode) {
    if (buttonCode != 0 && m_fallback != nullptr) {
        //Switch to fallback menu
        parentManager->switch_menu(m_fallback);
    }
}

void ErrorMenu::close_menu() {

}
#pragma endregion NoFilesystem

#pragma region Mainmenu
//Main Manager
MainMenu::MainMenu(UIManager *parent, FileManager *fm) : UIMenu(parent), fm(fm) {}

void MainMenu::start_menu() {
    printf("Starting Main-Menu\n");
}

void MainMenu::draw_menu(canvas_config_t *canvas) {
    canvas_set_colorscale(canvas, 2);
    canvas_update_color_depth(canvas);

    canvas_clear(canvas, CANVAS_COLOR_BW_WHITE);

    uint32_t xCoord = 16;
    uint32_t yCoord = 16;

    uint16_t allArr[13] = { 'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd', ' ', '!' };
    canvas_draw_text(canvas, allArr, 13, xCoord, yCoord, CANVAS_COLOR_BW_BLACK, 2, 0);
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
        canvas_draw_text(canvas, curAlbum.name, name_len, xCoord, yCoord, CANVAS_COLOR_BW_BLACK, 2, 0);
        yCoord += 16;
    }

    canvas_refresh_screen(canvas);

    //Set epd to partial mode
    canvas_init_partial(canvas);
    //canvas_draw_rect(canvas, 0, 0, 16, 127, CANVAS_COLOR_BW_BLACK, DOT_SIZE_1X1, DRAW_FILL_FULL);
    canvas_draw_circle(canvas, 8, 20, 4, CANVAS_COLOR_BW_BLACK, DOT_SIZE_1X1, DRAW_FILL_FULL);
    canvas_refresh_partial(canvas, 0, 0, 16, 127);
    //canvas_refresh_screen(canvas);
}

void MainMenu::update_menu() {

}

void MainMenu::button_input(uint8_t buttonCode) {

}

void MainMenu::close_menu() {

}
#pragma endregion