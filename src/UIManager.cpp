#include "UIMenus.h"

/*
 *  Most of this functionallity could have been handeled via a Stack and it would've been much safer and easier to work with, but whatever 
 */

UIManager::UIManager() {}

void UIManager::init() {
    canvas_cfg = canvas_build(2, CANVAS_ROTATE_90, CANVAS_COLOR_BW_WHITE);
    canvas_init(&canvas_cfg);
}

void UIManager::switch_menu(UIMenu *menu) {
    if (currentMenu) currentMenu->close_menu();
    currentMenu = menu;
    redraw();
    currentMenu->start_menu();
}

void UIManager::update() {
    // E-Ink service
    epd_service_async(&canvas_cfg.driverConfig, 10000);

    if (currentMenu != NULL) currentMenu->update_menu();
}

void UIManager::redraw() {
    if (currentMenu != NULL) currentMenu->draw_menu(&canvas_cfg);
    //printf("Is active: %u\n", currentMenu != NULL);
}

void UIManager::input(InputEvent &event) {
    if (currentMenu != NULL) currentMenu->button_input(event);
}