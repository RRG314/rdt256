# RDT256 Repository Inventory and Reproduction Report

Report generated from the working tree at:
- repository root: `/Users/stevenreid/Documents/New project/repos/rdt256`
- remote: `https://github.com/RRG314/rdt256.git`
- branch: `steven/rdt256-honest-upgrades`
- HEAD commit: `e72d6665a538e4566f406a13a998bd5935e3edae`
- report timestamp: `2026-03-13 10:06:29 EDT`

This document is intended to give a reviewer enough context to:
- understand what is in the repo
- understand how the current implementations work
- reproduce the local builds and test runs
- understand which claims are currently supported by artifacts in this tree
- understand which results are still incomplete or in-flight

## 1. Scope and Current State

This repository is a research codebase centered on an RDT mixing primitive and several generators built on top of it.

At the time of this report, the working tree is not clean. The report describes the actual local state, not only the last committed origin state. That matters because the repo currently contains new source files, updated docs, updated benchmark outputs, and generated local test artifacts that are not yet all committed.

Current `git status --short` snapshot:

```text
 M Makefile
 M QUICKSTART.md
 M README.md
 M benchmarks/benchmark_streams.py
 M docs/prng.md
 M docs/security.md
 M results/stream_benchmark_report.md
 M results/stream_benchmark_results.json
 M src/rdt.h
 M tests/prng_test.c
?? ECOSYSTEM_ROLE.md
?? benchmarks/run_external_batteries.py
?? benchmarks/testu01_rdt_drbg_v2
?? benchmarks/testu01_rdt_drbg_v2.c
?? rdt_drbg_v2
?? rdt_drbg_v2_system_test
?? rdt_drbg_v2_test
?? results/external_battery_results.json
?? src/rdt_drbg_stream.c
?? src/rdt_drbg_v2.c
?? src/rdt_drbg_v2.h
?? src/rdt_drbg_v2_stream.c
?? src/rdt_sha256.c
?? src/rdt_sha256.h
?? tests/rdt_drbg_v2_system_test.c
?? tests/rdt_drbg_v2_test.c
```

Practical implication:
- any reviewer reproducing this exact state needs this working tree, not just the remote repo at the listed commit
- generated binaries and result files are present locally and are described below

## 2. Directory and File Inventory

### 2.1 Root-level files

Source and documentation:
- `.gitignore`: local build/log ignore rules
- `Makefile`: primary build/test/benchmark entry point
- `README.md`: top-level project description and caveats
- `QUICKSTART.md`: user-facing build and usage guide
- `CHANGELOG.md`: release changelog
- `LICENSE`: MIT license
- `package.json`: optional npm wrapper scripts for build/test/benchmark flows
- `RELEASE_NOTES_v1.2.0.md`: release description for v1.2.0
- `REPO_INVENTORY_AND_REPRODUCTION_REPORT.md`: this report

Generated binaries currently present in the repo root:
- `rdt_prng_stream`
- `rdt_prng_stream_v2`
- `rdt_prng_stream_v3`
- `rdt_drbg`
- `rdt_drbg_v2`
- `rdt_seed_extractor`
- `splitmix64_stream`
- `rdt_drbg_v2_test`
- `rdt_drbg_v2_system_test`

Generated object files currently present in the repo root:
- `rdt_core.o`
- `rdt_drbg.o`
- `rdt_drbg_stream.o`
- `rdt_drbg_v2.o`
- `rdt_drbg_v2_stream.o`
- `rdt_prng_stream.o`
- `rdt_seed_extractor.o`
- `rdt_sha256.o`

Untracked local file not part of the generator/test implementation:
- `ECOSYSTEM_ROLE.md`

### 2.2 `src/`

Core primitive:
- `src/rdt_core.h`: public declaration of `rdt_mix`
- `src/rdt_core.c`: implementation of the RDT nonlinear mixing primitive

Legacy public API:
- `src/rdt.h`: public header exposing `rdt_mix`, legacy PRNG, and legacy DRBG functions

Legacy PRNG path:
- `src/rdt_prng.c`: stateful 256-bit PRNG API implementation
- `src/rdt_prng_stream.c`: streaming binary for the legacy PRNG

