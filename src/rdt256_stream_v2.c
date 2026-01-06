/*
 * RDT-PRNG_STREAM_v2 - Enhanced Cross-Diffusion Variant
 * 
 * Author: Steven Reid
 * ORCID: 0009-0003-9132-3410
 * License: MIT
 * 
 * IMPORTANT DISCLAIMER:
 * This is experimental research code, NOT production cryptographic software.
 * Do NOT use for real-world security, encryption, authentication, or key generation.
 * 
 * Changes from v1:
 *   - 256-bit (4 × 64-bit) seed initialization with mixing
 *   - Cross-state rotational diffusion after each step
 *   - Enhanced avalanche and statistical properties
 * 
 * Validation:
 *   - NIST SP 800-22 Rev 1a: 15/15 tests passed (100 streams × 1M bits)
 *   - TestU01 BigCrush: 160/160 tests passed
 */

#include "rdt256_stream_v2.h"
#include <stdint.h>
#include <string.h>

/* ============================================================================
 * RDT-CORE Primitives (unchanged from v1)
 * ============================================================================ */

static inline uint32_t bit_length(uint64_t x) {
    return x ? (uint32_t)(64u - __builtin_clzll(x)) : 0u;
}

static inline uint32_t popcount64(uint64_t x) {
    return (uint32_t)__builtin_popcountll(x);
}

static inline uint64_t rotl64(uint64_t x, uint32_t r) {
    r &= 63u;
    return r ? ((x << r) | (x >> (64u - r))) : x;
}

static inline uint32_t rdt_depth_fast(uint64_t x) {
    uint32_t bl = bit_length(x);
    uint32_t pc = popcount64(x);
    uint32_t mid = (bl ? (uint32_t)(x >> (bl >> 1)) : 0u);
    return (bl ^ (pc << 1) ^ mid) & 63u;
}

static inline uint32_t isqrt32(uint32_t x) {
    uint32_t r = 0, bit = 1u << 30;
    while (bit > x) bit >>= 2;
    while (bit != 0) {
        if (x >= r + bit) {
            x -= r + bit;
            r += bit << 1;
        }
        r >>= 1;
        bit >>= 2;
    }
    return r;
}

static inline uint32_t scalar_field(uint64_t x) {
    uint32_t a = (uint32_t)(x & 0xFFFFu);
    uint32_t b = (uint32_t)((x >> 16) & 0xFFFFu);
    uint32_t t = isqrt32(a * a + b * b);
    return rdt_depth_fast((uint64_t)t);
}

/* Prime table for epsilon channel */
static const uint64_t P[7] = {3, 5, 7, 11, 13, 17, 19};

/* Fixed key schedule */
static const uint64_t K[4] = {
    0xA3B1C6E5D4879F12ULL,
    0xC1D2E3F4A596B708ULL,
    0x9A7B6C5D4E3F2A19ULL,
    0x123456789ABCDEF0ULL
};

/*
 * RDT-CORE mixing function.
 * Combines recursive depth analysis, scalar field projection,
 * epsilon-channel perturbation, and ARX diffusion.
 */
static uint64_t rdt_mix(uint64_t x) {
    uint32_t d = rdt_depth_fast(x);
    uint32_t g = scalar_field(x);
    uint64_t m = (x + (uint64_t)g) * 0x9E3779B97F4A7C15ULL;
    uint64_t p = m ^ ((uint64_t)d * 0xBF58476D1CE4E5B9ULL);

    /* Epsilon-channel mixing */
    uint64_t eps = 0;
    uint32_t rounds = (d < 6 ? d : 6);
    for (uint32_t i = 0; i <= rounds; i++) {
        uint64_t c = p * (P[i] * 0xC2B2AE3D27D4EB4FULL);
        c ^= (p >> (i + 1)) * P[i];
        c ^= K[i & 3];
        c = rotl64(c, 13u + 7u * i);
        eps ^= c;
    }

    uint64_t z = p ^ eps;

    /* ARX finalizer */
    static const uint32_t ROT[3] = {13, 23, 43};
    static const uint64_t MUL[3] = {19, 29, 47};
    uint32_t rp = ROT[d % 3];
    uint64_t mp = MUL[d % 3] * 0xD6E8FEB86659FD93ULL;

    z ^= (z << rp);
    z ^= (z >> (rp >> 1));
    z *= mp;
    z ^= K[(d ^ rp) & 3];

    return rotl64(z, (uint32_t)(d ^ rp));
}

/* ============================================================================
 * RDT-PRNG_STREAM_v2 State and Functions
 * ============================================================================ */

/* 256-bit PRNG state */
static uint64_t S[4];

/*
 * Initialize from 4 × 64-bit seed values.
 * Applies mixing to ensure good initial state distribution
 * even from low-entropy seeds.
 */
