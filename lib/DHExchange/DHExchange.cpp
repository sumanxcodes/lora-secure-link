// ===========================================
// DHExchange.cpp (Dummy Implementation)
// For testing: Simulates DH key exchange logic
// ===========================================

#include "DHExchange.h"

// Dummy prime and generator values are not used in this mock
#define DH_PRIME 2147483647UL
#define DH_GENERATOR 5

// Fixed dummy key values for simulation
static const uint32_t DUMMY_PRIVATE_KEY = 123456;
static const uint32_t DUMMY_PUBLIC_KEY = 654321;
static const uint32_t DUMMY_SHARED_KEY = 111111;

/**
 * @brief Return a fixed dummy private key
 */
uint32_t generatePrivateKey()
{
    return DUMMY_PRIVATE_KEY;
}

/**
 * @brief Return a fixed dummy public key
 */
uint32_t generatePublicKey(uint32_t privateKey)
{
    (void)privateKey; // unused
    return DUMMY_PUBLIC_KEY;
}

/**
 * @brief Return a fixed dummy shared session key
 */
uint32_t generateSharedKey(uint32_t remotePublicKey, uint32_t privateKey)
{
    (void)remotePublicKey; // unused
    (void)privateKey;      // unused
    return DUMMY_SHARED_KEY;
}
