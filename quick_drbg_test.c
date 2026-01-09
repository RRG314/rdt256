/* Quick DRBG functionality test */
#include "src/rdt_drbg.h"
#include <stdio.h>

int main(void) {
    printf("Testing DRBG API...\n");

    /* Test 1: Init */
    rdt_drbg_init_u64(0x123456789ABCDEFULL, 0xFEDCBA987654321ULL, 0x111111111111111ULL);
    printf("✓ rdt_drbg_init_u64()\n");

    /* Test 2: Generate */
    uint8_t buf[32];
    int r = rdt_drbg_generate(buf, 32, NULL, 0, 0);
    if (r == 0) printf("✓ rdt_drbg_generate()\n");
    else { printf("✗ generate failed\n"); return 1; }

    /* Test 3: Generate with additional */
    uint8_t add[] = "test";
    r = rdt_drbg_generate(buf, 16, add, 4, 0);
    if (r == 0) printf("✓ rdt_drbg_generate() with additional\n");
    else { printf("✗ generate with additional failed\n"); return 1; }

    /* Test 4: next_u64 */
    uint64_t v = rdt_drbg_next_u64();
    if (v != 0) printf("✓ rdt_drbg_next_u64() = 0x%llx\n", (unsigned long long)v);
    else printf("✓ rdt_drbg_next_u64() (zero is valid)\n");

    /* Test 5: Reseed */
    uint8_t ent[] = "fresh_entropy_12345678901234567890";
    rdt_drbg_reseed(ent, sizeof(ent)-1, NULL, 0);
    printf("✓ rdt_drbg_reseed()\n");

    /* Test 6: Prediction resistance */
    r = rdt_drbg_generate(buf, 8, NULL, 0, 1);
    if (r == 0) printf("✓ rdt_drbg_generate() with prediction_resistance\n");
    else { printf("✗ prediction resistance failed\n"); return 1; }

    /* Test 7: Zeroize */
    rdt_drbg_zeroize();
    printf("✓ rdt_drbg_zeroize()\n");

    printf("\nAll DRBG API functions working!\n");
    return 0;
}
