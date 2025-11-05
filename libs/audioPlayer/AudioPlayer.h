#ifndef AUDIOPLAYER_H
#define AUDIOPLAYER_H

#include <stdio.h>
#include "pico/stdlib.h"

#include "pico/audio_i2s.h"
#include "pico/audio.h"

typedef struct {
    int data_pin;          // DIN
    int clock_pin_base;    // BCK
    uint8_t pio_sm;         // which state machine to use
    uint8_t dma_ch0;        // two DMA channels for ping/pong
    uint8_t dma_ch1;        
    int xsmt_pin;          // mute
    int sample_rate;       // e.g., 44100
    bool s32;               // false => S16, true => S32
} audio_player_config_t;

typedef struct {
    audio_buffer_pool_t *producer_pool;                 // Producer - Pool
    bool running;                                       // Running State
} audio_player_handle_t;

#ifdef __cplusplus
extern "C" {
#endif

bool audio_player_init(audio_player_handle_t *h, const audio_player_config_t *cfg);
audio_buffer_pool_t* audio_player_get_producer(audio_player_handle_t *h);

// Start / stop the I2S Engine (DMA + PIO)
bool audio_player_start(audio_player_handle_t *h);
void audio_player_stop(audio_player_handle_t *h);

void audio_player_mute(bool muted);

// Helpers
audio_buffer_t* audio_player_take_buffer(audio_player_handle_t *h, bool block);
void audio_player_queue_buffer(audio_player_handle_t *h, audio_buffer_t *buf);
size_t audio_player_write_pcm(audio_player_handle_t *h, const int16_t *stereo, size_t frames);
void audio_player_prime_silence(audio_player_handle_t *h, int n_buffers);

#ifdef __cplusplus
}
#endif

#endif