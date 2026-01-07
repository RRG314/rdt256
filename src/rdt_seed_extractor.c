/*
 * RDT Seed Extractor - C Implementation
 * =====================================
 * Extracts high-quality 256-bit seeds from sensor data files.
 *
 * Author: Steven Reid
 * ORCID: 0009-0003-9132-3410
 * License: MIT
 *
 * Pipeline:
 *   1. Extract numeric values with positional context
 *   2. Extract structural fingerprint
 *   3. Apply entropy precursor layer (block-wise flip + shift)
 *   4. Apply recursive entropy mixer (mixer_a + mixer_b)
 *   5. SHA-256 finalization with domain separation
 *
 * Compile:
 *   gcc -O3 -o rdt_seed_extractor rdt_seed_extractor.c -lm
 */

#include "rdt_seed_extractor.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* ========================================================================== */
/* SHA-256 Implementation (minimal, self-contained)                           */
/* ========================================================================== */

typedef struct {
    uint32_t state[8];
    uint64_t count;
    uint8_t buffer[64];
} sha256_ctx;

static const uint32_t sha256_k[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
    0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
    0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
    0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
    0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
    0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
    0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
    0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
    0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

#define ROR32(x, n) (((x) >> (n)) | ((x) << (32 - (n))))
#define CH(x, y, z) (((x) & (y)) ^ (~(x) & (z)))
#define MAJ(x, y, z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define EP0(x) (ROR32(x, 2) ^ ROR32(x, 13) ^ ROR32(x, 22))
#define EP1(x) (ROR32(x, 6) ^ ROR32(x, 11) ^ ROR32(x, 25))
#define SIG0(x) (ROR32(x, 7) ^ ROR32(x, 18) ^ ((x) >> 3))
#define SIG1(x) (ROR32(x, 17) ^ ROR32(x, 19) ^ ((x) >> 10))

static void sha256_transform(sha256_ctx *ctx, const uint8_t data[64]) {
    uint32_t a, b, c, d, e, f, g, h, t1, t2, m[64];
    int i;

    for (i = 0; i < 16; i++) {
        m[i] = ((uint32_t)data[i * 4] << 24) |
               ((uint32_t)data[i * 4 + 1] << 16) |
               ((uint32_t)data[i * 4 + 2] << 8) |
               ((uint32_t)data[i * 4 + 3]);
    }
    for (; i < 64; i++) {
        m[i] = SIG1(m[i - 2]) + m[i - 7] + SIG0(m[i - 15]) + m[i - 16];
    }

    a = ctx->state[0]; b = ctx->state[1];
    c = ctx->state[2]; d = ctx->state[3];
    e = ctx->state[4]; f = ctx->state[5];
    g = ctx->state[6]; h = ctx->state[7];

    for (i = 0; i < 64; i++) {
        t1 = h + EP1(e) + CH(e, f, g) + sha256_k[i] + m[i];
        t2 = EP0(a) + MAJ(a, b, c);
        h = g; g = f; f = e; e = d + t1;
        d = c; c = b; b = a; a = t1 + t2;
    }

    ctx->state[0] += a; ctx->state[1] += b;
    ctx->state[2] += c; ctx->state[3] += d;
    ctx->state[4] += e; ctx->state[5] += f;
    ctx->state[6] += g; ctx->state[7] += h;
}

static void sha256_init(sha256_ctx *ctx) {
    ctx->state[0] = 0x6a09e667;
    ctx->state[1] = 0xbb67ae85;
    ctx->state[2] = 0x3c6ef372;
    ctx->state[3] = 0xa54ff53a;
    ctx->state[4] = 0x510e527f;
    ctx->state[5] = 0x9b05688c;
    ctx->state[6] = 0x1f83d9ab;
    ctx->state[7] = 0x5be0cd19;
    ctx->count = 0;
}

static void sha256_update(sha256_ctx *ctx, const uint8_t *data, size_t len) {
    size_t i = 0;
    size_t idx = (size_t)(ctx->count & 63);
    ctx->count += len;

    if (idx) {
        size_t left = 64 - idx;
        if (len < left) {
            memcpy(ctx->buffer + idx, data, len);
            return;
        }
        memcpy(ctx->buffer + idx, data, left);
        sha256_transform(ctx, ctx->buffer);
        i = left;
    }

    for (; i + 64 <= len; i += 64) {
        sha256_transform(ctx, data + i);
    }

    if (i < len) {
        memcpy(ctx->buffer, data + i, len - i);
    }
}

static void sha256_final(sha256_ctx *ctx, uint8_t hash[32]) {
    size_t idx = (size_t)(ctx->count & 63);
    ctx->buffer[idx++] = 0x80;

    if (idx > 56) {
        memset(ctx->buffer + idx, 0, 64 - idx);
        sha256_transform(ctx, ctx->buffer);
        idx = 0;
    }
    memset(ctx->buffer + idx, 0, 56 - idx);

    uint64_t bits = ctx->count * 8;
    for (int i = 0; i < 8; i++) {
        ctx->buffer[63 - i] = (uint8_t)(bits >> (i * 8));
    }
    sha256_transform(ctx, ctx->buffer);

    for (int i = 0; i < 8; i++) {
        hash[i * 4] = (uint8_t)(ctx->state[i] >> 24);
        hash[i * 4 + 1] = (uint8_t)(ctx->state[i] >> 16);
        hash[i * 4 + 2] = (uint8_t)(ctx->state[i] >> 8);
        hash[i * 4 + 3] = (uint8_t)(ctx->state[i]);
    }
}

static void sha256(const uint8_t *data, size_t len, uint8_t hash[32]) {
    sha256_ctx ctx;
    sha256_init(&ctx);
    sha256_update(&ctx, data, len);
    sha256_final(&ctx, hash);
}

/* ========================================================================== */
/* Mixer Functions (Steven Reid's design)                                     */
/* ========================================================================== */

/*
 * mixer_a: Roll + Invert + XOR mixing
 * result[i] = roll(data,3)[i] XOR ~data[i] XOR data[len-1-i]
 */
static void mixer_a(uint8_t *data, size_t len) {
    if (len == 0) return;

    uint8_t *temp = (uint8_t *)malloc(len);
    if (!temp) return;

    for (size_t i = 0; i < len; i++) {
        size_t roll_idx = (i + len - 3) % len;  /* roll by 3 */
        uint8_t rolled = data[roll_idx];
        uint8_t flipped = ~data[i];
        uint8_t reversed = data[len - 1 - i];
        temp[i] = rolled ^ flipped ^ reversed;
    }

    memcpy(data, temp, len);
    free(temp);
}

/*
 * mixer_b: Half-swap + Reverse XOR mixing
 */
static void mixer_b(uint8_t *data, size_t len) {
    if (len < 2) return;

    /* Ensure even length */
    size_t work_len = len;
    uint8_t *work = data;
    uint8_t *allocated = NULL;

    if (len % 2 != 0) {
        work_len = len + 1;
        allocated = (uint8_t *)malloc(work_len);
        if (!allocated) return;
        memcpy(allocated, data, len);
        allocated[len] = 0;
        work = allocated;
    }

    uint8_t *temp = (uint8_t *)malloc(work_len);
    if (!temp) {
        free(allocated);
        return;
    }

    size_t half = work_len / 2;

    for (size_t i = 0; i < work_len; i++) {
        /* overlay: swap halves */
        uint8_t overlay;
        if (i < half) {
            overlay = work[half + i];
        } else {
            overlay = work[i - half];
        }

        /* reverse XOR overlay XOR original */
        uint8_t reversed = work[work_len - 1 - i];
        temp[i] = (reversed ^ overlay) ^ work[i];
    }

    if (allocated) {
        memcpy(data, temp, len);
        free(allocated);
    } else {
        memcpy(data, temp, len);
    }
    free(temp);
}

/*
 * entropy_precursor_layer: Block-wise flip + shift
 */
static void entropy_precursor_layer(uint8_t *data, size_t len, size_t block_size) {
    if (len == 0 || block_size == 0) return;

    uint8_t *block_temp = (uint8_t *)malloc(block_size);
    if (!block_temp) return;

    for (size_t i = 0; i < len; i += block_size) {
        size_t chunk = (i + block_size <= len) ? block_size : (len - i);

        /* Copy block, pad with zeros if needed */
        memset(block_temp, 0, block_size);
        memcpy(block_temp, data + i, chunk);

        /* Apply: flipped XOR shifted(5) */
        for (size_t j = 0; j < chunk; j++) {
            size_t shift_idx = (j + block_size - 5) % block_size;
            uint8_t flipped = ~block_temp[j];
            uint8_t shifted = (shift_idx < chunk) ? data[i + shift_idx] : 0;
            data[i + j] = flipped ^ shifted;
        }
    }

    free(block_temp);
}

/*
 * recursive_entropy_mixer: Divide-and-conquer mixing
 */
static void recursive_entropy_mixer_impl(uint8_t *data, size_t len, int depth) {
    /* Ensure even length */
    size_t work_len = len;
    if (len % 2 != 0) {
        work_len = len + 1;
    }

    if (depth == 0 || len < 64) {
        mixer_a(data, len);
        mixer_b(data, len);
        return;
    }

    size_t mid = len / 2;

    /* Recurse on halves */
    recursive_entropy_mixer_impl(data, mid, depth - 1);
    recursive_entropy_mixer_impl(data + mid, len - mid, depth - 1);

    /* Combine */
    mixer_b(data, len);
    mixer_a(data, len);
}

static void recursive_entropy_mixer(uint8_t *data, size_t len, int max_depth) {
    recursive_entropy_mixer_impl(data, len, max_depth);
}

/* ========================================================================== */
/* Numeric Extraction                                                         */
/* ========================================================================== */

typedef struct {
    uint8_t *data;
    size_t len;
    size_t capacity;
} byte_buffer;

static void buffer_init(byte_buffer *buf) {
    buf->capacity = 4096;
    buf->data = (uint8_t *)malloc(buf->capacity);
    buf->len = 0;
}

static void buffer_append(byte_buffer *buf, const void *data, size_t len) {
    if (!buf->data) return;

    while (buf->len + len > buf->capacity) {
        buf->capacity *= 2;
        buf->data = (uint8_t *)realloc(buf->data, buf->capacity);
        if (!buf->data) return;
    }

    memcpy(buf->data + buf->len, data, len);
    buf->len += len;
}

static void buffer_append_u32_le(byte_buffer *buf, uint32_t val) {
    uint8_t bytes[4] = {
        (uint8_t)(val),
        (uint8_t)(val >> 8),
        (uint8_t)(val >> 16),
        (uint8_t)(val >> 24)
    };
    buffer_append(buf, bytes, 4);
}

static void buffer_append_u64_le(byte_buffer *buf, uint64_t val) {
    uint8_t bytes[8];
    for (int i = 0; i < 8; i++) {
        bytes[i] = (uint8_t)(val >> (i * 8));
    }
    buffer_append(buf, bytes, 8);
}

static void buffer_append_double_le(byte_buffer *buf, double val) {
    uint64_t bits;
    memcpy(&bits, &val, sizeof(bits));
    buffer_append_u64_le(buf, bits);
}

static void buffer_free(byte_buffer *buf) {
    free(buf->data);
    buf->data = NULL;
    buf->len = 0;
}

/*
 * Extract numeric values with positions from data
 */
static void extract_numeric_with_positions(const uint8_t *data, size_t len, byte_buffer *out) {
    size_t i = 0;
    uint32_t line_num = 0;

    while (i < len) {
        /* Track line numbers */
        if (data[i] == '\n') {
            line_num++;
            i++;
            continue;
        }

        /* Look for start of number */
        if (isdigit(data[i]) ||
            ((data[i] == '-' || data[i] == '+' || data[i] == '.') &&
             i + 1 < len && (isdigit(data[i + 1]) || data[i + 1] == '.'))) {

            uint32_t pos = (uint32_t)i;

            /* Parse number */
            char num_buf[64];
            size_t num_len = 0;
            size_t j = i;

            /* Sign */
            if (j < len && (data[j] == '-' || data[j] == '+')) {
                num_buf[num_len++] = (char)data[j++];
            }

            /* Integer part */
            while (j < len && isdigit(data[j]) && num_len < 60) {
                num_buf[num_len++] = (char)data[j++];
            }

            /* Decimal part */
            if (j < len && data[j] == '.') {
                num_buf[num_len++] = (char)data[j++];
                while (j < len && isdigit(data[j]) && num_len < 60) {
                    num_buf[num_len++] = (char)data[j++];
                }
            }

            /* Exponent */
            if (j < len && (data[j] == 'e' || data[j] == 'E')) {
                num_buf[num_len++] = (char)data[j++];
                if (j < len && (data[j] == '-' || data[j] == '+')) {
                    num_buf[num_len++] = (char)data[j++];
                }
                while (j < len && isdigit(data[j]) && num_len < 60) {
                    num_buf[num_len++] = (char)data[j++];
                }
            }

            num_buf[num_len] = '\0';

            /* Convert to double */
            if (num_len > 0 && num_len < 60) {
                char *endptr;
                double val = strtod(num_buf, &endptr);

                /* Check for valid finite number */
                if (endptr != num_buf && val == val && val != 1.0/0.0 && val != -1.0/0.0) {
                    /* Pack: position (4B) + line (4B) + float64 (8B) */
                    buffer_append_u32_le(out, pos);
                    buffer_append_u32_le(out, line_num);
                    buffer_append_double_le(out, val);
                }
            }

            i = j;
        } else {
            i++;
        }
    }
}

/*
 * Extract structural fingerprint
 */
static void extract_structure_fingerprint(const uint8_t *data, size_t len, byte_buffer *out) {
    /* File length */
    buffer_append_u64_le(out, (uint64_t)len);

    /* Count delimiters */
    uint32_t newlines = 0, commas = 0, tabs = 0, spaces = 0, semicolons = 0;
    for (size_t i = 0; i < len; i++) {
        switch (data[i]) {
            case '\n': newlines++; break;
            case ',': commas++; break;
            case '\t': tabs++; break;
            case ' ': spaces++; break;
            case ';': semicolons++; break;
        }
    }

    buffer_append_u32_le(out, newlines);
    buffer_append_u32_le(out, commas);
    buffer_append_u32_le(out, tabs);
    buffer_append_u32_le(out, spaces);
    buffer_append_u32_le(out, semicolons);

    /* Sample raw bytes */
    size_t interval = (len > 128) ? (len / 128) : 1;
    for (size_t i = 0; i < len; i += interval) {
        buffer_append_u32_le(out, (uint32_t)i);
        size_t chunk = (i + 4 <= len) ? 4 : (len - i);
        buffer_append(out, data + i, chunk);
        if (chunk < 4) {
            uint8_t pad[4] = {0};
            buffer_append(out, pad, 4 - chunk);
        }
    }
}

/* ========================================================================== */
/* Main Extraction Functions                                                  */
/* ========================================================================== */

int rdt_seed_extract(const uint8_t *data, size_t data_len, uint8_t seed_out[32]) {
    if (!data || !seed_out) return -1;

    byte_buffer pool;
    buffer_init(&pool);

    if (!pool.data) return -1;

    /* Build entropy pool */

    /* Numeric extraction with positions */
    buffer_append(&pool, "NUMERIC:", 8);
    byte_buffer numeric;
    buffer_init(&numeric);
    extract_numeric_with_positions(data, data_len, &numeric);
    buffer_append_u32_le(&pool, (uint32_t)numeric.len);
    if (numeric.len > 0) {
        buffer_append(&pool, numeric.data, numeric.len);
    }
    buffer_free(&numeric);

    /* Structure fingerprint */
    buffer_append(&pool, "STRUCTURE:", 10);
    extract_structure_fingerprint(data, data_len, &pool);

    /* Raw data hash */
    buffer_append(&pool, "RAWHASH:", 8);
    uint8_t raw_hash[32];
    sha256(data, data_len, raw_hash);
    buffer_append(&pool, raw_hash, 32);

    /* Apply mixing */
    entropy_precursor_layer(pool.data, pool.len, 256);
    recursive_entropy_mixer(pool.data, pool.len, 4);

    /* Final SHA-256 with domain separation */
    sha256_ctx ctx;
    sha256_init(&ctx);
    sha256_update(&ctx, (const uint8_t *)"RDT256-SEEDGEN-v1\x00", 18);
    sha256_update(&ctx, (const uint8_t *)"StevenReid:RDT256\x00", 18);
    sha256_update(&ctx, pool.data, pool.len);
    sha256_final(&ctx, seed_out);

    buffer_free(&pool);
    return 0;
}

int rdt_seed_extract_u64(const uint8_t *data, size_t data_len, uint64_t seed_out[4]) {
    uint8_t seed[32];
    int result = rdt_seed_extract(data, data_len, seed);
    if (result != 0) return result;

    /* Little-endian conversion */
    for (int i = 0; i < 4; i++) {
        seed_out[i] = 0;
        for (int j = 0; j < 8; j++) {
            seed_out[i] |= ((uint64_t)seed[i * 8 + j]) << (j * 8);
        }
    }

    return 0;
}

int rdt_seed_extract_file(const char *filepath, uint8_t seed_out[32]) {
    FILE *f = fopen(filepath, "rb");
    if (!f) return -1;

    /* Get file size */
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (size <= 0) {
        fclose(f);
        return -1;
    }

    uint8_t *data = (uint8_t *)malloc((size_t)size);
    if (!data) {
        fclose(f);
        return -1;
    }

    size_t read = fread(data, 1, (size_t)size, f);
    fclose(f);

    if (read != (size_t)size) {
        free(data);
        return -1;
    }

    int result = rdt_seed_extract(data, read, seed_out);
    free(data);
    return result;
}

int rdt_seed_extract_files(const char **filepaths, size_t num_files, uint8_t seed_out[32]) {
    byte_buffer combined;
    buffer_init(&combined);

    if (!combined.data) return -1;

    for (size_t i = 0; i < num_files; i++) {
        const char *path = filepaths[i];

        /* Add file marker */
        buffer_append(&combined, "FILE:", 5);
        buffer_append(&combined, path, strlen(path));
        buffer_append(&combined, "\x00", 1);

        /* Read file */
        FILE *f = fopen(path, "rb");
        if (!f) {
            buffer_free(&combined);
            return -1;
        }

        fseek(f, 0, SEEK_END);
        long size = ftell(f);
        fseek(f, 0, SEEK_SET);

        if (size > 0) {
            uint8_t *data = (uint8_t *)malloc((size_t)size);
            if (data) {
                size_t bytes_read = fread(data, 1, (size_t)size, f);
                buffer_append(&combined, data, bytes_read);
                free(data);
            }
        }

        fclose(f);
        buffer_append(&combined, "\x1E", 1);  /* Record separator */
    }

    int result = rdt_seed_extract(combined.data, combined.len, seed_out);
    buffer_free(&combined);
    return result;
}

/* ========================================================================== */
/* CLI Tool                                                                   */
/* ========================================================================== */

#ifdef RDT_SEED_EXTRACTOR_MAIN

static void print_hex(const uint8_t *data, size_t len) {
    for (size_t i = 0; i < len; i++) {
        printf("%02x", data[i]);
    }
}

static void print_usage(const char *prog) {
    fprintf(stderr, "RDT Seed Extractor\n");
    fprintf(stderr, "Author: Steven Reid (ORCID: 0009-0003-9132-3410)\n\n");
    fprintf(stderr, "Usage: %s [options] <file1> [file2] ...\n\n", prog);
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  -h, --help      Show this help\n");
    fprintf(stderr, "  -c              Output in C format\n");
    fprintf(stderr, "  -u              Output as 4 x uint64_t\n");
    fprintf(stderr, "  -b              Output raw bytes (binary)\n");
}

int main(int argc, char **argv) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    int format = 0;  /* 0=hex, 1=C, 2=u64, 3=binary */
    int file_start = 1;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        } else if (strcmp(argv[i], "-c") == 0) {
            format = 1;
            file_start = i + 1;
        } else if (strcmp(argv[i], "-u") == 0) {
            format = 2;
            file_start = i + 1;
        } else if (strcmp(argv[i], "-b") == 0) {
            format = 3;
            file_start = i + 1;
        } else {
            break;
        }
    }

    if (file_start >= argc) {
        fprintf(stderr, "Error: No input files specified\n");
        return 1;
    }

    int num_files = argc - file_start;
    const char **files = (const char **)(argv + file_start);

    uint8_t seed[32];
    int result;

    if (num_files == 1) {
        result = rdt_seed_extract_file(files[0], seed);
    } else {
        result = rdt_seed_extract_files(files, (size_t)num_files, seed);
    }

    if (result != 0) {
        fprintf(stderr, "Error: Failed to extract seed\n");
        return 1;
    }

    switch (format) {
        case 0:  /* Hex */
            print_hex(seed, 32);
            printf("\n");
            break;

        case 1:  /* C format */
            printf("uint64_t seed[4] = {\n");
            for (int i = 0; i < 4; i++) {
                uint64_t val = 0;
                for (int j = 0; j < 8; j++) {
                    val |= ((uint64_t)seed[i * 8 + j]) << (j * 8);
                }
                printf("    0x%016llxULL%s\n",
                       (unsigned long long)val,
                       (i < 3) ? "," : "");
            }
            printf("};\n");
            break;

        case 2:  /* uint64 */
            for (int i = 0; i < 4; i++) {
                uint64_t val = 0;
                for (int j = 0; j < 8; j++) {
                    val |= ((uint64_t)seed[i * 8 + j]) << (j * 8);
                }
                printf("[%d] 0x%016llx\n", i, (unsigned long long)val);
            }
            break;

        case 3:  /* Binary */
            fwrite(seed, 1, 32, stdout);
            break;
    }

    return 0;
}

#endif /* RDT_SEED_EXTRACTOR_MAIN */