Enhanced RDT stream path:
- `src/rdt256_stream_v2.h`: public API for `rdt_prng_v2_*`
- `src/rdt256_stream_v2.c`: v2 and v3 generator implementations plus stream mains, selected by build-time macros

Legacy DRBG path:
- `src/rdt_drbg.h`: public API for the original experimental DRBG
- `src/rdt_drbg.c`: original custom DRBG built around `rdt_mix`
- `src/rdt_drbg_stream.c`: streaming binary for the legacy DRBG

New SHA/HMAC support:
- `src/rdt_sha256.h`: public SHA-256 / HMAC-SHA256 declarations
- `src/rdt_sha256.c`: in-repo SHA-256 / HMAC-SHA256 implementation

Improved DRBG path:
- `src/rdt_drbg_v2.h`: public API for the improved DRBG
- `src/rdt_drbg_v2.c`: HMAC-SHA256-based DRBG implementation
- `src/rdt_drbg_v2_stream.c`: streaming binary for the improved DRBG

Seed extractor:
- `src/rdt_seed_extractor.h`: public seed extractor API
- `src/rdt_seed_extractor.c`: implementation of the seed extraction pipeline

### 2.3 `tests/`

- `tests/prng_test.c`: simple legacy DRBG stream smoke generator
- `tests/rdt_drbg_v2_test.c`: known-answer test for `rdt_drbg_v2`
- `tests/rdt_drbg_v2_system_test.c`: system-entropy init/reseed smoke test for `rdt_drbg_v2`
- `tests/rdt_seed_extractor_test.c`: direct seed extractor API regression test
- `tests/run_results.py`: internal statistical smoke-test harness for `rdt_prng_stream_v2`
- `tests/validate_seed_extractor.py`: fixture-based seed extractor validator and micro-benchmark

### 2.4 `benchmarks/`

- `benchmarks/benchmark_streams.py`: side-by-side throughput/statistical comparison harness
- `benchmarks/splitmix64_stream.c`: conventional non-crypto baseline stream generator
- `benchmarks/testu01_rdt_drbg_v2.c`: TestU01 wrapper for `rdt_drbg_v2`
- `benchmarks/testu01_rdt_drbg_v2`: locally built TestU01 wrapper binary
- `benchmarks/run_external_batteries.py`: external battery runner for Dieharder, PractRand, and TestU01

### 2.5 `examples/`

- `examples/README.md`: example usage notes
- `examples/rdt_seed_extractor.py`: Python wrapper around the seed extractor binary
- `examples/integration_example.c`: C integration example wiring the seed extractor into `rdt_prng_v2`
- `examples/sensor_data.csv`: sample input file for the seed extractor

### 2.6 `docs/`

- `docs/architecture.md`: architecture-level project notes
- `docs/background.md`: background/context notes
- `docs/core.md`: notes on the RDT core primitive
- `docs/prng.md`: PRNG/DRBG structure and intended use
- `docs/security.md`: security limitations and threat-model caveats
- `docs/tests.md`: testing-related notes
- `docs/roadmap.md`: roadmap notes
- `docs/seed_extractor.md`: seed extractor notes

### 2.7 `results/`

Current generated result files:
- `results/stream_benchmark_results.json`
- `results/stream_benchmark_report.md`
- `results/seed_extractor_validation.json`
- `results/seed_extractor_validation.md`
- `results/external_battery_results.json`

Current external battery logs:
- `results/external_battery_logs/dieharder.log`
- `results/external_battery_logs/practrand.log`
- `results/external_battery_logs/practrand_manual.log`
- `results/external_battery_logs/testu01_small.log`
- `results/external_battery_logs/testu01_small_manual.log`
- `results/external_battery_logs/testu01_big_manual.log`

### 2.8 `.github/`

- `.github/workflows/ci.yml`: GitHub Actions workflow for build and maintained local tests

## 3. Build System

The repo uses a single `Makefile`.

Compiler configuration:
- `CC = gcc`
- `CFLAGS = -O3 -std=c11 -march=native -Wall -Wextra -Wshadow -Wconversion`

