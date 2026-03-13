#!/usr/bin/env python3
"""Validate and benchmark the seed extractor using stable local fixtures."""

from __future__ import annotations

import json
from pathlib import Path
import statistics
import subprocess
import tempfile
import time


ROOT = Path(__file__).resolve().parents[1]
EXAMPLE = ROOT / "examples" / "sensor_data.csv"
EXAMPLE_REL = EXAMPLE.relative_to(ROOT).as_posix()
EXPECTED_SINGLE = "f2cb746be11d12125cacd7d2b3e09a9dd7f9775dde8e12edfdc0391a920b1e8e"
EXPECTED_DOUBLE = "654a9d34e91286afe2203ee6eddfd108021bd966046f737bea5b6f5b94af60a1"


def run_seed(args: list[str], binary: bool = False) -> bytes | str:
    result = subprocess.run(
        ["./rdt_seed_extractor", *args],
        cwd=ROOT,
        check=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )
    return result.stdout if binary else result.stdout.decode().strip()


def popcount_bytes(blob: bytes) -> int:
    return sum(byte.bit_count() for byte in blob)


def xor_bytes(a: bytes, b: bytes) -> bytes:
    return bytes(x ^ y for x, y in zip(a, b))


def measure_cli(sample_runs: int) -> tuple[list[float], str]:
    timings = []
    last = ""
    for _ in range(sample_runs):
        t0 = time.perf_counter()
        last = run_seed([EXAMPLE_REL])
        timings.append(time.perf_counter() - t0)
    return timings, last


def main() -> None:
    subprocess.run(["make", "rdt_seed_extractor"], cwd=ROOT, check=True)

    baseline = run_seed([EXAMPLE_REL])
    if baseline != EXPECTED_SINGLE:
        raise SystemExit(f"unexpected baseline seed: {baseline}")

    duplicate = run_seed([EXAMPLE_REL, EXAMPLE_REL])
    if duplicate != EXPECTED_DOUBLE:
        raise SystemExit(f"unexpected duplicate-file seed: {duplicate}")

    binary_seed = run_seed(["-b", EXAMPLE_REL], binary=True)
    if len(binary_seed) != 32:
        raise SystemExit(f"binary output length mismatch: {len(binary_seed)}")
    if binary_seed.hex() != EXPECTED_SINGLE:
        raise SystemExit("binary output mismatch")

    repeated = [run_seed([EXAMPLE_REL]) for _ in range(8)]
    if len(set(repeated)) != 1:
        raise SystemExit("seed extraction is not deterministic across repeated runs")

    source = EXAMPLE.read_bytes()
    positions = sorted({0, len(source) // 5, (2 * len(source)) // 5, (3 * len(source)) // 5, (4 * len(source)) // 5, len(source) - 1})
    avalanche_counts = []
    unique_mutated = set()

    for pos in positions:
        mutated = bytearray(source)
        mutated[pos] ^= 0x01
        with tempfile.NamedTemporaryFile(dir=ROOT, suffix=".bin", delete=False) as tmp:
            tmp.write(mutated)
            tmp_path = Path(tmp.name)
        try:
            mutated_seed = run_seed([tmp_path.relative_to(ROOT).as_posix()])
        finally:
            tmp_path.unlink(missing_ok=True)
        unique_mutated.add(mutated_seed)
        avalanche_counts.append(popcount_bytes(xor_bytes(bytes.fromhex(baseline), bytes.fromhex(mutated_seed))))

    timings, last = measure_cli(25)
    if last != EXPECTED_SINGLE:
        raise SystemExit("timed run produced unstable output")

    avg_s = statistics.mean(timings)
    median_s = statistics.median(timings)
    throughput_mib_s = (EXAMPLE.stat().st_size / (1024.0 * 1024.0)) / max(avg_s, 1e-9)

    result = {
        "fixture": str(EXAMPLE.relative_to(ROOT)),
        "fixture_bytes": EXAMPLE.stat().st_size,
        "single_file_seed_hex": baseline,
        "double_file_seed_hex": duplicate,
        "deterministic_runs": len(repeated),
        "average_cli_s": avg_s,
        "median_cli_s": median_s,
        "throughput_mib_s": throughput_mib_s,
        "avalanche_positions": positions,
        "avalanche_bit_flips": avalanche_counts,
        "avalanche_mean_bits": statistics.mean(avalanche_counts),
        "avalanche_min_bits": min(avalanche_counts),
        "avalanche_max_bits": max(avalanche_counts),
        "unique_mutated_outputs": len(unique_mutated),
    }

    out_json = ROOT / "results" / "seed_extractor_validation.json"
    out_md = ROOT / "results" / "seed_extractor_validation.md"
    out_json.parent.mkdir(parents=True, exist_ok=True)
    out_json.write_text(json.dumps(result, indent=2), encoding="utf-8")

    lines = [
        "# Seed Extractor Validation",
        "",
        f"- fixture: `{result['fixture']}`",
        f"- fixture bytes: `{result['fixture_bytes']}`",
        f"- single-file seed: `{baseline}`",
        f"- double-file seed: `{duplicate}`",
        f"- deterministic repeated runs: `{result['deterministic_runs']}`",
        f"- average CLI time: `{avg_s:.6f} s`",
        f"- median CLI time: `{median_s:.6f} s`",
        f"- throughput: `{throughput_mib_s:.3f} MiB/s`",
        f"- avalanche mean: `{result['avalanche_mean_bits']:.2f} / 256 bits`",
        f"- avalanche range: `{result['avalanche_min_bits']}` to `{result['avalanche_max_bits']}` bits",
        f"- unique mutated outputs: `{result['unique_mutated_outputs']}` / `{len(positions)}`",
        "",
        "## Avalanche Samples",
        "",
        "| byte position | flipped bits in output seed |",
        "|---|---:|",
    ]
    for pos, count in zip(positions, avalanche_counts):
        lines.append(f"| {pos} | {count} |")
    out_md.write_text("\n".join(lines) + "\n", encoding="utf-8")

    print(f"Wrote JSON: {out_json}")
    print(f"Wrote report: {out_md}")


if __name__ == "__main__":
    main()
