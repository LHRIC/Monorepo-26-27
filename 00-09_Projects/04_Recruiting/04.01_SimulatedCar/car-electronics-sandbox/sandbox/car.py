"""CAN simulator, safe Python configuration parser, and staged diagnostics."""
import ast
import json
import math
import multiprocessing as mp
import os
import threading
from decimal import Decimal, InvalidOperation
from pathlib import Path

from .files import UserError, safe_text

REQUIRED_IDS = {1, 2, 3, 7, 8}


def recovered_data():
    lines = ["sample,battery_v,coolant_c,rpm,gear,throttle_pct"]
    for n in range(64):
        lines.append(f"{n},12.6,{78 + n % 9},{1800 + (n * 137) % 2800},{1 + n // 16},{20 + n * 7 % 61}")
    return "\n".join(lines) + "\n"


def atomic_json(path, value):
    temp = path.with_suffix(".tmp")
    temp.write_text(json.dumps(value), encoding="utf-8")
    os.replace(temp, path)


def publish_can(bus, battery, stop):
    """A separate OS process publishes complete one-byte CAN snapshots at 4 Hz."""
    tick = 0
    while not stop.is_set():
        with bus.get_lock():
            bus[1] = battery.value  # tenths of a volt
            bus[2] = 82 + round(3 * math.sin(tick / 24))
            bus[3] = 2
            bus[7] = 24 + round(4 * math.sin(tick / 8))  # hundreds of RPM
            bus[8] = 30 + round(6 * math.sin(tick / 11))
            bus[255] = (tick % 255) + 1  # private heartbeat; not a public CAN ID
        tick += 1
        stop.wait(0.25)


class Backend:
    def __init__(self, state):
        self.state = Path(state)
        self.state.mkdir(parents=True, exist_ok=True)
        saved = self.state / "battery.json"
        value = json.loads(saved.read_text()) if saved.exists() else 91
        if type(value) is not int or not 0 <= value <= 160:
            raise ValueError("Invalid saved battery state")
        self.bus = mp.Array("i", 256)
        self.battery = mp.Value("i", value)
        self.stop = mp.Event()
        self.process = mp.Process(target=publish_can, args=(self.bus, self.battery, self.stop),
                                  name="can-backend", daemon=True)
        self.charge_lock = threading.Lock()

    def start(self):
        self.process.start()
        # The parent seeds the battery for an immediate first command.
        with self.bus.get_lock():
            self.bus[1] = self.battery.value

    def close(self):
        self.stop.set()
        self.process.join(timeout=3)
        if self.process.is_alive():
            self.process.terminate()
            self.process.join(timeout=3)

    def snapshot(self):
        if not self.process.is_alive():
            raise UserError("The CAN simulator is unavailable. Ask a facilitator to restart it.")
        with self.bus.get_lock():
            return {i: self.bus[i] for i in REQUIRED_IDS}

    def charge(self, raw):
        try:
            volts = Decimal(raw)
            if not volts.is_finite() or not 0 <= volts <= 16 or volts * 10 != (volts * 10).to_integral_value():
                raise ValueError
        except (InvalidOperation, ValueError):
            raise UserError("Use a voltage from 0 to 16, with at most one decimal place.") from None
        value = int(volts * 10)
        with self.charge_lock:
            atomic_json(self.state / "battery.json", value)
            # Same lock as the publisher prevents a stale battery frame after charging.
            with self.bus.get_lock():
                self.battery.value = value
                self.bus[1] = value
        return f"Battery set to {value / 10:.1f} V. The CAN backend has been updated."


def config(text, name, kind):
    """Accept ONLY a literal assignment. Never execute participant Python."""
    if len(text) > 8192:
        raise UserError(f"Keep {name}'s file under 8 KiB.")
    try:
        tree = ast.parse(text)
    except (SyntaxError, ValueError, RecursionError):
        raise UserError(f"Cannot read {name}. Check the Python syntax and save the file.") from None
    nodes = list(ast.walk(tree))
    if len(nodes) > 256 or len(tree.body) != 1:
        raise UserError(f"Keep just the {name} assignment and comments in this file.")
    statement = tree.body[0]
    if (not isinstance(statement, ast.Assign) or len(statement.targets) != 1
            or not isinstance(statement.targets[0], ast.Name) or statement.targets[0].id != name):
        raise UserError(f"This file must contain the assignment {name} = ...")
    if kind == "boolean":
        node = statement.value
        if not isinstance(node, ast.Constant) or type(node.value) is not bool:
            raise UserError(f"{name} must be True or False.")
        return node.value
    node = statement.value
    if (not isinstance(node, ast.List) or len(node.elts) > 32
            or any(not isinstance(v, ast.Constant) or type(v.value) is not int
                   or not 0 <= v.value <= 255 for v in node.elts)):
        raise UserError(f"{name} must be a list of integer CAN IDs, such as [3, 7, 8].")
    return set(v.value for v in node.elts)


