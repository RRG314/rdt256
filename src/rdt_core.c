#include "rdt_core.h"

/* ---------- helpers (hot path, inline) ---------- */

static inline uint32_t bit_length(uint64_t x) {
    return x ? 64u - (uint32_t)__builtin_clzll(x) : 0u;
}

static inline uint32_t popcount64(uint64_t x) {
    return (uint32_t)__builtin_popcountll(x);
}

static inline uint32_t rdt_depth_fast(uint64_t x) {
    uint32_t bl = bit_length(x);
    uint32_t mid = bl ? (uint32_t)(x >> (bl >> 1)) : 0u;
    return bl + popcount64(mid);
}

/* ---------- core nonlinear mix ---------- */

uint64_t rdt_mix(uint64_t x, const uint64_t K[4]) {
    uint32_t d = rdt_depth_fast(x);
    uint32_t r = d & 63u;

    x ^= K[d & 3u];
    x ^= (x << r) | (x >> (64u - r));
    x *= (uint64_t)(0x9E3779B97F4A7C15ULL ^ d);

    return x;
}
