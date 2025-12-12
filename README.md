RDT Cryptographic Suite (Experimental Research Code)

Recursive Depth Transform (RDT) Randomness Primitives
Author: Steven Reid
License: MIT

IMPORTANT DISCLAIMER
This repository contains experimental research code, not production cryptographic software.
The RDT-CORE, RDT-PRNG, and RDT-DRBG components:

have not undergone formal cryptanalysis

have no proven security guarantees

are not standardized primitives

must NOT be used for real-world security, encryption, authentication, key generation, or any system requiring cryptographically secure randomness

All components are intended solely for research, experimentation, benchmarking, and mathematical exploration.
Nothing here should be considered secure.

OVERVIEW
The RDT Cryptographic Suite is a set of experimental randomness primitives based on a nonlinear transformation combining:

recursive bit-depth analysis

scalar-field projections

epsilon-channel perturbation

ARX (Add-Rotate-Xor) diffusion

The project explores statistical and structural properties of recursive-depth nonlinear functions.
It is not intended to produce hardened cryptographic primitives.

This repository includes:
RDT-CORE (Experimental) — Core 64-bit nonlinear mixing primitive
RDT-PRNG (Experimental) — High-diffusion pseudorandom generator
RDT-DRBG (Experimental) — DRBG with internal key evolution and reseeding
Test Suite (Stable) — Avalanche and statistical randomness tests

KEY FEATURES (Experimental Only)

strong empirical diffusion

256-bit internal state

high avalanche performance (~32 flipped bits per 64-bit output)

stable behavior across seeds

good performance on statistical tests

simple, portable C99 implementation

None of these properties imply cryptographic security.

REPOSITORY STRUCTURE

src/
rdt_core.c
rdt_prng.c
rdt_drbg.c
rdt.h

tests/
run_results.py

docs/
architecture.md
core.md
prng.md
security.md
tests.md
roadmap.md

LICENSE
README.md

RDT-CORE (Research Primitive)
RDT-CORE is a 64-bit nonlinear transformation mixing:

recursive bit depth

scalar-field projections

epsilon-channel perturbation

ARX-based rotation and multiplication

Evaluated using:

avalanche and multi-round avalanche tests

bit diffusion heatmaps

differential trail scanning

autocorrelation and FFT analysis

seed-sweep avalanche stability

These results describe statistical behavior only and do not constitute evidence of cryptographic strength.

RDT-PRNG (Not Cryptographically Secure)
The PRNG is a deterministic 256-bit state machine using RDT-CORE.
It shows strong statistical properties and stable behavior across seeds.
Suitable for: simulations, visualization, experimental analysis, academic exploration.
Not suitable for: cryptographic use, secure randomness, protocols, or systems requiring attack resistance.

RDT-DRBG (Experimental Only)
Adds evolving key material, reseeding, and forward/backward mixing concepts.
Research mechanism only.
Not a standardized or vetted DRBG.
Not appropriate for real-world security.

TESTING
Statistical Tests:

Shannon entropy

byte-frequency distribution

chi-square

serial tests

poker test

runs test

FFT spectrum

autocorrelation

Maurer universal

Diffusion Tests:

single-bit avalanche

multi-round avalanche

bit-position diffusion heatmaps

differential trail mapping

1000-seed avalanche sweep

These tests measure statistical behavior, not cryptographic strength.

DOCUMENTATION
Documentation in docs/ includes:

architecture overview

breakdown of RDT-CORE

PRNG and DRBG design

security considerations and limitations

testing methodology

roadmap

Documentation emphasizes the experimental and non-cryptographic nature of the project.

ROADMAP
Future research directions (non-binding):

RDT-based hash function

sponge/XOF experiments

KDF constructions

MAC experiments

deeper nonlinear analysis

extended multi-terabyte testing

cycle-structure exploration
These are exploratory ideas only.

LICENSE
MIT License — appropriate for open research and experimentation.

FINAL WARNING
Nothing in this repository should be used for real-world cryptography.
This is purely experimental research code and has not been evaluated for security.
