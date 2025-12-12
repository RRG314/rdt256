# Development Roadmap

## Completed Components

The following components are fully implemented and included in the repository:

- RDT-CORE: the 64-bit nonlinear mixing primitive
- RDT-PRNG: deterministic pseudorandom generator with 256-bit state
- RDT-DRBG: deterministic random bit generator with reseeding and key evolution
- Avalanche and diffusion testing suite
- Statistical randomness test suite
- Documentation of architecture, core, PRNG/DRBG, and testing methodology

These components are suitable for experimentation, evaluation, and research.

---

## In Progress

### Extended Analysis

Ongoing work includes:

- dieharder, nist, and all other tests need to be conductucted
- deeper structural evaluation of the RDT-CORE transform
- consistency testing across a broader range of seeds
- expanded avalanche and differential analysis
- produce additional reference output vectors for reproducibility

### Repository Tooling

Planned improvements:

- standalone CLI utilities for PRNG and DRBG output
- build scripts or Makefiles for easier compilation
- streamlined wrapper functions for automated large-scale testing

---

## Possible Future Work

These items represent potential directions for further development.
They are not included in the current repository and are not guaranteed to be added.

### 1. RDT-Based Hash Function
A 512-bit sponge-based construction using repeated RDT-CORE permutations.

### 2. Extendable Output Function (XOF)
A variable-length output mode based on the hash sponge.

### 3. Key-Derivation Function (KDF)
A simple HKDF-style derivation layer based on RDT hashing.

### 4. Message Authentication Code (MAC)
A MAC construction using RDT hashing or keyed compression.

### 5. Enhanced DRBG
Extensions may include:
- configurable reseed interval
- multi-source entropy integration
- deterministic or hybrid operational modes

### 6. Vectorization and Optimization
Options include:
- SSE2, AVX2, NEON implementations
- assembly-level ARX optimization
- unrolled loops for high throughput

### 7. Extended Empirical Testing
Plans include:
- PractRand multi-terabyte streams
- Dieharder multi-seed batch testing
- NIST STS automated evaluations
- full autocorrelation surface mapping

### 8. Formal Analysis
Areas of theoretical study:
- recursive-depth-based nonlinear mappings
- cycle structure of repeated RDT-CORE composition
- algebraic degree / nonlinearity analysis
- resistance to approximations or differential patterns

---

## Summary

The RDT suite is a structured research project centered on recursive-depth nonlinear transformations and their pseudorandom properties. This roadmap describes potential extensions and ongoing development directions. The current implementation is complete and suitable for experimentation, benchmarking, and continued research.
