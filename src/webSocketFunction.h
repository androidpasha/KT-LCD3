#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>
#include "struct.h"
#include "KTLCD_Display.h"
#include "AverageValue.h"
#include "SaveTemplateDataToLFS.h"
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "TimeBasedCounter.h"

extern uint32_t lastSaveToFsTime;
extern KTLCD_Display bicycle;
extern AverageValue averageSpeed;
extern SaveToFsStruct measurementData;
extern User userParameters;
extern AsyncWebSocket webSocket;
extern bool saveFlagMeasurementDataToFile;
// extern bool clientConected;
extern TimeBasedCounter odometr;
extern TimeBasedCounter wattMeter;
extern TimeBasedCounter calories;
extern SaveTemplateDataToLFS<SettingsStruct> SaveControllerDataToLFS;
extern SaveTemplateDataToLFS<User> SaveUserDataToLFS;

void webSocketSendBicycleDataToDisplay()
{
  StaticJsonDocument<900> doc;

  JsonObject Velocity = doc.createNestedObject("Velocity");
  Velocity["Current_speed"] = bicycle.receiveData.speed; // bicycle.receiveData.speed; // random(10,45);
  Velocity["avg"] = averageSpeed.resultAverage;
  Velocity["Vmax"] = averageSpeed.maxValue;

  JsonObject Odometer = doc.createNestedObject("Odometer");
  Odometer["general"] = measurementData.odoGeneral / 1000.0f;
  Odometer["afterPowerOn"] = measurementData.odoDrive / 1000.0f;
  Odometer["afterCharging"] = measurementData.odoCharging / 1000.0f;
  Odometer["afterService"] = measurementData.odoService / 1000.0f;
  Odometer["afterLubrication"] = measurementData.odoLubrication / 1000.0f;
  Odometer["daily"] = measurementData.odoDaily / 1000.0f;

  JsonObject Cal = doc.createNestedObject("Cal");
  Cal["CalTotal"] = measurementData.caloriesTotal / 1000.0f;
  Cal["CalDrive"] = (measurementData.caloriesTotal - measurementData.caloriesDrive) / 1000.0f;
  Cal["FatTotal"] = measurementData.fatTotal() / 1000.0f;
  Cal["FatDrive"] = measurementData.fatDrive() / 1000.0f;

  JsonObject Power = doc.createNestedObject("Power");
  Power["momentary"] = bicycle.receiveData.power;
  Power["sum"] = measurementData.wattMeter / 1000;
  uint8_t ratedVoltage = bicycle.receiveData.ratedVoltage;
  Power["current"] = bicycle.receiveData.power / ratedVoltage;
  Power["voltage"] = bicycle.receiveData.ratedVoltage;
  Power["BatteryPercent"] = bicycle.receiveData.batteryPercent();

  JsonObject driveTime = doc.createNestedObject("driveTime");
  driveTime["drive"] = millis();
  driveTime["all"] = (measurementData.timeGeneral + (millis() - lastSaveToFsTime) / 1000) / 60 / 60; // in hour

  JsonObject ledsInTablo = doc.createNestedObject("ledsInTablo");
  ledsInTablo["LedCruise"] = bicycle.settings.cruise; // bicycle.receiveData.cruise;// bicycle.settings.cruise;
  ledsInTablo["LedBrake"] = bicycle.receiveData.brake;
  ledsInTablo["LedCheck"] = bicycle.receiveData.error;
  bicycle.receiveData.batteryLevel == 1 ? ledsInTablo["LedAkkumulator"] = true : ledsInTablo["LedAkkumulator"] = false;
  bicycle.receiveData.temperature > 70 ? ledsInTablo["LedTemperature"] = true : ledsInTablo["LedTemperature"] = false;

  doc["throttle"] = bicycle.receiveData.throttle;
  doc["assist"] = bicycle.receiveData.assistant;
  doc["PAS"] = bicycle.settings.pasLevel;
  doc["Button6km"] = bicycle.settings.walkMode;
  doc["freeHeap"] = ESP.getFreeHeap();
  doc["cadence"] = bicycle.receiveData.cadencePeriod;

  JsonObject inArr = doc.createNestedObject("inArr");
  for (int i = 0; i < 12; i++)
    inArr["iB" + String(i)] = bicycle.receiveBuffer[i];

  String jsonData;
  serializeJson(doc, jsonData);
  webSocket.textAll(jsonData);
}

void webSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
//(uint8_t num, WStype_t type, uint8_t *payload, size_t length)
{
  if (type == WS_EVT_DATA)
  {
    AwsFrameInfo *info = (AwsFrameInfo *)arg;
    if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT)
    {
      data[len] = '\0';

      StaticJsonDocument<900> doc; // 768
      deserializeJson(doc, (char *)data);

      if (doc.containsKey("PASS"))
        bicycle.settings.pasLevel = doc["PASS"];
      else if (doc.containsKey("Cruise"))
        bicycle.settings.cruise = doc["Cruise"];
      else if (doc.containsKey("resetVmax"))
        averageSpeed.resetMaxValue();
      else if (doc.containsKey("resetOdometerDaily"))
      {
        measurementData.odoDaily = measurementData.odoGeneral;
        saveFlagMeasurementDataToFile = true;
      }
      else if (doc.containsKey("resetOdometerAfterCharging"))
        measurementData.odoCharging = measurementData.odoGeneral;
      else if (doc.containsKey("resetWattMeter"))
      {
        wattMeter.reset();
        measurementData.wattMeter = 0;
        saveFlagMeasurementDataToFile = true;
      }
      else if (doc.containsKey("resetCalories"))
      {
        calories.reset();
        measurementData.caloriesTotal = 0;
        measurementData.caloriesDrive = 0;
        saveFlagMeasurementDataToFile = true;
      }
      else if (doc.containsKey("resetOdometerafterLubrication"))
      {
        measurementData.odoLubrication = measurementData.odoGeneral;
        saveFlagMeasurementDataToFile = true;
      }
      else if (doc.containsKey("resetOdometerAfterService"))
      {
        measurementData.odoService = measurementData.odoGeneral;
        saveFlagMeasurementDataToFile = true;
      }
      else if (doc.containsKey("WalkMode"))
        bicycle.settings.walkMode = doc["WalkMode"];
      else if (doc.containsKey("OdometerGeneral"))
      {
        measurementData.reset((uint32_t)doc["OdometerGeneral"] * 1000);
        odometr.reset((float)doc["OdometerGeneral"] * 1000);
        saveFlagMeasurementDataToFile = true;
      }

      else if (doc.containsKey("powerFactor") and doc.containsKey("C14_PasAdjustment"))
      {
        SettingsStruct *BS = &bicycle.settings;
        BS->powerFactor = (float)doc["powerFactor"];
        BS->maxSpeedLimit = (uint8_t)doc["maxSpeedLimit"];
        BS->wheelRimDiameter = (uint16_t)doc["wheelRimDiameter"];
        BS->P1_MotorFeature = (uint8_t)doc["P1_MotorFeature"];
        BS->P2_WheelSetSpeedPulse = (uint8_t)doc["P2_WheelSetSpeedPulse"];
        BS->P3_PasControlMode = (bool)doc["P3_PasControlMode"];
        BS->P4_ThrottleActiveMode = (bool)doc["P4_ThrottleActiveMode"];
        BS->P5_BatteryMonitorModes = (uint8_t)doc["P5_BatteryMonitorModes"];
        BS->C1_PasSensorSensitivity = (uint8_t)doc["C1_PasSensorSensitivity"];
        BS->C2_MotorPhase = (uint8_t)doc["C2_MotorPhase"];
        BS->C4_ThrottleFunction = (uint8_t)doc["C4_ThrottleFunction"];
        BS->C4_SpeedLimitValueOfThrottle = (uint8_t)doc["C4_SpeedLimitValueOfThrottle"];
        BS->C4_PercentageValueOfTheFirstGearSpeed = (uint8_t)doc["C4_PercentageValueOfTheFirstGearSpeed"];
        BS->C5_ControllerMaxCurrent = (uint8_t)doc["C5_ControllerMaxCurrent"];
        BS->C12_ControllerLowestVoltage = (uint8_t)doc["C12_ControllerLowestVoltage"];
        BS->C13_ControllerAbsBraking = (uint8_t)doc["C13_ControllerAbsBraking"];
        BS->C14_PasAdjustment = (uint8_t)doc["C14_PasAdjustment"];

        userParameters.age = (uint8_t)doc["Age"]; //
        userParameters.height = (uint8_t)doc["Height"];
        userParameters.sex = (bool)doc["SEX"];
        userParameters.weight = (uint8_t)doc["weight"];
        userParameters.calCorrectFactor = doc["calCorrectFactor"];

        SaveUserDataToLFS.writeToFile(userParameters);
        SaveControllerDataToLFS.writeToFile(*BS);
      }
      else if (doc.containsKey("getSettings"))
      {
        doc.clear();
        SettingsStruct *BS = &bicycle.settings;
        doc["powerFactor"] = BS->powerFactor;
        doc["maxSpeedLimit"] = BS->maxSpeedLimit;
        doc["wheelRimDiameter"] = BS->wheelRimDiameter;
        doc["P1_MotorFeature"] = BS->P1_MotorFeature;
        doc["P2_WheelSetSpeedPulse"] = BS->P2_WheelSetSpeedPulse;
        doc["P3_PasControlMode"] = BS->P3_PasControlMode;
        doc["P4_ThrottleActiveMode"] = BS->P4_ThrottleActiveMode;
        doc["P5_BatteryMonitorModes"] = BS->P5_BatteryMonitorModes;
        doc["C1_PasSensorSensitivity"] = BS->C1_PasSensorSensitivity;
        doc["C2_MotorPhase"] = BS->C2_MotorPhase;
        doc["C4_ThrottleFunction"] = BS->C4_ThrottleFunction;
        doc["C4_SpeedLimitValueOfThrottle"] = BS->C4_SpeedLimitValueOfThrottle;
        doc["C4_PercentageValueOfTheFirstGearSpeed"] = BS->C4_PercentageValueOfTheFirstGearSpeed;
        doc["C5_ControllerMaxCurrent"] = BS->C5_ControllerMaxCurrent;
        doc["C12_ControllerLowestVoltage"] = BS->C12_ControllerLowestVoltage;
        doc["C13_ControllerAbsBraking"] = BS->C13_ControllerAbsBraking;
        doc["C14_PasAdjustment"] = BS->C14_PasAdjustment;
        doc["Age"] = userParameters.age;
        doc["Height"] = userParameters.height;
        doc["weight"] = userParameters.weight;
        doc["SEX"] = userParameters.sex;
        doc["calCorrectFactor"] = userParameters.calCorrectFactor;

        String jsonData;
        serializeJson(doc, jsonData);
        // if (clientConected == true)
        webSocket.textAll(jsonData);
      }
    }
  }
}
