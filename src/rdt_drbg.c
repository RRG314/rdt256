#include "rdt_drbg.h"
#include "rdt_core.h"
#include <string.h>

/* -----------------------
   Internal DRBG state
   ----------------------- */
static uint64_t Kd[4];     /* 256-bit key */
static uint64_t Vd[2];     /* 128-bit counter */
static uint64_t reseed_counter = 0;

/* -----------------------
   Low-level helpers
   ----------------------- */

static inline uint64_t rotl64(uint64_t x, uint32_t r) {
    return (x << (r & 63)) | (x >> ((64 - r) & 63));
}

/* SplitMix64 expander for init material */
static uint64_t splitmix64_next(uint64_t *x) {
    uint64_t z = (*x += 0x9E3779B97F4A7C15ULL);
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
    return z ^ (z >> 31);
}

/* 128-bit increment of V */
static inline void inc_V(void) {
    Vd[0] += 1;
    if (Vd[0] == 0) Vd[1] += 1;
}

/*
We want shell indices driven by your "recursive ultrametric" depth notion.
You already have d = rdt_depth_fast(x) inside rdt_mix, but we also want a shell
to control schedule OUTSIDE rdt_mix.

We can't call rdt_depth_fast() directly since it's static inline inside your core C file.
So we re-implement a compatible "shell driver" using only public operations.

This driver doesn't need to match your internal depth bit-for-bit; it just needs
to be a stable, deterministic ultrametric-ish classifier.
*/
static inline uint32_t bit_length_u64(uint64_t x) {
#if defined(__GNUC__) || defined(__clang__)
    return x ? (uint32_t)(64 - __builtin_clzll(x)) : 0u;
#else
    uint32_t n = 0;
    while (x) { n++; x >>= 1; }
    return n;
#endif
}

static inline uint32_t popcount_u64(uint64_t x) {
#if defined(__GNUC__) || defined(__clang__)
    return (uint32_t)__builtin_popcountll(x);
#else
    uint32_t c = 0;
    while (x) { c += (uint32_t)(x & 1u); x >>= 1; }
    return c;
#endif
}

/* Shell classifier (0..63) similar in spirit to your rdt_depth_fast */
static inline uint32_t rdt_shell64(uint64_t x) {
    uint32_t bl = bit_length_u64(x);
    uint32_t pc = popcount_u64(x);
    uint64_t mid = (bl ? (x >> (bl >> 1)) : 0);
    return (bl ^ (pc << 1) ^ (uint32_t)mid) & 63u;
}