Default build target:

```make
all: rdt_prng_stream rdt_prng_stream_v2 rdt_prng_stream_v3 rdt_drbg rdt_drbg_v2 rdt_seed_extractor
```

Named build targets:
- `rdt_prng_stream`
- `rdt_prng_stream_v2`
- `rdt_prng_stream_v3`
- `rdt_drbg`
- `rdt_drbg_v2`
- `rdt_seed_extractor`
- `splitmix64_stream`

Test targets:
- `test-v2-dieharder`
- `test-drbg-v2`
- `test-drbg-v2-kat`
- `test-drbg-v2-system`
- `test-seed-extractor`
- `validate-seed-extractor`
- `test-all`
- `test-v2-smokerand`
- `test-v2-ent`

Benchmark targets:
- `benchmark-v2`
- `benchmark-honest`

Debug target:
- `debug`: adds `-g -O0 -fsanitize=address -fsanitize=undefined -DDEBUG`

Housekeeping:
- `clean`

At the time this report was written, `make -n all` returned `Nothing to be done for 'all'`, so the default binaries were already built in the local tree.

## 4. External Dependencies

### 4.1 Required for basic repo build

Basic repo builds use:
- GCC or a compatible C compiler
- Python 3 for benchmark and test scripts

### 4.2 Python packages used by internal scripts

The benchmark/test Python scripts use `numpy`.

### 4.3 External batteries used locally for this report

Installed via Homebrew or local source builds:
- `autoconf 2.72`
- `automake 1.18.1`
- `cmake 4.2.3`
- `gsl 2.8`
- `libtool 2.5.4`
- `pkgconf 2.5.1`

Local tool locations used during testing:
- Dieharder binary: `/tmp/rdt256-batteries/local/bin/dieharder`
- PractRand `RNG_test`: `/tmp/rdt256-batteries/PractRand/RNG_test`
- PractRand `RNG_output`: `/tmp/rdt256-batteries/PractRand/RNG_output`
- TestU01 headers/libs: `/tmp/rdt256-batteries/local/include` and `/tmp/rdt256-batteries/local/lib`

These external tools are not vendored into the repo. They were built or installed outside the tree and pointed to explicitly during testing.

## 5. Implementation Details

### 5.1 `rdt_mix`: the RDT core primitive

Defined in `src/rdt_core.c`.

High-level structure:
1. Compute a bit-length-based and popcount-based depth signal.
2. Compute a low-word scalar-field projection using integer square root of the low 32-bit plane.
3. Form a mixed intermediate `p = (x + g) * C1 XOR (d * C2)`.
4. Run an epsilon-channel loop indexed by a small prime table `P[7] = {3,5,7,11,13,17,19}`.
5. XOR the epsilon-channel result back into the intermediate.
6. Apply an ARX-style finalizer with depth-dependent rotation and multiplication constants.
7. Return a rotated final word.

Public interface:
- `uint64_t rdt_mix(uint64_t x, const uint64_t K[4]);`

This primitive is the base nonlinear building block for the legacy PRNG, the legacy DRBG, and the deterministic `init_u64` seeding path in `rdt_drbg_v2`.

### 5.2 Legacy PRNG: `rdt_prng`

Files:
- `src/rdt_prng.c`
- `src/rdt_prng_stream.c`

State:
- `uint64_t S[4]`

Initialization:
- expands a 64-bit seed into four state words with fixed constants and simple transforms

Step function:
- each lane XORs in `rdt_mix` of another lane using the fixed key schedule
- returns `S[0]`

Stream binary:
- `rdt_prng_stream` writes continuous 64-bit outputs to `stdout`
- default seed is `0x0123456789ABCDEF`

This is an experimental deterministic generator. The repo documentation explicitly does not describe it as cryptographically secure.

### 5.3 Enhanced stream generator: `rdt_prng_stream_v2`

Files:
- `src/rdt256_stream_v2.h`
- `src/rdt256_stream_v2.c`

State:
- `uint64_t S[4]`

Initialization:
- accepts four 64-bit seed words or a 32-byte seed buffer
- each lane is pre-mixed with `rdt_mix` and distinct constants
- cross-lane rotations are applied
- state is finalized through additional `rdt_mix` calls
- an all-zero state is replaced with fixed fallback constants

