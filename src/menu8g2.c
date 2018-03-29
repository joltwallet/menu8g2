#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#include "u8g2.h"
#include "menu8g2.h"
#include "helpers.h"

menu8g2_err_t menu8g2_init(menu8g2_t *menu, u8g2_t *u8g2,
        QueueHandle_t *input_queue){
    menu->prev = NULL;
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

menu8g2_err_t menu8g2_set_prev(menu8g2_t *menu, menu8g2_t *prev){
    menu->prev = prev;
    return E_SUCCESS;
}

#define BORDER_SIZE 1
#define LINE_BUFFER_SIZE 80
const char MENU8G2_INDICATOR[] = " > ";

/* Generic Vertical Scrolling Menu 
 * meta - some pointer to meta data to be used in function pointer
 * index_to_option*/
bool menu8g2_create_vertical_menu(menu8g2_t *menu,
        const char title[],
        const void *meta,
        const void (*index_to_option)(char buf[], const void *meta, const uint8_t index),
        const uint32_t max_lines
        ){
    uint8_t base_height; // Starting y value after title
    uint8_t item_height; // Height of a menu item
    uint8_t max_onscreen_items; // Maximum Number of onscreen menu items

    /* Plotting Variables */
    uint8_t element_y_pos; // variable for element positioning
    int8_t top_menu_element;
    uint8_t j;
    char buf[LINE_BUFFER_SIZE]; // buffer for printing strings
	uint8_t input_buf; // holds the incoming button presses

    u8g2_SetFont(menu->u8g2, u8g2_font_profont12_tf);
    item_height = u8g2_GetAscent(menu->u8g2) - u8g2_GetDescent(menu->u8g2) + BORDER_SIZE;

    base_height = item_height + 2 * BORDER_SIZE;

    // Compute how many items can fit on the screen at once
    max_onscreen_items = (u8g2_GetDisplayHeight(menu->u8g2) - base_height)
            / (item_height+BORDER_SIZE);

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
                    strcpy(buf, MENU8G2_INDICATOR); // Selector Indicator
                }
                else{
                    // Pad with spaces to be even with selection
                    for(int i=0; i<strlen(MENU8G2_INDICATOR); i++){
                        buf[i] = ' ';
                    }
                }
                element_y_pos += item_height + BORDER_SIZE;
                (*index_to_option)(buf + strlen(MENU8G2_INDICATOR), meta, j);
                u8g2_DrawStr(menu->u8g2, BORDER_SIZE, element_y_pos, buf);
            }
        } while(u8g2_NextPage(menu->u8g2));

        // Block until user inputs a button
		if(xQueueReceive(menu->input_queue, &input_buf, portMAX_DELAY)) {
			if(input_buf & (0x01 << LEFT)){
                return false;
			}
			else if(input_buf & (0x01 << UP)){
                if(menu->index > 0){
                    menu->index--;
                }
			}
			else if(input_buf & (0x01 << DOWN)){
                if(menu->index < max_lines - 1){
                    menu->index++;
                }
			}
			else if(input_buf & (0x01 << ENTER)){
                return true;
			}
		}
	}
	return false; // should never reach here, but just in case
}

static menu8g2_err_t linear_string_selector(char buf[], const char *options[], const uint32_t index){
    /* Simple function that copies the string at index from options into the buf 
     * Used in menu8g2_create_simple as the index_to_option function*/
    strcpy(buf, options[index]);
    return E_SUCCESS;
}

bool menu8g2_create_simple(menu8g2_t *menu,
        const char title[],
        const char *options[],
        const uint32_t options_len
        ){
    return menu8g2_create_vertical_menu(menu, title, options,
            &linear_string_selector, options_len);
}

