#!/usr/bin/env python3
"""Sweep SNR values and shortening counts with the two-pass strategy."""

from pathlib import Path
import subprocess
import sys


ROOT = Path(__file__).resolve().parent

# Edit these arrays to select the sweep points.
SNR_VALUES = [-6.9]
SHORTENING_COUNTS = [64]

# Shared simulation parameters.
FRAMES = 10_000
GF = 64
N = 256
METRIC = "entropy"
THREADS = 4
BUILD_JOBS = 4


def main() -> int:
    total = len(SNR_VALUES) * len(SHORTENING_COUNTS)
    run_number = 0

    for snr in SNR_VALUES:
        for shortening_count in SHORTENING_COUNTS:
            run_number += 1
            command = [
                sys.executable,
                str(ROOT / "run_sim.py"),
                str(FRAMES),
                str(snr),
                str(GF),
                str(N),
                METRIC,
                "--strategy",
                "non-iterative",
                "--max-shortening",
                str(shortening_count),
                "--threads",
                str(THREADS),
                "--jobs",
                str(BUILD_JOBS),
            ]

            print(
                f"\n[{run_number}/{total}] SNR={snr}, S={shortening_count}",
                flush=True,
            )
            print("Running:", " ".join(command), flush=True)
            subprocess.run(command, cwd=ROOT, check=True)

    print(f"\nCompleted all {total} simulations.", flush=True)
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except subprocess.CalledProcessError as error:
        print(
            f"Sweep stopped because a simulation exited with code "
            f"{error.returncode}.",
            file=sys.stderr,
        )
        raise SystemExit(error.returncode)
