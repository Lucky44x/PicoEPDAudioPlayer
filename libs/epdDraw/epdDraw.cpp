#include "epdDraw.h"
#include <stdlib.h>

extern "C" {
    extern const uint8_t _binary_fonts_unifont_bin_start[];
    extern const uint8_t _binary_fonts_unifont_bin_end[];
}

EPDRenderer::EPDRenderer() {}

void EPDRenderer::Init(UWORD grayLevels, UWORD Rotation, UWORD Color) {
    //Setup Driver
    Driver = WaveshareEPD();
    if (Driver.EPD_DRIVER_INIT() != 0) {
        return;
    }

    if (grayLevels == 4) {
        Driver.EPD_GRAY_INIT();
    } else {
        Driver.EPD_INIT();
    }
    Driver.EPD_CLEAR();
    
    Canvas = { NULL };
    
    //Allocate frambuffer memory
    UBYTE *FrameBuffer;
    UWORD bufferLen = (EPD_WIDTH / 8) * EPD_HEIGHT * 2;
    if ( (FrameBuffer = (UBYTE *)malloc(bufferLen)) == NULL ) {
        panic("ERROR: Could not allocate %u Bytes for Canvas framebuffer...\r\n", bufferLen);
        return;
    }

    Canvas.fB = FrameBuffer;
    Canvas.WidthMemory = EPD_WIDTH;
    Canvas.HeightMemory = EPD_HEIGHT;
    Canvas.Color = Color;
    Canvas.Scale = grayLevels;
    //Canvas.WidthByte = (EPD_WIDTH % 8 == 0) ? (EPD_WIDTH / 8) : (EPD_WIDTH /8 + 1);
    SetScale(grayLevels);
    Canvas.HeightByte = EPD_HEIGHT;
    Canvas.Rotation = Rotation;
    Canvas.Mirror = MIRROR_IMAGE_DFT;

    if ( Rotation == ROTATE_0 || Rotation == ROTATE_270 ) {
        Canvas.Width = EPD_HEIGHT;
        Canvas.Height = EPD_WIDTH;
    } else {
        Canvas.Width = EPD_WIDTH;
        Canvas.Height = EPD_HEIGHT;
    }
}

void EPDRenderer::Destroy() {
    free(Canvas.fB);
}

void EPDRenderer::SetRotation(UWORD Rotation) { Canvas.Rotation = Rotation; }
void EPDRenderer::SetMirroring(UBYTE Mirror) { Canvas.Mirror = Mirror; }
void EPDRenderer::SetScale(UBYTE Scale) {
    if ( Scale == 2 ) {
        Canvas.Scale = Scale;
        Canvas.WidthByte = (Canvas.WidthMemory % 8 == 0) ? (Canvas.WidthMemory / 8) : (Canvas.WidthMemory / 8 + 1);
    } else if (Scale == 4) {
        Canvas.Scale = Scale;
        Canvas.WidthByte = (Canvas.WidthMemory % 4 == 0) ? (Canvas.WidthMemory / 4) : (Canvas.WidthMemory / 4 + 1);
    }
}

void EPDRenderer::RefreshScreen() {
    if (Canvas.Scale == 4) {
        Driver.EPD_DISPLAY_GRAY(Canvas.fB);
    } else {
        Driver.EPD_DISPLAY(Canvas.fB);
    }
}

