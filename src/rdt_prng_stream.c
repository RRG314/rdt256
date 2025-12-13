/*
 * RDT-PRNG
 * Recursive Division Tree–based PRNG
 * Author: Steven Reid
 *
 * 64-bit output, 256-bit state
 * Non-cryptographic
 */

#define _GNU_SOURCE
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>

/* -------------------------------------------------- */
/* Utilities                                          */
/* -------------------------------------------------- */

static inline uint64_t rotl64(uint64_t x, uint32_t r) {
    return (x << r) | (x >> (64 - r));
}

/* -------------------------------------------------- */
/* Recursive depth (RDT core)                         */
/* -------------------------------------------------- */

static inline uint32_t rdt_depth(uint64_t x) {
    if (x <= 1) return 0;
    uint32_t depth = 0;

    while (x > 1 && depth < 16) {
        double lx = log((double)x);
        uint32_t d = (uint32_t)(pow(lx, 1.5));
        if (d < 2) d = 2;
        uint64_t nx = x / d;
        if (nx == x || nx == 0) break;
        x = nx;
        depth++;
    }
    return depth;
}

/* -------------------------------------------------- */
/* Scalar field                                       */
/* -------------------------------------------------- */

static inline uint32_t isqrt32(uint32_t x) {
    uint32_t r = 0, bit = 1u << 30;
    while (bit > x) bit >>= 2;
    while (bit) {
        if (x >= r + bit) {
            x -= r + bit;
            r = (r >> 1) + bit;
        } else {
            r >>= 1;
        }
        bit >>= 2;
    }
    return r;
}

static inline uint32_t scalar_field(uint64_t x) {
    uint32_t a = (uint32_t)(x & 0xFFFFu);
    uint32_t b = (uint32_t)((x >> 16) & 0xFFFFu);
    return rdt_depth(isqrt32(a * a + b * b));
}

/* -------------------------------------------------- */
/* State                                              */
/* -------------------------------------------------- */

static uint64_t S[4];

static const uint64_t K[4] = {
    0xA3B1C6E5D4879F12ULL,
    0xC1D2E3F4A596B708ULL,
    0x9A7B6C5D4E3F2A19ULL,
    0x123456789ABCDEF0ULL
};

static const uint64_t P[7] = {3, 5, 7, 11, 13, 17, 19};

/* -------------------------------------------------- */
/* Init                                               */
/* -------------------------------------------------- */

static inline void rdt_init(uint64_t seed) {
    S[0] = seed ^ 0x9E3779B97F4A7C15ULL;
    S[1] = (seed << 1) ^ 0xC2B2AE3D27D4EB4FULL;
    S[2] = ~seed;
    S[3] = seed ^ (seed >> 1);
}

/* -------------------------------------------------- */
/* Core step                                          */
/* -------------------------------------------------- */

static inline uint64_t rdt_next(void) {
    uint32_t d = rdt_depth(S[0]);
    uint32_t g = scalar_field(S[0]);
    d = (d + (g & 3)) & 15;

    uint64_t z = S[0] ^ K[d & 3];
    z *= 0x9E3779B97F4A7C15ULL;
    z = rotl64(z, 17);

    uint64_t acc = z, eps = 0;

    for (uint32_t i = 0; i < d; i++) {
        acc ^= K[(i + d) & 3];
        acc *= P[i % 7] * 0xC2B2AE3D27D4EB4FULL;
        acc = rotl64(acc, 11 + i);
        acc += (uint64_t)(i + 1) * 0xBF58476D1CE4E5B9ULL;
        eps ^= acc;
    }

    z ^= eps;
    z ^= z << 23;
    z ^= z >> 11;
    z *= 0xD6E8FEB86659FD93ULL;

    S[0] = S[1];
    S[1] = S[2];
    S[2] = S[3];
    S[3] = z;

    return z;
}

/* -------------------------------------------------- */
/* Modes                                              */
/* -------------------------------------------------- */

static void do_stream(uint64_t seed) {
    rdt_init(seed);
    for (;;) {
        uint64_t v = rdt_next();
        fwrite(&v, sizeof(uint64_t), 1, stdout);
    }
}

static double now_s(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + 1e-9 * (double)ts.tv_nsec;
}

static void do_bench(uint64_t N) {
    rdt_init(0x0123456789ABCDEFULL);
    volatile uint64_t sink = 0;

    for (uint64_t i = 0; i < 100000; i++) sink ^= rdt_next();

    double t0 = now_s();
    for (uint64_t i = 0; i < N; i++) sink ^= rdt_next();
    double t1 = now_s();

    double dt = t1 - t0;
    double bytes = (double)N * 8.0;

    printf("outputs=%llu\n", (unsigned long long)N);
    printf("time_s=%.6f\n", dt);
    printf("MB_per_s=%.3f\n", (bytes / (1024.0 * 1024.0)) / dt);
    printf("ns_per_u64=%.3f\n", (dt * 1e9) / (double)N);

    if (sink == 0xDEADBEEF) printf("%llu\n", (unsigned long long)sink);
}

/* -------------------------------------------------- */
/* Main                                               */
/* -------------------------------------------------- */

int main(int argc, char **argv) {
    if (argc >= 2 && strcmp(argv[1], "stream") == 0) {
        uint64_t seed = 0x0123456789ABCDEFULL;
        if (argc >= 3) seed = strtoull(argv[2], NULL, 0);
        do_stream(seed);
        return 0;
    }

    if (argc >= 2 && strcmp(argv[1], "bench") == 0) {
        uint64_t N = (argc >= 3) ? strtoull(argv[2], NULL, 0) : 200000000ULL;
        do_bench(N);
        return 0;
    }

    fprintf(stderr,
        "Usage:\n"
        "  %s stream [seed]\n"
        "  %s bench [N]\n",
        argv[0], argv[0]
    );
    return 0;
}
