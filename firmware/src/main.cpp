#include <Arduino.h>     // built in library
#include <WiFi.h>        // built in library
#include <WebServer.h>   // built in library
#include <HTTPClient.h>  // built in library
#include <map>           // built in library
#include <array>         // built in library
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <ArduinoJson.h> // https://github.com/bblanchon/ArduinoJson
#include <FastLED.h>     // https://github.com/FastLED/FastLED
#include <fw_defines.h>
#include <pacifica.h>

#define DEBUG



WiFiManager wifiManager;
WiFiClient client;
CRGB leds[LED_NUMBER];
CRGB offset = 0xFFFF00; // color that gets blended with original one (color correction)

float brightness_offset;
unsigned long last_update;
unsigned long last_connected;
unsigned long last_pressed;
unsigned long last_refresh;

boolean wifi_connected;
boolean showing;

// LPF class
class LPF
{
private:
  float alpha;
  float value;

public:
  void LFP()
  {
    alpha = 0;
    value = 0;
  };
  void init(float a, float _value = 0)
  {
    alpha = a;
    value = _value;
  };
  float update(float sample)
  {
    value += (sample - value) * alpha;
    return value;
  };
};

// Circular Buffer class
class CircularBuffer
{
private:
  byte position, size;
  float *memory;

public:
  void init(byte _size)
  {
    size = _size;
    // init array
    memory = new float[size];
    // set every value to zero
    for (byte i = 0; i < size; i++)
    {
      memory[i] = 0;
    }
    position = 0;
  }

  float update(float value)
  {
    // add value to array
    memory[position] = value;
    // move current position
    position = (position + 1) % size;

    // compute and return memory sum
    float sum;
    sum = 0;
    for (byte i = 0; i < size; i++)
    {
      sum += memory[i];
    }

    return sum;
  }
};

// Button class
class Button
{

private:
  byte input_pin;
  byte threshold;
  byte outlier_threshold;
  float old_reading;
  bool pressed, old_pressed, rising;

  LPF low_pass;
  CircularBuffer buffer;
  
  float read()
  {
    return touchRead(input_pin);
  }

public:
  Button(byte _input_pin, byte _threshold = 5, byte _outlier_threshold = 100)
  {
    input_pin = _input_pin;
    threshold = _threshold;
    outlier_threshold = _outlier_threshold;
  }

