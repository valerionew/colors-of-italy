#include <WiFi.h>
#include <WebServer.h>
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager -use development branch to get it working on esp32
#include <ArduinoJson.h> // https://github.com/bblanchon/ArduinoJson
#include <FastLED.h>     // https://github.com/FastLED/FastLED
#include <HTTPClient.h>  // built in library
#include <map>           // built in library
#include <array>         // built in library

#define DEBUG

#define LED_NUMBER 22         // change this with the actual number of leds
#define MAX_LEDS_PER_REGION 2 // change this with the maximum number of led in each region
#define WIFI_TIMEOUT 500
#define WIFI_SSID_NAME "DCPM-map"
#define UPDATE_INTERVAL 10000
#define TERRITORIES_REQUEST_URL "https://vaccinocovid19.live/get/colore_territori_slim"
#define COLORS_REQUEST_URL "https://vaccinocovid19.live/get/colore_territori_rgb"
#define WIFI_RESET_BUTTON 32
#define WIFI_RESET_TIMEOUT 5000
#define WIFI_MAX_TIME 600000
#define LED_PIN 5           // pin connected to WS2812b data cable
#define LIGHT_SENSOR_PIN 33 // must be and ADC PIN, cannot use ADC2
#define NO_LED 255

#define TOUCH_PLUS_PIN T3  // GPIO 4
#define TOUCH_RESET_PIN T2 // GPIO 2
#define TOUCH_MINUS_PIN T0 // GPIO 15

WiFiManager wifiManager;
WiFiClient client;
CRGB leds[LED_NUMBER];

unsigned long last_update;
unsigned long last_pressed;

boolean wifi_connected;

// color mapping
// colors are fetched from internet
// initialization
std::map<String, unsigned long> color_map;

// brightness mapping
// ISTAT CODE -> led brightness translation
// 0 to 255 (darkse to brightest)
// initialization
std::map<String, byte> brightness_map = {
    {"01", 255}, // ABRUZZO
    {"02", 255}, // BASILICATA
    {"03", 255}, // BOLZANO
    {"04", 255}, // CALABRIA
    {"05", 255}, // CAMPANIA
    {"06", 255}, // EMILIA ROMAGNA
    {"07", 255}, // FRIULI VENEZIA GIULIA
    {"08", 255}, // LAZIO
    {"09", 255}, // LIGURIA
    {"10", 255}, // LOMBARDIA
    {"11", 255}, // MARCHE
    {"12", 255}, // MOLISE
    {"13", 255}, // PIEMONTE
    {"14", 255}, // PUGLIA
    {"15", 255}, // SARDEGNA
    {"16", 255}, // SICILIA
    {"17", 255}, // TOSCANA
    {"18", 255}, // TRENTO
    {"19", 255}, // UMBRIA
    {"20", 255}, // VALLE D'AOSTA
    {"21", 255}  // VENETO
};

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
      StaticJsonDocument<128> doc;
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
          // translate it to actual hex color
          unsigned long color = color_map.find(color_code)->second;
          // get leds brightness (unique for each territory)
          byte brightness = brightness_map.find(p.key().c_str())->second;
          // load the list of addresses from the map
          std::array<byte, MAX_LEDS_PER_REGION>
              addresses = led_map.find(p.key().c_str())->second;
          // loop throught each address
          for (const auto &address : addresses)
          {
            // the address must be different from the array filler
            if (address != NO_LED)
            {
              // color the corrisponding led
              leds[address] = color;
              // set its brightness
              leds[address].fadeToBlackBy(255 - brightness);
            }
          }

#ifdef DEBUG
          Serial.print(" led addresses ");
          for (const auto &address : addresses)
          {
            Serial.print(address);
            Serial.print(" ");
          }
          Serial.print("color code ");
          Serial.print(color_code);
          Serial.print(" color hex ");
          Serial.print(color);
          Serial.print(" brightness ");
          Serial.println(brightness);
#endif
        }
      }

      // free the memory
      doc.clear();
    }
    http.end();
  }
  // read light level from sensor
  unsigned int light = analogRead(LIGHT_SENSOR_PIN);
  // 500 should be the max value
  // this will need some tweaking
  if (light > 500)
    light = 500;
  // calculate the actual brightness compared to the sensor output
  byte brightness = map(light, 500, 0, 255, 20);
  // set the leds brightness
  FastLED.setBrightness(brightness);

  //      #ifdef DEBUG
  //        Serial.print("ambient light ");
  //        Serial.print(light);
  //        Serial.print(" led brightness ");
  //        Serial.println(brightness);
  //      #endif

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

  FastLED.show();
  delay(10);
}

//
//  "Pacifica"
//  Gentle, blue-green ocean waves.
//  December 2019, Mark Kriegsman and Mary Corey March.
//  For Dan.
//
CRGBPalette16 pacifica_palette_1 =
    {0x000507, 0x000409, 0x00030B, 0x00030D, 0x000210, 0x000212, 0x000114, 0x000117,
     0x000019, 0x00001C, 0x000026, 0x000031, 0x00003B, 0x000046, 0x14554B, 0x28AA50};
