#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"

#include "AudioCore.h"

audio_player_handle_t g_player;
AudioCore *g_core = nullptr;

void core1_entry() {
    audio_player_config_t cfg = {
        .data_pin = 8,
        .clock_pin_base = 6,
        .pio_sm = 0,
        .dma_ch0 = 0,
        .dma_ch1 = 1,
        .xsmt_pin = 27,
        .sample_rate = 44100,
        .s32 = false,
    };

    if (!audio_player_init(&g_player, &cfg)) {
        printf("AudioPlayer inti failed\n");
        multicore_fifo_push_blocking(0xBAD); // signal failure
        while (true) sleep_ms(100);
    }

    // signal core0 that AudioPlayer is ready
    multicore_fifo_push_blocking(0xA11D0);   // any magic value you like
    while(true) { printf("Hello from core1"); sleep_ms(100); }
}

int main()
{
    stdio_init_all();
    sleep_ms(5000); //Allow USB serial
    printf("Starting Audio Test\n");

    // Launch DAC-Thread
    multicore_launch_core1(core1_entry);

    uint32_t token = multicore_fifo_pop_blocking();
    if(token != 0xA11D0) {
        printf("Core1 init failed");
        while(true) sleep_ms(100);
    }
    printf("Core1 ready");

    static AudioCore core(&g_player);
    g_core = &core;

    core.configure(323.6f, 44100);
    if (!core.start()) {
        printf("Audio core could not start");
        while(true) sleep_ms(100);
    }

    printf("Tone test started..\n");

    while (true) {
        core.pump();
        //printf("test");
        sleep_ms(1);
    }
}