  void init()
  {
    old_reading = read();
    pressed = false;
    old_pressed = false;

    low_pass.init(0.001, old_reading); // filter average
    buffer.init(10);                   // circular buffer - remember to change threshold accordingly
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

LPF brightness_filter;
Button touch_minus(TOUCH_MINUS_PIN, 70);
Button touch_reset(TOUCH_RESET_PIN, 70);
Button touch_plus (TOUCH_PLUS_PIN , 70);


// region mapping
std::map<String, unsigned long> 
    region_map;


// territory mapping
// 1 must be subtracted from the numbering of the regions for the array
// source https://github.com/pcm-dpc/COVID-19/blob/master/dati-regioni/dpc-covid19-ita-regioni-latest.csv
std::map<String, std::array<byte, MAX_LEDS_PER_REGION>>
    led_map = {
        {"12", {14, NO_LED}}, // ABRUZZO
        {"16", {19, NO_LED}}, // BASILICATA
        {"20", {NO_LED}},     // BOLZANO
        {"17", {20, NO_LED}}, // CALABRIA
        {"14", {16, NO_LED}}, // CAMPANIA
        {"7", {9, NO_LED}},  // EMILIA ROMAGNA
        {"5", {7, NO_LED}},  // FRIULI VENEZIA GIULIA
        {"11", {12, NO_LED}}, // LAZIO
        {"6", {1, 2}},       // LIGURIA
        {"2", {5, NO_LED}},  // LOMBARDIA
        {"10", {10, NO_LED}}, // MARCHE
        {"13", {15, NO_LED}}, // MOLISE
        {"0", {3, NO_LED}},  // PIEMONTE
        {"15", {17, 18}},     // PUGLIA
        {"19", {13, NO_LED}}, // SARDEGNA
        {"18", {21, NO_LED}}, // SICILIA
        {"8", {0, NO_LED}},  // TOSCANA
        {"3", {6, NO_LED}},  // TRENTO
        {"9", {11, NO_LED}}, // UMBRIA
        {"1", {4, NO_LED}},  // VALLE D'AOSTA
        {"4", {8, NO_LED}}   // VENETO
};


// force a value into and interval
float force(float value, float min, float max)
{
  if (value > max)
    value = max;
  else if (value < min)
    value = min;

  return value;
}

// x: 0->1
// return: 0->1
float easing(float x)
{
  // quadratic easing
  return 1 - (1 - x) * (1 - x);
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
  for(int i = 1; i < 10; i++) leds[i] = 0x008c45;                       //Verde: pantone 17-6153 
  for(int i = 10; i < 15; i++) leds[i] = 0xf4f5f0; leds[0] = 0xf4f5f0;  //Bianco: pantone 11-0601
  for(int i = 15; i < LED_NUMBER; i++) leds[i] = 0xcd212a;              //Rosso: pantone 18-1662
  FastLED.show();
  delay(3000);

  // Touch initialization
  touchSetCycles(0xA000, 0xA000);
  touch_minus.init();
  touch_reset.init();
  touch_plus.init();

  // PINs initialization
  pinMode(WIFI_RESET_BUTTON, INPUT_PULLUP);
  pinMode(LIGHT_SENSOR_PIN, INPUT);

  // variables initialization
  last_update = 0;
  last_pressed = 0;
  last_refresh = 0;
  brightness_offset = -40;
  showing = true;

  // init brightness filter
  brightness_filter.init(0.05);

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
  for(int i = 0; i < LED_NUMBER; i++) leds[i] = 0x000000;
  FastLED.show();


  //Serial.println("-------Fine SETUP-------");  
}




//-----------------------------------LOOP-----------------------------------------------------
void loop()
{
  // check if it's time to update
  if (last_update == 0 || millis() - last_update > UPDATE_INTERVAL)
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
      last_update = millis();
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

          //dati abitanti Regioni http://dati.istat.it/Index.aspx?DataSetCode=DCIS_POPRES1
          unsigned long abitantiRegArr[20] = 
          { 4252279, 123337, 9965046, 1077932, 4854633, 1197295, 1507438, 4431816, 3676285, 859572, 1489789, 5715190, 1273660, 290769, 5590681, 3912166, 539999, 1844586, 4801468, 1579181 };
          
          unsigned long numberNorm;
          unsigned long numberNormArr[20];

          for (JsonObject item : doc.as<JsonArray>()) 
          {
            String region_code = item["codice_regione"];
            unsigned long number = item["nuovi_positivi"];

            // sum P.A. Bolzano and P.A. Trento
            if(region_code == "21") {
              trentinoSum += number;
              continue;
            }
            if(region_code == "22") {
              number += trentinoSum;
              region_code = 4;
            }

            // Normalizzazione 100k abitanti (numero casi/popolazione)*100000
            unsigned long abitantiReg = abitantiRegArr[region_code.toInt()-1];
            numberNorm = (number * 100000) / abitantiReg;

            //Array numeri normalizzati
            numberNormArr[region_code.toInt()-1] = numberNorm;

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

          // calculation of percentages for color selection
          unsigned long perc25max = numberNorMax - ((numberNorMax * 25) / 100);
          unsigned long perc25min = numberNorMin + ((numberNorMin * 25) / 100);
          unsigned long percMedia = (numberNorMax - numberNorMin) / 2 ;

#ifdef DEBUG 
          Serial.print("max: "); Serial.println(numberNorMax);
          Serial.print("min: "); Serial.println(numberNorMin);
          Serial.print("perc25max: "); Serial.println(perc25max);
          Serial.print("perc25min: "); Serial.println(perc25min);
          Serial.print("percMedia: "); Serial.println(percMedia);
#endif 

          // color selection
          for(int i = 0; i < 20; i++) 
          {
            if (numberNormArr[i] > perc25max) {
              //Serial.print(numberNormArr[i]); Serial.println(" percmax");  
              numberNormArr[i] = 0xff0000; //rosso
            }
            else if ((numberNormArr[i] <= perc25max) && (numberNormArr[i] > percMedia)) {
              //Serial.print(numberNormArr[i]); Serial.println(" max-media"); 
              numberNormArr[i] = 0xff5203; //arancio
            }
            else if ((numberNormArr[i] <= percMedia) && (numberNormArr[i] > perc25min)) {
              //Serial.print(numberNormArr[i]); Serial.println(" media-min"); 
              numberNormArr[i] = 0xffd103; //giallo
            }            
            else if (numberNormArr[i] <= perc25min) {
              //Serial.print(numberNormArr[i]); Serial.println(" percmin"); 
              numberNormArr[i] = 0xffffff; //bianco
            }            

            // save color into map
            region_map[String(i)] = numberNormArr[i];
          }
        }
        // free the memory
        doc.clear();
      }
      http.end();
    }
  }

  // check if it's time to refresh the leds
  if (last_refresh == 0 || ((millis() - last_refresh > REFRESH_INTERVAL) && showing))
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

          if (brightness <= BRIGHTNESS_BLEND_CUTOFF)
          {
            // calculate percent
            float percent = (float)brightness / BRIGHTNESS_BLEND_CUTOFF;
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
          leds[address] = blended;
        }
      }
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