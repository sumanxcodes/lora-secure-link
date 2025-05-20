// ===========================================
// DHExchange.h
// Header for Diffie-Hellman key exchange
// (Dummy implementation for testing)
// ===========================================

#ifndef DH_EXCHANGE_H
#define DH_EXCHANGE_H

#include <Arduino.h>

/**
 * @brief Generates a dummy private key (hardcoded).
 *
 * @return uint32_t The dummy private key
 */
uint32_t generatePrivateKey();

/**
 * @brief Generates a dummy public key using the private key (mocked).
 *
 * @param privateKey The private key (ignored in dummy mode)
 * @return uint32_t The dummy public key
 */
uint32_t generatePublicKey(uint32_t privateKey);

/**
 * @brief Generates a dummy shared session key.
 *
 * @param remotePublicKey The peer's public key (ignored in dummy mode)
 * @param privateKey The local private key (ignored in dummy mode)
 * @return uint32_t The dummy shared key
 */
uint32_t generateSharedKey(uint32_t remotePublicKey, uint32_t privateKey);

#endif // DH_EXCHANGE_H
