// ===========================================
// DHExchange.h
// Header for Diffie-Hellman key exchange
// ===========================================

#ifndef DH_EXCHANGE_H
#define DH_EXCHANGE_H

#include <Arduino.h>

/**
 * @brief Generates a  private key.
 *
 * @return uint32_t The  private key
 */
uint32_t generatePrivateKey(uint32_t seed);

/**
 * @brief Generates a  public key using the private key
 *
 * @param privateKey The private key
 * @return uint32_t The  public key
 */
uint32_t generatePublicKey(uint32_t privateKey);

/**
 * @brief Performs modular exponentiation using the square-and-multiply method.
 *
 * This function calculates (base^exp) % mod efficiently, which is essential for
 * cryptographic algorithms like Diffie-Hellman where large exponents and moduli
 * are involved. It avoids integer overflow by working with 64-bit intermediate results.
 *
 * @param base The base number.
 * @param exp The exponent.
 * @param mod The modulus.
 * @return uint32_t The result of (base^exp) % mod.
 */
uint32_t modexp(uint32_t base, uint32_t exp, uint32_t mod);

/**
 * @brief Generates a shared session key.
 *
 * @param remotePublicKey The peer's public key
 * @param privateKey The local private key
 * @return uint32_t The  shared key
 */
uint32_t generateSharedKey(uint32_t remotePublicKey, uint32_t privateKey);

#endif // DH_EXCHANGE_H
