#include <Arduino.h>
#include "KTLCD_Display.h"
#include "AverageValue.h"
#include "TimeBasedCounter.h"
#include <ESP8266WiFi.h> // Библиотека для создания Wi-Fi подключения (клиент или точка доступа)
#include <DNSServer.h>
#include <ESP8266WebServer.h> // Библиотека для управления устройством по HTTP (например из браузера)
#include <WebSocketsServer.h>
#include "ArduinoJson.h"
#include <FS.h> // Библиотека для работы с файловой системой
#include <LittleFS.h>
#include "SaveTemplateDataToLFS.h"
#include "Ticker.h"
#define SERIAL1 // Закоментировать если будет использоваться обычный Serial (0)
#define MINIMUM_TIME_BETWEEN_WRITES_TO_THE_FILE_SYSTEM_SEC 60

    const char *ssidAP = "Bicycle"; // Название генерируемой точки доступа

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

KTLCD_Display bicycle;
const char *dataStorageFileNameController PROGMEM = "/controllerSettingsStorage.bin";
SaveTemplateDataToLFS<SettingsStruct> SaveControllerDataToLFS(bicycle.settings, dataStorageFileNameController);

TimeBasedCounter odometr;
TimeBasedCounter wattMeter;
AverageValue averageSpeed;
Ticker scheduler_100ms;
const char *dataStorageFileNameOdometr PROGMEM = "/odometrDataStorage.bin";
DNSServer dnsServer; // DNS сервер для перехода на главную страницу с любой другой
ESP8266WebServer webServer(80);
WebSocketsServer webSocket(81);
bool scheduleFlag = false;
uint32_t lastSaveToFsTime = 0;
bool saveFlagOdometrDataToFile = false;
void WebServer();
bool handleFileRead(String path);
String getContentType(String filename);
struct saveToFsStruct
{
  float
      general,
      daily,
      afterPowerOn,
      afterCharging,
      afterService,
      afterLubrication;
  uint32_t timeGeneral,
      driveTime,
      wattMeter;
  void reset(int resetValue)
  {
    general = daily = afterPowerOn = afterCharging = afterService = afterLubrication = resetValue;
  }
} saveDataToFS;

SaveTemplateDataToLFS<saveToFsStruct> SaveOdometrDataToLFS(saveDataToFS, dataStorageFileNameOdometr);

void webSocketSendBicycleDataToDisplay();

void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length)
{
  if (type == WStype_TEXT)
  {
    StaticJsonDocument<768> doc;
    deserializeJson(doc, (char *)payload);

    if (doc.containsKey("PASS"))
      bicycle.settings.pasLevel = doc["PASS"];
    else if (doc.containsKey("Cruise"))
      bicycle.settings.cruise = doc["Cruise"];
    else if (doc.containsKey("resetVmax"))
      averageSpeed.resetMaxValue();
    else if (doc.containsKey("resetOdometerDaily"))
    {
      saveDataToFS.daily = saveDataToFS.general;
      saveFlagOdometrDataToFile = true;
    }
    else if (doc.containsKey("resetOdometerAfterCharging"))
      saveDataToFS.afterCharging = saveDataToFS.general;
    else if (doc.containsKey("resetWattMeter"))
    {
      wattMeter.reset();
      saveDataToFS.wattMeter = 0;
      saveFlagOdometrDataToFile = true;
    }
    else if (doc.containsKey("resetOdometerafterLubrication"))
    {
      saveDataToFS.afterLubrication = saveDataToFS.general;
      saveFlagOdometrDataToFile = true;
    }
    else if (doc.containsKey("resetOdometerAfterService"))
    {
      saveDataToFS.afterService = saveDataToFS.general;
      saveFlagOdometrDataToFile = true;
    }
    else if (doc.containsKey("WalkMode"))
      bicycle.settings.walkMode = doc["WalkMode"];
    else if (doc.containsKey("OdometerGeneral"))
    {
      saveDataToFS.reset((uint32_t)doc["OdometerGeneral"] * 1000);
      odometr.reset((float)doc["OdometerGeneral"] * 1000);
      saveFlagOdometrDataToFile = true;
    }

    else if (doc.containsKey("powerFactor") and doc.containsKey("C14_PasAdjustment"))
    {
      SettingsStruct *BS = &bicycle.settings;
      BS->powerFactor = (uint8_t)doc["powerFactor"];
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
      String jsonData;
      serializeJson(doc, jsonData);
      webSocket.broadcastTXT(jsonData);
    }
  }
}

