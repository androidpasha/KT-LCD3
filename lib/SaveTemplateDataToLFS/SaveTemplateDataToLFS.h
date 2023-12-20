#pragma once
#include <FS.h>
#include <LittleFS.h>

template <typename typeOfSavedData>
class SaveTemplateDataToLFS
{
  public:
    SaveTemplateDataToLFS(typeOfSavedData &savedData, const char *path)
    {
      LittleFS.begin();
      this->path = path;
      readFromFile(savedData);
    }

    bool writeToFile(const typeOfSavedData &savedData)
    {
      File file = LittleFS.open(path, "w");
      if (!file)
        return false;
      file.write((byte *)&savedData, sizeof(typeOfSavedData) / sizeof(byte));
      file.close();
      return true;
    }

    bool readFromFile(typeOfSavedData &savedData)
    {
      File file = LittleFS.open(path, "r");
      if (file.size() != sizeof(typeOfSavedData) or !file)
        return false;
      file.read((byte *)&savedData, file.size());
      file.close();
      return true;
    }

  private:
    const char *path;
};