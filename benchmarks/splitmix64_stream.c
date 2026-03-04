#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static inline uint64_t splitmix64_next(uint64_t *x) {
    uint64_t z = (*x += 0x9E3779B97F4A7C15ULL);
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
    return z ^ (z >> 31);
}

int main(int argc, char **argv) {
    uint64_t state = 0x0123456789ABCDEFULL;
    if (argc > 1) state = strtoull(argv[1], NULL, 0);

    uint64_t buf[1024];
    for (;;) {
        for (int i = 0; i < 1024; i++) {
            buf[i] = splitmix64_next(&state);
        }
        fwrite(buf, sizeof(uint64_t), 1024, stdout);
    }
}
