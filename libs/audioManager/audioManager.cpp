#define PICO_AUDIO_I2S_DMA_IRQ 1
#define PICO_AUDIO_I2S_PIO 0

#include "audioManager.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/dma.h"
#include "pico/audio_i2s.h"
#include <algorithm>
#include <math.h>

static constexpr drmp3_uint64 FRAMES_PER_CHUNK = 1152 * 4; // ~4 mp3 frames
static int16_t pcm[FRAMES_PER_CHUNK * 2]; // *2 covers stereo worst case

static constexpr int PRODUCER_BUF_COUNT = 4;
static constexpr int PRODUCER_SAMPLES_PER_BUF = 1024;

uint64_t cap_frames_left = 1152*260;

AUDIOMANAGER::AUDIOMANAGER(FILEMANAGER& fm) : fileManager(fm) {}

void AUDIOMANAGER::init_audio_output(uint32_t sample_rate) {
    // Claim two free channels so we don’t collide with SPI/SD DMA
    int ch0 = dma_claim_unused_channel(true);
    int ch1 = dma_claim_unused_channel(true);

    audio_i2s_config_t i2s_cfg = {
        .data_pin       = 8,   // DIN -> GP8
        .clock_pin_base = 6,   // BCK=GP6, LRCK=GP7
        .dma_channel0   = (uint8_t)ch0,
        .dma_channel1   = (uint8_t)ch1,
        .pio_sm         = 0,
    };

    pcm5102a_unmute(27);

    // 2) describe the input PCM 
    producer_format.sample_freq = sample_rate;
    producer_format.pcm_format = AUDIO_PCM_FORMAT_S16;
    producer_format.channel_count = AUDIO_CHANNEL_STEREO;

    producer_buffer_format.format = &producer_format;
    producer_buffer_format.sample_stride = 2 * producer_format.channel_count;

    // 3) Describe device output format that PIO expects
    audio_format_t output_format;
    output_format.sample_freq = sample_rate;
    output_format.pcm_format = AUDIO_PCM_FORMAT_S16;
    output_format.channel_count = AUDIO_CHANNEL_STEREO;

    // 4) call audio_i2s_setup to install PIO and configure DMA...
    const audio_format_t *ret = audio_i2s_setup(&producer_format, &output_format, &i2s_cfg);
    if (!ret) {
        printf("i2s setup failed \n");
        return;
    }

    // 5) create producer pool
    producer_pool = audio_new_producer_pool(&producer_buffer_format, PRODUCER_BUF_COUNT, PRODUCER_SAMPLES_PER_BUF);
    if (!producer_pool) {
        printf("audio_new_producer_pool failed");
        return;
    }

    // 6) Connect producer pool into the I2S consumer pipeline
    if (!audio_i2s_connect(producer_pool)) {
        printf("i2s could not connect");
        return;
    }

    // Prefill 2 zero-buffers
    for (int i = 0; i < 2; ++i) {
        audio_buffer_t* b = get_free_audio_buffer(producer_pool, true);
        memset(b->buffer->bytes, 0, PRODUCER_SAMPLES_PER_BUF * producer_buffer_format.sample_stride);
        b->sample_count = PRODUCER_SAMPLES_PER_BUF;
        give_audio_buffer(producer_pool, b);
    }

    /*
    // Optional: enqueue one short 440 Hz chunk to verify audio path
    {
        audio_buffer_t* b = get_free_audio_buffer(producer_pool, true);
        auto *p = (int16_t*)b->buffer->bytes;
        const float fs = (float)producer_format.sample_freq;
        for (int n = 0; n < PRODUCER_SAMPLES_PER_BUF; ++n) {
            float t = (float)n / fs;
            int16_t s = (int16_t)(0.3f * 32767.0f * sinf(2.0f * 3.14159265f * 320.0f * t));
            p[2*n+0] = s; // L
            p[2*n+1] = s; // R
        }
        b->sample_count = PRODUCER_SAMPLES_PER_BUF;
        give_audio_buffer(producer_pool, b);
    }
    */

    dma_channel_unclaim(ch0);
    dma_channel_unclaim(ch1);

    // 7) enable DMA/PIO transfers
    audio_i2s_set_enabled(true);

    printf("Audio output initialized: pool %d×%d frames\n", PRODUCER_BUF_COUNT, PRODUCER_SAMPLES_PER_BUF);
    printf("I2S ready on DMA ch %d/%d\n", ch0, ch1);
}


