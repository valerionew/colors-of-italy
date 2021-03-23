#include <Arduino.h>
#include <FastLED.h>

#define LED_NUMBER 22
#define LED_PIN 5
#define MAX_BLEND 100     // max color blending (0-255)
#define MIN_BRIGHTNESS 80 // cutoff brightness (0-255)

CRGB offset = 0xFFFF00; // color that gets added to original one
CRGB leds[LED_NUMBER];

long int colors[4] = {0xFF0000, 0xB93C00, 0xFFBB00, 0xFFFFFF};
byte brightness = 255;
long int count = 0;

void setup();
void loop();
float easing(float x);

// x: 0->1
// return: 0->1
float easing(float x)
{
  // quadratic easing
  return 1 - (1 - x) * (1 - x);
}

void setup()
{
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, LED_NUMBER);
}

void fillLeds()
{
  for (int i = 0; i < LED_NUMBER; i++)
  {
    // color correction starts here
    CRGB blended;
    float eased;

    if (brightness <= MIN_BRIGHTNESS && count % 2 == 0)
    {
      // calculate percent
      float percent = (float)brightness / MIN_BRIGHTNESS;
      // ease percent
      // we need to invert it (1-easing) in order to get 1 for low brightness values
      // and 0 for high brightness values, so that more color gets blended at lower
      // brightness to compensate the unbalancing of the leds
      eased = (1 - easing(percent)) * MAX_BLEND;
    }
    else
    {
      // skip color correction if brightness if over the cutoff value (or, in this case,
      // count is even)
      eased = 1;
    }

    // blend colors
    blended = blend(colors[i % 3], offset, eased);
    // add color
    leds[i] = blended;
  }
  // color correction ends here

  // set global brightness
  FastLED.setBrightness(brightness);
  // actually show leds
  FastLED.show();
}

void loop()
{
  brightness = 20;
  count++;
  fillLeds();
  delay(1000);
}