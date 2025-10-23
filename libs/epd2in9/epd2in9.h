#ifndef EPD_2IN9_H
#define EPD_2IN9_H

#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "stdio.h"

#define EPD_WIDTH   128
#define EPD_HEIGHT  296

/*
    Data Types
*/
#define UBYTE   uint8_t
#define UWORD   uint16_t
#define UDOUBLE uint32_t

class EPD2IN9 {
public:
    EPD2IN9(int RST = 20, int DC = 16, int CS = 17, int BUSY = 21, int CLK = 18, int MOSI = 19, spi_inst_t *port = (spi_inst_t*)spi0);
    //Define EPD-Specific Functions
    void EPD_INIT();
    void EPD_GRAY_INIT();
    void EPD_CLEAR();
    void EPD_DISPLAY(UBYTE *Image);
    void EPD_DISPLAY_BASE(UBYTE *Image);
    void EPD_DISPLAY_GRAY(UBYTE *Image);
    void EPD_DISPLAY_PARTIAL(UBYTE *Image);
    void EPD_SLEEP();
    void EPD_RESET();

    //EPD Specific Driver functions
    UBYTE EPD_DRIVER_INIT();
    void EPD_REFRESH_FULL();
    void EPD_REFRESH_PARTIAL();
    void EPD_SET_PARTIAL(UWORD XStart, UWORD YStart, UWORD XEnd, UWORD YEnd);
    void EPD_SET_CURSOR(UWORD XStart, UWORD YStart);

private:
    //EPD Specific Driver functions
    void EPD_SEND_COMMAND(UBYTE Reg);
    void EPD_SEND_DATA(UBYTE Data);
    void EPD_READ_BUSY();
    void EPD_LUT(UBYTE *lut);
    void EPD_LUT_BY_HOST(UBYTE *lut);

    //Define Driver Specific stuff
    void EPD_DIGITAL_WRITE(UWORD Pin, UBYTE Value);
    UBYTE EPD_DIGITAL_READ(UWORD Pin);

    void EPD_SPI_WRITE(UBYTE Value);
    void EPD_SPI_WRITE(uint8_t *data, uint32_t len);

    void EPD_DRIVER_EXIT();
    void EPD_GPIO_INIT();
    void EPD_SPI_INIT();
    void EPD_SPI_SEND_DATA(UBYTE Reg);
    UBYTE EPD_SPI_READ_DATA();

    void EPD_GPIO_MODE(UWORD Pin, UWORD Mode);

    int EPD_RST_PIN;
    int EPD_DC_PIN;
    int EPD_CS_PIN;
    int EPD_BUSY_PIN;
    int EPD_CLK_PIN;
    int EPD_MOSI_PIN;
    spi_inst_t *EPD_PORT_SPI;
};

#endif //EPD_2IN9_H