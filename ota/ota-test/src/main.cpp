
#include <Arduino.h>     // built in library
#include <WiFi.h>        // built in library
#include <ArduinoJson.h> // https://github.com/bblanchon/ArduinoJson
#include <HTTPClient.h>  // built in library
#include "certs.h"
#include "HttpsOTAUpdate.h"

// This sketch shows how to implement HTTPS firmware update Over The Air.
// Please provide your WiFi credentials, https URL to the firmware image and the server certificate.

static const int MAX_TRIES = 5;
static const char *SSID = "GatesNuovoOrdineMondiale";                                      // your network SSID (name of wifi network)
static const char *PASSWORD = "piramidediastana";                                          // your network password
static const char *OTA_VERSION_REQUEST_URL = "https://vaccinocovid19.live/get/ota_update"; //state url of your firmware image
#define VERSION "2.0.6"

class Version
{
public:
  byte major, minor, build;

  Version()
  {
    major = 0;
    minor = 0;
    build = 0;
  };

  bool fromString(String to_parse)
  {
    unsigned int pos;
    pos = findPos(to_parse, sep, 0);
    major = to_parse.charAt(pos - 1) - 48;
    if (major < 0 || major > 9)
    {
      major = 0;
      return false;
    }

    pos = findPos(to_parse, sep, pos + 1);
    minor = to_parse.charAt(pos - 1) - 48;
    if (minor < 0 || minor > 9)
    {
      minor = 0;
      return false;
    }

    pos = findPos(to_parse, sep, pos + 1);
    build = to_parse.charAt(pos - 1) - 48;
    if (build < 0 || build > 9)
    {
      build = 0;
      return false;
    }

    return true;
  }

  void fromInt(int _major, int _minor, int _build)
  {
    major = _major;
    minor = _minor;
    build = _build;
  }

  void printString()
  {
    Serial.print(major);
    Serial.print(".");
    Serial.print(minor);
    Serial.print(".");
    Serial.print(build);
    Serial.println();
  }

  boolean newerThan(Version other)
  {
    if (major > other.major)
    {
      return true;
    }

    if (major == other.major)
    {
      if (minor > other.minor)
      {
        return true;
      }

      if (minor == other.minor && build > other.build)
      {
        return true;
      }
    }

    return false;
  }

  boolean olderThan(Version other)
  {
    return !newerThan(other);
  }

private:
  char sep = '.';

  unsigned int findPos(String str, char sep, byte start_pos)
  {
    for (unsigned int i = start_pos; i < str.length(); i++)
    {
      if (str.charAt(i) == sep)
      {
        return i;
      }
    }

    return -1;
  }
};

static HttpsOTAStatus_t otastatus;

static Version current_version;

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
  current_version.fromString(VERSION);

  Serial.begin(115200);
  Serial.println();
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
      String ota_version = obj["v1"]["version"];
      Version ota_version_obj;
      ota_version_obj.fromString(ota_version);

      Serial.print("Ota version: ");
      ota_version_obj.printString();
      Serial.print("Current version: ");
      current_version.printString();

      // get remote version andcheck if the current version is different
      if (ota_version_obj.newerThan(current_version))
      {
        Serial.print("Current version is older. Starting ota.");

        // get .bin url and update for real
        String ota_url = obj["v1"]["url"];
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