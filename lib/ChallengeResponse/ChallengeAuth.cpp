#include "ChallengeAuth.h"

void handleAuthChallenge(NodeState *peer, const String &selfId, uint32_t ttl)
{

    randomSeed(peer->sharedSessionKey + peer->messageCount);
    uint32_t challenge = random(100000, 999999);

    peer->challenge = challenge;

    String challengeStr = String(challenge);
    String encryptedChallenge = encryptString(challengeStr, peer->sharedSessionKey, peer->messageCount);

    String chalMsg = createMessageWithTTL("CHAL", selfId, peer->id, ttl, peer->messageCount, encryptedChallenge);
    LoRa.beginPacket();
    LoRa.print(chalMsg);
    LoRa.endPacket();

    Serial.println("🔐 Sending CHAL to " + peer->id);
    Serial.println("Challenge (plain): " + challengeStr);
    Serial.println("Session Key: " + String(peer->sharedSessionKey));
    Serial.println("Message Count: " + String(peer->messageCount));
    Serial.println("Encrypted CHAL: " + encryptedChallenge);
    Serial.println("CHAL Message Sent: " + chalMsg);

    peer->messageCount++; // Next message (RESP) will use this updated count
    peer->state = PeerState::CHAL_SENT;
}

bool verifyAuthResponse(NodeState *peer, const String &payload, uint32_t messageCount, const String &selfId)
{
    String decrypted = decryptString(payload, peer->sharedSessionKey, messageCount);
    uint32_t expected = peer->challenge ^ peer->sharedSessionKey;

    Serial.println("📥 RESP Decrypted from " + peer->id + ": " + decrypted);
    Serial.println("Expected response: " + String(expected));

    if (decrypted.toInt() == expected)
    {
        peer->state = PeerState::AUTHENTICATED;
        Serial.println("✅ Authentication successful with " + peer->id);

        // Notify peer that authentication succeeded
        String successMsg = createMessage("AUTH_SUCCESS", selfId, peer->id, "OK");
        LoRa.beginPacket();
        LoRa.print(successMsg);
        LoRa.endPacket();
        return true;
    }
    else
    {
        Serial.println("❌ Authentication failed with " + peer->id);
        resetPeer(peer); // Reset peer state if response was wrong
        return false;
    }
}

void handleChallengeResponse(NodeState *peer, const LoRaMessage &msg, const String &selfId, uint32_t ttl)
{
    // Serial.println("📥 Raw LoRa Message: " + msg);
    Serial.println("Session Key: " + String(peer->sharedSessionKey));
    Serial.println("Message Count: " + String(msg.messageCount));

    String decryptedChallenge = decryptString(msg.payload, peer->sharedSessionKey, msg.messageCount);
    Serial.println("📥 CHAL Decrypted: " + decryptedChallenge);

    // Compute response
    uint32_t responseValue = decryptedChallenge.toInt() ^ peer->sharedSessionKey;
    String responseStr = String(responseValue);

    // Advance message counter
    peer->messageCount = msg.messageCount + 1;

    // Encrypt and send response
    String encryptedResponse = encryptString(responseStr, peer->sharedSessionKey, peer->messageCount);
    String respMsg = createMessageWithTTL("RESP", selfId, peer->id, ttl, peer->messageCount, encryptedResponse);

    LoRa.beginPacket();
    LoRa.print(respMsg);
    LoRa.endPacket();

    Serial.println("🔐 Sent RESP to " + peer->id + ": " + responseStr);
}
