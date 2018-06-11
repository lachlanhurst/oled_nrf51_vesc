
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include "app_error.h"
#include "app_util_platform.h"
#include "nrf_drv_twi.h"

#include "ssd1306.h"

#define ssd1306_swap(a, b) { int16_t t = a; a = b; b = t; }
#define adagfxswap(a, b) { int16_t t = a; a = b; b = t; }

static const nrf_drv_twi_t m_twi = NRF_DRV_TWI_INSTANCE(TWI_INSTANCE_ID);

static int16_t _width, _height, WIDTH, HEIGHT, cursor_x, cursor_y;
static uint8_t textsize, rotation;
static uint16_t textcolor, textbgcolor;
bool wrap,   // If set, 'wrap' text at right edge of display
     _cp437; // If set, use correct CP437 charset (default is off)



static uint8_t buffer[SSD1306_WIDTH * SSD1306_HEIGHT / 8] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80,
    0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x80, 0x80, 0xC0, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x80, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC, 0xF8, 0xE0, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x00, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0x80, 0x80, 0x80, 0x00, 0xFF,
    0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0x80, 0x80, 0x00, 0x00, 0x80, 0x80, 0x00, 0x00,
    0x80, 0xFF, 0xFF, 0x80, 0x80, 0x00, 0x80, 0x80, 0x00, 0x80, 0x80, 0x80, 0x80, 0x00, 0x80, 0x80,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0x00, 0x00, 0x8C, 0x8E, 0x84, 0x00, 0x00, 0x80, 0xF8,
    0xF8, 0xF8, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xE0, 0xE0, 0xC0, 0x80,
    0x00, 0xE0, 0xFC, 0xFE, 0xFF, 0xFF, 0xFF, 0x7F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFE, 0xFF, 0xC7, 0x01, 0x01,
    0x01, 0x01, 0x83, 0xFF, 0xFF, 0x00, 0x00, 0x7C, 0xFE, 0xC7, 0x01, 0x01, 0x01, 0x01, 0x83, 0xFF,
    0xFF, 0xFF, 0x00, 0x38, 0xFE, 0xC7, 0x83, 0x01, 0x01, 0x01, 0x83, 0xC7, 0xFF, 0xFF, 0x00, 0x00,
    0x01, 0xFF, 0xFF, 0x01, 0x01, 0x00, 0xFF, 0xFF, 0x07, 0x01, 0x01, 0x01, 0x00, 0x00, 0x7F, 0xFF,
    0x80, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x7F, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x01, 0xFF,
    0xFF, 0xFF, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x03, 0x0F, 0x3F, 0x7F, 0x7F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xE7, 0xC7, 0xC7, 0x8F,
    0x8F, 0x9F, 0xBF, 0xFF, 0xFF, 0xC3, 0xC0, 0xF0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFC, 0xFC, 0xFC,
    0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xF8, 0xF8, 0xF0, 0xF0, 0xE0, 0xC0, 0x00, 0x01, 0x03, 0x03, 0x03,
    0x03, 0x03, 0x01, 0x03, 0x03, 0x00, 0x00, 0x00, 0x00, 0x01, 0x03, 0x03, 0x03, 0x03, 0x01, 0x01,
    0x03, 0x01, 0x00, 0x00, 0x00, 0x01, 0x03, 0x03, 0x03, 0x03, 0x01, 0x01, 0x03, 0x03, 0x00, 0x00,
    0x00, 0x03, 0x03, 0x00, 0x00, 0x00, 0x03, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
    0x03, 0x03, 0x03, 0x03, 0x03, 0x01, 0x00, 0x00, 0x00, 0x01, 0x03, 0x01, 0x00, 0x00, 0x00, 0x03,
    0x03, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x80, 0xC0, 0xE0, 0xF0, 0xF9, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x3F, 0x1F, 0x0F,
    0x87, 0xC7, 0xF7, 0xFF, 0xFF, 0x1F, 0x1F, 0x3D, 0xFC, 0xF8, 0xF8, 0xF8, 0xF8, 0x7C, 0x7D, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7F, 0x3F, 0x0F, 0x07, 0x00, 0x30, 0x30, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xFE, 0xFE, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE0, 0xC0, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x30, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0xC0, 0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7F, 0x7F, 0x3F, 0x1F,
    0x0F, 0x07, 0x1F, 0x7F, 0xFF, 0xFF, 0xF8, 0xF8, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0xF8, 0xE0,
    0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFE, 0xFE, 0x00, 0x00,
    0x00, 0xFC, 0xFE, 0xFC, 0x0C, 0x06, 0x06, 0x0E, 0xFC, 0xF8, 0x00, 0x00, 0xF0, 0xF8, 0x1C, 0x0E,
    0x06, 0x06, 0x06, 0x0C, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0xFE, 0xFE, 0x00, 0x00, 0x00, 0x00, 0xFC,
    0xFE, 0xFC, 0x00, 0x18, 0x3C, 0x7E, 0x66, 0xE6, 0xCE, 0x84, 0x00, 0x00, 0x06, 0xFF, 0xFF, 0x06,
    0x06, 0xFC, 0xFE, 0xFC, 0x0C, 0x06, 0x06, 0x06, 0x00, 0x00, 0xFE, 0xFE, 0x00, 0x00, 0xC0, 0xF8,
    0xFC, 0x4E, 0x46, 0x46, 0x46, 0x4E, 0x7C, 0x78, 0x40, 0x18, 0x3C, 0x76, 0xE6, 0xCE, 0xCC, 0x80,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x01, 0x07, 0x0F, 0x1F, 0x1F, 0x3F, 0x3F, 0x3F, 0x3F, 0x1F, 0x0F, 0x03,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x0F, 0x00, 0x00,
    0x00, 0x0F, 0x0F, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x0F, 0x00, 0x00, 0x03, 0x07, 0x0E, 0x0C,
    0x18, 0x18, 0x0C, 0x06, 0x0F, 0x0F, 0x0F, 0x00, 0x00, 0x01, 0x0F, 0x0E, 0x0C, 0x18, 0x0C, 0x0F,
    0x07, 0x01, 0x00, 0x04, 0x0E, 0x0C, 0x18, 0x0C, 0x0F, 0x07, 0x00, 0x00, 0x00, 0x0F, 0x0F, 0x00,
    0x00, 0x0F, 0x0F, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x0F, 0x00, 0x00, 0x00, 0x07,
    0x07, 0x0C, 0x0C, 0x18, 0x1C, 0x0C, 0x06, 0x06, 0x00, 0x04, 0x0E, 0x0C, 0x18, 0x0C, 0x0F, 0x07,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};


