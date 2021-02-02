#include <WiFi.h>
#include <WebServer.h>
#include <WiFiManager.h> // use development branch to get it working on esp32
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <FastLED.h>
#include <map>

#define DEBUG

#define WIFI_TIMEOUT 500
#define WIFI_SSID_NAME "DCPM-map"
#define UPDATE_INTERVAL 3.6e6
#define REQUEST_URL "https://vaccinocovid19.live/get/colore_territori_slim"
#define WIFI_RESET_BUTTON 3
#define WIFI_RESET_TIMEOUT 5000
#define LED_PIN 12


WiFiManager wifiManager;
WiFiClient client;
CRGB leds[21];

unsigned long last_update;
unsigned long last_pressed;

// color mapping
// e.g. red -> color 0 -> 0xdd222a (red)
// colori scopiazzati dalle faq del ministero
std::map<unsigned short, unsigned long> color_map = {
  {0, 0xdd222a},
  {1, 0xe78314},
  {2, 0xf8c300},
  {3, 0xf7f7f7}
};

// territory mapping
// ISTAT CODE -> led position translation
// e.g. code 01 -> led with address led_map["01"]
// source https://www.agenziaentrate.gov.it/portale/Strumenti/Codici+attivita+e+tributo/F24+Codici+tributo+per+i+versamenti/Tabelle+dei+codici+tributo+e+altri+codici+per+il+modello+F24/Tabella+T0+codici+delle+Regioni+e+delle+Province+autonome

std::map<String, unsigned short> led_map = {
  {"01", 0},
  {"02", 1},
  {"03", 2},
  {"04", 3},
  {"05", 4},
  {"06", 5},
  {"07", 6},
  {"08", 7},
  {"09", 8},
  {"10", 9},
  {"11", 10},
  {"12", 11},
  {"13", 12},
  {"14", 13},
  {"15", 14},
  {"16", 15},
  {"17", 16},
  {"18", 17},
  {"19", 18},
  {"20", 19},
  {"21", 20}
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
  pinMode(WIFI_RESET_BUTTON, INPUT_PULLUP);
  last_update = 0;
  last_pressed = 0;

  #ifdef DEBUG
    Serial.println(WiFi.status());
  #endif

  // set up WS2812b
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, 21);
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
          // load the address by pairing the key (istat code) to the led address
          unsigned short address = led_map.find(p.key().c_str())->second;
          unsigned short color_code = p.value().as<unsigned short>();
          unsigned long color = color_map.find(color_code)->second;
          // color the corrisponding led
          leds[address] = color;

          #ifdef DEBUG
            Serial.print(p.key().c_str());
            Serial.print(" ");
            Serial.print(p.value().as<int>());
            Serial.print(" ");
            Serial.print(address);
            Serial.print(" ");
            Serial.print(color_code);
            Serial.print(" ");
            Serial.println(color);
          #endif
        }
      }

      // free the memory
      doc.clear();
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
