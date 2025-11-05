#include "AudioCore.h"
#include <math.h>
#include <string.h>

static const uint16_t volume_table_q8_8[16] = {
    0,      //  mute
    1,      // -45 dB
    2,      // -41.8 dB
    3,      // -38.6 dB
    4,      // -35.4 dB
    6,      // -32.1 dB
    9,      // -28.9 dB
    13,     // -25.7 dB
    19,     // -22.5 dB
    27,     // -19.3 dB
    40,     // -16.1 dB
    58,     // -12.9 dB
    84,     // -9.6 dB
    122,    // -16.4 dB
    176,    // -3.2 dB
    256     // 0 dB 100%
};

AudioCore::AudioCore() {}

bool AudioCore::start_song(uint32_t song_index) {
    if (!m_player || !audio_player_get_producer(m_player)) return false;

    FRESULT file_result = m_fm->open_song_file(song_index);
    if (file_result != FR_OK) return false;

    drwav_bool32 wav_ok = drwav_init(&m_wav, wav_read, wav_seek, wav_tell, &(m_fm->current_song_file), NULL);
    if (!wav_ok) return false;
    m_eof = false;
    m_running = true;

    audio_player_prime_silence(m_player, 3);
    pause(false);
    return true;
}

bool AudioCore::open() {
    if (!m_player || !audio_player_get_producer(m_player)) return false;

    audio_player_mute(true);
    audio_player_prime_silence(m_player, 3);
    if (!audio_player_start(m_player)) return false;

    audio_player_mute(false);
    m_running = false;
    m_eof = false;
    return true;
}

void AudioCore::close() {
    stop();
    audio_player_stop(m_player);
}

void AudioCore::stop() {
    if (!m_player) return;
    //mute(true);
    pause(true);
    drwav_uninit(&m_wav);
    m_fm->close_song_file();
    m_running = false;
    played_samples = 0;
    first_frames_finished = false;
}

void AudioCore::mute(bool muted) {
    if(!m_player) return;
    audio_player_mute(muted);
}

void AudioCore::pause(bool state) {
    if (state) {
        // NOOP
    } else {
        // NOOP
    }
    m_paused = state;
}

void AudioCore::change_volume(int delta) {
    m_vol_q8_8 += delta;
    m_vol_q8_8 = m_vol_q8_8 < 0 ? 0 : m_vol_q8_8;
    m_vol_q8_8 = m_vol_q8_8 > 15 ? 15 : m_vol_q8_8;
}

void AudioCore::apply_volume(int16_t *s, size_t frames) {
    const uint32_t g = volume_table_q8_8[m_vol_q8_8];
    for (size_t i = 0; i < frames * 2; ++i) {
        int32_t v = (int32_t(s[i]) * int32_t(g)) >> 8;
        if (v > 32767) v = 32767;
        else if (v < -32767) v = -32767;
        s[i] = int16_t(v);
    }
}

void AudioCore::pump() {
    if (!m_player) return;

    for (;;) {
        audio_buffer_t *b = audio_player_take_buffer(m_player, false);
        if (!b) break;

        const size_t want_frames = b->max_sample_count;
        int16_t *dst = (int16_t *)b->buffer->bytes;
        memset(dst, 0, want_frames * 2 * sizeof(int16_t));

        size_t frames_read = 0;
        if (m_paused || m_eof || !m_running) {
            memset(dst, 0, want_frames * 2 * sizeof(int16_t));
            frames_read = want_frames;
        } else {
            if(m_wav.channels == 2) {
                frames_read = drwav_read_pcm_frames_s16(&m_wav, want_frames, dst);
            } else {
                frames_read = drwav_read_pcm_frames_s16(&m_wav, want_frames, dst);

                // Expand from end: [L0 L1 ...] -> [L0 R0 L1 R1 ...]
                for (size_t i = frames_read; i-- > 0; ) {
                    int16_t s = dst[i];
                    dst[2*i + 0] = s;
                    dst[2*i + 1] = s;
                }
            }

            if (frames_read == 0) {
                m_eof = true;
                break;
            }

            /*
            if (played_samples < 256) {
                memset(dst, 0, want_frames * 2 * sizeof(int16_t));
                played_samples += frames_read;
                printf("New state: %u", played_samples);
            } else {
                if (played_samples > 256 && !first_frames_finished) {
                    mute(false);
                    first_frames_finished = true;
                }
            }
            */

            apply_volume(dst, frames_read);
        } // if not paused

        b->sample_count = frames_read;
        audio_player_queue_buffer(m_player, b);
        if (frames_read < want_frames) {
            m_eof = true;
            break;
        }
    }

    if (m_eof) stop();
}