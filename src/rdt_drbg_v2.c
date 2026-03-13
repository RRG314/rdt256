#include "rdt_drbg_v2.h"

#include "rdt_core.h"
#include "rdt_sha256.h"

#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static const uint64_t rdt_drbg_v2_init_key[4] = {
    0x243F6A8885A308D3ULL,
    0x13198A2E03707344ULL,
    0xA4093822299F31D0ULL,
    0x082EFA98EC4E6C89ULL
};

static inline uint64_t rotl64(uint64_t x, uint32_t r) {
    r &= 63u;
    return r ? ((x << r) | (x >> (64u - r))) : x;
}

static void secure_zero(void *ptr, size_t len) {
    volatile uint8_t *p = (volatile uint8_t *)ptr;
    size_t i;
    for (i = 0; i < len; i++) {
        p[i] = 0;
    }
}

static void store_u64_le(uint8_t out[8], uint64_t x) {
    unsigned i;
    for (i = 0; i < 8u; i++) {
        out[i] = (uint8_t)(x >> (i * 8u));
    }
}

static void hmac_sha256_parts(const uint8_t key[RDT_DRBG_V2_SEED_BYTES],
                              const uint8_t *a, size_t a_len,
                              const uint8_t *b, size_t b_len,
                              const uint8_t *c, size_t c_len,
                              uint8_t out[RDT_DRBG_V2_SEED_BYTES]) {
    rdt_hmac_sha256_ctx hmac;
    rdt_hmac_sha256_init(&hmac, key, RDT_DRBG_V2_SEED_BYTES);
    if (a_len) {
        rdt_hmac_sha256_update(&hmac, a, a_len);
    }
    if (b_len) {
        rdt_hmac_sha256_update(&hmac, b, b_len);
    }
    if (c_len) {
        rdt_hmac_sha256_update(&hmac, c, c_len);
    }
    rdt_hmac_sha256_final(&hmac, out);
}

static void drbg_v2_update(rdt_drbg_v2_ctx *ctx, const uint8_t *provided_data, size_t provided_len) {
    uint8_t sep0 = 0x00u;
    uint8_t sep1 = 0x01u;

    hmac_sha256_parts(ctx->K, ctx->V, sizeof(ctx->V), &sep0, 1u, provided_data, provided_len, ctx->K);
    rdt_hmac_sha256(ctx->K, sizeof(ctx->K), ctx->V, sizeof(ctx->V), ctx->V);

    if (provided_len) {
        hmac_sha256_parts(ctx->K, ctx->V, sizeof(ctx->V), &sep1, 1u, provided_data, provided_len, ctx->K);
        rdt_hmac_sha256(ctx->K, sizeof(ctx->K), ctx->V, sizeof(ctx->V), ctx->V);
    }
}

static int concat_inputs(uint8_t **out, size_t *out_len,
                         const uint8_t *a, size_t a_len,
                         const uint8_t *b, size_t b_len,
                         const uint8_t *c, size_t c_len) {
    uint8_t *buf;
    size_t total;
    size_t pos = 0;

    if (!out || !out_len) {
        return RDT_DRBG_V2_ERR_ARGS;
    }
    *out = NULL;
    *out_len = 0;

    if ((!a && a_len) || (!b && b_len) || (!c && c_len)) {
        return RDT_DRBG_V2_ERR_ARGS;
    }
    if (a_len > SIZE_MAX - b_len || a_len + b_len > SIZE_MAX - c_len) {
        return RDT_DRBG_V2_ERR_ARGS;
    }
    total = a_len + b_len + c_len;
    if (!total) {
        return RDT_DRBG_V2_OK;
    }

    buf = (uint8_t *)malloc(total);
    if (!buf) {
        return RDT_DRBG_V2_ERR_ALLOC;
    }

    if (a_len) {
        memcpy(buf + pos, a, a_len);
        pos += a_len;
    }
    if (b_len) {
        memcpy(buf + pos, b, b_len);
        pos += b_len;
    }
    if (c_len) {
        memcpy(buf + pos, c, c_len);
    }

    *out = buf;
    *out_len = total;
    return RDT_DRBG_V2_OK;
}

