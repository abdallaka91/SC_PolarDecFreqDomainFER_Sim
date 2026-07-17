#!/usr/bin/env python3
"""Configure, build, and run one non-binary polar construction simulation."""

import argparse
import os
from pathlib import Path
import shutil
import subprocess
import sys


ROOT = Path(__file__).resolve().parent


def positive_int(value: str) -> int:
    number = int(value)
    if number <= 0:
        raise argparse.ArgumentTypeError("must be greater than zero")
    return number


def power_of_two(value: str) -> int:
    number = positive_int(value)
    if number & (number - 1):
        raise argparse.ArgumentTypeError("must be a power of two")
    return number


def main() -> int:
    parser = argparse.ArgumentParser(description="Configure, build, and run Sim2.")
    parser.add_argument("frames", type=positive_int)
    parser.add_argument("snr_db", type=float)
    parser.add_argument("gf", type=power_of_two)
    parser.add_argument("length", type=power_of_two)
    parser.add_argument(
        "mode", choices=("entropy", "probability"), nargs="?", default="entropy"
    )
    parser.add_argument("--threads", type=positive_int, default=4)
    parser.add_argument("--jobs", type=positive_int, default=None)
    parser.add_argument(
        "--build-type", choices=("Release", "Debug"), default="Release"
    )
    args = parser.parse_args()

    build_dir = ROOT / "build"
    if build_dir.exists():
        shutil.rmtree(build_dir)

    configure = [
        "cmake", "-S", str(ROOT), "-B", str(build_dir),
        f"-DCMAKE_BUILD_TYPE={args.build_type}",
        f"-DCODE_N={args.length}",
        f"-DCODE_GF={args.gf}",
    ]
    subprocess.run(configure, cwd=ROOT, check=True)

    build = ["cmake", "--build", str(build_dir)]
    if args.jobs:
        build.extend(("-j", str(args.jobs)))
    subprocess.run(build, cwd=ROOT, check=True)

    environment = os.environ.copy()
    environment["OMP_NUM_THREADS"] = str(args.threads)
    simulation = [
        str(build_dir / "Sim2"), str(args.frames), str(args.snr_db),
        str(args.gf), str(args.length), args.mode,
    ]
    print("Running:", " ".join(simulation), flush=True)
    return subprocess.run(simulation, cwd=ROOT, env=environment).returncode


if __name__ == "__main__":
    sys.exit(main())
