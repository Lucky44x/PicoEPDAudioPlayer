#include "epd2in9.h"

//Partial Waveform -- MAGIC Value Don't touch
UBYTE _WF_PARTIAL_2IN9[159] = {
0x0,0x40,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x80,0x80,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x40,0x40,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x80,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0A,0x0,0x0,0x0,0x0,0x0,0x2,  
0x1,0x0,0x0,0x0,0x0,0x0,0x0,
0x1,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x22,0x22,0x22,0x22,0x22,0x22,0x0,0x0,0x0,
0x22,0x17,0x41,0xB0,0x32,0x36,
};

//Full Waveform ? -- MAGIC Value Don't touch
UBYTE WS_20_30[159] =
{											
0x80,	0x66,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x40,	0x0,	0x0,	0x0,
0x10,	0x66,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x20,	0x0,	0x0,	0x0,
0x80,	0x66,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x40,	0x0,	0x0,	0x0,
0x10,	0x66,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x20,	0x0,	0x0,	0x0,
0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,
0x14,	0x8,	0x0,	0x0,	0x0,	0x0,	0x1,					
0xA,	0xA,	0x0,	0xA,	0xA,	0x0,	0x1,					
0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,					
0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,					
0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,					
0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,					
0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,					
0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,					
0x14,	0x8,	0x0,	0x1,	0x0,	0x0,	0x1,					
0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x1,					
0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,					
0x0,	0x0,	0x0,	0x0,	0x0,	0x0,	0x0,					
0x44,	0x44,	0x44,	0x44,	0x44,	0x44,	0x0,	0x0,	0x0,			
0x22,	0x17,	0x41,	0x0,	0x32,	0x36
};	

//Gray Waveform ??? -- MAGIC Value Don't touch (Why tf are we using unsigned chars again??)
UBYTE Gray4[159] =			
{											
0x00,	0x60,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	//VS L0	 //2.28s			
0x20,	0x60,	0x10,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	//VS L1				
0x28,	0x60,	0x14,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	//VS L2				
0x2A,	0x60,	0x15,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	//VS L3 				
0x00,	0x90,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	//VS L4 				
0x00,	0x02,	0x00,	0x05,	0x14,	0x00,	0x00,						//TP, SR, RP of Group0				
0x1E,	0x1E,	0x00,	0x00,	0x00,	0x00,	0x01,						//TP, SR, RP of Group1				
0x00,	0x02,	0x00,	0x05,	0x14,	0x00,	0x00,						//TP, SR, RP of Group2				
0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,						//TP, SR, RP of Group3				
0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,						//TP, SR, RP of Group4				
0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,						//TP, SR, RP of Group5				
0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,						//TP, SR, RP of Group6				
0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,						//TP, SR, RP of Group7				
0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,						//TP, SR, RP of Group8				
0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,						//TP, SR, RP of Group9				
0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,						//TP, SR, RP of Group10				
0x00,	0x00,	0x00,	0x00,	0x00,	0x00,	0x00,						//TP, SR, RP of Group11				
0x24,	0x22,	0x22,	0x22,	0x23,	0x32,	0x00,	0x00,	0x00,				//FR, XON				
0x22,	0x17,	0x41,	0xAE,	0x32,	0x28,							//EOPT VGH VSH1 VSH2 VSL VCOM				
};

//Public functions
WaveshareEPD::WaveshareEPD(int RST, int DC, int CS, int BUSY, int CLK, int MOSI, spi_inst_t *port) {
    EPD_RST_PIN = RST;
    EPD_DC_PIN = DC;
    EPD_CS_PIN = CS;
    EPD_BUSY_PIN = BUSY;
    EPD_CLK_PIN = CLK;
    EPD_MOSI_PIN = MOSI;
    EPD_PORT_SPI = port;
}

