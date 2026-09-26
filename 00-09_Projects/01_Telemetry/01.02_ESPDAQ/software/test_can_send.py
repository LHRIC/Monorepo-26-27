#!/usr/bin/env python3
"""
Simple test script to send random CAN messages over socketcan.
Usage: sudo python3 test_can_send.py [kbps]

Default rate: 100 kbps (matching typical CAN bus load)
"""

import socket
import struct
import time
import random
import sys

CAN_INTERFACE = "can0"
CAN_EFF_FLAG = 0x80000000  # Extended frame format flag
CAN_RTR_FLAG = 0x40000000  # Remote transmission request
CAN_ERR_FLAG = 0x20000000  # Error frame

# CAN frame overhead: 47 bits for standard 11-bit ID, 8 bytes data
# ~144 bits total per frame at 1 Mbps
BITS_PER_FRAME = 144


def setup_can_socket(interface):
    """Create and bind a CAN socket"""
    s = socket.socket(socket.AF_CAN, socket.SOCK_RAW, socket.CAN_RAW)
    try:
        s.bind((interface,))
        print(f"Bound to {interface}")
        return s
    except OSError as e:
        print(f"Error binding to {interface}: {e}")
        print(f"Make sure {interface} is up: sudo ip link set {
              interface} up type can bitrate 1000000")
        raise


def send_can_frame(sock, can_id, data):
    """Send a CAN frame"""
    # CAN frame format: <can_id> <can_dlc> <data>
    can_dlc = len(data)
    frame = struct.pack("=IB3x8s", can_id, can_dlc, data.ljust(8, b'\x00'))
    sock.send(frame)
    return can_id, data


def calculate_checksum(data):
    """Calculate checksum: sum of data bytes only, mod 2^64"""
    checksum = 0

    # Add all data bytes
    for byte in data:
        checksum += byte

    # Modulo 2^64 (but Python ints are arbitrary precision, so mask it)
    checksum &= 0xFFFFFFFFFFFFFFFF

    return checksum


def main():
    # Parse target bus load from command line
    target_kbps = 100  # Default to 100 kbps
    if len(sys.argv) > 1:
        try:
            target_kbps = float(sys.argv[1])
        except ValueError:
            print(f"Invalid rate: {sys.argv[1]}")
            return 1

    # Calculate messages per second for target bus load
    # At 1 Mbps bit rate, each frame is ~144 bits
    # For target_kbps load, we need: (target_kbps * 1000) / 144 messages/sec
    msgs_per_sec = (target_kbps * 1000) / BITS_PER_FRAME
    interval = 1.0 / msgs_per_sec

    print("CAN Test Script - Sending random messages")
    print("=" * 50)
    print(f"Target bus load: {target_kbps} kbps")
    print(f"≈ {msgs_per_sec:.1f} msgs/sec, interval ≈ {interval*1000:.2f}ms")

    # Setup interface
    try:
        sock = setup_can_socket(CAN_INTERFACE)
    except:
        return 1

    # Use 11-bit CAN IDs (0x000 - 0x7FF range)
    # Generate list of random 11-bit CAN IDs
    can_ids = [random.randint(0x100, 0x7FF) for _ in range(20)]

    print("\nSending random data messages (Ctrl+C to stop):\n")
    print(f"Using {len(can_ids)} different CAN IDs in range 0x100-0x7FF")

    try:
        msg_count = 0
        start_time = time.time()
        total_checksum = 0
        total_data_bytes = 0

        while True:
            # Pick a random CAN ID from our list
            can_id = random.choice(can_ids)

            # Generate random 8 bytes of data
            data = bytes([random.randint(0, 255) for _ in range(8)])

            # Send the frame
            sent_id, sent_data = send_can_frame(sock, can_id, data)
            msg_count += 1
            total_data_bytes += 8  # Always 8 bytes of data

            # Update running checksum (data only)
            frame_checksum = calculate_checksum(data)
            total_checksum = (
                total_checksum + frame_checksum) & 0xFFFFFFFFFFFFFFFF

            elapsed = time.time() - start_time
            actual_rate = (msg_count * BITS_PER_FRAME / 1000.0) / elapsed
            print(f"{can_id:03X},{data.hex()}")

            # Sleep to maintain target rate
            time.sleep(interval)

    except KeyboardInterrupt:
        elapsed = time.time() - start_time
        actual_rate = (msg_count * BITS_PER_FRAME / 1000.0) / elapsed
        print(f"\n\nSent {msg_count} messages total")
        print(f"Total DATA bytes sent: {total_data_bytes}")
        print(f"Average rate: {actual_rate:.1f} kbps")
        print(f"Total checksum: 0x{total_checksum:016X} ({total_checksum})")
    finally:
        sock.close()
        print("Socket closed")

    return 0


if __name__ == "__main__":
    exit(main())
