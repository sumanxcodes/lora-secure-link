// MessageUtils.h
#ifndef MESSAGE_UTILS_H
#define MESSAGE_UTILS_H

#include <Arduino.h>

// Generic LoRa message structure.
// For complex types (e.g., CHAL, RESP), decode payload separately.
struct LoRaMessage
{
    String type;       // e.g., "PK", "ACK", "CHAL", "RESP", "MSG"
    String senderId;   // Sender's device ID
    String receiverId; // Intended receiver ID or "ALL"
    String payload;    // Can hold a sub-structure: "value|hash|ttl" etc.
};

inline LoRaMessage parseMessage(const String &raw)
{
    LoRaMessage msg;
    int idx1 = raw.indexOf(':');
    int idx2 = raw.indexOf(':', idx1 + 1);
    int idx3 = raw.indexOf(':', idx2 + 1);

    if (idx1 == -1 || idx2 == -1 || idx3 == -1)
    {
        msg.type = "INVALID";
        return msg;
    }

    msg.type = raw.substring(0, idx1);
    msg.senderId = raw.substring(idx1 + 1, idx2);
    msg.receiverId = raw.substring(idx2 + 1, idx3);
    msg.payload = raw.substring(idx3 + 1); // Could contain "challenge|hash|ttl" etc.

    return msg;
}

inline String createMessage(const String &type, const String &senderId, const String &receiverId, const String &payload)
{
    return type + ":" + senderId + ":" + receiverId + ":" + payload;
}

inline String createPKMessage(const String &senderId, const String &receiverId, uint32_t publicKey)
{
    return createMessage("PK", senderId, receiverId, String(publicKey));
}

#endif
