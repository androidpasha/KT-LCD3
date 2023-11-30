#pragma once

#include "Arduino.h"

#define RECEIVE_BUFFER_SIZE 12
#define TRANSFER_BUFFER_SIZE 13
#define SERIALTIMEOUT 50
#define MINSPEED 2

typedef struct SettingsStruct
{
    uint8_t pasLevel : 3; // Range:0-3
    bool cruise;
    bool walkMode;
    bool frontLight;
    // float speedDivided;
    float powerFactor;
    uint8_t maxSpeedLimit;
    uint16_t wheelRimDiameter;         // 6-28 inch and 700C
    uint8_t P1_MotorFeature : 8;       // Range：1-255.
    uint8_t P2_WheelSetSpeedPulse : 3; // Rage:0-6
    bool P3_PasControlMode;
    bool P4_ThrottleActiveMode;
    uint8_t P5_BatteryMonitorModes : 6;  // Range:0-40
    uint8_t C1_PasSensorSensitivity : 3; // Range:0-7
    uint8_t C2_MotorPhase : 3;           // Range:0-7
    // uint8_t C3_PasRestartMemory : 4;     // Range: 0-8
    uint8_t C4_ThrottleFunction : 3;                   // Range:0-4
    uint8_t C4_SpeedLimitValueOfThrottle : 7;          // Range:0-127
    uint8_t C4_PercentageValueOfTheFirstGearSpeed : 6; // Range:0-100
    uint8_t C5_ControllerMaxCurrent : 4;               // Range:0-10
    // uint8_t C6_LcdScreenBrightness:3; // Range:1-5
    // bool C7_CruiseControlFunction;
    // bool C8_MotorWorkingTemperature;
    // bool C9_LcdRestartPasswordFunction;
    // uint16_t LcdPasword:6; //Range:0-999
    // bool C10_LcdReset;
    // uint8_t C11_LcdFeature : 2;
    uint8_t C12_ControllerLowestVoltage : 3; // Range:0-7
    uint8_t C13_ControllerAbsBraking : 3;    // Range:0-5
    uint8_t C14_PasAdjustment : 2;           // Range:1-3

    float onePeriodDistance() // ДОБАВИЛ КОД ЧТОБ НЕ ПЕРЕСЧИТЫВАЛО КАЖДЫЙ РАЗ. ПРОВЕРИТЬ!!!!
    {
        static float result = 0;
        static uint16_t previousDiameter = 0;
        if (previousDiameter != wheelRimDiameter)
        {
            previousDiameter = wheelRimDiameter;
            float diametrInc = (float)wheelRimDiameter;
            if (wheelRimDiameter == 700)
                diametrInc = 27.5;
            result = PI * diametrInc * 0.0254f;
        }
        return result;
    }

    uint8_t wheelRimDiameterHEX()
    {
        const uint16_t wheelSize[] = {5, 6, 8, 10, 12, 14, 16, 18, 20, 23, 24, 26, 28, 29, 700};
        const uint8_t wheelSizeHEX[] = {0x16, 0x12, 0x0A, 0x0E, 0x02, 0x06, 0x00, 0x04, 0x08, 0x0C, 0x10, 0x14, 0x1C, 0x1E, 0x18};
        for (uint8_t i = 0; i < sizeof(wheelSize) / sizeof(wheelSize[0]); i++)
            if (wheelRimDiameter == wheelSize[i])
                return wheelSizeHEX[i];
        return 0x10; // default 24`
    }

} SettingsStruct;

