#include "rdt_drbg_v2.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

static int hex_to_bytes(const char *hex, uint8_t *out, size_t out_len) {
    size_t i;
    if (strlen(hex) != out_len * 2u) {
        return 0;
    }
    for (i = 0; i < out_len; i++) {
        unsigned value;
        if (sscanf(hex + i * 2u, "%2x", &value) != 1) {
            return 0;
        }
        out[i] = (uint8_t)value;
    }
    return 1;
}

int main(void) {
    rdt_drbg_v2_ctx ctx;
    uint8_t entropy[32];
    uint8_t nonce[16];
    uint8_t expected[128];
    uint8_t first[128];
    uint8_t second[128];
    uint8_t sample[4096];
    size_t i;

    if (!hex_to_bytes("ca851911349384bffe89de1cbdc46e6831e44d34a4fb935ee285dd14b71a7488", entropy, sizeof(entropy))) {
        fprintf(stderr, "failed to parse entropy vector\n");
        return 1;
    }
    if (!hex_to_bytes("659ba96c601dc69fc902940805ec0ca8", nonce, sizeof(nonce))) {
        fprintf(stderr, "failed to parse nonce vector\n");
        return 1;
    }
    if (!hex_to_bytes(
            "e528e9abf2dece54d47c7e75e5fe302149f817ea9fb4bee6f4199697d04d5b89"
            "d54fbb978a15b5c443c9ec21036d2460b6f73ebad0dc2aba6e624abf07745bc1"
            "07694bb7547bb0995f70de25d6b29e2d3011bb19d27676c07162c8b5ccde0668"
            "961df86803482cb37ed6d5c0bb8d50cf1f50d476aa0458bdaba806f48be9dcb8",
            expected, sizeof(expected))) {
        fprintf(stderr, "failed to parse expected vector\n");
        return 1;
    }

    if (rdt_drbg_v2_instantiate(&ctx, entropy, sizeof(entropy), nonce, sizeof(nonce), NULL, 0u) != RDT_DRBG_V2_OK) {
        fprintf(stderr, "instantiate failed\n");
        return 1;
    }
    if (rdt_drbg_v2_generate(&ctx, first, sizeof(first), NULL, 0u, 0) != RDT_DRBG_V2_OK) {
        fprintf(stderr, "first generate failed\n");
        return 1;
    }
    if (rdt_drbg_v2_generate(&ctx, second, sizeof(second), NULL, 0u, 0) != RDT_DRBG_V2_OK) {
        fprintf(stderr, "second generate failed\n");
        return 1;
    }
    if (memcmp(second, expected, sizeof(expected)) != 0) {
        fprintf(stderr, "NIST SHA-256 HMAC-DRBG known-answer test failed\n");
        return 1;
    }

    if (rdt_drbg_v2_init_u64(&ctx, 0x123456789abcdef0ULL, 0x0f1e2d3c4b5a6978ULL, 0xa55aa55aa55aa55aULL) != RDT_DRBG_V2_OK) {
        fprintf(stderr, "u64 init failed\n");
        return 1;
    }
    if (rdt_drbg_v2_generate(&ctx, sample, sizeof(sample), (const uint8_t *)"rdt-v2-smoke", 12u, 0) != RDT_DRBG_V2_OK) {
        fprintf(stderr, "smoke generate failed\n");
        return 1;
    }
    for (i = 1; i < sizeof(sample); i++) {
        if (sample[i] != sample[0]) {
            rdt_drbg_v2_zeroize(&ctx);
            puts("rdt_drbg_v2_test: ok");
            return 0;
        }
    }

    fprintf(stderr, "degenerate sample detected\n");
    return 1;
}
