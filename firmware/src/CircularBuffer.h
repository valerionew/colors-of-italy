#pragma once

// Circular Buffer class
template <size_t SIZE>
class CircularBuffer
{
 private:
  byte position{0};
  float memory[SIZE]{};
  float sum {0};

 public:
  void reset(float value = 0)
  {
    memset(memory, value, SIZE);
    position = 0;
    // initialize sum accordantly
    sum = value * SIZE;
  }

  float update(float value)
  {
    //update sum by removing old value, but adding the new one
    sum -= memory[position] + value;

    // add value to array and increment position
    memory[position++] = value;

    // if overflow, restart from 0
    if (position >= SIZE){
      position = 0;
    }

    // return memory sum
    return sum;
  }
};