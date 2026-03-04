#!/usr/bin/env python3
"""Internal statistical smoke test using actual RDT stream output."""

from __future__ import annotations

import subprocess
from pathlib import Path
import numpy as np


ROOT = Path(__file__).resolve().parents[1]



def collect_stream_bytes(n_bytes: int) -> bytes:
    proc = subprocess.Popen(["./rdt_prng_stream_v2"], cwd=ROOT, stdout=subprocess.PIPE, stderr=subprocess.DEVNULL)
    try:
        out = bytearray()
        assert proc.stdout is not None
        while len(out) < n_bytes:
            chunk = proc.stdout.read(min(65536, n_bytes - len(out)))
            if not chunk:
                break
            out.extend(chunk)
    finally:
        proc.kill()
        proc.wait(timeout=5)
    return bytes(out)



def shannon_entropy_bytes(data: np.ndarray) -> float:
    counts = np.bincount(data, minlength=256)
    total = len(data)
    probs = counts / max(1, total)
    probs = probs[probs > 0]
    return float(-np.sum(probs * np.log2(probs)))



def runs_test(bits: np.ndarray) -> tuple[int, float]:
    b = bits.astype(np.int8) * 2 - 1
    runs = int(np.sum(b[:-1] != b[1:]) + 1)
    p = float(bits.mean())
    expected = float(2 * len(bits) * p * (1 - p))
    return runs, expected



def chi_square(data: np.ndarray) -> float:
    counts = np.bincount(data, minlength=256)
    expected = len(data) / 256
    return float(np.sum((counts - expected) ** 2 / max(1e-12, expected)))



def serial_chi_square(data: np.ndarray) -> float:
    pairs = (data[:-1].astype(np.uint16) << 8) | data[1:]
    counts = np.bincount(pairs, minlength=65536)
    expected = len(pairs) / 65536
    return float(np.sum((counts - expected) ** 2 / max(1e-12, expected)))



def autocorr(arr: np.ndarray, lag: int) -> float:
    if lag <= 0 or lag >= len(arr):
        return 0.0
    a = arr[:-lag].astype(np.float64)
    b = arr[lag:].astype(np.float64)
    if np.std(a) == 0.0 or np.std(b) == 0.0:
        return 0.0
    return float(np.corrcoef(a, b)[0, 1])



def avalanche_test(words: np.ndarray, n_pairs: int = 20000) -> tuple[float, int, int]:
    n = min(n_pairs * 2, len(words))
    x = words[:n:2]
    y = words[1:n:2]
    if len(x) == 0 or len(y) == 0:
        return 0.0, 0, 0
    flips = np.bitwise_xor(x, y)
    counts = np.array([int(v).bit_count() for v in flips], dtype=np.int32)
    return float(np.mean(counts)), int(np.min(counts)), int(np.max(counts))



def main() -> None:
    # Build binary if needed.
    subprocess.run(["make", "rdt_prng_stream_v2"], cwd=ROOT, check=True)

    print("Collecting RDT stream output...")
    blob = collect_stream_bytes(8 * 500000)
    arr = np.frombuffer(blob, dtype=np.uint8)
    words = np.frombuffer(blob[: (len(blob) // 8) * 8], dtype=np.uint64)
    bits = np.unpackbits(arr)

    entropy = shannon_entropy_bytes(arr)
    mono = float(bits.mean())
    runs, runs_exp = runs_test(bits)
    chi = chi_square(arr)
    chi_ser = serial_chi_square(arr)
    ac1 = autocorr(words, 1)
    ac2 = autocorr(words, 2)
    ac8 = autocorr(words, 8)
    ac64 = autocorr(words, 64)
    aval_mean, aval_min, aval_max = avalanche_test(words)

    report = []
    report.append("RDT SUITE RESULTS (STREAM TEST)\n")
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

    text = "\n".join(report)
    print(text)

    out_path = ROOT / "RDT_RESULTS.txt"
    out_path.write_text(text, encoding="utf-8")
    print(f"\nResults saved to {out_path}")


if __name__ == "__main__":
    main()
