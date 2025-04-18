#ifndef LORA_SETUP_H
#define LORA_SETUP_H

#include <SPI.h>
#include <LoRa.h>
#include "LoRaConfig.h"

bool setupLoRa()
{
    LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);

    if (!LoRa.begin(LORA_BAND))
    {
        Serial.println("LoRa init failed!");
        return false;
    }

    Serial.println("LoRa init succeeded.");
    return true;
}

#endif