int16_t ssd1306_width(void)
{
    return _width;
}

int16_t ssd1306_height(void)
{
    return _height;
}

void set_rotation(uint8_t x)
{
    rotation = (x & 3);
    switch (rotation) {
    case 0:
    case 2:
        _width  = WIDTH;
        _height = HEIGHT;
        break;
    case 1:
    case 3:
        _width  = HEIGHT;
        _height = WIDTH;
        break;
    }
}

void ssd1306_setup()
{
    _width    = WIDTH;
    _height   = HEIGHT;
    cursor_y  = cursor_x    = 0;
    textsize  = 1;
    textcolor = textbgcolor = 0xFFFF;
    wrap      = true;
    _cp437    = false;

    _width = WIDTH = SSD1306_WIDTH;
    _height = HEIGHT = SSD1306_HEIGHT;
    rotation  = 0;
}


void ssd1306_draw_pixel(int16_t x, int16_t y, uint16_t color)
{
    if ((x < 0) || (x >= ssd1306_width()) || (y < 0) || (y >= ssd1306_height()))
        return;

    // check rotation, move pixel around if necessary
    switch (rotation) {
    case 1:
        ssd1306_swap(x, y);
        x = WIDTH - x - 1;
        break;
    case 2:
        x = WIDTH - x - 1;
        y = HEIGHT - y - 1;
        break;
    case 3:
        ssd1306_swap(x, y);
        y = HEIGHT - y - 1;
        break;
    }

    // x is which column
    switch (color) {
    case WHITE:
        buffer[x + (y / 8)*SSD1306_WIDTH] |=  (1 << (y & 7));
        break;
    case BLACK:
        buffer[x + (y / 8)*SSD1306_WIDTH] &= ~(1 << (y & 7));
        break;
    case INVERSE:
        buffer[x + (y / 8)*SSD1306_WIDTH] ^=  (1 << (y & 7));
        break;
    }
}


