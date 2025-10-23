#ifndef EPDDRAW_H
#define EPDDRAW_H

#include "epd2in9.h"

#define ROTATE_0   0
#define ROTATE_90   90
#define ROTATE_180  180
#define ROTATE_270  270

typedef enum {
    MIRROR_NONE = 0x00,
    MIRROR_HORIZONTAL = 0x01,
    MIRROR_VERTICAL = 0x02,
    MIRROR_ORIGIN = 0x03,
} MIRROR_IMAGE;
#define MIRROR_IMAGE_DFT MIRROR_NONE

/**
 *  Colors 
**/
//B/W
#define WHITE 0xFF
#define BLACK 0x00

//Grayscale
#define GRAY1 0x03 //Blackest
#define GRAY2 0x02
#define GRAY3 0x01
#define GRAY4 0x00 //Whitest

/**
 * The size of the point
**/
typedef enum {
    DOT_PIXEL_1X1  = 1,		// 1 x 1
    DOT_PIXEL_2X2  , 		// 2 X 2
    DOT_PIXEL_3X3  ,		// 3 X 3
    DOT_PIXEL_4X4  ,		// 4 X 4
    DOT_PIXEL_5X5  , 		// 5 X 5
    DOT_PIXEL_6X6  , 		// 6 X 6
    DOT_PIXEL_7X7  , 		// 7 X 7
    DOT_PIXEL_8X8  , 		// 8 X 8
} DOT_PIXEL;
#define DOT_PIXEL_DFT  DOT_PIXEL_1X1  //Default dot pilex


/**
 * Point size fill style
**/
typedef enum {
    DOT_FILL_AROUND  = 1,		// dot pixel 1 x 1
    DOT_FILL_RIGHTUP  , 		// dot pixel 2 X 2
} DOT_STYLE;
#define DOT_STYLE_DFT  DOT_FILL_AROUND  //Default dot pilex

/**
 * Line style, solid or dashed
**/
typedef enum {
    LINE_STYLE_SOLID = 0,
    LINE_STYLE_DOTTED,
} LINE_STYLE;

/**
 * Whether the graphic is filled
**/
typedef enum {
    DRAW_FILL_EMPTY = 0,
    DRAW_FILL_FULL,
} DRAW_FILL;

class EPRENDERER {
    public:
        EPRENDERER();
        void Init(UWORD grayLevels, UWORD Rotation = ROTATE_90, UWORD Color = GRAY4);
        void Destroy();

        //Properties
        void SetRotation(UWORD Rotation);
        void SetMirroring(UBYTE Mirror);
        void SetScale(UBYTE scale);
        
        //Clearing
        void Clear(UWORD Color);
        void ClearPartial(UWORD Xstart, UWORD Ystart, UWORD Xend, UWORD Yend, UWORD Color);

        //Drawing
        void DrawPoint(UWORD Xpoint, UWORD Ypoint, UWORD Color, DOT_PIXEL PixelStyle = DOT_PIXEL_DFT, DOT_STYLE FillStyle = DOT_STYLE_DFT);
        void DrawLine(UWORD Xstart, UWORD Ystart, UWORD Xend, UWORD Yend, UWORD Color, DOT_PIXEL LineWidth = DOT_PIXEL_DFT, LINE_STYLE LineStyle = LINE_STYLE_SOLID);
        void DrawRect(UWORD Xstart, UWORD Ystart, UWORD Xend, UWORD Yend, UWORD Color, DOT_PIXEL LineWidth = DOT_PIXEL_DFT, DRAW_FILL FillStyle = DRAW_FILL_FULL);
        void DrawCircle(UWORD Xcenter, UWORD Ycenter, UWORD Radius, UWORD Color, DOT_PIXEL LineWidth = DOT_PIXEL_DFT, DRAW_FILL FillStyle = DRAW_FILL_FULL);
        uint8_t DrawChar(UWORD character, UWORD Xpoint, UWORD Ypoint, UWORD Color);
        void DrawText(const UWORD* text, size_t length, UWORD xPoint, UWORD yPoint, UWORD Color, UBYTE spacing = 0, UWORD max_text_area = 0xFFFF);
        uint8_t GetCharWidth(UWORD character);
        void DrawBitmap(const uint8_t* image_buffer, UWORD xPoint, UWORD yPoint, UWORD width, UWORD height);

        //General Commands
        void RefreshScreen();
    private:
        void SetPixel(UWORD Xpoint, UWORD Ypoint, UWORD Color);

        typedef struct {
            UBYTE *fB;
            UWORD Width;
            UWORD Height;
            UWORD WidthMemory;
            UWORD HeightMemory;
            UWORD Color;
            UWORD Rotation;
            UWORD Mirror;
            UWORD WidthByte;
            UWORD HeightByte;
            UWORD Scale;
        } CANVAS;
        
        CANVAS Canvas;
        EPD2IN9 Driver;
};

#endif  //EPDDRAW