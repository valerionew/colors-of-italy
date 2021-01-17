#include <WiFi.h>
#include <WebServer.h>
#include <WiFiManager.h>

#define WIFI_TIMEOUT 500
#define WIFI_SSID_NAME "DCPM-map"

void setup() {
  WiFiManager wifiManager;
  wifiManager.setTimeout(WIFI_TIMEOUT);
  // se qualcuno ha cazzi, si può personalizzare il codice HTML/CSS della pagina
  if (!wifiManager.autoConnect(WIFI_SSID_NAME)) {
    // blocking loop waiting for connection
    // this code gets called after timeout is hit
    ESP.restart();
  }
}

void loop() {

}