Step function:
1. compute `t0..t3 = rdt_mix(S[1]), rdt_mix(S[2]), rdt_mix(S[3]), rdt_mix(S[0])`
2. XOR `t0..t3` back into `S[0..3]`
3. apply cross-state rotational diffusion:
   - `S[0] ^= rotl64(S[1], 21)`
   - `S[1] ^= rotl64(S[2], 35)`
   - `S[2] ^= rotl64(S[3], 49)`
   - `S[3] ^= rotl64(S[0], 11)`
4. return `S[0]`

Purpose:
- this is the main RDT-centered streaming generator path
- it is the path exercised by `tests/run_results.py`

### 5.4 Output-scrambled variant: `rdt_prng_stream_v3`

Built from the same source file `src/rdt256_stream_v2.c` with `-DRDT_PRNG_V3_MAIN`.

Differences from v2:
- same internal v2 state transition
- larger buffered write path
- applies a SplitMix-style output finalizer to each generated 64-bit value before writing
- maintains an additional `stream_ctr`

This is an output-stage variant only. It does not change the core v2 state-update logic.

### 5.5 Legacy DRBG: `rdt_drbg`

Files:
- `src/rdt_drbg.h`
- `src/rdt_drbg.c`
- `src/rdt_drbg_stream.c`

State:
- `Kd[4]`: 256-bit key-like state
- `Vd[2]`: 128-bit counter-like state
- `reseed_counter`

Initialization:
- `rdt_drbg_init_u64(entropy_seed, nonce, personalization)`
- uses SplitMix64 expansion of combined 64-bit inputs
- derives an additional `PD[4]` material block and applies `drbg_update`

Generation model:
- supports optional `additional` input
- prediction-resistance argument is accepted but treated as caller policy rather than enforced entropy collection
- output blocks come from `rdt_drbg_block(Vd[0], Vd[1], Kd)`
- post-generate update is always applied

`rdt_drbg_block`:
- computes a shell/depth-like classifier from `V0`, `V1`, and the key
- chooses round count and permutation schedule from that shell
- churns four local lanes through ARX-style coupling and repeated `rdt_mix` calls
- final block is another `rdt_mix` of the lane fold

This is novel/custom logic. It is not modeled on a standard published DRBG construction.

### 5.6 SHA-256 / HMAC-SHA256 support

Files:
- `src/rdt_sha256.h`
- `src/rdt_sha256.c`

Purpose:
- reusable, in-repo SHA-256 and HMAC-SHA256 implementation
- introduced to support `rdt_drbg_v2`

Notable implementation details:
- standard SHA-256 round constants
- internal `secure_zero` helper for local state cleanup
- one-shot and incremental interfaces

### 5.7 Improved DRBG: `rdt_drbg_v2`

Files:
- `src/rdt_drbg_v2.h`
- `src/rdt_drbg_v2.c`
- `src/rdt_drbg_v2_stream.c`

State:
- `uint8_t K[32]`
- `uint8_t V[32]`
- `uint64_t reseed_counter`
- `int seeded`

Behavioral model:
- HMAC-SHA256 DRBG-like instantiate / reseed / generate design
- not a full NIST validation module, but much closer to standard DRBG behavior than the legacy custom DRBG

Instantiate:
- concatenates `entropy || nonce || personalization`
- initializes `K = 0x00...00`, `V = 0x01...01`
- runs `drbg_v2_update` once with the seed material
- sets `reseed_counter = 1`

Update function:
- `K = HMAC(K, V || 0x00 || provided_data)`
- `V = HMAC(K, V)`
- if `provided_data` is non-empty:
  - `K = HMAC(K, V || 0x01 || provided_data)`
  - `V = HMAC(K, V)`

Generate:
- optional pre-generate update with `additional_input`
- repeated `V = HMAC(K, V)` blocks copied into the caller output buffer
- post-generate update with the same `additional_input`
- increments `reseed_counter`

