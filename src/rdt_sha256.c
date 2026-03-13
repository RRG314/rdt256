#include "rdt_sha256.h"

#include <string.h>

static void secure_zero(void *ptr, size_t len) {
    volatile uint8_t *p = (volatile uint8_t *)ptr;
    size_t i;
    for (i = 0; i < len; i++) {
        p[i] = 0;
    }
}

static const uint32_t sha256_k[64] = {
    0x428a2f98u, 0x71374491u, 0xb5c0fbcfu, 0xe9b5dba5u,
    0x3956c25bu, 0x59f111f1u, 0x923f82a4u, 0xab1c5ed5u,
    0xd807aa98u, 0x12835b01u, 0x243185beu, 0x550c7dc3u,
    0x72be5d74u, 0x80deb1feu, 0x9bdc06a7u, 0xc19bf174u,
    0xe49b69c1u, 0xefbe4786u, 0x0fc19dc6u, 0x240ca1ccu,
    0x2de92c6fu, 0x4a7484aau, 0x5cb0a9dcu, 0x76f988dau,
    0x983e5152u, 0xa831c66du, 0xb00327c8u, 0xbf597fc7u,
    0xc6e00bf3u, 0xd5a79147u, 0x06ca6351u, 0x14292967u,
    0x27b70a85u, 0x2e1b2138u, 0x4d2c6dfcu, 0x53380d13u,
    0x650a7354u, 0x766a0abbu, 0x81c2c92eu, 0x92722c85u,
    0xa2bfe8a1u, 0xa81a664bu, 0xc24b8b70u, 0xc76c51a3u,
    0xd192e819u, 0xd6990624u, 0xf40e3585u, 0x106aa070u,
    0x19a4c116u, 0x1e376c08u, 0x2748774cu, 0x34b0bcb5u,
    0x391c0cb3u, 0x4ed8aa4au, 0x5b9cca4fu, 0x682e6ff3u,
    0x748f82eeu, 0x78a5636fu, 0x84c87814u, 0x8cc70208u,
    0x90befffau, 0xa4506cebu, 0xbef9a3f7u, 0xc67178f2u
};

#define RDT_ROR32(x, n) (((x) >> (n)) | ((x) << (32u - (n))))
#define RDT_CH(x, y, z) (((x) & (y)) ^ (~(x) & (z)))
#define RDT_MAJ(x, y, z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define RDT_EP0(x) (RDT_ROR32((x), 2u) ^ RDT_ROR32((x), 13u) ^ RDT_ROR32((x), 22u))
#define RDT_EP1(x) (RDT_ROR32((x), 6u) ^ RDT_ROR32((x), 11u) ^ RDT_ROR32((x), 25u))
#define RDT_SIG0(x) (RDT_ROR32((x), 7u) ^ RDT_ROR32((x), 18u) ^ ((x) >> 3u))
#define RDT_SIG1(x) (RDT_ROR32((x), 17u) ^ RDT_ROR32((x), 19u) ^ ((x) >> 10u))

static void rdt_sha256_transform(rdt_sha256_ctx *ctx, const uint8_t data[64]) {
    uint32_t a, b, c, d, e, f, g, h;
    uint32_t t1, t2;
    uint32_t m[64];
    int i;

    for (i = 0; i < 16; i++) {
        m[i] = ((uint32_t)data[i * 4] << 24)
             | ((uint32_t)data[i * 4 + 1] << 16)
             | ((uint32_t)data[i * 4 + 2] << 8)
             | ((uint32_t)data[i * 4 + 3]);
    }
    for (; i < 64; i++) {
        m[i] = RDT_SIG1(m[i - 2]) + m[i - 7] + RDT_SIG0(m[i - 15]) + m[i - 16];
    }

    a = ctx->state[0];
    b = ctx->state[1];
    c = ctx->state[2];
    d = ctx->state[3];
    e = ctx->state[4];
    f = ctx->state[5];
    g = ctx->state[6];
    h = ctx->state[7];

    for (i = 0; i < 64; i++) {
        t1 = h + RDT_EP1(e) + RDT_CH(e, f, g) + sha256_k[i] + m[i];
        t2 = RDT_EP0(a) + RDT_MAJ(a, b, c);
        h = g;
        g = f;
        f = e;
        e = d + t1;
        d = c;
        c = b;
        b = a;
        a = t1 + t2;
    }

    ctx->state[0] += a;
    ctx->state[1] += b;
    ctx->state[2] += c;
    ctx->state[3] += d;
    ctx->state[4] += e;
    ctx->state[5] += f;
    ctx->state[6] += g;
    ctx->state[7] += h;
}

void rdt_sha256_init(rdt_sha256_ctx *ctx) {
    if (!ctx) {
        return;
    }
    ctx->state[0] = 0x6a09e667u;
    ctx->state[1] = 0xbb67ae85u;
    ctx->state[2] = 0x3c6ef372u;
    ctx->state[3] = 0xa54ff53au;
    ctx->state[4] = 0x510e527fu;
    ctx->state[5] = 0x9b05688cu;
    ctx->state[6] = 0x1f83d9abu;
    ctx->state[7] = 0x5be0cd19u;
    ctx->count = 0;
    memset(ctx->buffer, 0, sizeof(ctx->buffer));
}