void ssd1306_clear_buffer(void)
{
    memset(buffer, 0, (SSD1306_WIDTH * SSD1306_HEIGHT / 8));
}


void ssd1306_draw_fast_vline(int16_t x, int16_t y, int16_t h, uint16_t color)
{
    bool __swap = false;
    switch (rotation) {
    case 0:
        break;
    case 1:
        // 90 degree rotation, swap x & y for rotation, then invert x and adjust x for h (now to become w)
        __swap = true;
        ssd1306_swap(x, y);
        x = WIDTH - x - 1;
        x -= (h - 1);
        break;
    case 2:
        // 180 degree rotation, invert x and y - then shift y around for height.
        x = WIDTH - x - 1;
        y = HEIGHT - y - 1;
        y -= (h - 1);
        break;
    case 3:
        // 270 degree rotation, swap x & y for rotation, then invert y
        __swap = true;
        ssd1306_swap(x, y);
        y = HEIGHT - y - 1;
        break;
    }

    if (__swap) {
        ssd1306_draw_fast_hline_internal(x, y, h, color);
    }
    else {
        ssd1306_draw_fast_vline_internal(x, y, h, color);
    }
}


void ssd1306_draw_fast_vline_internal(int16_t x, int16_t __y, int16_t __h, uint16_t color)
{

    // do nothing if we're off the left or right side of the screen
    if (x < 0 || x >= WIDTH) {
        return;
    }

    // make sure we don't try to draw below 0
    if (__y < 0) {
        // __y is negative, this will subtract enough from __h to account for __y being 0
        __h += __y;
        __y = 0;

    }

    // make sure we don't go past the height of the display
    if ( (__y + __h) > HEIGHT) {
        __h = (HEIGHT - __y);
    }

    // if our height is now negative, punt
    if (__h <= 0) {
        return;
    }

    // this display doesn't need ints for coordinates, use local byte registers for faster juggling
    register uint8_t y = __y;
    register uint8_t h = __h;


    // set up the pointer for fast movement through the buffer
    register uint8_t *pBuf = buffer;
    // adjust the buffer pointer for the current row
    pBuf += ((y / 8) * SSD1306_WIDTH);
    // and offset x columns in
    pBuf += x;

    // do the first partial byte, if necessary - this requires some masking
    register uint8_t mod = (y & 7);
    if (mod) {
        // mask off the high n bits we want to set
        mod = 8 - mod;

        // note - lookup table results in a nearly 10% performance improvement in fill* functions
        // register uint8_t mask = ~(0xFF >> (mod));
        static uint8_t premask[8] = {0x00, 0x80, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC, 0xFE };
        register uint8_t mask = premask[mod];

        // adjust the mask if we're not going to reach the end of this byte
        if ( h < mod) {
            mask &= (0XFF >> (mod - h));
        }

        switch (color) {
        case WHITE:
            *pBuf |=  mask;
            break;
        case BLACK:
            *pBuf &= ~mask;
            break;
        case INVERSE:
            *pBuf ^=  mask;
            break;
        }

        // fast exit if we're done here!
        if (h < mod) {
            return;
        }

        h -= mod;

        pBuf += SSD1306_WIDTH;
    }


    // write solid bytes while we can - effectively doing 8 rows at a time
    if (h >= 8) {
        if (color == INVERSE)  {          // separate copy of the code so we don't impact performance of the black/white write version with an extra comparison per loop
            do  {
                *pBuf = ~(*pBuf);

                // adjust the buffer forward 8 rows worth of data
                pBuf += SSD1306_WIDTH;

                // adjust h & y (there's got to be a faster way for me to do this, but this should still help a fair bit for now)
                h -= 8;
            }
            while (h >= 8);
        }
        else {
            // store a local value to work with
            register uint8_t val = (color == WHITE) ? 255 : 0;

            do  {
                // write our value in
                *pBuf = val;

                // adjust the buffer forward 8 rows worth of data
                pBuf += SSD1306_WIDTH;

                // adjust h & y (there's got to be a faster way for me to do this, but this should still help a fair bit for now)
                h -= 8;
            }
            while (h >= 8);
        }
    }

    // now do the final partial byte, if necessary
    if (h) {
        mod = h & 7;
        // this time we want to mask the low bits of the byte, vs the high bits we did above
        // register uint8_t mask = (1 << mod) - 1;
        // note - lookup table results in a nearly 10% performance improvement in fill* functions
        static uint8_t postmask[8] = {0x00, 0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F };
        register uint8_t mask = postmask[mod];
        switch (color) {
        case WHITE:
            *pBuf |=  mask;
            break;
        case BLACK:
            *pBuf &= ~mask;
            break;
        case INVERSE:
            *pBuf ^=  mask;
            break;
        }
    }
}