/******************************************************************************
function :	Initialize B/W Waveform
parameter:
******************************************************************************/
void WaveshareEPD::EPD_INIT() {
    EPD_RESET();
    sleep_ms(100);

    EPD_READ_BUSY();
    EPD_SEND_COMMAND(0x12); //Soft Reset
    EPD_READ_BUSY();

    EPD_SEND_COMMAND(0x01); //Driver output control
    EPD_SEND_DATA(0x27);
    EPD_SEND_DATA(0x01);
    EPD_SEND_DATA(0x00);

    EPD_SEND_COMMAND(0x11); //Data Entry mode
    EPD_SEND_DATA(0x03);

    EPD_SET_PARTIAL(0, 0, EPD_WIDTH-1, EPD_HEIGHT-1);

    EPD_SEND_COMMAND(0x21); //Display Update Control
    EPD_SEND_DATA(0x00);
    EPD_SEND_DATA(0x80);

    EPD_SET_CURSOR(0, 0);
    EPD_READ_BUSY();

    EPD_LUT_BY_HOST(WS_20_30);
}

/******************************************************************************
function :	Initialize Gray-Waveform
parameter:
******************************************************************************/
void WaveshareEPD::EPD_GRAY_INIT() {
    EPD_RESET();
    sleep_ms(100);

    EPD_READ_BUSY();
    EPD_SEND_COMMAND(0x12); //Soft Reset
    EPD_READ_BUSY();

    EPD_SEND_COMMAND(0x01); //Driver output control
    EPD_SEND_DATA(0x27);
    EPD_SEND_DATA(0x01);
    EPD_SEND_DATA(0x00);

    EPD_SEND_COMMAND(0x11); //Data Entry mode
    EPD_SEND_DATA(0x03);

    EPD_SET_PARTIAL(8, 0, EPD_WIDTH, EPD_HEIGHT-1);

    EPD_SEND_COMMAND(0x3C);
    EPD_SEND_DATA(0x04);

    EPD_SET_CURSOR(1, 0);
    EPD_READ_BUSY();

    EPD_LUT_BY_HOST(Gray4);
}

/******************************************************************************
function :	Software reset
parameter:
******************************************************************************/
void WaveshareEPD::EPD_RESET() {
    EPD_DIGITAL_WRITE(EPD_RST_PIN, 1);
    sleep_ms(10);
    EPD_DIGITAL_WRITE(EPD_RST_PIN, 0);
    sleep_ms(2);
    EPD_DIGITAL_WRITE(EPD_RST_PIN, 1);
    sleep_ms(10);
}

void WaveshareEPD::EPD_CLEAR() {
    UWORD i;

    EPD_SEND_COMMAND(0x24); //Write RAM for b/w (0,1)
    for( i = 0; i < 4736; i++ ) {
        EPD_SEND_DATA(0xFF);
    }

    EPD_SEND_COMMAND(0x26); //Write RAM for b/w (0,1)
    for( i = 0; i < 4736; i++ ) {
        EPD_SEND_DATA(0xFF);
    }
    EPD_REFRESH_FULL();
}

void WaveshareEPD::EPD_DISPLAY(UBYTE *Image) {
	UWORD i;	
	EPD_SEND_COMMAND(0x24);   //write RAM for black(0)/white (1)
	for ( i=0; i<4736; i++ )
	{
		EPD_SEND_DATA(Image[i]);
	}
	EPD_REFRESH_FULL();	
}

void WaveshareEPD::EPD_DISPLAY_BASE(UBYTE *Image) {
    UWORD i;

	EPD_SEND_COMMAND(0x24);   //Write Black and White image to RAM
	for ( i=0; i<4736; i++ )
	{               
		EPD_SEND_DATA(Image[i]);
	}

	EPD_SEND_COMMAND(0x26);   //Write Black and White image to RAM
	for ( i=0; i<4736; i++ )
	{               
		EPD_SEND_DATA(Image[i]);
	}
	EPD_REFRESH_FULL();
}

/**
 * Absolute Definition of MAGIC FUNCTION
 * Don't touch, think about or even look at this function... I beg you
 **/
