#pragma once
#include "Arduino.h"
#define maxMeasureTime 1000
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
      uint32_t measureTime = millis() - lastMeasureTime;
      if (measureTime > maxMeasureTime)
        measureTime = maxMeasureTime;
      this->lastMeasureTime = millis();
      this->result = calculateResult(currentValue, measureTime);
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

  float calculateResult(const float currentValue, const uint32_t &measureTime)
  {
    result += currentValue * measureTime / 3600.0f;
    return result;
  }
};