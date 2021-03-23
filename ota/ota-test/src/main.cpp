
#include <Arduino.h>     // built in library
#include <WiFi.h>        // built in library
#include <ArduinoJson.h> // https://github.com/bblanchon/ArduinoJson
#include <HTTPClient.h>  // built in library
#include "esp_https_ota.h"
#include "cert_pem.h"

#define SSID "IL TUO SSID"
#define PASSWORD "LA PASSWORD DEL WIFI"
#define VERSION "0.0.9"

#define OTA_VERSION_REQUEST_URL "https://vaccinocovid19.live/get/ota_update"

WiFiClient client;

void setup()
{
  Serial.begin(115200);

  WiFi.begin(SSID, PASSWORD);

  while (WiFi.status() != WL_CONNECTED)
  {
    // wait for connection
  };

  Serial.println("WiFi connected");

  HTTPClient http;
  int httpResponseCode;
  http.begin(OTA_VERSION_REQUEST_URL);
  httpResponseCode = http.GET();
  if (httpResponseCode > 0)
  {
    StaticJsonDocument<256> doc;
    // buffer size calculated here: https://arduinojson.org/v6/assistant/
    String json_data = http.getString();
    // parse JSON data
    DeserializationError err = deserializeJson(doc, json_data);

    if (!err)
    {
      JsonObject obj = doc.as<JsonObject>();
      // get remote version andcheck if the current version is different
      if (obj["version"] != VERSION)
      {
        // get .bin url and update for real

        esp_http_client_config_t config;
        String ota_url = obj["url"];
        config.url = ota_url.c_str();
        config.cert_pem = CERT_PEM;

        esp_err_t ret = esp_https_ota(&config);
        if (ret == ESP_OK)
        {
          // update successfull
          ESP.restart();
        }
        else
        {
          // SOMETHING FAILED
        }
      }
    }

    doc.clear();
  }
  http.end();
}

void loop()
{
}