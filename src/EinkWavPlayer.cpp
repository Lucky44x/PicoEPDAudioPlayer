#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"

#include "files.h"
#include "AudioCore.h"

audio_player_handle_t g_player;
AudioCore g_core;
FileManager fileManager;

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
    while(true) { sleep_ms(100); }
}

int main()
{
    int current_song = 0;

    stdio_init_all();
    sleep_ms(5000); //Allow USB serial

    printf("Launching File-System..\n");
    fileManager = FileManager();
    FRESULT fileManager_ok = fileManager.init();

    if (fileManager_ok != FR_OK) {
        printf("Failed to initialize File-Manager %u", fileManager_ok);
        return 1;
    }

    printf("Starting Audio Test\n");

    // Launch DAC-Thread
    multicore_launch_core1(core1_entry);

    uint32_t token = multicore_fifo_pop_blocking();
    if(token != 0xA11D0) {
        printf("Core1 init failed");
        while(true) sleep_ms(100);
    }
    printf("Core1 ready... Launching Audio-Core");
    g_core = AudioCore(&g_player, &fileManager);

    if (!g_core.open()) {
        printf("Audio core could not start");
        while(true) sleep_ms(100);
    }

    if (!g_core.start_song(0)) {
        printf("Could not open song 0 from disk");
        while(true) sleep_ms(100);
    }

    printf("Tone test started..\n");

    while (true) {
        if (g_core.awaitingNext()) { 
            g_core.mute(true);
            if (!g_core.start_song(++current_song)) break;
            g_core.mute(false);
        }

        g_core.pump();
        //printf("test");
        sleep_ms(1);
    }
}
