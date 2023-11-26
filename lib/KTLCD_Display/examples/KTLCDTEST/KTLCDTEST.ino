#include "KTLCD_Display.h"

SettingsStruct *settings = new SettingsStruct{
  LCD_RX_PIN : 3, //green wire
  LCD_TX_PIN : 2,// yellow wire
  pasLevel : 5,                               // Range:0-5
  cruise : false,                             // Rage:0-1
  walkMode : false,                           // Rage:0-1
  frontLight : false,                         // Rage:0-1
  speedDivided : 7420.0,                      // Делимое для расчета скорости, зная период. Влияет на отображение скорости библиотекой, не передается в контроллер. ReceiveData.speed = settings.speedDivided / ReceiveData.speedPeriod;
  maxSpeedLimit : 30,                         //
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

void setup()
{
  Serial.begin(74880);
  bicycle.printTransferBuffer();
}

void loop()
{
  float speed = bicycle.ReceiveData.speed;
  bicycle.sendSettingsToController();
  delay(1000);
  if (bicycle.dataAvailable() == true)
    bicycle.ReceiveData.print();
}