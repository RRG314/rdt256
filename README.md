# **RDT Cryptographic Suite**

Recursive Depth Transform (RDT) Randomness Primitives
Author: Steven Reid
License: MIT (or Apache-2.0)

---

## Overview

The RDT Cryptographic Suite is a collection of randomness primitives based on a nonlinear transformation combining:

* recursive bit-depth evaluation
* scalar-field projections
* ARX (Add-Rotate-Xor) diffusion
* epsilon-channel perturbation layers

The result is a high-diffusion, high-entropy transformation suitable for pseudorandom number generation and deterministic random bit generation.

This repository currently includes:

| Component  | Status | Description                                       |
| ---------- | ------ | ------------------------------------------------- |
| RDT-CORE   | Stable | Core 64-bit nonlinear mixing primitive            |
| RDT-PRNG   | Stable | High-quality pseudorandom generator               |
| RDT-DRBG   | Stable | Deterministic Random Bit Generator with reseeding |
| Test Suite | Stable | Avalanche testing and statistical analysis tools  |

There are **no encryption or stream cipher components** in the repository.

---

## Key Features

* 64-bit mixing primitive with strong diffusion
* 256-bit state and 256-bit keying for PRNG/DRBG
* High avalanche performance (~32 flipped bits per 64-bit output)
* Stable behavior across seeds (verified via large seed sweeps)
* Strong performance on Shannon entropy, chi-square, runs, serial tests
* DRBG includes automatic reseeding, forward secrecy, and backward secrecy
* Implemented in portable C99 with simple, self-contained code

---

## Repository Structure

```
RDT-Crypto/
│
├── src/
│   ├── rdt_core.c
│   ├── rdt_prng.c
│   ├── rdt_drbg.c
│   └── rdt.h
│
├── tests/
│   ├── avalanche_test.py
│   ├── statistics_test.py
│   ├── dieharder_instructions.md
│   └── practrand_instructions.md
│
├── docs/
│   ├── architecture.md
│   ├── core.md
│   ├── prng.md
│   ├── security.md
│   ├── tests.md
│   └── roadmap.md
│
├── LICENSE
└── README.md
```

---

## RDT-CORE

RDT-CORE is the foundational 64-bit nonlinear function used by all components in the suite.
It combines:

* recursive bit-depth
* scalar-field interactions
* epsilon-based mixing
* ARX diffusion steps
* data-dependent rotational schedules

The function has been subjected to testing including:

* avalanche and multi-round avalanche
* bit-position diffusion uniformity
* differential trail scanning
* seed-sweep avalanche stability
* FFT and autocorrelation analysis

Empirical tests show strong diffusion and no detectable structural weaknesses under standard randomness diagnostics.

---

## RDT-PRNG

The PRNG uses the core mixing function to evolve a 256-bit internal state and produce 64-bit outputs.

Properties:

* deterministic
* passes common statistical randomness tests
* high diffusion across outputs
* retains stable behavior independent of seed
* simple interface for integration
* suitable for simulations, procedural generation, and non-cryptographic applications

---

## RDT-DRBG

The DRBG expands the PRNG design into a stateful, periodically reseeded generator supporting:

* internal state mixing
* key updates
* automatic reseeding after a configurable interval
* forward secrecy
* backward secrecy
* deterministic reproducibility for testing purposes

Extensive testing (multi-MB streams) confirms strong statistical quality and stable output characteristics.

---

## Testing

The repository provides tooling for:

### Avalanche and Diffusion Tests

* single-round avalanche
* multi-round avalanche
* bit-flip histograms
* differential trails
* 1000-seed avalanche sweeps

### Statistical Randomness Tests

* Shannon entropy
* byte-frequency distribution
* serial chi-square
* poker test
* runs test
* autocorrelation
* FFT frequency analysis
* Maurer universal test

External test instructions for Dieharder and PractRand are also included.

---

## Documentation

The `docs/` directory contains:

* detailed architecture of the RDT transformation
* mathematical description of RDT-CORE
* PRNG and DRBG specifications
* security considerations and limitations
* description of test methodology
* development roadmap for future improvements
  (hash, MAC, KDF components are listed as possibilities, not implemented)

---

## Roadmap

future research areas include:

* RDT-based hash function
* KDF constructions
* MAC based on RDT-CORE
* extended test vectors and formal proofs

These components are not included in this repository at this stage.

---

## License

MIT License 

---