void every100ms()
{
  scheduleFlag = true;
}

bool bicycleIsAvailableData()
{
  bool dataAvailable = bicycle.dataAvailable();

  if (dataAvailable == true)
  {
    averageSpeed.measure(bicycle.receiveData.speed);
    odometr.measure(bicycle.receiveData.speed);
    wattMeter.measure((float)bicycle.receiveData.power);
    saveDataToFS.general = odometr.getResult();
    saveDataToFS.wattMeter = (uint32_t)wattMeter.getResult();
  }
  return dataAvailable;
}

void setup()
{
  Serial.begin(9600);
#ifdef SERIAL1
  Serial.swap(); // Переключаем апаратный serial на другие пины
#endif
  LittleFS.begin(); // Инициализируем работу с файловой системой

  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssidAP);
  dnsServer.start(53, "bicycle", WiFi.softAPIP()); // Переходим с любой страницы на IP точки доступа ESP8266
  webServer.begin();                               // Инициализируем Web-сервер
  WebServer();                                     // Функция обработки запросов
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);

  odometr.reset(saveDataToFS.general);
  wattMeter.reset(saveDataToFS.wattMeter);
  saveDataToFS.afterPowerOn = saveDataToFS.general;
  scheduler_100ms.attach_ms(100, every100ms);
}

void loop()
{
  dnsServer.processNextRequest(); // Переходим с любой страницы на IP точки доступа ESP8266
  webServer.handleClient();       // Обработчик HTTP-событий (отлавливает HTTP-запросы к устройству и обрабатывает их)
  webSocket.loop();
  if (scheduleFlag == true)
  {
    bicycle.sendSettingsToController();
    scheduleFlag = false;
  }
  if (bicycleIsAvailableData() == true)
  {
    webSocketSendBicycleDataToDisplay();

    static uint32_t minNextTimeSaveOdometrDataToFile = millis();
    if (bicycle.receiveData.speed <= 3 and saveFlagOdometrDataToFile == true and millis() > minNextTimeSaveOdometrDataToFile)
    {
      saveDataToFS.timeGeneral += (millis() - lastSaveToFsTime) / 1000;
      lastSaveToFsTime = millis();
      SaveOdometrDataToLFS.writeToFile(saveDataToFS);
      saveFlagOdometrDataToFile = false;
      minNextTimeSaveOdometrDataToFile = millis() + MINIMUM_TIME_BETWEEN_WRITES_TO_THE_FILE_SYSTEM_SEC * 1e3;
    }
    else if (bicycle.receiveData.speed > 3)
      saveFlagOdometrDataToFile = true;
  }
}

