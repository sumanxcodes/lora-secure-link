#include <Arduino.h>
#include <LoRa.h>
#include "LoRaConfig.h"
#include "LoRaSetup.h"
#include "EEPROMReader.h"
#include "MessageUtils.h"

// -------------------------------
// Device and Message Identity
// -------------------------------

String id;
uint32_t seed;

// -------------------------------
// Seen message memory (per TTL)
// -------------------------------

struct SeenMessage
{
    String senderId;
    uint32_t messageCount;
    int ttl;
};

#define MAX_SEEN 30
SeenMessage seenMessages[MAX_SEEN];
int seenIndex = 0;

bool isDuplicate(String senderId, uint32_t msgCount, int ttl)
{
    for (int i = 0; i < MAX_SEEN; i++)
    {
        if (seenMessages[i].senderId == senderId &&
            seenMessages[i].messageCount == msgCount &&
            seenMessages[i].ttl == ttl)
        {
            return true;
        }
    }
    return false;
}

void markMessageSeen(String senderId, uint32_t msgCount, int ttl)
{
    seenMessages[seenIndex] = {senderId, msgCount, ttl};
    seenIndex = (seenIndex + 1) % MAX_SEEN;
}

// -------------------------------
// Track relayed TTLs (used to cancel others)
// -------------------------------

struct SeenTTL
{
    String senderId;
    uint32_t messageCount;
    int ttl;
};

#define MAX_SEEN_TTL 30
SeenTTL seenLowerTTLs[MAX_SEEN_TTL];
int seenTTLIndex = 0;

bool hasSeenLowerTTL(String senderId, uint32_t msgCount, int ttl)
{
    for (int i = 0; i < MAX_SEEN_TTL; i++)
    {
        if (seenLowerTTLs[i].senderId == senderId &&
            seenLowerTTLs[i].messageCount == msgCount &&
            seenLowerTTLs[i].ttl == ttl)
        {
            return true;
        }
    }
    return false;
}

void markLowerTTLSeen(String senderId, uint32_t msgCount, int ttl)
{
    seenLowerTTLs[seenTTLIndex] = {senderId, msgCount, ttl};
    seenTTLIndex = (seenTTLIndex + 1) % MAX_SEEN_TTL;
}

// -------------------------------
// Pending relay
// -------------------------------

struct PendingRelay
{
    String senderId;
    uint32_t messageCount;
    int ttl;
    String packet;
    unsigned long sendTime;
    bool valid;
};

PendingRelay pending;

// -------------------------------
// Utility: Time formatting
// -------------------------------

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

// -------------------------------
// Arduino Setup
// -------------------------------

void setup()
{
    Serial.begin(9600);
    // while (!Serial)
    // {
    // }

    if (!setupLoRa())
    {
        Serial.println("❌ LoRa init failed!");
        while (1)
        {
        }
    }

    id = loadDeviceIdFromEEPROM();
    seed = loadSeedFromEEPROM();

    Serial.println("\n============= RELAY NODE =============");
    Serial.println("DEVICE_ID: " + id);
    Serial.println("SEED: " + String(seed));
    Serial.println("======================================\n");

    randomSeed(seed);
}

// -------------------------------
// Arduino Loop
// -------------------------------

void loop()
{
    unsigned long now = millis();

    // 1. Check and possibly send pending relay
    if (pending.valid && now >= pending.sendTime)
    {
        // Only send if we have NOT seen the TTL-1 version already
        if (!hasSeenLowerTTL(pending.senderId, pending.messageCount, pending.ttl))
        {
            LoRa.beginPacket();
            LoRa.print(pending.packet);
            LoRa.endPacket();

            Serial.print("[");
            Serial.print(currentTime());
            Serial.print("] 🔁 Relayed from ");
            Serial.print(pending.senderId);
            Serial.print(" | TTL=");
            Serial.print(pending.ttl);
            Serial.print(" | msgCount=");
            Serial.println(pending.messageCount);
        }
        else
        {
            Serial.print("[");
            Serial.print(currentTime());
            Serial.print("] ⏸ Skipped redundant relay from ");
            Serial.print(pending.senderId);
            Serial.print(" | TTL=");
            Serial.print(pending.ttl);
            Serial.print(" | msgCount=");
            Serial.println(pending.messageCount);
        }

        pending.valid = false;
    }

    // 2. Listen for LoRa messages
    if (LoRa.parsePacket())
    {
        String received = "";
        while (LoRa.available())
            received += (char)LoRa.read();

        LoRaMessage msg = parseMessageWithTTL(received);

        // Track what TTLs we hear for suppression
        markLowerTTLSeen(msg.senderId, msg.messageCount, msg.ttl);

        if (msg.type != "INVALID" && msg.ttl > 0)
        {
            if (isDuplicate(msg.senderId, msg.messageCount, msg.ttl))
            {
                // Cancel if we already scheduled this
                if (pending.valid &&
                    pending.senderId == msg.senderId &&
                    pending.messageCount == msg.messageCount &&
                    pending.ttl == msg.ttl)
                {
                    pending.valid = false;
                }
                return;
            }

            // Mark this TTL level as seen
            markMessageSeen(msg.senderId, msg.messageCount, msg.ttl);

            // Prepare next TTL packet
            int newTTL = msg.ttl - 1;

            if (newTTL > 0)
            {
                String relayed = createMessageWithTTL(
                    msg.type,
                    msg.senderId,
                    msg.receiverId,
                    newTTL,
                    msg.messageCount,
                    msg.payload);

                // Schedule it
                pending = {
                    msg.senderId,
                    msg.messageCount,
                    newTTL,
                    relayed,
                    now + random(300, 1000),
                    true};
            }
            else
            {
                Serial.print("[");
                Serial.print(currentTime());
                Serial.print("] ⏹ TTL expired for msgCount=");
                Serial.print(msg.messageCount);
                Serial.print(" from ");
                Serial.println(msg.senderId);
            }
        }
    }
}
