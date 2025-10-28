#ifndef UIMENU_H
#define UIMENU_H

#include "pico/stdlib.h"
#include "stdio.h"
#include <memory.h>

class UIMenu;
class UIManager;

class UIManager {
    public:
        UIManager() = default;
    private:
        UIMenu &currentMenu;
};

class UIMenu {
    public:
        explicit UIMenu(UIManager &parent) : parentManager(parent) {}
        virtual ~UIMenu() = default;

        virtual void start_menu() {}
        virtual void button_input(uint8_t buttonCode) = 0;
        virtual void draw_menu();
    protected:
        UIManager &parentManager;
};

class SongMenu : public UIMenu {
    public:
        explicit SongMenu(UIManager &parent, uint16_t albumID);

        void start_menu() override;
        void button_input(uint8_t buttonCode) override;
        void draw_menu() override;
    private:
        uint16_t albumID;
};

class AlbumMenu : UIMenu {
    public:
        explicit AlbumMenu(UIManager &parent);
        void start_menu() override;
        void button_input(uint8_t buttonCode) override;
};

class ArtistMenu : UIMenu {
    public:
        explicit ArtistMenu(UIManager& parent);
        void start_menu() override;
        void button_input(uint8_t buttonCode) override;
};

class Playback : UIMenu {
    public:
        explicit Playback(UIManager& parent);
        void start_menu() override;
        void button_input(uint8_t buttonCode) override;
};

#endif