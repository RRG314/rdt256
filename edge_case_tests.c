/* Edge case testing for RDT components */
#include "src/rdt_drbg.h"
#include "src/rdt_seed_extractor.h"
#include <stdio.h>
#include <string.h>

int test_passed = 0;
int test_failed = 0;

void test(const char *name, int condition) {
    if (condition) {
        printf("✓ %s\n", name);
        test_passed++;
    } else {
        printf("✗ %s FAILED\n", name);
        test_failed++;
    }
}

int main(void) {
    printf("=== RDT Edge Case Testing ===\n\n");

    /* ========================================
       DRBG Edge Cases
       ======================================== */
    printf("--- DRBG Edge Cases ---\n");

    /* Test 1: Generate 0 bytes (should succeed with no output) */
    rdt_drbg_init_u64(0x1111111111111111ULL, 0x2222222222222222ULL, 0x3333333333333333ULL);
    uint8_t dummy[1];
    int r = rdt_drbg_generate(dummy, 0, NULL, 0, 0);
    test("DRBG generate 0 bytes", r == 0);

    /* Test 2: Generate very large output (10KB) */
    uint8_t large_buf[10240];
    r = rdt_drbg_generate(large_buf, sizeof(large_buf), NULL, 0, 0);
    test("DRBG generate 10KB", r == 0);

    /* Test 3: Multiple consecutive generates */
    uint8_t buf1[32], buf2[32], buf3[32];
    r = rdt_drbg_generate(buf1, 32, NULL, 0, 0);
    r &= rdt_drbg_generate(buf2, 32, NULL, 0, 0);
    r &= rdt_drbg_generate(buf3, 32, NULL, 0, 0);

    /* Check they're all different */
    int all_different = (memcmp(buf1, buf2, 32) != 0) &&
                        (memcmp(buf2, buf3, 32) != 0) &&
                        (memcmp(buf1, buf3, 32) != 0);
    test("DRBG consecutive generates are different", r == 0 && all_different);

    /* Test 4: Reseed and verify output changes */
    uint8_t before[32], after[32];
    rdt_drbg_init_u64(0xAAAAAAAAAAAAAAAAULL, 0xBBBBBBBBBBBBBBBBULL, 0xCCCCCCCCCCCCCCCCULL);
    rdt_drbg_generate(before, 32, NULL, 0, 0);

    uint8_t ent[] = "new_entropy_material_12345678901234567890";
    rdt_drbg_reseed(ent, sizeof(ent)-1, NULL, 0);
    rdt_drbg_generate(after, 32, NULL, 0, 0);

    test("DRBG reseed changes output", memcmp(before, after, 32) != 0);

    /* Test 5: Additional input changes output */
    rdt_drbg_init_u64(0x5555555555555555ULL, 0x6666666666666666ULL, 0x7777777777777777ULL);
    uint8_t no_add[32], with_add[32];
    rdt_drbg_generate(no_add, 32, NULL, 0, 0);

    rdt_drbg_init_u64(0x5555555555555555ULL, 0x6666666666666666ULL, 0x7777777777777777ULL);
    uint8_t add_input[] = "additional";
    rdt_drbg_generate(with_add, 32, add_input, sizeof(add_input)-1, 0);

    test("DRBG additional input changes output", memcmp(no_add, with_add, 32) != 0);

    /* Test 6: Zeroize clears state */
    rdt_drbg_zeroize();
    rdt_drbg_init_u64(0x1234567890ABCDEFULL, 0xFEDCBA0987654321ULL, 0x1111111111111111ULL);
    uint64_t v1 = rdt_drbg_next_u64();
    rdt_drbg_zeroize();
    rdt_drbg_init_u64(0x1234567890ABCDEFULL, 0xFEDCBA0987654321ULL, 0x1111111111111111ULL);
    uint64_t v2 = rdt_drbg_next_u64();
    test("DRBG zeroize + reinit gives same output", v1 == v2);

    /* Test 7: Generate 1 byte (boundary) */
    uint8_t single;
    r = rdt_drbg_generate(&single, 1, NULL, 0, 0);
    test("DRBG generate 1 byte", r == 0);

    /* Test 8: Generate 7 bytes (non-aligned) */
    uint8_t seven[7];
    r = rdt_drbg_generate(seven, 7, NULL, 0, 0);
    test("DRBG generate 7 bytes (non-aligned)", r == 0);

    /* Test 9: NULL output with 0 length */
    r = rdt_drbg_generate(NULL, 0, NULL, 0, 0);
    test("DRBG NULL output with 0 length", r == 0);

    /* Test 10: Prediction resistance flag */
    r = rdt_drbg_generate(buf1, 32, NULL, 0, 1);
    test("DRBG prediction_resistance=1", r == 0);

    printf("\n");

    /* ========================================
       Seed Extractor Edge Cases
       ======================================== */
    printf("--- Seed Extractor Edge Cases ---\n");

    /* Test 11: Non-existent file returns error */
    uint8_t seed[32];
    r = rdt_seed_extract_file("nonexistent_file_12345.csv", seed);
    test("Seed extractor rejects non-existent file", r != 0);

    /* Test 12: NULL filepath returns error */
    r = rdt_seed_extract_file(NULL, seed);
    test("Seed extractor rejects NULL filepath", r != 0);

    /* Test 13: NULL output buffer returns error */
    r = rdt_seed_extract_file("examples/sensor_data.csv", NULL);
    test("Seed extractor rejects NULL output", r != 0);

    /* Test 14: NULL data with 0 length returns error */
    r = rdt_seed_extract(NULL, 0, seed);
    test("Seed extractor rejects NULL data", r != 0);

    /* Test 15: Valid data with 0 length returns error */
    uint8_t some_data[] = "test";
    r = rdt_seed_extract(some_data, 0, seed);
    test("Seed extractor rejects 0-length data", r != 0);

    /* Test 16: Minimal valid data (1 byte) */
    uint8_t minimal[] = "1";
    r = rdt_seed_extract(minimal, 1, seed);
    test("Seed extractor accepts 1 byte", r == 0);

    /* Test 17: Small CSV data */
    uint8_t csv[] = "1,2,3\n4,5,6\n";
    r = rdt_seed_extract(csv, sizeof(csv)-1, seed);
    test("Seed extractor processes small CSV", r == 0);

    /* Test 18: Same input produces same seed (deterministic) */
    uint8_t seed1[32], seed2[32];
    uint8_t input[] = "deterministic_test_123456789";
    rdt_seed_extract(input, sizeof(input)-1, seed1);
    rdt_seed_extract(input, sizeof(input)-1, seed2);
    test("Seed extractor is deterministic", memcmp(seed1, seed2, 32) == 0);

    /* Test 19: Different inputs produce different seeds */
    uint8_t input_a[] = "input_A";
    uint8_t input_b[] = "input_B";
    rdt_seed_extract(input_a, sizeof(input_a)-1, seed1);
    rdt_seed_extract(input_b, sizeof(input_b)-1, seed2);
    test("Seed extractor produces different seeds for different inputs", memcmp(seed1, seed2, 32) != 0);

    /* Test 20: Extract as u64 format */
    uint64_t seed_u64[4];
    r = rdt_seed_extract_u64(csv, sizeof(csv)-1, seed_u64);
    test("Seed extractor u64 format works", r == 0);

    printf("\n");

    /* ========================================
       Integration Edge Cases
       ======================================== */
    printf("--- Integration Edge Cases ---\n");

    /* Test 21: Extract seed and use with DRBG */
    uint8_t extracted[32];
    r = rdt_seed_extract_file("examples/sensor_data.csv", extracted);
    if (r == 0) {
        /* Use first 24 bytes as seed+nonce+personalization */
        uint64_t eseed, enonce, eperson;
        memcpy(&eseed, extracted, 8);
        memcpy(&enonce, extracted + 8, 8);
        memcpy(&eperson, extracted + 16, 8);

        rdt_drbg_init_u64(eseed, enonce, eperson);
        uint8_t out[64];
        r = rdt_drbg_generate(out, 64, NULL, 0, 0);
        test("Seed extractor → DRBG pipeline", r == 0);
    } else {
        test("Seed extractor → DRBG pipeline", 0);
    }

    /* Test 22: Multiple reseeds in sequence */
    rdt_drbg_init_u64(0x1111111111111111ULL, 0x2222222222222222ULL, 0x3333333333333333ULL);
    for (int i = 0; i < 10; i++) {
        uint8_t ent_data[32];
        memset(ent_data, i, sizeof(ent_data));
        rdt_drbg_reseed(ent_data, sizeof(ent_data), NULL, 0);
    }
    r = rdt_drbg_generate(buf1, 32, NULL, 0, 0);
    test("DRBG multiple reseeds", r == 0);

    /* Test 23: Interleaved generate and reseed */
    rdt_drbg_init_u64(0xAAAAAAAAAAAAAAAAULL, 0xBBBBBBBBBBBBBBBBULL, 0xCCCCCCCCCCCCCCCCULL);
    rdt_drbg_generate(buf1, 32, NULL, 0, 0);
    rdt_drbg_reseed((uint8_t*)"reseed1", 7, NULL, 0);
    rdt_drbg_generate(buf2, 32, NULL, 0, 0);
    rdt_drbg_reseed((uint8_t*)"reseed2", 7, NULL, 0);
    rdt_drbg_generate(buf3, 32, NULL, 0, 0);
    test("DRBG interleaved generate/reseed",
         memcmp(buf1, buf2, 32) != 0 && memcmp(buf2, buf3, 32) != 0);

    printf("\n");

    /* ========================================
       Summary
       ======================================== */
    printf("=== Test Summary ===\n");
    printf("Passed: %d\n", test_passed);
    printf("Failed: %d\n", test_failed);
    printf("Total:  %d\n", test_passed + test_failed);

    if (test_failed == 0) {
        printf("\n✅ All edge case tests passed!\n");
        return 0;
    } else {
        printf("\n⚠️  Some tests failed\n");
        return 1;
    }
}