/*
Block function built from your rdt_mix, but with a shell-controlled schedule.
Returns 64-bit block.
*/
static uint64_t rdt_drbg_block(uint64_t V0, uint64_t V1, const uint64_t K[4]) {
    uint64_t probe = (V0 ^ K[0]) ^ rotl64(V1, 17);
    uint32_t d = rdt_shell64(probe);
    uint32_t shell = d & 7u;

    /* rounds 3..6 depending on shell */
    uint32_t rounds = 3u + (shell & 3u);

    /* small permutation family based on shell */
    uint8_t perm[4];
    switch (shell) {
        case 0: perm[0]=0; perm[1]=1; perm[2]=2; perm[3]=3; break;
        case 1: perm[0]=1; perm[1]=0; perm[2]=3; perm[3]=2; break;
        case 2: perm[0]=2; perm[1]=3; perm[2]=0; perm[3]=1; break;
        case 3: perm[0]=3; perm[1]=2; perm[2]=1; perm[3]=0; break;
        case 4: perm[0]=0; perm[1]=2; perm[2]=1; perm[3]=3; break;
        case 5: perm[0]=1; perm[1]=3; perm[2]=2; perm[3]=0; break;
        case 6: perm[0]=2; perm[1]=0; perm[2]=3; perm[3]=1; break;
        default:perm[0]=3; perm[1]=1; perm[2]=0; perm[3]=2; break;
    }

    /* local lanes */
    uint64_t x[4];
    x[0] = V0 + K[0] + 0xD1342543DE82EF95ULL;
    x[1] = V1 + K[1] + 0xC42B7E5E3A6C1B47ULL;
    x[2] = (V0 ^ V1) + K[2] + 0x9E3779B97F4A7C15ULL;
    x[3] = (rotl64(V0, 32) ^ V1) + K[3] + 0xBF58476D1CE4E5B9ULL;

    /* shell-controlled churn */
    for (uint32_t r = 0; r < rounds; r++) {
        uint8_t a = perm[(r + 0u) & 3u];
        uint8_t b = perm[(r + 1u) & 3u];
        uint8_t c = perm[(r + 2u) & 3u];
        uint8_t e = perm[(r + 3u) & 3u];

        /* ARX-ish cross-coupling */
        x[a] += x[b] + (uint64_t)(shell + 1u) * 0x9E3779B97F4A7C15ULL;
        x[e] ^= rotl64(x[a], 13u + 7u * (r & 3u));

        /* your keyed nonlinear primitive does the heavy lifting */
        x[c] = rdt_mix(x[c] ^ x[e] ^ ((uint64_t)r << 32) ^ (uint64_t)shell, K);
        x[b] = rdt_mix(x[b] + x[c] + (uint64_t)(r + shell), K);

        /* extra diffusion */
        x[(a + 1u) & 3u] ^= rotl64(x[(c + 2u) & 3u], 23u + (shell & 7u));
        x[(b + 2u) & 3u] += x[(e + 3u) & 3u] ^ (uint64_t)d;
    }

    return rdt_mix(x[0] ^ x[1] ^ x[2] ^ x[3] ^ ((uint64_t)d << 56), K);
}

/* absorb bytes into 4 words using rdt_mix keyed by current K */
static void absorb_bytes(uint64_t W[4], const uint8_t *in, size_t in_len, const uint64_t K[4]) {
    uint64_t lane = 0;
    unsigned shift = 0;
    size_t wi = 0;

    for (size_t i = 0; i < in_len; i++) {
        lane |= ((uint64_t)in[i]) << shift;
        shift += 8;
        if (shift == 64) {
            W[wi & 3u] ^= rdt_mix(lane ^ (0xA5A5A5A5A5A5A5A5ULL + (uint64_t)wi), K);
            wi++;
            lane = 0;
            shift = 0;
        }
    }
    if (shift) {
        W[wi & 3u] ^= rdt_mix(lane ^ (0xC2B2AE3D27D4EB4FULL + (uint64_t)wi), K);
    }

    /* diffuse */
    for (int r = 0; r < 2; r++) {
        for (int j = 0; j < 4; j++) {
            W[j] = rdt_mix(W[j] + 0x9E3779B97F4A7C15ULL * (uint64_t)(j + 1 + 4*r), K);
        }
    }
}

/* make provided_data PD[4] from (entropy||additional) */
static void make_PD(uint64_t PD[4],
                    const uint8_t *entropy, size_t entropy_len,
                    const uint8_t *additional, size_t additional_len) {
    PD[0] = 0x243F6A8885A308D3ULL;
    PD[1] = 0x13198A2E03707344ULL;
    PD[2] = 0xA4093822299F31D0ULL;
    PD[3] = 0x082EFA98EC4E6C89ULL;

    if (entropy && entropy_len) absorb_bytes(PD, entropy, entropy_len, Kd);
    if (additional && additional_len) absorb_bytes(PD, additional, additional_len, Kd);

    for (int i = 0; i < 4; i++) {
        PD[i] = rdt_mix(PD[i] ^ (uint64_t)(0x9E3779B97F4A7C15ULL * (uint64_t)(i + 1)), Kd);
    }
}

