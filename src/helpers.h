#ifndef __MENU8G2_HELPERS_H__
#define __MENU8G2_HELPERS_H__

uint8_t get_center_x(u8g2_t *u8g2, const char *text);

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

#endif

