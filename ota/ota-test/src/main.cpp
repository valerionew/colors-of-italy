
#include <Arduino.h>     // built in library
#include <WiFi.h>        // built in library
#include <ArduinoJson.h> // https://github.com/bblanchon/ArduinoJson
#include <HTTPClient.h>  // built in library
#include "certs.h"
#include "HttpsOTAUpdate.h"

// This sketch shows how to implement HTTPS firmware update Over The Air.
// Please provide your WiFi credentials, https URL to the firmware image and the server certificate.

static const int MAX_TRIES = 5;
static const char *VERSION = "0.0.9";
static const char *SSID = "GatesNuovoOrdineMondiale";                                      // your network SSID (name of wifi network)
static const char *PASSWORD = "piramidediastana";                                          // your network password
static const char *OTA_VERSION_REQUEST_URL = "https://vaccinocovid19.live/get/ota_update"; //state url of your firmware image

static HttpsOTAStatus_t otastatus;

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
  Serial.print("Attempting to connect to SSID: ");
  WiFi.begin(SSID, PASSWORD);

  // attempt to connect to Wifi network:
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(100);
  }

  Serial.print("Connected to ");
  Serial.println(SSID);

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
        const char *char_url = ota_url.c_str();

        Serial.print("ota url: ");
        Serial.println(ota_url);

        HttpsOTA.onHttpEvent(HttpEvent);
        Serial.println("Starting OTA");
        HttpsOTA.begin(char_url, CERT_PEM);

        Serial.println("Starting set...");

        int tries = 0;
        while (tries < MAX_TRIES)
        {
          otastatus = HttpsOTA.status();
          if (otastatus == HTTPS_OTA_SUCCESS)
          {
            Serial.println("Firmware written successfully. To reboot device, call API ESP.restart() or PUSH restart button on device");
            Serial.print("It took ");
            Serial.print(tries + 1);
            Serial.print(" tries");
            Serial.println();
            break;
          }
          else if (otastatus == HTTPS_OTA_FAIL)
          {
            Serial.println("Firmware Upgrade Fail");
            tries++;
            delay(100);
          }
        }
      }
    }

    doc.clear();
  }
  http.end();
}

void loop()
{
  Serial.println("What are we doing here?");
  Serial.println("Is this even updated?");
  delay(5000);
}