void WaveshareEPD::EPD_DISPLAY_GRAY(UBYTE *Image) {
    UDOUBLE i,j,k;
    UBYTE temp1,temp2,temp3;

    // old  data
    EPD_SEND_COMMAND(0x24);
    for(i=0; i<4736; i++) { 
        temp3=0;
        for(j=0; j<2; j++) {
            temp1 = Image[i*2+j];
            for(k=0; k<2; k++) {
                temp2 = temp1&0xC0;
                if(temp2 == 0xC0)
                    temp3 |= 0x00;
                else if(temp2 == 0x00)
                    temp3 |= 0x01; 
                else if(temp2 == 0x80)
                    temp3 |= 0x01; 
                else //0x40
                    temp3 |= 0x00; 
                temp3 <<= 1;

                temp1 <<= 2;
                temp2 = temp1&0xC0 ;
                if(temp2 == 0xC0) 
                    temp3 |= 0x00;
                else if(temp2 == 0x00) 
                    temp3 |= 0x01;
                else if(temp2 == 0x80)
                    temp3 |= 0x01; 
                else    //0x40
                    temp3 |= 0x00;	
                if(j!=1 || k!=1)
                    temp3 <<= 1;

                temp1 <<= 2;
            }
        }
        EPD_SEND_DATA(temp3);
        // printf("%x ",temp3);
    }

    EPD_SEND_COMMAND(0x26);   //write RAM for black(0)/white (1)
    for(i=0; i<4736; i++) {            
        temp3=0;
        for(j=0; j<2; j++) {
            temp1 = Image[i*2+j];
            for(k=0; k<2; k++) {
                temp2 = temp1&0xC0 ;
                if(temp2 == 0xC0)
                    temp3 |= 0x00;//white
                else if(temp2 == 0x00)
                    temp3 |= 0x01;  //black
                else if(temp2 == 0x80)
                    temp3 |= 0x00;  //gray1
                else //0x40
                    temp3 |= 0x01; //gray2
                temp3 <<= 1;

                temp1 <<= 2;
                temp2 = temp1&0xC0 ;
                if(temp2 == 0xC0)  //white
                    temp3 |= 0x00;
                else if(temp2 == 0x00) //black
                    temp3 |= 0x01;
                else if(temp2 == 0x80)
                    temp3 |= 0x00; //gray1
                else    //0x40
                    temp3 |= 0x01;	//gray2
                if(j!=1 || k!=1)
                    temp3 <<= 1;

                temp1 <<= 2;
            }
        }
        EPD_SEND_DATA(temp3);
        // printf("%x ",temp3);
    }

    EPD_REFRESH_FULL();
}

void WaveshareEPD::EPD_DISPLAY_PARTIAL(UBYTE *Image) {
    UWORD i;

    //Reset
    EPD_DIGITAL_WRITE(EPD_RST_PIN, 0);
    sleep_ms(2);
    EPD_DIGITAL_WRITE(EPD_RST_PIN, 1);
    sleep_ms(2);

    EPD_LUT(_WF_PARTIAL_2IN9);
    EPD_SEND_COMMAND(0x37);
    EPD_SEND_DATA(0x00);
    EPD_SEND_DATA(0x00);
    EPD_SEND_DATA(0x00);
    EPD_SEND_DATA(0x00);
    EPD_SEND_DATA(0x00);
    EPD_SEND_DATA(0x40);
    EPD_SEND_DATA(0x00);
    EPD_SEND_DATA(0x00);
    EPD_SEND_DATA(0x00);
    EPD_SEND_DATA(0x00);

    EPD_SEND_COMMAND(0x3C); //Border Waveform
    EPD_SEND_DATA(0x80);

    EPD_SEND_COMMAND(0x22);
    EPD_SEND_DATA(0xC0);
    EPD_SEND_COMMAND(0x20);
    EPD_READ_BUSY();

    EPD_SET_PARTIAL(0, 0, EPD_WIDTH-1, EPD_HEIGHT-1);
    EPD_SET_CURSOR(0, 0);

    EPD_SEND_COMMAND(0x24); //Write Black and white Image to RAM
    for(i - 0; i < 4736; i ++) {
        EPD_SEND_DATA(Image[i]);
    }
    EPD_REFRESH_PARTIAL();
}

void WaveshareEPD::EPD_SLEEP() {
    EPD_SEND_COMMAND(0x10); //Enter Deep Sleep
    EPD_SEND_DATA(0x01);
    sleep_ms(100);
}

/******************************************************************************
function :	Refresh Display Full
parameter:
******************************************************************************/
void WaveshareEPD::EPD_REFRESH_FULL() {
    EPD_SEND_COMMAND(0x22); //Display Update Control
	EPD_SEND_DATA(0xc7);
	EPD_SEND_COMMAND(0x20); //Activate Display Update Sequence
	EPD_READ_BUSY();
}

