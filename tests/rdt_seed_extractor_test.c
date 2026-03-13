#include "rdt_seed_extractor.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
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

static int read_file(const char *path, uint8_t **data, size_t *len) {
    FILE *f;
    long size;
    uint8_t *buf;

    if (!path || !data || !len) {
        return -1;
    }

    f = fopen(path, "rb");
    if (!f) {
        return -1;
    }
    if (fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        return -1;
    }
    size = ftell(f);
    if (size <= 0 || fseek(f, 0, SEEK_SET) != 0) {
        fclose(f);
        return -1;
    }

    buf = (uint8_t *)malloc((size_t)size);
    if (!buf) {
        fclose(f);
        return -1;
    }
    if (fread(buf, 1, (size_t)size, f) != (size_t)size) {
        free(buf);
        fclose(f);
        return -1;
    }
    fclose(f);

    *data = buf;
    *len = (size_t)size;
    return 0;
}

int main(void) {
    static const uint64_t expected_words[4] = {
        0x12121de16b74cbf2ULL,
        0x9d9ae0b3d2d7ac5cULL,
        0xed128ede5d77f9d7ULL,
        0x8e1e0b921a39c0fdULL
    };
    const char *duplicate_files[2] = {
        "examples/sensor_data.csv",
        "examples/sensor_data.csv"
    };
    uint8_t expected_single[32];
    uint8_t expected_duplicate[32];
    uint8_t seed_api[32];
    uint8_t seed_file[32];
    uint8_t seed_dup_a[32];
    uint8_t seed_dup_b[32];
    uint64_t words[4];
    uint8_t *data = NULL;
    size_t len = 0;
    int i;

    if (!hex_to_bytes("f2cb746be11d12125cacd7d2b3e09a9dd7f9775dde8e12edfdc0391a920b1e8e",
                      expected_single, sizeof(expected_single))) {
        fprintf(stderr, "failed to parse expected single-file seed\n");
        return 1;
    }
    if (!hex_to_bytes("654a9d34e91286afe2203ee6eddfd108021bd966046f737bea5b6f5b94af60a1",
                      expected_duplicate, sizeof(expected_duplicate))) {
        fprintf(stderr, "failed to parse expected duplicate-file seed\n");
        return 1;
    }

    if (read_file("examples/sensor_data.csv", &data, &len) != 0) {
        fprintf(stderr, "failed to read examples/sensor_data.csv\n");
        return 1;
    }

    if (rdt_seed_extract(data, len, seed_api) != 0) {
        fprintf(stderr, "rdt_seed_extract failed\n");
        free(data);
        return 1;
    }
    if (memcmp(seed_api, expected_single, sizeof(seed_api)) != 0) {
        fprintf(stderr, "single-file API seed mismatch\n");
        free(data);
        return 1;
    }

    if (rdt_seed_extract_file("examples/sensor_data.csv", seed_file) != 0) {
        fprintf(stderr, "rdt_seed_extract_file failed\n");
        free(data);
        return 1;
    }
    if (memcmp(seed_file, expected_single, sizeof(seed_file)) != 0) {
        fprintf(stderr, "single-file file-path seed mismatch\n");
        free(data);
        return 1;
    }

    if (rdt_seed_extract_u64(data, len, words) != 0) {
        fprintf(stderr, "rdt_seed_extract_u64 failed\n");
        free(data);
        return 1;
    }
    for (i = 0; i < 4; i++) {
        if (words[i] != expected_words[i]) {
            fprintf(stderr, "u64 seed mismatch at lane %d\n", i);
            free(data);
            return 1;
        }
    }

    if (rdt_seed_extract_files(duplicate_files, 2u, seed_dup_a) != 0 ||
        rdt_seed_extract_files(duplicate_files, 2u, seed_dup_b) != 0) {
        fprintf(stderr, "rdt_seed_extract_files failed\n");
        free(data);
        return 1;
    }
    if (memcmp(seed_dup_a, expected_duplicate, sizeof(seed_dup_a)) != 0) {
        fprintf(stderr, "duplicate-file seed mismatch\n");
        free(data);
        return 1;
    }
    if (memcmp(seed_dup_a, seed_dup_b, sizeof(seed_dup_a)) != 0) {
        fprintf(stderr, "duplicate-file extraction is not deterministic\n");
        free(data);
        return 1;
    }

    if (rdt_seed_extract(NULL, len, seed_api) == 0 ||
        rdt_seed_extract(data, 0u, seed_api) == 0 ||
        rdt_seed_extract_file("examples/does_not_exist.csv", seed_api) == 0) {
        fprintf(stderr, "error-path validation failed\n");
        free(data);
        return 1;
    }

    free(data);
    puts("rdt_seed_extractor_test: ok");
    return 0;
}