typedef struct ReceiveDataStruct
{
    uint32_t speedPeriod;
    float speed;
    uint16_t power;
    uint16_t temperature;
    bool throttle;
    bool cruise;
    bool assistant;
    byte brake;
    uint8_t error;        //(0x20 or 0x25 or 0x28): "0info", 0x21: "6info", 0x22: "1info", 0x23: "2info", 0x24: "3info",  0x26: "4info"
    uint8_t ratedVoltage; // 24,36,48V
    uint8_t batteryLevel; // 0: empty, 1: border flashing, 2: charging, 3: empty, 4: 1 bar, 8: 2 bars, 16: full
    uint8_t pasData;
    int8_t pasDataForCRC;
    uint8_t batteryPercent()
    {
        uint8_t byteToPercent[2][6] = {
            {0, 1, 3, 4, 8, 16}, // 0: empty, 1: border flashing, 2: charging, 3: empty, 4: 1 bar, 8: 2 bars, 16: full
            {0, 10, 25, 50, 75, 100}};
        for (int i = 0; i < 6; i++)
            if (batteryLevel == byteToPercent[0][i])
                return byteToPercent[1][i];
        return 0;
    }

} ReceiveDataStruct;

class KTLCD_Display
{
public:
    KTLCD_Display(SettingsStruct *settings_variable_to_remove)
    {
        Serial.setTimeout(SERIALTIMEOUT);
        settings = *settings_variable_to_remove;
        delete[] settings_variable_to_remove;
        settings_variable_to_remove = nullptr;
    }

    KTLCD_Display()
    {
        Serial.setTimeout(SERIALTIMEOUT);
    }

    ReceiveDataStruct receiveData;
    SettingsStruct settings;
    //   String testString = "";
    ~KTLCD_Display()
    {
        delete[] receiveBuffer;
        delete[] transferBuffer;
    }

    void sendSettingsToController()
    {
        // Проверить ИФ!!!!!
        if ((receiveData.cruise == true) && (receiveData.throttle == true) && (receiveData.assistant == true))
            settings.cruise = false;
        fillTransferBuffer();
        sendBufferToController(transferBuffer, TRANSFER_BUFFER_SIZE);
    }

    bool dataAvailable()
    {
        if ((bool)Serial.available() == true)
        {
            return getReceivedBufferFromController();
        }
        return false;
    }

private:
    uint8_t *receiveBuffer = new uint8_t[RECEIVE_BUFFER_SIZE];
    uint8_t *transferBuffer = new uint8_t[TRANSFER_BUFFER_SIZE];

    bool getReceivedBufferFromController()
    {
        if (serialRead(receiveBuffer, RECEIVE_BUFFER_SIZE) == true)
        {
            getDataFromReceiveBuffer(receiveBuffer, RECEIVE_BUFFER_SIZE);
            return true;
        }
        return false;
    }

    void fillTransferBuffer()
    {

#define BYTE transferBuffer
#define parameter settings

        uint8_t SizeHEX = parameter.wheelRimDiameterHEX();

        BYTE[0] = parameter.P5_BatteryMonitorModes;

        BYTE[1] = (parameter.pasLevel & 0x07);
        bitWrite(BYTE[1], 6, parameter.walkMode);
        bitWrite(BYTE[1], 7, parameter.frontLight);

        BYTE[2] = ((parameter.maxSpeedLimit - 10) << 3 & 0xF8) |
                  (SizeHEX >> 2 & 0x07);

        BYTE[3] = parameter.P1_MotorFeature;

        BYTE[4] = (SizeHEX << 6 & 0xC0) |
                  (parameter.P2_WheelSetSpeedPulse & 0x07) |
                  (parameter.P3_PasControlMode << 3 & 0x08) |
                  (parameter.P4_ThrottleActiveMode << 4 & 0x10);
        bitWrite(BYTE[4], 5, bitRead(parameter.maxSpeedLimit - 10, 5));

        BYTE[6] = (parameter.C1_PasSensorSensitivity << 3 & 0x38) |
                  (parameter.C2_MotorPhase & 0x07);

        BYTE[7] = (parameter.C5_ControllerMaxCurrent & 0x0F) |
                  (parameter.C14_PasAdjustment << 5 & 0x60);

        BYTE[8] = ((parameter.C4_ThrottleFunction + 1) << 5 & 0xE0) |
                  (parameter.C12_ControllerLowestVoltage & 0x07) |
                  (parameter.cruise << 4 & 0x10);

        BYTE[9] = parameter.C4_SpeedLimitValueOfThrottle & 0x7F;

        BYTE[10] = (parameter.C13_ControllerAbsBraking << 2 & 0x1C);

        BYTE[11] = parameter.C4_PercentageValueOfTheFirstGearSpeed & 0x3F;

        BYTE[12] = 0x0E;

        BYTE[5] = XOR(transferBuffer, TRANSFER_BUFFER_SIZE, 5) ^ 2;
        //  BYTE[0] ^ BYTE[1] ^ BYTE[2] ^ BYTE[3] ^ BYTE[4] ^ BYTE[6] ^ BYTE[7] ^ BYTE[8] ^ BYTE[9] ^ BYTE[10] ^ BYTE[11] ^ BYTE[12] ^ 2;

#undef BYTE
#undef parameter
    }

