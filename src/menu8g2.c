#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#include "u8g2.h"
#include "menu8g2.h"
#include "easy_input.h"
#include "helpers.h"

menu8g2_err_t menu8g2_init(menu8g2_t *menu, u8g2_t *u8g2,
        QueueHandle_t *input_queue){
    menu->u8g2 = u8g2;
    menu->input_queue = input_queue;
    menu->index = 0;

    return E_SUCCESS;
}

menu8g2_err_t menu8g2_set_index(menu8g2_t *menu, const uint32_t index){
    menu->index = index;
    return E_SUCCESS;
}

uint32_t menu8g2_get_index(menu8g2_t *menu){
    return menu->index;
}

QueueHandle_t *menu8g2_get_input_queue(menu8g2_t *menu){
    return menu->input_queue;
}

u8g2_t *menu8g2_get_u8g2(menu8g2_t *menu){
    return menu->u8g2;
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
        u8g2_FirstPage(menu->u8g2);
        do{
            // Draw the menu title and horizontal line underneith it
            u8g2_DrawStr(menu->u8g2, get_center_x(menu->u8g2, title), item_height,
                    title);
            u8g2_DrawHLine(menu->u8g2, 0, item_height,
                    u8g2_GetDisplayWidth(menu->u8g2));

            // figure out what list item should be drawn at the top
            element_y_pos = base_height;
            top_menu_element = menu->index - (max_onscreen_items / 2);
            if(top_menu_element < 0){
                top_menu_element = 0;
            }

            for(uint8_t i=0; i<max_onscreen_items; i++){
                // Add the cursor to the buffer
                j = top_menu_element + i;
                if(j >= max_lines){
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
                (*index_to_option)(buf + strlen(CONFIG_MENU8G2_INDICATOR),
                        sizeof(buf) - strlen(CONFIG_MENU8G2_INDICATOR),
                        meta, j);
                u8g2_DrawStr(menu->u8g2, CONFIG_MENU8G2_BORDER_SIZE, element_y_pos, buf);
            }
        } while(u8g2_NextPage(menu->u8g2));

        // Block until user inputs a button
		if(xQueueReceive(menu->input_queue, &input_buf, portMAX_DELAY)) {
			if(input_buf & (0x01 << EASY_INPUT_BACK)){
                return false;
			}
			else if(input_buf & (0x01 << EASY_INPUT_UP)){
                if(menu->index > 0){
                    menu->index--;
                }
			}
			else if(input_buf & (0x01 << EASY_INPUT_DOWN)){
                if(menu->index < max_lines - 1){
                    menu->index++;
                }
			}
			else if(input_buf & (0x01 << EASY_INPUT_ENTER)){
                return true;
			}
		}
	}
	return false; // should never reach here, but just in case
}

static menu8g2_err_t linear_string_selector(char buf[], const size_t buf_len, 
        const char *options[], const uint32_t index){
    /* Simple function that copies the string at index from options into the buf 
     * Used in menu8g2_create_simple as the index_to_option function*/
    strlcpy(buf, options[index], buf_len);
    return E_SUCCESS;
}

bool menu8g2_create_simple(menu8g2_t *menu,
        const char title[],
        const char *options[],
        const uint32_t options_len
        ){
    return menu8g2_create_vertical_menu(menu, title, options,
            (void *) &linear_string_selector, options_len);
}

menu8g2_err_t menu8g2_display_text(menu8g2_t *menu, const char *text){
    /* Wraps text to fit on the dispaly; need to add scrolling */
    uint8_t item_height; // Height of a menu item
	uint8_t input_buf; // holds the incoming button presses

    u8g2_SetFont(menu->u8g2, u8g2_font_profont12_tf);
    item_height = u8g2_GetAscent(menu->u8g2) - u8g2_GetDescent(menu->u8g2) + CONFIG_MENU8G2_BORDER_SIZE;

    uint16_t str_len = strlen(text);
    uint16_t n_lines = 1 + ((str_len - 1) / CHAR_PER_LINE_WRAP);
    char buf[CHAR_PER_LINE_WRAP+1];

    u8g2_FirstPage(menu->u8g2);
    do{
        for(int i=0; i<n_lines; i++){
            strlcpy(buf, text+i*CHAR_PER_LINE_WRAP, sizeof(buf));
            buf[CHAR_PER_LINE_WRAP] = '\0';
            u8g2_DrawStr(menu->u8g2, 0, item_height + i*item_height, buf);
        }
    } while(u8g2_NextPage(menu->u8g2));

    // Block until user inputs a button
    for(;;){
        if(xQueueReceive(menu->input_queue, &input_buf, portMAX_DELAY)) {
            if(input_buf & (0x01 << EASY_INPUT_BACK)){
                return E_SUCCESS;
            }
            else if(input_buf & (0x01 << EASY_INPUT_UP)){
            }
            else if(input_buf & (0x01 << EASY_INPUT_DOWN)){
            }
            else if(input_buf & (0x01 << EASY_INPUT_ENTER)){
            }
        }
    }
    return E_FAILURE;
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
        u8g2_FirstPage(menu->u8g2);
        do{
            // Draw the menu title and horizontal line underneith it
            u8g2_DrawStr(menu->u8g2, get_center_x(menu->u8g2, title), item_height,
                    title);
            u8g2_DrawHLine(menu->u8g2, 0, item_height,
                    u8g2_GetDisplayWidth(menu->u8g2));

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
        } while(u8g2_NextPage(menu->u8g2));

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
