#include <stdio.h>
#include <stdint.h>
#include "rdt_drbg.h"

int main(void)
{
    rdt_drbg_init(0x123456789ABCDEF0ULL);

    while (1) {
        uint64_t x = rdt_drbg_next();
        fwrite(&x, sizeof(x), 1, stdout);
    }
}
