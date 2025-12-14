#include "rdt.h"

/* 256-bit internal state */
static uint64_t S[4];

/* Fixed key schedule */
static const uint64_t K[4] = {
    0xA3B1C6E5D4879F12ULL,
    0xC1D2E3F4A596B708ULL,
    0x9A7B6C5D4E3F2A19ULL,
    0x123456789ABCDEF0ULL
};

void rdt_prng_init(uint64_t seed) {
    S[0] = seed ^ 0x9E3779B97F4A7C15ULL;
    S[1] = (seed << 1) ^ 0xC2B2AE3D27D4EB4FULL;
    S[2] = ~seed;
    S[3] = seed ^ (seed >> 1);
}

uint64_t rdt_prng_next(void) {
    uint64_t t0 = rdt_mix(S[1], K);
    uint64_t t1 = rdt_mix(S[2], K);
    uint64_t t2 = rdt_mix(S[3], K);
    uint64_t t3 = rdt_mix(S[0], K);

    S[0] ^= t0;
    S[1] ^= t1;
    S[2] ^= t2;
    S[3] ^= t3;

    return S[0];
}