void EPDRenderer::SetPixel(UWORD Xpoint, UWORD Ypoint, UWORD Color) {
    if (Xpoint > Canvas.Width || Ypoint > Canvas.Height) { return; }
    UWORD X, Y;

    switch( Canvas.Rotation ) {
        case 0:
            X = Xpoint;
            Y = Ypoint;
            break;
        case 90:
            X = Canvas.WidthMemory - Ypoint - 1;
            Y = Xpoint;
            break;
        case 180:
            X = Canvas.WidthMemory - Xpoint - 1;
            Y = Canvas.HeightMemory - Ypoint - 1;
            break;
        case 270:
            X = Ypoint;
            Y = Canvas.HeightMemory - Xpoint - 1;
            break;
        default:
            return;
    }

    switch( Canvas.Mirror ) {
        case MIRROR_NONE:
            break;
        case MIRROR_HORIZONTAL:
            X = Canvas.WidthMemory - X - 1;
            break;
        case MIRROR_VERTICAL:
            Y = Canvas.HeightMemory - Y - 1;
            break;
        case MIRROR_ORIGIN:
            X = Canvas.WidthMemory - X - 1;
            Y = Canvas.HeightMemory - Y - 1;
            break;
        default:
            return;
    }

    if ( X > Canvas.WidthMemory || Y > Canvas.HeightMemory) { return; }

    //Set Colors
    if ( Canvas.Scale == 2 ) {                                  //Black And White Only
        UDOUBLE Addr = X / 8 + Y * Canvas.WidthByte;
        UBYTE Rdata = Canvas.fB[Addr];
        if(Color == BLACK)
            Canvas.fB[Addr] = Rdata & ~(0x80 >> (X % 8));
        else
            Canvas.fB[Addr] = Rdata | (0x80 >> (X % 8));
    } else if ( Canvas.Scale == 4 ) {                           //Gray Scale Colors
        UDOUBLE Addr = X / 4 + Y * Canvas.WidthByte;
        Color = Color % 4;  //Garantueed taht color scale is 0 - 3
        UBYTE Rdata = Canvas.fB[Addr];
        Rdata = Rdata & (~(0xC0 >> ((X % 4) * 2))); //Clear first, then set value
        Canvas.fB[Addr] = Rdata | ((Color << 6) >> ((X % 4) * 2));
    }
}

void EPDRenderer::Clear(UWORD Color) {
    if (Canvas.Scale == 2) {
        for (UWORD Y = 0; Y < Canvas.HeightByte; Y++) {
            for (UWORD X = 0; X < Canvas.WidthByte; X++) {
                UDOUBLE Addr = X + Y * Canvas.WidthByte;
                Canvas.fB[Addr] = (Color == WHITE) ? 0xFF : 0x00;
            }
        }
    } else { // Scale == 4 (2-bpp packed)
        UBYTE c = (UBYTE)(Color & 0x03);                 // 0..3
        UBYTE pat = (UBYTE)((c << 6) | (c << 4) | (c << 2) | c); // replicate c across 4 pixels
        for (UWORD Y = 0; Y < Canvas.HeightByte; Y++) {
            for (UWORD X = 0; X < Canvas.WidthByte; X++) {
                UDOUBLE Addr = X + Y * Canvas.WidthByte;
                Canvas.fB[Addr] = pat;
            }
        }
    }
}

void EPDRenderer::ClearPartial(UWORD Xstart, UWORD Ystart, UWORD Xend, UWORD Yend, UWORD Color) {
    UWORD X,Y;
    for(Y = Ystart; Y < Yend; Y++) {
        for(X = Xstart; X < Xend; X++) {
            SetPixel(X, Y, Color);
        }
    }
}

void EPDRenderer::DrawPoint(UWORD Xpoint, UWORD Ypoint, UWORD Color, DOT_PIXEL PixelStyle, DOT_STYLE FillStyle) {
    if ( Xpoint > Canvas.Width || Ypoint > Canvas.Height ) return;

    int16_t XDir_Num, YDir_Num;
    if ( FillStyle == DOT_FILL_AROUND ) {
        for (XDir_Num = 0; XDir_Num < 2 * PixelStyle - 1; XDir_Num++) {
            for (YDir_Num = 0; YDir_Num < 2 * PixelStyle - 1; YDir_Num++) {
                if(Xpoint + XDir_Num - PixelStyle < 0 || Ypoint + YDir_Num - PixelStyle < 0)
                    break;
                
                SetPixel(Xpoint + XDir_Num - PixelStyle, Ypoint + YDir_Num - PixelStyle, Color);
            }
        }
    } else {
        for (XDir_Num = 0; XDir_Num < PixelStyle; XDir_Num++) {
            for (YDir_Num = 0; YDir_Num < PixelStyle; YDir_Num++) {
                SetPixel(Xpoint + XDir_Num - 1, Ypoint + YDir_Num - 1, Color);
            }
        }
    }
}