    uint8_t XOR(uint8_t *buffer, size_t bufferSize, uint8_t placeXor)
    {
        uint8_t XOR = 0;
        for (uint8_t i = 0; i < bufferSize; i++)
        {
            if (i == placeXor)
                continue;
            XOR ^= buffer[i];
        }
        return XOR;
    }

    bool serialRead(uint8_t *buffer, size_t bufferSize = RECEIVE_BUFFER_SIZE)
    {
        Serial.readBytes(buffer, bufferSize);

        uint16_t bufferSum = 0;
        for (uint8_t i = 0; i < bufferSize; i++)
            bufferSum += buffer[i];

        //  CRC зависит от последнего байта.
        //  Если B11 <65 и B11 четное то B6 = CRC расч ^(B11+65)
        //  Если B11 <65 и B11 НЕ четное то B6 = CRC расч ^(B11+63)
        //  Если B11 >65 и B11 четное то B6 = CRC расч ^(B11-63)
        //  Если B11 >65 и B11 НЕ четное то B6 = CRC расч ^(B11-65)
        //  Если B11 == 65 то B6 == CRC расч
        int8_t added;
        if (buffer[11] < 65)
            added = 63 + (!(buffer[11] % 2)) * 2;
        else
            added = -63 - (buffer[11] % 2) * 2;

        uint8_t XOR11BYTE = XOR(buffer, bufferSize - 1, 6);
        XOR11BYTE ^= (buffer[11] + added);

        if (XOR11BYTE != buffer[6] or bufferSum == 0) // последний байт буфера не рассчитывается в XOR, поэтому bufferSize - 1
            return false;

        receiveData.pasData = buffer[11];
        receiveData.pasDataForCRC = buffer[11] + added;

        return true;
    }

    void getDataFromReceiveBuffer(uint8_t *buffer, size_t bufferSize = RECEIVE_BUFFER_SIZE)
    {
#define BYTE buffer
        receiveData.batteryLevel = BYTE[1];
        receiveData.ratedVoltage = BYTE[2];
        receiveData.speedPeriod = (BYTE[3] << 8) | BYTE[4]; // BYTE[3] * 256 + BYTE[4];
        receiveData.speed = settings.onePeriodDistance() * 3600 / receiveData.speedPeriod;
        if (receiveData.speed < MINSPEED)
            receiveData.speed = 0;
        receiveData.error = BYTE[5];
        receiveData.throttle = !bitRead(BYTE[7], 0);
        receiveData.cruise = bitRead(BYTE[7], 3);
        receiveData.assistant = bitRead(BYTE[7], 4);
        receiveData.brake = bitRead(BYTE[7], 5);
        receiveData.power = BYTE[8] * settings.powerFactor;
        receiveData.temperature = (int8_t)BYTE[9] + 15;

#undef BYTE
    }

    void sendBufferToController(uint8_t *buffer, size_t bufferSize = TRANSFER_BUFFER_SIZE)
    {
        Serial.write(buffer, bufferSize);
    }
};