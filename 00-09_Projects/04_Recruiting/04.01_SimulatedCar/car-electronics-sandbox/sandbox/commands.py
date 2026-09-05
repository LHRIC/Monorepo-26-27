"""The entire participant command surface. Never launch an external executable."""
import re
import shlex

from .car import REQUIRED_IDS
from .files import UserError, VIRTUAL_HOME
from .terminal import Editor

COUNTS = {"ls": 0, "cat": 1, "cd": 1, "echo": None, "man": 0, "help": 0, "pwd": 0,
          "ecu_protocol": 0, "check_CAN": 1, "check_display": 0, "charge_battery": 1,
          "test_car": 0, "nano": 1, "vim": 1}
FORBIDDEN = re.compile(r"[<>|;&`$\\*?\[\]{}()!\x00-\x1f\x7f]")


def parse(line):
    if len(line) > 1024 or FORBIDDEN.search(line):
        raise UserError("Shell operators, substitutions, wildcards, and control characters are not allowed.")
    try:
        words = shlex.split(line)
    except ValueError:
        raise UserError("A quote is unfinished. Use simple names or close the quote.") from None
    if not words:
        return None, []
    command, args = words[0], words[1:]
    if command not in COUNTS:
        raise UserError("That command is not available. Run help to see the list.")
    count = COUNTS[command]
    if count is not None and len(args) != count:
        raise UserError(f"{command} takes {'no arguments' if count == 0 else 'exactly one argument'}.")
    if any(a.startswith("-") for a in args):
        raise UserError("Flags are not supported. Run help for the simplified usage.")
    return command, args


class Session:
    def __init__(self, files, car, terminal=None):
        self.files, self.car, self.terminal = files, car, terminal
        self.cwd = ()

    def execute(self, line):
        command, args = parse(line)
        if command is None:
            return ""
        if command == "ls":
            return self.files.list(self.cwd)
        if command == "pwd":
            return VIRTUAL_HOME + ("/" + "/".join(self.cwd) if self.cwd else "")
        if command == "echo":
            return " ".join(args)
        if command in ("man", "help"):
            return self.files.read(("commands.txt",))
        if command == "cat":
            return self.files.read(self.files.path(args[0], self.cwd))
        if command == "cd":
            path = self.files.path(args[0], self.cwd)
            with self.files.directory(path):
                self.cwd = path
            return ""
        if command == "ecu_protocol":
            return self.files.read(("challenge", "ecu_protocol.txt"))
        if command == "check_CAN":
            if not re.fullmatch(r"[0-9]{1,3}", args[0]) or int(args[0]) not in REQUIRED_IDS:
                raise UserError("Unknown CAN ID. Run ecu_protocol to see the available IDs.")
            return format(self.car.backend.snapshot()[int(args[0])], "08b")
        if command == "charge_battery":
            return self.car.backend.charge(args[0])
        if command == "check_display":
            return self.car.display()
        if command == "test_car":
            return self.car.test()
        if command in ("nano", "vim"):
            if self.terminal is None:
                raise UserError("The editor requires an interactive SSH terminal.")
            Editor(self.terminal, self.files, self.files.path(args[0], self.cwd), command).run()
            return ""
        raise UserError("Command unavailable.")

    def run(self):
        self.terminal.send("\x1b[?2004h")
        try:
            self.terminal.output("Welcome! Start by running: cat commands.txt\nUse Ctrl+D at the prompt to disconnect.")
            while True:
                line = self.terminal.line("car> ")
                try:
                    result = self.execute(line)
                    if result:
                        self.terminal.output(result)
                except UserError as error:
                    self.terminal.output(str(error))
        except (EOFError, OSError):
            pass
        finally:
            try:
                self.terminal.send("\x1b[?2004l")
            except OSError:
                pass