void rdt_sha256_update(rdt_sha256_ctx *ctx, const uint8_t *data, size_t len) {
    size_t i = 0;
    size_t idx;

    if (!ctx || (!data && len)) {
        return;
    }

    idx = (size_t)(ctx->count & 63u);
    ctx->count += len;

    if (idx) {
        size_t left = 64u - idx;
        if (len < left) {
            memcpy(ctx->buffer + idx, data, len);
            return;
        }
        memcpy(ctx->buffer + idx, data, left);
        rdt_sha256_transform(ctx, ctx->buffer);
        i = left;
    }

    for (; i + 64u <= len; i += 64u) {
        rdt_sha256_transform(ctx, data + i);
    }

    if (i < len) {
        memcpy(ctx->buffer, data + i, len - i);
    }
}

void rdt_sha256_final(rdt_sha256_ctx *ctx, uint8_t hash[RDT_SHA256_DIGEST_SIZE]) {
    size_t idx;
    uint64_t bits;
    int i;

    if (!ctx || !hash) {
        return;
    }

    idx = (size_t)(ctx->count & 63u);
    ctx->buffer[idx++] = 0x80u;

    if (idx > 56u) {
        memset(ctx->buffer + idx, 0, 64u - idx);
        rdt_sha256_transform(ctx, ctx->buffer);
        idx = 0;
    }

    memset(ctx->buffer + idx, 0, 56u - idx);
    bits = ctx->count * 8u;
    for (i = 0; i < 8; i++) {
        ctx->buffer[63 - i] = (uint8_t)(bits >> (unsigned)(i * 8));
    }
    rdt_sha256_transform(ctx, ctx->buffer);

    for (i = 0; i < 8; i++) {
        hash[i * 4] = (uint8_t)(ctx->state[i] >> 24);
        hash[i * 4 + 1] = (uint8_t)(ctx->state[i] >> 16);
        hash[i * 4 + 2] = (uint8_t)(ctx->state[i] >> 8);
        hash[i * 4 + 3] = (uint8_t)(ctx->state[i]);
    }

    secure_zero(ctx->buffer, sizeof(ctx->buffer));
}

void rdt_sha256(const uint8_t *data, size_t len, uint8_t hash[RDT_SHA256_DIGEST_SIZE]) {
    rdt_sha256_ctx ctx;
    rdt_sha256_init(&ctx);
    rdt_sha256_update(&ctx, data, len);
    rdt_sha256_final(&ctx, hash);
    secure_zero(&ctx, sizeof(ctx));
}

void rdt_hmac_sha256_init(rdt_hmac_sha256_ctx *ctx, const uint8_t *key, size_t key_len) {
    uint8_t block[RDT_SHA256_BLOCK_SIZE];
    uint8_t hashed_key[RDT_SHA256_DIGEST_SIZE];
    size_t i;

    if (!ctx || (!key && key_len)) {
        return;
    }

    memset(block, 0, sizeof(block));
    if (key_len > RDT_SHA256_BLOCK_SIZE) {
        rdt_sha256(key, key_len, hashed_key);
        memcpy(block, hashed_key, sizeof(hashed_key));
    } else if (key_len) {
        memcpy(block, key, key_len);
    }

    for (i = 0; i < sizeof(block); i++) {
        block[i] ^= 0x36u;
    }
    rdt_sha256_init(&ctx->inner);
    rdt_sha256_update(&ctx->inner, block, sizeof(block));

    for (i = 0; i < sizeof(block); i++) {
        block[i] ^= (uint8_t)(0x36u ^ 0x5cu);
    }
    rdt_sha256_init(&ctx->outer);
    rdt_sha256_update(&ctx->outer, block, sizeof(block));

    secure_zero(block, sizeof(block));
    secure_zero(hashed_key, sizeof(hashed_key));
}

void rdt_hmac_sha256_update(rdt_hmac_sha256_ctx *ctx, const uint8_t *data, size_t len) {
    if (!ctx) {
        return;
    }
    rdt_sha256_update(&ctx->inner, data, len);
}

void rdt_hmac_sha256_final(rdt_hmac_sha256_ctx *ctx, uint8_t mac[RDT_SHA256_DIGEST_SIZE]) {
    uint8_t inner_hash[RDT_SHA256_DIGEST_SIZE];

    if (!ctx || !mac) {
        return;
    }

    rdt_sha256_final(&ctx->inner, inner_hash);
    rdt_sha256_update(&ctx->outer, inner_hash, sizeof(inner_hash));
    rdt_sha256_final(&ctx->outer, mac);
    secure_zero(inner_hash, sizeof(inner_hash));
}

void rdt_hmac_sha256(const uint8_t *key, size_t key_len,
                     const uint8_t *data, size_t len,
                     uint8_t mac[RDT_SHA256_DIGEST_SIZE]) {
    rdt_hmac_sha256_ctx ctx;
    rdt_hmac_sha256_init(&ctx, key, key_len);
    rdt_hmac_sha256_update(&ctx, data, len);
    rdt_hmac_sha256_final(&ctx, mac);
    secure_zero(&ctx, sizeof(ctx));
}
