
#include <Arduino.h>     // built in library
#include <WiFi.h>        // built in library
#include <ArduinoJson.h> // https://github.com/bblanchon/ArduinoJson
#include <HTTPClient.h>  // built in library
#include "esp_https_ota.h"

#define SSID "GatesNuovoOrdineMondiale"
#define PASSWORD "piramidediastana"
#define VERSION "0.0.9"

#define OTA_VERSION_REQUEST_URL "https://vaccinocovid19.live/get/ota_update"

extern const char server_cert_pem_start[] asm("_binary_server_certs_certs_pem_start");
extern const char server_cert_pem_end[] asm("_binary_server_certs_certs_pem_end");

WiFiClient client;

void setup()
{
  Serial.begin(115200);

  WiFi.begin(SSID, PASSWORD);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(100);
  };
  Serial.println("WiFi connected");

  HTTPClient http;
  int httpResponseCode;
  http.begin(OTA_VERSION_REQUEST_URL);
  httpResponseCode = http.GET();

  Serial.println();
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
      String ota_version = obj["version"];
      Serial.print("ota version: ");
      Serial.print(ota_version);
      Serial.println();
      // get remote version andcheck if the current version is different
      if (ota_version != VERSION)
      {
        // get .bin url and update for real
        String ota_url = obj["url"];

        Serial.print("ota url: ");
        Serial.println(ota_url);

        esp_http_client_config_t config;
        config.url = ota_url.c_str();
        config.cert_pem = server_cert_pem_start,

        Serial.println("Config set...");

        esp_err_t ret = esp_https_ota(&config);
        if (ret == ESP_OK)
        {
          // update successfull
          Serial.println("Update successfull!");
          ESP.restart();
        }
        else
        {
          Serial.println("Update failed.");
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