static int fill_system_entropy(uint8_t *out, size_t out_len) {
    int fd;
    size_t offset = 0;

    if (!out || !out_len) {
        return RDT_DRBG_V2_ERR_ARGS;
    }

    fd = open("/dev/urandom", O_RDONLY);
    if (fd < 0) {
        return RDT_DRBG_V2_ERR_ENTROPY;
    }

    while (offset < out_len) {
        ssize_t got = read(fd, out + offset, out_len - offset);
        if (got <= 0) {
            secure_zero(out, out_len);
            close(fd);
            return RDT_DRBG_V2_ERR_ENTROPY;
        }
        offset += (size_t)got;
    }

    if (close(fd) != 0) {
        secure_zero(out, out_len);
        return RDT_DRBG_V2_ERR_ENTROPY;
    }
    return RDT_DRBG_V2_OK;
}

static uint64_t derive_word(uint64_t entropy_seed,
                            uint64_t nonce,
                            uint64_t personalization,
                            uint64_t domain) {
    uint64_t base = entropy_seed
                  ^ rotl64(nonce + domain * 0x9E3779B97F4A7C15ULL, 17u)
                  ^ rotl64(personalization ^ domain * 0xBF58476D1CE4E5B9ULL, 41u)
                  ^ (0xD6E8FEB86659FD93ULL * domain);
    uint64_t mixed = rdt_mix(base, rdt_drbg_v2_init_key);
    mixed ^= rdt_mix(base ^ mixed ^ rotl64(entropy_seed, (uint32_t)(domain & 31u)), rdt_drbg_v2_init_key);
    return mixed;
}

int rdt_drbg_v2_instantiate(rdt_drbg_v2_ctx *ctx,
                            const uint8_t *entropy, size_t entropy_len,
                            const uint8_t *nonce, size_t nonce_len,
                            const uint8_t *personalization, size_t personalization_len) {
    uint8_t *seed_material = NULL;
    size_t seed_material_len = 0;
    int rc;

    if (!ctx || !entropy || !entropy_len) {
        return RDT_DRBG_V2_ERR_ARGS;
    }

    rc = concat_inputs(&seed_material, &seed_material_len,
                       entropy, entropy_len,
                       nonce, nonce_len,
                       personalization, personalization_len);
    if (rc != RDT_DRBG_V2_OK) {
        return rc;
    }

    memset(ctx->K, 0x00, sizeof(ctx->K));
    memset(ctx->V, 0x01, sizeof(ctx->V));
    ctx->reseed_counter = 0;
    ctx->seeded = 1;

    drbg_v2_update(ctx, seed_material, seed_material_len);
    ctx->reseed_counter = 1;

    if (seed_material) {
        secure_zero(seed_material, seed_material_len);
        free(seed_material);
    }
    return RDT_DRBG_V2_OK;
}

int rdt_drbg_v2_init_u64(rdt_drbg_v2_ctx *ctx,
                         uint64_t entropy_seed,
                         uint64_t nonce,
                         uint64_t personalization) {
    uint8_t entropy[RDT_DRBG_V2_SEED_BYTES];
    uint8_t nonce_bytes[RDT_DRBG_V2_NONCE_BYTES];
    uint64_t words[6];
    int i;

    if (!ctx) {
        return RDT_DRBG_V2_ERR_ARGS;
    }

    for (i = 0; i < 6; i++) {
        words[i] = derive_word(entropy_seed, nonce, personalization, (uint64_t)(i + 1));
    }

    for (i = 0; i < 4; i++) {
        store_u64_le(entropy + (size_t)i * 8u, words[i]);
    }
    for (i = 0; i < 2; i++) {
        store_u64_le(nonce_bytes + (size_t)i * 8u, words[i + 4]);
    }

    i = rdt_drbg_v2_instantiate(ctx,
                                entropy, sizeof(entropy),
                                nonce_bytes, sizeof(nonce_bytes),
                                NULL, 0u);
    secure_zero(entropy, sizeof(entropy));
    secure_zero(nonce_bytes, sizeof(nonce_bytes));
    secure_zero(words, sizeof(words));
    return i;
}

int rdt_drbg_v2_init_system(rdt_drbg_v2_ctx *ctx,
                            const uint8_t *personalization, size_t personalization_len) {
    uint8_t entropy[RDT_DRBG_V2_SEED_BYTES];
    uint8_t nonce[RDT_DRBG_V2_NONCE_BYTES];
    int rc;

    if (!ctx || (!personalization && personalization_len)) {
        return RDT_DRBG_V2_ERR_ARGS;
    }

    rc = fill_system_entropy(entropy, sizeof(entropy));
    if (rc == RDT_DRBG_V2_OK) {
        rc = fill_system_entropy(nonce, sizeof(nonce));
    }
    if (rc == RDT_DRBG_V2_OK) {
        rc = rdt_drbg_v2_instantiate(ctx,
                                     entropy, sizeof(entropy),
                                     nonce, sizeof(nonce),
                                     personalization, personalization_len);
    }

    secure_zero(entropy, sizeof(entropy));
    secure_zero(nonce, sizeof(nonce));
    return rc;
}

