"""Password-free SSH protocol endpoint. The account is virtual, with no OS login."""
import logging
import os
import signal
import socket
import threading
from pathlib import Path

import paramiko

from .car import Backend, Car
from .commands import Session
from .files import Files
from .terminal import Terminal

log = logging.getLogger(__name__)


class Access(paramiko.ServerInterface):
    def __init__(self):
        self.ready = threading.Event()
        self.width, self.height = 80, 24
        self.opened = False

    def get_allowed_auths(self, username):
        return "none"

    def check_auth_none(self, username):
        return paramiko.AUTH_SUCCESSFUL if username == "recruit" else paramiko.AUTH_FAILED

    def check_channel_request(self, kind, chanid):
        if kind == "session" and not self.opened:
            self.opened = True
            return paramiko.OPEN_SUCCEEDED
        return paramiko.OPEN_FAILED_ADMINISTRATIVELY_PROHIBITED

    def check_channel_pty_request(self, channel, term, width, height, pixelwidth, pixelheight, modes):
        self.width = max(20, min(width or 80, 240))
        self.height = max(8, min(height or 24, 80))
        return True

    def check_channel_window_change_request(self, channel, width, height, pixelwidth, pixelheight):
        return self.check_channel_pty_request(channel, b"", width, height, 0, 0, b"")

    def check_channel_shell_request(self, channel):
        if self.ready.is_set():
            return False
        self.ready.set()
        return True

    def check_channel_exec_request(self, channel, command):
        return False

    def check_channel_subsystem_request(self, channel, name):
        return False

    def check_channel_env_request(self, channel, name, value):
        return False

    def check_channel_direct_tcpip_request(self, chanid, origin, destination):
        return paramiko.OPEN_FAILED_ADMINISTRATIVELY_PROHIBITED

    def check_port_forward_request(self, address, port):
        return False

    def check_channel_forward_agent_request(self, channel):
        return False

    def check_channel_x11_request(self, channel, single_connection, auth_protocol, auth_cookie, screen_number):
        return False


class SSHServer:
    def __init__(self, files, car, host_key, host="0.0.0.0", port=2222, max_clients=24):
        self.files, self.car, self.host_key = files, car, host_key
        self.host, self.port = host, port
        self.stop = threading.Event()
        self.slots = threading.BoundedSemaphore(max_clients)
        self.listener = None
        self.connections = set()
        self.connections_lock = threading.Lock()

    def client(self, connection):
        transport = None
        try:
            transport = paramiko.Transport(connection)
            transport.local_version = "SSH-2.0-CarChallenge"
            transport.banner_timeout = 10
            transport.auth_timeout = 15
            transport.add_server_key(self.host_key)
            access = Access()
            transport.start_server(server=access)
            channel = transport.accept(timeout=15)
            if channel is None or not access.ready.wait(timeout=10):
                return
            channel.settimeout(900)
            Session(self.files, self.car, Terminal(channel, access)).run()
            channel.send_exit_status(0)
            channel.close()
        except (EOFError, OSError, paramiko.SSHException):
            pass
        except Exception:
            log.exception("Session failed; details are visible only to the facilitator")
        finally:
            if transport:
                transport.close()
            connection.close()
            with self.connections_lock:
                self.connections.discard(connection)
            self.slots.release()

    def bind(self):
        self.listener = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.listener.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.listener.bind((self.host, self.port))
        self.port = self.listener.getsockname()[1]
        self.listener.listen(24)
        self.listener.settimeout(0.5)

    def serve(self):
        if self.listener is None:
            self.bind()
        log.info("Car challenge listening on port %s", self.port)
        try:
            while not self.stop.is_set():
                if not self.car.backend.process.is_alive():
                    raise RuntimeError("CAN backend stopped; restarting container is required")
                try:
                    connection, _ = self.listener.accept()
                except socket.timeout:
                    continue
                if not self.slots.acquire(blocking=False):
                    connection.close()
                    continue
                with self.connections_lock:
                    self.connections.add(connection)
                threading.Thread(target=self.client, args=(connection,), daemon=True).start()
        finally:
            self.listener.close()
            with self.connections_lock:
                for connection in tuple(self.connections):
                    try:
                        connection.shutdown(socket.SHUT_RDWR)
                    except OSError:
                        pass
                    connection.close()


def main():
    logging.basicConfig(level=logging.INFO, format="%(asctime)s %(levelname)s %(message)s")
    # Malformed handshake diagnostics are noisy and do not belong in activity logs.
    logging.getLogger("paramiko").setLevel(logging.CRITICAL)
    state = Path(os.environ.get("CAR_STATE", "/state"))
    state.mkdir(parents=True, exist_ok=True)
    key_file = state / "ssh_host_rsa_key"
    if not key_file.exists():
        old_mask = os.umask(0o077)
        try:
            paramiko.RSAKey.generate(3072).write_private_key_file(str(key_file))
        finally:
            os.umask(old_mask)
    key = paramiko.RSAKey.from_private_key_file(str(key_file))
    files = Files(os.environ.get("CAR_HOME", "/home/recruit"))
    backend = Backend(state)
    backend.start()
    car = Car(files, backend, os.environ.get("CAR_PRIZE", "/opt/prize/datasheet.txt"))
    server = SSHServer(files, car, key, port=int(os.environ.get("SSH_PORT", "2222")))
    for signum in (signal.SIGTERM, signal.SIGINT):
        signal.signal(signum, lambda *_: server.stop.set())
    try:
        server.serve()
    finally:
        backend.close()


if __name__ == "__main__":
    main()
