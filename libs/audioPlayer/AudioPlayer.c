#include "audioPlayer.h"
#include "hardware/gpio.h"
#include <string.h>

// Formats
static audio_format_t i2s_in_fmt;
static audio_format_t i2s_out_fmt;
static audio_i2s_config_t i2s_cfg;

// Mute Pin
static uint xsmt_pin = 0xFFFF;

bool audio_player_init(audio_player_handle_t *h, const audio_player_config_t *cfg) {
    if (!h || !cfg) return false;           // Early return on invalid init

    xsmt_pin = cfg->xsmt_pin;
    if (xsmt_pin != 0xFFFF) {          // If XSMT defined
        gpio_init(cfg->xsmt_pin);
        gpio_set_dir(cfg->xsmt_pin, true);
        gpio_put(cfg->xsmt_pin, 0);         // Mute until started
    }

    // Init Input-Formats
    i2s_in_fmt.sample_freq = cfg->sample_rate;
    i2s_in_fmt.channel_count = AUDIO_CHANNEL_STEREO;
    i2s_in_fmt.pcm_format = cfg->s32 ? AUDIO_PCM_FORMAT_S32 : AUDIO_PCM_FORMAT_S16;

    audio_buffer_format_t producer_buf_fmt = {
        .format = &i2s_in_fmt,
        .sample_stride = (cfg->s32 ? 4 : 2) * i2s_in_fmt.channel_count,
    };

    // Keep Elehobic defaults: 2 buffers of 256 frames on the consumer
    // producer, 4-8 buffers
    const int producer_buffers = 6;
    const int frames_per_buf = 256;
    h->producer_pool = audio_new_producer_pool(&producer_buf_fmt, producer_buffers, frames_per_buf);
    if (!h->producer_pool) return false;

    // Init Output Formats
    i2s_out_fmt.sample_freq = cfg->sample_rate;
    i2s_out_fmt.channel_count = AUDIO_CHANNEL_STEREO;
    i2s_out_fmt.pcm_format    = cfg->s32 ? AUDIO_PCM_FORMAT_S32 : AUDIO_PCM_FORMAT_S16;

    // I2S Pin config
    i2s_cfg.data_pin = cfg->data_pin;
    i2s_cfg.clock_pin_base = cfg->clock_pin_base;
    i2s_cfg.pio_sm = cfg->pio_sm;
    i2s_cfg.dma_channel0 = cfg->dma_ch0;
    i2s_cfg.dma_channel1 = cfg->dma_ch1;

    // Bring up the PIO + DMA
    const audio_format_t *ok = audio_i2s_setup(&i2s_in_fmt, &i2s_out_fmt, &i2s_cfg);
    if(!ok) return false;

    // Connect
    if (!audio_i2s_connect_extra(h->producer_pool, false, 2, 256, NULL)) return false;
    
    h->running = false;
    return true;
}

// Start / stop the I2S Engine (DMA + PIO)
bool audio_player_start(audio_player_handle_t *h) {
    if (!h || h->running) return false;
    if (xsmt_pin != 0xFFFF) gpio_put(xsmt_pin, 1);  // unmute if defined

    audio_i2s_set_enabled(true);
    h->running = true;
    return true;
}

void audio_player_stop(audio_player_handle_t *h) {
    if (!h || !h->running) return;
    if (xsmt_pin != 0xFFFF) gpio_put(xsmt_pin, 0);  // mute if defined

    audio_i2s_set_enabled(false);
    h->running = false;
}

// Helpers
audio_buffer_pool_t* audio_player_get_producer(audio_player_handle_t *h) {
    return h ? h->producer_pool : NULL;
}

audio_buffer_t* audio_player_take_buffer(audio_player_handle_t *h, bool block) {
    if (!h || !h->producer_pool) return NULL;
    return producer_pool_take_buffer_default(h->producer_pool->connection, block);
}

void audio_player_queue_buffer(audio_player_handle_t *h, audio_buffer_t *buf) {
    if (!h || !h->producer_pool || !buf) return;
    producer_pool_give_buffer_default(h->producer_pool->connection, buf);
}

size_t audio_player_write_pcm(audio_player_handle_t *h, const int16_t *stereo, size_t frames) {
    if (!h || !stereo || i2s_in_fmt.pcm_format != AUDIO_PCM_FORMAT_S16) return 0;
    size_t done = 0;
    while (done < frames) {
        audio_buffer_t *b = audio_player_take_buffer(h, false);
        if (!b) break;  // No Space right now
        int16_t *dst = (int16_t *) b->buffer->bytes;
        size_t cap = b->max_sample_count;
        size_t n = (frames - done < cap) ? (frames - done) : cap;
        memcpy(dst, stereo + done * 2, n * 2 * sizeof(int16_t));
        b->sample_count = n;
        audio_player_queue_buffer(h, b);
        done += n;
    }
    return done;
}