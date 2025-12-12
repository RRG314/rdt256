# RDT-CORE: Mixing Primitive Specification

## Introduction

RDT-CORE is a 64-bit nonlinear transform combining layered operations to produce high-diffusion output. It is the foundational component for the PRNG and DRBG in this repository. The design emphasizes structure, determinism, diffusion, and testability.

## Notation

Let:

- x be a 64-bit input word
- K[4] be a 256-bit key array
- rotl(x, r) denote 64-bit left rotation

The transformation is defined as a composition of four nonlinear layers followed by an output transformation.

## 1. Depth Function

The depth function measures three structural properties of x:

1. Bit-length
2. Population count
3. A middle-bit extraction

These are combined as:

bl  = bit_length(x)
pc  = popcount(x)
mid = (bl > 0) ? (x >> floor(bl/2)) : 0
depth = (bl ^ (pc << 1) ^ mid) & 63

The depth value is a 6-bit integer used to parameterize all downstream mixing.

## 2. Scalar-Field Projection

A secondary projection is computed from the lower-order subfields of x:

a = x & 0xFFFF
b = (x >> 16) & 0xFFFF
t = isqrt(a*a + b*b)
g = depth(t)

The projection g introduces additional nonlinear dependencies and magnifies small input variations.

## 3. Epsilon Channel

Define a small fixed prime set:

P = {3, 5, 7, 11, 13, 17, 19}

The epsilon channel applies iterative perturbations:

eps = 0
rounds = min(depth, 6)

for i = 0 to rounds:
    c = x * (P[i] * constant)
    c ^= (x >> (i+1)) * P[i]
    c ^= K[i mod 4]
    c = rotl(c, 13 + 7*i)
    eps ^= c

This channel produces controlled divergence tied to both depth and the key.

## 4. ARX Depth Mixer

The ARX mixer incorporates multiplication and rotation schedules:

ROT = {13, 23, 43}
MUL = {19, 29, 47}

rp = ROT[depth mod 3]
mp = MUL[depth mod 3]

z  = x
z ^= (z << rp)
z ^= (z >> (rp >> 1))
z *= (mp * constant)
z ^= K[(depth ^ rp) & 3]
output = rotl(z, (depth ^ rp))

This ensures high nonlinearity, broad bit diffusion, and strong avalanche behavior.

## 5. Full Mixing Function

The complete exported RDT-CORE function is:

m = (x + g) * constant1 + constant2
p = m ^ (depth * constant3)
e = epsilon(p, depth, K)
h = arx(p ^ e, depth, K)
return h

This structure ensures every output bit depends in a nonlinear way on the entire input word.

## Structural Properties

1. All layers depend on depth, which depends on the global structure of x.
2. No inversion or algebraic shortcuts are known.
3. Bit-differences propagate rapidly, producing near-ideal avalanche.
4. The mapping is deterministic and branchless, ensuring consistent behavior across platforms.
5. The epsilon channel introduces key-dependent perturbations without overwhelming diffusion.

## Empirical Behavior

Testing on tens of millions of outputs demonstrates:

- Avalanche mean ≈ 32 bits flipped per round
- Stable bit-flip probability across positions
- Low autocorrelation at all lags
- Flat FFT spectrum without identifiable periodicity
- Strong performance on Shannon entropy and chi-square tests
- Differential trail bias consistent with idealized random mappings

The primitive shows stable behavior across seed sweeps and fixed-round experiments.

## Security Notes

RDT-CORE is not a cipher and is not intended to be used directly for encryption. It is a mixing primitive suitable for constructing PRNG and DRBG mechanisms. No formal security proofs are provided. All evaluation is empirical.

## Summary

RDT-CORE is a compact 64-bit nonlinear transformation designed for pseudorandom generation and state evolution. Its layered structure provides high diffusion and stable statistical behavior, forming the basis of the RDT-PRNG and RDT-DRBG components.
