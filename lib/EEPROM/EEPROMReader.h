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
    String result = String(id);

    // hadling error if empty or invalid prefix
    if (result.length() < 4 || !(result.startsWith("TX") || result.startsWith("RX")))
    {
        return "INVALID_ID";
    }

    return result;
}

inline uint32_t loadSeedFromEEPROM()
{
    uint32_t seed;
    EEPROM.get(20, seed);
    // Handling error
    if (seed == 0 || seed == 0xFFFFFFFF)
    {
        return 0; // returns 0 as invalid seed
    }
    return seed;
}

#endif
