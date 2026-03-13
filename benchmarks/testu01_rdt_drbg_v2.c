#include "rdt_drbg_v2.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <bbattery.h>
#include <unif01.h>

static rdt_drbg_v2_ctx ctx;

static unsigned int rdt_drbg_v2_bits(void) {
    uint32_t x = 0;
    if (rdt_drbg_v2_generate(&ctx, (uint8_t *)&x, sizeof(x), NULL, 0u, 0) != RDT_DRBG_V2_OK) {
        fprintf(stderr, "rdt_drbg_v2_generate failed inside BigCrush wrapper\n");
        return 0u;
    }
    return (unsigned int)x;
}

int main(int argc, char **argv) {
    unif01_Gen *gen;
    const char *battery = "big";

    if (rdt_drbg_v2_init_u64(&ctx,
                             0xe607dabdfc9538b5ULL,
                             0x0050f7866258289cULL,
                             0xedc2d97a03b312adULL) != RDT_DRBG_V2_OK) {
        fprintf(stderr, "failed to initialize rdt_drbg_v2\n");
        return 1;
    }

    if (argc > 1) {
        battery = argv[1];
    }

    gen = unif01_CreateExternGenBits("rdt_drbg_v2", rdt_drbg_v2_bits);
    if (strcmp(battery, "small") == 0) {
        bbattery_SmallCrush(gen);
    } else if (strcmp(battery, "crush") == 0) {
        bbattery_Crush(gen);
    } else {
        bbattery_BigCrush(gen);
    }
    unif01_DeleteExternGenBits(gen);
    rdt_drbg_v2_zeroize(&ctx);
    return 0;
}
