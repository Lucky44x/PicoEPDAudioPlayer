#ifndef EPD_2IN9_H
#define EPD_2IN9_H

#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "stdio.h"

#define EPD_WIDTH   128
#define EPD_HEIGHT  296
#define EPD_JOBS_MAX 256

//-- ASYNC-ENGINE
typedef enum {
    EPD_AS_IDLE = 0,
    EPD_AS_SENDING_CMD,
    EPD_AS_SENDING_DATA,
    EPD_AS_BUSY
} epd_async_state_t;

typedef enum {
    PACKET_CMD,
    PACKET_DATA,
    PACKET_WAIT_BUSY,
    PACKET_WAIT,
    PACKET_RESET
} epd_packet_type_t;

typedef struct {
    epd_packet_type_t type;
    uint8_t cmd;
    uint8_t *bytes;
    uint32_t len;           // WAIT: delay_us       DATA: Bytes total
    uint32_t pos;           // WAIT: started flag   DATA: Bytes sent
    uint8_t small[16];
    bool use_small;
} epd_packet_t;

typedef struct {
    int pin_rst;
    int pin_dc;
    int pin_cs;
    int pin_busy;
    int pin_clk;
    int pin_mosi;
    spi_inst_t *epd_port_spi;

    epd_packet_t q_packets[EPD_JOBS_MAX];
    uint8_t queue_head;
    uint8_t queue_tail;
    epd_async_state_t state;

} epd_config_t;

static epd_config_t epd_spi0_default_config = {
    20, 16, 17, 21, 18, 19, (spi_inst_t*)spi0, {}, 0, 0, EPD_AS_IDLE
};

epd_config_t epd_build(int rst, int dc, int cs, int busy, int clk, int mosi, spi_inst_t *port);
void epd_init(epd_config_t *cfg);
void epd_gray_init(epd_config_t *cfg);
void epd_clear(epd_config_t *cfg);
void epd_display(epd_config_t *cfg, uint8_t *image_data);
void epd_display_base(epd_config_t *cfg, uint8_t *image_data);
void epd_display_gray(epd_config_t *cfg, uint8_t *image_data);
void epd_display_partial(epd_config_t *cfg, uint8_t *image_data);
void epd_sleep(epd_config_t *cfg);
void epd_reset(epd_config_t *cfg);

uint8_t epd_driver_init(epd_config_t *cfg);
void epd_refresh_full(epd_config_t *cfg);
void epd_refresh_partial(epd_config_t *cfg);
void epd_set_partial(epd_config_t *cfg, uint16_t xStart, uint16_t yStart, uint16_t xEnd, uint16_t yEnd);
void epd_set_cursor(epd_config_t *cfg, uint16_t xStart, uint16_t yStart);

void epd_service_async(epd_config_t *cfg, uint32_t time_budget_us);

#endif //EPD_2IN9_H