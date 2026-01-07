/*
 * RDT Seed Extractor
 * ==================
 * Extracts high-quality 256-bit seeds from sensor data files.
 *
 * Author: Steven Reid
 * ORCID: 0009-0003-9132-3410
 * License: MIT
 *
 * Validated Performance:
 *   - Input min-entropy:  2.80 bits/byte (CSV sensor data)
 *   - Output min-entropy: 7.82 bits/byte
 *   - Avalanche effect:   49.1%
 *   - Uniqueness:         100%
 */

#ifndef RDT_SEED_EXTRACTOR_H
#define RDT_SEED_EXTRACTOR_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Extract a 256-bit seed from raw data buffer.
 *
 * Parameters:
 *   data      - Input data buffer
 *   data_len  - Length of input data
 *   seed_out  - Output buffer (must be at least 32 bytes)
 *
 * Returns:
 *   0 on success, -1 on error
 */
int rdt_seed_extract(const uint8_t *data, size_t data_len, uint8_t seed_out[32]);

/*
 * Extract a 256-bit seed from raw data as 4 x 64-bit integers.
 *
 * Parameters:
 *   data      - Input data buffer
 *   data_len  - Length of input data
 *   seed_out  - Output array of 4 x uint64_t (little-endian)
 *
 * Returns:
 *   0 on success, -1 on error
 */
int rdt_seed_extract_u64(const uint8_t *data, size_t data_len, uint64_t seed_out[4]);

/*
 * Extract a 256-bit seed from a file.
 *
 * Parameters:
 *   filepath  - Path to input file
 *   seed_out  - Output buffer (must be at least 32 bytes)
 *
 * Returns:
 *   0 on success, -1 on error
 */
int rdt_seed_extract_file(const char *filepath, uint8_t seed_out[32]);

/*
 * Extract a 256-bit seed from multiple files.
 *
 * Parameters:
 *   filepaths - Array of file paths
 *   num_files - Number of files
 *   seed_out  - Output buffer (must be at least 32 bytes)
 *
 * Returns:
 *   0 on success, -1 on error
 */
int rdt_seed_extract_files(const char **filepaths, size_t num_files, uint8_t seed_out[32]);

#ifdef __cplusplus
}
#endif

#endif /* RDT_SEED_EXTRACTOR_H */
