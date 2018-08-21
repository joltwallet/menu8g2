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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "u8g2.h"
#include "menu8g2.h"
#include "easy_input.h"


void menu8g2_init(menu8g2_t *menu, u8g2_t *u8g2,
        QueueHandle_t input_queue,
        SemaphoreHandle_t disp_mutex,
        void (*pre_draw)(menu8g2_t *menu),
        void (*post_draw)(menu8g2_t *menu)
        ){
    menu->u8g2 = u8g2;
    menu->input_queue = input_queue;
    menu->index = 0;
    menu->disp_mutex = disp_mutex;
    menu->pre_draw = pre_draw;
    menu->post_draw = post_draw;
}

void menu8g2_copy(menu8g2_t *new, const menu8g2_t *old){
    /* Copies everything except index */
    new->u8g2 = old->u8g2;
    new->input_queue = old->input_queue;
    new->index = 0;
    new->pre_draw = old->pre_draw;
    new->post_draw = old->post_draw;
    new->disp_mutex = old->disp_mutex;
}

menu8g2_err_t menu8g2_set_index(menu8g2_t *menu, const uint32_t index){
    menu->index = index;
    return MENU8G2_SUCCESS;
}

uint32_t menu8g2_get_index(menu8g2_t *menu){
    return menu->index;
}

QueueHandle_t menu8g2_get_input_queue(menu8g2_t *menu){
    return menu->input_queue;
}

SemaphoreHandle_t menu8g2_get_disp_mutex(menu8g2_t *menu){
    return menu->disp_mutex;
}

u8g2_t *menu8g2_get_u8g2(menu8g2_t *menu){
    return menu->u8g2;
}

uint8_t menu8g2_buf_header(menu8g2_t *menu, const char *title){
    /* Adds menu title and horizontal line underneith it to display buffer
     * returns the space took up. */
    uint8_t title_height; // Height of a menu item
    title_height = u8g2_GetAscent(menu->u8g2) - u8g2_GetDescent(menu->u8g2) 
            + CONFIG_MENU8G2_BORDER_SIZE;

    #if CONFIG_MENU8G2_HEADER_CENTER_JUST
    u8g2_DrawStr(menu->u8g2, menu8g2_get_center_x(menu, title), title_height,
            title);
    #endif
    #if CONFIG_MENU8G2_HEADER_LEFT_JUST
    u8g2_DrawStr(menu->u8g2, CONFIG_MENU8G2_BORDER_SIZE, title_height,
            title);
    #endif

    title_height += 1;
    u8g2_DrawHLine(menu->u8g2, 0, title_height, u8g2_GetDisplayWidth(menu->u8g2));
    return title_height;
}

/* Generic Vertical Scrolling Menu 
 * meta - some pointer to meta data to be used in function pointer
 * index_to_option*/
