#include "epd2in9-impl.h"

#define EPD_WIDTH   128
#define EPD_HEIGHT  296
#define EPD_PIX (EPD_WIDTH * EPD_HEIGHT)
#define EPD_2B (EPD_PIX / 4u)
#define EPD_1B (EPD_PIX / 8u)

/* Convert 2bpp -> 1bpp for one plane into a staging buffer.
   - src2bpp points to the whole 2bpp image (9472 B), row-major, MSB-first per byte.
   - out_bytes is how many output bytes to produce this call (<= EPD_1B).
   - out_off is the output-byte offset (0..EPD_1B-1). We read 2*out_bytes from src starting at 2*out_off.
   - plane_bit: 0 = use LSB (for cmd 0x24), 1 = use MSB (for cmd 0x26).
   - We invert the chosen bit to match your “==0 → 1” mapping from the original code. */
static void pack_2bpp_plane(const uint8_t *src2bpp, uint32_t out_off,
                            uint8_t *dst1bpp, uint32_t out_bytes, int plane_bit)
{
    const uint32_t in_off = out_off * 2u;
    const uint8_t *in = src2bpp + in_off;

    for (uint32_t i = 0; i < out_bytes; ++i) {
        uint8_t b0 = in[0], b1 = in[1];
        uint8_t out = 0;

        for (int pix = 0; pix < 8; ++pix) {
            uint8_t byte  = (pix < 4) ? b0 : b1;
            int      shift = 6 - 2 * (pix & 3);
            uint8_t  two   = (byte >> shift) & 0x03;

            uint8_t bit;
            switch (two) {
            case 0x3: bit = (plane_bit == 0) ? 1 : 0; break; // 11 → white
            case 0x0: bit = (plane_bit == 0) ? 0 : 1; break; // 00 → black
            case 0x2: bit = (plane_bit == 0) ? 1 : 1; break; // 10 → light-gray
            case 0x1: bit = (plane_bit == 0) ? 0 : 0; break; // 01 → dark-gray
            }
            out = (uint8_t)((out << 1) | bit);
        }

        dst1bpp[i] = out;
        in += 2;
    }
}

/******************************************************************************
function :	Build config for EPD-Driver
parameter:  rst-pin, 
            dc-pin, 
            cs-pin, 
            busy-pin, 
            clk-pin, 
            mosi-pin, 
            spi_instance*
******************************************************************************/
epd_config_t epd_build(int rst, int dc, int cs, int busy, int clk, int mosi, spi_inst_t *port) {
    epd_config_t cfg = {
        rst, dc, cs, busy, clk, mosi, port
    };
    return cfg;
}

/******************************************************************************
function :	Initialize B/W Waveform
parameter:  cfg
******************************************************************************/
void epd_init(epd_config_t *cfg) {
    epd_reset(cfg);
    sleep_ms(100);

    epd_queue_busy(cfg);
    epd_send_command(cfg, 0x12); // Soft-Reset
    epd_queue_busy(cfg);

    epd_send_command(cfg, 0x01); // Driver output control
    epd_send_data(cfg, 0x27);
    epd_send_data(cfg, 0x01);
    epd_send_data(cfg, 0x00);
    
    epd_send_command(cfg, 0x11); // Data entry
    epd_send_data(cfg, 0x03);

    epd_set_partial(cfg, 0, 0, EPD_WIDTH-1, EPD_HEIGHT-1);
    
    epd_send_command(cfg, 0x21); // Display update control
    epd_send_data(cfg, 0x00);
    epd_send_data(cfg, 0x80);

    epd_set_cursor(cfg, 0,0);
    epd_queue_busy(cfg);

    epd_lut_by_host(cfg, WS_20_30);
}

/******************************************************************************
function :	Initialize Gray Waveform
parameter:  cfg
******************************************************************************/
void epd_init_gray(epd_config_t *cfg) {
    epd_reset(cfg);
    epd_packet_t slp_pkt = {.type=PACKET_WAIT,.len=100};
    epd_queue_push(cfg, &slp_pkt);
    epd_flush_until_idle(cfg, 100000); //100ms timeout

    epd_queue_busy(cfg);
    epd_send_command(cfg, 0x12); // Soft-Reset
    epd_queue_busy(cfg);

    epd_send_command(cfg, 0x01); // Driver output control
    epd_send_data(cfg, 0x27);
    epd_send_data(cfg, 0x01);
    epd_send_data(cfg, 0x00);
    
    epd_send_command(cfg, 0x11); // Data entry
    epd_send_data(cfg, 0x03);

    epd_set_partial(cfg, 8, 0, EPD_WIDTH, EPD_HEIGHT-1);
    
    epd_send_command(cfg, 0x3C); // Display update control
    epd_send_data(cfg, 0x04);

    epd_set_cursor(cfg, 1,0);
    epd_queue_busy(cfg);

    epd_lut_by_host(cfg, Gray4);
}

