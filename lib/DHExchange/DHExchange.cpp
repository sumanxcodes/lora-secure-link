// ===========================================
// DHExchange.cpp
// ===========================================

#include "DHExchange.h"

// Dummy prime and generator values are not used in this mock
#define DH_PRIME 2147483647UL
#define DH_GENERATOR 5

/**
 * @brief Return a fixed dummy private key
 */
uint32_t generatePrivateKey(uint32_t seed)
{
    randomSeed(seed);               // Seeding Arduino's PRNG with EEPROM value
    return random(10000, 99999999); // Generates private key
}

// Perform modular exponentiation efficiently (base^exp) % mod
// This is the core operation behind DH public/shared key generation
uint32_t modexp(uint32_t base, uint32_t exp, uint32_t mod)
{
    uint64_t result = 1;
    uint64_t base64 = base;

    while (exp > 0)
    {
        if (exp % 2 == 1)
            result = (result * base64) % mod;

        base64 = (base64 * base64) % mod;
        exp = exp / 2;
    }

    return (uint32_t)result;
}

/**
 * @brief Return a fixed dummy public key
 */
uint32_t generatePublicKey(uint32_t privateKey)
{
    return modexp(DH_GENERATOR, privateKey, DH_PRIME);
}

/**
 * @brief Return a fixed dummy shared session key
 */
uint32_t generateSharedKey(uint32_t remotePublicKey, uint32_t privateKey)
{
    return modexp(remotePublicKey, privateKey, DH_PRIME);
}
