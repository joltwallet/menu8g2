#ifndef __MENU8G2_H__
#define __MENU8G2_H__

#include "u8g2.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

typedef enum menu8g2_err_t{
    E_SUCCESS=0
} menu8g2_err_t;

typedef enum buttons{
    UP, DOWN, LEFT, RIGHT, ENTER, NUMBER_OF_BUTTON_TYPES
} buttons_t;

//typedef struct menu8g2_t menu8g2_t;
typedef struct menu8g2_t{
    uint32_t index;
    QueueHandle_t *input_queue;
    u8g2_t *u8g2;
    struct menu8g2 *prev; // Previous Menu
} menu8g2_t;

/* Allocate memory for menu8g2_t object and set display object
 * menu - menu object to initializie
 * u8g2 - display object for menu to use
 * input_queue - bitmasked inputs for controlling the menu; Suggested to have
 * enumerated bitshifts.
 * */
menu8g2_err_t menu8g2_init(menu8g2_t *menu,
        u8g2_t *u8g2,
        QueueHandle_t *input_queue
        );

/* Change the menu's index (default starting value of 0 */
menu8g2_err_t menu8g2_set_index(menu8g2_t *menu, const uint32_t index);

menu8g2_err_t menu8g2_get_index(menu8g2_t *menu);

/* Generic Vertical Scrolling Menu */
bool menu8g2_create_vertical_menu(menu8g2_t *menu,
        const char title[],
        const void *meta,
        const void (*index_to_option)(char buf[], const void *meta, const uint8_t index),
        const uint32_t max_lines
        );

/* Simple Menu That only takes in an array of strings and returns selected index */
bool menu8g2_create_simple(menu8g2_t *menu,
        const char title[],
        const char *options[],
        const uint32_t options_len
        );

/* Create a FreeRTOS Task For the Menu */
menu8g2_err_t menu_task(menu8g2_t *menu);

#endif
