import os
import shutil
import tempfile
import time
import unittest
from pathlib import Path

from sandbox.car import Backend, Car, config, recovered_data
from sandbox.commands import COUNTS, Session, parse
from sandbox.files import Files, MAX_FILE, UserError, safe_text
from sandbox.terminal import Buffer, Editor, Terminal

ROOT = Path(__file__).resolve().parents[1]


class Fixture(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.home = self.root / "home"
        self.home.mkdir()
        shutil.copytree(ROOT / "challenge", self.home / "challenge")
        shutil.copyfile(ROOT / "challenge/commands.txt", self.home / "commands.txt")
        self.files = Files(self.home)

    def write(self, path, value):
        parts = self.files.path(path)
        old = self.files.read(parts, missing_ok=True)
        self.files.write(parts, value, self.files.revision(old))


class FilesTests(Fixture):
    def test_navigation(self):
        self.assertEqual(self.files.path("~/challenge/car"), ("challenge", "car"))
        self.assertEqual(self.files.path("..", ("challenge", "car")), ("challenge",))
        self.assertEqual(self.files.path("/home/recruit/challenge"), ("challenge",))
        self.assertEqual(self.files.path("."), ())

    def test_traversal_and_hidden_paths(self):
        for path in ["..", "../../etc/passwd", "/etc/passwd", "/proc/self/environ",
                     "/home/recruit-other", "~other", "~/bin", "challenge/.hidden",
                     "challenge/../../etc/passwd", "challenge/bin/python", "\x1bfile"]:
            with self.subTest(path=path), self.assertRaises(UserError):
                self.files.path(path)

    def test_symlink_reads_and_edits(self):
        secret = self.root / "secret"
        secret.write_text("private")
        link = self.home / "challenge/work/link.txt"
        link.symlink_to(secret)
        self.assertNotIn("link.txt", self.files.list(("challenge", "work")))
        for operation in [lambda: self.files.read(("challenge", "work", "link.txt")),
                          lambda: self.files.write(("challenge", "work", "link.txt"), "oops", None)]:
            with self.assertRaises(UserError):
                operation()
        self.assertEqual(secret.read_text(), "private")

    def test_directory_symlink(self):
        (self.home / "challenge/escape").symlink_to(self.root, target_is_directory=True)
        with self.assertRaises(UserError):
            self.files.list(("challenge", "escape"))

    def test_hardlinks_and_special_files(self):
        original = self.root / "secret"
        original.write_text("private")
        os.link(original, self.home / "challenge/work/linked.txt")
        os.mkfifo(self.home / "challenge/work/pipe")
        for name in ["linked.txt", "pipe"]:
            with self.assertRaises(UserError):
                self.files.read(("challenge", "work", name))

    def test_hidden_listing(self):
        (self.home / "bin").mkdir()
        (self.home / ".hidden").write_text("private")
        self.assertEqual(self.files.list(()), "challenge/\ncommands.txt")

    def test_read_only_documents_and_reward(self):
        for path in ["commands.txt", "challenge/mission.txt", "challenge/reward.txt", "outside.txt"]:
            with self.subTest(path=path), self.assertRaises(UserError):
                self.write(path, "changed")

    def test_create_and_conflict(self):
        path = ("challenge", "work", "notes.txt")
        first = self.files.write(path, "first", None)
        self.files.write(path, "teammate", first)
        with self.assertRaises(UserError):
            self.files.write(path, "overwrite", first)
        self.assertEqual(self.files.read(path), "teammate")

    def test_read_write_limits(self):
        with self.assertRaises(UserError):
            self.write("challenge/work/big.txt", "a" * (MAX_FILE + 1))
        (self.home / "challenge/work/big.txt").write_bytes(b"a" * (MAX_FILE + 1))
        with self.assertRaises(UserError):
            self.files.read(("challenge", "work", "big.txt"))

    def test_file_quota(self):
        folder = self.home / "challenge/work"
        for n in range(127):
            (folder / f"note{n}.txt").write_text("")
        with self.assertRaises(UserError):
            self.write("challenge/work/overflow.txt", "hello")

    def test_control_sequence_sanitization(self):
        self.assertNotIn("\x1b", safe_text("\x1b]52;c;copy\x07"))
        with self.assertRaises(UserError):
            self.write("challenge/work/payload.txt", "\x1b[2J")


class CommandTests(unittest.TestCase):
    def test_exact_command_surface(self):
        self.assertEqual(set(COUNTS), {"ls", "cat", "cd", "echo", "man", "help", "pwd",
                         "ecu_protocol", "check_CAN", "check_display", "charge_battery",
                         "test_car", "nano", "vim"})

    def test_arguments(self):
        for line in ["ls ..", "ls -a", "cat", "cat a b", "cd", "man cat", "help x", "pwd x",
                     "test_car x", "check_CAN 1 2", "nano -R", "vim a b", "echo -n hello"]:
            with self.subTest(line=line), self.assertRaises(UserError):
                parse(line)
        self.assertEqual(parse("check_CAN 7"), ("check_CAN", ["7"]))
        self.assertEqual(parse('echo "hello team"'), ("echo", ["hello team"]))

    def test_shell_escape_attempts(self):
        for line in ["cat a > b", "ls | cat", "echo x; ls", "ls && pwd", "echo $(id)",
                     "echo `id`", "echo $PATH", "cat *", "cat a\nls", "cat a\x00",
                     "bash", "/bin/sh", "python x.py", "scp x y", "sftp", "ls &",
                     "echo hi >> x", "cat < x", "cat a\\b", "exec sh", "env", "exit"]:
            with self.subTest(line=line), self.assertRaises(UserError):
                parse(line)

    def test_safe_configuration(self):
        self.assertTrue(config("# comment\nshutdown_continuous = True\n", "shutdown_continuous", "boolean"))
        self.assertEqual(config("allow_list = [1, 2, 3, 7, 8]", "allow_list", "list"), {1, 2, 3, 7, 8})

    def test_configuration_never_executes(self):
        for value in ["import os\nshutdown_continuous = True", "shutdown_continuous = bool(1)",
                      "shutdown_continuous = __import__('os').system('id')", "shutdown_continuous = 1",
                      "shutdown_continuous = True\nopen('/tmp/escaped','w')", "while True: pass"]:
            with self.subTest(value=value), self.assertRaises(UserError):
                config(value, "shutdown_continuous", "boolean")
        for value in ["[True]", "list(range(256))", "[x for x in range(8)]", "[256]", "'1,2,3'", "[1] * 999999"]:
            with self.subTest(value=value), self.assertRaises(UserError):
                config("allow_list = " + value, "allow_list", "list")


class CarTests(Fixture):
    def setUp(self):
        super().setUp()
        self.backend = Backend(self.root / "state")
        self.backend.start()
        self.addCleanup(self.backend.close)
        deadline = time.monotonic() + 2
        while self.backend.snapshot()[7] == 0 and time.monotonic() < deadline:
            time.sleep(0.01)
        self.car = Car(self.files, self.backend, ROOT / "prize/datasheet.txt")
        self.session = Session(self.files, self.car)

    def test_full_progression_and_required_conversation(self):
        self.assertIn("doesn't turn on", self.session.execute("test_car"))
        self.assertEqual(self.session.execute("check_CAN 1"), "01011011")
        self.session.execute("charge_battery 12.6")
        self.assertEqual(self.session.execute("check_CAN 1"), "01111110")
        self.assertIn("flashing red", self.session.execute("test_car"))
        self.assertIn("PDM Shutdown Discontinuous", self.session.execute("check_display"))
        self.write("challenge/car/pdm.py", "shutdown_continuous = True\n")
        self.assertIn("read 0", self.session.execute("test_car"))
        self.assertIn("Battery: 0.0 V", self.session.execute("check_display"))
        self.write("challenge/car/display.py", "allow_list = [1, 2, 3, 7, 8]\n")
        self.assertIn("Find Jack", self.session.execute("test_car"))
        self.assertFalse((self.home / "challenge/reward.txt").exists())
        # Correct data alone does not bypass the human interaction.
        self.write("challenge/daq/data.txt", recovered_data())
        self.assertIn("Find Jack", self.session.execute("test_car"))
        (self.backend.state / "jack-approved").touch()
        self.assertIn("PASS!", self.session.execute("test_car"))
        self.assertTrue((self.backend.state / "daq-fixed").exists())
        self.assertIn("CUSTOM IC HANDOFF", self.session.execute("cat challenge/reward.txt"))

    def test_corrupted_recording_is_recoverable(self):
        raw = self.files.read(("challenge", "daq", "data.txt"))
        recovered = bytes(int(t) ^ 128 for t in raw.split()).decode("ascii")
        self.assertEqual(recovered, recovered_data())
        self.assertEqual(len(recovered.splitlines()), 65)

    def test_bad_data_does_not_unlock_prize(self):
        self.backend.charge("12.6")
        self.write("challenge/car/pdm.py", "shutdown_continuous = True")
        self.write("challenge/car/display.py", "allow_list = [1, 2, 3, 7, 8]")
        (self.backend.state / "jack-approved").touch()
        for value in ["", "PASS", recovered_data()[:-10], recovered_data().replace("12.6", "12.7", 1)]:
            self.write("challenge/daq/data.txt", value)
            self.assertIn("isn't quite right", self.session.execute("test_car"))
            self.assertFalse((self.home / "challenge/reward.txt").exists())

    def test_charge_input_and_boundary(self):
        for value in ["NaN", "sNaN", "Infinity", "17", "-1", "12.65", "hello"]:
            with self.subTest(value=value), self.assertRaises(UserError):
                self.backend.charge(value)
        self.backend.charge("11.4")
        self.assertIn("OFF", self.car.display())
        self.backend.charge("11.5")
        self.assertNotIn("OFF", self.car.display())

    def test_publisher_is_live_and_preserves_recording(self):
        self.assertNotEqual(self.backend.process.pid, os.getpid())
        data = self.files.read(("challenge", "daq", "data.txt"))
        first = self.backend.bus[255]
        time.sleep(0.3)
        self.assertNotEqual(first, self.backend.bus[255])
        self.assertEqual(data, self.files.read(("challenge", "daq", "data.txt")))
        self.backend.charge("13.2")
        time.sleep(0.3)
        self.assertEqual(self.backend.snapshot()[1], 132)

    def test_battery_persists(self):
        self.backend.charge("12.6")
        second = Backend(self.backend.state)
        self.assertEqual(second.battery.value, 126)

    def test_shared_car_and_independent_working_directories(self):
        second = Session(self.files, self.car)
        self.session.execute("cd challenge/car")
        self.assertEqual(second.execute("pwd"), "/home/recruit")
        self.session.execute("charge_battery 12.6")
        self.assertEqual(second.execute("check_CAN 1"), "01111110")

    def test_unknown_ids(self):
        for value in ["0", "255", "999", "7.0", "rpm"]:
            with self.subTest(value=value), self.assertRaises(UserError):
                self.session.execute("check_CAN " + value)


class FakeTerminal:
    def __init__(self, keys):
        self.keys = iter(keys)
        self.dimensions = type("Size", (), {"width": 80, "height": 24})()
        self.rendered = []

    def key(self):
        return next(self.keys)

    def send(self, text):
        self.rendered.append(text)

    def line(self, prompt, limit=1024):
        return self.key()


class EditorTests(Fixture):
    def test_nano_replace_and_save(self):
        path = ("challenge", "daq", "data.txt")
        term = FakeTerminal(["\x14", "y", ("PASTE", recovered_data()), "\x0f", "\x18"])
        Editor(term, self.files, path, "nano").run()
        self.assertEqual(self.files.read(path), recovered_data())

    def test_editor_discard(self):
        path = ("challenge", "work", "new.txt")
        term = FakeTerminal(["a", "\x18", "n"])
        Editor(term, self.files, path, "nano").run()
        self.assertIsNone(self.files.read(path, missing_ok=True))

    def test_vim_save_and_block_escape(self):
        path = ("challenge", "work", "new.txt")
        term = FakeTerminal([":", "!sh", "i", ("PASTE", "hello"), "ESC", ":", "wq"])
        Editor(term, self.files, path, "vim").run()
        self.assertEqual(self.files.read(path), "hello")
        self.assertTrue(any("Only :w" in s for s in term.rendered))

    def test_buffer_edits(self):
        b = Buffer("first\nsecond")
        b.move("END")
        b.insert("\nnew")
        self.assertEqual(b.text(), "first\nnew\nsecond")
        b.col = 0
        b.backspace()
        self.assertEqual(b.text(), "firstnew\nsecond")
        b.move("END")
        b.delete()
        self.assertEqual(b.text(), "firstnewsecond")

    def test_large_paste_rejected(self):
        b = Buffer("original")
        with self.assertRaises(UserError):
            b.insert("x" * MAX_FILE)
        self.assertEqual(b.text(), "original")

    def test_escape_does_not_eat_next_vim_command(self):
        term = Terminal(None, None)
        term.pending = bytearray(b"\x1b:wq\r")
        self.assertEqual(term.key(), "ESC")
        self.assertEqual(term.key(), ":")

    def test_bracketed_paste_is_one_event(self):
        term = Terminal(None, None)
        term.pending = bytearray(b"\x1b[200~a\r\nb\x1b[201~")
        self.assertEqual(term.key(), ("PASTE", "a\nb"))


if __name__ == "__main__":
    unittest.main()
