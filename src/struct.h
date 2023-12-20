#pragma once
#include <Arduino.h>
const float calKgRatio = 7.716179176;

struct SaveToFsStruct
{
  float
      odoGeneral,
      odoDaily,
      odoDrive,
      odoCharging,
      odoService,
      odoLubrication,
      caloriesTotal,
      caloriesDrive;
  uint32_t
      timeGeneral,
      wattMeter;
  float fatTotal() { return caloriesTotal / calKgRatio; }
  float fatDrive() { return (caloriesTotal - caloriesDrive) / calKgRatio; }
  void reset(int startValue = 0)
  {
    odoGeneral = odoDaily = odoDrive = odoCharging = odoService = odoLubrication = startValue;
  }
};

struct User
{
  uint8_t age, height, weight, sex;
  float calCorrectFactor; //
};

/*
SettingsStruct *settings = new SettingsStruct{
  pasLevel : 5,                               // Range:0-5
  cruise : false,                             // Rage:0-1
  walkMode : false,                           // Rage:0-1
  frontLight : false,                         // Rage:0-1
  powerFactor : 6,                            // Множитель мощности для корректного отображения мощности. Не передается в контроллер
  maxSpeedLimit : 45,                         //
  wheelRimDiameter : 26,                      // Range: 5, 6, 8, 10, 12, 14, 16, 18, 20, 23, 24, 26, 28, 29, 700
  P1_MotorFeature : 100,                      // Range：1-255
  P2_WheelSetSpeedPulse : 1,                  // Rage:0-6
  P3_PasControlMode : true,                   // Rage:0-1
  P4_ThrottleActiveMode : false,              // Rage:0-1
  P5_BatteryMonitorModes : 12,                // Range:0-40
  C1_PasSensorSensitivity : 2,                // Range:0-7
  C2_MotorPhase : 0,                          // Range:0-7
  C4_ThrottleFunction : 3,                    // Range:0-4
  C4_SpeedLimitValueOfThrottle : 20,          // Range:0-127
  C4_PercentageValueOfTheFirstGearSpeed : 50, // Range:0-100
  C5_ControllerMaxCurrent : 0,                // Range:0-10
  C12_ControllerLowestVoltage : 4,            // Range:0-7
  C13_ControllerAbsBraking : 0,               // Range:0-5
  C14_PasAdjustment : 2,                      // Range:1-3
};
 KTLCD_Display bicycle(settings);
*/