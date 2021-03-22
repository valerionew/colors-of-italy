#include <Arduino.h>
#include <FastLED.h>

#define LED_NUMBER 22
#define LED_PIN 5
#define MAX_BLEND 40

CRGB leds[LED_NUMBER];
CRGB offset;
long int colors[4] = {0xFF0000, 0xB93C00, 0xFFBB00, 0xFFFFFF};
byte brightness = 40;

void setup();
void loop();

void setup()
{
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, LED_NUMBER)
      .setCorrection(Tungsten40W);
  offset = 0xFFFF00;
}

void fillLeds()
{
  for (int i = 0; i < LED_NUMBER; i++)
  {
    float percent;
    percent = map(brightness, 0, 255, MAX_BLEND, 0);
    CRGB blended = blend(colors[i % 4], offset, percent);
    leds[i] = blended;
  }

  FastLED.setBrightness(brightness);
  FastLED.show();
}

void loop()
{
  fillLeds();
  delay(1000);
}