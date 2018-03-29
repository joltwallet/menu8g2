/* Routines involving setting up and using physical buttons
 *
 */
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "soc/timer_group_struct.h"
#include "driver/timer.h"
#include "driver/gpio.h"

#include "buttons.h"
#include "menu8g2.h"

static void init_button_status(button_status_t *x, uint8_t pin){
    x->pin = pin;
    x->counter = 0;
    x->prev_state = true;
}

static bool button_periodic_update(button_status_t *button){
    // Button Debounce Logic
    if(!gpio_get_level(button->pin)){
        if(button->counter < BUTTON_DEBOUNCE_PERIOD){
            (button->counter)++;
        }
        else if(button->prev_state && \
                button->counter >= BUTTON_DEBOUNCE_PERIOD){
            button->prev_state = false;
            return true;
        }
    }
    else{
        if(button->counter > 0){
            button->counter--;
        }
        else{
            button->prev_state = true;
        }
    }
    return false;
}

TaskFunction_t vButtonDebounceTask( QueueHandle_t *input_queue ){
    // Timer Setup
    TickType_t xNextWakeTime = xTaskGetTickCount();

    // Declare buttons
    button_status_t back_status;
    button_status_t up_status;
    button_status_t down_status;
    button_status_t enter_status;

    // Initialize button debouncing structs
    init_button_status(&back_status, BACK_PIN);
    init_button_status(&up_status, UP_PIN);
    init_button_status(&down_status, DOWN_PIN);
    init_button_status(&enter_status, ENTER_PIN);

    // Send Variable
    unsigned char triggered_buttons = 0;
    for(;;){
        // This puts the task in a blocking state until polling period is up.
        // the task does not use any processing time while blocked
        vTaskDelayUntil(&xNextWakeTime, BUTTON_POLLING_PERIOD_MS);
        triggered_buttons = 0;

        triggered_buttons |= (button_periodic_update(&back_status) << LEFT);
        triggered_buttons |= (button_periodic_update(&up_status) << UP);
        triggered_buttons |= (button_periodic_update(&down_status) << DOWN);
        triggered_buttons |= (button_periodic_update(&enter_status) << ENTER);

        // If a debounced button is triggered, send it off to the queue
        if(triggered_buttons){
            xQueueSend(*input_queue, &triggered_buttons, 0);
        }
    }

    vTaskDelete(NULL); // Should never reach here!
}

void setup_buttons(){
    // Configures I/O, pull up/down, interrupts, etc.
    gpio_config_t config; // config object

    config.mode = GPIO_MODE_INPUT;
    config.pin_bit_mask = BACK_PIN_SEL | UP_PIN_SEL |
                           DOWN_PIN_SEL | ENTER_PIN_SEL;
    config.pull_up_en = 1;
    config.pull_down_en = 0;
    config.intr_type = GPIO_PIN_INTR_NEGEDGE;

    gpio_config(&config); // Apply Configurations

}