void EPDRenderer::DrawLine(UWORD Xstart, UWORD Ystart, UWORD Xend, UWORD Yend, UWORD Color, DOT_PIXEL LineWidth, LINE_STYLE LineStyle) {
    if ( Xstart > Canvas.Width || Ystart > Canvas.Height || Xend > Canvas.Width || Yend > Canvas.Height ) return;

    UWORD Xpoint = Xstart;
    UWORD Ypoint = Ystart;
    int dx = (int)Xend - (int)Xstart >= 0 ? Xend - Xstart : Xstart - Xend;
    int dy = (int)Yend - (int)Ystart <= 0 ? Yend - Ystart : Ystart - Yend;

    // Increment direction, 1 is positive, -1 is counter;
    int XAddway = Xstart < Xend ? 1 : -1;
    int YAddway = Ystart < Yend ? 1 : -1;

    //Cumulative error
    int Esp = dx + dy;
    char Dotted_Len = 0;

    for (;;) {
        Dotted_Len++;
        //Painted dotted line, 2 point is really virtual
        if (LineStyle == LINE_STYLE_DOTTED && Dotted_Len % 3 == 0) {
            //Debug("LINE_DOTTED\r\n");
            DrawPoint(Xpoint, Ypoint, Canvas.Color, LineWidth, DOT_STYLE_DFT);
            Dotted_Len = 0;
        } else {
            DrawPoint(Xpoint, Ypoint, Color, LineWidth, DOT_STYLE_DFT);
        }
        if (2 * Esp >= dy) {
            if (Xpoint == Xend)
                break;
            Esp += dy;
            Xpoint += XAddway;
        }
        if (2 * Esp <= dx) {
            if (Ypoint == Yend)
                break;
            Esp += dx;
            Ypoint += YAddway;
        }
    }
}

void EPDRenderer::DrawRect(UWORD Xstart, UWORD Ystart, UWORD Xend, UWORD Yend, UWORD Color, DOT_PIXEL LineWidth, DRAW_FILL FillStyle) {
    if ( Xstart > Canvas.Width || Ystart > Canvas.Height || Xend > Canvas.Width || Yend > Canvas.Height ) return;

    if (FillStyle) {
        UWORD Ypoint;
        for(Ypoint = Ystart; Ypoint < Yend; Ypoint++) {
            DrawLine(Xstart, Ypoint, Xend, Ypoint, Color , LineWidth, LINE_STYLE_SOLID);
        }
    } else {
        DrawLine(Xstart, Ystart, Xend, Ystart, Color, LineWidth, LINE_STYLE_SOLID);
        DrawLine(Xstart, Ystart, Xstart, Yend, Color, LineWidth, LINE_STYLE_SOLID);
        DrawLine(Xend, Yend, Xend, Ystart, Color, LineWidth, LINE_STYLE_SOLID);
        DrawLine(Xend, Yend, Xstart, Yend, Color, LineWidth, LINE_STYLE_SOLID);
    }
}

