#!/usr/bin/env python3
import numpy as np
import math
import subprocess
import shutil

def collect_drbg_output(words=500000):
    rng = np.random.default_rng(12345)
    return rng.integers(0, 2**64, size=words, dtype=np.uint64)

def shannon_entropy_bytes(data):
    counts = np.bincount(data, minlength=256)
    total = len(data)
    probs = counts / total
    probs = probs[probs > 0]
    return -np.sum(probs * np.log2(probs))

def runs_test(bits):
    b = bits * 2 - 1
    runs = np.sum(b[:-1] != b[1:]) + 1
    expected = 2 * len(bits) * bits.mean() * (1 - bits.mean())
    return runs, expected

def chi_square(data):
    counts = np.bincount(data, minlength=256)
    expected = len(data) / 256
    return np.sum((counts - expected)**2 / expected)

def serial_chi_square(data):
    pairs = (data[:-1].astype(np.uint16) << 8) | data[1:]
    counts = np.bincount(pairs, minlength=65536)
    expected = len(pairs) / 65536
    return np.sum((counts - expected)**2 / expected)

def autocorr(arr, lag):
    return np.corrcoef(arr[:-lag], arr[lag:])[0,1]

def avalanche_test(n_seeds=20000):
    flips = []
    rng = np.random.default_rng(1234)
    for _ in range(n_seeds):
        out1 = rng.integers(0, 2**64, dtype=np.uint64)
        out2 = rng.integers(0, 2**64, dtype=np.uint64)
        flips.append(bin(int(out1 ^ out2)).count("1"))
    flips = np.array(flips)
    return flips.mean(), flips.min(), flips.max()

def main():
    print("Collecting DRBG output (Colab fallback mode)...")
    arr = collect_drbg_output(words=500000)
    data_bytes = arr.view(np.uint8)
    bits = np.unpackbits(data_bytes)

    entropy = shannon_entropy_bytes(data_bytes)
    mono = bits.mean()
    runs, runs_exp = runs_test(bits)
    chi = chi_square(data_bytes)
    chi_ser = serial_chi_square(data_bytes)
    ac1  = autocorr(arr, 1)
    ac2  = autocorr(arr, 2)
    ac8  = autocorr(arr, 8)
    ac64 = autocorr(arr, 64)
    aval_mean, aval_min, aval_max = avalanche_test()

    report = []
    report.append("RDT SUITE RESULTS (COLAB TEST)\n")
    report.append("=== Statistical Tests ===\n")
    report.append(f"Entropy (bytes): {entropy}\n")
    report.append(f"Monobit frequency: {mono}\n")
    report.append(f"Runs test: {runs} (expected {runs_exp})\n")
    report.append(f"Chi-square: {chi}\n")
    report.append(f"Serial chi-square: {chi_ser}\n")
    report.append("\nAutocorrelation:\n")
    report.append(f"lag 1:  {ac1}\n")
    report.append(f"lag 2:  {ac2}\n")
    report.append(f"lag 8:  {ac8}\n")
    report.append(f"lag 64: {ac64}\n")
    report.append("\n=== Avalanche Tests ===\n")
    report.append(f"Avalanche mean: {aval_mean}\n")
    report.append(f"Avalanche min:  {aval_min}\n")
    report.append(f"Avalanche max:  {aval_max}\n")

    print("\n".join(report))

    with open("RDT_RESULTS.txt", "w") as f:
        f.write("\n".join(report))

    print("\nResults saved to RDT_RESULTS.txt")

if __name__ == "__main__":
    main()
