#!/usr/bin/env python3
"""Honest stream benchmark for RDT256 vs conventional baseline generator."""

from __future__ import annotations

import argparse
import json
import math
from pathlib import Path
import shutil
import subprocess
import time

import numpy as np


ROOT = Path(__file__).resolve().parents[1]



def compile_tools() -> None:
    subprocess.run(
        ["make", "rdt_prng_stream_v2", "rdt_prng_stream_v3", "rdt_drbg_v2", "splitmix64_stream"],
        cwd=ROOT,
        check=True,
    )



def read_stream(cmd: list[str], n_bytes: int, cwd: Path) -> tuple[bytes, float]:
    t0 = time.perf_counter()
    proc = subprocess.Popen(cmd, cwd=cwd, stdout=subprocess.PIPE, stderr=subprocess.DEVNULL)
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
    elapsed = time.perf_counter() - t0
    return bytes(out), elapsed



def shannon_entropy_bytes(data: np.ndarray) -> float:
    counts = np.bincount(data, minlength=256)
    probs = counts / max(1, data.size)
    probs = probs[probs > 0]
    return float(-np.sum(probs * np.log2(probs)))



def runs_test(bits: np.ndarray) -> tuple[int, float]:
    b = bits.astype(np.int8) * 2 - 1
    runs = int(np.sum(b[:-1] != b[1:]) + 1)
    p = float(bits.mean())
    expected = float(2.0 * len(bits) * p * (1.0 - p))
    return runs, expected



def lag1_corr_u64(words: np.ndarray) -> float:
    if words.size < 3:
        return 0.0
    a = words[:-1].astype(np.float64)
    b = words[1:].astype(np.float64)
    if np.std(a) == 0.0 or np.std(b) == 0.0:
        return 0.0
    return float(np.corrcoef(a, b)[0, 1])