Interface limits:
- max request size: `65536` bytes
- reseed interval constant: `281474976710656`
- explicit errors for bad args, not initialized, reseed required, oversized request, unsupported prediction resistance, allocation failure, entropy failure

RDT-specific seeding path:
- `rdt_drbg_v2_init_u64` derives 4 entropy words and 2 nonce words from three 64-bit user inputs via `rdt_mix`
- this preserves an RDT-centered deterministic seeding story for experiments and reproducible streams

OS-entropy path:
- `rdt_drbg_v2_init_system`
- `rdt_drbg_v2_reseed_system`
- both read from `/dev/urandom`

Stream binary:
- deterministic mode:
  - `./rdt_drbg_v2`
  - `./rdt_drbg_v2 <entropy_seed> <nonce> <personalization>`
- system-entropy mode:
  - `./rdt_drbg_v2 --system`
  - `./rdt_drbg_v2 --system <personalization-string>`

Important caveat:
- the deterministic `init_u64` path is useful for reproducible testing
- the least misleading local cryptographic usage surface is `--system` or the `init_system/reseed_system` C APIs

### 5.8 Seed extractor

Files:
- `src/rdt_seed_extractor.h`
- `src/rdt_seed_extractor.c`
- `examples/rdt_seed_extractor.py`
- `examples/integration_example.c`
- `examples/sensor_data.csv`

Pipeline in `src/rdt_seed_extractor.c`:
1. extract numeric values with positional context from input data
2. extract a structural fingerprint
3. apply a block-wise entropy precursor layer
4. apply recursive entropy mixing through `mixer_a` and `mixer_b`
5. finalize with SHA-256 and domain separation

Outputs:
- 32 raw seed bytes
- 4 x `uint64_t`
- file-based and multi-file extraction helpers

The seed extractor is logically separate from the DRBGs. It is an entropy-conditioning / seed-material tool, not the DRBG itself.

## 6. Test and Benchmark Harnesses

### 6.1 Internal stream smoke test

File:
- `tests/run_results.py`

What it does:
- builds `rdt_prng_stream_v2`
- reads 4,000,000 bytes from the stream
- computes:
  - byte entropy
  - monobit frequency
  - runs test
  - byte chi-square
  - serial chi-square
  - autocorrelation at multiple lags
  - avalanche statistics on paired outputs
- writes `RDT_RESULTS.txt`

### 6.2 Honest benchmark harness

File:
- `benchmarks/benchmark_streams.py`

Generators compared:
- `rdt_prng_stream_v2`
- `rdt_prng_stream_v3`
- `rdt_drbg_v2`
- `splitmix64`
- `openssl rand` if available

Metrics computed:
- throughput MiB/s
- entropy bits per byte
- monobit mean
- runs and expected runs
- lag-1 correlation on 64-bit words
- simple `quality_proxy = abs(monobit - 0.5) + abs(lag1_corr_u64)`

Outputs:
- `results/stream_benchmark_results.json`
- `results/stream_benchmark_report.md`

### 6.3 `rdt_drbg_v2` known-answer test

File:
- `tests/rdt_drbg_v2_test.c`

What it verifies:
- instantiate from an official HMAC-DRBG SHA-256 vector
- first generate
- second generate
- compare second generate against the expected 128-byte `ReturnedBits`
- then run a deterministic `init_u64` smoke generate

The vector used is the NIST SHA-256 HMAC-DRBG validation case with:
- 32-byte entropy input
- 16-byte nonce
- expected 128-byte second-generate output

### 6.4 `rdt_drbg_v2` system-init smoke test

File:
- `tests/rdt_drbg_v2_system_test.c`

What it verifies:
- system init succeeds
- generate after system init succeeds
- output is not all-zero
- system reseed succeeds
- generate after system reseed succeeds

### 6.5 External battery harness

Files:
- `benchmarks/run_external_batteries.py`
- `benchmarks/testu01_rdt_drbg_v2.c`

Capabilities:
- builds `rdt_drbg_v2`
- optionally runs:
  - Dieharder
  - PractRand
  - TestU01 SmallCrush / Crush / BigCrush
- writes JSON results and tool logs

