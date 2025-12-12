#ifndef RDT_H
#define RDT_H

#include <stdint.h>

uint64_t rdt_mix(uint64_t x, uint64_t K[4]);

/* PRNG */
void rdt_prng_init(uint64_t seed);
uint64_t rdt_prng_next(void);

/* DRBG */
void rdt_drbg_init(uint64_t seed);
uint64_t rdt_drbg_next(void);

#endif
