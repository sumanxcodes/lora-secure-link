#ifndef EEPROMREADER_H
#define EEPROMREADER_H

#include <Arduino.h>
#include <EEPROM.h>

inline String loadDeviceIdFromEEPROM()
{
    char id[20];
    for (uint8_t i = 0; i < sizeof(id) - 1; i++)
    {
        id[i] = EEPROM.read(i);
        if (id[i] == '\0')
            break;
    }
    id[sizeof(id) - 1] = '\0';
    return String(id);
}

inline uint32_t loadSeedFromEEPROM()
{
    uint32_t seed;
    EEPROM.get(20, seed);
    return seed;
}

#endif
