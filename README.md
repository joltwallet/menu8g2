# Menu8g2
A lightweight menuing library using the [`u8g2`](https://github.com/olikraus/u8g2) graphics library and [`easy_input`](https://github.com/joltwallet/easy_input) input library designed for the ESP32.

This library was developed with the 128x64 SSD1306 OLED display in mind, but may work fine with other screens.

# Dependencies:
The following components need to be in the build path:
* [U8G2](https://github.com/olikraus/u8g2)
* [easy_input](https://github.com/joltwallet/easy_input)

By default, you should clone these components into `$(IDF_PATH)/../third-party-components`

# Unit Tests
Unit tests can be used by selecting this library with a target using the [ESP32 Unit Tester](https://github.com/BrianPugh/esp32_unit_tester).

```
make flash TEST_COMPONENTS='menu8g2' monitor
```

The unit tests (in the `test` folder) is a good source of examples on how to use this library. The unit tests are not properly isolated and may need some tweaking to work with your development setup. This library was developed on the [Heltec WiFi_Kit_32](https://github.com/Heltec-Aaron-Lee/WiFi_Kit_series/tree/master/esp32/variants/wifi_kit_32). 

# Todo:
* Move button stuff in test/ to its own "inputs" component
* Make the button code a bit more general
* More Unit tests
* Clean up code a further
