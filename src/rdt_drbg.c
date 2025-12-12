#include "rdt.h"
#include "rdt_core.h"
#include <stdint.h>

static uint64_t Kd[4];
static uint64_t Sd[4];
static uint64_t reseed_counter = 0;

void rdt_drbg_init(uint64_t seed)
{
    Kd[0] = seed ^ 0xA5A5A5A5A5A5A5A5ULL;
    Kd[1] = ~seed;
    Kd[2] = seed * 0x9E3779B97F4A7C15ULL;
    Kd[3] = seed ^ (seed << 1);

    Sd[0] = seed ^ 0xC2B2AE3D27D4EB4FULL;
    Sd[1] = ~seed;
    Sd[2] = (seed << 1) ^ 0xD6E8FEB86659FD93ULL;
    Sd[3] = seed ^ 0x123456789ABCDEF0ULL;
}

uint64_t rdt_drbg_next(void)
{
    for (int i = 0; i < 4; i++) {
        Sd[i] = rdt_mix(Sd[i] ^ Kd[i], Kd);
        Kd[i] ^= rdt_mix(Sd[i], Kd);
    }

    reseed_counter++;

    return Sd[0];
}