void ssd1306_draw_fast_hline(int16_t x, int16_t y, int16_t w, uint16_t color)
{
    bool __swap = false;
    switch (rotation) {
    case 0:
        // 0 degree rotation, do nothing
        break;
    case 1:
        // 90 degree rotation, swap x & y for rotation, then invert x
        __swap = true;
        ssd1306_swap(x, y);
        x = WIDTH - x - 1;
        break;
    case 2:
        // 180 degree rotation, invert x and y - then shift y around for height.
        x = WIDTH - x - 1;
        y = HEIGHT - y - 1;
        x -= (w - 1);
        break;
    case 3:
        // 270 degree rotation, swap x & y for rotation, then invert y  and adjust y for w (not to become h)
        __swap = true;
        ssd1306_swap(x, y);
        y = HEIGHT - y - 1;
        y -= (w - 1);
        break;
    }

    if (__swap) {
        ssd1306_draw_fast_vline_internal(x, y, w, color);
    }
    else {
        ssd1306_draw_fast_hline_internal(x, y, w, color);
    }
}

void ssd1306_draw_fast_hline_internal(int16_t x, int16_t y, int16_t w, uint16_t color)
{
    // Do bounds/limit checks
    if (y < 0 || y >= HEIGHT) {
        return;
    }

    // make sure we don't try to draw below 0
    if (x < 0) {
        w += x;
        x = 0;
    }

    // make sure we don't go off the edge of the display
    if ( (x + w) > WIDTH) {
        w = (WIDTH - x);
    }

    // if our width is now negative, punt
    if (w <= 0) {
        return;
    }

    // set up the pointer for  movement through the buffer
    register uint8_t *pBuf = buffer;
    // adjust the buffer pointer for the current row
    pBuf += ((y / 8) * SSD1306_WIDTH);
    // and offset x columns in
    pBuf += x;

    register uint8_t mask = 1 << (y & 7);

    switch (color) {
    case WHITE:
        while (w--) {
            *pBuf++ |= mask;
        };
        break;
    case BLACK:
        mask = ~mask;
        while (w--) {
            *pBuf++ &= mask;
        };
        break;
    case INVERSE:
        while (w--) {
            *pBuf++ ^= mask;
        };
        break;
    }
}




// Bresenham's algorithm - thx wikpedia
void ssd1306_draw_line(int16_t x0, int16_t y0,
                       int16_t x1, int16_t y1,
                       uint16_t color)
{
    int16_t steep = abs(y1 - y0) > abs(x1 - x0);
    if (steep) {
        adagfxswap(x0, y0);
        adagfxswap(x1, y1);
    }

    if (x0 > x1) {
        adagfxswap(x0, x1);
        adagfxswap(y0, y1);
    }

    int16_t dx, dy;
    dx = x1 - x0;
    dy = abs(y1 - y0);

    int16_t err = dx / 2;
    int16_t ystep;

    if (y0 < y1) {
        ystep = 1;
    }
    else {
        ystep = -1;
    }

    for (; x0 <= x1; x0++) {
        if (steep) {
            ssd1306_draw_pixel(y0, x0, color);
        }
        else {
            ssd1306_draw_pixel(x0, y0, color);
        }
        err -= dy;
        if (err < 0) {
            y0 += ystep;
            err += dx;
        }
    }
}


