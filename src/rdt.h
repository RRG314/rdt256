#ifndef RDT_H
#define RDT_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ---------------- Core Mixing Primitive ---------------- */
uint64_t rdt_mix(uint64_t x, const uint64_t K[4]);

/* ---------------- Simple PRNG (non-cryptographic) ---------------- */
typedef struct {
    uint64_t S[4];
} rdt_prng_state;

void rdt_prng_init(rdt_prng_state *st, uint64_t seed);
uint64_t rdt_prng_next(rdt_prng_state *st);

/* ---------------- DRBG (cryptographically structured) ---------------- */
typedef struct {
    uint64_t K[4];     /* key material */
    uint64_t S[4];     /* internal state */
    uint64_t reseed_counter;
} rdt_drbg_state;

void rdt_drbg_init(rdt_drbg_state *st, const uint64_t seed[4]);
void rdt_drbg_reseed(rdt_drbg_state *st, const uint64_t seed[4]);
uint64_t rdt_drbg_next(rdt_drbg_state *st);

#ifdef __cplusplus
}
#endif

#endif
