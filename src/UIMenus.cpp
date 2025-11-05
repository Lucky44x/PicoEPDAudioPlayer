#include "UIMenus.h"

static const uint8_t all_songs_icon[32] = {
    0xfc, 0x7f, 0xf7, 0xcf, 0xd9, 0xb7, 0xa1, 0x8b, 0xa1, 0x07, 0x40, 0x05, 0x42, 0x01, 0x45, 0xf8,
    0x45, 0xe0, 0x42, 0x2e, 0x40, 0x2e, 0xa0, 0x2e, 0xa0, 0x2e, 0xd8, 0x6c, 0xf7, 0x8c, 0xfc, 0x5f
};

static const uint8_t shuffle_on_icon[32] = {
    0xff, 0xff, 0xff, 0xff, 0xff, 0xf7, 0xff, 0xf3, 0x83, 0x81, 0xc9, 0x11, 0xfc, 0x73, 0xfe, 0xff,
    0xfe, 0x77, 0xfc, 0x73, 0xa1, 0x09, 0x83, 0x81, 0xff, 0xf3, 0xff, 0xf7, 0xff, 0xff, 0xff, 0xff
};

static const uint8_t shuffle_off_icon[32] = {
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xef, 0xe0, 0x07, 0xff, 0xef, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xef, 0xe0, 0x07, 0xff, 0xef, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};

static const uint8_t loop_song_icon[32] = {
    0xff, 0xff, 0xff, 0xff, 0xff, 0xef, 0xe0, 0x03, 0xc0, 0x03, 0x9f, 0xef, 0xbf, 0xff, 0xbf, 0xfd, 
    0xbf, 0xfd, 0xff, 0xfd, 0xf7, 0xf9, 0xc0, 0x03, 0xc0, 0x07, 0xf7, 0xff, 0xff, 0xff, 0xff, 0xff
};

static const uint8_t loop_list_icon[32] = {
    0xff, 0xff, 0x00, 0x07, 0x00, 0x07, 0xff, 0xff, 0xff, 0xff, 0x00, 0x07, 0x00, 0x07, 0xff, 0xff, 
    0xff, 0xff, 0x00, 0x7f, 0x00, 0x67, 0xff, 0xe1, 0xff, 0xe0, 0xff, 0xe3, 0xff, 0xef, 0xff, 0xff
};


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

#pragma region Error
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

void ErrorMenu::button_input(InputEvent &e) {
    if (e.code != 0 && m_fallback != nullptr) {
        //Switch to fallback menu
        parentManager->switch_menu(m_fallback);
        m_fallback = nullptr;
    }
}

void ErrorMenu::update_menu() {}
void ErrorMenu::close_menu() {}
#pragma endregion NoFilesystem

#pragma region Mainmenu
//Main Manager
MainMenu::MainMenu(UIManager *parent, FileManager *fm) : UIMenu(parent), fm(fm) {}

void MainMenu::start_menu() {
    printf("Starting Main-Menu\n");
}

