#include <Arduino.h>

#define TOUCH_PLUS_PIN T3  // GPIO 4
#define TOUCH_RESET_PIN T2 // GPIO 2
#define TOUCH_MINUS_PIN T0 // GPIO 15

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
      float box_filtered = buffer.update(reading - lp_filtered); // compare to average
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
    // is this the button first press?
    return rising;
  }
};

Button touch_minus(TOUCH_MINUS_PIN, 70);
Button touch_reset(TOUCH_RESET_PIN, 50);
Button touch_plus(TOUCH_PLUS_PIN, 50);

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