/******************************************************************************
function :	Refresh Display Partial
parameter:
******************************************************************************/
void WaveshareEPD::EPD_REFRESH_PARTIAL() {
    EPD_SEND_COMMAND(0x22); //Display Update Control
	EPD_SEND_DATA(0x0F);
	EPD_SEND_COMMAND(0x20); //Activate Display Update Sequence
	EPD_READ_BUSY();
}

/******************************************************************************
function :	Setting the display window
parameter:
******************************************************************************/
void WaveshareEPD::EPD_SET_PARTIAL(UWORD XStart, UWORD YStart, UWORD XEnd, UWORD YEnd) {
    EPD_SEND_COMMAND(0x44); //Set Ram X Address
    EPD_SEND_DATA((XStart>>3) & 0xFF);
    EPD_SEND_DATA((XEnd>>3) & 0xFF);

    EPD_SEND_COMMAND(0x45); ///Set Ram y Address
    EPD_SEND_DATA(YStart & 0xFF);
    EPD_SEND_DATA((YStart >> 8) & 0xFF);
    EPD_SEND_DATA(YEnd & 0xFF);
    EPD_SEND_DATA((YEnd >> 8) & 0xFF);
}

/******************************************************************************
function :	Set Cursor
parameter:
******************************************************************************/
void WaveshareEPD::EPD_SET_CURSOR(UWORD XStart, UWORD YStart) {
    EPD_SEND_COMMAND(0x4E); //Set Ram X counter
    EPD_SEND_DATA(XStart & 0xFF);

    EPD_SEND_COMMAND(0x4F); //Set Ram Y counter
    EPD_SEND_DATA(YStart & 0xFF);
    EPD_SEND_DATA((YStart >> 8) & 0xFF);
}

// Something Inbetween

/******************************************************************************
function :	send command
parameter:
     Reg : Command register
******************************************************************************/
void WaveshareEPD::EPD_SEND_COMMAND(UBYTE Reg) {
    EPD_DIGITAL_WRITE(EPD_DC_PIN, 0);
    EPD_DIGITAL_WRITE(EPD_CS_PIN, 0);
    EPD_SPI_WRITE(Reg);
    EPD_DIGITAL_WRITE(EPD_CS_PIN, 1);
}

/******************************************************************************
function :	send data
parameter:
    Data : Write data
******************************************************************************/
void WaveshareEPD::EPD_SEND_DATA(UBYTE Data) {
    EPD_DIGITAL_WRITE(EPD_DC_PIN, 1);
    EPD_DIGITAL_WRITE(EPD_CS_PIN, 0);
    EPD_SPI_WRITE(Data);
    EPD_DIGITAL_WRITE(EPD_CS_PIN, 1);
}

/******************************************************************************
function :	Wait until the busy_pin goes LOW
parameter:
******************************************************************************/
void WaveshareEPD::EPD_READ_BUSY() {
    printf("e-Paper busy\r\n");
    while(1) {
        //=1 Busy
        if( EPD_DIGITAL_READ(EPD_BUSY_PIN) == 0 )
            break;
        sleep_ms(10);
        //printf("e-Paper no longer busy...\r\n");
    }
}

void WaveshareEPD::EPD_LUT(UBYTE *lut) {
    UBYTE count;
    EPD_SEND_COMMAND(0x32);
    for ( count=0; count<153; count++ ) {
        EPD_SEND_DATA(lut[count]);
    }
    EPD_READ_BUSY();
}

//Magic function dont touch
void WaveshareEPD::EPD_LUT_BY_HOST(UBYTE *lut) {
	EPD_LUT((UBYTE *)lut);			//lut
	EPD_SEND_COMMAND(0x3f);
	EPD_SEND_DATA(*(lut+153));
	EPD_SEND_COMMAND(0x03);	// gate voltage
	EPD_SEND_DATA(*(lut+154));
	EPD_SEND_COMMAND(0x04);	// source voltage
	EPD_SEND_DATA(*(lut+155));	// VSH
	EPD_SEND_DATA(*(lut+156));	// VSH2
	EPD_SEND_DATA(*(lut+157));	// VSL
	EPD_SEND_COMMAND(0x2c);		// VCOM
	EPD_SEND_DATA(*(lut+158));
}

