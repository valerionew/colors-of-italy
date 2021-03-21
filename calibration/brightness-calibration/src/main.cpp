#include <Arduino.h>
#include <FastLED.h>

#define LED_NUMBER 22
#define LED_PIN 5

CRGB leds[LED_NUMBER];
long int colors[4] = {0xFF0000, 0xB95000, 0xFFBB00, 0xFFFFFF};
byte count = 0;

void setup();
void loop();

void setup()
{
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, LED_NUMBER)
      .setCorrection(Tungsten40W);
}

void fillLeds(long int color)
{
  for (int i = 0; i < LED_NUMBER; i++)
  {
    byte brightness;
    brightness = 255 / LED_NUMBER * i;

    // color correction begins here
    // basically, i did math
    // some kind of lerp i don't know
    // adds green and red to darker brightness
    float percent;
    percent = (float)brightness / 255.0;

    byte red;
    red = (color & 0xFF0000) >> 16;
    long int dRed;
    dRed = byte(red * percent) << 16;

    byte green;
    green = (color & 0x00FF00) >> 8;
    long int dGreen;
    dGreen = byte(green * percent) << 8;

    byte blue;
    blue = (color && 0x0000FF);
    long int dBlue;
    dBlue = byte(blue * percent);

    color = color + byte(dGreen) + byte(dRed) + byte(dBlue);
    // color correction ends here

    leds[i] = color;
    leds[i].fadeToBlackBy(brightness);
  }

  FastLED.show();
}

void loop()
{
  fillLeds(colors[count % 4]);
  count++;
  delay(1000);
}