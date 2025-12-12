#include "rdt.h"

/* Utility functions */
static inline uint64_t rotl64(uint64_t x, uint32_t r) {
    return (x << r) | (x >> (64 - r));
}

static inline uint32_t bit_length(uint64_t x) {
    return x ? 64 - __builtin_clzll(x) : 0;
}

static inline uint32_t popcount64(uint64_t x) {
    return __builtin_popcountll(x);
}

/* RDT depth function */
static inline uint32_t rdt_depth(uint64_t x) {
    uint32_t bl = bit_length(x);
    uint32_t pc = popcount64(x);
    uint32_t mid = bl ? (x >> (bl >> 1)) : 0;
    return (bl ^ (pc << 1) ^ mid) & 63;
}

/* 32-bit integer square root */
static inline uint32_t isqrt32(uint32_t x) {
    uint32_t r = 0;
    uint32_t bit = 1U << 30;

    while (bit > x) bit >>= 2;
    while (bit) {
        if (x >= r + bit) {
            x -= r + bit;
            r += bit << 1;
        }
        r >>= 1;
        bit >>= 2;
    }
    return r;
}

/* Scalar projection */
static inline uint32_t scalar_field(uint64_t x) {
    uint32_t a = (uint32_t)(x & 0xFFFF);
    uint32_t b = (uint32_t)(x >> 16);
    return rdt_depth(isqrt32(a*a + b*b));
}

/* Epsilon channel */
static const uint64_t PSET[7] = {3,5,7,11,13,17,19};

static inline uint64_t eps_channel(uint64_t x, uint32_t depth, const uint64_t K[4]) {
    uint64_t eps = 0;
    uint32_t rounds = (depth < 6 ? depth : 6);

    for (uint32_t i = 0; i <= rounds; i++) {
        uint64_t p = PSET[i];
        uint64_t c = x * (p * 0x9E3779B97F4A7C15ULL);
        c ^= ((x >> (i+1)) * p);
        c ^= K[i & 3];
        c = rotl64(c, (13 + 7*i) & 63);
        eps ^= c;
    }
    return eps;
}

/* ARX depth-mixer */
static const uint32_t ROT_P[3] = {13,23,43};
static const uint32_t MUL_P[3] = {19,29,47};

static inline uint64_t depth_arx(uint64_t x, uint32_t d, const uint64_t K[4]) {
    uint32_t rp = ROT_P[d % 3];
    uint64_t mp = MUL_P[d % 3];

    uint64_t z = x;
    z ^= (z << (rp & 63));
    z ^= (z >> ((rp >> 1) & 63));
    z *= (mp * 0x9E3779B97F4A7C15ULL);
    z ^= K[(d ^ rp) & 3];

    return rotl64(z, (rp ^ d) & 63);
}

/* Exported mixing function */
uint64_t rdt_mix(uint64_t x, const uint64_t K[4]) {
    uint32_t d = rdt_depth(x);
    uint32_t g = scalar_field(x);
    uint64_t m = (x + g) * ((1ULL << 16) - 1) + 0x9E3779B97F4A7C15ULL;
    uint64_t p = m ^ (d * 0xE7037ED1ULL);
    uint64_t e = eps_channel(p, d, K);
    uint64_t h = depth_arx(p ^ e, d, K);
    return h;
}
