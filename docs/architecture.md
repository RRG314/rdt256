# RDT Cryptographic Suite Architecture

## Overview

The RDT Cryptographic Suite is built around a nonlinear 64-bit transformation called the RDT mixing primitive. This primitive combines several interacting components:

1. A bit-depth evaluation function
2. A scalar-field projection
3. A multistage epsilon-channel perturbation
4. An ARX (Add-Rotate-Xor) depth-modulated transform
5. A rotation schedule tied to both value and round structure

These components collectively produce high diffusion, strong avalanche behavior, and stable statistical properties.

The suite currently provides:

- RDT-CORE: the mixing primitive
- RDT-PRNG: a 256-bit state pseudorandom generator
- RDT-DRBG: a stateful deterministic random bit generator with reseeding

No encryption components are included in this repository.

---

## High-Level Structure

The architecture follows a layered design:

Input Word (64 bits)
       |
       v
+-----------------------------+
| Recursive Depth Layer       |
+-----------------------------+
       |
       v
+-----------------------------+
| Scalar Projection Layer     |
+-----------------------------+
       |
       v
+-----------------------------+
| Epsilon Perturbation Layer  |
+-----------------------------+
       |
       v
+-----------------------------+
| ARX Depth Mixer             |
+-----------------------------+
       |
       v
Output Word (64 bits)

Each layer contributes independent nonlinear structure. All layers depend, directly or indirectly, on the bit composition of the input word, ensuring value-dependent diffusion.

---

## Design Goals

The design requirements for the primitive and associated generators were:

- high diffusion per round
- uniform avalanche behavior
- minimal linear structure
- deterministic execution without data-dependent branching
- portability across platforms
- self-contained operation with no external dependencies

These constraints were selected to support both statistical quality and architectural clarity.

---

## Core Components

### 1. Depth Function

The depth function computes:

- bit-length  
- population count  
- a middle-bit extraction  

These are combined to form a 6-bit index.  
The depth output influences epsilon mixing, ARX parameters, and rotational offsets.

### 2. Scalar-Field Projection

A projection onto a 32-bit derived quantity produces additional structure.  
The projection is designed to amplify differences in lower bits through integer square-root interactions.

### 3. Epsilon Channel

This layer applies perturbations controlled by small primes.  
The channel is limited to a small number of rounds based on depth, creating controlled divergence without over-amplification.

### 4. ARX Depth Mixer

A three-way rotation/multiplication schedule ensures full 64-bit diffusion.  
The mixer is dependent on depth, preventing uniform rotation patterns.

### 5. RDT-CORE Integration

The final primitive is:

output = ARX( epsilon(p(x)) )

with depth and scalar-field determining all intermediate parameters.

---

## PRNG Architecture

The PRNG uses:

- a 256-bit internal state  
- four parallel calls to RDT-CORE per update  
- XOR-based state feedback  

This provides a balance between speed and diffusion.  
It is not intended as a cryptographic generator but performs well under statistical analysis.

---

## DRBG Architecture

The DRBG introduces the following:

- separate key and state arrays  
- per-generation key evolution  
- reseeding after a defined interval  
- forward secrecy  
- backward secrecy  
- deterministic reproducibility  

The DRBG is well suited for controlled deterministic randomness experiments.

---

## Summary

The architecture provides a compact, portable framework for studying recursive-depth nonlinear transformations and their use in pseudorandom generation. The components are designed for clarity, testability, and modular extension.