void webSocketSendBicycleDataToDisplay()
{

  StaticJsonDocument<630> doc;
  JsonObject Velocity = doc.createNestedObject("Velocity");
  Velocity["Current_speed"] = bicycle.receiveData.speed; // bicycle.receiveData.speed; // random(10,45);
  Velocity["avg"] = averageSpeed.resultAverage;
  Velocity["Vmax"] = averageSpeed.maxValue;

  JsonObject Odometer = doc.createNestedObject("Odometer");
  Odometer["general"] = saveDataToFS.general / 1000.0f;
  Odometer["afterPowerOn"] = saveDataToFS.afterPowerOn / 1000.0f;
  Odometer["afterCharging"] = saveDataToFS.afterCharging / 1000.0f;
  Odometer["afterService"] = saveDataToFS.afterService / 1000.0f;
  Odometer["afterLubrication"] = saveDataToFS.afterLubrication / 1000.0f;
  Odometer["daily"] = saveDataToFS.daily / 1000.0f;

  JsonObject Power = doc.createNestedObject("Power");
  Power["momentary"] = bicycle.receiveData.power;
  Power["sum"] = saveDataToFS.wattMeter / 1000;
  uint8_t ratedVoltage = bicycle.receiveData.ratedVoltage;
  if (ratedVoltage == 0)
    ratedVoltage = 36;
  Power["current"] = bicycle.receiveData.power / ratedVoltage;
  Power["voltage"] = bicycle.receiveData.ratedVoltage;
  Power["BatteryPercent"] = bicycle.receiveData.batteryPercent();

  JsonObject driveTime = doc.createNestedObject("driveTime");
  driveTime["drive"] = millis();
  driveTime["all"] = (saveDataToFS.timeGeneral + (millis() - lastSaveToFsTime) / 1000) / 60 / 60; // in hour

  JsonObject ledsInTablo = doc.createNestedObject("ledsInTablo");
  ledsInTablo["LedCruise"] = bicycle.settings.cruise; // bicycle.receiveData.cruise;// bicycle.settings.cruise;
  ledsInTablo["LedBrake"] = bicycle.receiveData.brake;
  ledsInTablo["LedCheck"] = bicycle.receiveData.error;
  bicycle.receiveData.batteryLevel == 1 ? ledsInTablo["LedAkkumulator"] = true : ledsInTablo["LedAkkumulator"] = false;
  bicycle.receiveData.temperature > 70 ? ledsInTablo["LedTemperature"] = true : ledsInTablo["LedTemperature"] = false;

  doc["throttle"]=bicycle.receiveData.throttle;
  doc["assist"]=bicycle.receiveData.assistent;
  doc["PAS"] = bicycle.settings.pasLevel;
  doc["Button6km"] = bicycle.settings.walkMode;
  doc["freeHeap"] = ESP.getFreeHeap();

  String jsonData;
  serializeJson(doc, jsonData);
  webSocket.broadcastTXT(jsonData);
}

void WebServer() // Обработка HTTP-запросов
{
  webServer.on("/settings", HTTP_GET, []()
               {
                 // Выполняем перенаправление на другой URL
                 webServer.sendHeader("Location", "/settings/settings.html"); // Замените "/new-location" на нужный вам URL
                 webServer.send(302);                                         // Отправляем статус "302 Found" для перенаправления
               });

  // webServer.on(F("/Request"), HTTP_POST, []() // Тут принимаем пользовательские данные от страницы
  //              { webServer.send(200, "Content-Type: application/json", "RequestOk"); });

  webServer.onNotFound([]() {             // Описываем действия при событии "Не найдено"
    if (!handleFileRead(webServer.uri())) // Если функция handleFileRead (описана ниже) возвращает значение false в ответ на поиск файла в файловой системе
      handleFileRead("/");                // То возвращаем index.html
  });
}

bool handleFileRead(String path)
{ // Функция работы с файловой системой
  if (path.endsWith("/"))
    path += "index.html"; // Если устройство вызывается по корневому адресу, то должен вызываться файл index.html (добавляем его в конец адреса)
  if (LittleFS.exists(path))
  {                                                   // Если в файловой системе существует файл по адресу обращения
    File file = LittleFS.open(path, "r");             //  Открываем файл для чтения
    webServer.streamFile(file, getContentType(path)); //  Выводим содержимое файла по HTTP, указывая заголовок типа содержимого contentType
    file.close();                                     //  Закрываем файл
    return true;                                      //  Завершаем выполнение функции, возвращая результатом ее исполнения true (истина)
  }
  return false; // Завершаем выполнение функции, возвращая результатом ее исполнения false (если не обработалось предыдущее условие)
}

String getContentType(String filename) // Функция, возвращающая необходимый заголовок типа содержимого в зависимости от расширения файла
{
  String ContentType[2][7] = {
      {F(".html"), F(".css"), F(".js"), F(".png"), F(".jpg"), F(".gif"), F(".ico")},
      {F("text/html"), F("text/css"), F("application/javascript"), F("image/png"), F("image/jpeg"), F("image/gif"), F("image/x-icon")}};
  for (int i = 0; i < 7; i++)
  {
    if (filename.endsWith(ContentType[0][i]))
      return ContentType[1][i];
  }
  return F("text/plain");
}