// Draw a circle outline
void ssd1306_draw_circle(int16_t x0, int16_t y0, int16_t r, uint16_t color)
{
    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x = 0;
    int16_t y = r;

    ssd1306_draw_pixel(x0  , y0 + r, color);
    ssd1306_draw_pixel(x0  , y0 - r, color);
    ssd1306_draw_pixel(x0 + r, y0  , color);
    ssd1306_draw_pixel(x0 - r, y0  , color);

    while (x < y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        ssd1306_draw_pixel(x0 + x, y0 + y, color);
        ssd1306_draw_pixel(x0 - x, y0 + y, color);
        ssd1306_draw_pixel(x0 + x, y0 - y, color);
        ssd1306_draw_pixel(x0 - x, y0 - y, color);
        ssd1306_draw_pixel(x0 + y, y0 + x, color);
        ssd1306_draw_pixel(x0 - y, y0 + x, color);
        ssd1306_draw_pixel(x0 + y, y0 - x, color);
        ssd1306_draw_pixel(x0 - y, y0 - x, color);
    }
}

void ssd1306_draw_circle_helper(int16_t x0, int16_t y0,
                                int16_t r, uint8_t cornername, uint16_t color)
{
    int16_t f     = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x     = 0;
    int16_t y     = r;

    while (x < y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f     += ddF_y;
        }
        x++;
        ddF_x += 2;
        f     += ddF_x;
        if (cornername & 0x4) {
            ssd1306_draw_pixel(x0 + x, y0 + y, color);
            ssd1306_draw_pixel(x0 + y, y0 + x, color);
        }
        if (cornername & 0x2) {
            ssd1306_draw_pixel(x0 + x, y0 - y, color);
            ssd1306_draw_pixel(x0 + y, y0 - x, color);
        }
        if (cornername & 0x8) {
            ssd1306_draw_pixel(x0 - y, y0 + x, color);
            ssd1306_draw_pixel(x0 - x, y0 + y, color);
        }
        if (cornername & 0x1) {
            ssd1306_draw_pixel(x0 - y, y0 - x, color);
            ssd1306_draw_pixel(x0 - x, y0 - y, color);
        }
    }
}

void ssd1306_fill_circle(int16_t x0, int16_t y0, int16_t r, uint16_t color)
{
    ssd1306_draw_fast_vline(x0, y0 - r, 2 * r + 1, color);
    ssd1306_fill_circle_helper(x0, y0, r, 3, 0, color);
}

// Used to do circles and roundrects
void ssd1306_fill_circle_helper(int16_t x0, int16_t y0, int16_t r,
                                uint8_t cornername, int16_t delta, uint16_t color)
{

    int16_t f     = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x     = 0;
    int16_t y     = r;

    while (x < y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f     += ddF_y;
        }
        x++;
        ddF_x += 2;
        f     += ddF_x;

        if (cornername & 0x1) {
            ssd1306_draw_fast_vline(x0 + x, y0 - y, 2 * y + 1 + delta, color);
            ssd1306_draw_fast_vline(x0 + y, y0 - x, 2 * x + 1 + delta, color);
        }
        if (cornername & 0x2) {
            ssd1306_draw_fast_vline(x0 - x, y0 - y, 2 * y + 1 + delta, color);
            ssd1306_draw_fast_vline(x0 - y, y0 - x, 2 * x + 1 + delta, color);
        }
    }
}




static bool ssd1306_send_cmd(uint8_t cmd) {
    ret_code_t err_code;

    uint8_t w2_data[2];

    w2_data[0] = 0x00;
    w2_data[1] = cmd;

    err_code = nrf_drv_twi_tx(&m_twi, SSD1306_ADDR, w2_data, sizeof(w2_data), false);
    APP_ERROR_CHECK(err_code);
}

