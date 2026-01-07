/*
 * RDT256 Integration Example
 * ==========================
 * Demonstrates using the RDT Seed Extractor with RDT-PRNG_STREAM_v2
 *
 * This example shows how to:
 * 1. Extract a high-quality seed from sensor data
 * 2. Initialize RDT-PRNG_STREAM_v2 with the extracted seed
 * 3. Generate random output
 *
 * Compile:
 *   gcc -O3 -I../src -o integration_example integration_example.c \
 *       ../src/rdt_seed_extractor.c ../src/rdt256_stream_v2.c
 *
 * Usage:
 *   ./integration_example sensor_data.csv
 */

#include "rdt_seed_extractor.h"
#include "rdt256_stream_v2.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <sensor_file.csv> [additional_files...]\n", argv[0]);
        fprintf(stderr, "\nExample:\n");
        fprintf(stderr, "  %s sensor_data.csv\n", argv[0]);
        fprintf(stderr, "  %s sensor1.csv sensor2.csv sensor3.csv\n", argv[0]);
        return 1;
    }

    printf("RDT256 Integration Example\n");
    printf("==========================\n\n");

    /* Extract seed from input files */
    uint64_t seed[4];
    int result;

    if (argc == 2) {
        printf("Extracting seed from: %s\n", argv[1]);
        result = rdt_seed_extract_file(argv[1], (uint8_t *)seed);
    } else {
        printf("Extracting seed from %d files:\n", argc - 1);
        for (int i = 1; i < argc; i++) {
            printf("  - %s\n", argv[i]);
        }
        const char **files = (const char **)(argv + 1);
        result = rdt_seed_extract_files(files, (size_t)(argc - 1), (uint8_t *)seed);
    }

    if (result != 0) {
        fprintf(stderr, "Error: Failed to extract seed from files\n");
        return 1;
    }

    /* Display extracted seed */
    printf("\nExtracted 256-bit seed:\n");
    printf("  [0] 0x%016llx\n", (unsigned long long)seed[0]);
    printf("  [1] 0x%016llx\n", (unsigned long long)seed[1]);
    printf("  [2] 0x%016llx\n", (unsigned long long)seed[2]);
    printf("  [3] 0x%016llx\n", (unsigned long long)seed[3]);

    /* Initialize RDT-PRNG_STREAM_v2 with extracted seed */
    printf("\nInitializing RDT-PRNG_STREAM_v2...\n");
    rdt_prng_v2_init(seed);

    /* Generate some random output */
    printf("\nGenerating random output:\n");
    printf("First 10 values:\n");
    for (int i = 0; i < 10; i++) {
        uint64_t val = rdt_prng_v2_next();
        printf("  [%d] 0x%016llx\n", i, (unsigned long long)val);
    }

    /* Generate a buffer of random bytes */
    printf("\nGenerating 1024 random bytes...\n");
    uint8_t buffer[1024];
    rdt_prng_v2_fill(buffer, sizeof(buffer));

    /* Show byte distribution */
    int counts[256] = {0};
    for (size_t i = 0; i < sizeof(buffer); i++) {
        counts[buffer[i]]++;
    }

    int unique_bytes = 0;
    for (int i = 0; i < 256; i++) {
        if (counts[i] > 0) unique_bytes++;
    }

    printf("Byte distribution in 1024-byte sample:\n");
    printf("  Unique byte values: %d/256\n", unique_bytes);
    printf("  Min count: ");
    int min_count = 1024;
    for (int i = 0; i < 256; i++) {
        if (counts[i] > 0 && counts[i] < min_count) min_count = counts[i];
    }
    printf("%d\n", min_count);

    printf("  Max count: ");
    int max_count = 0;
    for (int i = 0; i < 256; i++) {
        if (counts[i] > max_count) max_count = counts[i];
    }
    printf("%d\n", max_count);
    printf("  Average: %.2f (expected: 4.0)\n", 1024.0 / 256.0);

    printf("\nIntegration test successful!\n");
    printf("\nYou can now pipe the output to statistical test suites:\n");
    printf("  ./rdt_prng_stream_v2 <seed> | dieharder -a -g 200\n");
    printf("  ./rdt_prng_stream_v2 <seed> | smokerand default stdin64\n");

    return 0;
}