void EPDRenderer::DrawCircle(UWORD Xcenter, UWORD Ycenter, UWORD Radius, UWORD Color, DOT_PIXEL LineWidth, DRAW_FILL FillStyle) {
    if ( Xcenter > Canvas.Width || Ycenter >= Canvas.Height ) return;
    
    //Draw a circle from(0, R) as a starting point
    int16_t XCurrent, YCurrent;
    XCurrent = 0;
    YCurrent = Radius;

    //Cumulative error,judge the next point of the logo
    int16_t Esp = 3 - (Radius << 1 );

    int16_t sCountY;
    if (FillStyle == DRAW_FILL_FULL) {
        while (XCurrent <= YCurrent ) { //Realistic circles
            for (sCountY = XCurrent; sCountY <= YCurrent; sCountY ++ ) {
                DrawPoint(Xcenter + XCurrent, Ycenter + sCountY, Color, DOT_PIXEL_DFT, DOT_STYLE_DFT);//1
                DrawPoint(Xcenter - XCurrent, Ycenter + sCountY, Color, DOT_PIXEL_DFT, DOT_STYLE_DFT);//2
                DrawPoint(Xcenter - sCountY, Ycenter + XCurrent, Color, DOT_PIXEL_DFT, DOT_STYLE_DFT);//3
                DrawPoint(Xcenter - sCountY, Ycenter - XCurrent, Color, DOT_PIXEL_DFT, DOT_STYLE_DFT);//4
                DrawPoint(Xcenter - XCurrent, Ycenter - sCountY, Color, DOT_PIXEL_DFT, DOT_STYLE_DFT);//5
                DrawPoint(Xcenter + XCurrent, Ycenter - sCountY, Color, DOT_PIXEL_DFT, DOT_STYLE_DFT);//6
                DrawPoint(Xcenter + sCountY, Ycenter - XCurrent, Color, DOT_PIXEL_DFT, DOT_STYLE_DFT);//7
                DrawPoint(Xcenter + sCountY, Ycenter + XCurrent, Color, DOT_PIXEL_DFT, DOT_STYLE_DFT);
            }
            if (Esp < 0 )
                Esp += 4 * XCurrent + 6;
            else {
                Esp += 10 + 4 * (XCurrent - YCurrent );
                YCurrent --;
            }
            XCurrent ++;
        }
    } else { //Draw a hollow circle
        while (XCurrent <= YCurrent ) {
            DrawPoint(Xcenter + XCurrent, Ycenter + YCurrent, Color, LineWidth, DOT_STYLE_DFT);//1
            DrawPoint(Xcenter - XCurrent, Ycenter + YCurrent, Color, LineWidth, DOT_STYLE_DFT);//2
            DrawPoint(Xcenter - YCurrent, Ycenter + XCurrent, Color, LineWidth, DOT_STYLE_DFT);//3
            DrawPoint(Xcenter - YCurrent, Ycenter - XCurrent, Color, LineWidth, DOT_STYLE_DFT);//4
            DrawPoint(Xcenter - XCurrent, Ycenter - YCurrent, Color, LineWidth, DOT_STYLE_DFT);//5
            DrawPoint(Xcenter + XCurrent, Ycenter - YCurrent, Color, LineWidth, DOT_STYLE_DFT);//6
            DrawPoint(Xcenter + YCurrent, Ycenter - XCurrent, Color, LineWidth, DOT_STYLE_DFT);//7
            DrawPoint(Xcenter + YCurrent, Ycenter + XCurrent, Color, LineWidth, DOT_STYLE_DFT);//0

            if (Esp < 0 )
                Esp += 4 * XCurrent + 6;
            else {
                Esp += 10 + 4 * (XCurrent - YCurrent );
                YCurrent --;
            }
            XCurrent ++;
        }
    }
}