bool menu8g2_create_vertical_menu(menu8g2_t *menu,
        const char title[],
        void *meta,
        void (*index_to_option)(char buf[], const size_t buf_len,
             void *meta, const uint8_t index),
        const uint32_t max_lines
        ){
    uint8_t base_height; // Starting y value after title
    uint8_t item_height; // Height of a menu item
    uint8_t max_onscreen_items; // Maximum Number of onscreen menu items
    bool outcome;

    /* Plotting Variables */
    uint8_t element_y_pos; // variable for element positioning
    int32_t top_menu_element, old_top_menu_element;
    uint64_t input_buf; // holds the incoming button presses

    u8g2_SetFont(menu->u8g2, u8g2_font_profont12_tf);
    item_height = u8g2_GetAscent(menu->u8g2) - u8g2_GetDescent(menu->u8g2) + CONFIG_MENU8G2_BORDER_SIZE;

    base_height = item_height + 2 * CONFIG_MENU8G2_BORDER_SIZE;

    // Compute how many items can fit on the screen at once
    max_onscreen_items = (u8g2_GetDisplayHeight(menu->u8g2) - base_height)
            / (item_height+CONFIG_MENU8G2_BORDER_SIZE);

    // Allocate buffer
    char **buf = (char**) calloc(max_onscreen_items, sizeof(char*));
    for ( uint8_t i = 0; i < max_onscreen_items; i++ ){
        buf[i] = (char*) calloc(CONFIG_MENU8G2_LINE_BUFFER_LEN, sizeof(char));
    }
    top_menu_element = menu->index - (max_onscreen_items / 2);
    if(top_menu_element < 0){
        top_menu_element = 0;
    }
    // Populate buffer
    for(uint8_t i=0; i<max_onscreen_items; i++){
        // Add the cursor to the buffer
        uint8_t j = top_menu_element + i;
        if(j >= max_lines){
            break; // No more options to display
        }
        // Pad with spaces to be even with selection
        for(int k=0; k<strlen(CONFIG_MENU8G2_INDICATOR); k++){
            buf[i][k] = ' ';
        }

        element_y_pos += item_height + CONFIG_MENU8G2_BORDER_SIZE;
        (*index_to_option)(buf[i] + strlen(CONFIG_MENU8G2_INDICATOR),
                CONFIG_MENU8G2_LINE_BUFFER_LEN - strlen(CONFIG_MENU8G2_INDICATOR),
                meta, j);
    }

    for(;;){
        old_top_menu_element = menu->index - (max_onscreen_items / 2);
        if(old_top_menu_element < 0){
            old_top_menu_element = 0;
        }
        element_y_pos = base_height;

        MENU8G2_BEGIN_DRAW(menu)
            menu8g2_buf_header(menu, title);

            for(uint8_t i=0; i<max_onscreen_items; i++){
                uint8_t j = old_top_menu_element + i;
                element_y_pos += item_height + CONFIG_MENU8G2_BORDER_SIZE;
                u8g2_DrawStr(menu->u8g2, CONFIG_MENU8G2_BORDER_SIZE, element_y_pos, buf[i]);
                if(j == menu->index){
                    u8g2_DrawStr(menu->u8g2, CONFIG_MENU8G2_BORDER_SIZE, element_y_pos, CONFIG_MENU8G2_INDICATOR);
                }
            }
        MENU8G2_END_DRAW(menu)

        // Block until user inputs a button
        if(xQueueReceive(menu->input_queue, &input_buf, portMAX_DELAY)) {
            if(input_buf & (1ULL << EASY_INPUT_BACK)){
                outcome = false;
                goto exit;
            }
            else if(input_buf & (1ULL << EASY_INPUT_UP)){
                if(menu->index > 0){
                    menu->index--;
                    top_menu_element = menu->index - (max_onscreen_items / 2);
                    if(top_menu_element < 0){
                        top_menu_element = 0;
                    }
                    if(old_top_menu_element != top_menu_element){
                        for(uint8_t i=max_onscreen_items-1; i>0; i--){
                            strlcpy(buf[i], buf[i-1], CONFIG_MENU8G2_LINE_BUFFER_LEN);
                        }
                        (*index_to_option)(buf[0] + strlen(CONFIG_MENU8G2_INDICATOR),
                                CONFIG_MENU8G2_LINE_BUFFER_LEN - strlen(CONFIG_MENU8G2_INDICATOR),
                                meta, top_menu_element);
                    }
                }
            }
            else if(input_buf & (1ULL << EASY_INPUT_DOWN)){
                if(menu->index < max_lines - 1){
                    menu->index++;
                    top_menu_element = menu->index - (max_onscreen_items / 2);
                    if(top_menu_element < 0){
                        top_menu_element = 0;
                    }
                    if(old_top_menu_element != top_menu_element){
                        for(uint8_t i=0; i<max_onscreen_items-1; i++){
                            strcpy(buf[i], buf[i+1]);
                        }
                        if(top_menu_element+max_onscreen_items-1 < max_lines){
                            (*index_to_option)(buf[max_onscreen_items-1] + strlen(CONFIG_MENU8G2_INDICATOR),
                                    CONFIG_MENU8G2_LINE_BUFFER_LEN - strlen(CONFIG_MENU8G2_INDICATOR),
                                    meta, top_menu_element + max_onscreen_items -1);
                        }
                        else{
                            for(int k=0; k<strlen(CONFIG_MENU8G2_INDICATOR); k++){
                                buf[max_onscreen_items-1][k] = ' ';
                            }
                            buf[max_onscreen_items-1][strlen(CONFIG_MENU8G2_INDICATOR)] = '\0';
                        }
                    }
                }
            }
            else if(input_buf & (1ULL << EASY_INPUT_ENTER)){
                outcome = true;
                goto exit;
            }
        }
    }
    exit:
        for ( uint8_t i = 0; i < max_onscreen_items; i++ ){
            free(buf[i]);
        }
        free(buf);
        return outcome;
}

static menu8g2_err_t linear_string_selector(char buf[], const size_t buf_len, 
        const char *options[], const uint32_t index){
    /* Simple function that copies the string at index from options into the buf 
     * Used in menu8g2_create_simple as the index_to_option function*/
    strlcpy(buf, options[index], buf_len);
    return MENU8G2_SUCCESS;
}

bool menu8g2_create_simple(menu8g2_t *menu,
        const char title[],
        const char *options[],
        const uint32_t options_len
        ){
    return menu8g2_create_vertical_menu(menu, title, options,
            (void *) &linear_string_selector, options_len);
}

uint64_t menu8g2_display_text(menu8g2_t *menu, const char *text){
    /* Print text full screen */
    return menu8g2_display_text_title(menu, text, NULL);
}