Important note on schema/history:
- earlier local runs wrote TestU01 results under a `bigcrush` key even when the battery executed was `small`
- the authoritative source for which TestU01 battery actually ran is the command line and the corresponding log file
- current logs should be trusted over old field names in stale JSON

## 7. Reproduction Commands

### 7.1 Baseline build

```bash
cd /Users/stevenreid/Documents/New\ project/repos/rdt256
make clean
make all
npm run build
```

### 7.2 Deterministic local DRBG validation

```bash
make test-drbg-v2-kat
make test-drbg-v2-system
make test-seed-extractor
python3 tests/validate_seed_extractor.py
npm run test:seed
```

### 7.3 Internal stream smoke test

```bash
python3 tests/run_results.py
```

### 7.4 Throughput/statistical benchmark

```bash
make benchmark-honest
```

### 7.5 Example stream commands

Legacy PRNG stream:

```bash
./rdt_prng_stream | head -c 1048576 > sample.bin
```

RDT v2 stream:

```bash
./rdt_prng_stream_v2 | head -c 1048576 > sample.bin
```

RDT v3 stream:

```bash
./rdt_prng_stream_v3 | head -c 1048576 > sample.bin
```

Legacy DRBG stream:

```bash
./rdt_drbg | head -c 1048576 > sample.bin
```

Improved DRBG stream, deterministic:

```bash
./rdt_drbg_v2 | head -c 1048576 > sample.bin
./rdt_drbg_v2 0x123456789abcdef0 0x0f1e2d3c4b5a6978 0xa55aa55aa55aa55a | head -c 1048576 > sample.bin
```

Improved DRBG stream, OS entropy:

```bash
./rdt_drbg_v2 --system | head -c 1048576 > sample.bin
./rdt_drbg_v2 --system reviewer-seed | head -c 1048576 > sample.bin
```

### 7.6 External battery reproduction

Dieharder:

```bash
./rdt_drbg_v2 | /tmp/rdt256-batteries/local/bin/dieharder -a -g 200
```

PractRand:

```bash
./rdt_drbg_v2 | /tmp/rdt256-batteries/PractRand/RNG_test stdin64 -tlmin 1GB -tlmax 1GB -tf 2 -te 1 -tlmaxonly -a
```

TestU01 SmallCrush:

```bash
DYLD_LIBRARY_PATH=/tmp/rdt256-batteries/local/lib ./benchmarks/testu01_rdt_drbg_v2 small
```

TestU01 Crush:

```bash
DYLD_LIBRARY_PATH=/tmp/rdt256-batteries/local/lib ./benchmarks/testu01_rdt_drbg_v2 crush
```

TestU01 BigCrush:

```bash
DYLD_LIBRARY_PATH=/tmp/rdt256-batteries/local/lib ./benchmarks/testu01_rdt_drbg_v2 big
```

Or through the repo wrapper:

```bash
python3 benchmarks/run_external_batteries.py \
  --dieharder-bin /tmp/rdt256-batteries/local/bin/dieharder \
  --practrand-bin /tmp/rdt256-batteries/PractRand/RNG_test \
  --testu01-prefix /tmp/rdt256-batteries/local \
  --testu01-battery small \
  --dieharder-timeout 5400 \
  --practrand-timeout 5400 \
  --bigcrush-timeout 5400 \
  --practrand-args 'stdin64 -tlmin 1GB -tlmax 1GB -tf 2 -te 1 -tlmaxonly -a'
```

## 8. Current Result Inventory

### 8.1 Built and locally verified in this tree

Verified commands:

```bash
make rdt_drbg_v2 test-drbg-v2-kat test-drbg-v2-system
./rdt_drbg_v2 --system rdt-cli-smoke | head -c 1024 > /dev/null
```

Observed local status:
- `rdt_drbg_v2_test: ok`
- `rdt_drbg_v2_system_test: ok`

### 8.2 Honest benchmark results

Source files:
- `results/stream_benchmark_results.json`
- `results/stream_benchmark_report.md`

Current 64 MiB benchmark snapshot:

