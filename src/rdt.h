#ifndef RDT_H
#define RDT_H

#include <stddef.h>
#include <stdint.h>

/* Core */
uint64_t rdt_mix(uint64_t x, const uint64_t K[4]);

/* PRNG */
void rdt_prng_init(uint64_t seed);
uint64_t rdt_prng_next(void);

/* Legacy experimental DRBG */
void rdt_drbg_init_u64(uint64_t entropy_seed,
                       uint64_t nonce,
                       uint64_t personalization);
int rdt_drbg_generate(uint8_t *out, size_t out_len,
                      const uint8_t *additional, size_t additional_len,
                      int prediction_resistance);
uint64_t rdt_drbg_next_u64(void);
void rdt_drbg_zeroize(void);

#endif /* RDT_H */
