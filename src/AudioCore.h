#ifndef AUDIOCORE_H
#define AUDIOCORE_H

extern "C" {
#include "audioPlayer.h"
}

class AudioCore {
    public:
        explicit AudioCore(audio_player_handle_t *player) : player_(player) {}

        void configure(float freq_hz, uint32_t sample_rate_hz);
        bool start();
        void stop();
        void pump();
        void setFrequency(float freq_hz);
    private:
        void _recalcPhaseIncrement();

        bool running_ = false;
        audio_player_handle_t *player_ = nullptr;

        double phase_ = 0.0;
        double phase_inc_ = 0.0;
        float freq_hz_ = 323.6f;
        uint32_t sample_rate_ = 44100;

        // amplitude
        int16_t amplitude_ = 28000;
};

#endif  //AUDIOCORE_H