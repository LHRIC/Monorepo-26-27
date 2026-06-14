#!/bin/python3

import datetime
import json
import sys
import argparse

from typing import Dict, Any
from cantools.database import Database, load_file, UnsupportedDatabaseFormatError


def parse_args():
    parser = argparse.ArgumentParser(description="Convert a DBC file to JSON")
    parser.add_argument("dbc_input_path", help="Path to the DBC file to convert")
    parser.add_argument("json_output_path", help="Path to the JSON file to output")
    return parser.parse_args()


args = parse_args()
DBC_INPUT_PATH = args.dbc_input_path
JSON_OUTPUT_PATH = args.json_output_path


def main():
    try:
        db: Database = load_file(DBC_INPUT_PATH)
    except UnsupportedDatabaseFormatError as e:
        error_msg = str(e)
        if error_msg.startswith("DBC: "):
            specific_error = error_msg[5:].strip('"')
            print(
                f"Error parsing DBC file '{DBC_INPUT_PATH}': {specific_error}",
                file=sys.stderr,
            )
        else:
            print(
                f"Error: Unsupported database format for file '{DBC_INPUT_PATH}'.",
                file=sys.stderr,
            )
        return 1
    except Exception as e:
        print(
            f"Error reading DBC file '{DBC_INPUT_PATH}': {e}",
            file=sys.stderr,
        )
        return 1

    # Create a dictionary to store all messages
    # Initialize with version header
    messages_dict: Dict[str, Any] = {
        "version": {
            "datetime": datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S"),
            "uuid": db.version,
        },
        "messages": [],
    }

    # Iterate through all messages in the database and save to json
    for message in db.messages:
        message_dict: Dict[str, Any] = {}

        message_dict["message_name"] = message.name
        message_dict["frame_id"] = message.frame_id
        message_dict["length"] = message.length
        message_dict["signals"] = []

        for signal in message.signals:
            signal_dict: Dict[str, Any] = {}

            # General Signal info
            signal_dict["signal_name"] = signal.name
            signal_dict["start_bit"] = signal.start
            signal_dict["length"] = signal.length
            signal_dict["is_big_endian"] = signal.byte_order == "big_endian"
            signal_dict["is_signed"] = signal.is_signed

            # Conversion
            signal_dict["scale"] = signal.conversion.scale
            signal_dict["offset"] = signal.conversion.offset

            # Multiplex Related
            signal_dict["is_multiplexer"] = signal.is_multiplexer
            signal_dict["multiplexer_ids"] = signal.multiplexer_ids
            signal_dict["multiplexer_signal"] = signal.multiplexer_signal

            message_dict["signals"].append(signal_dict)

        messages_dict["messages"].append(message_dict)

    with open(JSON_OUTPUT_PATH, "w") as json_file:
        json.dump(messages_dict, json_file)

    return 0


if __name__ == "__main__":
    sys.exit(main())
