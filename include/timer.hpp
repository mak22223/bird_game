#pragma once

#include <Arduino.h>

class Timer
{
public:
  Timer(unsigned long period)
  : d_lastReset(millis()), d_period(period) {}

  bool check()
  {
    return millis() - d_lastReset >= d_period;
  }

  void reset()
  {
    d_lastReset = millis();
  }

protected:
  unsigned long d_lastReset;
  unsigned long d_period;
};