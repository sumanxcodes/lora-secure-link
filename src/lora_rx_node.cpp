// ===========================================
// FSM-based LoRa RX Node (Multi-peer support)
// ===========================================

#include <Arduino.h>
#include <LoRa.h>
#include "LoRaConfig.h"
#include "LoRaSetup.h"
#include "EEPROMReader.h"
#include "MessageUtils.h"
#include "NodeManager.h"
#include "ChallengeAuth.h"
#include "MessageHandlers.h"

// -------------------------------
// Global Variables and Constants
// -------------------------------

uint32_t ttl = 3;
String id;

static unsigned long lastPing = 0;
static unsigned long lastAckRetry = 0;

const unsigned long pingInterval = 4000;     // Interval for sending PINGs
const unsigned long ackRetryInterval = 3000; // Interval for retrying ACKs

// -------------------------------
// Utility Functions
// -------------------------------

/**
 * Returns current system uptime as HH:MM:SS.
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

/**
 * Sends a CLEAR broadcast to reset peer states on all nodes.
 */
void broadcastClear()
{
    String clearMsg = createMessage("CLEAR", id, "ALL", "RESET");
    LoRa.beginPacket();
    LoRa.print(clearMsg);
    LoRa.endPacket();
    Serial.println("📢 Broadcasted CLEAR to ALL peers");
}

// -------------------------------
// Arduino Setup Routine
// -------------------------------

void setup()
{
    Serial.begin(9600);
    while (!Serial)
    {
    } // Wait for serial to be ready

    if (!setupLoRa())
    {
        Serial.println("LoRa init failed");
        while (1)
        {
        } // Infinite loop if LoRa fails
    }

    id = loadDeviceIdFromEEPROM();
    uint32_t seed = loadSeedFromEEPROM();

    Serial.println("\n============= RX NODE =============");
    Serial.println("DEVICE_ID: " + id);
    Serial.println("Seed: " + String(seed));
    Serial.println("===================================\n");

    delay(500);       // Let LoRa settle
    broadcastClear(); // Clear network state
}

// -------------------------------
// Main Loop: FSM-driven behavior
// -------------------------------

void loop()
{
    unsigned long now = millis();

    // 🔁 Retry ACKs for peers who may have missed it
    if (now - lastAckRetry >= ackRetryInterval)
    {
        for (auto &peer : peers)
        {
            if (peer.pkReceived && peer.state == PeerState::SECURE_COMM)
            {
                String ack = createMessage("ACK", id, peer.id, "OK");
                LoRa.beginPacket();
                LoRa.print(ack);
                LoRa.endPacket();
                Serial.println("🔁 Retried ACK to " + peer.id);
            }
        }
        lastAckRetry = now;
    }

    // 📡 Periodically broadcast PING to discover peers
    if (now - lastPing >= pingInterval)
    {
        String pingMsg = createMessage("PING", id, "ALL", "Who is out there?");
        LoRa.beginPacket();
        LoRa.print(pingMsg);
        LoRa.endPacket();
        lastPing = now;
    }

    // 📩 Handle received LoRa packets
    if (LoRa.parsePacket())
    {
        String received = "";
        while (LoRa.available())
            received += (char)LoRa.read();

        LoRaMessage msg;
        if (received.startsWith("MSG:") || received.startsWith("RESP:") || received.startsWith("CHAL:"))
            msg = parseMessageWithTTL(received);
        else
            msg = parseMessage(received);

        // ✉️ Dispatch to appropriate handler based on message type
        if (msg.type == "PING")
        {
            handlePing(msg);
        }
        else if (msg.type == "PK")
        {
            handlePkExchange(msg);
        }
        else if (msg.type == "ACK")
        {
            handleAck(msg);
        }
        else if (msg.type == "CLEAR")
        {
            handleClear(msg);
        }
        else if (msg.type == "MSG")
        {
            handleMsg(msg);
        }
        else if (msg.type == "PONG")
        {
            // ⚙️ Begin DH key exchange after receiving PONG
            NodeState *peer = findOrCreatePeer(msg.senderId);
            if (!peer->pkSent)
            {
                peer->privateKey = generatePrivateKey();
                peer->publicKey = generatePublicKey(peer->privateKey);
                String pkMsg = createMessage("PK", id, msg.senderId, String(peer->publicKey));
                LoRa.beginPacket();
                LoRa.print(pkMsg);
                LoRa.endPacket();
                peer->pkSent = true;
            }
        }
        else if (msg.type == "CHAL")
        {
            // 🔐 Respond to challenge for authentication
            NodeState *peer = findOrCreatePeer(msg.senderId);
            handleAuthChallenge(peer, id, ttl); // Moved to modular function
        }
        else if (msg.type == "RESP")
        {
            // ✅ Verify challenge response
            NodeState *peer = findOrCreatePeer(msg.senderId);
            verifyAuthResponse(peer, msg.payload, msg.messageCount, id);
        }
    }
}