static bool ssd1306_send_data(uint8_t data) {
    ret_code_t err_code;

    uint8_t w2_data[2];

    w2_data[0] = 0x40;
    w2_data[1] = data;

    err_code = nrf_drv_twi_tx(&m_twi, SSD1306_ADDR, w2_data, 2, false);
    APP_ERROR_CHECK(err_code);
}

bool ssd1306_init() {
    ssd1306_send_cmd(0xAE);
    ssd1306_send_cmd(0xD5);
    ssd1306_send_cmd(0x80);
    ssd1306_send_cmd(0XA8);
    ssd1306_send_cmd(0x3F);
    ssd1306_send_cmd(0xD3);
    ssd1306_send_cmd(0x00);
    ssd1306_send_cmd(0x40 | 0x00);
    ssd1306_send_cmd(0x8D);
    ssd1306_send_cmd(0x14);
    ssd1306_send_cmd(0x20);
    ssd1306_send_cmd(0x00);
    ssd1306_send_cmd(0xA0 | 0x1);
    ssd1306_send_cmd(0xC8);
    ssd1306_send_cmd(0xDA);
    ssd1306_send_cmd(0x12);
    ssd1306_send_cmd(0x81);
    ssd1306_send_cmd(0x8F);
    ssd1306_send_cmd(0xD9);
    ssd1306_send_cmd(0xF1);
    ssd1306_send_cmd(0xDB);
    ssd1306_send_cmd(0x40);
    ssd1306_send_cmd(0xA4);
    ssd1306_send_cmd(0xA6);
    return ssd1306_send_cmd(0xAF);
}

void ssd1306_clear() {
    ssd1306_send_cmd(0x21); // set column start/end
    ssd1306_send_cmd(0);
    ssd1306_send_cmd(127);

    ssd1306_send_cmd(0x22); // set page start/end
    ssd1306_send_cmd(0);
    ssd1306_send_cmd(7);

    for (int i = 0; i < (SSD1306_WIDTH * SSD1306_HEIGHT)/8; i++) {
        ssd1306_send_data(0x00);
    }
}

void ssd1306_inverse(bool inverted) {
	if (inverted) {
        ssd1306_send_cmd(0xA7);
    } else {
        ssd1306_send_cmd(0xA6);
    }
}

void ssd1306_display() {

    ssd1306_send_cmd(0x21); // set column start/end
    ssd1306_send_cmd(0);
    ssd1306_send_cmd(127);

    ssd1306_send_cmd(0x22); // set page start/end
    ssd1306_send_cmd(0);
    ssd1306_send_cmd(7);

    uint8_t w2_data[SSD1306_BUFFER_PAGE_SIZE+1];
    uint8_t *p = &w2_data;
    p++;
    w2_data[0] = 0x40;

    uint8_t *bp = &buffer;
    for (int i = 0; i < (SSD1306_WIDTH * SSD1306_HEIGHT)/8; i=i+SSD1306_BUFFER_PAGE_SIZE) {
        //ssd1306_send_data(buffer[i]);
        memcpy(p, bp, SSD1306_BUFFER_PAGE_SIZE);

        nrf_drv_twi_tx(&m_twi, SSD1306_ADDR, w2_data, SSD1306_BUFFER_PAGE_SIZE+1, false);

        bp=bp+SSD1306_BUFFER_PAGE_SIZE;
    }

}

/**
 * @brief TWI initialization.
 */
void twi_init(void)
{
    ret_code_t err_code;

    const nrf_drv_twi_config_t twi_lm75b_config = {
       .scl                = SSD1306_SCL,
       .sda                = SSD1306_SDA,
       .frequency          = NRF_TWI_FREQ_400K,
       .interrupt_priority = APP_IRQ_PRIORITY_HIGH,
       .clear_bus_init     = false
    };

    err_code = nrf_drv_twi_init(&m_twi, &twi_lm75b_config, NULL, NULL);
    APP_ERROR_CHECK(err_code);

    nrf_drv_twi_enable(&m_twi);
}
