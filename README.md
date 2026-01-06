**RDT256 Cryptographic Suite (Experimental Research Code)**

Author: Steven Reid

ORCID:0009-0003-9132-3410

License: MIT



**IMPORTANT DISCLAIMER**
This repository contains experimental research code, not production cryptographic software.
The RDT-CORE, RDT-PRNG, RDT-PRNG_STREAM, RDT-PRNG_STREAM_v2, and RDT-DRBG components:

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
RDT-PRNG_STREAM (Experimental) — Streaming RDT-PRNG variant for external test batteries
RDT-PRNG_STREAM_v2 (Experimental) — Enhanced variant with cross-state diffusion (NIST & BigCrush validated)
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
rdt_prng_stream_v2.c
rdt_prng_stream_v2.h
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

**Empirical properties (RDT-PRNG_STREAM)**:

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

**RDT-PRNG_STREAM_v2 (Enhanced Cross-Diffusion Variant)**

RDT-PRNG_STREAM_v2 is an enhanced variant that adds **cross-state rotational diffusion** after each generation step. This provides improved avalanche properties and stronger statistical quality while using the same core RDT mixing primitive.

**Changes from v1:**

| Component | v1 | v2 |
|-----------|----|----|
| Seed initialization | Single 64-bit value | 4 × 64-bit values with mixing |
| State update | Direct XOR | XOR + cross-state rotation |

**v2-specific enhancement:**
```c
/* After mixing each state lane */
S[0] ^= t0; S[1] ^= t1; S[2] ^= t2; S[3] ^= t3;

/* v2: Cross-state rotational diffusion */
S[0] ^= rotl64(S[1], 21);
S[1] ^= rotl64(S[2], 35);
S[2] ^= rotl64(S[3], 49);
S[3] ^= rotl64(S[0], 11);
```

This ensures changes in any state lane propagate to all other lanes within 2 steps.

**Statistical test results (RDT-PRNG_STREAM_v2):**

* **NIST SP 800-22 Rev 1a**: Tested with official sts-2.1.2 suite. Configuration: 100 bitstreams × 1,000,000 bits each (100M bits total) at significance level α = 0.01. **Result: 15/15 tests passed.** All proportions exceeded minimum threshold (96/100 for standard tests, 57/60 for random excursion tests).

| Test | Proportion | P-Value |
|------|------------|---------|
| Frequency | 98/100 | 0.719747 |
| BlockFrequency | 99/100 | 0.289667 |
| CumulativeSums | 98/100 | 0.924076 |
| Runs | 99/100 | 0.739918 |
| LongestRun | 100/100 | 0.851383 |
| Rank | 99/100 | 0.699313 |
| FFT | 98/100 | 0.851383 |
| NonOverlappingTemplate | 96-100/100 | Various |
| OverlappingTemplate | 99/100 | 0.678686 |
| Universal | 100/100 | 0.946308 |
| ApproximateEntropy | 98/100 | 0.153763 |
| RandomExcursions | 58-60/60 | Various |
| RandomExcursionsVariant | 57-60/60 | Various |
| Serial | 98-100/100 | 0.719747 |
| LinearComplexity | 99/100 | 0.001757 |

* **TestU01 BigCrush**: 160 statistical tests executed over ~5-6 hours. **Result: All tests passed** with p-values within acceptable range [0.001, 0.999].

**Usage (v2):**
```bash
# Build
make rdt_prng_stream_v2

# Streaming mode
./rdt_prng_stream_v2 | dieharder -a -g 200
./rdt_prng_stream_v2 | smokerand default stdin64

# With custom 256-bit seed (4 × 64-bit hex values)
./rdt_prng_stream_v2 0xe607dabdfc9538b5 0x0050f7866258289c 0xedc2d97a03b312ad 0xcaedbc215ece9a31
```

These are empirical statistical results only and do not imply cryptographic strength.

**Performance (streaming mode, same environment)**:

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

**Performance Update (Revised Streaming Results)**
Background
Earlier versions of RDT-PRNG_STREAM reported substantially lower streaming throughput (≈ 15 MiB/s). Those measurements were obtained using an unbuffered reference implementation, where each 64-bit output was written individually to stdout.

Subsequent profiling identified output I/O overhead—rather than the core nonlinear mixing function—as the dominant performance bottleneck in that configuration.

Implementation Changes

The streaming implementation was revised to improve throughput while preserving identical generator logic and output behavior. Key changes include:

introduction of buffered output for stdout

aggressive inlining of core mixing operations

reduced function call overhead

improved instruction-level parallelism

compilation with -O3 -march=native

No changes were made to:

the RDT-CORE mixing primitive

the PRNG state transition logic

the statistical properties of the generator

The update is purely an implementation-level optimization.

Revised Benchmark Methodology

Updated benchmarks were conducted using the same stream-based testing model, with improved buffering:

platform: x86-64 Linux (Google Colab VM)

compiler: GCC

flags: -O3 -march=native -std=c11

output size: 256 MiB

timing method: shell builtin time

identical I/O paths for all generators

Updated Comparative Streaming Results
Generator	State size	Time (256 MiB)	Throughput
SplitMix64	64-bit	1.708 s	~150 MiB/s
xoshiro256**	256-bit	1.641 s	~156 MiB/s
PCG64 (XSL RR)	128-bit	1.804 s	~142 MiB/s
RDT-PRNG_STREAM	256-bit	1.102 s	~232 MiB/s
Interpretation

The revised results demonstrate that, when I/O overhead is amortized appropriately, RDT-PRNG_STREAM achieves substantially higher sustained streaming throughput than previously reported.

This does not imply that the underlying algorithm is intrinsically faster per step than minimal ARX generators. Rather, it reflects:

effective overlap of computation and I/O

deeper arithmetic pipelines within the nonlinear mixing core

implementation-level efficiency under pipe-based workloads

The original lower throughput measurements remain valid for the unbuffered reference implementation and are retained for historical accuracy.

Scope and Limitations

Results are empirical and environment-dependent

No claims are made regarding cryptographic security

No claims are made regarding per-output latency or cycle-optimality

Results apply specifically to sustained streaming workloads
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

**DOCUMENTATION**
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
