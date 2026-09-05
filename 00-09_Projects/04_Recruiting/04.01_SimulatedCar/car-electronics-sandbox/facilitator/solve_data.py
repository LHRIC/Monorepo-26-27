"""FACILITATOR REFERENCE SOLUTION. Run locally, never give it to recruits first.

Usage: python facilitator/solve_data.py corrupted.txt recovered.csv
"""
import argparse
from pathlib import Path


def recover(raw):
    values = [int(token) for token in raw.split()]
    if not values or any(not 128 <= value <= 255 for value in values):
        raise ValueError("Expected nonempty, high-bit-corrupted decimal bytes")
    return bytes(value ^ 0x80 for value in values).decode("ascii")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("input", type=Path)
    parser.add_argument("output", type=Path)
    args = parser.parse_args()
    args.output.write_text(recover(args.input.read_text()), encoding="ascii")
    print(f"Recovered text written to {args.output}")


if __name__ == "__main__":
    main()