void MainMenu::draw_menu(canvas_config_t *canvas) {
    cached_canvas = canvas;
    canvas_set_colorscale(canvas, 2);
    canvas_update_color_depth(canvas);

    canvas_clear(canvas, CANVAS_COLOR_BW_WHITE);

    uint32_t xCoord = 16;
    uint32_t yCoord = 4;

    uint16_t allArr[9] = { 'A', 'l', 'l', ' ', 'S', 'o', 'n', 'g', 's' };
    canvas_draw_text(canvas, allArr, 9, xCoord, yCoord, CANVAS_COLOR_BW_BLACK, 2, 0);
    canvas_draw_bitmap(canvas, (const uint8_t *)&all_songs_icon, 272, yCoord, 16, 16, 1, true);
    yCoord += 16;

    uint16_t albumNum = fm->read_album_count();
    printf("Album count: %u\n", albumNum);
    for (uint i = 0; i < albumNum; i++) {
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

    //Make sure no ghosting is left here
    canvas_draw_rect(canvas, 8, 4 + (selected_index * 16), 12, 20 + (selected_index * 16), CANVAS_COLOR_BW_BLACK, DOT_SIZE_1X1, DRAW_FILL_FULL);
    canvas_refresh_screen_fast(canvas);
    updates = 0;
}

void MainMenu::button_input(InputEvent &e) {
    if (e.type == InputManager::EVENT_RELEASE) return;

    // UI stuff
    if (e.code == BUTTON_SELECT) {
        songMenu->init(selected_index);
        parentManager->switch_menu(songMenu);
        return;
    }

    if (e.code == BUTTON_DOWN) selected_index += 1;
    if (e.code == BUTTON_UP) selected_index -= 1;
    selected_index = selected_index < 0 ?  0 : selected_index;
    selected_index = selected_index > fm->read_album_count() ? fm->read_album_count() : selected_index;

    if (cached_canvas == nullptr) return;
    canvas_clear_partial(cached_canvas, 0, 0, 16, 127, CANVAS_COLOR_BW_WHITE);
    canvas_draw_rect(cached_canvas, 8, 4 + (selected_index * 16), 12, 20 + (selected_index * 16), CANVAS_COLOR_BW_BLACK, DOT_SIZE_1X1, DRAW_FILL_FULL);

    if (updates >= 8) {
        draw_menu(cached_canvas);
    } else {
        canvas_refresh_screen_fast(cached_canvas);
        updates++;
    }
}

void MainMenu::close_menu(){}
void MainMenu::update_menu(){}
#pragma endregion

#pragma region Song-Menu

SongMenu::SongMenu(UIManager *parent, FileManager *fm, MainMenu *mainMenu, PlaybackMenu *playbackMenu, ErrorMenu *errorMenu) : UIMenu(parent), fm(fm), mainMenu(mainMenu), playbackMenu(playbackMenu), errorMenu(errorMenu) {}

void SongMenu::start_menu() {

}

void SongMenu::button_input(InputEvent &e) {
    if (e.type == InputManager::EVENT_RELEASE) return;

    // UI stuff
    if (e.code == BUTTON_SELECT) {
        if (selected_index == 0) {
            parentManager->switch_menu(mainMenu);
            return;
        }
        FRESULT fr = playbackMenu->init(selected_index - 1, album_record.song_count == 0 ? -1 : albumID - 1);
        if (fr != FR_OK) {
            errorMenu->set_fallback(this);
            errorMenu->set_message_utf8("Could not read this song");
            parentManager->switch_menu(errorMenu);
            return;
        }

        parentManager->switch_menu(playbackMenu);
        bool playback_ok = playbackMenu->begin_playback();
        if (!playback_ok) {
            errorMenu->set_fallback(this);
            errorMenu->set_message_utf8("Could not start this song");
            parentManager->switch_menu(errorMenu);
            return;
        }
        return;
    }

    // UI stuff
    if (e.code == BUTTON_DOWN) selected_index += 1;
    if (e.code == BUTTON_UP) selected_index -=1;
    selected_index = selected_index < 0 ? 0 : selected_index;   //Clamp to 0
    if (album_record.song_count == 0) selected_index = selected_index >= fm->read_song_count() ? fm->read_song_count()-1 : selected_index;
    else selected_index = selected_index >= album_record.song_count ? album_record.song_count - 1 : selected_index;

    if (cached_canvas == nullptr) return;
    canvas_clear_partial(cached_canvas, 0, 0, 16, 127, CANVAS_COLOR_BW_WHITE);
    canvas_draw_rect(cached_canvas, 8, 4 + (selected_index * 16), 12, 20 + (selected_index * 16), CANVAS_COLOR_BW_BLACK, DOT_SIZE_1X1, DRAW_FILL_FULL);

    if (updates >= 8) {
        draw_menu(cached_canvas);
    } else {
        canvas_refresh_screen_fast(cached_canvas);
        updates++;
    }
}

void SongMenu::draw_menu(canvas_config_t *canvas) {
    cached_canvas = canvas;
    canvas_set_colorscale(canvas, 2);
    canvas_update_color_depth(canvas);

    canvas_clear(canvas, CANVAS_COLOR_BW_WHITE);

    uint32_t xCoord = 16;
    uint32_t yCoord = 4;

    uint16_t allArr[6] = { '<', ' ', 'B', 'a', 'c', 'k' };
    canvas_draw_text(canvas, allArr, 6, xCoord, yCoord, CANVAS_COLOR_BW_BLACK, 2, 0);
    yCoord += 16;

    uint32_t songCount = 0;
    if (album_record.song_count == 0) songCount = fm->read_song_count();
    else songCount = album_record.song_count;
    songCount = MIN(songCount, 6);

    for (uint i = 0; i < songCount; i++) {
        song_record_t song_record;
        FRESULT fr;
        if (album_record.song_count != 0) fr = fm->read_song_album_index(i, &album_record, &song_record);
        else fr = fm->read_song_index(i, &song_record);

        size_t name_len = cp_len_0term(song_record.name, SONG_NAME_CODEPOINTS);
        //print_utf16le(song_record.name, name_len);
        canvas_draw_text(canvas, song_record.name, name_len, xCoord, yCoord, CANVAS_COLOR_BW_BLACK, 2, 0);
        yCoord += 16;
    }

    canvas_refresh_screen(canvas);

    //Make sure no ghosting is left here
    canvas_draw_rect(canvas, 8, 4 + (selected_index * 16), 12, 20 + (selected_index * 16), CANVAS_COLOR_BW_BLACK, DOT_SIZE_1X1, DRAW_FILL_FULL);
    canvas_refresh_screen_fast(canvas);
    updates = 0;
}

void SongMenu::init(uint16_t new_albumID) {
    // Album ID is always offsey by 1 to encode the virtual Album "ALL"
    this->albumID = new_albumID;
    if (new_albumID == 0) {
        // All songs - virtual album
        album_record.song_count = 0;
    }
    else {
        // Actual stored album
        fm->read_album_index(new_albumID - 1, &this->album_record);
    }
}

void SongMenu::close_menu(){}
void SongMenu::update_menu(){}

#pragma endregion

#pragma region Playback-Menu

PlaybackMenu::PlaybackMenu(UIManager *parent, FileManager *fm, AudioCore *ac, InputManager *im) : UIMenu(parent), fm(fm), ac(ac), im(im) {}

void PlaybackMenu::start_menu() {
    shuffle_mode = false;
    loop_mode = false;
    running = false;
}

void PlaybackMenu::draw_menu(canvas_config_t *canvas) {
    cached_canvas = canvas;
    canvas_set_colorscale(canvas, 4);
    canvas_update_color_depth(canvas);

    canvas_clear(canvas, CANVAS_COLOR_GRAY_G1);

    uint8_t imageBuf[3600];
    fm->read_image_index(song_record.image_id, imageBuf);
    canvas_draw_bitmap(canvas, imageBuf, 4, 8, 120, 120, 2, false);

    // Draw Mode-Icons
    canvas_draw_bitmap(canvas,(const uint8_t *) (loop_mode ? &loop_song_icon : &loop_list_icon), 272, 20, 16, 16, 1, false);
    canvas_draw_bitmap(canvas,(const uint8_t *) (shuffle_mode ? &shuffle_on_icon : &shuffle_off_icon), 252, 20, 16, 16, 1, false);

    size_t name_len = cp_len_0term(song_record.name, SONG_NAME_CODEPOINTS);
    canvas_draw_text(canvas, song_record.name, name_len, 128, 56, CANVAS_COLOR_GRAY_G4, 2, 176);
    
    name_len = cp_len_0term(artist_record.name, ARTIST_NAME_CODEPOINTS);
    canvas_draw_text(canvas, artist_record.name, name_len, 128, 72, CANVAS_COLOR_GRAY_G3, 2, 176);

    canvas_refresh_screen(canvas);
}

FRESULT PlaybackMenu::init(uint32_t song_idx, uint32_t album_idx) {
    if (song_idx < 0) song_idx = 0; // Do not wrap backwards just forwards (may change this later but not for now)
    this->songID = song_idx;
    this->albumID = album_idx;

    FRESULT fr;

    // Global song list
    if (albumID == -1) {
        if (song_idx >= fm->read_song_count()) {
            // In this case, assume loop behavivour
            //TODO: Add shuffle behaviour
            song_idx = 0;
        }

        fr = fm->read_song_index(song_idx, &song_record);
        if (fr != FR_OK) return fr;
        fr = fm->read_artist_index(song_record.artist_id, &artist_record);
        global_song_id = song_idx;
    }
    else {
        // Album song list
        fr = fm->read_album_index(album_idx, &album_record);
        if (fr != FR_OK) return fr;

        if (song_idx >= album_record.song_count) {
            // In this case, assume loop behavivour
            //TODO: Add shuffle behaviour
            song_idx = 0;
        }

        fr = fm->read_song_album_index(song_idx, &album_record, &song_record);
        if (fr != FR_OK) return fr;
        fr = fm->read_artist_index(song_record.artist_id, &artist_record);
        if (fr != FR_OK) return fr;
        fr = fm->read_song_index_from_album(song_idx, &album_record, &global_song_id);
        if (fr != FR_OK) return fr;
    }

    if (!ac->start_song(global_song_id)) { 
        ac->stop();
        ac->close();
        return FR_DENIED;
    }

    return fr;
}

bool PlaybackMenu::begin_playback() {
    ac->mute(true);
    if (!ac->start_song(global_song_id)) { 
        ac->stop();
        ac->close();
        return false;
    }
    ac->mute(false);
    return true;
}

void PlaybackMenu::button_input(InputEvent &e) {
    if (e.type == InputManager::EVENT_RELEASE) return;

    if (e.code ==  BUTTON_UP) {
        ac->change_volume(1);
        return;
    } else if (e.code == BUTTON_DOWN) {
        ac->change_volume(-1);
        return;
    }

    if (im->get_state(BUTTON_SELECT) == InputManager::EVENT_PRESS) {
        // Special functions
        switch (e.code)
        {
            case BUTTON_NEXT:
                switch_loop();
            break;
            case BUTTON_PREV:
            break;
            case BUTTON_PLAY:
                ac->stop();
                parentManager->switch_menu(sm);
            break;
            default: break;
        }
        return;
    }

    // Normal functions
    switch (e.code) {
        case BUTTON_NEXT:
            skip_forwards();
        break;
        case BUTTON_PREV:
            skip_backwards();
        break;
        case BUTTON_PLAY:
            if (ac->isPaused()) ac->pause(false);
            else ac->pause(true);
        break;
    }
}

void PlaybackMenu::skip_forwards() {
    ac->stop();
    sleep_us(500);
    ac->pause(true);
    ac->mute(true);
    init(songID + 1, albumID);
    parentManager->redraw();
    begin_playback();
    ac->pause(false);
    ac->mute(false);
}

void PlaybackMenu::skip_backwards() {
    ac->stop();
    sleep_us(500);
    ac->pause(true);
    ac->mute(true);
    init(songID - 1, albumID);
    parentManager->redraw();
    begin_playback();
    ac->pause(false);
    ac->mute(false);
}

void PlaybackMenu::reset_current_song() {
    ac->stop();
    sleep_us(500);
    ac->pause(true);
    ac->mute(true);
    init(songID, albumID);
    begin_playback();
    ac->pause(false);
    ac->mute(false);
}

void PlaybackMenu::switch_loop() {
    //ac->mute(true);
    ac->pause(true);
    ac->mute(true);
    loop_mode = !loop_mode;

    if (cached_canvas == NULL) return;
    canvas_clear_partial(cached_canvas, 272, 20, 288, 36, CANVAS_COLOR_GRAY_G1);
    canvas_draw_bitmap(cached_canvas, (const uint8_t *) (loop_mode ? &loop_song_icon : &loop_list_icon), 272, 20, 16, 16, 1, false);
    canvas_refresh_screen(cached_canvas);
    ac->pause(false);
    ac->mute(false);
    //if(!ac->isPaused()) ac->mute(false);
}

void PlaybackMenu::update_menu() {
    if (ac->awaitingNext()) {
        if(loop_mode) reset_current_song();
        else skip_forwards();
    }
}

void PlaybackMenu::close_menu(){}

#pragma endregion