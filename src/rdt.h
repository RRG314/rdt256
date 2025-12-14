#ifndef RDT_H
#define RDT_H

#include <stdint.h>

/* ================= CORE ================= */

/* Core RDT mixing primitive */
uint64_t rdt_mix(uint64_t x, const uint64_t K[4]);

/* ================= PRNG ================= */

/* Initialize PRNG with 64-bit seed */
void rdt_prng_init(uint64_t seed);

/* Generate next 64-bit output */
uint64_t rdt_prng_next(void);

/* ================= DRBG ================= */

/* Initialize DRBG with 256-bit seed material */
void rdt_drbg_init(const uint64_t seed[4]);

/* Generate next 64-bit DRBG output */
uint64_t rdt_drbg_next(void);

#endif /* RDT_H */
