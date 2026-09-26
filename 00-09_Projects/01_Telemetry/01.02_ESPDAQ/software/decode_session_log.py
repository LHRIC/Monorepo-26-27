#!/usr/bin/env python3
"""
Decoder for text CAN session log files with checksum verification.
Usage: python3 decode_session_log.py <path_to_session.txt>

Text format per line:
  UNIXTIMESTAMP.MICROS,CANID,DATA\n
  
  Where:
  - UNIXTIMESTAMP: Unix timestamp in seconds (10 digits, zero-padded)
  - MICROS: Microseconds (6 digits, zero-padded)
  - CANID: hex CAN ID (no 0x prefix)
  - DATA: hex data bytes (no spaces, uppercase)
  
Example (with time sync):
  1740176039.123456,123,137C000000000000
  1740176039.125789,500,039F375FD5671EB1
  
Example (without time sync, boot time):
  0000000123.456789,123,137C000000000000
  0000000125.789012,500,039F375FD5671EB1
"""

import sys
from pathlib import Path
from datetime import datetime


def calculate_checksum(data):
    """Calculate checksum: sum of data bytes only, mod 2^64"""
    checksum = 0

    # Add all data bytes
    for byte in data:
        checksum += byte

    # Modulo 2^64
    checksum &= 0xFFFFFFFFFFFFFFFF

    return checksum


def decode_session_file(filepath):
    """Decode a text CAN session file (supports both old and new formats)"""
    print(f"Decoding: {filepath}")
    print("=" * 80)

    total_checksum = 0
    total_data_bytes = 0
    first_timestamp = None
    last_timestamp = None
    format_detected = None  # Will be "old" or "new"

    with open(filepath, 'r') as f:
        record_num = 0
        for line_num, line in enumerate(f, 1):
            line = line.strip()
            if not line:
                continue  # Skip empty lines

            try:
                parts = line.split(',')

                # Auto-detect format on first valid line
                if format_detected is None:
                    if len(parts) == 2:
                        format_detected = "old"
                        print(f"Detected OLD format (CANID,DATA)")
                    elif len(parts) == 3:
                        format_detected = "new"
                        print(f"Detected NEW format (TIMESTAMP.MICROS,CANID,DATA)")
                    else:
                        print(f"Warning: Cannot detect format at line {
                              line_num}: {line}")
                        continue

                # Parse based on detected format
                if format_detected == "new":
                    # Parse format: TIMESTAMP.MICROS,CANID,DATA
                    if len(parts) != 3:
                        print(f"Warning: Invalid format at line {
                              line_num}: {line}")
                        continue

                    timestamp_str, can_id_str, data_str = parts

                    # Parse timestamp (seconds.microseconds)
                    timestamp_parts = timestamp_str.split('.')
                    if len(timestamp_parts) != 2:
                        print(f"Warning: Invalid timestamp at line {
                              line_num}: {timestamp_str}")
                        continue

                    seconds = int(timestamp_parts[0])
                    micros = int(timestamp_parts[1])
                    timestamp = seconds + micros / 1_000_000.0

                    # Track first and last timestamps
                    if first_timestamp is None:
                        first_timestamp = timestamp
                    last_timestamp = timestamp

                else:  # old format
                    # Parse format: CANID,DATA
                    if len(parts) != 2:
                        print(f"Warning: Invalid format at line {
                              line_num}: {line}")
                        continue

                    can_id_str, data_str = parts
                    seconds = None  # No timestamp in old format
                    micros = None

                # Parse CAN ID (hex)
                can_id = int(can_id_str, 16)

                # Parse data (hex string to bytes)
                data = bytes.fromhex(data_str)
                data_len = len(data)
                total_data_bytes += data_len

                # Update running checksum (data only)
                frame_checksum = calculate_checksum(data)
                total_checksum = (
                    total_checksum + frame_checksum) & 0xFFFFFFFFFFFFFFFF

                # Print record (every 100th)
                if record_num % 100 == 0:
                    data_hex = data.hex().upper()

                    # Format output based on whether timestamp exists
                    if format_detected == "new":
                        # Format timestamp for display
                        # After Jan 1, 2000 (real time)
                        if seconds >= 946684800:
                            dt = datetime.fromtimestamp(seconds)
                            ts_display = f"{dt.strftime(
                                '%Y-%m-%d %H:%M:%S')}.{micros:06d}"
                        else:  # Boot time
                            ts_display = f"Boot+{seconds:6d}.{micros:06d}s"

                        print(f"[{record_num:6d}] {ts_display}  CAN ID: 0x{can_id:03X}  "
                              f"Len: {data_len}  Data: {data_hex}")
                    else:  # old format - no timestamp
                        print(f"[{record_num:6d}] CAN ID: 0x{can_id:03X}  "
                              f"Len: {data_len}  Data: {data_hex}")

                record_num += 1

            except ValueError as e:
                print(f"Warning: Parse error at line {line_num}: {e}")
                continue

    print("=" * 80)
    print(f"Total records: {record_num}")
    print(f"Total DATA bytes: {total_data_bytes}")
    print(f"Total checksum: 0x{total_checksum:016X} ({total_checksum})")

    if first_timestamp is not None and last_timestamp is not None:
        duration = last_timestamp - first_timestamp
        if record_num > 0 and duration > 0:
            avg_rate = record_num / duration
            print(f"\nSession duration: {duration:.3f} seconds")
            print(f"Average message rate: {avg_rate:.1f} msgs/sec")

    print("\nCompare this checksum with the sender's checksum to verify data integrity!")


def main():
    if len(sys.argv) < 2:
        print("Usage: python3 decode_session_log.py <path_to_session.txt>")
        print("\nExample:")
        print("  python3 decode_session_log.py /sdcard/logs/session_12345678.txt")
        return 1

    filepath = Path(sys.argv[1])

    if not filepath.exists():
        print(f"Error: File not found: {filepath}")
        return 1

    decode_session_file(filepath)
    return 0


if __name__ == "__main__":
    exit(main())
