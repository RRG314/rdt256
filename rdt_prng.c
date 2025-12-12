#include "rdt.h"

void rdt_prng_init(rdt_prng_state *st, uint64_t seed) {
    st->S[0] = seed;
    st->S[1] = seed ^ 0x9E3779B97F4A7C15ULL;
    st->S[2] = seed << 1;
    st->S[3] = ~seed;
}

uint64_t rdt_prng_next(rdt_prng_state *st) {
    static const uint64_t K[4] = {
        0x243F6A8885A308D3ULL,
        0x13198A2E03707344ULL,
        0xA4093822299F31D0ULL,
        0x082EFA98EC4E6C89ULL
    };

    /* evolve internal state */
    st->S[0] ^= rdt_mix(st->S[1], K);
    st->S[1] ^= rdt_mix(st->S[2], K);
    st->S[2] ^= rdt_mix(st->S[3], K);
    st->S[3] ^= rdt_mix(st->S[0], K);

    return st->S[0];
}
