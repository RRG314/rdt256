#include "rdt_drbg_v2.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    rdt_drbg_v2_ctx ctx;
    uint8_t out[64];
    uint8_t zeros[sizeof(out)];

    memset(zeros, 0, sizeof(zeros));

    if (rdt_drbg_v2_init_system(&ctx, (const uint8_t *)"rdt-system-test", 15u) != RDT_DRBG_V2_OK) {
        fprintf(stderr, "system init failed\n");
        return 1;
    }
    if (rdt_drbg_v2_generate(&ctx, out, sizeof(out), NULL, 0u, 0) != RDT_DRBG_V2_OK) {
        fprintf(stderr, "generate after system init failed\n");
        return 1;
    }
    if (memcmp(out, zeros, sizeof(out)) == 0) {
        fprintf(stderr, "all-zero output after system init\n");
        return 1;
    }
    if (rdt_drbg_v2_reseed_system(&ctx, (const uint8_t *)"rdt-system-reseed", 17u) != RDT_DRBG_V2_OK) {
        fprintf(stderr, "system reseed failed\n");
        return 1;
    }
    if (rdt_drbg_v2_generate(&ctx, out, sizeof(out), NULL, 0u, 0) != RDT_DRBG_V2_OK) {
        fprintf(stderr, "generate after system reseed failed\n");
        return 1;
    }

    rdt_drbg_v2_zeroize(&ctx);
    puts("rdt_drbg_v2_system_test: ok");
    return 0;
}
