#ifndef __MENU8G2_HELPERS_H__
#define __MENU8G2_HELPERS_H__

uint8_t get_center_x(u8g2_t *u8g2, const char *text);
bool menu8g2_draw_str(menu8g2_t *menu, const uint16_t x, const uint16_t y, const char *str, const uint16_t line_start);
char *word_wrap(char* buffer, char* string, int line_width);

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

#endif

