#ifndef UIMENU_H
#define UIMENU_H

#include "pico/stdlib.h"
#include "stdio.h"
#include <memory.h>
#include "files.h"

extern "C" {
#include "epdDraw.h"
}

static size_t utf8_to_16arr(const char* utf8, uint16_t* out, size_t max_len) {
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

static void print_hex(const char* label, const uint8_t* data, size_t len) {
    printf("%s (%u bytes):", label, (unsigned)len);
    for (size_t i = 0; i < len; ++i) {
        if ((i % 16) == 0) printf("\n%04u: ", (unsigned)i);
        printf("%02X ", data[i]);
    }
    printf("\n");
}


class UIMenu;
class UIManager;

class UIManager {
    public:
       UIManager();
        void switch_menu(UIMenu *newMenu);
        void update();
        void redraw();
        void init();
    private:
        UIMenu *currentMenu;
        canvas_config_t canvas_cfg;
};

class UIMenu {
    public:
        explicit UIMenu(UIManager *parent) : parentManager(parent) {}
        virtual ~UIMenu() = default;

        virtual void start_menu() = 0;
        virtual void button_input(uint8_t buttonCode) = 0;
        virtual void draw_menu(canvas_config_t *canvas) = 0;
        virtual void update_menu() = 0;
        virtual void close_menu() = 0;
    protected:
        UIManager *parentManager;
};

class ErrorMenu : public UIMenu {
    public:
        explicit ErrorMenu(UIManager *parent, FileManager *fm);

        //Setting
        void set_message_utf8(const char* msg_utf8);
        void set_message_cp(const uint16_t* msg_cp, size_t len);

        void set_fallback(UIMenu *fallback) { m_fallback = fallback; }

        void start_menu() override;
        void update_menu() override;
        void button_input(uint8_t buttonCode) override;
        void draw_menu(canvas_config_t *canvas) override;
        void close_menu() override;
    private:
        static constexpr size_t kMaxMsg = 64;
        uint16_t m_message[kMaxMsg] = {0};
        size_t m_msgLen = 0;
        UIMenu *m_fallback = nullptr;
        FileManager *m_fm;
};

class MainMenu : public UIMenu {
    public:
        explicit MainMenu(UIManager *parent, FileManager *fm);
        void start_menu() override;
        void update_menu() override;
        void button_input(uint8_t buttonCode) override;
        void draw_menu(canvas_config_t *canvas) override;
        void close_menu() override;
    private:
        FileManager *fm;
        uint selected_index = 0;
};

class SongMenu : public UIMenu {
    public:
        explicit SongMenu(UIManager *parent, uint16_t albumID);

        void start_menu() override;
        void button_input(uint8_t buttonCode) override;
        void draw_menu(canvas_config_t *canvas) override;
    private:
        uint16_t albumID;
};

class Playback : public UIMenu {
    public:
        explicit Playback(UIManager& parent);
        void start_menu() override;
        void button_input(uint8_t buttonCode) override;
};

#endif