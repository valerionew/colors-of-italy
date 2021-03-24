
#include <Arduino.h>     // built in library
#include <WiFi.h>        // built in library
#include <ArduinoJson.h> // https://github.com/bblanchon/ArduinoJson
#include <HTTPClient.h>  // built in library
#include "certs.h"
#include "HttpsOTAUpdate.h"

// This sketch shows how to implement HTTPS firmware update Over The Air.
// Please provide your WiFi credentials, https URL to the firmware image and the server certificate.

static const uint8_t MAX_TRIES = 5;
static const char *SSID = "GatesNuovoOrdineMondiale";                                      // your network SSID (name of wifi network)
static const char *PASSWORD = "piramidediastana";                                          // your network password
static const char *OTA_VERSION_REQUEST_URL = "https://vaccinocovid19.live/get/ota_update"; //state url of your firmware image
static const char *VERSION = "1.0.0";

class Version
{
public:
  uint16_t major, minor, build, patch;

  Version()
  {
    major = 0;
    minor = 0;
    build = 0;
    patch = 0;
  };

  bool fromString(String _to_parse)
  {
    to_parse = _to_parse;

    uint16_t pos;

    pos = parseValue(0, &major) + 1;
    if (pos == -1 || pos == to_parse.length() - 1)
      return false;
    pos = parseValue(pos, &minor) + 1;
    if (pos == -1 || pos == to_parse.length() - 1)
      return true;
    pos = parseValue(pos, &build) + 1;
    if (pos == -1 || pos == to_parse.length() - 1)
      return true;
    pos = parseValue(pos, &patch) + 1;

    return true;
  }

  void fromInt(uint16_t _major = 0, uint16_t _minor = 0, uint16_t _build = 0, uint16_t _patch = 0)
  {
    major = _major;
    minor = _minor;
    build = _build;
    patch = _patch;
  }

  void printString()
  {
    Serial.print(major);
    Serial.print(".");
    Serial.print(minor);
    Serial.print(".");
    Serial.print(build);
    Serial.print(".");
    Serial.print(patch);
    Serial.println();
  }

  boolean newerThan(Version other)
  {
    // check if this version is newer than the other version

    // compare major
    if (major > other.major)
    {
      return true;
    }
    // if major is the same, compare minor
    if (minor > other.minor)
    {
      return true;
    }
    // if minor is the same, compare build
    if (build > other.build)
    {
      return true;
    }
    // if build is the same, compare patch
    if (patch > other.patch)
    {
      return true;
    }

    return false;
  }

  boolean
  asOldAs(Version other)
  {
    // check if this version is as old as the other version
    return major == other.major && minor == other.minor && build == other.build && patch == other.patch;
  }

  boolean olderThan(Version other)
  {
    // check if this version is older than the other version
    return !newerThan(other) && !asOldAs(other);
  }

private:
  char sep = '.';
  String to_parse;

  uint16_t parseValue(uint8_t start_pos, uint16_t *dest)
  {
    // set destination value to be null
    *dest = 0;
    // start looping throught the string
    for (uint16_t i = start_pos; i < to_parse.length(); i++)
    {
      if (to_parse.charAt(i) == sep)
      {
        // if the current char is a separator, return the pos
        return i;
      }
      else
      {
        // otherwise, attempt to get the number
        uint8_t new_digit;
        new_digit = to_parse.charAt(i) - 48;
        if (new_digit < 0 || new_digit > 9)
        {
          // if we hit something that's not a number, then return it
          return i;
        }
        // current number is 10 times the old number plus the new number
        *dest = *dest * 10 + new_digit;
      }
    }

    // we hit the end, let's just return -1
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

  Serial.begin(115200);
  Serial.println();

  current_version.fromString(VERSION);

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
  uint16_t httpResponseCode;
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

        uint8_t tries = 0;
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