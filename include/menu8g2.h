#ifndef __MENU8G2_H__
#define __MENU8G2_H__

#include "u8g2.h"
#include "menu8g2.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "easy_input.h"

#define CHAR_PER_LINE_WRAP 20

typedef enum menu8g2_err_t{
    MENU8G2_SUCCESS=0,
    MENU8G2_FAILURE
} menu8g2_err_t;


//typedef struct menu8g2_t menu8g2_t;
typedef struct menu8g2_t{
    uint32_t index;
    QueueHandle_t *input_queue;
    u8g2_t *u8g2;
    SemaphoreHandle_t *disp_mutex;
    void (*pre_draw)(struct menu8g2_t *);
    void (*post_draw)(struct menu8g2_t *);
} menu8g2_t;

// For modular construction of menus from element structs
typedef struct menu8g2_element_t{
    char *name;
    void (*menu_h)(menu8g2_t *prev);
} menu8g2_element_t;

typedef struct menu8g2_elements_t{
    uint32_t index;
    uint32_t n;
    struct menu8g2_element_t *elements;
} menu8g2_elements_t;


/* Allocate memory for menu8g2_t object and set display object
 * menu - menu object to initializie
 * u8g2 - display object for menu to use
 * input_queue - bitmasked inputs for controlling the menu; Suggested to have
 * enumerated bitshifts.
 * */
void menu8g2_init(menu8g2_t *menu,
        u8g2_t *u8g2,
        QueueHandle_t input_queue,
        SemaphoreHandle_t disp_mutex,
        void (*pre_draw)(menu8g2_t *menu),
        void (*post_draw)(menu8g2_t *menu)
        );
void menu8g2_copy(menu8g2_t *menu, menu8g2_t *old);

/* Change the menu's index (default starting value of 0 */
menu8g2_err_t menu8g2_set_index(menu8g2_t *menu, const uint32_t index);
uint32_t menu8g2_get_index(menu8g2_t *menu);

QueueHandle_t *menu8g2_get_input_queue(menu8g2_t *menu);
SemaphoreHandle_t *menu8g2_get_disp_mutex(menu8g2_t *menu);

u8g2_t *menu8g2_get_u8g2(menu8g2_t *menu);

/* Generic Vertical Scrolling Menu */
bool menu8g2_create_vertical_menu(menu8g2_t *menu,
        const char title[],
        void *meta,
        void (*index_to_option)(char buf[], const size_t buf_len, void *meta, const uint8_t index),
        const uint32_t max_lines
        );

void menu8g2_set_element(menu8g2_elements_t *elements, char *name, void *func);
void menu8g2_elements_init(menu8g2_elements_t *elements, uint32_t n);
void menu8g2_elements_free(menu8g2_elements_t *elements);

void menu8g2_create_vertical_element_menu(menu8g2_t *menu,
        const char title[],
        menu8g2_elements_t *elements);

/* Simple Menu That only takes in an array of strings and returns selected index */
bool menu8g2_create_simple(menu8g2_t *menu,
        const char title[],
        const char *options[],
        const uint32_t options_len
        );

uint64_t menu8g2_display_text(menu8g2_t *menu, const char *text);

/* Create a FreeRTOS Task For the Menu */
menu8g2_err_t menu_task(menu8g2_t *menu); // Not yet implemented

#endif