size_t AUDIOMANAGER::push_pcm_to_producer(const int16_t* pcm_src, size_t frames) {
    if (!producer_pool || frames == 0) return 0;

    size_t idx = 0;
    while (idx < frames) {
        // Non-blocking: if no free buffer, stop and report progress
        audio_buffer_t *buf = get_free_audio_buffer(producer_pool, false);
        if (!buf) break;

        int16_t* out = (int16_t*)buf->buffer->bytes;
        size_t to_write = std::min(frames - idx, (size_t)PRODUCER_SAMPLES_PER_BUF);

        memcpy(out, pcm_src + idx*2, to_write * 2 * sizeof(int16_t));
        buf->sample_count = (int)to_write;

        give_audio_buffer(producer_pool, buf);
        idx += to_write;
    }
    return idx; // frames actually queued
}

void AUDIOMANAGER::initialize_playback(uint32_t index) {
    FRESULT fr = this->fileManager.read_song_file(index);
    if (fr != FR_OK) { 
        printf("Could not read song file... %d", fr);
        return;
    }

    printf("First file pos: %d\n",(unsigned)f_tell(&fileManager.currentSongFile));

    bool ok = drmp3_init(&mp3Instance, mp3_read, mp3_seek, mp3_tell, NULL, &fileManager.currentSongFile, NULL);
    if (!ok) { 
        printf("Could not initialize song file playback mp3 instance");
        return;
    }

    init_audio_output(mp3Instance.sampleRate);

    //fileManager.wav_begin("capture.wav",mp3Instance.sampleRate,2,16);
}

// Decode only the first `max_frames_total` frames, then stop.
bool AUDIOMANAGER::play_first_frames(uint32_t index, size_t max_frames_total) {
    FRESULT fr = fileManager.read_song_file(index);
    if (fr != FR_OK) { printf("open fail %d\n", fr); return false; }

    // init dr_mp3 with safe callbacks
    if (!drmp3_init(&mp3Instance, mp3_read, mp3_seek, mp3_tell, NULL, &fileManager.currentSongFile, NULL)) {
        printf("drmp3_init failed\n");
        fileManager.close_song_file();
        return false;
    }

    // Init I2S output once we know sample rate
    init_audio_output(mp3Instance.sampleRate);

    const size_t CHUNK_FRAMES = 1152;        // one MP3 frame
    int16_t buf[CHUNK_FRAMES * 2];           // interleaved stereo

    size_t frames_left = max_frames_total;
    while (frames_left > 0) {
        drmp3_uint64 want = (frames_left < CHUNK_FRAMES) ? frames_left : CHUNK_FRAMES;

        // time-guard this read so we see if it blocks
        absolute_time_t t0 = get_absolute_time();
        drmp3_uint64 got = drmp3_read_pcm_frames_s16(&mp3Instance, want, buf);
        int64_t us = absolute_time_diff_us(t0, get_absolute_time());
        if (us > 200000) printf("decode took %lld us\n", (long long)us);

        if (got == 0) { printf("early EOS\n"); break; }

        // If mono, duplicate to stereo in-place
        if (mp3Instance.channels == 1) {
            for (drmp3_uint64 i = got; i-- > 0; ) {
                int16_t s = buf[i];
                buf[2*i+0] = s;
                buf[2*i+1] = s;
            }
        }

        // Push to I2S producer (non-blocking retries)
        size_t sent = 0;
        while (sent < (size_t)got) {
            size_t to_send = (size_t)got - sent;
            size_t n = push_pcm_to_producer(buf + sent*2, to_send);
            if (n == 0) { sleep_us(300); continue; }
            sent += n;
        }

        sleep_ms(2);

        frames_left -= (size_t)got;
        printf("queued %llu frames (remain %u)\n", (unsigned long long)got, (unsigned)frames_left);
    }

    sleep_ms(20); // let pipeline drain a touch
    audio_i2s_set_enabled(false);

    drmp3_uninit(&mp3Instance);
    fileManager.close_song_file();
    return true;
}

