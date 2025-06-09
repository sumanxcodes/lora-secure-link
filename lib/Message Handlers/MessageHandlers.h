#ifndef MESSAGE_HANDLERS_H
#define MESSAGE_HANDLERS_H

#include <Arduino.h>
#include <LoRa.h>
#include "DHExchange.h"
#include "MessageUtils.h"
#include "NodeManager.h"
#include "EncryptionUtils.h"
#include "ChallengeAuth.h"

// These are declared in main RX node file
extern String id;
extern uint32_t ttl;

// Handles individual message types
void handlePing(const LoRaMessage &msg);
void handlePkExchange(const LoRaMessage &msg, uint32_t seed);
void handleAck(const LoRaMessage &msg);
void handleClear(const LoRaMessage &msg);
void handleMsg(const LoRaMessage &msg);

#endif
