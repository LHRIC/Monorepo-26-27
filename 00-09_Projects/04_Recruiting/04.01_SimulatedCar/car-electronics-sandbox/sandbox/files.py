"""Small virtual filesystem. Every component is opened relative to a trusted fd.

No symlinks, devices, hidden paths, host absolute paths, or hardlinked files.
Participant paths are never passed to a shell or an external editor.
"""
import hashlib
import os
import re
import secrets
import stat
import threading
from contextlib import contextmanager

MAX_FILE = 64 * 1024
MAX_FILES = 128
VIRTUAL_HOME = "/home/recruit"
NAME = re.compile(r"[A-Za-z0-9_][A-Za-z0-9_.-]{0,79}\Z")


class UserError(Exception):
    pass


def safe_text(text):
    """Never replay terminal escape/control sequences from participant files."""
    return "".join(c if c in "\n\t" or 32 <= ord(c) <= 126 else "?" for c in text)


class Files:
    def __init__(self, root):
        self.root = os.fspath(root)
        self.lock = threading.RLock()

    def path(self, raw, cwd=()):
        if not raw or len(raw) > 512:
            raise UserError("Use a file or folder name of at most 512 characters.")
        if raw == "~" or raw.startswith("~/"):
            parts, raw = [], raw[2:] if raw.startswith("~/") else ""
        elif raw == VIRTUAL_HOME or raw.startswith(VIRTUAL_HOME + "/"):
            parts, raw = [], raw[len(VIRTUAL_HOME):].lstrip("/")
        elif raw.startswith("/"):
            raise UserError("You can only access your home folder.")
        else:
            parts = list(cwd)
        for item in raw.split("/"):
            if item in ("", "."):
                continue
            if item == "..":
                if not parts:
                    raise UserError("You are already at your home folder.")
                parts.pop()
            elif not NAME.fullmatch(item) or item == "bin":
                raise UserError("That file or folder is not available.")
            else:
                parts.append(item)
        return tuple(parts)

    @contextmanager
    def directory(self, parts):
        fd = os.open(self.root, os.O_RDONLY | os.O_DIRECTORY | os.O_NOFOLLOW)
        try:
            for part in parts:
                next_fd = os.open(part, os.O_RDONLY | os.O_DIRECTORY | os.O_NOFOLLOW,
                                  dir_fd=fd)
                os.close(fd)
                fd = next_fd
            yield fd
        except OSError:
            raise UserError("That file or folder is not available.") from None
        finally:
            os.close(fd)

    def list(self, parts):
        with self.directory(parts) as fd:
            items = []
            for name in sorted(os.listdir(fd)):
                if not NAME.fullmatch(name) or name == "bin":
                    continue
                info = os.stat(name, dir_fd=fd, follow_symlinks=False)
                if stat.S_ISDIR(info.st_mode):
                    items.append(name + "/")
                elif stat.S_ISREG(info.st_mode) and info.st_nlink == 1:
                    items.append(name)
            return "\n".join(items) or "(empty folder)"

    def read_bytes(self, parts, missing_ok=False):
        if not parts:
            raise UserError("Choose a file, not a folder.")
        with self.directory(parts[:-1]) as fd:
            try:
                f = os.open(parts[-1], os.O_RDONLY | os.O_NOFOLLOW | os.O_NONBLOCK,
                            dir_fd=fd)
            except FileNotFoundError:
                if missing_ok:
                    return None
                raise UserError("That file does not exist.") from None
            except OSError:
                raise UserError("That file is not available.") from None
            with os.fdopen(f, "rb") as stream:
                info = os.fstat(stream.fileno())
                if not stat.S_ISREG(info.st_mode) or info.st_nlink != 1:
                    raise UserError("Only ordinary files are available.")
                data = stream.read(MAX_FILE + 1)
                if len(data) > MAX_FILE:
                    raise UserError("Files must be at most 64 KiB.")
                return data

    def read(self, parts, missing_ok=False):
        if not parts:
            raise UserError("Choose a file, not a folder.")
        with self.directory(parts[:-1]) as fd:
            try:
                f = os.open(parts[-1], os.O_RDONLY | os.O_NOFOLLOW | os.O_NONBLOCK,
                            dir_fd=fd)
            except FileNotFoundError:
                if missing_ok:
                    return None
                raise UserError("That file does not exist.") from None
            except OSError:
                raise UserError("That file is not available.") from None
            with os.fdopen(f, "rb") as stream:
                info = os.fstat(stream.fileno())
                if not stat.S_ISREG(info.st_mode) or info.st_nlink != 1:
                    raise UserError("Only ordinary text files are available.")
                data = stream.read(MAX_FILE + 1)
                if len(data) > MAX_FILE:
                    raise UserError("Files must be at most 64 KiB.")
                try:
                    return data.decode("utf-8")
                except UnicodeDecodeError:
                    raise UserError("This editor reads UTF-8 text files only.") from None

    def can_edit(self, parts):
        if (len(parts) < 2 or parts[0] != "challenge"
                or parts == ("challenge", "reward.txt")
                or parts[-1] in ("commands.txt", "mission.txt", "team.txt",
                                  "ecu_protocol.txt", "README.txt", "display_backend.py", "pdm_backend.py", "sensors_backend.py")):
            raise UserError("That document is read-only. Edit a config file instead.")

    @staticmethod
    def revision(text):
        return None if text is None else hashlib.sha256(text.encode()).hexdigest()

    def write(self, parts, text, expected):
        self.can_edit(parts)
        data = text.encode("utf-8")
        if len(data) > MAX_FILE:
            raise UserError("Files must be at most 64 KiB.")
        if text != safe_text(text):
            raise UserError("Use plain ASCII text, tabs, and line breaks.")
        with self.lock:
            previous = self.read(parts, missing_ok=True)
            if self.revision(previous) != expected:
                raise UserError("A teammate changed this file. Copy your edits, exit without saving, and reopen it.")
            with self.directory(parts[:-1]) as fd:
                if previous is None and len(os.listdir(fd)) >= MAX_FILES:
                    raise UserError("This folder has reached its file limit.")
                temp = ".save-" + secrets.token_hex(8)
                try:
                    out = os.open(temp, os.O_WRONLY | os.O_CREAT | os.O_EXCL | os.O_NOFOLLOW,
                                  0o600, dir_fd=fd)
                    with os.fdopen(out, "wb") as stream:
                        stream.write(data)
                        stream.flush()
                        os.fsync(stream.fileno())
                    os.replace(temp, parts[-1], src_dir_fd=fd, dst_dir_fd=fd)
                finally:
                    try:
                        os.unlink(temp, dir_fd=fd)
                    except FileNotFoundError:
                        pass
        return self.revision(text)
