#!/usr/bin/env python3
"""Flip the most significant bit of every byte in a file."""

import argparse
from pathlib import Path


def flip_msb(input_path: Path, output_path: Path) -> None:
    data = input_path.read_bytes()
    print(data)
    output_path.write_bytes(bytes(byte ^ 0x80 for byte in data))


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Flip the most significant bit of every byte in a file."
    )
    parser.add_argument("input", type=Path, help="input file")
    parser.add_argument("output", type=Path, help="output file")
    args = parser.parse_args()

    flip_msb(args.input, args.output)


if __name__ == "__main__":
    main()
