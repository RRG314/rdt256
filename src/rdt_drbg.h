#ifndef RDT_DRBG_H
#define RDT_DRBG_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Instantiate */
void rdt_drbg_init_u64(uint64_t entropy_seed,
                       uint64_t nonce,
                       uint64_t personalization);

/* Reseed with fresh entropy bytes + optional additional input bytes */
void rdt_drbg_reseed(const uint8_t *entropy, size_t entropy_len,
                     const uint8_t *additional, size_t additional_len);

/*
Generate bytes.
- additional: optional data mixed in pre-generate
- prediction_resistance: if 1, caller should reseed with fresh entropy first;
  we still accept additional, but additional != entropy.
Returns 0 on success, -1 on bad args.
*/
int rdt_drbg_generate(uint8_t *out, size_t out_len,
                      const uint8_t *additional, size_t additional_len,
                      int prediction_resistance);

/* Convenience 64-bit output */
uint64_t rdt_drbg_next_u64(void);

/* Optional: wipe state */
void rdt_drbg_zeroize(void);

#ifdef __cplusplus
}
#endif

#endif /* RDT_DRBG_H */
