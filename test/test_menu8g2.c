#include "unity.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <esp_system.h>

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#include "menu8g2.h"
#include "u8g2.h"
#include "u8g2_esp32_hal.h"
#include "esp32_1306.h"

u8g2_t u8g2;

bool display_initialized = false;

#define NUM_OF(x) (sizeof (x) / sizeof (*x))

/* Configure this to your test screen to see results */
static void setup_screen(u8g2_t *u8g2){
    if (display_initialized){
        return;
    }
    // Initialize OLED Screen I2C params
	u8g2_esp32_hal_t u8g2_esp32_hal = U8G2_ESP32_HAL_DEFAULT;
	u8g2_esp32_hal.sda  = SCREEN_PIN_SDA;
	u8g2_esp32_hal.scl  = SCREEN_PIN_SCL;
    u8g2_esp32_hal.reset = SCREEN_PIN_RESET;
	u8g2_esp32_hal_init(u8g2_esp32_hal);
   
	u8g2_Setup_ssd1306_i2c_128x64_noname_f(
		u8g2,
		U8G2_R0,
		u8g2_esp32_i2c_byte_cb,
		u8g2_esp32_gpio_and_delay_cb
    );  // init u8g2 structure

    // Note: SCREEN_ADDRESS is already shifted left by 1
	u8x8_SetI2CAddress(&(u8g2->u8x8), SCREEN_ADDRESS);

	u8g2_InitDisplay(u8g2);
	u8g2_SetPowerSave(u8g2, 0); // wake up display
    u8g2_ClearDisplay(u8g2);
	u8g2_ClearBuffer(u8g2);

    u8g2_SetContrast(u8g2, 255);

    display_initialized = true;
}

TEST_CASE("Basic Vertical Menu", "[menu8g2]"){
    setup_screen(&u8g2);
    QueueHandle_t input_queue;
    easy_input_queue_init(&input_queue);
    TaskHandle_t h_push_button = NULL;
    xTaskCreate(easy_input_push_button_task, \
            "ButtonDebounce", 2048,
            (void *)&input_queue, 20, \
            &h_push_button);

    const char title[] = "Basic Vertical Menu";
    const char *options[] = {
        "alpha",
        "beta",
        "gamma",
        "delta",
        "epsilon",
        "zeta",
        "eta",
        "theta",
        "iota"
    };

    menu8g2_t menu;
    menu8g2_init(&menu, &u8g2, input_queue);

    bool res = menu8g2_create_simple(&menu, title, options, 9);
    if(res==false){
        printf("Menu exited by pressing BACK.\n");
    }
    else{
        printf("Menu exited by pressing ENTER.\n");
    }
    vTaskDelete(h_push_button);
}

static menu8g2_err_t squarer(char buf[], size_t buf_len, const char *options[], const uint32_t index){
    sprintf(buf, "sqr: %d", index*index);
    return MENU8G2_SUCCESS;
}

TEST_CASE("On-The-Fly Vertical Menu", "[menu8g2]"){
    setup_screen(&u8g2);
    QueueHandle_t input_queue;
    easy_input_queue_init(&input_queue);
    TaskHandle_t h_push_button = NULL;
    xTaskCreate(easy_input_push_button_task, \
            "ButtonDebounce", 2048,
            (void *)&input_queue, 20, \
            &h_push_button);

    const char title[] = "OTF Vert Menu Max:20";
    menu8g2_t menu;
    menu8g2_init(&menu, &u8g2, input_queue);

    bool res = menu8g2_create_vertical_menu(&menu, title, NULL,
            (void *)&squarer, 20);

    if(res==false){
        printf("Menu exited by pressing BACK.\n");
    }
    else{
        printf("Menu exited by pressing ENTER.\n");
    }

    vTaskDelete(h_push_button);
}

