# Testing Methodology

## Overview

The RDT suite includes procedures and scripts for evaluating:

- avalanche behavior
- diffusion strength
- cross-seed stability
- bit-level statistical randomness
- large-sample structural properties

Testing is performed using both internal Python scripts and standard external test suites.  
All tests are deterministic and reproducible.

---

## Avalanche Testing

Avalanche tests measure the sensitivity of the output to 1-bit changes in the input.  
These tests are applied to both the RDT-CORE primitive and to the PRNG/DRBG outputs.

### 1. Single-Round Avalanche

Given two inputs differing by a single bit:

input1 = seed  
input2 = seed xor (1 << bit)

Compute:

output1 = mix(input1)  
output2 = mix(input2)  
flips   = popcount(output1 xor output2)

Expected mean for 64-bit outputs: approximately 32 flipped bits.

### 2. Multi-Round Avalanche

The output of one mix call is fed into the next.  
Avalanche is tracked across several rounds to verify stability and convergence.

### 3. Bitwise Avalanche Probability

For many seeds, record flip probability for each output bit position.  
A uniform distribution indicates consistent diffusion.

### 4. Differential Trail Scanning

For a fixed seed set, all 64 single-bit input differences are tested.  
This identifies any unusually low- or high-weight transitions.

### 5. 1000-Seed Avalanche Sweep

Random seeds are sampled:

- compute avalanche between paired seeds
- record statistics across the entire set

This test ensures the avalanche property is not seed-dependent.

---

## Statistical Randomness Tests

### Shannon Entropy

Entropy is computed over large (≥16 MB) output samples.  
Expected values for byte-level entropy: near 7.999–8.000 bits per byte.

### Monobit Frequency

The mean of all bits:

mean(bits)

Expected value: approximately 0.5.

### Runs Test

Counts the number of consecutive runs of equal bits.  
Expected number is based on binomial assumptions.

### Chi-Square Test (Byte-Level)

Measures distribution uniformity across 0–255.  
Expected chi-square ~255±sqrt(2*255) for random distributions.

### Serial Chi-Square (16-bit Symbols)

Analyzes uniformity of consecutive byte pairs.  
Detects structural biases in transitions.

### Poker Test

Examines uniformity of 4-bit symbol frequencies.

### Autocorrelation

Computes correlation at lags:

lag ∈ {1, 2, 8, 64, 256}

Low absolute autocorrelation values indicate lack of periodic patterns.

### FFT Spectral Test

The frequency spectrum of the generated bits is computed.  
A flat spectrum indicates lack of hidden periodicity.

### Maurer Universal Test

Estimates compressibility of the output stream.  
Values near those for random generators indicate good structural unpredictability.

---

## External Test Suites

### Dieharder

To test with Dieharder, generate at least 128MB:

./drbg > data.bin  
dieharder -a -g 201 -f data.bin

The “-g 201” option selects file input.

### PractRand

For live testing:

./drbg | RNG_test stdin32

PractRand adapts automatically to stream size and will continue until stopped.

---

## Testing Recommendations

1. Run avalanche tests before statistical tests to verify diffusion.  
2. Use at least 16MB of output for entropy and chi-square analysis.  
3. For large-scale evaluation, run PractRand to at least 256GB if possible.  
4. Examine autocorrelation and FFT plots for non-random structure.  
5. Repeat all tests on multiple seeds to ensure stability.

---

## Summary

The included tests provide both structural (avalanche) and statistical (entropy, chi-square, etc.) evaluation of the RDT suite. These procedures help analyze the nonlinear behavior and mixing quality of the RDT-CORE transformation and the generators derived from it.
