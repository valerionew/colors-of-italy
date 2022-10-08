#pragma once

#include <cstddef>

// Button class
class Button
{

 private:
  const uint8_t input_pin;
  const uint8_t THRESHOLD;
  const uint8_t BIG_THRESHOLD;
  const uint8_t ANTI_BOUNCE = 20;


  bool pressed{false};
  uint32_t lastUpdate = 0;
  uint32_t startPress = 0;

  const uint32_t peso = 3000;
  uint32_t sumMediaNormalizzata = 0;

 public:
  Button(uint8_t _input_pin, uint8_t _threshold = 10, uint8_t _big_threashold = 50):
    input_pin(_input_pin), THRESHOLD(_threshold), BIG_THRESHOLD(_big_threashold) // initialization list
  {
  }

  void update()
  {
    //force fixed update rate
    uint32_t now = millis();
    if (now - lastUpdate < 50){
      return;
    }
    lastUpdate = now;

    touch_value_t reading = touchRead(input_pin);

    int32_t mediaNormalizzata = sumMediaNormalizzata / peso;

/*
    Serial.print("touch_minus ");
    Serial.print(sumMediaNormalizzata);
    Serial.print(" ");
    Serial.print(mediaNormalizzata);
    Serial.print(" ");
    Serial.print(reading);
*/    
    if (mediaNormalizzata - reading > BIG_THRESHOLD || reading - mediaNormalizzata > BIG_THRESHOLD){
      // too big of a gap,reset
      Serial.println(" RESET MEAN");
      sumMediaNormalizzata = reading * peso;
      pressed = false;
    } if (mediaNormalizzata - reading > THRESHOLD) {
      // decent gap, probably a touch
      //Serial.println(" pressed");
      pressed = true;
      startPress = now;
    }else if(pressed && now - startPress < ANTI_BOUNCE){
      // we got a bounce, increment a the mean by 1 (aka, peso)
      Serial.println("BOUNCE");
      sumMediaNormalizzata += peso;
      pressed = true;
    }else{
      sumMediaNormalizzata += -mediaNormalizzata + reading;
      pressed = false;
    }
  }

  bool is_pressed()
  {
    return pressed;
  }
};
