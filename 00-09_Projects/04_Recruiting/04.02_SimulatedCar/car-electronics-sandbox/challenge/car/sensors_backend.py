#!/usr/bin/env python3
"""
==================================================================
   Sensor Board Sim (Fluffy Python Stand-in for Real Sensor PCBs)
==================================================================

Purpose
-------
Sensor boards are the small, distributed PCBs scattered around the
car that turn a physical quantity (wheel rotation, brake line
pressure, suspension travel, tire temperature, etc.) into a number
the rest of the car can use -- usually by putting it on the CAN
bus for things like a Smart PDM or a CarDisplay cluster to consume.

This script is a fluffy re-creation of that pipeline. Nothing here
is attached to a real sensing element, a real ADC, or a real CAN
transceiver. It just makes up plausible numbers and prints them.

Core Architecture
------------------
Every sensor board here goes through the same four fluffy stages,
mirroring a real board's pipeline:

    1. Sensing Element     -- SensingElement   (the actual transducer)
    2. Signal Conditioning  -- SignalConditioner (amplify/filter/bias)
    3. ADC + Digital Filter -- Adc              (analog -> digital, smoothed)
    4. CAN Packing/TX       -- CanTransmitter    (digital value -> CAN frame)

Block Diagram (fluffy ASCII edition)
-------------------------------------

    Physical quantity (rotation, pressure, travel, temp, ...)
        |
        v
    +--------------------+
    |  SensingElement      |  -- transducer, outputs a raw analog-ish signal
    +--------------------+
        |
        v
    +--------------------+
    |  SignalConditioner   |  -- amplifies / filters / biases the raw signal
    +--------------------+
        |
        v
    +--------------------+
    |  Adc                 |  -- samples + digitizes + light smoothing
    +--------------------+
        |
        v
    +--------------------+
    |  CanTransmitter       |  -- packs value into a CAN frame, "sends" it
    +--------------------+
        |
        v
    CAN Bus (consumed by CarDisplay, Smart PDM, whatever else is listening)

None of the arrows above carry anything real. It's dataclasses and
random.uniform() all the way down.
"""

import time
import random
from dataclasses import dataclass, field
from enum import Enum


# ==================================================================
# Shared "hardware" constants (fluffy stand-ins, not real specs)
# ==================================================================

ADC_BITS = 12
ADC_MAX_COUNTS = (2 ** ADC_BITS) - 1
CAN_BUS_NOMINAL_KBPS = 500


class SensorKind(Enum):
    WHEEL_SPEED = "wheel_speed"
    BRAKE_PRESSURE = "brake_pressure"
    SUSPENSION_TRAVEL = "suspension_travel"
    TIRE_TEMP = "tire_temp"


# ==================================================================
# SensingElement -- fluffy stand-in for the actual transducer
# (hall-effect tooth counter, pressure diaphragm, LVDT, thermistor,
# whatever the real board actually uses). Measures nothing.
# ==================================================================

@dataclass
class SensingElement:
    kind: SensorKind
    baseline: float
    noise: float = 0.05
    units: str = ""

    def raw_signal(self) -> float:
        """
        Pretends to produce a raw analog-ish reading. In a real
        board this would be volts off a bridge, a pulse train off a
        hall sensor, etc. Here it's a baseline plus wobble.
        """
        return max(0.0, self.baseline + random.uniform(-self.noise, self.noise) * self.baseline)


# ==================================================================
# SignalConditioner -- fluffy stand-in for the analog front end:
# amplification, filtering, level shifting. Does none of that.
# ==================================================================

class SignalConditioner:
    """
    Named after the analog front-end stage that takes a raw,
    often tiny or noisy signal and makes it usable by an ADC --
    gain, offset, and a low-pass filter, typically. This version
    just nudges a number a little and calls it 'conditioned.'
    """

    def __init__(self, gain: float = 1.0, offset: float = 0.0):
        self.gain = gain
        self.offset = offset

    def condition(self, raw_value: float) -> float:
        return raw_value * self.gain + self.offset


# ==================================================================
# Adc -- fluffy stand-in for the analog-to-digital conversion +
# light smoothing stage. No real sampling clock, no real quantizing
# noise, just a rounded number with a exponential-moving-average
# for flavor.
# ==================================================================

class Adc:
    def __init__(self, full_scale: float, bits: int = ADC_BITS, smoothing: float = 0.3):
        self.full_scale = full_scale
        self.bits = bits
        self.smoothing = smoothing
        self._filtered_counts = 0

    def sample(self, conditioned_value: float) -> int:
        """
        Pretends to digitize conditioned_value into an N-bit count,
        then applies a fluffy exponential smoothing filter, same
        shape as a real firmware's noise-reduction step.
        """
        clamped = max(0.0, min(conditioned_value, self.full_scale))
        raw_counts = int((clamped / self.full_scale) * ADC_MAX_COUNTS)
        self._filtered_counts = int(
            self.smoothing * raw_counts + (1 - self.smoothing) * self._filtered_counts
        )
        return self._filtered_counts

    def counts_to_engineering_units(self, counts: int) -> float:
        return (counts / ADC_MAX_COUNTS) * self.full_scale


# ==================================================================
# CanTransmitter -- fluffy stand-in for packing a value into a CAN
# frame and putting it on the bus. No real arbitration, no real
# bit-stuffing, no real transceiver.
# ==================================================================

@dataclass
class CanFrame:
    can_id: int
    signal_name: str
    value: float
    units: str
    tx_count: int


