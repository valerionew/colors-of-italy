#ifndef FW_DEFINES_H
#define FW_DEFINES_H

// led related
#define LED_NUMBER 22         // change this with the actual number of leds
#define MAX_LEDS_PER_REGION 2 // change this with the maximum number of led in each region
#define MIN_BRIGHTNESS 80     // led minimum brightness (brightness cutoff for led color blending) 0-255
#define MAX_BLEND 100         // max color blending (0-255)
#define LED_PIN 5             // pin connected to WS2812b data cable
#define NO_LED 255            // map filler

// wifi related
#define WIFI_SSID_NAME "Colors-of-italy"
#define WIFI_RESET_BUTTON 32
#define WIFI_RESET_TIMEOUT 5000
#define WIFI_MAX_TIME 600000

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
#define TOUCH_PLUS_PIN T3  // GPIO 4
#define TOUCH_RESET_PIN T2 // GPIO 2
#define TOUCH_MINUS_PIN T0 // GPIO 15

#endif