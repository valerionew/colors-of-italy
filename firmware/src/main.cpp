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
#include "CircularBuffer.h"
#include "Regioni.h"
#include "pacifica.h"
#include "LPF.h"
#include "Button.h"

#define DEBUG

WiFiManager wifiManager;
WiFiClient client;

uint8_t brightness_scale{0};
unsigned long last_connected{0};
unsigned long last_pressed{0};

bool showing{true};

LPF brightness_filter{0.05};

Button touch_minus(TOUCH_MINUS_PIN);
Button touch_reset(TOUCH_RESET_PIN);
Button touch_plus (TOUCH_PLUS_PIN);

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

// territory mapping
// 1 must be subtracted from the numbering of the regions for the array
// source https://github.com/pcm-dpc/COVID-19/blob/master/dati-regioni/dpc-covid19-ita-regioni-latest.csv

constexpr uint8_t NUMERO_REGIONI = 21;
Regioni region_map[NUMERO_REGIONI] {
  {NERO, &leds[3],  nullptr},  // 0, PIEMONTE
  {NERO, &leds[4],  nullptr},  // 1, VALLE D'AOSTA
  {NERO, &leds[5],  nullptr},  // 2, LOMBARDIA
  {NERO, &leds[6],  nullptr},  // 3, TRENTO
  {NERO, &leds[8],  nullptr},  // 4, VENETO
  {NERO, &leds[7],  nullptr},  // 5, FRIULI VENEZIA GIULIA
  {NERO, &leds[1],  &leds[2]}, // 6, LIGURIA
  {NERO, &leds[9],  nullptr},  // 7, EMILIA ROMAGNA
  {NERO, &leds[0],  nullptr},  // 8, TOSCANA
  {NERO, &leds[11], nullptr},  // 9, UMBRIA
  {NERO, &leds[10], nullptr},  // 10, MARCHE
  {NERO, &leds[12], nullptr},  // 11, LAZIO
  {NERO, &leds[14], nullptr},  // 12, ABRUZZO
  {NERO, &leds[15], nullptr},  // 13, MOLISE
  {NERO, &leds[16], nullptr},  // 14, CAMPANIA
  {NERO, &leds[17], &leds[18]},// 15, PUGLIA
  {NERO, &leds[19], nullptr},  // 16, BASILICATA
  {NERO, &leds[20], nullptr},  // 17, CALABRIA
  {NERO, &leds[21], nullptr},  // 18, SICILIA
  {NERO, &leds[13], nullptr},  // 19, SARDEGNA
  {NERO, nullptr,   nullptr},  // 20, BOLZANO
};

typedef CRGB SplashScreen[NUMERO_REGIONI];
/*struct SplashScreen{
  CRGB colors[NUMERO_REGIONI];
};*/

constexpr SplashScreen splashScreenItalia{
    VERDE, VERDE, VERDE, VERDE, VERDE, VERDE, VERDE, VERDE, VERDE,
    BIANCO, BIANCO, BIANCO, BIANCO, BIANCO,
    ROSSO, ROSSO, ROSSO, ROSSO, ROSSO, ROSSO, ROSSO,
};

void printSplashScreen(const SplashScreen &screen){
  for (int i = 0; i < NUMERO_REGIONI; i++){
    region_map[i].setColor(screen[i]);
  }
}

//---------------------------------SETUP----------------------------------------------
void setup()
{

#ifdef DEBUG
  Serial.begin(921600);
#endif

  Serial.println("-------SETUP-------");  


  // WS2812b initialization - actually they are 2813 mini
  // GRB = Color order depends on the Led model
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, LED_NUMBER);

  // Reset LEDs
  FastLED.clear();
  FastLED.show();

  printSplashScreen(splashScreenItalia);
  // Splashscreen Italia                  
  for(int i = 0; i < 10; i++) leds[i] = VERDE;
  for(int i = 10; i < 15; i++) leds[i] = BIANCO;
  for(int i = 15; i < LED_NUMBER; i++) leds[i] = ROSSO;
  FastLED.show();

  delay(3000);

  // Touch initialization
  touchSetCycles(0x1000, 0xA000);

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
    //float scaled_light = rescale(light, 2000, 0, 255, MIN_GLOBAL_BRIGHTENSS) + brightness_offset;
    //scaled_light = force(scaled_light, MIN_GLOBAL_BRIGHTENSS, 255);
    //byte brightness = (byte)brightness_filter.update(scaled_light);


#ifdef DEBUG
    Serial.print("ambient light ");
    Serial.print(light);
//    Serial.print(" scaled level ");
//    Serial.print(scaled_light);
    Serial.print(" led brightness ");
    Serial.println(brightness_scale);
#endif


    //for every region
    for(auto &region : region_map) 
    {
      region.setBrightness(brightness_scale);
    }

    FastLED.setBrightness(brightness_scale);
    FastLED.show();
  }

  touch_minus.update();
  if (touch_minus.is_pressed())
  {
#ifdef DEBUG
    static int count = 0;
    Serial.println(count++);
    Serial.println(" touch_minus is pressed");
#endif
    
    if (brightness_scale >= BRIGHTNESS_INCREMENT){
      brightness_scale -= BRIGHTNESS_INCREMENT;
    }else{
      brightness_scale = 0;
    }

#ifdef DEBUG
    Serial.println(brightness_scale);
#endif
    
  }

/*
  touch_reset.update();
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
*/

  touch_plus.update();
  if (touch_plus.is_pressed())
  {
#ifdef DEBUG
    static int count = 0;
    Serial.println(count++);
    Serial.println("touch_plus is pressed");
#endif

    if (255 - brightness_scale > BRIGHTNESS_INCREMENT){
      brightness_scale += BRIGHTNESS_INCREMENT;
    }else{
      brightness_scale = 255u;
    }
#ifdef DEBUG
    Serial.println(brightness_scale);
#endif
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

  delay(10);
}