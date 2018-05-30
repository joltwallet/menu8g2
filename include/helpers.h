/* menu8g2 - Menuing library using the U8G2 Graphical Library
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 3 of the License, or
 (at your option) any later version.
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software Foundation,
 Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
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

