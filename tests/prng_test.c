#include <stdio.h>
#include <stdint.h>
#include "rdt_drbg.h"

int main(void)
{
    rdt_drbg_init_u64(0x123456789ABCDEF0ULL,
                      0x0F1E2D3C4B5A6978ULL,
                      0xA55AA55AA55AA55AULL);

    while (1) {
        uint64_t x = rdt_drbg_next_u64();
        fwrite(&x, sizeof(x), 1, stdout);
    }
}
