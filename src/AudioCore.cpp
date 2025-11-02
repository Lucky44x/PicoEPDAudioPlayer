#include "AudioCore.h"
#include <math.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

void AudioCore::configure(float freq_hz, uint32_t sample_rate_hz) {
    this->freq_hz_ = freq_hz;
    sample_rate_ = sample_rate_hz;
    _recalcPhaseIncrement();
}

bool AudioCore::start() {
    if (!player_ || !audio_player_get_producer(player_)) return false;

    // Prime a few buffers so DMA has data
    for (int i = 0; i < 3; i++) {
        audio_buffer_t *b = audio_player_take_buffer(player_, true);
        if (!b) break;

        const size_t frames = b->max_sample_count;

        int16_t* dst = reinterpret_cast<int16_t*>(b->buffer->bytes);
        for (size_t n = 0; n < frames; ++n) {
            double s = std::sin(phase_);
            phase_ += phase_inc_;
            if (phase_ > M_PI * 2.0) phase_ -= 2.0 * M_PI;

            int16_t sample = static_cast<int16_t>(s * amplitude_);
            dst[2 * n + 0] = sample;
            dst[2 * n + 1] = sample;
        }

        b->sample_count = frames;
        audio_player_queue_buffer(player_, b);
    }

    if (!audio_player_start(player_)) return false;
    running_ = true;
    return true;
}

void AudioCore::stop() {
    if (!player_) return;
    audio_player_stop(player_);
    running_ = false;
}

void AudioCore::pump() {
    if (!running_ || !player_) return;

    // Fill as many free buffers as we can without blocking the UI loop.
    for (;;) {
        audio_buffer_t* b = audio_player_take_buffer(player_, /*block=*/false);
        if (!b) break;

        const size_t frames = b->max_sample_count;
        int16_t* dst = reinterpret_cast<int16_t*>(b->buffer->bytes);

        for (size_t n = 0; n < frames; ++n) {
            double s = std::sin(phase_);
            phase_ += phase_inc_;
            if (phase_ >= 2.0 * M_PI) phase_ -= 2.0 * M_PI;

            int16_t sample = static_cast<int16_t>(s * amplitude_);
            dst[2 * n + 0] = sample; // L
            dst[2 * n + 1] = sample; // R
        }

        b->sample_count = frames;
        audio_player_queue_buffer(player_, b);
    }
}

void AudioCore::setFrequency(float freq_hz) {
    freq_hz_ = freq_hz;
    _recalcPhaseIncrement();
}

void AudioCore::_recalcPhaseIncrement() {
    phase_inc_ = 2.0 * M_PI * static_cast<double>(freq_hz_) / static_cast<double>(sample_rate_);
}