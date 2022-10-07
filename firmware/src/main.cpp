#include <Arduino.h>     // built in library
#include <WiFi.h>        // built in library
#include <WebServer.h>   // built in library
#include <HTTPClient.h>  // built in library
#include <numeric>       // built in library
#include <array>         // built in library
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <ArduinoJson.h> // https://github.com/bblanchon/ArduinoJson
#include <FastLED.h>     // https://github.com/FastLED/FastLED
#include "fw_defines.h"
#include "pacifica.h"

#define DEBUG

WiFiManager wifiManager;
WiFiClient client;
const CRGB OFFSET = 0xFFFF00; // color that gets blended with original one (color correction)

float brightness_offset{-40};
unsigned long last_connected{0};
unsigned long last_pressed{0};

bool showing{true};

// LPF class
class LPF
{
private:
  const float alpha;
  float value{0};

public:
  LPF(float _alpha):
    alpha(_alpha)
  {
  }

  void reset(float _value = 0)
  {
    value = _value;
  }

  float update(float sample)
  {
    value += (sample - value) * alpha;
    return value;
  }
};

// Circular Buffer class
template <size_t SIZE>
class CircularBuffer
{
 private:
  byte position{0};
  float memory[SIZE]{};
  float sum {0};

 public:
  void reset(float value = 0)
  {
    memset(memory, value, SIZE);
    position = 0;
    // initialize sum accordantly
    sum = value * SIZE;
  }

  float update(float value)
  {
    //update sum by removing old value, but adding the new one
    sum -= memory[position] + value;

    // add value to array and increment position
    memory[position++] = value;

    // if overflow, restart from 0
    if (position >= SIZE){
      position = 0;
    }

    // return memory sum
    return sum;
  }
};

// Button class
class Button
{

 private:
  const byte input_pin;
  const byte threshold;
  const byte outlier_threshold;

  float old_reading = read();
  bool pressed{false}, old_pressed{false}, rising{false};

  LPF low_pass{0.001}; // filter average
  CircularBuffer<10> buffer{}; // circular buffer - remember to change threshold accordingly
  
  float read()
  {
    return touchRead(input_pin);
  }

 public:
  Button(byte _input_pin, byte _threshold = 5, byte _outlier_threshold = 100):
    input_pin(_input_pin), threshold(_threshold), outlier_threshold(_outlier_threshold) // initialization list
  {
  }

  void update()
  {
    float reading = read();
    if (abs(reading - old_reading) < outlier_threshold)
    {
      //ignore corrupt samples
      float lp_filtered = low_pass.update(reading);              // first low pass filtering
      float box_filtered = buffer.update(lp_filtered - reading); // compare to average
      pressed = box_filtered > threshold;                        // thresholding
    }
    // update value
    old_reading = reading;
  }

  bool is_pressed()
  {
    // update rising state
    rising = pressed && !old_pressed;
    old_pressed = pressed;
    // return current pressure state

    return pressed;
  }

  bool first_press()
  {
    //is the button first press?
    return rising;
  }
};

LPF brightness_filter{0.05};

Button touch_minus(TOUCH_MINUS_PIN, 70);
Button touch_reset(TOUCH_RESET_PIN, 70);
Button touch_plus (TOUCH_PLUS_PIN , 70);

// x: 0->1
// return: 0->1
float easing(float x)
{
  // quadratic easing
  return 1 - (1 - x) * (1 - x);
}

// force a value into and interval
float force(float value, float min, float max)
{
  if (value > max)
    value = max;
  else if (value < min)
    value = min;

  return value;
}

// rescale a value to new interval
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

CRGB leds[LED_NUMBER];

class Regioni{
  byte brightness = 255;
  CRGB base_color = 0;
  CRGB* const led[2];
  public:
  Regioni(const CRGB _color, CRGB * const _led0, CRGB * const _led1):
    led{_led0, _led1}
  {
    setColor(_color);
  }

  void setColor(const CRGB color){
    base_color = color;
    updateColor();
  }

  void setBrightness(const byte _brightness){
    brightness = _brightness;
    updateColor();
  }

