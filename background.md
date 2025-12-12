# Background and Research Context

This repository brings together several interconnected research threads exploring recursive depth, valuation-like structures, nonlinear entropy behavior, and depth-driven transformations. The RDT-CORE mixing function, and the experimental PRNG and DRBG built on top of it, are direct computational realizations of concepts developed across the papers included in this project. This document summarizes each manuscript, clarifies its contribution, and explains how these ideas ultimately converge into the design of the RDT Cryptographic Suite.

---

## 1. Recursive Division Tree: Foundations of Depth

**Recursive Division Tree: A Log-Log Algorithm for Integer Depth**  
DOI: 10.5281/zenodo.17487650  
Zenodo: https://zenodo.org/records/17487651

The Recursive Division Tree (RDT) introduces a fundamental structural operation: repeatedly halving integers to form a binary ancestry tree. From this operation arise:

- a recursive depth measure \(R(n)\)  
- a depth-layered hierarchical structure  
- a natural partial order (“ancestry”)  

This work establishes recursive depth as a meaningful mathematical invariant.  
**Influence on the PRNG:**  
RDT-CORE adopts a fast bitwise approximation of depth to modulate rotation schedules, mixing branches, and nonlinear perturbation.

---

## 2. Recursive-Adic Number Field: Depth as a Valuation

**The Recursive Adic Number Field: Construction, Analysis, and Recursive Depth Transforms**  
DOI: 10.5281/zenodo.17555643  
Zenodo: https://zenodo.org/records/17555644

This manuscript extends recursive depth into a valuation-like system analogous to p-adic norms. It introduces:

- a recursive-adic non-Archimedean norm  
- algebraic structures driven by depth  
- analytic interpretations of recursive complexity  

This reinterpretation elevates depth from a combinatorial quantity to an analytic one.  
**Influence on the PRNG:**  
Depth acts as a controlling parameter inside RDT-CORE, selecting rotations, influencing nonlinear layers, and shaping ARX behavior.

---

## 3. Recursive Entropy Calculus: Entropy and Resonance

**Recursive Entropy Calculus: Bounds and Resonance in Hierarchical Systems**  
DOI: 10.5281/zenodo.17862830  
Zenodo: https://zenodo.org/records/17862831

This paper analyzes entropy propagation in depth-layered structures, identifying:

- resonance ratios (e.g., 5:4)  
- recursive amplification of small perturbations  
- oscillatory behavior emerging from depth interactions  

**Influence on the PRNG:**  
The epsilon-channel in RDT-CORE is inspired by this work: it injects depth-dependent perturbations that strengthen diffusion and contribute to the observed avalanche behavior.

---

## 4. Recursive Depth Integration: Depth as a Weight

**Recursive Depth Integration: A Depth-Weighted Measure on the Integers**  
DOI: 10.5281/zenodo.17753501  
Zenodo: https://zenodo.org/records/17753502

This manuscript develops an integration theory using depth as a weighting measure. It demonstrates that depth functions behave consistently when used as modifiers in analytic transformations.

**Influence on the PRNG:**  
This guided the design of depth-modulated ARX rotations in RDT-CORE, where depth directly affects rotation amounts and mixing pathways.

---

## 5. RDT Scalar Field: Projection and Magnitude Interaction

**RDT Scalar Field**  
DOI: 10.5281/zenodo.17636826  
Zenodo: https://zenodo.org/records/17636827

This work investigates how integer projections and bit-level magnitudes interact with recursive depth. It formalizes:

- depth-based scalar projections  
- geometric interpretations of depth  
- nonlinear mappings tied to bit-field magnitude  

**Influence on the PRNG:**  
The `scalar_field()` component in RDT-CORE is built directly from concepts in this paper, introducing a second nonlinear branch complementary to depth.

---

## 6. Recursive Geometric Entropy: Shapes, Symmetry, and Perturbation

**Recursive Geometric Entropy: A Unified Framework for Information-Theoretic Shape Analysis**  
DOI: 10.5281/zenodo.17882309  
Zenodo: https://zenodo.org/records/17882310

This manuscript extends entropy considerations to recursive geometric objects and identifies:

- geometric entropy bounds  
- recursive tetrahedral interactions  
- symmetry-breaking effects  
- resonance structures across depth layers  

**Influence on the PRNG:**  
This shaped the design of recursive perturbation (epsilon-channel) within RDT-CORE and informed the DRBG’s evolving-key structure.

---

## 7. Affine Digit-Linear Transforms

**Affine Digit-Linear Transforms in Arbitrary Bases**  
DOI: 10.5281/zenodo.17783083  
Zenodo: https://zenodo.org/records/17783084

Though not central to RDT-CORE, this work explores affine transforms derived from digit representation. It contributes ideas related to:

- representation-driven mixing  
- alternate affine transform spaces  

**Influence on the PRNG:**  
Minor conceptual influence on mixing strategies and state-update structures.

---

## 8. MHD Topological Stability

**Topological Stability in Magnetohydrodynamics**  
DOI: 10.5281/zenodo.17489663  
Zenodo: https://zenodo.org/records/17489664

This work is outside the recursive-depth series but reflects a broader research interest in nonlinear systems, topological structure, and stability.

**Influence on the PRNG:**  
Indirect; contributes background understanding of complex systems but not used explicitly in RDT-CORE.

---

# How These Works Converge Into RDT-CORE, RDT-PRNG, and RDT-DRBG

Across the manuscripts, several themes repeat and reinforce one another:

1. Recursive depth is a meaningful structural invariant.  
2. Depth behaves analogously to a valuation.  
3. Recursive systems exhibit entropy amplification.  
4. Depth can serve as a functional weight in transformations.  
5. Scalar projections provide additional nonlinear structure.  
6. Recursive geometric systems naturally generate perturbations and symmetry breaking.  

These elements form the foundation of the RDT-CORE function:

- depth-based modulation  
- scalar-field perturbation  
- epsilon-channel recursive mixing  
- ARX rotation and multiplication layers  

The **RDT-PRNG** uses repeated applications of RDT-CORE on a 256-bit internal state to produce high-diffusion, statistically strong output.  
The **RDT-DRBG** extends this by evolving internal key material and periodically reseeding to explore long-term dynamical behavior.

---

# Summary

Each manuscript in this project contributes to a unified investigation of recursive structures, depth-based transformations, entropy dynamics, and nonlinear mixing. The RDT Cryptographic Suite represents a practical experimental framework built directly on insights developed throughout this research lineage. It is not intended as secure cryptographic software but as a platform for exploring recursive nonlinear transformations and their statistical behavior.
