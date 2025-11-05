#ifndef AUDIOCORE_H
#define AUDIOCORE_H

#include "files.h"
#include "dr_wav.h"

extern "C" {
#include "audioPlayer.h"
}

class AudioCore {
    public:
        AudioCore();
        explicit AudioCore(audio_player_handle_t *player, FileManager *fm) : m_player(player), m_fm(fm) {}

        bool start_song(uint32_t song_index);

        bool open();
        void close();
        void stop();
        void mute(bool muted);
        void pump();
        void change_volume(int delta);
        void pause(bool state);
        bool isPaused() const { return m_paused; }

        bool awaitingNext() const { return !m_running && m_eof; }
    private:
        void apply_volume(int16_t *s, size_t frames);

        //File
        drwav m_wav;

        //Stream Data
        bool m_paused = false;
        bool m_running = false;
        bool m_eof = false;
        uint16_t m_vol_q8_8 = 8; // 100%
        uint64_t played_samples;
        bool first_frames_finished = false;

        //Instances
        audio_player_handle_t *m_player = nullptr;
        FileManager *m_fm = nullptr;
};

#endif  //AUDIOCORE_H