| Generator | Throughput MiB/s | Entropy bits/byte | Monobit mean | Lag-1 corr(u64) | Quality proxy |
|---|---:|---:|---:|---:|---:|
| `rdt_prng_stream_v2` | 55.77 | 7.999997 | 0.500005 | -0.000239 | 0.000245 |
| `rdt_prng_stream_v3` | 47.44 | 7.999997 | 0.500016 | 0.000189 | 0.000205 |
| `rdt_drbg_v2` | 23.59 | 7.999997 | 0.500004 | 0.000117 | 0.000122 |
| `splitmix64` | 209.78 | 7.999997 | 0.499986 | -0.000146 | 0.000159 |
| `openssl_rand` | 353.92 | 7.999997 | 0.500042 | -0.000435 | 0.000476 |

Derived observations from the benchmark JSON:
- `rdt_prng_stream_v3` is slower than `rdt_prng_stream_v2` on this machine (`0.851x`)
- `rdt_drbg_v2` is much slower than SplitMix64 (`0.112x`)
- `rdt_drbg_v2` is much slower than `openssl rand` (`0.067x`)

Interpretation:
- the current improved DRBG path is more defensible structurally than the legacy DRBG
- it is not yet speed-competitive with conventional existing generators on this host

### 8.3 PractRand results

Authoritative logs:
- `results/external_battery_logs/practrand.log`
- `results/external_battery_logs/practrand_manual.log`

Observed outcome:
- completed through `1 gigabyte (2^30 bytes)`
- duration in JSON: about `102.72 s`
- no `FAIL`, `ERROR`, `unusual`, or `suspicious` lines were observed in the log
- visible log entries are marked `normal`

Interpretation:
- current evidence supports “PractRand clean through 1 GiB on this machine”
- it does not support broader claims beyond that tested length

### 8.4 Dieharder results

Authoritative log:
- `results/external_battery_logs/dieharder.log`

Current JSON result:
- `status: timeout`
- timeout used: `5400 s`

Observed evidence inside the saved tail:
- the saved tail consists of `PASSED` entries from later `rgb_*` tests

Interpretation:
- this is not a full completed Dieharder battery result
- it is a partial timeout snapshot with many passing lines visible near the cutoff
- no honest claim should phrase this as “full Dieharder passed”

### 8.5 TestU01 SmallCrush results

Authoritative logs:
- `results/external_battery_logs/testu01_small.log`
- `results/external_battery_logs/testu01_small_manual.log`

Observed outcomes:
- `Summary results of SmallCrush`
- `Number of statistics: 15`
- `All tests were passed`

Timing:
- wrapper-run log: `00:14:38.96`
- manual run log: `00:15:19.35`

Interpretation:
- current evidence supports “SmallCrush passed”

### 8.6 TestU01 BigCrush status

Live log:
- `results/external_battery_logs/testu01_big_manual.log`

Current runtime status at report generation:
- process `./benchmarks/testu01_rdt_drbg_v2 big` is still running
- observed elapsed time from `ps`: `07:10:54`

Current visible tail of the live log shows BigCrush progress in:
- `smultin_MultinomialOver`
- current visible parameters in the tail:
  - `N = 30`
  - `n = 20000000`
  - `d = 64`
  - `t = 7`
  - `Sparse = TRUE`

Interpretation:
- BigCrush is in progress
- there is no final BigCrush pass/fail result yet in this tree

### 8.7 `results/external_battery_results.json` caveat

The current JSON file mixes completed wrapper output with earlier schema/history.

Specifically:
- it contains valid current entries for Dieharder and PractRand
- it contains a key named `bigcrush`, but the command inside that entry is actually:
  - `benchmarks/testu01_rdt_drbg_v2 small`
- the corresponding output tail is clearly a `SmallCrush` summary

Therefore:
- treat `results/external_battery_results.json` as partially authoritative
- for TestU01 battery identity, trust the log files more than the field name in that JSON

## 9. Generated Artifact Inventory

Generated binaries currently present:
- `rdt_prng_stream`
- `rdt_prng_stream_v2`
- `rdt_prng_stream_v3`
- `rdt_drbg`
- `rdt_drbg_v2`
- `rdt_seed_extractor`
- `splitmix64_stream`
- `rdt_drbg_v2_test`
- `rdt_drbg_v2_system_test`
- `benchmarks/testu01_rdt_drbg_v2`

