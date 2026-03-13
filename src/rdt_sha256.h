#ifndef RDT_SHA256_H
#define RDT_SHA256_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RDT_SHA256_DIGEST_SIZE 32u
#define RDT_SHA256_BLOCK_SIZE 64u

typedef struct {
    uint32_t state[8];
    uint64_t count;
    uint8_t buffer[RDT_SHA256_BLOCK_SIZE];
} rdt_sha256_ctx;

typedef struct {
    rdt_sha256_ctx inner;
    rdt_sha256_ctx outer;
} rdt_hmac_sha256_ctx;

void rdt_sha256_init(rdt_sha256_ctx *ctx);
void rdt_sha256_update(rdt_sha256_ctx *ctx, const uint8_t *data, size_t len);
void rdt_sha256_final(rdt_sha256_ctx *ctx, uint8_t hash[RDT_SHA256_DIGEST_SIZE]);
void rdt_sha256(const uint8_t *data, size_t len, uint8_t hash[RDT_SHA256_DIGEST_SIZE]);

void rdt_hmac_sha256_init(rdt_hmac_sha256_ctx *ctx, const uint8_t *key, size_t key_len);
void rdt_hmac_sha256_update(rdt_hmac_sha256_ctx *ctx, const uint8_t *data, size_t len);
void rdt_hmac_sha256_final(rdt_hmac_sha256_ctx *ctx, uint8_t mac[RDT_SHA256_DIGEST_SIZE]);
void rdt_hmac_sha256(const uint8_t *key, size_t key_len,
                     const uint8_t *data, size_t len,
                     uint8_t mac[RDT_SHA256_DIGEST_SIZE]);

#ifdef __cplusplus
}
#endif

#endif /* RDT_SHA256_H */
