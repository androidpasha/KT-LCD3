#include "SaveTemplateDataToLFS.h"

typedef struct settings {
  int setting1 = 0;
  int setting2 = 0;
  void print() {
    Serial.printf("setting1=%d, setting2=%d\n", setting1, setting2);
  }
} settings;

const char *settingsFileName PROGMEM = "/settings.bin";
settings mySettings;
SaveTemplateDataToLFS<settings> LFS_DATA(mySettings, settingsFileName);

int a;
SaveTemplateDataToLFS<int> LFS_DATAint(a, "/settingsint.bin");


void setup() {
  Serial.begin(9600);
  mySettings.print();
  mySettings.setting1++;
  mySettings.setting2++;
  LFS_DATA.writeToFile(mySettings);

  a++;
  LFS_DATAint.writeToFile(a);
  Serial.printf("a=%d\n", a);

}
void loop() {
  yield();
}
