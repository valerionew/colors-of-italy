#include <WiFi.h>
#include <WebServer.h>
#include <WiFiManager.h>
#include <HTTPClient.h>

#define WIFI_TIMEOUT 500
#define WIFI_SSID_NAME "DCPM-map"
#define UPDATE_INTERVAL 3.6e6
#define REQUEST_URL "www.vaccinocovid19.live/get/colore_territori"

WiFiManager wifiManager;
WiFiClient client;

unsigned long last_update;

void setup() {
  wifiManager.setTimeout(WIFI_TIMEOUT);
  // se qualcuno ha cazzi, si può personalizzare il codice HTML/CSS della pagina
  if (!wifiManager.autoConnect(WIFI_SSID_NAME)) {
    // blocking loop waiting for connection
    // this code gets called after timeout is hit
    ESP.restart();
  }

  last_update = 0;
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
}
