#ifndef ENCRYPTION_UTILS_H
#define ENCRYPTION_UTILS_H

#include <Arduino.h>

// Encrypts a byte array using a stream cipher with sessionKey and messageCount
void streamCipherBytes(uint8_t *input, uint8_t *output, size_t length, uint32_t sessionKey, uint32_t messageCount);

// Encrypts a plain text string (output is gibberish but reversible)
String encryptString(const String &plainText, uint32_t sessionKey, uint32_t messageCount);

// Decrypts a stream-ciphered string back into plain text
String decryptString(const String &encryptedText, uint32_t sessionKey, uint32_t messageCount);

#endif
