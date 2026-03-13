
# RDT-PRNG and RDT-DRBG Specification

## Introduction

This document describes the pseudorandom generators included in the RDT suite:

1. **RDT-PRNG** — a 256-bit state deterministic generator
2. **RDT-PRNG_STREAM** — a streaming reference implementation of RDT-PRNG for external testing
3. **RDT-DRBG** — a structured deterministic random bit generator with reseeding
4. **RDT-DRBG_v2** — a context-based DRBG path using an HMAC-SHA256 core with RDT-conditioned convenience seeding

All generators rely on the shared **RDT-CORE** nonlinear mixing primitive.

This document specifies structure, behavior, and intended use.
It does **not** claim cryptographic security.

---

## RDT-PRNG

### State

The PRNG maintains a 256-bit internal state:

```
uint64_t S[4];
```

Each state word is 64 bits. No additional counters or buffers are used.

### Initialization

The state is initialized from a single 64-bit seed:

```
S[0] = seed
S[1] = seed xor constant
S[2] = seed << 1
S[3] = bitwise_not(seed)
```

This initialization ensures separation between state words and avoids trivial or degenerate evolution.

### Update Function

Each call updates the internal state using the RDT-CORE mixing primitive:

```
S[0] ^= rdt_mix(S[1], K)
S[1] ^= rdt_mix(S[2], K)
S[2] ^= rdt_mix(S[3], K)
S[3] ^= rdt_mix(S[0], K)
```

This four-way feedback structure provides rapid diffusion and reduces low-order correlations.

### Output

The generator returns:

```
return S[0];
```

One 64-bit value is produced per generation step.

### Intended Use

RDT-PRNG is designed for:

* simulations
* procedural generation
* statistical experimentation
* serving as a baseline generator for DRBG testing

It is **not intended** as a cryptographically secure random number generator.

---

## RDT-PRNG_STREAM

### Overview

RDT-PRNG_STREAM is a **streaming reference implementation** of RDT-PRNG intended for **external statistical evaluation and benchmarking**.

It uses the **same state, initialization, and update function** as RDT-PRNG, but exposes output as a continuous binary stream compatible with standard test batteries.

### State

The internal state is identical to RDT-PRNG:

```
uint64_t S[4];
```

No additional state variables are introduced.

### Initialization

Initialization is identical to RDT-PRNG and uses the same 64-bit seed expansion:

```
S[0] = seed
S[1] = seed xor constant
S[2] = seed << 1
S[3] = bitwise_not(seed)
```

For a given seed, RDT-PRNG and RDT-PRNG_STREAM generate identical output sequences.

### Update Function

State evolution is identical to RDT-PRNG:

```
S[0] ^= rdt_mix(S[1], K)
S[1] ^= rdt_mix(S[2], K)
S[2] ^= rdt_mix(S[3], K)
S[3] ^= rdt_mix(S[0], K)
```

No stream-specific modifications are applied to the internal logic.

### Output and Interface

Instead of returning a value to a caller, RDT-PRNG_STREAM:

* emits successive 64-bit outputs (`S[0]`)
* writes them continuously to `stdout`
* produces a raw binary stream compatible with `stdin64` interfaces

Conceptually:

```
while (true) {
    uint64_t x = rdt_prng_next();
    write(x);
}
```

This design allows the generator to be piped directly into external test suites (e.g. Dieharder, SmokeRand) without wrappers or adapters.

### Intended Use

RDT-PRNG_STREAM is intended for:

* external statistical test batteries
* long-run empirical evaluation
* benchmarking and comparative testing
* validation of RDT-PRNG behavior under continuous output

It is the **most thoroughly tested implementation** of the RDT-PRNG family and the **recommended variant for independent evaluation**.

Like RDT-PRNG, it is **not intended for cryptographic use**.

### Relationship to RDT-PRNG

* RDT-PRNG defines the core generator logic
* RDT-PRNG_STREAM exposes the same logic as a continuous stream
* Internal behavior is identical for a given seed
* Differences are limited to output interface, not algorithmic structure

### Implementation and Performance Update

Earlier performance measurements for RDT-PRNG_STREAM were obtained using an
unbuffered reference implementation, in which each 64-bit output was written
individually to `stdout`. In that configuration, output I/O overhead dominated
runtime and resulted in relatively low observed throughput.

Subsequent revisions introduced implementation-level optimizations that preserve
identical generator logic and output behavior, including:

- buffered output for streaming writes
- aggressive inlining of core mixing functions
- reduced function call overhead
- higher compiler optimization levels

