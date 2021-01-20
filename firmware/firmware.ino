#include <WiFi.h>
#include <WebServer.h>
#include <WiFiManager.h> // use development branch to get it working on esp32
#include <HTTPClient.h>

#define DEBUG

#define WIFI_TIMEOUT 500
#define WIFI_SSID_NAME "DCPM-map"
#define UPDATE_INTERVAL 3.6e6
#define REQUEST_URL "www.vaccinocovid19.live/get/colore_territori"
#define WIFI_RESET_BUTTON 3
#define WIFI_RESET_TIMEOUT 5000


WiFiManager wifiManager;
WiFiClient client;

unsigned long last_update;
unsigned long last_pressed;


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
    String json_data = http.getString();
    // IT'S JSON PARSE TIME!!!
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
