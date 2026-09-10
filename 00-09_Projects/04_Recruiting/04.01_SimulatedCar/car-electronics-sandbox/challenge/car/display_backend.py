#!/usr/bin/env python3
"""
==================================================================
   CarDisplay OS (Python "Port" of an STM32H7 / TouchGFX Cluster)
==================================================================

This is a fluffy, do-nothing Python re-creation of a typical
automotive cluster architecture:

    Microcontroller   : STM32H7   (not really, this is Python)
    Display           : Riverdi LCD (not really, this is a terminal)
    Graphics Library  : TouchGFX  (not really, this is print())
    Language          : C / C++   (not really, this is Python)
    RTOS              : None (simplified loop via Model.tick)
    Data Source       : ECU via CAN bus (not really, this is random.random)

No RTOS is used here either -- everything runs in one simple loop
via Model.tick(), same as the real thing. Unlike the real thing,
nothing here actually reads a CAN bus, moves a needle, or means
anything at all.

Architecture
------------

    CAN Bus (mostly ECU, though it can be anything on CAN bus)
        |
        v
    CAN Interrupt (main_c_style.on_can_frame)
        |  updates
        v
    can_types.py  --  CAN value types & handlers
    -----
    Model.tick()   --  data update tick
        |
        v
    Screen1Presenter
        |
        v
    Screen1View   --  UI element rendering
"""
import display_backend
import time
import random
from dataclasses import dataclass
from enum import Enum


# ==================================================================
# can_types.py (fluffy stand-in)
# ------------------------------------------------------------------
# Declares CAN value types, IDs, and default values, same idea as
# the real can_types.hpp/.cpp, except none of these IDs correspond
# to any real DBC, and no bytes ever actually arrive on a wire.
# ==================================================================

class CanId(Enum):
    """Pretend CAN arbitration IDs. Entirely made up."""
    THROTTLE = 0x0C0
    GEAR = 0x0D0
    BATTERY_VOLTAGE = 0x0E0
    ENGINE_RPM = 0x0F0
    COOLANT_TEMP = 0x100


@dataclass
class CanValue:
    """A single 'signal' decoded from a CAN frame. Purely decorative."""
    can_id: CanId
    name: str
    value: float = 0.0
    units: str = ""
    last_updated_ms: int = 0


def _default_can_values():
    """Equivalent of the CAN_value_ptrs array in can_types.cpp."""
    return [
        CanValue(CanId.THROTTLE, "throttle", 0.0, "%"),
        CanValue(CanId.GEAR, "gear", 0.0, ""),
        CanValue(CanId.BATTERY_VOLTAGE, "battery_voltage", 0.0, "V"),
        CanValue(CanId.ENGINE_RPM, "engine_rpm", 0.0, "rpm"),
        CanValue(CanId.COOLANT_TEMP, "coolant_temp", 0.0, "C"),
    ]


# The "array" every layer above reaches into, exactly like CAN_value_ptrs.
CAN_VALUE_PTRS = {cv.can_id: cv for cv in _default_can_values()}


def handle_can_frame(can_id: CanId, raw_payload: float):
    """
    Fluff version of the per-ID handler dispatch you'd see in
    can_types.cpp. Just stuffs a float into the table. No scaling,
    no DBC math, no checksum, no nothing.
    """
    if can_id in CAN_VALUE_PTRS:
        CAN_VALUE_PTRS[can_id].value = raw_payload
        CAN_VALUE_PTRS[can_id].last_updated_ms += 1


# ==================================================================
# main.c (fluffy stand-in)
# ------------------------------------------------------------------
# In the real firmware this sets up the CAN peripheral and its
# RxFifo0Callback interrupt. Here, "interrupts" are just a function
# we call manually once per tick, because Python doesn't have an
# NVIC and we are not about to pretend that hard.
# ==================================================================

class FakeCanBus:
    """Stands in for an actual CAN peripheral. Generates nothing real."""

    def __init__(self):
        self._connected = False

    def init(self):
        # CubeMX would have generated real HAL init calls here.
        self._connected = True

    def poll_frames(self):
        """
        Equivalent of RxFifo0Callback firing on frame receipt.
        Instead of real bus traffic, we just wiggle numbers around
        so the dashboard has something to draw.
        """
        if not self._connected:
            return

        throttle = CAN_VALUE_PTRS[CanId.THROTTLE].value
        handle_can_frame(CanId.THROTTLE, throttle)  # pedal never actually pressed

        gear = CAN_VALUE_PTRS[CanId.GEAR].value
        handle_can_frame(CanId.GEAR, gear)  # transmission never actually shifts

        battery = CAN_VALUE_PTRS[CanId.BATTERY_VOLTAGE].value
        handle_can_frame(CanId.BATTERY_VOLTAGE, battery)  # alternator does nothing

        rpm = CAN_VALUE_PTRS[CanId.ENGINE_RPM].value
        handle_can_frame(CanId.ENGINE_RPM, rpm)  # engine never actually turns over

        coolant = CAN_VALUE_PTRS[CanId.COOLANT_TEMP].value
        handle_can_frame(CanId.COOLANT_TEMP, coolant)  # thermostatically fluffy


