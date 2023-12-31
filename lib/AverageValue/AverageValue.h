#pragma once
#include "Arduino.h"

class AverageValue
{
public:
  void measure(float currentValue)
  {
      this->resultAverage = calculateAverage(currentValue);
  }

  void resetMaxValue()
  {
    maxValue = 0;
  }

  void resetAverageResult()
  {
    sumValues = 0.0f;
    numReadings = 1;
    resultAverage = 0;
  }

  float resultAverage;
  float maxValue;

private:
  float calculateAverage(float &currentValue)
  {
    if (maxValue < currentValue)
      maxValue = currentValue;
    sumValues += currentValue;
    numReadings++;
    return sumValues / (float)numReadings;
  }

  float sumValues = 0.0F;
  uint32_t numReadings = 1;
};