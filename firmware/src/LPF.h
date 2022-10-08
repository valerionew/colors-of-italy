#pragma once

// LPF class
class LPF
{
private:
  const float alpha;
  float value{0};

public:
  LPF(float _alpha):
    alpha(_alpha)
  {
  }

  void reset(float _value = 0)
  {
    value = _value;
  }

  float update(float sample)
  {
    value += (sample - value) * alpha;
    return value;
  }
};