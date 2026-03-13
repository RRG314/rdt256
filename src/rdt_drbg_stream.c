#include "rdt_drbg.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    uint64_t entropy_seed = 0x0123456789ABCDEFULL;
    uint64_t nonce = 0xFEDCBA9876543210ULL;
    uint64_t personalization = 0xA5A5A5A5A5A5A5A5ULL;
    uint64_t buf[4096];
    size_t i;
    static char io_buf[1 << 20];

    if (argc > 1) {
        entropy_seed = strtoull(argv[1], NULL, 0);
    }
    if (argc > 2) {
        nonce = strtoull(argv[2], NULL, 0);
    }
    if (argc > 3) {
        personalization = strtoull(argv[3], NULL, 0);
    }

    setvbuf(stdout, io_buf, _IOFBF, sizeof(io_buf));
    rdt_drbg_init_u64(entropy_seed, nonce, personalization);

    for (;;) {
        for (i = 0; i < sizeof(buf) / sizeof(buf[0]); i++) {
            buf[i] = rdt_drbg_next_u64();
        }
        if (fwrite(buf, sizeof(buf[0]), sizeof(buf) / sizeof(buf[0]), stdout)
            != sizeof(buf) / sizeof(buf[0])) {
            break;
        }
    }

    return 0;
}
