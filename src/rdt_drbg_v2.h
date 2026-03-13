#ifndef RDT_DRBG_V2_H
#define RDT_DRBG_V2_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RDT_DRBG_V2_SEED_BYTES 32u
#define RDT_DRBG_V2_NONCE_BYTES 16u
#define RDT_DRBG_V2_MAX_REQUEST_BYTES 65536u
#define RDT_DRBG_V2_RESEED_INTERVAL 281474976710656ULL

enum {
    RDT_DRBG_V2_OK = 0,
    RDT_DRBG_V2_ERR_ARGS = -1,
    RDT_DRBG_V2_ERR_NOT_INIT = -2,
    RDT_DRBG_V2_ERR_RESEED_REQUIRED = -3,
    RDT_DRBG_V2_ERR_REQUEST_TOO_LARGE = -4,
    RDT_DRBG_V2_ERR_PREDICTION_RESISTANCE = -5,
    RDT_DRBG_V2_ERR_ALLOC = -6,
    RDT_DRBG_V2_ERR_ENTROPY = -7
};

typedef struct {
    uint8_t K[RDT_DRBG_V2_SEED_BYTES];
    uint8_t V[RDT_DRBG_V2_SEED_BYTES];
    uint64_t reseed_counter;
    int seeded;
} rdt_drbg_v2_ctx;

int rdt_drbg_v2_instantiate(rdt_drbg_v2_ctx *ctx,
                            const uint8_t *entropy, size_t entropy_len,
                            const uint8_t *nonce, size_t nonce_len,
                            const uint8_t *personalization, size_t personalization_len);

int rdt_drbg_v2_init_u64(rdt_drbg_v2_ctx *ctx,
                         uint64_t entropy_seed,
                         uint64_t nonce,
                         uint64_t personalization);

int rdt_drbg_v2_init_system(rdt_drbg_v2_ctx *ctx,
                            const uint8_t *personalization, size_t personalization_len);

int rdt_drbg_v2_reseed(rdt_drbg_v2_ctx *ctx,
                       const uint8_t *entropy, size_t entropy_len,
                       const uint8_t *additional, size_t additional_len);

int rdt_drbg_v2_reseed_system(rdt_drbg_v2_ctx *ctx,
                              const uint8_t *additional, size_t additional_len);

int rdt_drbg_v2_generate(rdt_drbg_v2_ctx *ctx,
                         uint8_t *out, size_t out_len,
                         const uint8_t *additional, size_t additional_len,
                         int prediction_resistance);

int rdt_drbg_v2_next_u64(rdt_drbg_v2_ctx *ctx, uint64_t *value);

void rdt_drbg_v2_zeroize(rdt_drbg_v2_ctx *ctx);

#ifdef __cplusplus
}
#endif

#endif /* RDT_DRBG_V2_H */