Generated object files currently present:
- `rdt_core.o`
- `rdt_drbg.o`
- `rdt_drbg_stream.o`
- `rdt_drbg_v2.o`
- `rdt_drbg_v2_stream.o`
- `rdt_prng_stream.o`
- `rdt_seed_extractor.o`
- `rdt_sha256.o`

Generated result files currently present:
- `results/stream_benchmark_results.json`
- `results/stream_benchmark_report.md`
- `results/seed_extractor_validation.json`
- `results/seed_extractor_validation.md`
- `results/external_battery_results.json`
- `results/external_battery_logs/dieharder.log`
- `results/external_battery_logs/practrand.log`
- `results/external_battery_logs/practrand_manual.log`
- `results/external_battery_logs/testu01_small.log`
- `results/external_battery_logs/testu01_small_manual.log`
- `results/external_battery_logs/testu01_big_manual.log`

File sizes at the time of this report:
- `results/external_battery_logs/dieharder.log`: about `6.7K`
- `results/external_battery_logs/practrand.log`: about `157K`
- `results/external_battery_logs/practrand_manual.log`: about `157K`
- `results/external_battery_logs/testu01_small.log`: about `7.9K`
- `results/external_battery_logs/testu01_small_manual.log`: about `7.9K`
- `results/external_battery_logs/testu01_big_manual.log`: about `9.3K` and growing

## 10. Reviewer Notes and Caveats

### 10.1 What is currently well-supported by artifacts in this tree

Supported:
- the repo builds locally
- `rdt_drbg_v2` includes a working NIST-derived HMAC-DRBG known-answer test
- `rdt_drbg_v2` includes a working OS-entropy init/reseed path
- `rdt_drbg_v2` can be streamed and externally tested
- `rdt_drbg_v2` passed SmallCrush in local logs
- `rdt_drbg_v2` ran clean through 1 GiB of PractRand in local logs

### 10.2 What is not currently supported strongly enough for broad claims

Not supported yet:
- a completed full Dieharder `-a` battery pass
- a completed BigCrush pass
- any claim that `rdt_drbg_v2` is faster than conventional secure generators
- any claim of formal cryptographic security or certification
- any claim that the legacy custom DRBG is equivalent to a standard DRBG

### 10.3 Exact interpretation recommended for a reviewer

The strongest technical reading of the current tree is:
- the repo contains a novel RDT-centered family of experimental generators
- the legacy PRNG/DRBG paths remain custom and research-oriented
- `rdt_drbg_v2` is the most disciplined DRBG interface in the repo because it adopts an HMAC-SHA256 state machine and standard-like instantiate/reseed/generate semantics
- the improved DRBG path now has both deterministic RDT-based convenience seeding and OS-entropy seeding
- local statistical evidence is promising but still incomplete for strongest review claims because BigCrush is still running and Dieharder timed out before a full battery conclusion

## 11. Minimal Reviewer Checklist

If a reviewer wants the shortest honest reproduction path, use:

```bash
cd /Users/stevenreid/Documents/New\ project/repos/rdt256
make clean
make all
make test-drbg-v2-kat
make test-drbg-v2-system
make benchmark-honest
python3 tests/run_results.py
python3 benchmarks/run_external_batteries.py \
  --dieharder-bin /tmp/rdt256-batteries/local/bin/dieharder \
  --practrand-bin /tmp/rdt256-batteries/PractRand/RNG_test \
  --testu01-prefix /tmp/rdt256-batteries/local \
  --testu01-battery small \
  --dieharder-timeout 5400 \
  --practrand-timeout 5400 \
  --bigcrush-timeout 5400 \
  --practrand-args 'stdin64 -tlmin 1GB -tlmax 1GB -tf 2 -te 1 -tlmaxonly -a'
```

If the reviewer wants the strongest available external evidence after that:

```bash
DYLD_LIBRARY_PATH=/tmp/rdt256-batteries/local/lib ./benchmarks/testu01_rdt_drbg_v2 big
```

and wait for BigCrush to complete.
