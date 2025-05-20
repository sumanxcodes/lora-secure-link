#include "EncryptionUtils.h"

// XOR-based stream cipher with seeded PRNG
void streamCipherBytes(uint8_t *input, uint8_t *output, size_t length, uint32_t sessionKey, uint32_t messageCount)
{
    uint32_t seed = sessionKey + messageCount;
    randomSeed(seed);

    for (size_t i = 0; i < length; i++)
    {
        uint8_t keystreamByte = random(0, 256);
        output[i] = input[i] ^ keystreamByte;
    }
}

// Encrypts a string using XOR stream cipher
String encryptString(const String &plainText, uint32_t sessionKey, uint32_t messageCount)
{
    size_t length = plainText.length();
    uint8_t input[length], encrypted[length];

    for (size_t i = 0; i < length; i++)
        input[i] = (uint8_t)plainText[i];

    streamCipherBytes(input, encrypted, length, sessionKey, messageCount);

    String cipher = "";
    for (size_t i = 0; i < length; i++)
        cipher += (char)encrypted[i];

    return cipher;
}

// Decrypts a string encrypted with the above method
String decryptString(const String &encryptedText, uint32_t sessionKey, uint32_t messageCount)
{
    size_t length = encryptedText.length();
    uint8_t input[length], decrypted[length];

    for (size_t i = 0; i < length; i++)
        input[i] = (uint8_t)encryptedText[i];

    streamCipherBytes(input, decrypted, length, sessionKey, messageCount);

    String plainText = "";
    for (size_t i = 0; i < length; i++)
        plainText += (char)decrypted[i];

    return plainText;
}
