#include "MessageHandlers.h"

/**
 * Handles PING messages by replying with PONG.
 */
void handlePing(const LoRaMessage &msg)
{
    String pong = createMessage("PONG", id, msg.senderId, "READY");
    LoRa.beginPacket();
    LoRa.print(pong);
    LoRa.endPacket();
}

/**
 * Handles incoming public key (PK) exchange from peer.
 * Responds with our PK and computes shared session key.
 */
void handlePkExchange(const LoRaMessage &msg)
{
    NodeState *peer = findOrCreatePeer(msg.senderId);
    peer->remotePublicKey = msg.payload.toInt();
    peer->pkReceived = true;

    if (peer->privateKey == 0 || peer->publicKey == 0)
    {
        peer->privateKey = generatePrivateKey();
        peer->publicKey = generatePublicKey(peer->privateKey);
    }

    if (!peer->pkSent)
    {
        String pkMsg = createMessage("PK", id, msg.senderId, String(peer->publicKey));
        LoRa.beginPacket();
        LoRa.print(pkMsg);
        LoRa.endPacket();
        peer->pkSent = true;
    }

    if (peer->sharedSessionKey == 0 && peer->privateKey && peer->remotePublicKey)
    {
        peer->sharedSessionKey = generateSharedKey(peer->remotePublicKey, peer->privateKey);
        Serial.println("✅ Shared session key with " + msg.senderId + ": " + String(peer->sharedSessionKey));
    }

    peer->state = PeerState::ACK_PENDING;
}

/**
 * Handles ACK reception and transitions peer to SECURE_COMM state if valid.
 * Triggers challenge-response after handshake.
 */
void handleAck(const LoRaMessage &msg)
{
    Serial.println("Received ACK from " + msg.senderId);
    NodeState *peer = findOrCreatePeer(msg.senderId);

    if (!peer->ackReceived)
    {
        markPeerAckReceived(peer->id);

        if (!peer->ackSent)
        {
            String ack = createMessage("ACK", id, msg.senderId, "OK");
            LoRa.beginPacket();
            LoRa.print(ack);
            LoRa.endPacket();
            peer->ackSent = true;
            Serial.println("✅ Sent ACK in response to TX's ACK to " + peer->id);
        }

        if (peer->sharedSessionKey == 0 && peer->privateKey && peer->remotePublicKey)
        {
            peer->sharedSessionKey = generateSharedKey(peer->privateKey, peer->remotePublicKey);
            Serial.println("✅ Shared session key with " + peer->id + ": " + String(peer->sharedSessionKey));
        }

        if (isPeerDHComplete(peer->id) && peer->state != PeerState::SECURE_COMM)
        {
            peer->state = PeerState::SECURE_COMM;
            Serial.println("🤝 [RX] DH Exchange Complete with " + peer->id);
            printPeerStatus();
            delay(300);
            handleAuthChallenge(peer, id, ttl);
        }
    }
}

/**
 * Handles a CLEAR request (peer wants to reset handshake).
 */
void handleClear(const LoRaMessage &msg)
{
    if (msg.receiverId == "ALL" || msg.receiverId == id)
    {
        Serial.println("⚠️  Received CLEAR from " + msg.senderId + ". Removing peer.");
        NodeState *peer = findOrCreatePeer(msg.senderId);
        resetPeer(peer);
    }
}

void handleMsg(const LoRaMessage &msg)
{
    NodeState *peer = findOrCreatePeer(msg.senderId);

    if (peer->state != PeerState::AUTHENTICATED)
    {
        Serial.println("Received MSG before secure channel was established. Ignoring.");
        return;
    }

    String decrypted = decryptString(msg.payload, peer->sharedSessionKey, msg.messageCount);
    Serial.println("🔓 [" + String(millis() / 1000) + "] From " + msg.senderId +
                   " [msg " + String(msg.messageCount) + "]: " + decrypted);
}
