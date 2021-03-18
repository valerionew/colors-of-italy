/* Script used to pick the region colors that will be pushed from server
 Final picked colors: 
    - Red  FF0000
    - Orange B93C00
    - Yellow FFBB00
    - White FFE1B4

  To use:
    - Open serial monitor and write hex color inside (e.g. #FF0032 or ab32f1)
*/

#include <Arduino.h>
#include <FastLED.h>

#define LED_NUMBER 22
#define LED_PIN 5

CRGB leds[LED_NUMBER];
int count;
long int color;

void setup()
{
  Serial.begin(115200);

  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, LED_NUMBER).setCorrection(Tungsten40W);
  count = 0;
  color = 0;

  initLeds();
}

void initLeds()
{
  // the first loop is not enough
  // explain me why
  // you have to set the colors twice
  // or they all get fucky
  for (int j = 0; j < 2; j++)
  {
    for (int i = 0; i < LED_NUMBER; i++)
    {
      leds[i] = 0xFF0000;
    }
    FastLED.show();
  }
}

void fillLeds(long int color)
{
  for (int i = 0; i < LED_NUMBER; i++)
  {
    byte brightness;
    brightness = 255 / LED_NUMBER * i;
    leds[i] = color;
    leds[i].fadeToBlackBy(brightness);
  }

  FastLED.show();
}

void loop()
{
  while (Serial.available() > 0)
  {
    if (count < 6)
    {
      char c = Serial.read();
      long int hex;

      if (c >= 48 && c <= 57)
      {
        // decimal
        hex = (long int)(c - 48);
      }
      else if (c >= 65 && c <= 70)
      {
        // hex
        hex = (long int)(c - 55);
      }
      else if (c >= 97 && c <= 102)
      {
        // lowecase hex
        hex = (long int)(c - 87);
      }
      else
      {
        hex = -1;
      }

      if (hex != -1)
      {
        color += hex * pow(16, 5 - count);
        count++;
      }
    }

    if (count == 6)
    {
      fillLeds(color);

      Serial.print("Color #");
      Serial.print(color, HEX);
      Serial.println();
      count = 0;
      color = 0;
    }
  }
}