#ifndef SSD1306_H
#define SSD1306_H

#include <stdbool.h>
#include <stdint.h>

#include "nordic_common.h"

#define SSD1306_ADDR 0x3C
#define SSD1306_SDA 30
#define SSD1306_SCL 7

#define SSD1306_WIDTH 128
#define SSD1306_HEIGHT 64

#define SSD1306_BUFFER_PAGE_SIZE 128

/* TWI instance ID. */
#define TWI_INSTANCE_ID 0

#define BLACK 0
#define WHITE 1
#define INVERSE 2

// this one clears the ssd1306 chip (and display)
void ssd1306_clear();
// this one clears only the nrf buffer
void ssd1306_clear_buffer(void);
void ssd1306_display();
void ssd1306_draw_circle(int16_t x0, int16_t y0, int16_t r, uint16_t color);
void ssd1306_draw_fast_hline(int16_t x, int16_t y, int16_t w, uint16_t color);
void ssd1306_draw_fast_hline_internal(int16_t x, int16_t y, int16_t w, uint16_t color);
void ssd1306_draw_fast_vline(int16_t x, int16_t y, int16_t h, uint16_t color);
void ssd1306_draw_fast_vline_internal(int16_t x, int16_t __y, int16_t __h, uint16_t color);
void ssd1306_draw_line(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
void ssd1306_draw_pixel(int16_t x, int16_t y, uint16_t color);
void ssd1306_fill_circle(int16_t x0, int16_t y0, int16_t r, uint16_t color);
bool ssd1306_init();
void ssd1306_inverse(bool);
void ssd1306_setup();

void twi_init(void);

#endif
