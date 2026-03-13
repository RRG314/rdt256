#include "rdt_drbg_v2.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
    rdt_drbg_v2_ctx ctx;
    uint64_t entropy_seed = 0xe607dabdfc9538b5ULL;
    uint64_t nonce = 0x0050f7866258289cULL;
    uint64_t personalization = 0xedc2d97a03b312adULL;
    uint8_t buf[1 << 16];
    static char io_buf[1 << 20];
    int rc;

    if (argc > 1 && strcmp(argv[1], "--system") == 0) {
        const uint8_t *personalization_bytes = NULL;
        size_t personalization_len = 0u;

        if (argc > 2) {
            personalization_bytes = (const uint8_t *)argv[2];
            personalization_len = strlen(argv[2]);
        }

        rc = rdt_drbg_v2_init_system(&ctx, personalization_bytes, personalization_len);
    } else {
        if (argc > 1) {
            entropy_seed = strtoull(argv[1], NULL, 0);
        }
        if (argc > 2) {
            nonce = strtoull(argv[2], NULL, 0);
        }
        if (argc > 3) {
            personalization = strtoull(argv[3], NULL, 0);
        }

        rc = rdt_drbg_v2_init_u64(&ctx, entropy_seed, nonce, personalization);
    }

    if (rc != RDT_DRBG_V2_OK) {
        return 1;
    }

    setvbuf(stdout, io_buf, _IOFBF, sizeof(io_buf));
    for (;;) {
        if (rdt_drbg_v2_generate(&ctx, buf, sizeof(buf), NULL, 0u, 0) != RDT_DRBG_V2_OK) {
            break;
        }
        if (fwrite(buf, 1u, sizeof(buf), stdout) != sizeof(buf)) {
            break;
        }
    }

    rdt_drbg_v2_zeroize(&ctx);
    return 0;
}
