#include <Arduino.h>

#define TOUCH_PLUS_PIN T3  // GPIO 4
#define TOUCH_RESET_PIN T2 // GPIO 2
#define TOUCH_MINUS_PIN T0 // GPIO 15

// LPF class
class LPF
{
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

private:
  float alpha;
  float value;
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
  LPF filter_1;
  LPF filter_2;

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

    filter_1.init(0.5, old_reading);   // filter input
    filter_2.init(0.001, old_reading); // filter average
  }

  void update()
  {
    float reading = read();
    if (abs(reading - old_reading) < outlier_threshold)
    {
      //ignore corrupt samples
      float new_reading = filter_1.update(reading);
      float new_average = filter_2.update(new_reading);
      pressed = (new_average - new_reading) > threshold;
    }
    // update value
    old_reading = reading;
  }

  bool is_pressed()
  {
    rising = pressed && !old_pressed;
    old_pressed = pressed;

    return pressed;
  }

  bool first_press()
  {
    return rising;
  }
};

Button touch_minus(TOUCH_MINUS_PIN, 7);
Button touch_reset(TOUCH_RESET_PIN, 5);
Button touch_plus(TOUCH_PLUS_PIN, 5);

void setup()
{
  Serial.begin(115200);
  // Touch initialization
  touchSetCycles(0xA000, 0xA000);
  touch_minus.init();
  touch_reset.init();
  touch_plus.init();
}

void loop()
{
  // check touch buttons
  touch_minus.update();
  touch_reset.update();
  touch_plus.update();

  if (touch_minus.is_pressed() && touch_minus.first_press())
  {
    Serial.println("touch_minus is pressed");
    // DECREASE BRIGHTNESS
  }

  if (touch_reset.is_pressed() && touch_reset.first_press())
  {
    Serial.println("touch_reset is pressed");
    // TOGGLE LED UPDATE
  }

  if (touch_plus.is_pressed() && touch_plus.first_press())
  {
    Serial.println("touch_plus is pressed");
    // INCREASE BRIGHTNESS
  }
}