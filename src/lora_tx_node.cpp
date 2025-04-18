// lora_tx_node.cpp
#include <Arduino.h>
#include <LoRa.h>
#include "LoRaConfig.h"
#include "LoRaSetup.h"

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

    Serial.println("[RX] Central receiver ready.");
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
