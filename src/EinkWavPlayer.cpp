#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"

#include "input.h"
#include "files.h"
#include "AudioCore.h"
#include "UIMenus.h"

#include <malloc.h>

audio_player_handle_t g_player;
FileManager fileManager;
AudioCore g_core(&g_player, &fileManager);
UIManager uiManager;
InputManager inputManager;

//Menus
PlaybackMenu playbackMenu(&uiManager, &fileManager, &g_core, &inputManager);
ErrorMenu errorMenu(&uiManager, &fileManager);
MainMenu mainMenuUI(&uiManager, &fileManager);
SongMenu songMenuUI(&uiManager, &fileManager, &mainMenuUI, &playbackMenu, &errorMenu);

uint32_t getTotalHeap(void) {
   extern char __StackLimit, __bss_end__;
   
   return &__StackLimit  - &__bss_end__;
}

uint32_t getFreeHeap(void) {
   struct mallinfo m = mallinfo();

   return getTotalHeap() - m.uordblks;
}

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
        printf("AudioPlayer init failed\n");
        multicore_fifo_push_blocking(0xBAD); // signal failure
        while (true) sleep_ms(100);
    }

    // signal core0 that AudioPlayer is ready
    multicore_fifo_push_blocking(0xA11D0);   // any magic value you like
    while(true) { sleep_ms(100); }
}

int main()
{
    stdio_init_all();

    sleep_ms(10000);

    //Launch EPD
    printf("Launching EPD-Driver\n");
    uiManager.init();
    mainMenuUI.setup(&songMenuUI);
    playbackMenu.setup(&songMenuUI);
    sleep_ms(100);

    printf("Launching Input-Manager\n");
    inputManager.init(2000);
    bool input_ok = true;

    if (!inputManager.register_button_pin(26, BUTTON_PREV)) input_ok = false;
    if (!inputManager.register_button_pin(9, BUTTON_PLAY)) input_ok = false;
    if (!inputManager.register_button_pin(22, BUTTON_NEXT)) input_ok = false;
    if (!inputManager.register_button_pin(5,BUTTON_UP)) input_ok = false;
    if (!inputManager.register_button_pin(15, BUTTON_SELECT)) input_ok = false;
    if (!inputManager.register_button_pin(14, BUTTON_DOWN)) input_ok = false;

    if (!input_ok) {
        printf("Failed to initialize Input-Manager\n");
        errorMenu.set_message_utf8("Input init failed");
        uiManager.switch_menu(&errorMenu);
        return 1;
    }

    printf("Launching File-System..\n");
    FRESULT fileManager_ok = fileManager.init();

    if (fileManager_ok != FR_OK) {
        printf("Failed to initialize File-Manager %u", fileManager_ok);
        errorMenu.set_message_utf8("No SD-Card found");
        uiManager.switch_menu(&errorMenu);
        return 1;
    }

    // Launch DAC-Thread
    printf("Launching Audio on Core-1\n");
    multicore_launch_core1(core1_entry);
    
    uint32_t token = multicore_fifo_pop_blocking();
    if(token != 0xA11D0) {
        printf("Core1 init failed\n");
        errorMenu.set_message_utf8("Core-1 failed...");
        uiManager.switch_menu(&errorMenu);
        return 1;
    }
    printf("Core1 ready... Launching Audio-Core\n");

    if (!g_core.open()) {
        printf("Audio core could not start\n");
        errorMenu.set_message_utf8("Audio failed...");
        uiManager.switch_menu(&errorMenu);
        return 1;
    }
    g_core.mute(true);

    uiManager.switch_menu(&mainMenuUI);

    while (true) {

        //printf("Memory info:\n");
        //printf("%u / %u\n", getTotalHeap(), getFreeHeap());

        // Input stuff
        InputEvent ev;
        if (inputManager.poll_event(ev)) {
            uiManager.input(ev);
            //printf("Input Event: %u, %u\n", ev.code, ev.type);
        }

        // Ui Updates
        uiManager.update();

        // Audio pump
        g_core.pump();
    }
}
