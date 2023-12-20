#pragma once
#include "Arduino.h"
#define maxMeasureTime 5000
class TimeBasedCounter
{
public:
  TimeBasedCounter(uint32_t startValue = 0)
  {
    this->result = startValue;
    this->lastMeasureTime = millis();
  }

  void measure(const float currentValue)
  {
    uint32_t measureTime = millis() - this->lastMeasureTime;
    if (measureTime > maxMeasureTime)
      return;
    this->lastMeasureTime = millis();
    this->result += currentValue * measureTime / 3600.0f;
  }

  void reset(const float startValue = 0)
  {
    this->result = startValue;
    lastMeasureTime = millis();
  }

  float getResult()
  {
    return result;
  }

private:
  float result;
  uint32_t lastMeasureTime;
};