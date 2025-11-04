#include "UIMenus.h"

UIManager::UIManager() {}

void UIManager::init() {
    canvas_cfg = canvas_build(2, CANVAS_ROTATE_90, CANVAS_COLOR_BW_WHITE);
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
    printf("Is active: %u\n", currentMenu != NULL);
}

void UIManager::input(InputEvent &event) {
    if (currentMenu != NULL) currentMenu->button_input(event);
}