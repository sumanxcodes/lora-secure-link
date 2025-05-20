// ===========================================
// FSM-based LoRa TX Node (Multi-peer support)
// ===========================================

#include <Arduino.h>
#include <LoRa.h>
#include "LoRaConfig.h"
#include "LoRaSetup.h"
#include "EEPROMReader.h"
#include "MessageUtils.h"
#include "DHExchange.h"
#include "NodeManager.h"
#include "EncryptionUtils.h"
#include "ChallengeAuth.h"

// -------------------------------
// Global Variables and Constants
// -------------------------------

String id;
uint32_t ttl = 3; // TTL value for messages (used in flooding or expiry control)

unsigned long lastMessageSent = 0;
const unsigned long messageInterval = 10000; // 10s between messages

unsigned long lastAckRetry = 0;
const unsigned long ackRetryInterval = 5000; // Retry ACKs every 5s

const int lightSensorPin = A0; // Analog light sensor input

// -------------------------------
// Utility Functions
// -------------------------------

/**
 * Sends a CLEAR broadcast message to all nodes to reset their state.
 */
void resetTx(String id)
{
    String msg = createMessage("CLEAR", id, "ALL", "RESET");
    LoRa.beginPacket();
    LoRa.print(msg);
    LoRa.endPacket();
}

