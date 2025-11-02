#include "pico/util/buffer.h"
#include <stdlib.h>

mem_buffer_t *pico_buffer_alloc(size_t size) {
    mem_buffer_t *buf = (mem_buffer_t *)malloc(sizeof(mem_buffer_t));
    if (!buf) return NULL;
    buf->bytes = NULL;
    buf->size  = 0;

    if (size) {
        void *mem = malloc(size);
        if (!mem) { free(buf); return NULL; }
        buf->bytes = mem;
        buf->size  = size;
    }
    return buf;
}

void pico_buffer_free(mem_buffer_t *buf) {
    if (!buf) return;
    if (buf->bytes) free(buf->bytes);
    free(buf);
}
