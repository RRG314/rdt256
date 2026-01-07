#!/usr/bin/env python3
"""
RDT Seed Extractor - Python Wrapper
====================================
Python interface for the RDT Seed Extractor C library.

Author: Steven Reid
ORCID: 0009-0003-9132-3410
License: MIT

Usage:
    from rdt_seed_extractor import extract_seed, extract_seed_integers

    # Get 32-byte seed
    seed = extract_seed(['sensor1.csv', 'sensor2.csv'])

    # Get as 4 x uint64 for PRNG init
    s0, s1, s2, s3 = extract_seed_integers(['sensor_data.csv'])
"""

import subprocess
import sys
from typing import List, Tuple, Union
from pathlib import Path

def extract_seed(files: Union[str, List[str]], output_format: str = 'hex') -> Union[str, bytes, Tuple]:
    """
    Extract a 256-bit seed from sensor data files.

    Args:
        files: Single file path or list of file paths
        output_format: 'hex' (default), 'bytes', 'c', or 'u64'

    Returns:
        - 'hex': 64-character hex string
        - 'bytes': 32-byte bytes object
        - 'c': C array format string
        - 'u64': tuple of 4 uint64 integers

    Raises:
        RuntimeError: If seed extraction fails
        FileNotFoundError: If rdt_seed_extractor binary not found
    """
    # Find the binary
    script_dir = Path(__file__).parent
    binary_paths = [
        script_dir.parent / 'rdt_seed_extractor',  # Build in root
        script_dir / 'rdt_seed_extractor',          # Build in examples
        Path('rdt_seed_extractor'),                 # Current directory
    ]

    binary = None
    for path in binary_paths:
        if path.exists():
            binary = str(path)
            break

    if binary is None:
        raise FileNotFoundError(
            "rdt_seed_extractor binary not found. "
            "Please run 'make' in the rdt256 directory first."
        )

    # Normalize file list
    if isinstance(files, str):
        files = [files]

    # Verify files exist
    for f in files:
        if not Path(f).exists():
            raise FileNotFoundError(f"Input file not found: {f}")

    # Build command
    cmd = [binary]

    if output_format == 'bytes':
        cmd.append('-b')
    elif output_format == 'c':
        cmd.append('-c')
    elif output_format == 'u64':
        cmd.append('-u')
    # 'hex' is default, no flag needed

    cmd.extend(files)

    # Run extraction
    try:
        result = subprocess.run(
            cmd,
            capture_output=True,
            check=True
        )
    except subprocess.CalledProcessError as e:
        raise RuntimeError(f"Seed extraction failed: {e.stderr.decode()}")

    # Parse output
    if output_format == 'bytes':
        return result.stdout
    elif output_format == 'hex':
        return result.stdout.decode().strip()
    elif output_format == 'c':
        return result.stdout.decode()
    elif output_format == 'u64':
        lines = result.stdout.decode().strip().split('\n')
        values = []
        for line in lines:
            if '] 0x' in line:
                hex_val = line.split('] ')[1].strip()
                values.append(int(hex_val, 16))
        if len(values) != 4:
            raise RuntimeError(f"Expected 4 uint64 values, got {len(values)}")
        return tuple(values)

    return result.stdout.decode()


def extract_seed_integers(files: Union[str, List[str]]) -> Tuple[int, int, int, int]:
    """
    Extract a 256-bit seed as 4 x uint64 integers.

    This is the recommended format for initializing PRNGs.

    Args:
        files: Single file path or list of file paths

    Returns:
        Tuple of (seed[0], seed[1], seed[2], seed[3])

    Example:
        >>> s0, s1, s2, s3 = extract_seed_integers(['sensor.csv'])
        >>> # Use with RDT-PRNG_STREAM_v2:
        >>> rdt_prng_init256(s0, s1, s2, s3)
    """
    return extract_seed(files, output_format='u64')


def extract_seed_bytes(files: Union[str, List[str]]) -> bytes:
    """
    Extract a 256-bit seed as raw bytes.

    Args:
        files: Single file path or list of file paths

    Returns:
        32-byte bytes object
    """
    return extract_seed(files, output_format='bytes')


def extract_seed_hex(files: Union[str, List[str]]) -> str:
    """
    Extract a 256-bit seed as hexadecimal string.

    Args:
        files: Single file path or list of file paths

    Returns:
        64-character hex string
    """
    return extract_seed(files, output_format='hex')


def main():
    """Command-line interface."""
    if len(sys.argv) < 2:
        print("RDT Seed Extractor - Python Wrapper")
        print("====================================")
        print()
        print("Usage:")
        print(f"  {sys.argv[0]} <file1> [file2] ...")
        print()
        print("Example:")
        print(f"  {sys.argv[0]} sensor_data.csv")
        print(f"  {sys.argv[0]} sensor1.csv sensor2.csv sensor3.csv")
        sys.exit(1)

    files = sys.argv[1:]

    try:
        # Get seed in multiple formats
        hex_seed = extract_seed_hex(files)
        integers = extract_seed_integers(files)

        print("Extracted Seed:")
        print(f"  Hex: {hex_seed}")
        print()
        print("  C format:")
        print("  uint64_t seed[4] = {")
        for i, val in enumerate(integers):
            comma = "," if i < 3 else ""
            print(f"      0x{val:016x}ULL{comma}")
        print("  };")
        print()
        print("  Python format:")
        print(f"  seed = {integers}")

    except Exception as e:
        print(f"Error: {e}", file=sys.stderr)
        sys.exit(1)


if __name__ == '__main__':
    main()
