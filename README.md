
**RDT256 Cryptographic Suite (Experimental Research Code)**

Author: Steven Reid

ORCID:0009-0003-9132-3410

License: MIT



**IMPORTANT DISCLAIMER**
This repository contains experimental research code, not production cryptographic software.
The RDT-CORE, RDT-PRNG, RDT-PRNG_STREAM, and RDT-DRBG components:

* have not undergone formal cryptanalysis
* have no proven security guarantees
* are not standardized primitives
* must NOT be used for real-world security, encryption, authentication, key generation, or any system requiring cryptographically secure randomness

All components are intended solely for research, experimentation, benchmarking, and mathematical exploration.
Nothing here should be considered secure.

**OVERVIEW**
The RDT Cryptographic Suite is a set of experimental randomness primitives based on a nonlinear transformation combining:

* recursive bit-depth analysis
* scalar-field projections
* epsilon-channel perturbation
* ARX (Add-Rotate-Xor) diffusion

The project explores statistical and structural properties of recursive-depth nonlinear functions.
It is not intended to produce hardened cryptographic primitives.

This repository includes:
RDT-CORE (Experimental) — Core 64-bit nonlinear mixing primitive
RDT-PRNG (Experimental) — High-diffusion pseudorandom generator
RDT-PRNG_STREAM (Experimental) — Streaming RDT-PRNG variant for external test batteries (recommended and most tested)
RDT-DRBG (Experimental) — DRBG with internal key evolution and reseeding
Test Suite (Stable) — Avalanche and statistical randomness tests

**KEY FEATURES (Experimental Only)**

* strong empirical diffusion
* 256-bit internal state
* high avalanche performance (~32 flipped bits per 64-bit output)
* stable behavior across seeds
* good performance on statistical tests
* simple, portable C99 implementation

None of these properties imply cryptographic security.

**REPOSITORY STRUCTURE**

src/
rdt_core.c
rdt_prng.c
rdt_prng_stream.c
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

* recursive bit depth
* scalar-field projections
* epsilon-channel perturbation
* ARX-based rotation and multiplication

Evaluated using:

* avalanche and multi-round avalanche tests
* bit diffusion heatmaps
* differential trail scanning
* autocorrelation and FFT analysis
* seed-sweep avalanche stability

These results describe statistical behavior only and do not constitute evidence of cryptographic strength.

RDT-PRNG (Not Cryptographically Secure)
The PRNG is a deterministic 256-bit state machine using RDT-CORE.
It shows strong statistical properties and stable behavior across seeds.
Suitable for: simulations, visualization, experimental analysis, academic exploration.
Not suitable for: cryptographic use, secure randomness, protocols, or systems requiring attack resistance.

**RDT-PRNG_STREAM (Experimental, Recommended for Testing)**
RDT-PRNG_STREAM is a streaming reference implementation of RDT-PRNG. It uses the same 256-bit state and core mixing function as RDT-PRNG, but exposes a `stdin64`-compatible binary stream (64-bit values written continuously to stdout).

This variant is the **most thoroughly tested implementation** in this repository and is the **recommended version for external evaluation and benchmarking** (Dieharder, SmokeRand, etc.).

Empirical properties (RDT-PRNG_STREAM):

* bit balance ≈ 0.5 per bit over long runs
* average avalanche ≈ 32 flipped bits per 64-bit output under single-bit input changes
* low serial correlation (~10⁻⁴)
* stable behavior across wide seed ranges
* no detectable linear artifacts under tested batteries

Statistical test results (RDT-PRNG_STREAM):

* **Dieharder**: full battery run via `./rdt_prng stream | dieharder -a -g 200`; no `FAILED` tests; a few `WEAK` results (e.g. in `diehard_craps`, `sts_runs`, some `sts_serial` / `rgb_bitdist` cases), which is typical for non-cryptographic PRNGs over large batteries
* **SmokeRand express**: run via `./rdt_prng stream | smokerand express stdin64`; 7/7 tests reported as `Ok` (including `byte_freq`, `bspace32_1d`, `bspace8_4d`, `bspace4_8d`, `bspace4_8d_dec`, `linearcomp_high`, `linearcomp_low`); quality score **4.00 (good)**; ≈ 151,155,712 bytes processed (~2^27.17, ~128 MiB)
**SmokeRand default**: executed via ./rdt_prng stream | smokerand default stdin64; the default battery completed with all reported p-values falling within expected statistical ranges. Core tests (including monobit_freq on 2³⁴ bits with p ≈ 0.496488, byte_freq p ≈ 0.690814, word16_freq p ≈ 0.496131, and multiple bspace* variants) showed no failed tests and no systematic anomalies. Results were consistent with the SmokeRand express battery and repeated Dieharder runs.
Internal test harness measurements:

* average Hamming distance between paired outputs ≈ 32.09 bits
* entropy per bit reported as 1.000000
* bit frequencies per position in approximately [0.4993, 0.5007]

