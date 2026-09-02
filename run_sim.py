#!/usr/bin/env python3
"""Configure, build, and run an iterative shortening construction."""

import argparse
import os
from pathlib import Path
import shutil
import subprocess
import sys


ROOT = Path(__file__).resolve().parent


def read_cmake_cache(build_dir: Path) -> dict[str, str]:
    """Return the CMake cache entries needed to identify a compatible build."""
    cache_file = build_dir / "CMakeCache.txt"
    if not cache_file.is_file():
        return {}

    entries = {}
    for line in cache_file.read_text(encoding="utf-8").splitlines():
        if not line or line.startswith(("#", "//")) or "=" not in line:
            continue
        key_and_type, value = line.split("=", 1)
        key = key_and_type.split(":", 1)[0]
        entries[key] = value
    return entries


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


def nonnegative_int(value: str) -> int:
    number = int(value)
    if number < 0:
        raise argparse.ArgumentTypeError("must be zero or greater")
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
    parser.add_argument(
        "--strategy",
        choices=("iterative", "non-iterative"),
        default="iterative",
        help="Reliability recomputation strategy",
    )
    parser.add_argument(
        "--refresh-interval",
        type=positive_int,
        default=1,
        help=(
            "For iterative strategy, recompute reliability after this many "
            "shortening selections (default: 1)"
        ),
    )
    parser.add_argument("--threads", type=positive_int, default=4)
    parser.add_argument("--jobs", type=positive_int, default=None)
    parser.add_argument(
        "--max-shortening",
        type=nonnegative_int,
        default=None,
        help="Maximum S to construct; defaults to N-1 for the complete chain",
    )
    parser.add_argument(
        "--build-type", choices=("Release", "Debug"), default="Release"
    )
    args = parser.parse_args()

    if args.max_shortening is not None and args.max_shortening >= args.length:
        parser.error("--max-shortening must be smaller than the mother length")
    if args.strategy == "non-iterative" and not args.max_shortening:
        parser.error(
            "--strategy non-iterative requires --max-shortening S with S > 0"
        )
    if args.strategy == "non-iterative" and args.refresh_interval != 1:
        parser.error("--refresh-interval applies only to --strategy iterative")

    build_dir = ROOT / "build"
    executable = build_dir / "Sim2"
    cache = read_cmake_cache(build_dir)
    build_matches = (
        executable.is_file()
        and cache.get("CODE_N") == str(args.length)
        and cache.get("CODE_GF") == str(args.gf)
        and cache.get("CMAKE_BUILD_TYPE") == args.build_type
    )

    if build_matches:
        print(
            f"Reusing existing build: N={args.length}, GF={args.gf}, "
            f"type={args.build_type}",
            flush=True,
        )
    else:
        if build_dir.exists():
            print("Build configuration changed; removing old build directory.", flush=True)
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
        str(executable), str(args.frames), str(args.snr_db),
        str(args.gf), str(args.length), args.mode,
    ]
    simulation.extend(
        (str(args.max_shortening if args.max_shortening is not None else args.length - 1),
         args.strategy,
         str(args.refresh_interval))
    )
    print("Running:", " ".join(simulation), flush=True)
    return subprocess.run(simulation, cwd=ROOT, env=environment).returncode


if __name__ == "__main__":
    sys.exit(main())