static void animal_menu(menu8g2_t *prev){
    bool res;
    menu8g2_t menu;
    menu8g2_init(&menu,
            menu8g2_get_u8g2(prev),
            menu8g2_get_input_queue(prev));

    const char title[] = "Animal Menu";
    const char *options[] = {
        "Cat",
        "Dog",
        "Bird",
        "Fish",
        "Mouse",
        "Pig",
        "Cow",
        "Chicken",
        "Goat",
        "Turtle"
    };
    do{
        res = menu8g2_create_simple(&menu, title, options, 10);
        if(res==true){
            switch(menu8g2_get_index(&menu)){
                case 0:
                    menu8g2_display_text(&menu, "Meow");
                    break;
                case 1:
                    menu8g2_display_text(&menu, "Woof");
                    break;
                case 2:
                     menu8g2_display_text(&menu, "Chirp");
                     break;
                case 3:
                     menu8g2_display_text(&menu, "Blub");
                     break;
                case 4:
                     menu8g2_display_text(&menu, "Squeak");
                     break;
                case 5:
                     menu8g2_display_text(&menu, "Oink");
                     break;
                case 6:
                     menu8g2_display_text(&menu, "Mooooooooooooooooooooooooooooo");
                     break;
                case 7:
                     menu8g2_display_text(&menu, "Cock-a-doodle-doooooooooooooooo");
                     break;
                case 8:
                     menu8g2_display_text(&menu, "baaaaaaaaaa");
                     break;
                case 9:
                     menu8g2_display_text(&menu, "I like turtles");
                     break;
            }
        }
    }while(res==true);
    return;
}

static void crypto_menu(menu8g2_t *prev){
    bool res;
    menu8g2_t menu;
    menu8g2_init(&menu,
            menu8g2_get_u8g2(prev),
            menu8g2_get_input_queue(prev));

    const char title[] = "Crypto Menu";
    const char *options[] = {
        "Bitcoin",
        "Ethereum",
        "Monero",
        "Nano",
        "Iota",
        "NEO",
    };
    do{
        res = menu8g2_create_simple(&menu, title, options, 6);
        if(res==true){
            switch(menu8g2_get_index(&menu)){
                case 0:
                case 5:
                    menu8g2_display_text(&menu, "SHA256D, RIPEMD160");
                    break;
                case 1:
                    menu8g2_display_text(&menu, "Ethash");
                    break;
                case 2:
                     menu8g2_display_text(&menu, "CryptoNote");
                     break;
                case 3:
                     menu8g2_display_text(&menu, "Blake2b");
                     break;
                case 4:
                     menu8g2_display_text(&menu, "Curl");
                     break;
            }
        }
    }while(res==true);
    return;
}

TEST_CASE("Basic Stack Menu", "[menu8g2]"){
    setup_screen(&u8g2);
    QueueHandle_t input_queue;
    easy_input_queue_init(&input_queue);
    TaskHandle_t h_push_button = NULL;
    xTaskCreate(easy_input_push_button_task, \
            "ButtonDebounce", 2048,
            (void *)&input_queue, 20, \
            &h_push_button);

    const char title[] = "Basic Stack Menu";
    const char *options[] = {
        "Animals",
        "Cryptocurrencies"
    };

    menu8g2_t menu;
    menu8g2_init(&menu, &u8g2, input_queue);

    bool res;
    do{
        res = menu8g2_create_simple(&menu, title, options, 2);
        if(res==true){
            switch(menu8g2_get_index(&menu)){
                case 0:
                    animal_menu(&menu);
                    break;
                case 1:
                    crypto_menu(&menu);
                    break;
            }
        }
        else{
            printf("Menu exited by pressing BACK.\n");
        }

    }while(res == true);
    vTaskDelete(h_push_button);
}

TEST_CASE("Element Menu", "[menu8g2]"){
    setup_screen(&u8g2);
    QueueHandle_t input_queue;
    easy_input_queue_init(&input_queue);
    TaskHandle_t h_push_button = NULL;
    xTaskCreate(easy_input_push_button_task, \
            "ButtonDebounce", 2048,
            (void *)&input_queue, 20, \
            &h_push_button);

    menu8g2_t menu;
    menu8g2_init(&menu, &u8g2, input_queue);

    const char title[] = "Element Stack Menu";

    menu8g2_elements_t elements;
    menu8g2_elements_init(&elements, 2);
    menu8g2_set_element(&elements, "Animals", &animal_menu);
    menu8g2_set_element(&elements, "Cryptocurrencies", &crypto_menu);

    menu8g2_create_vertical_element_menu(&menu, title, &elements);

    menu8g2_elements_free(&elements);

    printf("Menu exited by pressing BACK.\n");

    vTaskDelete(h_push_button);
}

