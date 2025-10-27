#ifndef AUDIOMANAGER_H
#define AUDIOMANAGER_H

#include <stdint.h>
#include "fileManager.h"
#include "dr_wav.h"

extern "C" {
    #include "pico/audio_i2s.h"
}
#include "pico/stdlib.h"

static inline void pcm5102a_unmute(uint xsm_pin) {
    if (xsm_pin != (uint)-1) {
        gpio_init(xsm_pin);
        gpio_set_dir(xsm_pin, GPIO_OUT);
        gpio_put(xsm_pin, 1); // 1 = unmute on PCM5102A
    }
}

class AUDIOMANAGER {
    public:
        AUDIOMANAGER() = default;
        explicit AUDIOMANAGER(FILEMANAGER& fileManager);

        bool play_first_frames(uint32_t index, size_t max_frames_total);
        void play_song(uint32_t songIndex);
        void test();
    private:
        void init_audio_output(uint32_t sample_rate);
        size_t push_pcm_to_producer(const int16_t* pcm_src, size_t frames);
        void initialize_playback(uint32_t songIndex);
        void decode_chunk();
        void end_playback();
        void playback_loop();


        //drmp3 mp3Instance;
        drwav wavInstance;
        FILEMANAGER& fileManager;
        audio_buffer_pool_t *producer_pool = nullptr;
        audio_buffer_format_t producer_buffer_format;
        audio_format_t producer_format;
};

#endif