  void updateColor(){
    CRGB color = base_color;
    // color correction
    if (brightness <= BRIGHTNESS_BLEND_CUTOFF)
    {
      // calculate percent
      float percent = (float)brightness / BRIGHTNESS_BLEND_CUTOFF;
      // ease percent
      // we need to invert it (1-easing) in order to get 1 for low brightness values
      // and 0 for high brightness values, so that more color gets blended at lower
      // brightness to compensate the unbalancing of the leds
      float eased = (1 - easing(percent)) * MAX_BLEND;
      color = blend(base_color, OFFSET, eased);
    }
    for (auto &l : led)
      if (l) *l = color;
    
  }
};

// territory mapping
// 1 must be subtracted from the numbering of the regions for the array
// source https://github.com/pcm-dpc/COVID-19/blob/master/dati-regioni/dpc-covid19-ita-regioni-latest.csv
Regioni region_map[21] {
  {NERO,  &leds[3],  nullptr},  // 0, PIEMONTE
  {NERO,  &leds[4],  nullptr},  // 1, VALLE D'AOSTA
  {NERO,  &leds[5],  nullptr},  // 2, LOMBARDIA
  {NERO,  &leds[6],  nullptr},  // 3, TRENTO
  {NERO,  &leds[8],  nullptr},  // 4, VENETO
  {NERO,  &leds[7],  nullptr},  // 5, FRIULI VENEZIA GIULIA
  {NERO,  &leds[1],  &leds[2]}, // 6, LIGURIA
  {NERO,  &leds[9],  nullptr},  // 7, EMILIA ROMAGNA
  {NERO, &leds[0],  nullptr},  // 8, TOSCANA
  {NERO, &leds[11], nullptr},  // 9, UMBRIA
  {NERO, &leds[10], nullptr},  // 10, MARCHE
  {NERO, &leds[12], nullptr},  // 11, LAZIO
  {NERO, &leds[14], nullptr},  // 12, ABRUZZO
  {NERO, &leds[15], nullptr},  // 13, MOLISE
  {NERO,  &leds[16], nullptr},  // 14, CAMPANIA
  {NERO,  &leds[17], &leds[18]},// 15, PUGLIA
  {NERO,  &leds[19], nullptr},  // 16, BASILICATA
  {NERO,  &leds[20], nullptr},  // 17, CALABRIA
  {NERO,  &leds[21], nullptr},  // 18, SICILIA
  {NERO,  &leds[13], nullptr},  // 19, SARDEGNA
  {NERO,  nullptr,   nullptr},  // 20, BOLZANO
};

//---------------------------------SETUP----------------------------------------------
void setup()
{

#ifdef DEBUG
  Serial.begin(115200);
#endif

  //delay(3000);
  //Serial.println("-------SETUP-------");  


  // WS2812b initialization - actually they are 2813 mini
  // GRB = Color order depends on the Led model
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, LED_NUMBER);

  // Reset LEDs
  FastLED.clear();
  FastLED.show();

  // Splashscreen Italia                                            
  for(int i = 1; i < 10; i++) leds[i] = VERDE;
  for(int i = 10; i < 15; i++) leds[i] = BIANCO; leds[0] = BIANCO;
  for(int i = 15; i < LED_NUMBER; i++) leds[i] = ROSSO;
  FastLED.show();
  delay(3000);

  // Touch initialization
  touchSetCycles(0xA000, 0xA000);

  // PINs initialization
  pinMode(WIFI_RESET_BUTTON, INPUT_PULLUP);
  pinMode(LIGHT_SENSOR_PIN, INPUT);

#ifdef DEBUG
  Serial.println(WiFi.status());
#endif

  // setup non blocking loop
  wifiManager.setHostname(WIFI_SSID_NAME);
  wifiManager.setConfigPortalBlocking(false);
  wifiManager.setSaveConfigCallback(wifiParametersSet);

  unsigned long wifiStarted = millis();
  if (!wifiManager.autoConnect(WIFI_SSID_NAME))
  {
    while (1)
    {
      // ConfigPortal is now running
      wifiManager.process();

      // handle led animation
      EVERY_N_MILLISECONDS(0)
      {
        pacifica_loop();
        FastLED.show();
      }

      // check Reset Button
      if (digitalRead(WIFI_RESET_BUTTON) == LOW)
      {
        if (last_pressed == 0)
        {
          last_pressed = millis();
        }
        else if (millis() - last_pressed > WIFI_RESET_TIMEOUT)
        {
          // if the button is long pressed: delete WiFi credentials and reset esp
          wifiManager.resetSettings();
          ESP.restart();
        }
      }  
      else if (digitalRead(WIFI_RESET_BUTTON) == HIGH && last_pressed != 0)
      {
        // if the button is short pressed: only reset esp
        ESP.restart();
      }

      // handle WiFi timeout
      if (millis() - wifiStarted > WIFI_MAX_TIME)
      {
        // it's taking too long to connect in
        // reset everything
        #ifdef DEBUG
          Serial.println("WIFI TIMEOUT, resetting");
        #endif
        ESP.restart();
      }
    }
  }

  // now WiFi is connected
  last_connected = millis();


  // Blank screen
  FastLED.clear(); 
  FastLED.show();


  //Serial.println("-------Fine SETUP-------");  
}