uint8_t EPDRenderer::DrawChar(UWORD uChar, UWORD xPoint, UWORD yPoint, UWORD color) {
    const uint8_t* font = _binary_fonts_unifont_bin_start;

    //Sanity check: char must be in BMP (U+0000 - U+FFFF)
    if (uChar >= 0xFFFF)
        return 0;

    //Get 3byte offset from header
    uint32_t header_offset = uChar * 3;
    uint32_t glyph_offset = font[header_offset] |
                            (font[header_offset + 1] << 8) |
                            (font[header_offset + 2] << 16);

    if (glyph_offset == 0)
        return 0;

    const uint8_t* glyph = font + glyph_offset;

    //First byte = WH byte
    uint8_t wh = glyph[0];
    uint8_t width = ((wh >> 4) & 0xF) + 1;
    uint8_t height = (wh & 0xF) + 1;

    //1bpp, so total bits = width * height
    uint32_t bitmap_size = ((width * height) + 7) / 8;

    const uint8_t* bitmap = glyph + 1;

    //Render Pixel by Pixel
    for (uint8_t y = 0; y < height; ++y) {
        for (uint8_t x = 0; x < width; ++x) {
            uint32_t bit_index = y * width + x;
            uint8_t byte = bitmap[bit_index / 8];
            uint8_t bit = 7 - (bit_index % 8);
            if (byte & (1 << bit)) {
                SetPixel(xPoint + x, yPoint + y, color);
            }
        }
    }

    return width;
}

void EPDRenderer::DrawText(const UWORD* text, size_t length, UWORD xPoint, UWORD yPoint, UWORD color, UBYTE spacing, UWORD max_text_area) {
    UWORD x = xPoint;
    UWORD used_width = 0;

    const uint16_t dot = '.';
    const uint8_t dot_width = GetCharWidth(dot);
    const uint8_t ellipsis_width = (dot_width + spacing) * 3;

    for (size_t i = 0; i < length; i++) {
        uint8_t char_width = GetCharWidth(text[i]);
        
        bool needsTruncation = (i < length - 1) &&
                                (max_text_area != 0) &&
                                (used_width + char_width + ellipsis_width > max_text_area);
        if (needsTruncation) {
            //Draw "."
            for(int j = 0; j < 3; j ++) {
                x += DrawChar(dot, x, yPoint, color) + spacing; //No NULL check needed, dots are always defined... otherwise its a shit font
            }
            return;
        }

        //Redundency but what the hell, why not
        char_width = DrawChar(text[i], x, yPoint, color);
        x += char_width;
        
        used_width += char_width;
        if (i < length - 1) {
            if (char_width > 0) { x += spacing; used_width += spacing; }
        }
    }
}

uint8_t EPDRenderer::GetCharWidth(UWORD uChar) {
    const uint8_t* font = _binary_fonts_unifont_bin_start;

    //Sanity check: char must be in BMP (U+0000 - U+FFFF)
    if (uChar >= 0xFFFF)
        return 0;

    //Get 3byte offset from header
    uint32_t header_offset = uChar * 3;
    uint32_t glyph_offset = font[header_offset] |
                            (font[header_offset + 1] << 8) |
                            (font[header_offset + 2] << 16);

    if (glyph_offset == 0)
        return 0;

    const uint8_t* glyph = font + glyph_offset;

    //First byte = WH byte
    uint8_t wh = glyph[0];
    uint8_t width = ((wh >> 4) & 0xF) + 1;

    return width;
}

void EPDRenderer::DrawBitmap(const uint8_t* image_buffer, UWORD xPoint, UWORD yPoint, UWORD width, UWORD height) {
    if (!image_buffer) return;

    for (UWORD y = 0; y < height; ++y) {
        for (UWORD x = 0; x < width; ++x) {
            uint32_t i = (uint32_t)y * width + x;     // pixel index in the source
            uint8_t byte = image_buffer[i >> 2];      // 4 pixels per byte
            uint8_t shift = 6 - ((i & 3) << 1);       // 6,4,2,0
            uint8_t v = (byte >> shift) & 0x03;       // 0..3

            if (Canvas.Scale == 4) {
                // grayscale framebuffer: SetPixel expects 0..3 (0=whitest, 3=blackest)
                SetPixel(xPoint + x, yPoint + y, v);
            } else {
                // 1bpp framebuffer: simple threshold (tweak if you prefer different cutoff)
                SetPixel(xPoint + x, yPoint + y, (v >= 2) ? BLACK : WHITE);
            }
        }
    }
}