def evaluate_stream(name: str, cmd: list[str], n_bytes: int) -> dict[str, float | str | int]:
    blob, elapsed = read_stream(cmd, n_bytes, ROOT)
    arr = np.frombuffer(blob, dtype=np.uint8)
    words = np.frombuffer(blob[: (len(blob) // 8) * 8], dtype=np.uint64)
    bits = np.unpackbits(arr)

    entropy = shannon_entropy_bytes(arr)
    mono = float(bits.mean())
    runs, runs_exp = runs_test(bits)
    lag1 = lag1_corr_u64(words)
    throughput_mib_s = (len(blob) / (1024.0 * 1024.0)) / max(1e-9, elapsed)
    quality_proxy = abs(mono - 0.5) + abs(lag1)

    return {
        "name": name,
        "bytes": int(len(blob)),
        "elapsed_s": float(elapsed),
        "throughput_mib_s": float(throughput_mib_s),
        "entropy_bits_per_byte": float(entropy),
        "monobit_mean": mono,
        "runs": int(runs),
        "runs_expected": float(runs_exp),
        "lag1_corr_u64": float(lag1),
        "quality_proxy": float(quality_proxy),
    }



def run(sample_mib: int) -> dict[str, object]:
    compile_tools()
    n_bytes = sample_mib * 1024 * 1024

    results: list[dict[str, float | str | int]] = [
        evaluate_stream("rdt_prng_stream_v2", ["./rdt_prng_stream_v2"], n_bytes),
        evaluate_stream("rdt_prng_stream_v3", ["./rdt_prng_stream_v3"], n_bytes),
        evaluate_stream("rdt_drbg_v2", ["./rdt_drbg_v2"], n_bytes),
        evaluate_stream("splitmix64", ["./splitmix64_stream"], n_bytes),
    ]

    if shutil.which("openssl"):
        results.append(evaluate_stream("openssl_rand", ["openssl", "rand", str(n_bytes)], n_bytes))

    by_name = {r["name"]: r for r in results}
    findings = {
        "v3_speedup_vs_v2": by_name["rdt_prng_stream_v3"]["throughput_mib_s"]
        / max(1e-9, by_name["rdt_prng_stream_v2"]["throughput_mib_s"]),
        "v2_speed_ratio_vs_splitmix": by_name["rdt_prng_stream_v2"]["throughput_mib_s"]
        / max(1e-9, by_name["splitmix64"]["throughput_mib_s"]),
        "v3_speed_ratio_vs_splitmix": by_name["rdt_prng_stream_v3"]["throughput_mib_s"]
        / max(1e-9, by_name["splitmix64"]["throughput_mib_s"]),
        "drbg_v2_speed_ratio_vs_splitmix": by_name["rdt_drbg_v2"]["throughput_mib_s"]
        / max(1e-9, by_name["splitmix64"]["throughput_mib_s"]),
        "entropy_delta_v3_vs_v2": by_name["rdt_prng_stream_v3"]["entropy_bits_per_byte"]
        - by_name["rdt_prng_stream_v2"]["entropy_bits_per_byte"],
        "lag1_abs_delta_v3_vs_v2": abs(by_name["rdt_prng_stream_v3"]["lag1_corr_u64"])
        - abs(by_name["rdt_prng_stream_v2"]["lag1_corr_u64"]),
        "quality_proxy_delta_v3_vs_v2": by_name["rdt_prng_stream_v3"]["quality_proxy"]
        - by_name["rdt_prng_stream_v2"]["quality_proxy"],
        "drbg_v2_quality_proxy": by_name["rdt_drbg_v2"]["quality_proxy"],
    }
    if "openssl_rand" in by_name:
        findings["drbg_v2_speed_ratio_vs_openssl"] = (
            by_name["rdt_drbg_v2"]["throughput_mib_s"]
            / max(1e-9, by_name["openssl_rand"]["throughput_mib_s"])
        )
        findings["drbg_v2_quality_delta_vs_openssl"] = (
            by_name["rdt_drbg_v2"]["quality_proxy"]
            - by_name["openssl_rand"]["quality_proxy"]
        )

    return {
        "sample_mib": sample_mib,
        "results": results,
        "findings": findings,
    }



def to_markdown(obj: dict[str, object]) -> str:
    lines = ["# RDT256 Honest Stream Benchmark", "", f"- sample per generator: `{obj['sample_mib']} MiB`", ""]
    lines.append("| generator | throughput MiB/s | entropy | monobit | lag1 corr(u64) | quality_proxy (lower better) |")
    lines.append("|---|---:|---:|---:|---:|---:|")
    for r in obj["results"]:
        lines.append(
            f"| {r['name']} | {r['throughput_mib_s']:.2f} | {r['entropy_bits_per_byte']:.5f} | {r['monobit_mean']:.5f} | {r['lag1_corr_u64']:.6f} | {r['quality_proxy']:.6f} |"
        )

    f = obj["findings"]
    lines.append("")
    lines.append("## Findings")
    lines.append(f"- v3 speedup vs v2: `{f['v3_speedup_vs_v2']:.3f}x`")
    lines.append(f"- v2 speed ratio vs SplitMix64: `{f['v2_speed_ratio_vs_splitmix']:.3f}`")
    lines.append(f"- v3 speed ratio vs SplitMix64: `{f['v3_speed_ratio_vs_splitmix']:.3f}`")
    lines.append(f"- DRBG v2 speed ratio vs SplitMix64: `{f['drbg_v2_speed_ratio_vs_splitmix']:.3f}`")
    lines.append(f"- Entropy delta (v3-v2): `{f['entropy_delta_v3_vs_v2']:.6f}` bits/byte")
    lines.append(f"- |lag1| delta (v3-v2): `{f['lag1_abs_delta_v3_vs_v2']:.6f}`")
    lines.append(f"- quality_proxy delta (v3-v2): `{f['quality_proxy_delta_v3_vs_v2']:.6f}` (negative means v3 better)")
    lines.append(f"- DRBG v2 quality_proxy: `{f['drbg_v2_quality_proxy']:.6f}`")
    if "drbg_v2_speed_ratio_vs_openssl" in f:
        lines.append(f"- DRBG v2 speed ratio vs OpenSSL rand: `{f['drbg_v2_speed_ratio_vs_openssl']:.3f}`")
        lines.append(f"- DRBG v2 quality_proxy delta vs OpenSSL rand: `{f['drbg_v2_quality_delta_vs_openssl']:.6f}`")
    lines.append("- This is a statistical/throughput comparison only; not a security proof.")
    return "\n".join(lines) + "\n"



def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--sample-mib", type=int, default=64)
    parser.add_argument("--out", type=Path, default=ROOT / "results" / "stream_benchmark_results.json")
    parser.add_argument("--report", type=Path, default=ROOT / "results" / "stream_benchmark_report.md")
    args = parser.parse_args()

    obj = run(sample_mib=args.sample_mib)
    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.report.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_text(json.dumps(obj, indent=2), encoding="utf-8")
    args.report.write_text(to_markdown(obj), encoding="utf-8")
    print(f"Wrote JSON: {args.out}")
    print(f"Wrote report: {args.report}")


if __name__ == "__main__":
    main()