//-----------------------------------LOOP-----------------------------------------------------
void loop()
{
  // check if it's time to update
  static unsigned long last_update_wifi = 0;
  if (last_update_wifi == 0 || millis() - last_update_wifi > UPDATE_INTERVAL)
  {
    // check if client is still connected
    // if not, if enough time has passed, reboot the esp (will be set in wifimanager mode again)
    if (WiFi.status() != WL_CONNECTED && millis() - last_connected > WIFI_MAX_UNCONNECTED)
    {
#ifdef DEBUG
      Serial.println("Cannot find wifi. Resetting");
#endif
      // reset LEDs (not really necessary)
      FastLED.clear();
      FastLED.show();
      // restart ESP
      ESP.restart();
    }

    if (WiFi.status() == WL_CONNECTED)
    {
#ifdef DEBUG
      Serial.println("Updating");
#endif
      last_update_wifi = millis();
      last_connected = millis();

      HTTPClient http;
      int httpResponseCode;

      // get rgb colors
      http.begin(COLORS_REQUEST_URL);
      httpResponseCode = http.GET();

      if (httpResponseCode > 0)
      {
        //StaticJsonDocument<256> doc;
        DynamicJsonDocument doc(16384);
        // buffer size calculated here: https://arduinojson.org/v6/assistant/
        String json_data = http.getString();

        // parse JSON data
        DeserializationError err = deserializeJson(doc, json_data);
        if (err) {
          Serial.print("deserializeJson() failed: ");
          Serial.println(err.f_str());
          return;
        }
        if (!err)
        {
          Serial.println("deserializeJson() passed: ");

          unsigned long trentinoSum = 0;

          unsigned long abitantiRegArr[20] = { INHABITANTS_REGIONS };
          
          unsigned long numberNorm;
          unsigned long numberNormArr[20];

          for (JsonObject item : doc.as<JsonArray>()) 
          {
            int region_code = item["codice_regione"];
            region_code--;
            unsigned long number = item["nuovi_positivi"];

            // sum P.A. Bolzano and P.A. Trento
            if(region_code == 20) {
              trentinoSum += number;
              continue;
            }
            if(region_code == 21) {
              number += trentinoSum;
              region_code = 3;
            }
            if(region_code > 21) {
              Serial.print("Too many region: ");
              Serial.println(region_code);
              continue;//ignore to avoid overflow
            }

            // Normalizzazione 100k abitanti (numero casi/popolazione)*100000
            unsigned long abitantiReg = abitantiRegArr[region_code];
            numberNorm = (number * 100000) / abitantiReg;

            //Array numeri normalizzati
            numberNormArr[region_code] = numberNorm;

#ifdef DEBUG           
            Serial.print(" region code ");
            Serial.print(region_code);
            Serial.print(" number ");
            Serial.println(number);
            Serial.print(" abitanti ");
            Serial.print(abitantiReg);
            Serial.print(" number norm ");
            Serial.println(numberNorm);
#endif 
          }

          // selezione min max numeri normalizzati
          unsigned long numberNorMax = numberNormArr[0];
          unsigned long numberNorMin = numberNormArr[0];
          for(int i = 0; i < sizeof(numberNormArr) / sizeof(numberNormArr)[0]; i++) {
              if (numberNormArr[i] > numberNorMax) {
                numberNorMax = numberNormArr[i];
              }
              if (numberNormArr[i] < numberNorMin) {
                numberNorMin = numberNormArr[i];
              }
          }

          // thresholds for color selection
          unsigned long mediana = numberNorMax - ((numberNorMax - numberNorMin) / 2);
          unsigned long medianaMax = numberNorMax - ((numberNorMax - mediana) / 2);
          unsigned long medianaMin = mediana - ((mediana - numberNorMin) / 2);

#ifdef DEBUG 
          Serial.print("max: "); Serial.println(numberNorMax);
          Serial.print("min: "); Serial.println(numberNorMin);
          Serial.print("mediana: "); Serial.println(mediana);
          Serial.print("medianaMax: "); Serial.println(medianaMax);
          Serial.print("medianaMin: "); Serial.println(medianaMin);
#endif 

          // color selection
          for(int i = 0; i < 20; i++) 
          {
            if (numberNormArr[i] > medianaMax) {
              //Serial.print(numberNormArr[i]); Serial.println(" medmax");  
              region_map[i].setColor(0xff0000); //rosso
            }
            else if ((numberNormArr[i] <= medianaMax) && (numberNormArr[i] > mediana)) {
              //Serial.print(numberNormArr[i]); Serial.println(" max-media"); 
              region_map[i].setColor(0xff5203); //arancio
            }
            else if ((numberNormArr[i] <= mediana) && (numberNormArr[i] > medianaMin)) {
              //Serial.print(numberNormArr[i]); Serial.println(" media-min"); 
              region_map[i].setColor(0xffd103); //giallo
            }            
            else if (numberNormArr[i] <= medianaMin) {
              //Serial.print(numberNormArr[i]); Serial.println(" medmin"); 
              region_map[i].setColor(0xffffff); //bianco
            }
          }
        }
        // free the memory
        doc.clear();
      }
      http.end();
    }
  }

  static unsigned long last_refresh{0};
  // check if it's time to refresh the leds
  if (showing && (millis() - last_refresh > REFRESH_INTERVAL))
  {
    // update last refreshed
    last_refresh = millis();
    // read light level from sensor
    unsigned int light = analogRead(LIGHT_SENSOR_PIN);
    // calculate the actual brightness compared to the sensor output
    float scaled_light = rescale(light, 2000, 0, 255, MIN_GLOBAL_BRIGHTENSS) + brightness_offset;
    scaled_light = force(scaled_light, MIN_GLOBAL_BRIGHTENSS, 255);
    byte brightness = (byte)brightness_filter.update(scaled_light);


/* #ifdef DEBUG
    Serial.print("ambient light ");
    Serial.print(light);
    Serial.print(" scaled level ");
    Serial.print(scaled_light);
    Serial.print(" led brightness ");
    Serial.println(brightness);
#endif */


    //for every region
    for(auto &region : region_map) 
    {
      region.setBrightness(brightness);
    }

    FastLED.setBrightness(brightness);
    FastLED.show();
  }

  // TEMPORARILY DISABLED
/*   // check touch buttons
  touch_minus.update();
  touch_reset.update();
  touch_plus.update();

  if (touch_minus.is_pressed() && touch_minus.first_press())
  {
#ifdef DEBUG
    Serial.println("touch_minus is pressed");
#endif
    if (showing)
    {
      brightness_offset -= BRIGHTNESS_INCREMENT;
      brightness_offset = force(brightness_offset, -255, 255);
    }

#ifdef DEBUG
    Serial.println(brightness_offset);
#endif
    
  }

  if (touch_reset.is_pressed() && touch_reset.first_press())
  {
#ifdef DEBUG
    Serial.println("touch_reset is pressed");
#endif

    // toggle showing flag
    showing = !showing;
    // if not showing, turn off all leds
    if (!showing)
    {
      FastLED.clear();
      FastLED.show();
    }

    Serial.print(showing);
    Serial.println();
  }

  if (touch_plus.is_pressed() && touch_plus.first_press())
  {
#ifdef DEBUG
    Serial.println("touch_plus is pressed");
#endif

    if (showing)
    {
      brightness_offset += BRIGHTNESS_INCREMENT;
      brightness_offset = force(brightness_offset, -255, 255);
    }
#ifdef DEBUG
    Serial.println(brightness_offset);
#endif

  } */

  // check Reset Button
  if (digitalRead(WIFI_RESET_BUTTON) == LOW)
  {
    if (last_pressed == 0)
    {
      last_pressed = millis();
    }
    else if (millis() - last_pressed > WIFI_RESET_TIMEOUT)
    {
      // if the button is long pressed: delete WiFi credentials and reset esp
      wifiManager.resetSettings();
      ESP.restart();
    }
  }  
  else if (digitalRead(WIFI_RESET_BUTTON) == HIGH && last_pressed != 0)
  {
    // if the button is short pressed: only reset esp
    ESP.restart();
  }

  delay(10);
}