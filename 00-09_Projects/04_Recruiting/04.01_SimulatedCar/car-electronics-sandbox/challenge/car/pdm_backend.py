#!/usr/bin/env python3
"""
==================================================================
   Smart PDM Sim (Fluffy Python Stand-in for a Real Power Module)
==================================================================

Purpose
-------
Smart Power Distribution Module (Smart PDM) manages and protects
12V power distribution within a car. It replaces mechanical fuses
and relays (Dumb PDM) with semiconductor-based switching and
logic-driven fault detection -- faster fault response, better
reliability, and integration with vehicle comms.

This script is a fluffy re-creation of that architecture in
Python. It does not sense current, does not switch anything, and
is not connected to a battery, a PFET, or a car. It just prints
things that look like the real process.

Core Architecture
------------------
The three main components, same as the real thing:

    1. Current Sensing      -- CurrentSensor
    2. Digital Logic        -- CombinationalLogic (thresholds, hw + sw)
    3. Semiconductor Switch -- PowerSwitch (the "PFET")

Block Diagram (fluffy ASCII edition)
-------------------------------------

    Battery (12V)
        |
        v
    +--------------------+
    |   PowerSwitch       |  <-- gate controlled by NFETs
    |   (PFET, simulated) |
    +--------------------+
        |
        v
    Load (rest of the car)

    Load current
        |
        v
    +--------------------+
    |   CurrentSensor      |  -- measures current draw
    +--------------------+
        |  (light blue signal, i.e. from the "microcontroller")
        v
    +--------------------+
    |  CombinationalLogic  |  -- compares against hw/sw thresholds
    +--------------------+
        |
        v
    Gate command back to PowerSwitch (open on fault)

In the event of a short: the current sensor sees it, the
combinational logic decides it's over threshold, and the PFET
gate is commanded closed to stop 12V from reaching the rest of
the car. That's the whole idea. None of this actually happens in
this file -- it's all simulated with random numbers and print().
"""

import pdm_config
import time
import random
from dataclasses import dataclass, field
from enum import Enum, auto


# ==================================================================
# Shared "hardware" constants (fluffy stand-ins, not real thresholds)
# ==================================================================

BATTERY_NOMINAL_V = 12.0
HARDWARE_TRIP_CURRENT_A = 40.0   # instant hardware-level cutoff, in theory
SOFTWARE_TRIP_CURRENT_A = 25.0   # slower, logic-driven cutoff, in theory
NUM_CHANNELS = 4                 # e.g. headlights, fuel pump, ECU, accessory


class ChannelState(Enum):
    CLOSED = auto()   # PFET conducting, 12V flowing, all normal
    OPEN_FAULT = auto()  # PFET gate commanded open due to a fault
    OPEN_MANUAL = auto()  # commanded open on purpose (not a fault)


# ==================================================================
# CurrentSensor -- fluffy stand-in for the real current sensing
# stage. Does not measure anything. Generates plausible numbers.
# ==================================================================

@dataclass
class CurrentSensor:
    channel_name: str
    baseline_current_a: float
    noise_a: float = 0.3

    def sample(self) -> float:
        """
        Pretends to sample load current. In the real PDM this would
        be an actual analog measurement (shunt, hall-effect, whatever
        the board uses). Here it's just noisy math around a baseline.
        """
        return max(0.0, self.baseline_current_a + random.uniform(-self.noise_a, self.noise_a))


# ==================================================================
# CombinationalLogic -- fluffy stand-in for the digital logic stage.
# Compares sensed current against hardware and software thresholds,
# same two-tier idea as the real thing, but with no actual gate
# drive underneath it.
# ==================================================================

@dataclass
class FaultDecision:
    hardware_trip: bool
    software_trip: bool

    @property
    def should_open(self) -> bool:
        return self.hardware_trip or self.software_trip


class CombinationalLogic:
    """
    Named 'combinational' because the real logic block is built from
    combinational gates comparing sensed current to threshold levels
    (some fixed in hardware, some configurable in software). This
    version just does two if-statements and calls it a day.
    """

    def __init__(self,
                 hw_threshold_a: float = HARDWARE_TRIP_CURRENT_A,
                 sw_threshold_a: float = SOFTWARE_TRIP_CURRENT_A):
        self.hw_threshold_a = hw_threshold_a
        self.sw_threshold_a = sw_threshold_a

    def evaluate(self, sensed_current_a: float) -> FaultDecision:
        hardware_trip = sensed_current_a >= self.hw_threshold_a
        software_trip = sensed_current_a >= self.sw_threshold_a
        return FaultDecision(hardware_trip=hardware_trip, software_trip=software_trip)


# ==================================================================
# PowerSwitch -- fluffy stand-in for the semiconductor switching
# stage (the PFET, gate-driven by NFETs in the real design). No
# actual gate charge, no actual PFET, no actual 12V.
# ==================================================================

class PowerSwitch:
    """
    In the real Smart PDM this is a PFET whose gate is pulled low/high
    by a pair of NFETs, themselves driven by the combinational logic
    and the microcontroller. Here it's a bool with delusions of
    grandeur.
    """

    def __init__(self, name: str):
        self.name = name
        self.state = ChannelState.CLOSED
        self.gate_commands_received = 0

    def command_gate(self, open_gate: bool):
        self.gate_commands_received += 1
        if open_gate:
            if self.state != ChannelState.OPEN_MANUAL:
                self.state = ChannelState.OPEN_FAULT
        else:
            self.state = ChannelState.CLOSED

    def is_conducting(self) -> bool:
        return self.state == ChannelState.CLOSED