These are empirical statistical results only and do not imply cryptographic strength.

Performance (streaming mode, same environment):

Performance was measured by streaming **1 GiB** of output through a pipe on the same environment (Google Colab VM):

| Generator           | State   |    Throughput | Time per 64-bit output |
| ------------------- | ------- | ------------: | ---------------------: |
| SplitMix64          | 64-bit  |    ~155 MiB/s |                 ~49 ns |
| xoshiro256**        | 256-bit |    ~150 MiB/s |                 ~51 ns |
| **RDT-PRNG_STREAM** | 256-bit | **~15 MiB/s** |            **~500 ns** |

Interpretation:

* RDT-PRNG_STREAM is about **10× slower** than minimal ARX generators (SplitMix64, xoshiro256**)
* The slowdown is expected given the heavier, recursive nonlinear mixing
* Performance should be considered **moderate** and acceptable for simulations, experimentation, and statistical testing, but **not optimized for maximum throughput**
* No performance comparison has been made against cryptographic generators in this environment, so no claims are made in that direction

Acknowledgement (SmokeRand):
External statistical testing of RDT-PRNG_STREAM was performed using the **SmokeRand** test suite by GitHub user **`alvoskov`**.
The RDT author is solely responsible for interpreting these results; this use and mention do **not** imply endorsement or validation of RDT by the SmokeRand author.

RDT-DRBG (Experimental Only)
Adds evolving key material, reseeding, and forward/backward mixing concepts.
Research mechanism only.
Not a standardized or vetted DRBG.
Not appropriate for real-world security.

TESTING
Internal Statistical Tests

Shannon entropy

Byte-frequency distribution

Chi-square tests

Serial tests

Poker test

Runs test

FFT spectrum analysis

Autocorrelation

Maurer universal statistical test

Diffusion Tests

Single-bit avalanche testing

Multi-round avalanche testing

Bit-position diffusion heatmaps

Differential trail mapping

1000-seed avalanche stability sweep

External Statistical Batteries (RDT-PRNG_STREAM)

External statistical evaluation was performed using established third-party test suites. These tests assess statistical behavior only and do not imply cryptographic security.

Dieharder (Repeated Runs)

The Dieharder full battery was executed three independent times using the streaming interface:

./rdt_prng stream | dieharder -a -g 200


Results summary:

Across all three runs, no tests returned FAILED

Some tests occasionally returned WEAK classifications (e.g. diehard_craps, sts_runs, selected sts_serial and rgb_bitdist instances)

These same tests passed cleanly in at least one of the three runs

No test exhibited persistent failure across repeated executions

This behavior is consistent with expectations for non-cryptographic PRNGs under large statistical batteries, where isolated WEAK p-values may arise due to statistical variance and typically resolve across repeated runs.

SmokeRand Express Battery

SmokeRand express testing was performed using:

./rdt_prng stream | smokerand express stdin64


Summary:

7 / 7 tests reported Ok

Quality score: 4.00 (good)

Approximately 151,155,712 bytes (~144 MiB) processed

No suspicious or failed results observed

Covered tests included byte-frequency analysis, multiple birthday-spacing variants, decimation tests, and linear complexity checks.

SmokeRand Default Battery

The SmokeRand default battery was executed on RDT-PRNG_STREAM using:

./rdt_prng stream | smokerand default stdin64


Summary:

The default battery completed within expected statistical ranges

All reported p-values fell within acceptable bounds

No failed tests were observed

Results were consistent with both the express battery and Dieharder outcomes

As expected for long-run statistical testing, p-values varied across tests but showed no systematic anomalies or recurring deviations.

Interpretation

Taken together:

Repeated Dieharder runs demonstrate stable behavior with no persistent failures

SmokeRand express and default batteries indicate statistically well-behaved output

Observed WEAK results in Dieharder are intermittent, non-persistent, and within normal expectations for empirical testing

These results support the statistical quality and stability of RDT-PRNG_STREAM for research, experimentation, and benchmarking, while not implying cryptographic security.

All results are empirical and should not be interpreted as evidence of resistance to cryptanalytic attack.

DOCUMENTATION
Documentation in docs/ includes:

* architecture overview
* breakdown of RDT-CORE
* PRNG and DRBG design
* security considerations and limitations
* testing methodology
  and roadmap

Documentation emphasizes the experimental and non-cryptographic nature of the project.

ROADMAP
Future research directions (non-binding):

* RDT-based hash function
* sponge/XOF experiments
* KDF constructions
* MAC experiments
* deeper nonlinear analysis
* extended multi-terabyte testing
* cycle-structure exploration

These are exploratory ideas only.

LICENSE
MIT License — appropriate for open research and experimentation.

FINAL WARNING
Nothing in this repository should be used for real-world cryptography.
This is purely experimental research code and has not been evaluated for security.
