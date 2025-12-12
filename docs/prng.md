# RDT-PRNG and RDT-DRBG Specification

## Introduction

This document describes the two pseudorandom generators included in the RDT suite:

1. RDT-PRNG: a 256-bit state deterministic generator
2. RDT-DRBG: a structured deterministic random bit generator with reseeding

Both rely on the shared RDT-CORE mixing primitive.

---

## RDT-PRNG

### State

The PRNG maintains a 256-bit internal state:

uint64_t S[4];

### Initialization

The state is initialized from a single 64-bit seed:

S[0] = seed
S[1] = seed xor constant
S[2] = seed << 1
S[3] = bitwise_not(seed)

This provides separation between state words and avoids degenerate evolution.

### Update Function

Each call to the generator updates the internal state using RDT-CORE:

S[0] ^= rdt_mix(S[1], K)
S[1] ^= rdt_mix(S[2], K)
S[2] ^= rdt_mix(S[3], K)
S[3] ^= rdt_mix(S[0], K)

This four-way feedback structure ensures rapid diffusion and prevents low-order correlations.

### Output

The PRNG returns:

return S[0]

### Intended Use

This generator is designed for:

- simulations
- procedural generation
- statistical experimentation
- serving as a baseline driver for DRBG testing

It is not intended as a cryptographic random number generator.

---

## RDT-DRBG

### State

The DRBG maintains:

uint64_t K[4];   // key
uint64_t S[4];   // internal state
uint64_t reseed_counter;

Total: 512 bits of mixed key/state plus a reseed counter.

### Initialization

Seed material is expanded into key and state arrays:

K[i] = seed[i] xor constant
S[i] = seed[i] xor bitwise_not(constant)

This prevents zero-state lock and guarantees divergence for different seeds.

### Generation Step

Each call evolves state and key:

1. State Update
   S[i] = rdt_mix(S[i] xor K[i], K)

2. Key Update (forward secrecy)
   K[i] ^= rdt_mix(S[i], K)

3. Reseed Counter
   reseed_counter++

4. Automatic Reseed
   Triggered when reseed_counter exceeds a predefined interval.

### Output

return S[0]

### Security Properties (Empirical)

- Forward secrecy: compromise of current state does not reveal prior outputs.
- Backward secrecy: key evolution prevents reconstruction of future outputs.
- Reseed mixing: periodic reseeding prevents cycle formation.
- Statistical quality: high entropy and uniformity across large output sequences.

### Intended Use

Appropriate for:

- deterministic randomness in research
- structured PRNG testing
- reproducible experiments requiring high-quality randomness

Not intended for production cryptographic use without further analysis.

---

## Summary

RDT-PRNG provides a simple deterministic generator with strong statistical behavior.  
RDT-DRBG extends this design with key evolution and reseeding, offering structured secrecy properties suitable for research and controlled randomness experiments.
