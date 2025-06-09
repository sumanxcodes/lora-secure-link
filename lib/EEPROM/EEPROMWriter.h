#ifndef EEPROM_WRITER_H
#define EEPROM_WRITER_H

#include <Arduino.h>
#include <EEPROM.h>

inline void writeDeviceIdToEEPROM(const String &deviceId)
{
    for (uint8_t i = 0; i < deviceId.length(); i++)
    {
        EEPROM.write(i, deviceId[i]);
    }
    EEPROM.write(deviceId.length(), '\0'); // Null-terminate
}

inline void writeSeedToEEPROM(uint32_t seedValue)
{
    EEPROM.put(20, seedValue);
}

#endif
