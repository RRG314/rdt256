# RDT Seed Extractor - Validation Results

**Author:** Steven Reid
**ORCID:** 0009-0003-9132-3410
**Date:** January 7, 2026

---

## Summary

The RDT Seed Extractor transforms low-entropy sensor data into high-quality cryptographic seeds.

| Metric | Input (CSV) | Output (Seed) |
|--------|-------------|---------------|
| **Min-Entropy** | 2.80 bits/byte | **7.82 bits/byte** |
| Shannon Entropy | 3.69 bits/byte | 7.999 bits/byte |
| Unique Bytes | 49/256 | 256/256 |

**Entropy Gain: +5.02 bits/byte**

---

## Entropy Analysis

### Input (77,735 bytes CSV sensor data)

```
Shannon Entropy (H1):     3.688 bits/byte
Min-Entropy (H∞):         2.801 bits/byte
Collision Entropy (H2):   3.546 bits/byte
Unique byte values:       49/256
```

### After Mixing (142,848 bytes)

```
Shannon Entropy (H1):     7.999 bits/byte
Min-Entropy (H∞):         7.816 bits/byte
Collision Entropy (H2):   7.997 bits/byte
Unique byte values:       256/256
Byte count range:         [491, 634] (expected: 558)
```

---

## Statistical Tests

| Test | Result | Value | Threshold |
|------|--------|-------|-----------|
| Chi-Square Uniformity | ✅ PASS | χ² = 254.56 | < 310.46 |
| Runs Test | ✅ PASS | p = 0.059 | > 0.01 |
| Longest Run | ✅ PASS | 18 bits | < 40 |
| Serial Correlation | ✅ PASS | \|r\| < 0.008 | < 0.05 |

---

## Avalanche Analysis

Single bit flip in input → seed change:

| Position | Bits Changed | Percentage |
|----------|--------------|------------|
| 0 | 120/256 | 46.9% |
| 50 | 129/256 | 50.4% |
| 100 | 120/256 | 46.9% |
| 7733 | 132/256 | 51.6% |
| 15466 | 120/256 | 46.9% |
| 23199 | 117/256 | 45.7% |
| 30932 | 141/256 | 55.1% |
| **Average** | **125.6/256** | **49.1%** |

**Ideal: 50% — Result: Excellent**

---

## Validation Test Results

| Test | Result |
|------|--------|
| Seed Bit Balance | ✅ PASS (46.88% ones) |
| Input Avalanche | ✅ PASS (49.1%) |
| Uniqueness | ✅ PASS (21/21 unique) |
| Multi-Seed Entropy | ✅ PASS (7.73 bits/byte) |

---

## NIST SP 800-90B Estimates

| Estimator | Value |
|-----------|-------|
| Most Common Value | 7.82 bits/byte |
| Collision Estimate | 4.04 bits/byte |
| Markov Estimate | 5.77 bits/byte |

---

## Pipeline Architecture

The RDT Seed Extractor uses a multi-stage pipeline to extract high-quality entropy:

```
CSV Sensor Data
      │
      ▼
┌─────────────────────────────┐
│ 1. Numeric Extraction       │
│    • Parse floats           │
│    • Record positions       │
│    • Record line numbers    │
└─────────────────────────────┘
      │
      ▼
┌─────────────────────────────┐
│ 2. Structure Fingerprint    │
│    • File length            │
│    • Delimiter counts       │
│    • Sampled bytes          │
└─────────────────────────────┘
      │
      ▼
┌─────────────────────────────┐
│ 3. Entropy Precursor Layer  │
│    • Block-wise flip+shift  │
└─────────────────────────────┘
      │
      ▼
┌─────────────────────────────┐
│ 4. Recursive Entropy Mixer  │
│    • mixer_a + mixer_b      │
│    • Divide-and-conquer     │
└─────────────────────────────┘
      │
      ▼
┌─────────────────────────────┐
│ 5. SHA-256 Finalization     │
│    • Domain separation      │
└─────────────────────────────┘
      │
      ▼
   32-byte Seed → RDT256
```

---

## Test Configuration

- **Input:** 3 CSV sensor files (77,735 bytes total)
- **Content:** Timestamped temperature, humidity, pressure
- **Mixing Depth:** 4 levels
- **Block Size:** 256 bytes

---

## Example Seed

```
Input files: sensor1.csv, sensor2.csv, sensor3.csv

Hex: 683d5f2b0f90a422d3910ee7630033ac60b1a588a78f24e9652bae5f39059f42

C format:
uint64_t seed[4] = {
    0x22a4900f2b5f3d68ULL,
    0xac330063e70e91d3ULL,
    0xe9248fa788a5b160ULL,
    0x429f05395fae2b65ULL
};
```

---

## Integration with RDT256

The seed extractor is designed to work seamlessly with any PRNG:

### With RDT-PRNG_STREAM_v2

```c
#include "rdt_seed_extractor.h"
#include "rdt256_stream_v2.h"

// Extract seed from sensor data
uint64_t seed[4];
rdt_seed_extract_file("sensor_data.csv", (uint8_t *)seed);

// Initialize PRNG
rdt_prng_v2_init(seed);

// Generate random output
uint64_t random_value = rdt_prng_v2_next();
```

### With Other PRNGs

```c
// Extract 32-byte seed
uint8_t seed[32];
rdt_seed_extract_file("sensor_data.csv", seed);

// Use with any PRNG that accepts 256-bit seeds
// (ChaCha20, AES-CTR, etc.)
your_prng_init(seed, 32);
```

---

## Disclaimer

This seed extractor is experimental research code. While validation results show excellent statistical properties:

- It has not undergone formal cryptanalysis
- It has no proven security guarantees
- It is not a standardized primitive
- **Do NOT use for real-world cryptographic applications**

The extractor is intended for:
- Research and experimentation
- Benchmarking PRNG initialization
- Academic exploration of entropy extraction
- Non-cryptographic randomness generation

For production cryptographic use, employ standardized entropy extraction methods (e.g., NIST SP 800-90B approved extractors).
