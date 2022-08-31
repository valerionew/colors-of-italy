#ifndef PACIFICA_H
#define PACIFICA_H

#include <Arduino.h>
#include <FastLED.h>

void pacifica_add_whitecaps();
void pacifica_one_layer(CRGBPalette16 &p, uint16_t cistart, uint16_t wavescale, uint8_t bri, uint16_t ioff);
void pacifica_deepen_colors();
void pacifica_loop();

#endif