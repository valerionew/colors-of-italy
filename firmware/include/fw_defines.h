#ifndef FW_DEFINES_H
#define FW_DEFINES_H

// led related
#define LED_NUMBER 22              // change this with the actual number of leds
#define MAX_LEDS_PER_REGION 2      // change this with the maximum number of led in each region
#define BRIGHTNESS_BLEND_CUTOFF 80 // led minimum brightness (brightness cutoff for led color blending) 0-255
#define MAX_BLEND 100              // max color blending (0-255)
#define MIN_GLOBAL_BRIGHTENSS 20   // global minimum brightness (0-255)
#define LED_PIN 5                  // pin connected to WS2812b data cable
#define NO_LED 255                 // map filler

// wifi related
#define WIFI_SSID_NAME "Colors-of-italy" // SSID of the wifi portal
#define WIFI_RESET_BUTTON 32             // wifi reset button
#define WIFI_MAX_TIME 6e5                // timeout before the wifi portal (and the whole ESP) gets reset 6e5ms -> 10min
#define WIFI_MAX_UNCONNECTED 1.8e6       // how much time the ESP can go without being connected to the wifi 1.8e6ms -> 30 min
#define WIFI_RESET_TIMEOUT 5e3           // timeout for the reset button 500ms -> 5s

// routines related
#define UPDATE_INTERVAL 300e3 // 300s -> 5 min
#define REFRESH_INTERVAL 50   // 50ms -> 20fps

// URLS
#define TERRITORIES_REQUEST_URL "https://vaccinocovid19.live/get/colore_territori_slim"
#define COLORS_REQUEST_URL "https://vaccinocovid19.live/get/colore_territori_rgb"

// light sensor related
#define LIGHT_SENSOR_PIN 33 // must be and ADC PIN, cannot use ADC2
#define MAX_BRIGHTNESS 2000 // sensor max brightness read (min is 0)

// touch related
#define TOUCH_PLUS_PIN T3       // GPIO 4
#define TOUCH_RESET_PIN T2      // GPIO 2
#define TOUCH_MINUS_PIN T0      // GPIO 15
#define BRIGHTNESS_INCREMENT 35 // bright increment for plus/minus buttons

#endif