/******************************************************************************
function :	Software reset
parameter:  cfg
******************************************************************************/
void epd_reset(epd_config_t *cfg) {
    epd_packet_t pkt = {.type=PACKET_RESET};
    epd_queue_push(cfg, &pkt);
}

/******************************************************************************
function :	Clear EPD-Buffer
parameter:  cfg
******************************************************************************/
void epd_clear(epd_config_t *cfg) {
    static const uint8_t fill_ff[256]={
        [0 ... 255]=0xFF
    };

    epd_send_command(cfg, 0x24); //Write RAM for b/w (0,1)
    for( uint16_t offset = 0; offset < 4736; offset += sizeof(fill_ff) ) {
        uint32_t n = (4736 - offset) > sizeof(fill_ff) ? sizeof(fill_ff) : (4736 - offset);
        epd_send_data_len(cfg, fill_ff, n);
    }

    epd_send_command(cfg, 0x26); //Write RAM for b/w (0,1)
    for (uint16_t offset = 0; offset < 4736; offset += sizeof(fill_ff)) {
        uint32_t n = (4736 - offset) > sizeof(fill_ff) ? sizeof(fill_ff) : (4736 - offset);
        epd_send_data_len(cfg, fill_ff, n);
    }
    epd_refresh_full(cfg);
}

/******************************************************************************
function :	Write the image-data to the EPD
parameter:  the pointer to the image data
******************************************************************************/
void epd_display(epd_config_t *cfg, uint8_t *image) {
	uint16_t i;	
	epd_send_command(cfg, 0x24);   //write RAM for black(0)/white (1)
    epd_send_data_len(cfg, image, 4736);
	epd_refresh_full(cfg);
}

/******************************************************************************
function :	Write the image-data to the EPD
parameter:  the pointer to the image data
******************************************************************************/
void epd_display_base(epd_config_t *cfg, uint8_t *image) {
	epd_send_command(cfg, 0x24);   //Write Black and White image to RAM
    epd_send_data_len(cfg, image, 4736);
	epd_send_command(cfg, 0x26);   //Write Black and White image to RAM
    epd_send_data_len(cfg, image, 4736);
	epd_refresh_full(cfg);
}

/******************************************************************************
function :	Write the image-data to the EPD in grayscale
            Absolute Definition of MAGIC FUNCTION
            Don't touch, think about or even look at this function... I beg you
parameter:  the pointer to the image data
******************************************************************************/
void epd_display_gray(epd_config_t *cfg, uint8_t *image) {
    // Tune chunk size: 256 output bytes → 512 input bytes; good balance for queue/service cadence
    enum { OUT_CHUNK = 256 };

    // --- Plane for 0x24: use LSB (plane_bit = 0), inverted as per original
    epd_send_command(cfg, 0x24);
    for (uint32_t off = 0; off < EPD_1B; off += OUT_CHUNK) {
        uint8_t *staging = malloc(OUT_CHUNK);
        uint32_t n = (EPD_1B - off) > OUT_CHUNK ? OUT_CHUNK : (EPD_1B - off);
        pack_2bpp_plane(image, off, staging, n, /*plane_bit=*/0);
        epd_send_data_len_dyn(cfg, staging, n);
    }

    // --- Plane for 0x26: use MSB (plane_bit = 1), inverted as per original
    epd_send_command(cfg, 0x26);
    for (uint32_t off = 0; off < EPD_1B; off += OUT_CHUNK) {
        uint8_t *staging = malloc(OUT_CHUNK);
        uint32_t n = (EPD_1B - off) > OUT_CHUNK ? OUT_CHUNK : (EPD_1B - off);
        pack_2bpp_plane(image, off, staging, n, /*plane_bit=*/1);
        epd_send_data_len_dyn(cfg, staging, n);
    }

    epd_refresh_full(cfg); // enqueues 0x22/0x20 + WAIT_BUSY in your async path
}

