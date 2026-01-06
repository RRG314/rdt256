/*
 * RDT-PRNG_STREAM_v2 - Enhanced Cross-Diffusion Variant
 * 
 * Author: Steven Reid
 * ORCID: 0009-0003-9132-3410
 * License: MIT
 * 
 * IMPORTANT DISCLAIMER:
 * This is experimental research code, NOT production cryptographic software.
 * Do NOT use for real-world security, encryption, authentication, or key generation.
 * 
 * This variant adds cross-state rotational diffusion to the base RDT-PRNG_STREAM,
 * providing enhanced avalanche properties and improved statistical quality.
 * 
 * Validation:
 *   - NIST SP 800-22 Rev 1a: 15/15 tests passed (100 streams × 1M bits)
 *   - TestU01 BigCrush: 160/160 tests passed
 */

#ifndef RDT_PRNG_STREAM_V2_H
#define RDT_PRNG_STREAM_V2_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Initialize the PRNG with a 256-bit seed (4 × 64-bit values).
 * The seed undergoes mixing to ensure good initial state distribution.
 */
void rdt_prng_v2_init(const uint64_t seed[4]);

/*
 * Initialize from a 32-byte buffer (e.g., from SHA-256 hash of entropy).
 * Bytes are read in little-endian order.
 */
void rdt_prng_v2_init_bytes(const uint8_t seed[32]);

/*
 * Generate the next 64-bit pseudorandom value.
 */
uint64_t rdt_prng_v2_next(void);

/*
 * Fill a buffer with pseudorandom bytes.
 */
void rdt_prng_v2_fill(uint8_t *buf, size_t len);

#ifdef __cplusplus
}
#endif

#endif /* RDT_PRNG_STREAM_V2_H */