/**
 * Returns the current system time in HH:MM:SS format.
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

// -------------------------------
// Arduino Setup Routine
// -------------------------------

void setup()
{
    Serial.begin(9600);
    while (!Serial)
    {
    } // Wait for Serial

    if (!setupLoRa())
    {
        Serial.println("LoRa init failed");
        while (1)
        {
        } // Halt system
    }

    id = loadDeviceIdFromEEPROM();
    uint32_t seed = loadSeedFromEEPROM();

    Serial.println("\n============= TX NODE =============");
    Serial.println("DEVICE_ID: " + id);
    Serial.println("Seed: " + String(seed));
    Serial.println("===================================\n");

    delay(100);  // Let LoRa settle
    resetTx(id); // Signal CLEAR to network
}

// -------------------------------
// Main Loop: FSM-driven behavior
// -------------------------------

void loop()
{
    unsigned long now = millis();

    // -------------------------------
    // 🔁 Retry pending ACKs for peers
    // -------------------------------
    if (now - lastAckRetry >= ackRetryInterval)
    {
        for (auto &peer : peers)
        {
            if (peer.pkReceived && peer.state == PeerState::ACK_PENDING)
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

    // ----------------------------------
    // 📤 Send encrypted data to peers
    // ----------------------------------
    if (now - lastMessageSent >= messageInterval)
    {
        for (auto &peer : peers)
        {
            if (peer.state == PeerState::AUTHENTICATED)
            {
                String sensorReading = String(analogRead(lightSensorPin)); // Sample data
                String encryptedPayload = encryptString(sensorReading, peer.sharedSessionKey, peer.messageCount);
                String msg = createMessageWithTTL("MSG", id, peer.id, ttl, peer.messageCount, encryptedPayload);
                peer.messageCount++;

                LoRa.beginPacket();
                LoRa.print(msg);
                LoRa.endPacket();

                Serial.println("Plain Text Message: " + sensorReading);
                Serial.println("Encrypted Message: " + encryptedPayload);
                Serial.print("[");
                Serial.print(currentTime());
                Serial.print("] 🔐 Sent encrypted message to ");
                Serial.print(peer.id);
                Serial.print(" [msgCount=");
                Serial.print(peer.messageCount - 1);
                Serial.println("]");
                        }
        }
        lastMessageSent = now;
    }

    // --------------------------------
    // 📩 Handle incoming LoRa packets
    // --------------------------------
    if (LoRa.parsePacket())
    {
        String received = "";
        while (LoRa.available())
            received += (char)LoRa.read();

        LoRaMessage msg;
        if (received.startsWith("MSG:") || received.startsWith("CHAL:") || received.startsWith("RESP:"))
            msg = parseMessageWithTTL(received);
        else
            msg = parseMessage(received);

        // ---------------------
        // PING/PONG handshake
        // ---------------------
        if (msg.type == "PING")
        {
            String pong = createMessage("PONG", id, msg.senderId, "READY");
            LoRa.beginPacket();
            LoRa.print(pong);
            LoRa.endPacket();
        }

        // ---------------------
        // Diffie-Hellman Key Exchange
        // ---------------------
        else if (msg.type == "PK")
        {
            NodeState *peer = findOrCreatePeer(msg.senderId);

            if (isPeerDHComplete(peer->id))
                return;

            peer->remotePublicKey = msg.payload.toInt();
            peer->pkReceived = true;

            if (peer->privateKey == 0 || peer->publicKey == 0)
            {
                peer->privateKey = generatePrivateKey();
                peer->publicKey = generatePublicKey(peer->privateKey);
            }

            String pkMsg = createMessage("PK", id, msg.senderId, String(peer->publicKey));
            LoRa.beginPacket();
            LoRa.print(pkMsg);
            LoRa.endPacket();

            peer->pkSent = true;
            peer->state = PeerState::ACK_PENDING;

            // Send ACK immediately
            String ackMsg = createMessage("ACK", id, msg.senderId, "OK");
            LoRa.beginPacket();
            LoRa.print(ackMsg);
            LoRa.endPacket();

            // Derive shared session key
            if (peer->sharedSessionKey == 0 &&
                peer->remotePublicKey != 0 &&
                peer->privateKey != 0 &&
                peer->pkReceived && peer->pkSent)
            {
                peer->sharedSessionKey = generateSharedKey(peer->remotePublicKey, peer->privateKey);
            }

            markPeerAckReceived(peer->id);
        }

        // ---------------------
        // Acknowledgement handler
        // ---------------------
        else if (msg.type == "ACK")
        {
            NodeState *peer = findOrCreatePeer(msg.senderId);
            bool wasAckMissing = !peer->ackReceived;
            markPeerAckReceived(peer->id);

            if (peer->pkReceived && peer->state == PeerState::ACK_PENDING && wasAckMissing)
            {
                String ack = createMessage("ACK", id, msg.senderId, "OK");
                LoRa.beginPacket();
                LoRa.print(ack);
                LoRa.endPacket();
            }

            if (peer->sharedSessionKey == 0 &&
                peer->privateKey != 0 &&
                peer->remotePublicKey != 0 &&
                peer->pkSent && peer->pkReceived)
            {
                peer->sharedSessionKey = generateSharedKey(peer->privateKey, peer->remotePublicKey);
            }

            if (isPeerDHComplete(peer->id) && peer->state != PeerState::SECURE_COMM)
            {
                peer->state = PeerState::SECURE_COMM;
                printPeerStatus();
            }
        }

        // ---------------------
        // CLEAR message to reset peer state
        // ---------------------
        else if (msg.type == "CLEAR")
        {
            if (msg.receiverId == "ALL" || msg.receiverId == id)
            {
                Serial.println("⚠️  Received CLEAR from " + msg.senderId + ". Removing peer.");
                NodeState *peer = findOrCreatePeer(msg.senderId);
                resetPeer(peer);
            }
        }

        // ---------------------
        // Challenge-Response: CHAL
        // ---------------------
        else if (msg.type == "CHAL")
        {
            NodeState *peer = findOrCreatePeer(msg.senderId);

            Serial.println("📥 Raw LoRa Message: " + received);
            Serial.println("Session Key: " + String(peer->sharedSessionKey));
            Serial.println("Message Count: " + String(msg.messageCount));

            String decryptedChallenge = decryptString(msg.payload, peer->sharedSessionKey, msg.messageCount);
            Serial.println("📥 CHAL Decrypted: " + decryptedChallenge);

            // Compute and prepare response
            uint32_t responseValue = decryptedChallenge.toInt() + 1;
            String responseStr = String(responseValue);

            peer->messageCount = msg.messageCount + 1;

            String encryptedResponse = encryptString(responseStr, peer->sharedSessionKey, peer->messageCount);

            String respMsg = createMessageWithTTL("RESP", id, peer->id, 3, peer->messageCount, encryptedResponse);
            LoRa.beginPacket();
            LoRa.print(respMsg);
            LoRa.endPacket();

            Serial.println("🔐 Sent RESP to " + peer->id + ": " + responseStr);
        }

        // ---------------------
        // Authentication result
        // ---------------------
        else if (msg.type == "AUTH_SUCCESS")
        {
            NodeState *peer = findOrCreatePeer(msg.senderId);
            if (msg.payload == "OK")
            {
                Serial.println("✅ Received AUTH success from " + msg.senderId);
                peer->state = PeerState::AUTHENTICATED;
            }
        }
    }
}
