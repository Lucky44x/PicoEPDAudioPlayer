#include "UIMenus.h"

UIManager::UIManager() {}

void UIManager::init() {
    canvas_cfg = canvas_build(4, CANVAS_ROTATE_270, CANVAS_COLOR_GRAY_G1);
    canvas_init(&canvas_cfg);
}

void UIManager::switch_menu(UIMenu *menu) {
    if (currentMenu) currentMenu->close_menu();
    currentMenu = menu;
    currentMenu->start_menu();
}

void UIManager::update() {
    if (currentMenu != NULL) currentMenu->update_menu();
}

void UIManager::redraw() {
    if (currentMenu != NULL) currentMenu->draw_menu(&canvas_cfg);
    canvas_refresh_screen(&canvas_cfg);
}