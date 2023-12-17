// Остання версія коду знаходиться у репозиторії: https://github.com/androidpasha/KT-LCD3
#pragma region offsetof //includes libs
#include <Arduino.h>

// библиотеки собственной разработки:
#include "KTLCD_Display.h"
#include "AverageValue.h"
#include "TimeBasedCounter.h"
#include "SaveTemplateDataToLFS.h"
#include "struct.h"
#include "webSocketFunction.h" // функции веб сокета вынесены в отдельный файл
// конец списка библиотек собственной разработки

#include "Ticker.h"
#include <ESP8266WiFi.h> // Библиотека для создания Wi-Fi подключения (клиент или точка доступа)
#include <DNSServer.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <FS.h> // Библиотека для работы с файловой системой
#include <LittleFS.h>
#pragma endregion
#pragma region offsetof //constants
#define MET_CALCULATION_CALORIES // підрахунок калорій через MET еквівалент коли розкоментований рядок або попередньою формулою, якщо закоментувати
// #define SERIAL1                  // Закомментировать для ESP01 и если будет использоваться обычный Serial (0)
#define MINIMUM_TIME_BETWEEN_WRITES_TO_THE_FILE_SYSTEM_SEC 60
const char *ssidAP = "Bicycle";           // Название генерируемой точки доступа
const float passPercent[3][6] PROGMEM = { // таблица відсотків допомоги PAS згідно параметру С14
                                          // {0, 6, 17, 30, 45, 80},  масив у відсотках з інструкції
                                          // {0, 12, 22, 40, 55, 80},
                                          // {0, 17, 30, 45, 66, 80}};

    {1, 0.94, 0.83, 0.70, 0.55, 0.20}, // масив відсотків переведений у коефіцієнти за формулою
    {1, 0.88, 0.78, 0.60, 0.45, 0.20}, // 100 - passPercent[pasAdjustment - 1][pasLevel]) / 100.0
    {1, 0.83, 0.70, 0.55, 0.34, 0.20}};

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
DNSServer dnsServer; // DNS сервер для перехода на главную страницу с http://bicycle/
AsyncWebServer webServer(80);
AsyncWebSocket webSocket("/ws");
SaveTemplateDataToLFS<SettingsStruct> SaveControllerDataToLFS(bicycle.settings, dataStorageFileNameControllerSettings);
SaveTemplateDataToLFS<SaveToFsStruct> SaveMeasureDataToLFS(measurementData, dataStorageFileNameOdometr);
SaveTemplateDataToLFS<User> SaveUserDataToLFS(userParameters, dataStorageFileNameUser);
#pragma endregion
#pragma region offsetof //variables
bool scheduleFlag = false;
uint32_t lastSaveToFsTime = 0;
bool saveFlagMeasurementDataToFile = false;
#pragma endregion
#pragma region offsetof //function declarations
float calculateCalories(const User &user, float velocity, uint8_t pasLevel, uint8_t pasAdjustment);
void IRAM_ATTR every100ms();
bool bicycleIsAvailableData();
#pragma endregion

void setup()
{
  Serial.begin(9600); // Швидкість лінії контроллеру 9600 бод
#ifdef SERIAL1
  Serial.swap(); // Переключаем аппаратный serial c gpio3, gpio1 на gpio13, gpio15
#endif
  //  LittleFS.begin(); // Инициализируем работу с файловой системой/ это сделано в SaveTemplateDataToLFS

  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssidAP, "", 1, 0, 1);                              // (SSID, NOT PASS, Channel, no hidden ssid, max conenction)
  dnsServer.start(53, "bicycle", WiFi.softAPIP());               // Переходим с любой страницы на IP точки доступа ESP8266
  webServer.on("/", HTTP_GET, [](AsyncWebServerRequest *request) // откр index.html при запросе корня сервера
               { request->send(LittleFS, "/index.html", "text/html"); });
  // відправляємо файли на HTTP запити з ФС
  webServer.serveStatic("/", LittleFS, "/");
  // Инициализируем Web-сервер
  webServer.begin();
  webServer.addHandler(&webSocket);
  // Функция обработки запросов
  webSocket.onEvent(webSocketEvent);
  // Устанавливаем начальное значение счетчиков
  odometr.reset(measurementData.odoGeneral);
  wattMeter.reset(measurementData.wattMeter);
  calories.reset(measurementData.caloriesTotal);
  // обнуляем счетчики поездки после включения
  measurementData.odoDrive = measurementData.odoGeneral;
  measurementData.caloriesDrive = measurementData.caloriesTotal;
  // Вызываем функцию every100ms каждые 100 мс.
  scheduler_100ms.attach_ms(100, every100ms);
}

void loop()
{
  dnsServer.processNextRequest(); // Переходим с любой страницы на IP точки доступа ESP8266

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
  }
}

void IRAM_ATTR every100ms()
{
  scheduleFlag = true;
}

bool bicycleIsAvailableData()
{
  bool dataAvailable = bicycle.dataAvailable();

  if (dataAvailable == true)
  {
    if (bicycle.receiveData.speed >= 3.0f)
      averageSpeed.measure(bicycle.receiveData.speed);
    odometr.measure(bicycle.receiveData.speed);
    wattMeter.measure((float)bicycle.receiveData.power);

    float velocity = bicycle.receiveData.speed;
    if (bicycle.receiveData.throttle == true and velocity < 5)
      velocity = 5;
    float cal = calculateCalories(userParameters, velocity, bicycle.settings.pasLevel, bicycle.settings.C14_PasAdjustment);
    calories.measure(cal);

    measurementData.odoGeneral = odometr.getResult();
    measurementData.wattMeter = wattMeter.getResult();
    measurementData.caloriesTotal = calories.getResult();
  }
  return dataAvailable;
}

float calculateCalories(const User &user, float velocity, uint8_t pasLevel, uint8_t pasAdjustment)
{
#ifdef MET_CALCULATION_CALORIES
  static const float BMR = // базові витрати калорій людиною у спокої за добу розраховуються один раз
      (10.0 * user.weight + 6.25 * user.height - 5.0 * user.age + 5.0 * !user.sex - 161.0 * user.sex);
  float МЕТ = 0.0086 * velocity * velocity + 0.1167 * velocity + 1.95;                     // Метаболічний еквівалент:
  float DVK = BMR * (МЕТ - 1) * pgm_read_float(&passPercent[pasAdjustment - 1][pasLevel]); // додаткові витрати калорій людиною під час рівномірного фізичного навантаження (велосипед) без БВК з урахуванням допомоги PAS, за добу
  return (BMR + DVK) * userParameters.calCorrectFactor / 24;                                                                 // за годину
#else
  // Стара формула E = BMR * 0.34 * velocity + 1000.0, ккал/добу
  static const float BMR = (10.0 * user.weight + 6.25 * user.height - 5.0 * user.age + 5.0 * !user.sex - 161.0 * user.sex); // базові витрати калорій людиною у спокої за добу
  if (velocity < 1)                                                                                                         // Формула для катання а не для відпочинку!!!
    velocity = 1;
  float DVK = (BMR * 0.34 * velocity + 1000.0) - BMR;               // додаткові витрати калорій людиною під час рівномірного фізичного навантаження (велосипед) за добу
  DVK *= pgm_read_float(&passPercent[pasAdjustment - 1][pasLevel]); // урахування значення відсотків допомоги PAS
  float energy = BMR + DVK;
  energy *= userParameters.calCorrectFactor; // Поправка від користувача
  return energy / 24;                        // за годину
#endif
}
