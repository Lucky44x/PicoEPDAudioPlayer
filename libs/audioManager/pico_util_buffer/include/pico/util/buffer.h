#pragma once

// Minimal stand-in for Raspberry Pi "pico_util_buffer"
// Enough for Elehobica's pico_audio_32b

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct mem_buffer {
    void *bytes;     // pointer to raw bytes
    size_t size;     // total size in bytes
} mem_buffer_t;

// Allocate a mem_buffer_t + backing storage (size bytes). Returns NULL on failure.
// Caller frees with pico_buffer_free().
mem_buffer_t *pico_buffer_alloc(size_t size);

// Free a buffer allocated by pico_buffer_alloc(). Safe to pass NULL.
void pico_buffer_free(mem_buffer_t *buf);

#ifdef __cplusplus
}
#endif
