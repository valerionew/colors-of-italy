#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager -use development branch to get it working on esp32
#include <ArduinoJson.h> // https://github.com/bblanchon/ArduinoJson
#include <FastLED.h>     // https://github.com/FastLED/FastLED
#include <HTTPClient.h>  // built in library
#include <map>           // built in library
#include <array>         // built in library
#include <fw_defines.h>
#include <pacifica.h>

WiFiManager wifiManager;
WiFiClient client;
CRGB leds[LED_NUMBER];
CRGB offset = 0xFFFF00; // color that gets blended with original one (color correction)

unsigned long last_update;
unsigned long last_pressed;
unsigned long last_refresh;

boolean wifi_connected;

// LPF
class LPF
{
public:
  void LFP()
  {
    alpha = 0;
    value = 0;
  };
  void init(float a)
  {
    alpha = a;
    value = 0;
  };
  float update(float sample)
  {
    value += (sample - value) * alpha;
    return value;
  };

private:
  float alpha;
  float value;
};
LPF brightness_filter;

// color mapping
// e.g. red -> color 0 -> 0xdd222a (red)
// initialization
std::map<String, unsigned long>
    color_map;

// region mapping
std::map<String, unsigned long> region_map;

// territory mapping
// ISTAT CODE -> led position translation
// e.g. code 01 -> addresses of led_map["01"]
// source https://www.agenziaentrate.gov.it/portale/Strumenti/Codici+attivita+e+tributo/F24+Codici+tributo+per+i+versamenti/Tabelle+dei+codici+tributo+e+altri+codici+per+il+modello+F24/Tabella+T0+codici+delle+Regioni+e+delle+Province+autonome
std::map<String, std::array<byte, MAX_LEDS_PER_REGION>>
    led_map = {
        {"01", {14, NO_LED}}, // ABRUZZO
        {"02", {19, NO_LED}}, // BASILICATA
        {"03", {NO_LED}},     // BOLZANO
        {"04", {20, NO_LED}}, // CALABRIA
        {"05", {16, NO_LED}}, // CAMPANIA
        {"06", {9, NO_LED}},  // EMILIA ROMAGNA
        {"07", {7, NO_LED}},  // FRIULI VENEZIA GIULIA
        {"08", {12, NO_LED}}, // LAZIO
        {"09", {1, 2}},       // LIGURIA
        {"10", {5, NO_LED}},  // LOMBARDIA
        {"11", {10, NO_LED}}, // MARCHE
        {"12", {15, NO_LED}}, // MOLISE
        {"13", {3, NO_LED}},  // PIEMONTE
        {"14", {17, 18}},     // PUGLIA
        {"15", {13, NO_LED}}, // SARDEGNA
        {"16", {21, NO_LED}}, // SICILIA
        {"17", {0, NO_LED}},  // TOSCANA
        {"18", {6, NO_LED}},  // TRENTO
        {"19", {11, NO_LED}}, // UMBRIA
        {"20", {4, NO_LED}},  // VALLE D'AOSTA
        {"21", {8, NO_LED}}   // VENETO
};

// x: 0->1
// return: 0->1
float easing(float x)
{
  // quadratic easing
  return 1 - (1 - x) * (1 - x);
}

float rescale(float value, float old_min, float old_max, float new_min, float new_max)
{
  // swap the interval if the scale is inverted
  if (old_min > old_max)
  {
    float temp = old_min;
    old_min = old_max;
    old_max = temp;
  }

  // check if the value is in range
  if (value > old_max)
    value = old_max;
  else if (value < old_min)
    value = old_min;

  return (value - old_min) * (new_max - new_min) / (old_max - old_min) + new_min;
}

// this function gets called when the parameters are set
void wifiParametersSet()
{
#ifdef DEBUG
  Serial.println("Wifi parameters set. Resetting ESP32.");
#endif
  ESP.restart();
}

void setup()
{

#ifdef DEBUG
  Serial.begin(115200);
#endif

  // PINs initialization
  pinMode(WIFI_RESET_BUTTON, INPUT_PULLUP);
  pinMode(LIGHT_SENSOR_PIN, INPUT);

  // variables initialization
  last_update = 0;
  last_pressed = 0;
  last_refresh = 0;
  // init brightness filter
  brightness_filter.init(0.05);

#ifdef DEBUG
  Serial.println(WiFi.status());
  //wifiManager.resetSettings();
#endif

  // set up WS2812b - in realtà sono 2813 mini
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, LED_NUMBER);

  // setup non blocking loop
  wifiManager.setHostname(WIFI_SSID_NAME);
  wifiManager.setConfigPortalBlocking(false);
  wifiManager.setSaveConfigCallback(wifiParametersSet);

  unsigned long wifiStarted = millis();
  if (!wifiManager.autoConnect(WIFI_SSID_NAME))
  {
    while (!wifi_connected)
    {
      // ConfigPortal is now running
      wifiManager.process();

      // handle led animation
      EVERY_N_MILLISECONDS(20)
      {
        pacifica_loop();
        FastLED.show();
      }

      // handle wifi reset button
      if (digitalRead(WIFI_RESET_BUTTON) == LOW)
      {
        if (last_pressed == 0)
        {
          last_pressed = millis();
        }
        else if (millis() - last_pressed > WIFI_RESET_TIMEOUT)
        {
          // delete wifi credentials and reset esp
          wifiManager.resetSettings();
          ESP.restart();
        }
      }
      else if (digitalRead(WIFI_RESET_BUTTON) == HIGH && last_pressed != 0)
      {
        // reset last pressed
        last_pressed = 0;
      }

      // handle wifi timeout
      if (millis() - wifiStarted > WIFI_MAX_TIME)
      {
        // it's taking too long to connect in
        // reset everything
        ESP.restart();
      }
    }
  }
}

