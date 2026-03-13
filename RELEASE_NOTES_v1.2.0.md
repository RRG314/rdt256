# RDT256 v1.2.0

RDT256 is an experimental randomness research suite built around the RDT mixing principle. This release packages the current maintained generator paths, adds reproducible validation entry points, and upgrades the repo so reviewers can build, test, and inspect it with less guesswork.

## What is in the repo

- `rdt_core`: the base nonlinear 64-bit RDT mixing primitive
- `rdt_prng_stream`, `rdt_prng_stream_v2`, `rdt_prng_stream_v3`: stream generators built around the RDT state transition
- `rdt_drbg`: the original custom experimental DRBG
- `rdt_drbg_v2`: a more disciplined HMAC-SHA256 DRBG-style path with RDT-conditioned deterministic seeding and OS-entropy-backed seeding
- `rdt_seed_extractor`: a deterministic seed-conditioning pipeline for file and sensor-style inputs

## Release highlights

- Added a NIST-derived known-answer test for `rdt_drbg_v2`
- Added system-entropy init/reseed coverage for `rdt_drbg_v2`
- Added direct seed extractor API tests plus fixture-based validation output
- Added honest benchmark and external battery reporting paths
- Added reviewer-facing inventory and reproduction documentation
- Added GitHub Actions CI and npm wrapper scripts

## Current evidence

- `rdt_drbg_v2` known-answer test passes locally
- `rdt_drbg_v2` system-init smoke test passes locally
- seed extractor API test passes locally
- seed extractor fixture validation report is generated in `results/seed_extractor_validation.md`
- current honest stream benchmark report is generated in `results/stream_benchmark_report.md`
- current external battery summary is recorded in `results/external_battery_results.json`

## Important limitations

- This release is still experimental research code, not production cryptography
- No formal cryptographic proof or certification is claimed
- Speed is not yet competitive with conventional optimized secure RNG implementations on the tested host
- The strongest current DRBG path is `rdt_drbg_v2`, but it should still be treated as an experimental module