void rdt_prng_v2_init(const uint64_t seed[4]) {
    /* Initial mixing with different constants per lane */
    uint64_t a = rdt_mix(seed[0] ^ 0x9E3779B97F4A7C15ULL);
    uint64_t b = rdt_mix(seed[1] ^ 0xBF58476D1CE4E5B9ULL);
    uint64_t c = rdt_mix(seed[2] ^ 0x94D049BB133111EBULL);
    uint64_t d = rdt_mix(seed[3] ^ 0xD6E8FEB86659FD93ULL);

    /* Cross-lane diffusion */
    a ^= rotl64(b, 17);
    b ^= rotl64(c, 31);
    c ^= rotl64(d, 47);
    d ^= rotl64(a, 13);

    /* Final state initialization with additional mixing */
    S[0] = rdt_mix(a ^ d);
    S[1] = rdt_mix(b ^ S[0]);
    S[2] = rdt_mix(c ^ S[1]);
    S[3] = rdt_mix(d ^ S[2]);

    /* Ensure non-zero state */
    if ((S[0] | S[1] | S[2] | S[3]) == 0) {
        S[0] = 0x9E3779B97F4A7C15ULL;
        S[1] = 0xBF58476D1CE4E5B9ULL;
        S[2] = 0x94D049BB133111EBULL;
        S[3] = 0xD6E8FEB86659FD93ULL;
    }
}

/*
 * Initialize from a 32-byte buffer (e.g., SHA-256 output).
 * Bytes are interpreted in little-endian order.
 */
void rdt_prng_v2_init_bytes(const uint8_t seed[32]) {
    uint64_t s[4];
    for (int i = 0; i < 4; i++) {
        s[i] = 0;
        for (int j = 0; j < 8; j++) {
            s[i] |= ((uint64_t)seed[i * 8 + j]) << (8 * j);
        }
    }
    rdt_prng_v2_init(s);
}

/*
 * Generate the next 64-bit pseudorandom value.
 * 
 * v2 enhancement: After mixing each state lane, applies
 * cross-state rotational diffusion for improved avalanche.
 */
uint64_t rdt_prng_v2_next(void) {
    /* Mix each state lane */
    uint64_t t0 = rdt_mix(S[1]);
    uint64_t t1 = rdt_mix(S[2]);
    uint64_t t2 = rdt_mix(S[3]);
    uint64_t t3 = rdt_mix(S[0]);

    /* XOR mixed values back into state */
    S[0] ^= t0;
    S[1] ^= t1;
    S[2] ^= t2;
    S[3] ^= t3;

    /* v2: Cross-state rotational diffusion */
    S[0] ^= rotl64(S[1], 21);
    S[1] ^= rotl64(S[2], 35);
    S[2] ^= rotl64(S[3], 49);
    S[3] ^= rotl64(S[0], 11);

    return S[0];
}

/*
 * Fill a buffer with pseudorandom bytes.
 */
void rdt_prng_v2_fill(uint8_t *buf, size_t len) {
    size_t i = 0;
    
    /* Fill 8 bytes at a time */
    while (i + 8 <= len) {
        uint64_t x = rdt_prng_v2_next();
        buf[i + 0] = (uint8_t)(x);
        buf[i + 1] = (uint8_t)(x >> 8);
        buf[i + 2] = (uint8_t)(x >> 16);
        buf[i + 3] = (uint8_t)(x >> 24);
        buf[i + 4] = (uint8_t)(x >> 32);
        buf[i + 5] = (uint8_t)(x >> 40);
        buf[i + 6] = (uint8_t)(x >> 48);
        buf[i + 7] = (uint8_t)(x >> 56);
        i += 8;
    }
    
    /* Handle remaining bytes */
    if (i < len) {
        uint64_t x = rdt_prng_v2_next();
        while (i < len) {
            buf[i++] = (uint8_t)x;
            x >>= 8;
        }
    }
}

/* ============================================================================
 * Streaming Main (for external test batteries)
 * 
 * Usage:
 *   ./rdt_prng_stream_v2 [seed_hex]
 *   ./rdt_prng_stream_v2 | dieharder -a -g 200
 *   ./rdt_prng_stream_v2 | smokerand default stdin64
 * ============================================================================ */

#ifdef RDT_PRNG_V2_MAIN

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    uint64_t seed[4];
    
    if (argc > 1) {
        /* Parse hex seed from command line */
        seed[0] = strtoull(argv[1], NULL, 16);
        seed[1] = (argc > 2) ? strtoull(argv[2], NULL, 16) : seed[0] ^ 0xBF58476D1CE4E5B9ULL;
        seed[2] = (argc > 3) ? strtoull(argv[3], NULL, 16) : seed[0] ^ 0x94D049BB133111EBULL;
        seed[3] = (argc > 4) ? strtoull(argv[4], NULL, 16) : seed[0] ^ 0xD6E8FEB86659FD93ULL;
    } else {
        /* Default seed (from sensor entropy validation) */
        seed[0] = 0xe607dabdfc9538b5ULL;
        seed[1] = 0x0050f7866258289cULL;
        seed[2] = 0xedc2d97a03b312adULL;
        seed[3] = 0xcaedbc215ece9a31ULL;
    }
    
    rdt_prng_v2_init(seed);
    
    /* Buffered output for maximum throughput */
    uint64_t buf[1024];
    for (;;) {
        for (int i = 0; i < 1024; i++) {
            buf[i] = rdt_prng_v2_next();
        }
        if (fwrite(buf, sizeof(uint64_t), 1024, stdout) != 1024) {
            break;
        }
    }
    
    return 0;
}

#endif /* RDT_PRNG_V2_MAIN */
