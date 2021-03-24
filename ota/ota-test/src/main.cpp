
#include <Arduino.h>     // built in library
#include <WiFi.h>        // built in library
#include <ArduinoJson.h> // https://github.com/bblanchon/ArduinoJson
#include <HTTPClient.h>  // built in library
#include "certs.h"
#include "HttpsOTAUpdate.h"

#define SSID "GatesNuovoOrdineMondiale"
#define PASSWORD "piramidediastana"
#define VERSION "0.0.9"

#define OTA_VERSION_REQUEST_URL "https://vaccinocovid19.live/get/ota_update"

extern const char server_cert_pem_start[] asm("_binary_server_certs_certs_pem_start");
extern const char server_cert_pem_end[] asm("_binary_server_certs_certs_pem_end");

WiFiClient client;

void HttpEvent(HttpEvent_t *event)
{
  switch (event->event_id)
  {
  case HTTP_EVENT_ERROR:
    Serial.println("Http Event Error");
    break;
  case HTTP_EVENT_ON_CONNECTED:
    Serial.println("Http Event On Connected");
    break;
  case HTTP_EVENT_HEADER_SENT:
    Serial.println("Http Event Header Sent");
    break;
  case HTTP_EVENT_ON_HEADER:
    Serial.printf("Http Event On Header, key=%s, value=%s\n", event->header_key, event->header_value);
    break;
  case HTTP_EVENT_ON_DATA:
    break;
  case HTTP_EVENT_ON_FINISH:
    Serial.println("Http Event On Finish");
    break;
  case HTTP_EVENT_DISCONNECTED:
    Serial.println("Http Event Disconnected");
    break;
  }
}

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

        HttpsOTA.onHttpEvent(HttpEvent);
        Serial.println("Starting OTA");
        HttpsOTA.begin(ota_url, CERT_PEM);

        Serial.println("Config set...");

        otastatus = HttpsOTA.status();
        if (otastatus == HTTPS_OTA_SUCCESS)
        {
          Serial.println("Firmware written successfully. To reboot device, call API ESP.restart() or PUSH restart button on device");
        }
        else if (otastatus == HTTPS_OTA_FAIL)
        {
          Serial.println("Firmware Upgrade Fail");
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