#ifndef RDT_H
#define RDT_H

#include <stdint.h>

/* Core */
uint64_t rdt_mix(uint64_t x, const uint64_t K[4]);

/* PRNG */
void rdt_prng_init(uint64_t seed);
uint64_t rdt_prng_next(void);

/* DRBG (unchanged, declared elsewhere) */
void rdt_drbg_init(const uint64_t seed[4]);
uint64_t rdt_drbg_next(void);

#endif /* RDT_H */
