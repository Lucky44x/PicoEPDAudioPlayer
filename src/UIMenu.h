#ifndef UIMENU_H
#define UIMENU_H

#include "UIManager.h"

class UIMenu {
    public:
        void setup_menu(UIManager &parentManager);
    private:
        UIManager &parentManager;
};

#endif