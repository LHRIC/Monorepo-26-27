"""Facilitator-only utility. It is deliberately absent from the SSH command list."""
import argparse
import json
import os
from pathlib import Path


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("action", choices=["approve-jack", "status"])
    args = parser.parse_args()
    state = Path(os.environ.get("CAR_STATE", "/state"))
    if args.action == "approve-jack":
        (state / "jack-approved").write_text("Conversation confirmed by the facilitator.\n")
        print("Conversation recorded. Recruits may now complete the DAQ recovery stage.")
    else:
        path = state / "battery.json"
        battery = json.loads(path.read_text()) if path.exists() else 91
        print(f"Battery: {battery / 10:.1f} V")
        print(f"Jack conversation confirmed: {(state / 'jack-approved').exists()}")
        print(f"DAQ recovery completed at least once: {(state / 'daq-fixed').exists()}")


if __name__ == "__main__":
    main()
