#include "UIMenus.h"

//Main Manager
MainMenu::MainMenu(UIManager *parent, FileManager *fm) : UIMenu(parent, fm) {}

void MainMenu::start_menu() {
    printf("Starting Main-Menu\n");
}

void MainMenu::draw_menu(canvas_config_t *canvas) {
    canvas_clear(canvas, CANVAS_COLOR_GRAY_G1);

    /*
    uint32_t xCoord = 32;
    uint32_t yCoord = 16;

    uint16_t allArr[8];
    size_t allLen = utf8_to_16arr("All", allArr, 8);
    canvas_draw_text(canvas, allArr, allLen, xCoord, yCoord, CANVAS_COLOR_GRAY_G4, 2, 999);
    yCoord += 16;

    uint32_t album_count = fm->read_album_count();
    for (uint i = 0; i < (album_count < 4) ? album_count : 4; i++) {
        album_record_t curAlbum;
        fm->read_album_index(i, &curAlbum);
        canvas_draw_text(canvas, curAlbum.name, 22, xCoord, yCoord, CANVAS_COLOR_GRAY_G4, 2, 999);
        yCoord += 16;
    }
    */
   canvas_draw_rect(canvas, 0, 0, 64, 64, CANVAS_COLOR_GRAY_G4, CANVAS_DOT_SIZE_DFT, DRAW_FILL_FULL);
}

void MainMenu::update_menu() {

}

void MainMenu::button_input(uint8_t buttonCode) {

}

void MainMenu::close_menu() {

}