/******************************************************************************
function :	Write the image-data to the EPD in partial mode
parameter:  cfg, image-data-pointer
******************************************************************************/
void epd_display_partial(epd_config_t *cfg, uint8_t *image) {
    uint16_t i;

    //Reset
    epd_reset(cfg); //TODO: Make sure this doesnt break
    epd_packet_t slp_pkt = {.type=PACKET_WAIT,.len=2};
    epd_queue_push(cfg, &slp_pkt);

    epd_lut(cfg, _WF_PARTIAL_2IN9);
    epd_send_command(cfg, 0x37);
    epd_send_data(cfg, 0x00);
    epd_send_data(cfg, 0x00);
    epd_send_data(cfg, 0x00);
    epd_send_data(cfg, 0x00);
    epd_send_data(cfg, 0x00);
    epd_send_data(cfg, 0x40);
    epd_send_data(cfg, 0x00);
    epd_send_data(cfg, 0x00);
    epd_send_data(cfg, 0x00);
    epd_send_data(cfg, 0x00);

    epd_send_command(cfg, 0x3C); //Border Waveform
    epd_send_data(cfg, 0x80);

    epd_send_command(cfg, 0x22);
    epd_send_data(cfg, 0xC0);
    epd_send_command(cfg, 0x20);
    epd_queue_busy(cfg);

    epd_set_partial(cfg, 0, 0, EPD_WIDTH-1, EPD_HEIGHT-1);
    epd_set_cursor(cfg, 0, 0);

    epd_send_command(cfg, 0x24); //Write Black and white Image to RAM
    epd_send_data_len(cfg, image, 4736);
    epd_refresh_partial(cfg);
}

/******************************************************************************
function :	Put the display into deep-sleep
parameter:  cfg
******************************************************************************/
void epd_sleep(epd_config_t *cfg) {
    epd_send_command(cfg, 0x10); //Enter Deep Sleep
    epd_send_data(cfg, 0x01);
    epd_packet_t slp_pkt = {.type=PACKET_WAIT,.len=100};
    epd_queue_push(cfg, &slp_pkt);
}

/******************************************************************************
function :	Refresh Display Full
parameter:  cfg
******************************************************************************/
void epd_refresh_full(epd_config_t *cfg) {
    epd_send_command(cfg, 0x22); //Display Update Control
	epd_send_data(cfg, 0xc7);
	epd_send_command(cfg, 0x20); //Activate Display Update Sequence
	epd_queue_busy(cfg);
}

/******************************************************************************
function :	Refresh Display Partial
parameter:  cfg
******************************************************************************/
void epd_refresh_partial(epd_config_t *cfg) {
    epd_send_command(cfg, 0x22); //Display Update Control
	epd_send_data(cfg, 0x0F);
	epd_send_command(cfg, 0x20); //Activate Display Update Sequence
	epd_queue_busy(cfg);
}

/******************************************************************************
function :	Setting the display window
parameter:  cfg,
            xStart Coordinate
            yStart Coordinate
            xEnd Coordinate
            yEnd Coordinate
******************************************************************************/
void epd_set_partial(epd_config_t *cfg, uint16_t xStart, uint16_t yStart, uint16_t xEnd, uint16_t yEnd) {
    epd_send_command(cfg, 0x44); //Set Ram X Address
    epd_send_data(cfg, (xStart>>3) & 0xFF);
    epd_send_data(cfg, (xEnd>>3) & 0xFF);

    epd_send_command(cfg, 0x45); ///Set Ram y Address
    epd_send_data(cfg, yStart & 0xFF);
    epd_send_data(cfg, (yStart >> 8) & 0xFF);
    epd_send_data(cfg, yEnd & 0xFF);
    epd_send_data(cfg, (yEnd >> 8) & 0xFF);
}

/******************************************************************************
function :	set EPD-internal cursor position
parameter:  cfg, xStart, yStart
******************************************************************************/
void epd_set_cursor(epd_config_t *cfg, uint16_t xStart, uint16_t yStart) {
    epd_send_command(cfg, 0x4e); // Set Ram X Counter
    epd_send_data(cfg, (xStart >> 3) & 0xFF);

    epd_send_command(cfg, 0x4F); // Set Ram Y Counter
    epd_send_data(cfg, yStart & 0xFF);
    epd_send_data(cfg, (yStart >> 8) & 0xFF);
}

