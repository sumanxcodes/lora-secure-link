#ifndef CHALLENGE_AUTH_H
#define CHALLENGE_AUTH_H

#include <Arduino.h>
#include <LoRa.h>
#include "NodeManager.h"
#include "MessageUtils.h"
#include "EncryptionUtils.h"

/**
 * Sends an encrypted challenge to the peer to verify session key ownership.
 * The challenge is a simple number (e.g., 11) and the peer must respond with challenge+1.
 */
void handleAuthChallenge(NodeState *peer, const String &selfId, uint32_t ttl);

/**
 * Verifies a received response to a previous challenge.
 * Decrypts and checks if the response is equal to (challenge + 1).
 * If successful, the peer is marked AUTHENTICATED.
 */
bool verifyAuthResponse(NodeState *peer, const String &payload, uint32_t messageCount, const String &selfId);

#endif
