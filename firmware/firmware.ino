#include <WiFi.h>
#include <WebServer.h>
#include <WiFiManager.h> // use development branch to get it working on esp32
#include <HTTPClient.h>  // built in library
#include <ArduinoJson.h> // https://github.com/bblanchon/ArduinoJson
#include <FastLED.h> // https://github.com/FastLED/FastLED
#include <map> // built in library
#include <array> // built in library

#define DEBUG

#define LED_NUMBER 35 // change this with the actual number of leds
#define MAX_LEDS_PER_REGION 4 // change this with the maximum number of led in each region
#define WIFI_TIMEOUT 500
#define WIFI_SSID_NAME "DCPM-map"
#define UPDATE_INTERVAL 3.6e6
#define REQUEST_URL "https://vaccinocovid19.live/get/colore_territori_slim"
#define WIFI_RESET_BUTTON 3
#define WIFI_RESET_TIMEOUT 5000
#define LED_PIN 12 // pin connected to WS2812b data cable
#define LIGHT_SENSOR_PIN 13 // must be and ADC PIN

WiFiManager wifiManager;
WiFiClient client;
CRGB leds[LED_NUMBER];

unsigned long last_update;
unsigned long last_pressed;

// color mapping
// e.g. red -> color 0 -> 0xdd222a (red)
// colori scopiazzati dalle faq del ministero lmao
std::map<byte, unsigned long> color_map = {
  {0, 0xdd222a},
  {1, 0xe78314},
  {2, 0xf8c300},
  {3, 0xf7f7f7}
};

// territory mapping -> change this with actual values and update the LED NUMBER constant. RANDOM VALUES PROVIDED AS NOW
// ISTAT CODE -> led position translation
// e.g. code 01 -> addresses of led_map["01"]
// source https://www.agenziaentrate.gov.it/portale/Strumenti/Codici+attivita+e+tributo/F24+Codici+tributo+per+i+versamenti/Tabelle+dei+codici+tributo+e+altri+codici+per+il+modello+F24/Tabella+T0+codici+delle+Regioni+e+delle+Province+autonome
std::map<String, std::array<byte, MAX_LEDS_PER_REGION>> led_map = {
  {"01", {0, 1, 2, 3}},
  {"02", {4, 5, 6}},
  {"03", {7}},
  {"04", {8}},
  {"05", {9, 10}},
  {"06", {11}},
  {"07", {12}},
  {"08", {13, 14, 15}},
  {"09", {17}},
  {"10", {18, 19}},
  {"11", {16}},
  {"12", {20, 21}},
  {"13", {22}},
  {"14", {23}},
  {"15", {24, 25, 26}},
  {"16", {27}},
  {"17", {28, 29}},
  {"18", {30, 31, 32}},
  {"19", {33}},
  {"20", {34}},
  {"21", {35}}
};

void setup() {

  #ifdef DEBUG
    Serial.begin(115200);
  #endif

  wifiManager.setTimeout(WIFI_TIMEOUT);
  // se qualcuno ha cazzi, si può personalizzare il codice HTML/CSS della pagina
  if (!wifiManager.autoConnect(WIFI_SSID_NAME)) {
    // blocking loop waiting for connection
    // this code gets called after timeout is hit
    ESP.restart();
  }
  // PINs initialization
  pinMode(WIFI_RESET_BUTTON, INPUT_PULLUP);
  pinMode(LIGHT_SENSOR_PIN, INPUT);

  // variables initialization
  last_update = 0;
  last_pressed = 0;

  #ifdef DEBUG
    Serial.println(WiFi.status());
  #endif

  // set up WS2812b
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, LED_NUMBER
  );
}

void loop() {
  // check if it's time to update
  if (last_update == 0 || millis() - last_update > UPDATE_INTERVAL) {
    // check if client is still connected
    // if not, set the esp in wifimanager again
    if (!client.connected()) {
      if (!wifiManager.autoConnect(WIFI_SSID_NAME)) {
        // blocking loop waiting for connection
        // this code gets called after timeout is hit
        ESP.restart();
      }
    }
    last_update = millis();

    HTTPClient http;
    http.begin(REQUEST_URL);

    int httpResponseCode = http.GET();
    if (httpResponseCode > 0) {
      StaticJsonDocument<512> doc;
      // buffer size calculated here: https://arduinojson.org/v6/assistant/
      String json_data = http.getString();
      // parse JSON data
      DeserializationError err = deserializeJson(doc, json_data);

      if (!err) {
        // no error in parsing
        JsonObject obj = doc.as<JsonObject>();
        for (JsonPair p : obj) {
          // iterate through each key and value
          // load color code
          byte color_code = p.value().as<byte>();
          // translate it to actual hex color
          unsigned long color = color_map.find(color_code)->second;
          // load the list of addresses from the map
          std::array<byte, MAX_LEDS_PER_REGION> addresses =  led_map.find(p.key().c_str())->second;
          for (const auto& address : addresses) {
            // color the corrisponding led
            leds[address] = color;
          }

          #ifdef DEBUG
            Serial.print("key ");
            Serial.print(p.key().c_str());
            Serial.print(" value ");
            Serial.print(p.value().as<byte>());
            Serial.print(" led addresses ");
            for (const auto& address : addresses) {
              Serial.print(address);
              Serial.print(" ");
            }
            Serial.print("color code ");
            Serial.print(color_code);
            Serial.print(" color hex ");
            Serial.println(color);
          #endif
        }
      }

      // free the memory
      doc.clear();

      // read light level from sensor
      unsigned int light = analogRead(LIGHT_SENSOR_PIN);
      // 500 should be the max value
      // this will need some tweaking
      if (light > 500) light = 500;
      // calculate the actual brightness compared to the sensor output
      byte brightness = map(light, 500, 0, 255, 50);
      // set the leds brightness
      FastLED.setBrightness(brightness);

      #ifdef DEBUG
        Serial.print("ambient light ");
        Serial.print(light);
        Serial.print(" led brightness ");
        Serial.println(brightness);
      #endif

    }
    http.end();

  }

  if (digitalRead(WIFI_RESET_BUTTON) == LOW) {
    if (last_pressed == 0) {
      last_pressed = millis();
    } else if (millis() - last_pressed > WIFI_RESET_TIMEOUT) {
      // delete wifi credentials and reset esp
      wifiManager.resetSettings();
      ESP.restart();
    }
  }

  delay(10);
}