def on_can_frame(bus: FakeCanBus):
    """Named to look like the interrupt routed from main.c."""
    bus.poll_frames()


# ==================================================================
# Model.cpp (fluffy stand-in)
# ------------------------------------------------------------------
# Polls updated CAN data each tick and forwards it toward the
# presenter. No RTOS -- this tick is just called from a plain loop.
# ==================================================================

class Model:
    def __init__(self, bus: FakeCanBus, presenter: "Screen1Presenter" = None):
        self.bus = bus
        self.presenter = presenter
        self.tick_count = 0

    def bind_presenter(self, presenter: "Screen1Presenter"):
        self.presenter = presenter

    def tick(self):
        """The entire 'RTOS' of this application. One function. No threads."""
        self.tick_count += 1
        on_can_frame(self.bus)

        if self.presenter is not None:
            self.presenter.on_model_updated(dict(CAN_VALUE_PTRS))


# ==================================================================
# Screen1Presenter.cpp (fluffy stand-in)
# ------------------------------------------------------------------
# Mediates between Model and View. Does no real transformation here,
# same as most presenters until someone adds business logic nobody
# asked for.
# ==================================================================

class Screen1Presenter:
    def __init__(self, view: "Screen1View" = None):
        self.view = view

    def bind_view(self, view: "Screen1View"):
        self.view = view

    def on_model_updated(self, can_values: dict):
        if self.view is not None:
            self.view.update_ui(can_values)


# ==================================================================
# Screen1View.cpp (fluffy stand-in)
# ------------------------------------------------------------------
# Updates UI elements and handles "special visual logic" -- in the
# real firmware this might be needle rotation math, color zones on
# a gauge, animations, etc. Here it's just print() with extra steps.
# ==================================================================

class Screen1View:
    THEMES = ["Midnight Blue", "Racing Red", "Carbon Fiber Gray", "Sunset Orange"]

    def __init__(self):
        self.theme = random.choice(self.THEMES)
        self.frame_count = 0

    def _special_visual_logic(self, can_values: dict) -> str:
        """
        Stand-in for the kind of one-off visual rule that always ends
        up living in the View layer: e.g. flashing a warning color
        when oil life gets low. Never actually triggers here.
        """
        battery = can_values[CanId.BATTERY_VOLTAGE].value
        if battery < 11.0:
            return "  !! BATTERY LOW !!"
        return ""

    def update_ui(self, can_values: dict):
        self.frame_count += 1
        throttle = can_values[CanId.THROTTLE].value
        gear = can_values[CanId.GEAR].value
        battery = can_values[CanId.BATTERY_VOLTAGE].value
        rpm = can_values[CanId.ENGINE_RPM].value
        coolant = can_values[CanId.COOLANT_TEMP].value
        warning = self._special_visual_logic(can_values)

        print("=" * 52)
        print(f" Screen1View  |  theme: {self.theme}  |  frame #{self.frame_count}")
        print("=" * 52)
        print(f" Throttle:        {throttle:6.1f} %")
        print(f" Gear:            {gear:6.1f}")
        print(f" Battery Voltage: {battery:6.1f} V{warning}")
        print(f" Engine RPM:      {rpm:6.1f}")
        print(f" Coolant Temp:    {coolant:6.1f} C")
        print("=" * 52)


# ==================================================================
# Boot sequence (purely for vibes -- no CubeMX, no HAL, no flashing)
# ==================================================================

SPLASH_ART = r"""
   _____
  /  ___|
  | |     __ _ _ __
  | |    / _` | '__|
  | |___| (_| | |
  \_____|\__,_|_|
"""


def boot_sequence():
    print(SPLASH_ART)
    print("CarDisplay OS -- fluffy Python re-creation of an STM32H7/TouchGFX cluster")
    print("No RTOS. No CAN peripheral. No actual car. Just Model.tick().\n")

    steps = [
        "Pretending to configure CubeMX-generated clocks",
        "Pretending to bring up the Riverdi LCD",
        "Pretending to initialize TouchGFX",
        "Bringing up FakeCanBus",
        "Binding Model -> Presenter -> View",
    ]
    for step in steps:
        print(f"  [ OK ] {step}")
        time.sleep(0.05)
    print()


# ==================================================================
# Entry point -- the whole "RTOS" in a for-loop
# ==================================================================

def main(ticks: int = 5):
    boot_sequence()

    bus = FakeCanBus()
    bus.init()

    view = Screen1View()
    presenter = Screen1Presenter(view)
    model = Model(bus, presenter)

    for _ in range(ticks):
        model.tick()
        time.sleep(0.1)

    print("\nDone ticking. The car remains parked. Mission accomplished.")


if __name__ == "__main__":
    main()