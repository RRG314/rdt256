#!/usr/bin/env python3
"""Run external statistical batteries against a stream generator if the tools are installed."""

from __future__ import annotations

import argparse
import json
from pathlib import Path
import shlex
import shutil
import subprocess
import time


ROOT = Path(__file__).resolve().parents[1]


def write_results(path: Path, results: dict[str, object]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(results, indent=2), encoding="utf-8")


def run_cmd(command: str, timeout_s: int, log_path: Path | None = None) -> dict[str, object]:
    t0 = time.perf_counter()
    proc = subprocess.run(
        command,
        cwd=ROOT,
        shell=True,
        executable="/bin/zsh",
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
        timeout=timeout_s,
    )
    elapsed = time.perf_counter() - t0
    if log_path is not None:
        log_path.parent.mkdir(parents=True, exist_ok=True)
        log_path.write_text(proc.stdout, encoding="utf-8", errors="replace")
    return {
        "status": "ok" if proc.returncode == 0 else "failed",
        "returncode": proc.returncode,
        "elapsed_s": elapsed,
        "command": command,
        "output_tail": "\n".join(proc.stdout.splitlines()[-40:]),
    }


def run_cmd_safe(command: str, timeout_s: int, log_path: Path | None = None) -> dict[str, object]:
    try:
        return run_cmd(command, timeout_s, log_path)
    except subprocess.TimeoutExpired as exc:
        out = exc.stdout or ""
        if isinstance(out, bytes):
            out = out.decode("utf-8", errors="replace")
        if log_path is not None:
            log_path.parent.mkdir(parents=True, exist_ok=True)
            log_path.write_text(out, encoding="utf-8", errors="replace")
        return {
            "status": "timeout",
            "returncode": None,
            "elapsed_s": timeout_s,
            "command": command,
            "output_tail": "\n".join(out.splitlines()[-40:]),
        }


def try_compile_testu01(binary: Path, testu01_prefix: Path | None) -> bool:
    include_prefix = ""
    library_prefix = ""
    rpath_prefix = ""
    if testu01_prefix is not None:
        include_prefix = f"-I{shlex.quote(str(testu01_prefix / 'include'))} "
        library_prefix = f"-L{shlex.quote(str(testu01_prefix / 'lib'))} "
        rpath_prefix = f"-Wl,-rpath,{shlex.quote(str(testu01_prefix / 'lib'))} "
    compile_cmd = (
        f"cc -O3 -std=c11 -I./src {include_prefix}"
        "benchmarks/testu01_rdt_drbg_v2.c src/rdt_core.c src/rdt_sha256.c src/rdt_drbg_v2.c "
        f"-o {shlex.quote(str(binary))} {library_prefix}{rpath_prefix}"
        "-ltestu01 -lprobdist -lmylib -lm"
    )
    result = subprocess.run(
        compile_cmd,
        cwd=ROOT,
        shell=True,
        executable="/bin/zsh",
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
    )
    return result.returncode == 0


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--generator", default="./rdt_drbg_v2")
    parser.add_argument("--dieharder-bin", default=shutil.which("dieharder"))
    parser.add_argument("--practrand-bin", default=shutil.which("RNG_test"))
    parser.add_argument("--testu01-prefix", type=Path)
    parser.add_argument("--dieharder-timeout", type=int, default=7200)
    parser.add_argument("--practrand-timeout", type=int, default=7200)
    parser.add_argument("--bigcrush-timeout", type=int, default=21600)
    parser.add_argument("--out", type=Path, default=ROOT / "results" / "external_battery_results.json")
    parser.add_argument("--practrand-args", default="stdin64")
    parser.add_argument("--testu01-battery", choices=["small", "crush", "big"], default="big")
    parser.add_argument("--skip-bigcrush", action="store_true")
    args = parser.parse_args()

    subprocess.run(["make", "rdt_drbg_v2"], cwd=ROOT, check=True)

    results: dict[str, object] = {
        "generator": args.generator,
        "generated_at": time.strftime("%Y-%m-%d %H:%M:%S"),
        "tests": {},
    }
    write_results(args.out, results)

    generator_cmd = shlex.quote(args.generator)
    log_dir = ROOT / "results" / "external_battery_logs"

    if args.dieharder_bin:
        results["tests"]["dieharder"] = run_cmd_safe(
            f"{generator_cmd} | {shlex.quote(args.dieharder_bin)} -a -g 200",
            args.dieharder_timeout,
            log_dir / "dieharder.log",
        )
    else:
        results["tests"]["dieharder"] = {"status": "skipped", "reason": "dieharder not installed"}
    write_results(args.out, results)

    if args.practrand_bin:
        results["tests"]["practrand"] = run_cmd_safe(
            f"{generator_cmd} | {shlex.quote(args.practrand_bin)} {args.practrand_args}",
            args.practrand_timeout,
            log_dir / "practrand.log",
        )
    else:
        results["tests"]["practrand"] = {"status": "skipped", "reason": "RNG_test not installed"}
    write_results(args.out, results)

    testu01_bin = ROOT / "benchmarks" / "testu01_rdt_drbg_v2"
    testu01_result: dict[str, object]
    if args.skip_bigcrush:
        testu01_result = {"status": "skipped", "reason": "disabled by flag"}
    elif testu01_bin.exists() or try_compile_testu01(testu01_bin, args.testu01_prefix):
        testu01_result = run_cmd_safe(
            f"{shlex.quote(str(testu01_bin))} {args.testu01_battery}",
            args.bigcrush_timeout,
            log_dir / f"testu01_{args.testu01_battery}.log",
        )
    else:
        testu01_result = {
            "status": "skipped",
            "reason": "TestU01 development libraries not installed",
        }
    testu01_result["battery"] = args.testu01_battery
    results["tests"]["testu01"] = testu01_result

    write_results(args.out, results)
    print(f"Wrote JSON: {args.out}")


if __name__ == "__main__":
    main()