/******************************************************************************
function :	send command
parameter:  cfg, command-register
******************************************************************************/
void epd_send_command(epd_config_t *cfg, uint8_t cmd) {
    epd_packet_t p = { .type=PACKET_CMD, .cmd=cmd };
    epd_queue_push(cfg, &p);
}

/******************************************************************************
function :	send data
parameter:  cfg, data
******************************************************************************/
void epd_send_data(epd_config_t *cfg, uint8_t data) {
    epd_packet_t p = {.type=PACKET_DATA, .bytes=&data, .len=1, .pos=0};
    epd_queue_push_copy_small(cfg, &p, &data, 1);
}

/******************************************************************************
function :	send data-buffer
parameter:  cfg, data
******************************************************************************/
void epd_send_data_len(epd_config_t *cfg, const uint8_t *data, size_t len) {
    epd_packet_t p = {.type=PACKET_DATA, .bytes=(uint8_t *)data, .len=len, .pos=0};
    epd_queue_push(cfg, &p);
}

/******************************************************************************
function :	send data-buffer dynamiclly (has to be freed after consume)
parameter:  cfg, data
******************************************************************************/
void epd_send_data_len_dyn(epd_config_t *cfg, const uint8_t *data, size_t len) {
    epd_packet_t p = {.type=PACKET_DATA, .bytes=(uint8_t *)data, .len=len, .pos=0, .free_after=true};
    epd_queue_push(cfg, &p);
}

/******************************************************************************
function :	wait until busy-pin goes LOW
parameter:  cfg
******************************************************************************/
void epd_read_busy(epd_config_t *cfg) {
    while(1) {
        if(epd_digital_read(cfg->pin_busy) == 0) break;
        sleep_ms(10);
    }
}

/******************************************************************************
function :	set EPD-LUT
parameter:  cfg, LUT-ptr
******************************************************************************/
void epd_lut(epd_config_t *cfg, uint8_t *lut) {
    //uint8_t count;
    epd_send_command(cfg, 0x32);
    epd_send_data_len(cfg, lut, 153);
    epd_queue_busy(cfg);
}

/******************************************************************************
function :	set EPD-LUT via commands AND set voltages according to LUT
parameter:  cfg, LUT-ptr
******************************************************************************/
void epd_lut_by_host(epd_config_t *cfg, uint8_t *lut) {
    epd_lut(cfg, (uint8_t *)lut);
    epd_send_command(cfg, 0x3f);
    epd_send_data(cfg, *(lut + 153));
    epd_send_command(cfg, 0x03);    // gate voltage
    epd_send_data(cfg, *(lut + 154));
    epd_send_command(cfg, 0x04);    // source voltage
    epd_send_data(cfg, *(lut+155)); // VSH
    epd_send_data(cfg, *(lut+156)); // VSH2
    epd_send_data(cfg, *(lut+157)); // VSL
    epd_send_command(cfg, 0x2c);    // VCOM
    epd_send_data(cfg, *(lut+158));

}

/******************************************************************************
function :	Enable Driver
returns:    uint8_t - Error-Code (0 - nominal)
parameter:  cfg
******************************************************************************/
uint8_t epd_driver_init(epd_config_t *cfg) {
    epd_gpio_init(cfg);
    epd_spi_init(cfg);

    printf("EPD_MODULE_INIT_OK \r\n");
    return 0;
}

/******************************************************************************
function :	Disable Driver
parameter:  cfg
******************************************************************************/
void epd_driver_exit(epd_config_t *cfg) {
    //NOOP
}

/******************************************************************************
function :	setup GPIO-Pins for EPD
parameter:  cfg
******************************************************************************/
void epd_gpio_init(epd_config_t *cfg) {
    epd_gpio_mode(cfg->pin_rst, 1);
    epd_gpio_mode(cfg->pin_dc, 1);
    epd_gpio_mode(cfg->pin_cs, 1);
    epd_gpio_mode(cfg->pin_busy, 0);
}

/******************************************************************************
function :	set mode for GPIO Pin
parameter:  pin, mode
******************************************************************************/
void epd_gpio_mode(uint16_t pin, uint8_t mode) {
    gpio_init(pin);
    gpio_set_dir(pin, mode == 0 ? GPIO_IN : GPIO_OUT);
}

/******************************************************************************
function :	setup SPI for EPD
parameter:  cfg
******************************************************************************/
void epd_spi_init(epd_config_t *cfg) {
    spi_init(cfg->epd_port_spi, 4000 * 1000);
    gpio_set_function(cfg->pin_clk, GPIO_FUNC_SPI);
    gpio_set_function(cfg->pin_mosi, GPIO_FUNC_SPI);
}