int rdt_drbg_v2_reseed(rdt_drbg_v2_ctx *ctx,
                       const uint8_t *entropy, size_t entropy_len,
                       const uint8_t *additional, size_t additional_len) {
    uint8_t *seed_material = NULL;
    size_t seed_material_len = 0;
    int rc;

    if (!ctx || !ctx->seeded || !entropy || !entropy_len) {
        return RDT_DRBG_V2_ERR_ARGS;
    }

    rc = concat_inputs(&seed_material, &seed_material_len,
                       entropy, entropy_len,
                       additional, additional_len,
                       NULL, 0u);
    if (rc != RDT_DRBG_V2_OK) {
        return rc;
    }

    drbg_v2_update(ctx, seed_material, seed_material_len);
    ctx->reseed_counter = 1;

    if (seed_material) {
        secure_zero(seed_material, seed_material_len);
        free(seed_material);
    }
    return RDT_DRBG_V2_OK;
}

int rdt_drbg_v2_reseed_system(rdt_drbg_v2_ctx *ctx,
                              const uint8_t *additional, size_t additional_len) {
    uint8_t entropy[RDT_DRBG_V2_SEED_BYTES];
    int rc;

    if (!ctx || (!additional && additional_len)) {
        return RDT_DRBG_V2_ERR_ARGS;
    }

    rc = fill_system_entropy(entropy, sizeof(entropy));
    if (rc == RDT_DRBG_V2_OK) {
        rc = rdt_drbg_v2_reseed(ctx, entropy, sizeof(entropy), additional, additional_len);
    }

    secure_zero(entropy, sizeof(entropy));
    return rc;
}

int rdt_drbg_v2_generate(rdt_drbg_v2_ctx *ctx,
                         uint8_t *out, size_t out_len,
                         const uint8_t *additional, size_t additional_len,
                         int prediction_resistance) {
    size_t produced = 0;

    if (!ctx || (!out && out_len) || (!additional && additional_len)) {
        return RDT_DRBG_V2_ERR_ARGS;
    }
    if (!ctx->seeded) {
        return RDT_DRBG_V2_ERR_NOT_INIT;
    }
    if (prediction_resistance) {
        return RDT_DRBG_V2_ERR_PREDICTION_RESISTANCE;
    }
    if (out_len > RDT_DRBG_V2_MAX_REQUEST_BYTES) {
        return RDT_DRBG_V2_ERR_REQUEST_TOO_LARGE;
    }
    if (ctx->reseed_counter > RDT_DRBG_V2_RESEED_INTERVAL) {
        return RDT_DRBG_V2_ERR_RESEED_REQUIRED;
    }

    if (additional_len) {
        drbg_v2_update(ctx, additional, additional_len);
    }

    while (produced < out_len) {
        size_t take;
        rdt_hmac_sha256(ctx->K, sizeof(ctx->K), ctx->V, sizeof(ctx->V), ctx->V);
        take = out_len - produced;
        if (take > sizeof(ctx->V)) {
            take = sizeof(ctx->V);
        }
        memcpy(out + produced, ctx->V, take);
        produced += take;
    }

    drbg_v2_update(ctx, additional, additional_len);
    ctx->reseed_counter += 1;
    return RDT_DRBG_V2_OK;
}

int rdt_drbg_v2_next_u64(rdt_drbg_v2_ctx *ctx, uint64_t *value) {
    uint8_t bytes[8];
    int rc;
    unsigned i;
    uint64_t x = 0;

    if (!value) {
        return RDT_DRBG_V2_ERR_ARGS;
    }

    rc = rdt_drbg_v2_generate(ctx, bytes, sizeof(bytes), NULL, 0u, 0);
    if (rc != RDT_DRBG_V2_OK) {
        return rc;
    }

    for (i = 0; i < 8u; i++) {
        x |= ((uint64_t)bytes[i]) << (i * 8u);
    }
    secure_zero(bytes, sizeof(bytes));
    *value = x;
    return RDT_DRBG_V2_OK;
}

void rdt_drbg_v2_zeroize(rdt_drbg_v2_ctx *ctx) {
    if (!ctx) {
        return;
    }
    secure_zero(ctx, sizeof(*ctx));
}