uint64_t menu8g2_display_text_title(menu8g2_t *menu, const char *text, const char *title){
    /* Wraps text to fit on the dispaly; need to add scrolling */
    u8g2_SetFont(menu->u8g2, u8g2_font_profont12_tf);
    uint8_t item_height = u8g2_GetAscent(menu->u8g2) 
            - u8g2_GetDescent(menu->u8g2) 
            + CONFIG_MENU8G2_BORDER_SIZE;
    uint16_t char_per_line = u8g2_GetDisplayWidth(menu->u8g2) 
            / u8g2_GetMaxCharWidth(menu->u8g2);

    size_t buf_len;
    menu8g2_word_wrap(NULL, &buf_len, text, char_per_line);
    char *buf = calloc(buf_len, sizeof(char));
    menu8g2_word_wrap(buf, &buf_len, text, char_per_line);

    uint16_t line_start = 0;
    for(;;){
        uint16_t y_pos = item_height;
        bool more_text = false;
        uint16_t header_height = 0; 
        if(title){
            header_height = menu8g2_buf_header(menu, title);
            y_pos += header_height;
        }

        MENU8G2_BEGIN_DRAW(menu)
            menu8g2_buf_header(menu, title);
            more_text = menu8g2_draw_str(menu, 0, y_pos, buf, line_start);
        MENU8G2_END_DRAW(menu)

        // Block until user inputs a button
        uint64_t input_buf;
        if(xQueueReceive(menu->input_queue, &input_buf, portMAX_DELAY)) {
            if(input_buf & (1ULL << EASY_INPUT_UP)){
                if( line_start > 0 ){
                    line_start--;
                }
            }
            else if(input_buf & (1ULL << EASY_INPUT_DOWN)){
                if( more_text ){
                    line_start++;
                }
            }
            else {
                free( buf );
                return input_buf;
            }
        }
    }
    return 0; // Should never get here
}

void menu8g2_create_vertical_element_menu(menu8g2_t *menu,
        const char title[],
        menu8g2_elements_t *elements){
    uint8_t base_height; // Starting y value after title
    uint8_t item_height; // Height of a menu item
    uint8_t max_onscreen_items; // Maximum Number of onscreen menu items

    /* Plotting Variables */
    uint8_t element_y_pos; // variable for element positioning
    int8_t top_menu_element;
    uint8_t j;
    char buf[CONFIG_MENU8G2_LINE_BUFFER_LEN]; // buffer for printing strings
    uint8_t input_buf; // holds the incoming button presses

    assert(strlen(CONFIG_MENU8G2_INDICATOR) < sizeof(buf));

    u8g2_SetFont(menu->u8g2, u8g2_font_profont12_tf);
    item_height = u8g2_GetAscent(menu->u8g2) - u8g2_GetDescent(menu->u8g2) + CONFIG_MENU8G2_BORDER_SIZE;

    base_height = item_height + 2 * CONFIG_MENU8G2_BORDER_SIZE;

    // Compute how many items can fit on the screen at once
    max_onscreen_items = (u8g2_GetDisplayHeight(menu->u8g2) - base_height)
            / (item_height+CONFIG_MENU8G2_BORDER_SIZE);

    for(;;){
        MENU8G2_BEGIN_DRAW(menu)
            menu8g2_buf_header(menu, title);

            // figure out what list item should be drawn at the top
            element_y_pos = base_height;
            top_menu_element = menu->index - (max_onscreen_items / 2);
            if(top_menu_element < 0){
                top_menu_element = 0;
            }

            for(uint8_t i=0; i<max_onscreen_items; i++){
                // Add the cursor to the buffer
                j = top_menu_element + i;
                if(j >= elements->n){
                    break; // No more options to display
                }
                if(j == menu->index){
                    strlcpy(buf, CONFIG_MENU8G2_INDICATOR, sizeof(buf)); // Selector Indicator
                }
                else{
                    // Pad with spaces to be even with selection
                    for(int i=0; i<strlen(CONFIG_MENU8G2_INDICATOR); i++){
                        buf[i] = ' ';
                    }
                }
                element_y_pos += item_height + CONFIG_MENU8G2_BORDER_SIZE;
                strcpy(buf + strlen(CONFIG_MENU8G2_INDICATOR), (elements->elements)[j].name);
                u8g2_DrawStr(menu->u8g2, CONFIG_MENU8G2_BORDER_SIZE, element_y_pos, buf);
            }
        MENU8G2_END_DRAW(menu)

        // Block until user inputs a button
        if(xQueueReceive(menu->input_queue, &input_buf, portMAX_DELAY)) {
            if(input_buf & (0x01 << EASY_INPUT_BACK)){
                return;
            }
            else if(input_buf & (0x01 << EASY_INPUT_UP)){
                if(menu->index > 0){
                    menu->index--;
                }
            }
            else if(input_buf & (0x01 << EASY_INPUT_DOWN)){
                if(menu->index < elements->n - 1){
                    menu->index++;
                }
            }
            else if(input_buf & (0x01 << EASY_INPUT_ENTER)){
                if((elements->elements)[menu->index].menu_h){
                    (elements->elements)[menu->index].menu_h(menu);
                }
            }
        }
    }
}

void menu8g2_set_element(menu8g2_elements_t *elements, char *name, void *func){
    /* Warning: name must be a string literal pointer */
    elements->elements[elements->index].name = name;
    elements->elements[elements->index].menu_h = func;
    if(elements->index < elements->n - 1){
        (elements->index)++;
    }
}

void menu8g2_elements_init(menu8g2_elements_t *elements, uint32_t n){
    elements->index = 0;
    elements->n = n;
    elements->elements = calloc(n, sizeof(menu8g2_element_t));
}

void menu8g2_elements_free(menu8g2_elements_t *elements){
    free(elements->elements);
}