/******************************************************************************
function :	send data over SPI
parameter:  cfg, data
******************************************************************************/
void epd_spi_send_data(epd_config_t *cfg, uint8_t data) {
    uint8_t i,j=data;
    epd_gpio_mode(cfg->pin_mosi, 1);
    epd_gpio_mode(cfg->pin_clk, 1);
    epd_digital_write(cfg->pin_cs, 0);

    for( i = 0; i < 8; i++ ) {
        epd_digital_write(cfg->pin_clk, 0);
        if (  j & 0x80 ) {
            epd_digital_write(cfg->pin_mosi, 1);
        } else {
            epd_digital_write(cfg->pin_mosi, 0);
        }

        epd_digital_write(cfg->pin_clk, 1);
        j = j << 1;
    }
    epd_digital_write(cfg->pin_clk, 0);
    epd_digital_write(cfg->pin_cs, 1);
}

/******************************************************************************
function :	read data from SPI
returns  :  data (uint8_t)   
parameter:  cfg
******************************************************************************/
uint8_t epd_spi_read_data(epd_config_t *cfg) {
    uint8_t i,j=0xff;
    epd_gpio_mode(cfg->pin_mosi, 0);
    epd_gpio_mode(cfg->pin_clk, 1);
    epd_digital_write(cfg->pin_cs, 0);
    for ( i = 0; i < 8; i++ ) {
        epd_digital_write(cfg->pin_clk, 0);
        j = j << 1;
        if ( epd_digital_read(cfg->pin_clk) ) {
            j = j | 0x01;
        } else {
            j = j & 0xfe;
        }
        epd_digital_write(cfg->pin_clk, 1);
    }
    epd_digital_write(cfg->pin_clk, 0);
    epd_digital_write(cfg->pin_cs, 1);
}

/******************************************************************************
function :	digital pin-write
parameter:  pin, value
******************************************************************************/
void epd_digital_write(uint16_t pin, uint8_t value) {
    gpio_put(pin, value);
}

/******************************************************************************
function :	digital pin read
parameter:  pin
******************************************************************************/
uint8_t epd_digital_read(uint16_t pin) {
    return gpio_get(pin);
}

/******************************************************************************
function :	spi write
parameter:  cfg, value
******************************************************************************/
void epd_spi_write(epd_config_t *cfg, uint8_t value) {
    spi_write_blocking(cfg->epd_port_spi, &value, 1);
}

/******************************************************************************
function :	buffer spi write
parameter:  cfg, data-ptr, len
******************************************************************************/
void epd_spi_write_len(epd_config_t *cfg, uint8_t *data, uint32_t len) {
    spi_write_blocking(cfg->epd_port_spi, data, len);
}

/******************************************************************************
function :	send partition of the framebuffer to the epd
parameter:  cfg, data-ptr, len
******************************************************************************/
void epd_send_partial(epd_config_t *cfg, const uint8_t *buffer, uint16_t byteWidth, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    uint16_t xs = x0 & ~7u;
    uint16_t xe = x1 | 7u;
    if (xe >= byteWidth * 8) xe = (uint16_t)(byteWidth * 8 - 1);

    uint8_t byte_start_x = (xs >> 3);    //Divide by 8
    uint8_t byte_end_x = (xe >> 3);      //Divide by 8
    uint16_t line_bytes = (uint16_t)(byte_end_x - byte_start_x + 1);    //How many bytes per row

    //printf("Data: start: %u, end: %u, per_line: %u", byte_start_x, byte_end_x, line_bytes);

   //epd_set_partial(cfg, 0, 0, EPD_WIDTH-1, EPD_HEIGHT-1);
    for (uint16_t y = y0; y <= y1; y++) {
        epd_set_cursor(cfg, byte_start_x, y);
        epd_send_command(cfg, 0x24);
        const uint8_t *src = buffer + ((size_t)y * byteWidth + byte_start_x);
        epd_send_data_len(cfg, src, line_bytes);
    }
    epd_set_partial(cfg, x0, y0, x1, y1);
    /*
    for (uint16_t y = y0; y <= y1; y++) {
        epd_set_cursor(cfg, byte_start_x, y);
        epd_send_command(cfg, 0x24);
        for (uint16_t x = byte_start_x; x <= byte_end_x; x++) {
            uint16_t index = x + y * byteWidth;
            epd_send_data(cfg, buffer[index]);
        }
    }
    */

    epd_refresh_partial(cfg);
    //epd_refresh_partial(cfg);
    //epd_refresh_partial(cfg);
}

