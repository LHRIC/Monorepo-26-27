"""Real SSH integration tests; run in the Docker 'test' build target."""
import importlib.util
import shutil
import socket
import subprocess
import threading
import time
import unittest

from test_core import Fixture, ROOT
from sandbox.car import Backend, Car, recovered_data

HAS_PARAMIKO = importlib.util.find_spec("paramiko") is not None


@unittest.skipUnless(HAS_PARAMIKO, "Paramiko is not installed; use the Docker test target")
class SSHTests(Fixture):
    def setUp(self):
        super().setUp()
        import paramiko
        from sandbox.server import SSHServer
        self.paramiko = paramiko
        self.backend = Backend(self.root / "state")
        self.backend.start()
        self.addCleanup(self.backend.close)
        self.car = Car(self.files, self.backend, ROOT / "prize/datasheet.txt")
        self.server = SSHServer(self.files, self.car, paramiko.RSAKey.generate(2048),
                                host="127.0.0.1", port=0)
        self.server.bind()
        self.worker = threading.Thread(target=self.server.serve, daemon=True)
        self.worker.start()
        self.addCleanup(self.close_server)

    def close_server(self):
        self.server.stop.set()
        self.worker.join(timeout=3)
        self.assertFalse(self.worker.is_alive())

    def connect(self, username="recruit"):
        connection = socket.create_connection(("127.0.0.1", self.server.port), timeout=3)
        transport = self.paramiko.Transport(connection)
        self.addCleanup(transport.close)
        transport.start_client(timeout=5)
        transport.auth_none(username)
        return transport

    def shell(self):
        transport = self.connect()
        channel = transport.open_session(timeout=5)
        channel.get_pty(term="xterm-256color", width=100, height=30)
        channel.invoke_shell()
        channel.settimeout(5)
        self.read_until(channel, b"car> ")
        return transport, channel

    def read_until(self, channel, needle):
        output = b""
        deadline = time.monotonic() + 8
        while needle not in output and time.monotonic() < deadline:
            chunk = channel.recv(32768)
            if not chunk:
                break
            output += chunk
        self.assertIn(needle, output)
        return output

    def test_none_auth_and_interactive_commands(self):
        _, channel = self.shell()
        channel.sendall(b"cat commands.txt\r")
        self.read_until(channel, b"GENERAL LINUX UTILITY COMMANDS")
        channel.sendall(b"pwd\r")
        self.read_until(channel, b"/home/recruit")
        channel.sendall(b"charge_battery 12.6\r")
        self.read_until(channel, b"Battery set to 12.6 V")
        channel.sendall(b"check_CAN 1\r")
        self.read_until(channel, b"01111110")
        channel.sendall(b"\x04")

    def test_unknown_username_is_denied(self):
        with self.assertRaises(self.paramiko.AuthenticationException):
            self.connect("root")

    def test_remote_exec_is_denied(self):
        transport = self.connect()
        channel = transport.open_session(timeout=5)
        with self.assertRaises(self.paramiko.SSHException):
            channel.exec_command("cat /etc/passwd")

    def test_sftp_is_denied(self):
        transport = self.connect()
        channel = transport.open_session(timeout=5)
        with self.assertRaises(self.paramiko.SSHException):
            channel.invoke_subsystem("sftp")

    def test_scp_is_denied(self):
        transport = self.connect()
        channel = transport.open_session(timeout=5)
        with self.assertRaises(self.paramiko.SSHException):
            channel.exec_command("scp -t /tmp")

    def test_forwarding_is_denied(self):
        transport = self.connect()
        with self.assertRaises(self.paramiko.SSHException):
            transport.open_channel("direct-tcpip", ("127.0.0.1", 22), ("127.0.0.1", 45678), timeout=3)
        with self.assertRaises(self.paramiko.SSHException):
            transport.request_port_forward("127.0.0.1", 0)

    def test_multiple_channels_are_denied(self):
        transport, _ = self.shell()
        with self.assertRaises(self.paramiko.SSHException):
            transport.open_session(timeout=3)

    def test_shell_syntax_and_path_escape_over_ssh(self):
        _, channel = self.shell()
        channel.sendall(b"cat /etc/passwd\r")
        self.read_until(channel, b"only access your home")
        channel.sendall(b"ls | cat\r")
        self.read_until(channel, b"not allowed")
        channel.sendall(b"cd ..\r")
        self.read_until(channel, b"already at your home")

    def test_nano_and_vim_over_real_ssh(self):
        _, channel = self.shell()
        channel.sendall(b"nano challenge/work/notes.txt\r")
        self.read_until(channel, b"nano (sandbox)")
        channel.sendall(b"\x1b[200~hello team\n\x1b[201~\x0f")
        self.read_until(channel, b"Saved.")
        channel.sendall(b"\x18")
        self.read_until(channel, b"car> ")
        self.assertEqual(self.files.read(("challenge", "work", "notes.txt")), "hello team\n")
        channel.sendall(b"vim challenge/work/vim.txt\r")
        self.read_until(channel, b"vim (sandbox)")
        channel.sendall(b"ihello\x1b:wq\r")
        self.read_until(channel, b"car> ")
        self.assertEqual(self.files.read(("challenge", "work", "vim.txt")), "hello")

    @unittest.skipUnless(shutil.which("ssh"), "OpenSSH client not installed")
    def test_openssh_client_without_password_or_key(self):
        completed = subprocess.run([
            "ssh", "-T", "-p", str(self.server.port), "-o", "BatchMode=yes",
            "-o", "PreferredAuthentications=none", "-o", "StrictHostKeyChecking=no",
            "-o", "UserKnownHostsFile=/dev/null", "-o", "ConnectTimeout=5",
            "recruit@127.0.0.1",
        ], input=b"pwd\r\x04", capture_output=True, timeout=10)
        self.assertEqual(completed.returncode, 0, completed.stderr.decode())
        self.assertIn(b"/home/recruit", completed.stdout)


if __name__ == "__main__":
    unittest.main()