/*
Update (CTR_DRBG style):
temp = 4 blocks from Block(K, ++V)
temp ^= PD (if present)
K <- mix(temp, oldK)
V <- mix(temp, newK)
*/
static void drbg_update(const uint64_t PD[4], int has_pd) {
    uint64_t temp[4];

    for (int i = 0; i < 4; i++) {
        inc_V();
        temp[i] = rdt_drbg_block(Vd[0], Vd[1], Kd);
    }
    if (has_pd) {
        for (int i = 0; i < 4; i++) temp[i] ^= PD[i];
    }

    /* update key */
    uint64_t oldK[4] = {Kd[0], Kd[1], Kd[2], Kd[3]};
    for (int i = 0; i < 4; i++) {
        Kd[i] = rdt_mix(temp[i] + 0x9E3779B97F4A7C15ULL * (uint64_t)i, oldK);
    }

    /* update counter state from temp under new key (avoid pure counter V) */
    Vd[0] ^= rdt_mix(temp[0] ^ temp[2], Kd);
    Vd[1] ^= rdt_mix(temp[1] ^ temp[3], Kd);

    reseed_counter++;
}

/* -----------------------
   Public API
   ----------------------- */

void rdt_drbg_init_u64(uint64_t entropy_seed,
                       uint64_t nonce,
                       uint64_t personalization) {
    uint64_t sm = entropy_seed
                ^ rotl64(nonce, 13)
                ^ rotl64(personalization, 27)
                ^ 0xA5A5A5A5A5A5A5A5ULL;

    for (int i = 0; i < 4; i++) Kd[i] = splitmix64_next(&sm);
    Vd[0] = splitmix64_next(&sm);
    Vd[1] = splitmix64_next(&sm);

    reseed_counter = 1;

    /* one-time diffusion using PD derived from (seed,nonce,personalization) */
    uint8_t mat[24];
    memcpy(mat + 0,  &entropy_seed, 8);
    memcpy(mat + 8,  &nonce, 8);
    memcpy(mat + 16, &personalization, 8);

    uint64_t PD[4];
    make_PD(PD, mat, sizeof(mat), NULL, 0);
    drbg_update(PD, 1);
}

void rdt_drbg_reseed(const uint8_t *entropy, size_t entropy_len,
                     const uint8_t *additional, size_t additional_len) {
    uint64_t PD[4];
    make_PD(PD, entropy, entropy_len, additional, additional_len);
    reseed_counter = 1;
    drbg_update(PD, 1);
}

int rdt_drbg_generate(uint8_t *out, size_t out_len,
                      const uint8_t *additional, size_t additional_len,
                      int prediction_resistance) {
    if (!out && out_len) return -1;

    (void)prediction_resistance; /* policy is caller-driven reseed for true PR */

    /* pre-update with additional input (standard DRBG practice) */
    if (additional && additional_len) {
        uint64_t PD[4];
        make_PD(PD, NULL, 0, additional, additional_len);
        drbg_update(PD, 1);
    }

    /* generate bytes via Block(K, ++V) */
    size_t produced = 0;
    while (produced < out_len) {
        inc_V();
        uint64_t block = rdt_drbg_block(Vd[0], Vd[1], Kd);

        size_t take = (out_len - produced < 8) ? (out_len - produced) : 8;
        for (size_t i = 0; i < take; i++) {
            out[produced + i] = (uint8_t)(block >> (8u * i));
        }
        produced += take;
    }

    /* post-update with no PD for backtracking resistance */
    drbg_update(NULL, 0);

    return 0;
}

uint64_t rdt_drbg_next_u64(void) {
    uint64_t x = 0;
    (void)rdt_drbg_generate((uint8_t*)&x, sizeof(x), NULL, 0, 0);
    return x;
}

void rdt_drbg_zeroize(void) {
    volatile uint64_t *p = (volatile uint64_t*)Kd;
    for (int i = 0; i < 4; i++) p[i] = 0;
    volatile uint64_t *q = (volatile uint64_t*)Vd;
    q[0] = q[1] = 0;
    reseed_counter = 0;
}