//Private functions
void WaveshareEPD::EPD_DIGITAL_WRITE(UWORD Pin, UBYTE Value) {
    gpio_put(Pin, Value);
}

UBYTE WaveshareEPD::EPD_DIGITAL_READ(UWORD Pin) {
    return gpio_get(Pin);
}

void WaveshareEPD::EPD_SPI_WRITE(UBYTE Value) {
    spi_write_blocking(EPD_PORT_SPI, &Value, 1);
}

void WaveshareEPD::EPD_SPI_WRITE(uint8_t *data, uint32_t len) {
    spi_write_blocking(EPD_PORT_SPI, data, len);
}

/******************************************************************************
function :	Enable Driver
returns:    UBYTE - Error Code (0 - nominal)
parameter:
******************************************************************************/
UBYTE WaveshareEPD::EPD_DRIVER_INIT() {
    //GPIO Config
    EPD_GPIO_INIT();
    EPD_SPI_INIT();

    printf("EPD_MODULE_INIT OK \r\n");
    return 0;
}

void WaveshareEPD::EPD_DRIVER_EXIT() {
    //NOOP
}

void WaveshareEPD::EPD_GPIO_INIT() {
    EPD_GPIO_MODE(EPD_RST_PIN, 1);
    EPD_GPIO_MODE(EPD_DC_PIN, 1);
    EPD_GPIO_MODE(EPD_CS_PIN, 1);
    EPD_GPIO_MODE(EPD_BUSY_PIN, 0);
}

void WaveshareEPD::EPD_GPIO_MODE(UWORD Pin, UWORD Mode) {
    gpio_init(Pin);
    gpio_set_dir(Pin, Mode == 0 ? GPIO_IN : GPIO_OUT);
}

void WaveshareEPD::EPD_SPI_INIT() {
    spi_init(EPD_PORT_SPI, 4000 * 1000);
    gpio_set_function(EPD_CLK_PIN, GPIO_FUNC_SPI);
    gpio_set_function(EPD_MOSI_PIN, GPIO_FUNC_SPI);
}

void WaveshareEPD::EPD_SPI_SEND_DATA(UBYTE Reg) {
    UBYTE i,j=Reg;
    EPD_GPIO_MODE(EPD_MOSI_PIN, 1);
    EPD_GPIO_MODE(EPD_CLK_PIN, 1);
    EPD_DIGITAL_WRITE(EPD_CS_PIN, 0);
    for ( i = 0; i < 8; i++ ) {
        EPD_DIGITAL_WRITE(EPD_CLK_PIN, 0);
        if (j & 0x80) {
            EPD_DIGITAL_WRITE(EPD_MOSI_PIN, 1);
        } else {
            EPD_DIGITAL_WRITE(EPD_MOSI_PIN, 0);
        }

        EPD_DIGITAL_WRITE(EPD_CLK_PIN, 1);
        j = j << 1;
    }
    EPD_DIGITAL_WRITE(EPD_CLK_PIN, 0);
    EPD_DIGITAL_WRITE(EPD_CS_PIN, 1);
}

UBYTE WaveshareEPD::EPD_SPI_READ_DATA() {
    UBYTE i,j=0xff;
    EPD_GPIO_MODE(EPD_MOSI_PIN, 0);
    EPD_GPIO_MODE(EPD_CLK_PIN, 1);
    EPD_DIGITAL_WRITE(EPD_CS_PIN, 0);
    for ( i = 0; i < 8; i++ ) {
        EPD_DIGITAL_WRITE(EPD_CLK_PIN, 0);
        j = j << 1;
        if ( EPD_DIGITAL_READ(EPD_MOSI_PIN) ) {
            j = j | 0x01;
        } else {
            j = j & 0xfe;
        }
        EPD_DIGITAL_WRITE(EPD_CLK_PIN, 1);
    }
    EPD_DIGITAL_WRITE(EPD_CLK_PIN, 0);
    EPD_DIGITAL_WRITE(EPD_CS_PIN, 1);
    return i;
}