#ifndef UIMENU_H
#define UIMENU_H

#include "pico/stdlib.h"
#include "stdio.h"
#include <memory.h>
#include "files.h"
#include "input.h"
#include "AudioCore.h"

extern "C" {
    #include "epdDraw.h"
}

#define BUTTON_PREV 1
#define BUTTON_PLAY 2
#define BUTTON_NEXT 3
#define BUTTON_UP 4
#define BUTTON_SELECT 5
#define BUTTON_DOWN 6

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
class PlaybackMenu;
class SongMenu;
class MainMenu;
class ErrorMenu;

class UIManager {
    public:
       UIManager();
        void switch_menu(UIMenu *newMenu);
        void input(InputEvent &event);
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
        virtual void button_input(InputEvent &e) = 0;
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
        void button_input(InputEvent &e) override;
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
        void button_input(InputEvent &e) override;
        void draw_menu(canvas_config_t *canvas) override;
        void update_menu() override;
        void close_menu() override;
        void setup(SongMenu *songMenu) { this->songMenu = songMenu; };
    private:
        SongMenu *songMenu;
        FileManager *fm;
        uint selected_index = 0;
        uint updates = 0;
        canvas_config_t *cached_canvas;
};

class PlaybackMenu : public UIMenu {
    public:
        explicit PlaybackMenu(UIManager *parent, FileManager *fm, AudioCore *ac, InputManager *im);
        void start_menu() override;
        void button_input(InputEvent &e) override;
        void draw_menu(canvas_config_t *canvas) override;
        void update_menu() override;
        void close_menu() override;
        FRESULT init(uint32_t songID, uint32_t albumID);

        void setup(SongMenu *sm) { this->sm = sm; };
    private:
        void skip_forwards();
        void skip_backwards();
        void switch_loop();
        //void switch_shuffle();

        uint32_t songID;
        uint32_t global_song_id;
        uint32_t albumID;

        bool running;
        bool loop_mode;         //  True -> Song-Loop   False -> List-Loop
        bool shuffle_mode;      //  True -> Shuffle On  False -> Shuffle off

        song_record_t song_record;
        album_record_t album_record;
        artist_record_t artist_record;

        FileManager *fm;
        AudioCore *ac;
        InputManager *im;
        SongMenu *sm;

        canvas_config_t *cached_canvas;
};

class SongMenu : public UIMenu {
    public:
        explicit SongMenu(UIManager *parent, FileManager *fm, MainMenu *mainMenu, PlaybackMenu *playbackMenu, ErrorMenu *errorMenu);

        void start_menu() override;
        void button_input(InputEvent &e) override;
        void draw_menu(canvas_config_t *canvas) override;
        void update_menu() override;
        void close_menu() override;
        void init(uint16_t albumID);
    private:
        MainMenu *mainMenu;
        PlaybackMenu *playbackMenu;
        ErrorMenu *errorMenu;
        FileManager *fm;

        uint16_t albumID;
        album_record_t album_record;
        uint32_t selected_index;
        uint32_t updates = 0;
        canvas_config_t *cached_canvas;
};

#endif