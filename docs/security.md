# Security Considerations

## Scope

This repository includes the following components:

- RDT-CORE: mixing primitive
- RDT-PRNG: deterministic pseudorandom generator
- RDT-DRBG: deterministic random bit generator with reseeding

No encryption, MAC, hash, KDF, or authenticated components are included.  
The system is intended for research and experimentation, not for production cryptographic deployment.

---

## Assumptions

The system is designed under the following assumptions:

1. Deterministic, reproducible execution is required.
2. No side-channel protection is provided.
3. Seeds and keys are managed externally and securely.
4. The components will be evaluated under empirical, not formal, security analysis.
5. Users understand that this is not a standardized cryptographic primitive.

---

## Evaluation Approach

Security evaluation to date focuses exclusively on structural and statistical properties.  
The following tests have been performed on large output sets:

- Avalanche behavior (single-round and multi-round)
- Differential trail scanning
- Bitwise flip distributions
- Seed-sweep avalanche uniformity
- Shannon entropy measurement
- Byte-level chi-square
- Serial chi-square for 16-bit symbol pairs
- Runs test
- Poker test
- Autocorrelation at multiple lags
- FFT spectral analysis
- Maurer universal test (compressibility)

These tests are intended to evaluate:

- diffusion quality
- structural unpredictability
- distribution uniformity
- absence of low-weight linear structure
- empirical robustness across seeds

---

## Known Limitations

1. No formal cryptographic security proofs are provided.
2. The DRBG design does not follow a specific published standard (e.g., NIST SP 800-90A).
3. No protection against timing attacks, power analysis, or other side-channel leakage.
4. The mixing primitive has not undergone peer-reviewed cryptanalysis.
5. The PRNG is not designed to be cryptographically secure.
6. The DRBG, while structured, is experimental and not validated for production use.
7. All randomness is deterministic; external entropy sources must be managed separately.

---

## Forward and Backward Secrecy in DRBG

The DRBG includes basic secrecy enhancements:

- Key evolution after each output contributes to forward secrecy.
- State obfuscation through repeated mixing contributes to backward secrecy.
- Reseeding mixes in external seed material to prevent long-term structural drift.

These mechanisms reduce recoverability of future or past outputs from exposed state but do not constitute formal cryptographic guarantees.

---

## Recommended Usage

The components are appropriate for:

- algorithmic and mathematical research
- studying recursive-depth mixing structures
- statistical randomness experimentation
- controlled and reproducible pseudorandom generation

They are not recommended for:

- key generation
- secure communications
- cryptographic protocol deployment
- systems requiring certified randomness sources

---

## Summary

The RDT suite provides a well-structured environment for studying recursive-depth-based nonlinearity, diffusion properties, and deterministic randomness.  
The components exhibit strong empirical performance but require substantial further analysis before being considered candidates for cryptographic applications.
