// https://github.com/androidpasha/KT-LCD3
#pragma region offsetof //includes libs
#include <Arduino.h>
#include "KTLCD_Display.h"
#include "AverageValue.h"
#include "TimeBasedCounter.h"
#include <ESP8266WiFi.h> // Библиотека для создания Wi-Fi подключения (клиент или точка доступа)
// #include <DNSServer.h>
// #include <ESP8266WebServer.h> // Библиотека для управления устройством по HTTP (например из браузера)
#include <WebSocketsServer.h>
#include <ArduinoJson.h>
#include <FS.h> // Библиотека для работы с файловой системой
#include <LittleFS.h>
#include "SaveTemplateDataToLFS.h"
#include "Ticker.h"
#include "struct.h"
#include "webSocketFunction.h" // функции веб сокета вынесены в отдельный файл
// #include "webserverFunction.h" // функции веб сервера вынесены в отдельный файл
#pragma endregion
#pragma region offsetof //constants
#define SERIAL1 // Закомментировать если будет использоваться обычный Serial (0)
#define MINIMUM_TIME_BETWEEN_WRITES_TO_THE_FILE_SYSTEM_SEC 60
const char *ssidAP = "Bicycle";       // Название генерируемой точки доступа
const uint8_t passToPercent[3][6] = { // таблица процентов помощи PAS согласно параметру С14
    {0, 6, 17, 30, 45, 80},
    {0, 12, 22, 40, 55, 80},
    {0, 17, 30, 45, 66, 80}};
const char *dataStorageFileNameControllerSettings PROGMEM = "/controllerSettings.bin";
const char *dataStorageFileNameOdometr PROGMEM = "/measurementData.bin";
const char *dataStorageFileNameUser PROGMEM = "/userParameters.bin";
#pragma endregion
#pragma region offsetof //Class and Struct Instances
SaveToFsStruct measurementData;
User userParameters;
KTLCD_Display bicycle;
TimeBasedCounter odometr;
TimeBasedCounter wattMeter;
TimeBasedCounter calories;
AverageValue averageSpeed;
Ticker scheduler_100ms;
// DNSServer dnsServer; // DNS сервер для перехода на главную страницу с любой другой
// ESP8266WebServer webServer(80);
WebSocketsServer webSocket(81);
SaveTemplateDataToLFS<SettingsStruct> SaveControllerDataToLFS(bicycle.settings, dataStorageFileNameControllerSettings);
SaveTemplateDataToLFS<SaveToFsStruct> SaveMeasureDataToLFS(measurementData, dataStorageFileNameOdometr);
SaveTemplateDataToLFS<User> SaveUserDataToLFS(userParameters, dataStorageFileNameUser);
#pragma endregion
#pragma region offsetof //variables
bool scheduleFlag = false;
uint32_t lastSaveToFsTime = 0;
bool saveFlagMeasurementDataToFile = false;
//bool clientConected;
#pragma endregion
#pragma region offsetof //function declarations
float calculateCalories(const User &user, float velocity, uint8_t pasLevel, uint8_t pasAdjustment);
void every100ms();
bool bicycleIsAvailableData();
#pragma endregion

void setup()
{
  Serial.begin(9600);
#ifdef SERIAL1
  Serial.swap(); // Переключаем аппаратный serial на другие пины
#endif
  LittleFS.begin(); // Инициализируем работу с файловой системой

  WiFi.mode(WIFI_AP);
//   WiFi.softAP(ssidAP);
   WiFi.softAP(ssidAP, "", 1, 0, 1); // ссид, нет паспорта, канал, не скрывать ссид, макс одно подключение
  // dnsServer.start(53, "bicycle", WiFi.softAPIP()); // Переходим с любой страницы на IP точки доступа ESP8266
  // webServer.begin();                               // Инициализируем Web-сервер
  // WebServer();                                     // Функция обработки запросов
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);

  odometr.reset(measurementData.general);
  wattMeter.reset(measurementData.wattMeter);
  // measurementData.energyCaloriesTotal = 0;
  // measurementData.burnFatTotal = 0;
  // measurementData.daily=0;
  // measurementData.general=0;
  // SaveMeasureDataToLFS.writeToFile(measurementData);

  calories.reset(measurementData.energyCaloriesTotal);

  measurementData.afterPowerOn = measurementData.general;
  measurementData.energyCaloriesDrive = measurementData.energyCaloriesTotal;
  measurementData.burnFatDrive = measurementData.burnFatTotal;

  scheduler_100ms.attach_ms(100, every100ms);
}

void loop()
{
    // dnsServer.processNextRequest(); // Переходим с любой страницы на IP точки доступа ESP8266
    // webServer.handleClient();       // Обработчик HTTP-событий (отлавливает HTTP-запросы к устройству и обрабатывает их)
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
    if (bicycle.receiveData.speed <= 3 and saveFlagMeasurementDataToFile == true and millis() > minNextTimeSaveOdometrDataToFile)
    {
      measurementData.timeGeneral += (millis() - lastSaveToFsTime) / 1000;
      lastSaveToFsTime = millis();
      SaveMeasureDataToLFS.writeToFile(measurementData);
      saveFlagMeasurementDataToFile = false;
      minNextTimeSaveOdometrDataToFile = millis() + MINIMUM_TIME_BETWEEN_WRITES_TO_THE_FILE_SYSTEM_SEC * 1e3;
    }
    else if (bicycle.receiveData.speed > 3)
      saveFlagMeasurementDataToFile = true;
    //if (webSocket.connectedClients() > 1)
      //webSocket.disconnect();
  }
}

float calculateCalories(const User &user, float velocity, uint8_t pasLevel, uint8_t pasAdjustment)
{
  if (velocity < 1) // Формула для катання а не для відпочинку!!!
    velocity = 1;

  float calories;
  calories = ((10.0 * user.weight + 6.25 * user.height - 5.0 * user.age + 5.0 * !user.sex - 161.0 * user.sex) * 0.34 * velocity + 1000.0) / 24; // in 60 minutes
  // calories = ((10 * 73.6 + 6.25 * 173.3 - 5 * 52.2 + 5) * 0.34 * 19.6 + 1000.0) / 24.0; // in 60 minutes
  calories *= (float(100 - passToPercent[pasAdjustment - 1][pasLevel]) / 100.0); // урахування значення PAS
  calories *= userParameters.cal_coefficient;
  // String message = "pasAdjustment:" + String(pasAdjustment) +". pasLevel:"+ String(pasLevel) +"/ pgm_read:"+String(pgm_read_byte(&passToPercent[pasAdjustment-1][pasLevel]));

  // webSocket.broadcastTXT(message);
  return calories;
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
    bicycle.receiveData.speed = 60.0;
    if (bicycle.receiveData.speed >= 3.0f)
      averageSpeed.measure(bicycle.receiveData.speed);
    odometr.measure(60.0);
    wattMeter.measure((float)bicycle.receiveData.power);
    measurementData.general = odometr.getResult();
    measurementData.wattMeter = (uint32_t)wattMeter.getResult();

    if (bicycle.receiveData.assistant != true)
      calories.measure(calculateCalories(userParameters, bicycle.receiveData.speed, bicycle.settings.pasLevel, bicycle.settings.C14_PasAdjustment));
    else
    {
      if (bicycle.receiveData.speed > 5)
        calories.measure(calculateCalories(userParameters, 5, 0, 0));
}
    measurementData.energyCaloriesTotal = calories.getResult();
    measurementData.burnFatTotal = measurementData.energyCaloriesTotal / 7.716;
  }
  return dataAvailable;
}