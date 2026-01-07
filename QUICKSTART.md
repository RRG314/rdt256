# RDT256 Quick Start Guide

## Build Everything

```bash
make clean
make all
```

This builds:
- `rdt_prng_stream` - Original PRNG streaming generator
- `rdt_prng_stream_v2` - Enhanced PRNG with cross-state diffusion
- `rdt_seed_extractor` - High-quality seed extraction from sensor data

## Using the Seed Extractor

### Standalone Usage

Extract a high-quality 256-bit seed from any data file:

```bash
# From sensor data (CSV, text, etc.)
./rdt_seed_extractor examples/sensor_data.csv

# Output: f2cb746be11d12125cacd7d2b3e09a9dd7f9775dde8e12edfdc0391a920b1e8e
```

### Different Output Formats

```bash
# Hex (default)
./rdt_seed_extractor examples/sensor_data.csv

# C array format
./rdt_seed_extractor -c examples/sensor_data.csv

# 4 × uint64 format
./rdt_seed_extractor -u examples/sensor_data.csv

# Binary (32 bytes)
./rdt_seed_extractor -b examples/sensor_data.csv > seed.bin
```

### Multiple Files

Combine entropy from multiple sources:

```bash
./rdt_seed_extractor sensor1.csv sensor2.csv sensor3.csv
```

## Using with RDT-PRNG_STREAM_v2

### Method 1: Extract seed, then use manually

```bash
# Extract seed in C format
./rdt_seed_extractor -c examples/sensor_data.csv

# Copy output and use directly:
./rdt_prng_stream_v2 0x12121de16b74cbf2 0x9d9ae0b3d2d7ac5c \
                      0xed128ede5d77f9d7 0x8e1e0b921a39c0fd
```

### Method 2: Python Integration

```python
#!/usr/bin/env python3
import sys
sys.path.insert(0, 'examples')

from rdt_seed_extractor import extract_seed_integers

# Extract from sensor data
s0, s1, s2, s3 = extract_seed_integers(['examples/sensor_data.csv'])

print(f"Seed extracted:")
print(f"  0x{s0:016x}")
print(f"  0x{s1:016x}")
print(f"  0x{s2:016x}")
print(f"  0x{s3:016x}")
```

### Method 3: C Integration

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

## Using with Any PRNG

The seed extractor works with **any PRNG** that accepts 256-bit seeds:

### With Python's random module

```python
from rdt_seed_extractor import extract_seed_bytes
import random

seed = extract_seed_bytes(['sensor_data.csv'])
random.seed(int.from_bytes(seed, 'little'))
```

### With NumPy

```python
import numpy as np
from rdt_seed_extractor import extract_seed_integers

s0, s1, s2, s3 = extract_seed_integers(['sensor_data.csv'])
seed_array = np.array([s0, s1, s2, s3], dtype=np.uint64)
rng = np.random.Generator(np.random.PCG64(seed_array))
```

### With Other C/C++ PRNGs

```c
// Extract 32-byte seed
uint8_t seed[32];
rdt_seed_extract_file("sensor_data.csv", seed);

// Use with ChaCha20, AES-CTR, or any other PRNG
your_prng_init(seed, 32);
```

## Testing PRNG Output

### Generate Random Data

```bash
# Generate 1 MB of random data
./rdt_prng_stream_v2 0x12121de16b74cbf2 0x9d9ae0b3d2d7ac5c \
                      0xed128ede5d77f9d7 0x8e1e0b921a39c0fd \
    | head -c 1000000 > random.bin
```

### Run Statistical Tests

```bash
# Dieharder test suite
./rdt_prng_stream_v2 <seed_values> | dieharder -a -g 200

# SmokeRand test suite
./rdt_prng_stream_v2 <seed_values> | smokerand default stdin64

# Basic entropy test
./rdt_prng_stream_v2 <seed_values> | head -c 10000000 | ent
```

## What Makes the Seed Extractor Special?

### Entropy Improvement

| Metric | Raw Sensor Data | After Extraction | Improvement |
|--------|----------------|------------------|-------------|
| Min-Entropy | 2.80 bits/byte | **7.82 bits/byte** | **+5.02 bits** |
| Shannon Entropy | 3.69 bits/byte | 7.999 bits/byte | +4.31 bits |
| Unique Bytes | 49/256 | 256/256 | Full distribution |

### Key Features

1. **High Entropy Extraction**: Converts low-entropy sensor data into near-perfect randomness
2. **Avalanche Effect**: 49.1% bit flips (ideal is 50%)
3. **Deterministic**: Same input always produces same seed (reproducible)
4. **Universal**: Works with any PRNG, not just RDT256
5. **Multi-Source**: Combine multiple files for higher entropy

### Pipeline

```
Sensor Data → Numeric Extraction → Structure Fingerprint →
Entropy Precursor → Recursive Mixer → SHA-256 → 256-bit Seed
```

## Directory Structure

```
rdt256/
├── src/
│   ├── rdt_seed_extractor.c      # Seed extractor implementation
│   ├── rdt_seed_extractor.h      # Seed extractor API
│   ├── rdt256_stream_v2.c        # Enhanced PRNG
│   └── rdt256_stream_v2.h        # PRNG API
├── examples/
│   ├── rdt_seed_extractor.py     # Python wrapper
│   ├── integration_example.c     # C integration example
│   ├── sensor_data.csv           # Example sensor data
│   └── README.md                 # Examples documentation
├── docs/
│   └── seed_extractor.md         # Full validation results
├── Makefile                       # Build system
└── README.md                      # Main documentation
```

## Next Steps

1. **Read the validation results**: `docs/seed_extractor.md`
2. **Try the examples**: `examples/README.md`
3. **Integrate with your code**: See `examples/integration_example.c`
4. **Run statistical tests**: Use Dieharder or SmokeRand

## Important Disclaimers

⚠️ **This is experimental research code**

- Not for production cryptographic use
- No formal security guarantees
- Not a standardized primitive
- Use for research, testing, and experimentation only

For production systems, use:
- Standardized entropy sources (e.g., /dev/urandom)
- NIST-approved extractors (SP 800-90B)
- Certified cryptographic PRNGs (AES-CTR-DRBG, etc.)

## Support

- **Documentation**: See `README.md` and `docs/`
- **Examples**: See `examples/`
- **Issues**: GitHub Issues
- **Author**: Steven Reid (ORCID: 0009-0003-9132-3410)

## License

MIT License - See `LICENSE`
