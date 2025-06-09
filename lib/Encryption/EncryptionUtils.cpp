#include "EncryptionUtils.h"
#include "base64.hpp"

String base64Encode(uint8_t *data, size_t length)
{
    char encoded[256]; // adjust size as needed
    encode_base64(data, length, (uint8_t *)encoded);
    return String(encoded);
}

bool base64Decode(String input, uint8_t *output, size_t *decodedLength)
{
    *decodedLength = decode_base64((const uint8_t *)input.c_str(), input.length(), output);
    return *decodedLength > 0;
}

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

    return base64Encode(encrypted, length);
}

// Decrypts a string encrypted with the above method
String decryptString(const String &encryptedText, uint32_t sessionKey, uint32_t messageCount)
{
    size_t length = 0;
    uint8_t decoded[128], decrypted[128];

    if (!base64Decode(encryptedText, decoded, &length))
        return "";

    streamCipherBytes(decoded, decrypted, length, sessionKey, messageCount);

    String result = "";
    for (size_t i = 0; i < length; i++)
        result += (char)decrypted[i];

    return result;
}
