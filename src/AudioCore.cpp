#include "AudioCore.h"
#include <math.h>
#include <string.h>

AudioCore::AudioCore() {}

bool AudioCore::start_song(uint32_t song_index) {
    if (!m_player || !audio_player_get_producer(m_player)) return false;

    FRESULT file_result = m_fm->open_song_file(song_index);
    if (file_result != FR_OK) return false;

    drwav_bool32 wav_ok = drwav_init(&m_wav, wav_read, wav_seek, wav_tell, &(m_fm->current_song_file), NULL);
    if (!wav_ok) return false;
    m_eof = false;
    m_running = true;
    return true;
}

bool AudioCore::open() {
    if (!m_player || !audio_player_get_producer(m_player)) return false;
    if (!audio_player_start(m_player)) return false;
    m_running = false;
    m_eof = false;
    return true;
}

void AudioCore::close() {
    stop();
    audio_player_stop(m_player);
    m_running = false;
}

void AudioCore::stop() {
    if (!m_player) return;
    drwav_uninit(&m_wav);
    m_fm->close_song_file();
}

void AudioCore::mute(bool muted) {
    if(!m_player) return;
    audio_player_mute(muted);
}

void AudioCore::pump() {
    if (!m_running || !m_player || m_eof) return;

    for (;;) {
        audio_buffer_t *b = audio_player_take_buffer(m_player, false);
        if (!b) break;

        const size_t want_frames = b->max_sample_count;
        int16_t *dst = (int16_t *)b->buffer->bytes;

        size_t frames_read = 0;
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
            //EOF
            m_eof = true;
            break;
        }

        b->sample_count = frames_read;
        audio_player_queue_buffer(m_player, b);
        if (frames_read < want_frames) {
            //EOF
            m_eof = true;
            break;
        }
    }

    if (m_eof) stop();
}
