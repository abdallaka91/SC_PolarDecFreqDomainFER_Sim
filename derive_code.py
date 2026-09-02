#!/usr/bin/env python3
"""Derive one (NS, K) code from a completed shortening construction."""

import argparse
from pathlib import Path


def read_metadata(path: Path) -> dict[str, str]:
    metadata = {}
    for line in path.read_text(encoding="utf-8").splitlines():
        if not line.startswith("# "):
            continue
        fields = line[2:].split(maxsplit=1)
        if len(fields) == 2:
            metadata[fields[0]] = fields[1]
    return metadata


def read_shortening_order(path: Path) -> list[int]:
    for line in path.read_text(encoding="utf-8").splitlines():
        if line.startswith("# shortening_order"):
            return [int(value) for value in line.split()[2:]]
    raise ValueError(f"No shortening order found in {path}")


def read_reliability_order(path: Path) -> list[int]:
    lines = path.read_text(encoding="utf-8").splitlines()
    order = None
    active = {}
    for line_number, line in enumerate(lines):
        if line == "# reliability_order":
            if line_number + 1 >= len(lines) or not lines[line_number + 1].startswith("#"):
                raise ValueError(f"Missing reliability order values in {path}")
            order = [int(value) for value in lines[line_number + 1][1:].split()]
        elif line and not line.startswith("#"):
            fields = line.split()
            if len(fields) != 5:
                raise ValueError(f"Invalid reliability row in {path}: {line}")
            active[int(fields[0])] = int(fields[3]) != 0

    if order is None:
        raise ValueError(f"No reliability order found in {path}")
    if set(order) != set(active):
        raise ValueError(f"Reliability order and data rows do not contain the same indices")
    return [index for index in order if active[index]]


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Derive shortening, information, and frozen sets."
    )
    parser.add_argument("construction", type=Path)
    parser.add_argument("--NS", type=int, required=True,
                        help="Desired shortened length")
    parser.add_argument("--K", type=int, required=True,
                        help="Desired information dimension")
    parser.add_argument("--output", type=Path, default=None)
    args = parser.parse_args()

    summary = args.construction / "shortening_order.txt"
    metadata = read_metadata(summary)
    mother_length = int(metadata["N"])

    if not 1 <= args.K <= args.NS <= mother_length:
        parser.error("the requested code must satisfy 1 <= K <= NS <= N")

    depth = mother_length - args.NS
    shortening_order = read_shortening_order(summary)
    if len(shortening_order) < depth:
        parser.error(
            f"construction reaches S={len(shortening_order)}, but S={depth} is required"
        )

    snapshot_name = f"reliability_S{depth:04d}_NS{args.NS:04d}.txt"
    snapshot = args.construction / snapshot_name
    if not snapshot.is_file():
        parser.error(f"missing reliability snapshot: {snapshot}")

    shortened = shortening_order[:depth]
    active_order = read_reliability_order(snapshot)
    if len(active_order) != args.NS:
        raise ValueError(
            f"snapshot contains {len(active_order)} active inputs, expected {args.NS}"
        )

    information = active_order[:args.K]
    reliability_frozen = active_order[args.K:]
    total_frozen = sorted(shortened + reliability_frozen)

    result = "\n".join(
        [
            f"N {mother_length}",
            f"NS {args.NS}",
            f"K {args.K}",
            "shortened_outputs " + " ".join(map(str, shortened)),
            "shortening_frozen_inputs " + " ".join(map(str, shortened)),
            "reliability_frozen_inputs "
            + " ".join(map(str, reliability_frozen)),
            "total_frozen_inputs " + " ".join(map(str, total_frozen)),
            "information_inputs " + " ".join(map(str, information)),
        ]
    ) + "\n"

    if args.output is None:
        print(result, end="")
    else:
        args.output.write_text(result, encoding="utf-8")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