class Car:
    def __init__(self, files, backend, prize):
        self.files, self.backend, self.prize = files, backend, Path(prize)
        self.lock = threading.Lock()

    def settings(self):
        pdm = config(self.files.read(("challenge", "car", "pdm_config.py")),
                     "shutdown_continuous", "boolean")
        allowed = config(self.files.read(("challenge", "car", "display_config.py")), "allow_list", "list")
        return pdm, allowed

    def display(self):
        bus = self.backend.snapshot()
        if bus[1] < 115:
            return "DISPLAY OFF - no illuminated screen."
        elif bus[1] > 150:
            return "DISPLAY OFF - on the real car you might have fried everything"
        pdm, allowed = self.settings()
        lines = ["CAR DISPLAY"]
        if not pdm:
            lines.append("[FLASHING RED] PDM Shutdown Discontinuous")
        get = lambda i: bus[i] if i in allowed else 0
        lines.extend([f"Battery: {get(1) / 10:.1f} V", f"Coolant: {get(2)} C",
                      f"RPM: {get(7) * 100}", f"Gear: {get(3)}", f"Throttle: {get(8)} %"])
        return "\n".join(lines)

    def test(self):
        with self.lock:
            bus = self.backend.snapshot()
            if bus[1] < 115 or bus[1] > 150:
                return "You turn on power. The display doesn't turn on... Check the available measurements."
            pdm = config(self.files.read(("challenge", "car", "pdm_config.py")),
                         "shutdown_continuous", "boolean")
            if not pdm:
                return "The display turns on! But there is flashing red text. Run check_display."
            allowed = config(self.files.read(("challenge", "car", "display_config.py")), "allow_list", "list")
            if not REQUIRED_IDS <= allowed:
                return ("The warning has gone away, but display measurements are missing or read 0. "
                        "Compare check_display with the CAN bus and the display code.")
            state = self.backend.state
            # if not (state / "jack-approved").exists():
            #     return ("The display looks correct! DAQ is next, and its saved data looks like gibberish.\n"
            #             "Find Jack. Explain what you tested and ask him to inspect the logger.\n"
            #             "He must confirm your conversation before this stage can pass. Read challenge/daq/README.txt.")
            data = self.files.read(("challenge", "daq_logs", "logs_latest", "data.txt"))
            # Preserve exact contents except conventional CRLF and one optional final newline.
            normalized = data.replace("\r\n", "\n").removesuffix("\n")
            if normalized != recovered_data().removesuffix("\n"):
                return ("The display looks correct! DAQ is next, and the most recent data log looks like gibberish.\n"
                        "Jack happens to be nearby, sees the issue, and looks at the DAQ code.\n"
                        "He smirks: 'A silly logging bug. I'll fix the DAQ code; can you recover the data?\n\n"
                        "Compare the binary representation of the messed up new data log and correct old logs to try and figure out what the issue is.\n"
                        "Feel free to look up how to do this in the programming language of your choice. I suggest Python if you aren't sure."
                        "Write a program to reverse the issue and recover the data on your laptop,\n"
                        "then replace challenge/daq_logs/logs_latest/data.txt with the recovered text.'\n"
                        "The data in the latest log isn't quite right. Jack will finish the DAQ when you fix this data.")
            (state / "daq-fixed").write_text("fixed\n")
            # This file is outside the virtual home until the final check passes.
            prize_text = self.prize.read_text(encoding="utf-8")
            reward = Path(self.files.root) / "challenge" / "reward.txt"
            # Facilitator-provided templates are trusted; participant API cannot create this path.
            if reward.is_symlink():
                raise UserError("Reward unavailable. Ask a facilitator.")
            temp = reward.with_name(".reward.tmp")
            temp.write_text(safe_text(prize_text), encoding="utf-8")
            os.replace(temp, reward)
            return ("PASS! Congrats! The data looks correct, and Jack fixed the DAQ code.\n"
                    "Jack thanks you for recovering the data. Your next-part handoff is here:\n"
                    "cat ~/challenge/reward.txt")
