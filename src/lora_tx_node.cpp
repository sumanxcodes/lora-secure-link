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

    Serial.println("[TX] Receiver ready.");
}

void loop()
{
    String msg = "Hello from TX!";
    LoRa.beginPacket();
    LoRa.print(msg);
    LoRa.endPacket();
    Serial.println("Sent: " + msg);
    delay(5000); // Send every 5 seconds
}