/******************************************************************************
function :	prepares a partial update on the epd
parameter:  cfg
******************************************************************************/
void epd_prepare_partial(epd_config_t *cfg) {
    // Load the partial waveform
    epd_lut(cfg, _WF_PARTIAL_2IN9);                                // 0x32 + 153 bytes + busy wait  :contentReference[oaicite:2]{index=2}

    // Magic block (panel-internal settings for partial)
    epd_send_command(cfg, 0x37);
    epd_send_data(cfg, 0x00);
    epd_send_data(cfg, 0x00);
    epd_send_data(cfg, 0x00);
    epd_send_data(cfg, 0x00);
    epd_send_data(cfg, 0x00);
    epd_send_data(cfg, 0x40);
    epd_send_data(cfg, 0x00);
    epd_send_data(cfg, 0x00);
    epd_send_data(cfg, 0x00);
    epd_send_data(cfg, 0x00);                                      // same as your partial path  :contentReference[oaicite:3]{index=3}

    // Border waveform
    epd_send_command(cfg, 0x3C);
    epd_send_data(cfg, 0x80);                                      // :contentReference[oaicite:4]{index=4}

    // Precharge/update control (required once after LUT change)
    epd_send_command(cfg, 0x22);
    epd_send_data(cfg, 0xC0);
    epd_send_command(cfg, 0x20);
    epd_queue_busy(cfg);                                            // wait for controller ready  :contentReference[oaicite:5]{index=5}
}

void epd_service_async(epd_config_t *cfg, uint32_t time_budget_us) {
    if (!cfg) return;
    
    const uint32_t CHUNK = 256;
    epd_packet_t cur;
    absolute_time_t t0 = get_absolute_time();

    while ((uint32_t)absolute_time_diff_us(t0, get_absolute_time()) < time_budget_us) {
        if (!epd_queue_peek(cfg, &cur)) return; // No packets left
        printf("Service-tick, sending packet:\n");
        // Switch type:
        switch(cur.type) {
            case PACKET_WAIT:
                printf("    Type Wait: %u\n", cur.len);
                sleep_ms(cur.len);
                epd_queue_pop(cfg, &cur);
                break;
            case PACKET_RESET:
                printf("    Type Reset\n");
                epd_digital_write(cfg->pin_rst, 1);
                sleep_ms(10);
                epd_digital_write(cfg->pin_rst, 0);
                sleep_ms(2);
                epd_digital_write(cfg->pin_rst, 1);
                sleep_ms(10);
                epd_queue_pop(cfg, &cur);
                break;
            case PACKET_WAIT_BUSY:
                printf("    Waiting for busy to go low\n");
                if (epd_busy_low(cfg)) { 
                    epd_queue_pop(cfg, &cur);  // Pop waiting packet off queue to free sending up
                    continue;
                }
                return;                         // Try again next tick
            case PACKET_CMD:
                printf("    Type Cmd 0x%02X\n", cur.cmd);
                epd_digital_write(cfg->pin_dc, 0);
                epd_digital_write(cfg->pin_cs, 0);
                epd_spi_write(cfg, cur.cmd);
                epd_digital_write(cfg->pin_cs, 1);
                epd_queue_pop(cfg, &cur);  // Pop packet
                break;
            case PACKET_DATA:
                uint32_t remain = cur.len - cur.pos;
                printf("    Type Data: \n       Len: %u\n       Pos: %u\n       Remain: %u\n        First byte: 0x%02X\n", cur.len, cur.pos, remain, cur.bytes[0]);
                if (!remain) { 
                    epd_queue_pop(cfg, &cur);
                    if (cur.free_after) free(cur.bytes);
                    continue; 
                }
                uint32_t n = (remain > CHUNK) ? CHUNK : remain;

                epd_digital_write(cfg->pin_dc, 1);
                epd_digital_write(cfg->pin_cs, 0);
                epd_spi_write_len(cfg, cur.bytes + cur.pos, n);
                epd_digital_write(cfg->pin_cs, 1);
                epd_queue_advance_head(cfg, n);
                break;                                              // Otherwise, leave packet on queue and just roll over to next iteration
        }
    }
}