class CanTransmitter:
    def __init__(self, can_id: int, signal_name: str, units: str):
        self.can_id = can_id
        self.signal_name = signal_name
        self.units = units
        self.tx_count = 0

    def transmit(self, engineering_value: float) -> CanFrame:
        """
        Pretends to build and send a CAN frame. Nothing goes out on
        a wire; a dataclass just gets constructed and handed back.
        """
        self.tx_count += 1
        return CanFrame(
            can_id=self.can_id,
            signal_name=self.signal_name,
            value=engineering_value,
            units=self.units,
            tx_count=self.tx_count,
        )


# ==================================================================
# SensorBoard -- ties SensingElement + SignalConditioner + Adc +
# CanTransmitter together into one board, same grouping as one
# physical PCB living out near a wheel, a brake line, a corner of
# suspension, etc.
# ==================================================================

class SensorBoard:
    def __init__(self,
                 name: str,
                 kind: SensorKind,
                 baseline: float,
                 units: str,
                 full_scale: float,
                 can_id: int):
        self.name = name
        self.element = SensingElement(kind=kind, baseline=baseline, units=units)
        self.conditioner = SignalConditioner(gain=1.0, offset=0.0)
        self.adc = Adc(full_scale=full_scale)
        self.transmitter = CanTransmitter(can_id=can_id, signal_name=name, units=units)
        self.last_frame: CanFrame | None = None

    def service(self) -> CanFrame:
        """
        Fluffy equivalent of one pass through the real board's loop:
        sense -> condition -> digitize -> transmit. No RTOS, no
        interrupt, just a plain function call, same philosophy as
        the other fluffy modules in this family.
        """
        raw = self.element.raw_signal()
        conditioned = self.conditioner.condition(raw)
        counts = self.adc.sample(conditioned)
        engineering_value = self.adc.counts_to_engineering_units(counts)
        self.last_frame = self.transmitter.transmit(engineering_value)
        return self.last_frame


# ==================================================================
# SensorBoardNetwork -- the whole imaginary harness of boards.
# Comparable in spirit to "everything hanging off the CAN bus that
# isn't the ECU," per the CarDisplay note that CAN traffic is
# "mostly ECU though it can be anything on CAN bus."
# ==================================================================

class SensorBoardNetwork:
    def __init__(self):
        self.boards: list[SensorBoard] = []
        self._build_default_boards()

    def _build_default_boards(self):
        """Fluffy stand-in for whatever boards the real harness has."""
        specs = [
            ("Wheel Speed FL", SensorKind.WHEEL_SPEED, 45.0, "mph", 200.0, 0x200),
            ("Wheel Speed FR", SensorKind.WHEEL_SPEED, 45.0, "mph", 200.0, 0x201),
            ("Brake Pressure", SensorKind.BRAKE_PRESSURE, 300.0, "psi", 2000.0, 0x210),
            ("Suspension Travel FL", SensorKind.SUSPENSION_TRAVEL, 40.0, "mm", 100.0, 0x220),
            ("Tire Temp FL", SensorKind.TIRE_TEMP, 85.0, "C", 150.0, 0x230),
        ]
        for name, kind, baseline, units, full_scale, can_id in specs:
            self.boards.append(
                SensorBoard(name, kind, baseline, units, full_scale, can_id)
            )

    def service_all(self) -> list[CanFrame]:
        return [board.service() for board in self.boards]

    def render_status(self):
        print("=" * 62)
        print(f" Sensor Board Network  |  CAN bus (nominal): {CAN_BUS_NOMINAL_KBPS} kbps")
        print("=" * 62)
        for board in self.boards:
            f = board.last_frame
            if f is None:
                continue
            print(f" {f.signal_name:<20} | ID 0x{f.can_id:03X} | {f.value:8.2f} {f.units:<4} | tx #{f.tx_count}")
        print("=" * 62)


# ==================================================================
# Boot sequence (purely for vibes -- no real board bring-up here)
# ==================================================================

SPLASH_ART = r"""
   ____                            ____                     _
  / ___|  ___ _ __  ___  ___  _ __| __ )  ___   __ _ _ __ __| |___
  \___ \ / _ \ '_ \/ __|/ _ \| '__|  _ \ / _ \ / _` | '__/ _` / __|
   ___) |  __/ | | \__ \ (_) | |  | |_) | (_) | (_| | | | (_| \__ \
  |____/ \___|_| |_|___/\___/|_|  |____/ \___/ \__,_|_|  \__,_|___/
"""


def boot_sequence():
    print(SPLASH_ART)
    print("Sensor Board Sim -- fluffy re-creation of sense -> condition -> ADC -> CAN")
    print("No real transducers. No real ADCs. No real CAN transceivers.\n")

    steps = [
        "Pretending to power up sensing elements",
        "Pretending to bias analog front ends",
        "Configuring 12-bit ADC channels",
        "Zeroing exponential smoothing filters",
        "Bringing up CAN transmitters",
    ]
    for step in steps:
        print(f"  [ OK ] {step}")
        time.sleep(0.05)
    print()


# ==================================================================
# Entry point -- the whole "control loop" in a for-loop
# ==================================================================

def main(cycles: int = 5):
    boot_sequence()

    network = SensorBoardNetwork()

    for cycle in range(1, cycles + 1):
        print(f"--- Sensor board service pass #{cycle} ---")
        network.service_all()
        network.render_status()
        time.sleep(0.1)

    print("\nService loop complete. No physical quantity was harmed, sensed, or measured.")


if __name__ == "__main__":
    main()