# RDT256 Examples

This directory contains examples demonstrating the RDT Seed Extractor and its integration with RDT-PRNG_STREAM_v2.

## Files

### Seed Extractor Tools

- **`rdt_seed_extractor.py`** - Python wrapper for the RDT Seed Extractor
- **`sensor_data.csv`** - Example sensor data file (temperature, humidity, pressure)

### Integration Examples

- **`integration_example.c`** - C example showing seed extraction → PRNG initialization

## Quick Start

### 1. Build the Tools

From the repository root:

```bash
make
```

This builds:
- `rdt_seed_extractor` - Standalone seed extraction tool
- `rdt_prng_stream_v2` - Enhanced PRNG with v2 mixing
- `rdt_prng_stream` - Original PRNG stream

### 2. Extract a Seed (Command Line)

```bash
# Basic hex output
./rdt_seed_extractor examples/sensor_data.csv

# C array format (for copy-paste into code)
./rdt_seed_extractor -c examples/sensor_data.csv

# 4 × uint64 format
./rdt_seed_extractor -u examples/sensor_data.csv

# Binary output (pipe to file)
./rdt_seed_extractor -b examples/sensor_data.csv > seed.bin
```

### 3. Extract a Seed (Python)

```python
#!/usr/bin/env python3
import sys
sys.path.insert(0, 'examples')

from rdt_seed_extractor import extract_seed_integers

# Extract from one file
s0, s1, s2, s3 = extract_seed_integers(['examples/sensor_data.csv'])

print(f"Seed[0]: 0x{s0:016x}")
print(f"Seed[1]: 0x{s1:016x}")
print(f"Seed[2]: 0x{s2:016x}")
print(f"Seed[3]: 0x{s3:016x}")
```

Or use the command-line interface:

```bash
python3 examples/rdt_seed_extractor.py examples/sensor_data.csv
```

### 4. Use Extracted Seed with PRNG

#### Direct Command Line

```bash
# Extract seed and pass to PRNG
SEED=$(./rdt_seed_extractor -u examples/sensor_data.csv | awk '{print $2}')
echo "Using seed: $SEED"
./rdt_prng_stream_v2 $SEED | head -c 1000000 > random.bin
```

#### Integration Example (C)

Build and run the integration example:

```bash
cd examples
gcc -O3 -I../src -o integration_example integration_example.c \
    ../src/rdt_seed_extractor.c ../src/rdt256_stream_v2.c

./integration_example sensor_data.csv
```

This will:
1. Extract a 256-bit seed from the sensor data
2. Initialize RDT-PRNG_STREAM_v2 with the extracted seed
3. Generate sample random output
4. Display byte distribution statistics

## Use Cases

### 1. One-Time Seed from Sensor Data

Extract a high-quality seed from environmental sensor readings:

```bash
./rdt_seed_extractor -c sensor1.csv sensor2.csv sensor3.csv
```

Copy the output into your code for reproducible testing.

### 2. Multiple Files for Higher Entropy

Combine multiple data sources:

```bash
./rdt_seed_extractor temperature.csv humidity.csv pressure.csv wind.csv
```

### 3. Stream Random Data with Custom Seed

```bash
# Extract seed
./rdt_seed_extractor -u mydata.csv > seed.txt

# Use seed with PRNG
SEED_VALS=$(cat seed.txt | awk '{print $2}')
./rdt_prng_stream_v2 $SEED_VALS | dieharder -a -g 200
```

### 4. Python Integration

```python
from rdt_seed_extractor import extract_seed_bytes
import hashlib

# Extract seed from data
seed = extract_seed_bytes(['sensor_data.csv'])

# Use with any library that accepts byte seeds
# Example: Use as key for ChaCha20, AES-CTR, etc.
print(f"256-bit seed (hex): {seed.hex()}")
print(f"SHA-256 of seed: {hashlib.sha256(seed).hexdigest()}")
```

## Creating Your Own Sensor Data

The seed extractor works best with:
- **CSV files** with numeric data
- **Time-series data** (timestamps, measurements)
- **Real-world sensor readings** (temperature, humidity, accelerometer, etc.)
- **Multiple files** for increased entropy

Example CSV format:

```csv
timestamp,temperature,humidity,pressure
1704672000,22.5,45.3,1013.2
1704672060,22.6,45.1,1013.3
1704672120,22.4,45.5,1013.1
```

## Standalone Usage

The seed extractor can be used with **any PRNG**, not just RDT256:

### With Standard PRNGs

```c
#include "rdt_seed_extractor.h"
#include <stdlib.h>

// Extract seed
uint8_t seed[32];
rdt_seed_extract_file("sensor.csv", seed);

// Use with C's random() (not recommended for crypto!)
srandom(*(uint32_t*)seed);

// Or use with a cryptographic PRNG
// your_crypto_prng_init(seed, 32);
```

### With Python's random module

```python
from rdt_seed_extractor import extract_seed_bytes
import random

seed = extract_seed_bytes(['sensor_data.csv'])
random.seed(int.from_bytes(seed, 'little'))

# Generate random numbers
print([random.random() for _ in range(10)])
```

### With NumPy

```python
import numpy as np
from rdt_seed_extractor import extract_seed_integers

s0, s1, s2, s3 = extract_seed_integers(['sensor_data.csv'])

# Use with NumPy's Generator
seed_array = np.array([s0, s1, s2, s3], dtype=np.uint64)
rng = np.random.Generator(np.random.PCG64(seed_array))

# Generate random data
random_data = rng.random(1000)
```

## Performance Notes

- **Extraction Time**: Typically < 100ms for sensor files up to 1MB
- **Recommended File Sizes**: 10KB - 10MB
- **Multi-file Extraction**: Linear scaling with file count

## Validation

The seed extractor has been validated with:
- **Input min-entropy**: 2.80 bits/byte (sensor data)
- **Output min-entropy**: 7.82 bits/byte (extracted seed)
- **Avalanche effect**: 49.1% (near-ideal)
- **Uniqueness**: 100% across test samples

See `../docs/seed_extractor.md` for complete validation results.

## License

MIT License - See `../LICENSE`

## Author

Steven Reid (ORCID: 0009-0003-9132-3410)
