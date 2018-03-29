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

#include "buttons.h"

/* Configure this to your test screen to see results */
static void setup_screen(u8g2_t *u8g2){
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
}

TEST_CASE("Basic Vertical Menu", "[menu8g2]"){
    // Create Screen Object
    const char title[] = "Test App Title";
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
    u8g2_t u8g2;
    setup_screen(&u8g2);

    setup_buttons();
    QueueHandle_t input_queue = xQueueCreate(5, sizeof(char));
    xTaskCreate(vButtonDebounceTask, "ButtonDebounce", 1024,
            (void *)&input_queue, 20, NULL);

    menu8g2_t menu;
    menu8g2_init(&menu, &u8g2, input_queue);

    bool res = menu8g2_create_simple(&menu, title, options, 9);
}

