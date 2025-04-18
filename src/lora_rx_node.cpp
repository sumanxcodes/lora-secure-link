// lora_rx_node.cpp
#include <Arduino.h>
#include <LoRa.h>
#include "LoRaConfig.h"
#include "LoRaSetup.h"
#include "EEPROMReader.h"

void setup()
{
    Serial.begin(9600);
    while (!Serial)
        ;

    if (!setupLoRa())
    {
        Serial.println("LoRa init failed");
        while (1)
            ;
    }

    String id = loadDeviceIdFromEEPROM();
    uint32_t seed = loadSeedFromEEPROM();

    Serial.println("Loaded DEVICE_ID: " + id);
    Serial.println("Loaded Seed: " + String(seed));
}

void loop()
{
    if (LoRa.parsePacket())
    {
        String msg = "";
        while (LoRa.available())
            msg += (char)LoRa.read();
        Serial.println("Received: " + msg);
    }
}