# ==================================================================
# PdmChannel -- ties one CurrentSensor + CombinationalLogic +
# PowerSwitch together, same grouping as one "channel" on a real
# Smart PDM board (e.g. one channel per fused circuit, replacing
# one mechanical fuse/relay pair).
# ==================================================================

@dataclass
class PdmChannel:
    name: str
    sensor: CurrentSensor
    logic: CombinationalLogic
    switch: PowerSwitch
    last_current_a: float = 0.0
    last_decision: FaultDecision = field(
        default_factory=lambda: FaultDecision(False, False)
    )

    def service(self):
        """
        Fluffy equivalent of one pass through the real PDM's control
        loop for this channel: sense -> decide -> switch. No RTOS,
        no interrupt, just a plain function call.
        """
        self.last_current_a = self.sensor.sample()
        self.last_decision = self.logic.evaluate(self.last_current_a)
        self.switch.command_gate(open_gate=self.last_decision.should_open)


# ==================================================================
# SmartPdm -- the whole board. Owns all channels, runs the fluffy
# "control loop," and reports status. Comparable in spirit to the
# combined main.c + control loop of the real firmware, minus any
# of the parts that would make it real.
# ==================================================================

class SmartPdm:
    def __init__(self):
        self.channels: list[PdmChannel] = []
        self._build_default_channels()

    def _build_default_channels(self):
        """
        Fluffy stand-in for whatever channel map the real board
        actually has. Names picked for vibes, not for accuracy.
        """
        channel_specs = [
            ("Headlights", 8.0),
            ("Fuel Pump", 6.0),
            ("ECU", 2.0),
            ("Accessory", 4.0),
        ]
        for name, baseline in channel_specs[:NUM_CHANNELS]:
            sensor = CurrentSensor(channel_name=name, baseline_current_a=baseline)
            logic = CombinationalLogic()
            switch = PowerSwitch(name=name)
            self.channels.append(PdmChannel(name=name, sensor=sensor, logic=logic, switch=switch))

    def inject_fault(self, channel_name: str):
        """
        Manually shove a channel's baseline current sky-high, as a
        stand-in for 'simulate a short circuit.' Nothing electrical
        happens; a number just gets bigger.
        """
        for ch in self.channels:
            if ch.name == channel_name:
                ch.sensor.baseline_current_a = HARDWARE_TRIP_CURRENT_A + 10.0

    def service_all(self):
        for ch in self.channels:
            ch.service()

    def render_status(self):
        print("=" * 60)
        print(f" Smart PDM Status  |  Battery (nominal): {BATTERY_NOMINAL_V:.1f} V")
        print("=" * 60)
        for ch in self.channels:
            state_label = {
                ChannelState.CLOSED: "CLOSED (conducting)",
                ChannelState.OPEN_FAULT: "OPEN (fault trip)",
                ChannelState.OPEN_MANUAL: "OPEN (manual)",
            }[ch.switch.state]
            flags = []
            if ch.last_decision.hardware_trip:
                flags.append("HW-TRIP")
            if ch.last_decision.software_trip:
                flags.append("SW-TRIP")
            flag_str = f"  [{' '.join(flags)}]" if flags else ""
            print(f" {ch.name:<12} | {ch.last_current_a:6.2f} A | {state_label}{flag_str}")
        print("=" * 60)


# ==================================================================
# Boot sequence (purely for vibes -- no real board bring-up here)
# ==================================================================

SPLASH_ART = r"""
   ___                 _     ___ ___  __  __
  / __|_ __  __ _ _ _| |_  | _ \   \|  \/  |
  \__ \ '  \/ _` | '_|  _| |  _/ |) | |\/| |
  |___/_|_|_\__,_|_|  \__| |_| |___/|_|  |_|
"""


def boot_sequence():
    print(SPLASH_ART)
    print("Smart PDM Sim -- fluffy re-creation of a current-sensing / logic / PFET stack")
    print("No real current sensors. No real PFETs. No real 12V.\n")

    steps = [
        "Pretending to power up current sense amplifiers",
        "Pretending to configure hardware trip comparators",
        "Loading software trip thresholds",
        "Bringing up combinational logic block",
        "Commanding all PFET gates closed at boot",
    ]
    for step in steps:
        print(f"  [ OK ] {step}")
        time.sleep(0.05)
    print()


# ==================================================================
# Entry point -- the whole "control loop" in a for-loop
# ==================================================================

def main(cycles: int = 5, fault_on_cycle: int = 3, fault_channel: str = "Fuel Pump"):
    boot_sequence()

    pdm = SmartPdm()

    for cycle in range(1, cycles + 1):
        if cycle == fault_on_cycle:
            print(f">>> Simulating a short circuit on '{fault_channel}' (nothing real happens) <<<\n")
            pdm.inject_fault(fault_channel)

        print(f"--- Control loop pass #{cycle} ---")
        pdm.service_all()
        pdm.render_status()
        time.sleep(0.1)

    print("\nControl loop complete. The car's 12V system remains entirely imaginary.")


if __name__ == "__main__":
    main()