#pragma once

#include <cstddef>
#include <pixeltypes.h>

constexpr CRGB OFFSET{0xFFFF00}; // color that gets blended with original one (color correction)

class Regioni{
  uint8_t brightness = 255;
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

  void setBrightness(const uint8_t _brightness){
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

  // x: 0->1
  // return: 0->1
  float easing(float x)
  {
    // quadratic easing
    return 1 - (1 - x) * (1 - x);
  }
};