void AUDIOMANAGER::play_song(uint32_t index) {
    this->initialize_playback(index);
    printf("MP3 pos before decode: %u of %u\n",
       (unsigned)f_tell(&fileManager.currentSongFile),
       (unsigned)f_size(&fileManager.currentSongFile));
    playback_loop();
    end_playback();
}

void AUDIOMANAGER::decode_chunk() {
    drmp3_uint64 frames = drmp3_read_pcm_frames_s16(&mp3Instance, FRAMES_PER_CHUNK, pcm);
    if (frames == 0) return; // End of stream

    if (mp3Instance.channels == 1) {
        // Mono -> duplicate to stereo in-place (from end to front)
        for (drmp3_uint64 i = frames; i-- > 0; ) {
            int16_t s = pcm[i];
            pcm[2*i + 0] = s;
            pcm[2*i + 1] = s;
        }
    }
}

void AUDIOMANAGER::end_playback() {
    drmp3_uninit(&mp3Instance);
    fileManager.close_song_file();

    audio_i2s_set_enabled(false);
}

void AUDIOMANAGER::playback_loop() {
  for (;;) {
        printf("Playback Frame\n");

        // Decode up to FRAMES_PER_CHUNK frames into 'pcm' (interleaved)
        drmp3_uint64 frames = drmp3_read_pcm_frames_s16(&mp3Instance, FRAMES_PER_CHUNK, pcm);
        printf("Got: %llu frames \n", (unsigned long long)frames);

        if (frames > 0) {
            printf("read frames\n");
            printf("pcm frames: %llu\n", (unsigned long long)frames);
            printf("L0=%d R0=%d\n", pcm[0], (mp3Instance.channels==1? pcm[0] : pcm[1]));
        }

        if (frames == 0) {
            // EOS
            break;
        }

        // If mono input, duplicate to stereo (driver expects interleaved L,R)
        if (mp3Instance.channels == 1) {
            printf("Duplicating frames\n");
            for (drmp3_uint64 i = frames; i-- > 0; ) {
                int16_t s = pcm[i];
                pcm[2*i + 0] = s;
                pcm[2*i + 1] = s;
            }
            printf("Finished stereo\n");
        }

        /*
        //Write to WAV
        if (cap_frames_left > 0) {
            size_t to_cap = (frames > cap_frames_left) ? (size_t)cap_frames_left : (size_t)frames;
            fileManager.wav_write_pcm_s16(pcm, to_cap);
            cap_frames_left -= to_cap;
            if (cap_frames_left == 0) {
                fileManager.wav_end();
                printf("WAV FILE FINISHED\n");
            }
        }
        */

        printf("Starting push");
        /*
        size_t remaining = (size_t)frames;
        size_t sent = 0;
        while (sent < remaining) {
            size_t n = push_pcm_to_producer(pcm + sent*2, remaining - sent);
            if (n == 0) {
                // pipeline full — let IRQ/DMA drain
                sleep_us(300);
                continue;
            }
            sent += n;
            printf("Sent...");
        }
        */
    }

    printf("Song END");
    // Optionally wait a tiny moment to let the ring drain before stopping
    sleep_ms(10);
}

void AUDIOMANAGER::test() {

}

static void sample_pins_and_report(uint bck_pin, uint lrck_pin, uint din_pin, uint32_t ms=250) {
    gpio_set_dir(bck_pin,  GPIO_IN);
    gpio_set_dir(lrck_pin, GPIO_IN);
    gpio_set_dir(din_pin,  GPIO_IN);
    uint32_t t0 = to_ms_since_boot(get_absolute_time());
    uint32_t t1 = t0 + ms;
    int lb=gpio_get(bck_pin), ll=gpio_get(lrck_pin), ld=gpio_get(din_pin);
    uint32_t eb=0, el=0, ed=0;
    while (to_ms_since_boot(get_absolute_time()) < t1) {
        int vb=gpio_get(bck_pin); int vl=gpio_get(lrck_pin); int vd=gpio_get(din_pin);
        if (vb!=lb){eb++;lb=vb;} if (vl!=ll){el++;ll=vl;} if (vd!=ld){ed++;ld=vd;}
    }
    float s = ms/1000.f;
    printf("BCK~%.1fkHz LRCK~%.1fHz DIN edges=%u\n",
        (eb/(2*s))/1000.f, (el/(2*s)), ed);
}