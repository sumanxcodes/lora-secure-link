// MessageUtils.h
#ifndef MESSAGE_UTILS_H
#define MESSAGE_UTILS_H

#include <Arduino.h>

/**
 * Represents a parsed LoRa message with optional TTL and messageCount.
 */
// For complex types (e.g., CHAL, RESP), decode payload separately.
struct LoRaMessage
{
    String type;          // e.g., "PK", "ACK", "CHAL", "RESP", "MSG"
    String senderId;      // Sender's device ID
    String receiverId;    // Intended receiver ID or "ALL"
    String payload;       // Can hold a sub-structure: "value|hash|ttl" etc.
    int ttl = 0;          // Time-to-live (number of hops or expiry)
    int messageCount = 0; // Used for strem cipher synchronisation
};

/**
 * Parses a basic 4-part message: type:sender:receiver:payload
 */
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
    msg.payload = raw.substring(idx3 + 1);

    return msg;
}

/**
 * Parses a 6-part message with TTL and message count:
 * type:sender:receiver:ttl:count:payload
 */
inline LoRaMessage parseMessageWithTTL(const String &raw)
{
    LoRaMessage msg;
    int idx1 = raw.indexOf(':');
    int idx2 = raw.indexOf(':', idx1 + 1);
    int idx3 = raw.indexOf(':', idx2 + 1);
    int idx4 = raw.indexOf(':', idx3 + 1);
    int idx5 = raw.indexOf(':', idx4 + 1);

    if (idx1 == -1 || idx2 == -1 || idx3 == -1 || idx4 == -1 || idx5 == -1)
    {
        msg.type = "INVALID";
        return msg;
    }

    msg.type = raw.substring(0, idx1);
    msg.senderId = raw.substring(idx1 + 1, idx2);
    msg.receiverId = raw.substring(idx2 + 1, idx3);
    msg.ttl = raw.substring(idx3 + 1, idx4).toInt();
    msg.messageCount = raw.substring(idx4 + 1, idx5).toInt();
    msg.payload = raw.substring(idx5 + 1); // Could contain "challenge|message" etc.

    return msg;
}

/**
 * Builds a basic 4-part LoRa message string.
 */
inline String createMessage(const String &type, const String &senderId, const String &receiverId, const String &payload)
{
    return type + ":" + senderId + ":" + receiverId + ":" + payload;
}

/**
 * Builds a full 6-part message with TTL and message counter.
 */
inline String createMessageWithTTL(const String &type, const String &senderId, const String &receiverId, int ttl, int messageCount, const String &payload)
{
    return type + ":" + senderId + ":" + receiverId + ":" + String(ttl) + ":" + String(messageCount) + ":" + payload;
}

#endif
