#include "rdt_core.h"
#include <stdint.h>

static inline uint32_t bit_length(uint64_t x) {
    return x ? 64 - __builtin_clzll(x) : 0;
}

static inline uint32_t popcount64(uint64_t x) {
    return __builtin_popcountll(x);
}

static inline uint64_t rotl64(uint64_t x, uint32_t r) {
    return (x << r) | (x >> (64 - r));
}

static inline uint32_t rdt_depth_fast(uint64_t x) {
    uint32_t bl = bit_length(x);
    uint32_t pc = popcount64(x);
    uint32_t mid = (bl ? (x >> (bl >> 1)) : 0);
    return (bl ^ (pc << 1) ^ mid) & 63;
}

static inline uint32_t isqrt32(uint32_t x) {
    uint32_t r = 0, bit = 1u << 30;
    while (bit > x) bit >>= 2;
    while (bit != 0) {
        if (x >= r + bit) { x -= r + bit; r += bit << 1; }
        r >>= 1;
        bit >>= 2;
    }
    return r;
}

static inline uint32_t scalar_field(uint64_t x) {
    uint32_t a = (uint32_t)(x & 0xFFFFu);
    uint32_t b = (uint32_t)((x >> 16) & 0xFFFFu);
    uint32_t t = isqrt32(a*a + b*b);
    return rdt_depth_fast(t);
}

/* Prime table for epsilon channel */
static const uint64_t P[7] = {3,5,7,11,13,17,19};

uint64_t rdt_mix(uint64_t x, uint64_t K[4])
{
    uint32_t d = rdt_depth_fast(x);
    uint32_t g = scalar_field(x);
    uint64_t m = (x + g) * 0x9E3779B97F4A7C15ULL;
    uint64_t p = m ^ ((uint64_t)d * 0xBF58476D1CE4E5B9ULL);

    /* epsilon mixing */
    uint64_t eps = 0;
    uint32_t rounds = (d < 6 ? d : 6);

    for (uint32_t i = 0; i <= rounds; i++) {
        uint64_t c = p * (P[i] * 0xC2B2AE3D27D4EB4FULL);
        c ^= (p >> (i + 1)) * P[i];
        c ^= K[i & 3];
        c = rotl64(c, 13 + 7*i);
        eps ^= c;
    }

    uint64_t z = p ^ eps;

    /* ARX mixer */
    static const uint32_t ROT[3] = {13,23,43};
    static const uint64_t MUL[3] = {19,29,47};

    uint32_t rp = ROT[d % 3];
    uint64_t mp = MUL[d % 3] * 0xD6E8FEB86659FD93ULL;

    z ^= (z << rp);
    z ^= (z >> (rp >> 1));
    z *= mp;
    z ^= K[(d ^ rp) & 3];

    return rotl64(z, (d ^ rp));
}
