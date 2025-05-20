#include <Arduino.h>
#include <LoRa.h>
#include "LoRaConfig.h"
#include "LoRaSetup.h"
#include "EEPROMReader.h"
#include "MessageUtils.h" // for parseMessageWithTTL() and createMessageWithTTL()

String id;
uint32_t seed;

/**
 * Utility: Get current time as HH:MM:SS for logging.
 */
String currentTime()
{
    unsigned long now = millis() / 1000;
    int h = now / 3600;
    int m = (now % 3600) / 60;
    int s = now % 60;
    char buf[9];
    sprintf(buf, "%02d:%02d:%02d", h, m, s);
    return String(buf);
}

void setup()
{
    Serial.begin(9600);
    while (!Serial)
    {
    }

    if (!setupLoRa())
    {
        Serial.println("❌ LoRa init failed!");
        while (1)
        {
        } // Halt
    }

    id = loadDeviceIdFromEEPROM();
    seed = loadSeedFromEEPROM();

    Serial.println("\n============= RELAY NODE =============");
    Serial.println("DEVICE_ID: " + id);
    Serial.println("SEED: " + String(seed));
    Serial.println("======================================\n");

    // Optional: Seed PRNG if needed for relaying logic
    randomSeed(seed);
}

void loop()
{
    if (LoRa.parsePacket())
    {
        String received = "";
        while (LoRa.available())
            received += (char)LoRa.read();

        Serial.print("[");
        Serial.print(currentTime());
        Serial.println("] 📥 Received: " + received);

        // Parse message (supports MSG, CHAL, RESP with TTL and counter)
        LoRaMessage msg = parseMessageWithTTL(received);

        // Basic validity check
        if (msg.type == "INVALID")
        {
            Serial.println("⚠️ Invalid message format. Ignoring.");
            return;
        }

        // Only relay if TTL > 1
        if (msg.ttl > 1)
        {
            int newTTL = msg.ttl - 1;

            String relayed = createMessageWithTTL(
                msg.type,
                msg.senderId,   // Preserve original sender
                msg.receiverId, // Forward to original target
                newTTL,
                msg.messageCount, // Keep original counter
                msg.payload       // Unmodified encrypted or plain payload
            );

            // Re-broadcast
            LoRa.beginPacket();
            LoRa.print(relayed);
            LoRa.endPacket();

            Serial.print("🔁 Relayed with TTL=");
            Serial.println(newTTL);
        }
        else
        {
            Serial.println("⏹ TTL expired. Not relaying.");
        }
    }
}
