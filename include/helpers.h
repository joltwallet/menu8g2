/* helpers.h
Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
https://www.joltwallet.com/
*/

#ifndef __MENU8G2_HELPERS_H__
#define __MENU8G2_HELPERS_H__

struct menu8g2_t;

uint8_t menu8g2_get_center_x(struct menu8g2_t *menu, const char *text);
bool menu8g2_draw_str(struct menu8g2_t *menu, const uint16_t x, const uint16_t y, const char *str, const uint16_t line_start);
char *menu8g2_word_wrap(char* buffer, size_t *buf_len, const char* string, const int line_width);

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

#endif

