#include <stdio.h>
#include "rdt.h"

int main() {
    rdt_prng_state prng;
    rdt_prng_init(&prng, 12345);

    for (int i = 0; i < 10; i++)
        printf("%016llx\n", (unsigned long long)rdt_prng_next(&prng));
}
gcc -O3 rdt_core.c rdt_prng.c rdt_drbg.c example.c -o example
