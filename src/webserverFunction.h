#pragma once
#include <ESP8266WebServer.h>
#include <FS.h>
#include <LittleFS.h>
extern ESP8266WebServer webServer;

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

void WebServer() // Обработка HTTP-запросов
{
  webServer.on("/settings", HTTP_GET, []()
               {
                 // Выполняем перенаправление на другой URL
                 webServer.sendHeader("Location", "/settings/settings.html");
                 webServer.send(302);                                         // Отправляем статус "302 Found" для перенаправления
               });

  // webServer.on(F("/Request"), HTTP_POST, []() // Тут принимаем пользовательские данные от страницы
  //              { webServer.send(200, "Content-Type: application/json", "RequestOk"); });

  webServer.onNotFound([]() {             // Описываем действия при событии "Не найдено"
    if (!handleFileRead(webServer.uri())) // Если функция handleFileRead (описана ниже) возвращает значение false в ответ на поиск файла в файловой системе
     handleFileRead("/");                // То возвращаем index.html
  });
}