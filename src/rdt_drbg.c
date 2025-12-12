#include "rdt.h"

static const uint64_t KCONST[4] = {
    0x3BD39E10CB0EF593ULL,
    0xC1D1F0A3379B2E6AULL,
    0x5F7A13C9240BADF1ULL,
    0x9A2C5F0137E60C4EULL
};

void rdt_drbg_init(rdt_drbg_state *st, const uint64_t seed[4]) {
    for (int i = 0; i < 4; i++) {
        st->K[i] = seed[i] ^ KCONST[i];
        st->S[i] = seed[i] ^ ~KCONST[i];
    }
    st->reseed_counter = 1;
}

void rdt_drbg_reseed(rdt_drbg_state *st, const uint64_t seed[4]) {
    for (int i = 0; i < 4; i++) {
        st->K[i] ^= seed[i];
        st->S[i] ^= rdt_mix(seed[i], st->K);
    }
    st->reseed_counter = 1;
}

uint64_t rdt_drbg_next(rdt_drbg_state *st) {
    /* Mix state */
    st->S[0] = rdt_mix(st->S[0] ^ st->K[0], st->K);
    st->S[1] = rdt_mix(st->S[1] ^ st->K[1], st->K);
    st->S[2] = rdt_mix(st->S[2] ^ st->K[2], st->K);
    st->S[3] = rdt_mix(st->S[3] ^ st->K[3], st->K);

    /* Key evolution for forward secrecy */
    for (int i = 0; i < 4; i++)
        st->K[i] ^= rdt_mix(st->S[i], st->K);

    /* Reseed interval e.g. every 2^20 outputs */
    if (++st->reseed_counter >= (1ULL << 20))
        rdt_drbg_reseed(st, st->S);

    return st->S[0];
}
