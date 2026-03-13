# Changelog

## v1.2.0 - 2026-03-13

### Added
- `rdt_drbg_v2` known-answer and system-entropy tests
- seed extractor API test and fixture-based validation report
- reviewer-facing repository inventory and reproduction report
- external battery runner and TestU01 wrapper for `rdt_drbg_v2`
- GitHub Actions CI workflow
- npm wrapper scripts for build/test/benchmark/report tasks

### Changed
- hardened `rdt_drbg_v2` with OS-entropy init/reseed helpers and stricter state handling
- improved seed extractor robustness for allocation and file-read failure handling
- reduced seed extractor heap churn by reusing mixer workspace buffers
- updated project documentation to point readers to canonical generated evidence

### Notes
- the repo remains experimental research code
- current local evidence supports SmallCrush and 1 GiB PractRand coverage for `rdt_drbg_v2`
- full BigCrush completion is still a longer-running external step