void loop()
{
  // check if it's time to update
  if (last_update == 0 || millis() - last_update > UPDATE_INTERVAL)
  {
    // check if client is still connected
    // if not, set the esp in wifimanager again
    if (!client.connected())
    {
      if (!wifiManager.autoConnect(WIFI_SSID_NAME))
      {
        // blocking loop waiting for connection
        // this code gets called after timeout is hit
        ESP.restart();
      }
    }
    last_update = millis();

    HTTPClient http;
    int httpResponseCode;

    // get rgb colors
    http.begin(COLORS_REQUEST_URL);
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
        // no error in parsing
        JsonObject obj = doc.as<JsonObject>();
        for (JsonPair p : obj)
        {
          // iterate through each key and value
          // load color code
          String color_code = p.key().c_str();
          // load color value
          unsigned long color_hex = p.value().as<unsigned long>();
          // assing to map
          color_map[color_code] = color_hex;
#ifdef DEBUG
          Serial.print("color code ");
          Serial.print(color_code);
          Serial.print(" color hex ");
          Serial.println(color_hex);
#endif
        }
      }
    }

    // get territories color
    http.begin(TERRITORIES_REQUEST_URL);
    httpResponseCode = http.GET();
    if (httpResponseCode > 0)
    {
      StaticJsonDocument<512> doc;
      // buffer size calculated here: https://arduinojson.org/v6/assistant/
      String json_data = http.getString();
      // parse JSON data
      DeserializationError err = deserializeJson(doc, json_data);

      if (!err)
      {
        // no error in parsing
        JsonObject obj = doc.as<JsonObject>();
        for (JsonPair p : obj)
        {
          // iterate through each key and value
          // load color code
          String color_code = p.value().as<String>();
          // load region code
          String region_code = p.key().c_str();
          // load actual color
          unsigned long color = color_map.find(color_code)->second;
          // save color into map
          region_map[region_code] = color;

#ifdef DEBUG
          Serial.print("key ");
          Serial.print(p.key().c_str());
          Serial.print(" value ");
          Serial.print(p.value().as<byte>());
          Serial.print(" color code ");
          Serial.print(color_code);
          Serial.print(" color hex ");
          Serial.print(color, HEX);
          Serial.println();
#endif
        }
      }

      // free the memory
      doc.clear();
    }
    http.end();
  }

  // check if it's time to refresh
  if (last_refresh == 0 || millis() - last_refresh > REFRESH_INTERVAL)
  {
    // update last refreshed
    last_refresh = millis();
    // read light level from sensor
    unsigned int light = analogRead(LIGHT_SENSOR_PIN);
    // calculate the actual brightness compared to the sensor output
    float scaled_light = rescale(light, 2000, 0, 255, 10);
    byte brightness = (byte)brightness_filter.update(scaled_light);

#ifdef DEBUG
    Serial.print("ambient light ");
    Serial.print(light);
    Serial.print(" scaled level ");
    Serial.print(scaled_light);
    Serial.print(" led brightness ");
    Serial.println(brightness);
#endif

    for (auto region : region_map)
    {
      // load the list of addresses from the map
      std::array<byte, MAX_LEDS_PER_REGION> addresses = led_map.find(region.first)->second;
      for (const auto &address : addresses)
      {
        // color the corrisponding led
        if (address != NO_LED)
        {
          // the address must be different than the array filler

          // color correction
          CRGB blended;
          if (brightness <= MIN_BRIGHTNESS)
          {
            // calculate percent
            float percent = (float)brightness / MIN_BRIGHTNESS;
            // ease percent
            // we need to invert it (1-easing) in order to get 1 for low brightness values
            // and 0 for high brightness values, so that more color gets blended at lower
            // brightness to compensate the unbalancing of the leds
            float eased = (1 - easing(percent)) * MAX_BLEND;
            blended = blend(region.second, offset, eased);
          }
          else
          {
            // skip color correction if brightness if over the cutoff value
            blended = region.second;
          }
          leds[address] = region.second;
        }
      }
    }
    FastLED.setBrightness(brightness);
    FastLED.show();
  }

  if (digitalRead(WIFI_RESET_BUTTON) == LOW)
  {
    if (last_pressed == 0)
    {
      last_pressed = millis();
    }
    else if (millis() - last_pressed > WIFI_RESET_TIMEOUT)
    {
      // delete wifi credentials and reset esp
      wifiManager.resetSettings();
      ESP.restart();
    }
  }
  else if (digitalRead(WIFI_RESET_BUTTON) == HIGH && last_pressed != 0)
  {
    // reset last pressed
    last_pressed = 0;
  }

  delay(10);
}