CRGBPalette16 pacifica_palette_2 =
    {0x000507, 0x000409, 0x00030B, 0x00030D, 0x000210, 0x000212, 0x000114, 0x000117,
     0x000019, 0x00001C, 0x000026, 0x000031, 0x00003B, 0x000046, 0x0C5F52, 0x19BE5F};
CRGBPalette16 pacifica_palette_3 =
    {0x000208, 0x00030E, 0x000514, 0x00061A, 0x000820, 0x000927, 0x000B2D, 0x000C33,
     0x000E39, 0x001040, 0x001450, 0x001860, 0x001C70, 0x002080, 0x1040BF, 0x2060FF};

void pacifica_loop()
{
  // Increment the four "color index start" counters, one for each wave layer.
  // Each is incremented at a different speed, and the speeds vary over time.
  static uint16_t sCIStart1, sCIStart2, sCIStart3, sCIStart4;
  static uint32_t sLastms = 0;
  uint32_t ms = GET_MILLIS();
  uint32_t deltams = ms - sLastms;
  sLastms = ms;
  uint16_t speedfactor1 = beatsin16(3, 179, 269);
  uint16_t speedfactor2 = beatsin16(4, 179, 269);
  uint32_t deltams1 = (deltams * speedfactor1) / 256;
  uint32_t deltams2 = (deltams * speedfactor2) / 256;
  uint32_t deltams21 = (deltams1 + deltams2) / 2;
  sCIStart1 += (deltams1 * beatsin88(1011, 10, 13));
  sCIStart2 -= (deltams21 * beatsin88(777, 8, 11));
  sCIStart3 -= (deltams1 * beatsin88(501, 5, 7));
  sCIStart4 -= (deltams2 * beatsin88(257, 4, 6));

  // Clear out the LED array to a dim background blue-green
  fill_solid(leds, LED_NUMBER, CRGB(2, 6, 10));

  // Render each of four layers, with different scales and speeds, that vary over time
  pacifica_one_layer(pacifica_palette_1, sCIStart1, beatsin16(3, 11 * 256, 14 * 256), beatsin8(10, 70, 130), 0 - beat16(301));
  pacifica_one_layer(pacifica_palette_2, sCIStart2, beatsin16(4, 6 * 256, 9 * 256), beatsin8(17, 40, 80), beat16(401));
  pacifica_one_layer(pacifica_palette_3, sCIStart3, 6 * 256, beatsin8(9, 10, 38), 0 - beat16(503));
  pacifica_one_layer(pacifica_palette_3, sCIStart4, 5 * 256, beatsin8(8, 10, 28), beat16(601));

  // Add brighter 'whitecaps' where the waves lines up more
  pacifica_add_whitecaps();

  // Deepen the blues and greens a bit
  pacifica_deepen_colors();
}

// Add one layer of waves into the led array
void pacifica_one_layer(CRGBPalette16 &p, uint16_t cistart, uint16_t wavescale, uint8_t bri, uint16_t ioff)
{
  uint16_t ci = cistart;
  uint16_t waveangle = ioff;
  uint16_t wavescale_half = (wavescale / 2) + 20;
  for (uint16_t i = 0; i < LED_NUMBER; i++)
  {
    waveangle += 250;
    uint16_t s16 = sin16(waveangle) + 32768;
    uint16_t cs = scale16(s16, wavescale_half) + wavescale_half;
    ci += cs;
    uint16_t sindex16 = sin16(ci) + 32768;
    uint8_t sindex8 = scale16(sindex16, 240);
    CRGB c = ColorFromPalette(p, sindex8, bri, LINEARBLEND);
    leds[i] += c;
  }
}

// Add extra 'white' to areas where the four layers of light have lined up brightly
void pacifica_add_whitecaps()
{
  uint8_t basethreshold = beatsin8(9, 55, 65);
  uint8_t wave = beat8(7);

  for (uint16_t i = 0; i < LED_NUMBER; i++)
  {
    uint8_t threshold = scale8(sin8(wave), 20) + basethreshold;
    wave += 7;
    uint8_t l = leds[i].getAverageLight();
    if (l > threshold)
    {
      uint8_t overage = l - threshold;
      uint8_t overage2 = qadd8(overage, overage);
      leds[i] += CRGB(overage, overage2, qadd8(overage2, overage2));
    }
  }
}

// Deepen the blues and greens
void pacifica_deepen_colors()
{
  for (uint16_t i = 0; i < LED_NUMBER; i++)
  {
    leds[i].blue = scale8(leds[i].blue, 145);
    leds[i].green = scale8(leds[i].green, 200);
    leds[i] |= CRGB(2, 5, 7);
  }
}
