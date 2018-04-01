#ifndef __INCLUDE_BUTTONS_H__
#define __INCLUDE_BUTTONS_H__

    // I/O GPIO Definitions
	// NOTE: GPIO 6-11 are usually used for SPI Flash
	//       GPIO34-39 Can only be input and DO NOT have software pull up/down
    #define BACK_PIN 13
    #define UP_PIN 12
    #define DOWN_PIN 14
    #define ENTER_PIN 27

    // typedef enum{BACK, UP, DOWN, ENTER} button_id_t;

    // Convenience bit-shifted versions for masks
    #define BACK_PIN_SEL (1ULL<<BACK_PIN)
    #define UP_PIN_SEL (1ULL<<UP_PIN)	
    #define DOWN_PIN_SEL (1ULL<<DOWN_PIN)
    #define ENTER_PIN_SEL (1ULL<<ENTER_PIN)

	// Other Stuff
	#define ESP_INTR_FLAG_DEFAULT 0

    // Debounce Parameters
    #define BUTTON_POLLING_PERIOD_MS pdMS_TO_TICKS( 10 )
    #define BUTTON_DEBOUNCE_PERIOD 4

    // Struct to hold status for a button debounce
    typedef struct{
        uint8_t pin;
        uint8_t counter;
        bool prev_state;
    } button_status_t;
    
    void setup_buttons();

    TaskFunction_t vButtonDebounceTask( void *input_queue );

#endif
