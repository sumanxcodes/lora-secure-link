// ===========================================
// FSM-based LoRa TX Node (Multi-peer support)
// ===========================================

#include <Arduino.h>
#include <LoRa.h>
#include "LoRaConfig.h"
#include "LoRaSetup.h"
#include "EEPROMReader.h"
#include "EEPROMWriter.h"
#include "MessageUtils.h"
#include "DHExchange.h"
#include "NodeManager.h"
#include "EncryptionUtils.h"
#include "ChallengeAuth.h"

// -------------------------------
// Global Variables and Constants
// -------------------------------

String id;
uint32_t seed;
uint32_t ttl = 5; // TTL value for messages (used in flooding or expiry control)

unsigned long lastMessageSent = 0;
const unsigned long messageInterval = 20000; // 10s between messages

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

    // This allows to receive a serial command from dashboard and read data from eeprom
    if (Serial.available())
    {
        String input = Serial.readStringUntil('\n');
        input.trim();
        if (input == "READ_EEPROM")
        {
            String id = loadDeviceIdFromEEPROM();
            uint32_t seed = loadSeedFromEEPROM();
            Serial.println("ID:" + id + ",SEED:" + String(seed));
        }
        else if (input.startsWith("WRITE_INFO:"))
        {
            String payload = input.substring(String("WRITE_INFO:").length());
            int commaIndex = payload.indexOf(',');
            if (commaIndex > 0)
            {
                String deviceId = payload.substring(0, commaIndex);
                String seedStr = payload.substring(commaIndex + 1);
                uint32_t seedValue = seedStr.toInt();

                writeDeviceIdToEEPROM(deviceId);
                writeSeedToEEPROM(seedValue);

                Serial.println("INFO_UPDATED:" + deviceId + "," + String(seedValue));
            }
            else
            {
                Serial.println("ERROR:Invalid format");
            }
        }
    }

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
                Serial.print("] 🔐 Sent -> ");
                Serial.println(msg);
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
                peer->privateKey = generatePrivateKey(seed);
                peer->publicKey = generatePublicKey(peer->privateKey);
            }

            String pkMsg = createMessage("PK", id, msg.senderId, String(peer->publicKey));
            LoRa.beginPacket();
            LoRa.print(pkMsg);
            LoRa.endPacket();

            Serial.println(" \n======== STEP 4: Tx -> Rx :DH Key Exchange ========");
            Serial.println("PRIVATE KEY: " + String(peer->privateKey));
            Serial.println("PUBLIC KEY: " + String(peer->publicKey));
            Serial.println("[ " + pkMsg + " ] ");
            Serial.println("=====================================================");

            peer->pkSent = true;
            peer->state = PeerState::ACK_PENDING;

            // Send ACK immediately
            String ackMsg = createMessage("ACK", id, msg.senderId, "OK");
            LoRa.beginPacket();
            LoRa.print(ackMsg);
            LoRa.endPacket();

            Serial.println(" \n======== Tx -> Rx :Sends Acknowledgement ========");
            Serial.println("[ " + ackMsg + " ] ");
            Serial.println("=====================================================");

            // Derive shared session key
            if (peer->sharedSessionKey == 0 &&
                peer->remotePublicKey != 0 &&
                peer->privateKey != 0 &&
                peer->pkReceived && peer->pkSent)
            {
                peer->sharedSessionKey = generateSharedKey(peer->remotePublicKey, peer->privateKey);
                Serial.println(" \n\n======== STEP 6: Tx Generates Shared Session Key ========");
                Serial.println("SHARED SESSION KEY: " + String(peer->sharedSessionKey));
                Serial.println("==========================================================");
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
                Serial.println("STEP 2: ⚠️  Received CLEAR from " + msg.senderId + ". Removing peer.");
                Serial.println("[ " +
                               msg.type + ":" +
                               msg.senderId + ":" +
                               msg.receiverId + ":" +
                               msg.payload +
                               " ] \n");

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
            handleChallengeResponse(peer, msg, id, ttl);
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