No changes were made to the RDT-CORE mixing primitive, the PRNG state transition
logic, or the statistical properties of the generator.

With these changes, sustained streaming throughput increased substantially under
pipe-based workloads. Updated and comparative performance measurements are
documented in the README Performance section.

The original unbuffered measurements remain valid for historical reference and
are retained for transparency.

---

## RDT-DRBG

### State

The DRBG maintains:

```
uint64_t K[4];   // key material
uint64_t S[4];   // internal state
uint64_t reseed_counter;
```

This represents 512 bits of mixed key and state, plus a reseed counter.

### Initialization

Seed material is expanded into both key and state arrays:

```
K[i] = seed[i] xor constant
S[i] = seed[i] xor bitwise_not(constant)
```

This design prevents zero-state lock and guarantees divergence for distinct seeds.

### Generation Step

Each generation step performs:

1. **State Update**

```
S[i] = rdt_mix(S[i] xor K[i], K)
```

2. **Key Update (Forward Mixing)**

```
K[i] ^= rdt_mix(S[i], K)
```

3. **Reseed Counter Increment**

```
reseed_counter++
```

4. **Automatic Reseed**

Triggered when the reseed counter exceeds a predefined interval.

### Output

```
return S[0];
```

One 64-bit value is emitted per call.

### Security Properties (Empirical)

Observed empirical properties include:

* forward mixing behavior
* backward mixing behavior
* evolving key material
* reduced risk of short cycles
* stable statistical behavior under testing

These properties are **structural and empirical only** and do not constitute formal security guarantees.

### Intended Use

RDT-DRBG is appropriate for:

* deterministic randomness in research
* structured PRNG experimentation
* reproducible experiments requiring high-quality randomness

It is **not intended for production cryptographic use** without further analysis.

---

## RDT-DRBG_v2

### Purpose

RDT-DRBG_v2 is a newer DRBG path added alongside the original custom RDT-DRBG.
Its goal is to keep the repository's RDT-based seeding story while moving the actual
DRBG state machine onto a more disciplined HMAC-SHA256 construction.

### State

The DRBG maintains:

```
uint8_t K[32];
uint8_t V[32];
uint64_t reseed_counter;
int seeded;
```

This matches the standard HMAC-DRBG style key/value state layout.

### Instantiate / Reseed / Generate Model

RDT-DRBG_v2 follows the familiar instantiate/reseed/generate flow:

1. Instantiate from `entropy || nonce || personalization`
2. Optional reseed from `entropy || additional_input`
3. Optional pre-generate update with `additional_input`
4. Block generation via repeated `HMAC(K, V)`
5. Post-generate update and reseed-counter increment

### RDT Integration

The actual state machine is HMAC-SHA256 based.
RDT enters in the convenience `init_u64` path, which deterministically expands three
64-bit inputs through the repository's `rdt_mix` primitive into instantiate material.
There is also a separate `init_system` / `reseed_system` path that reads entropy from
`/dev/urandom` so the DRBG can be integrated without pretending the deterministic
`init_u64` wrapper is equivalent to real entropy collection.

This means:

* the DRBG core behavior is closer to standard HMAC-DRBG semantics
* the repository still preserves an RDT-centered deterministic seeding path
* the system-entropy path is the honest choice for local cryptographic-style use
* the cryptographic discussion should focus on the HMAC-SHA256 core, not on claiming the RDT primitive itself has been cryptographically validated

### Validation Status

RDT-DRBG_v2 includes:

* a built-in SHA-256 HMAC-DRBG known-answer test derived from the NIST validation vectors
* throughput/statistical benchmarking through `make benchmark-honest`
* an external-battery runner for Dieharder, PractRand, and BigCrush-style workflows when those tools are installed locally

### Intended Use

If someone wants a DRBG-style interface from this repository that is easier to justify technically than the original custom DRBG, `RDT-DRBG_v2` is the recommended path.
It is still **experimental** and **not a certified module**, but it is the better integration surface.

---

## Summary

RDT-PRNG provides a simple deterministic generator with strong empirical statistical behavior.
RDT-PRNG_STREAM exposes the same generator as a continuous stream for external evaluation and is the primary reference implementation used in testing.
RDT-DRBG extends this design with key evolution and reseeding, enabling structured experimentation with DRBG-style mechanisms.
RDT-DRBG_v2 adds a more standard-like HMAC-SHA256 DRBG core for users who want a stricter instantiate/reseed/generate interface inside the repo.

All components are **experimental**, **non-standard**, and **not cryptographically validated**.

---
