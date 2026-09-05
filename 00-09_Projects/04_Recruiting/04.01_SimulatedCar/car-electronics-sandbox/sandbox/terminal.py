"""A tiny VT100 terminal UI. No pty subprocess, native editor, or shell exists."""
import socket
import time

from .files import MAX_FILE, UserError, safe_text


class Terminal:
    def __init__(self, channel, dimensions):
        self.channel, self.dimensions = channel, dimensions
        self.pending = bytearray()

    def send(self, text):
        self.channel.sendall(text.encode("utf-8"))

    def output(self, text):
        self.send(safe_text(text).replace("\n", "\r\n") + "\r\n")

    def byte(self, timeout=900):
        if not self.pending:
            self.channel.settimeout(timeout)
            data = self.channel.recv(4096)
            if not data:
                raise EOFError
            self.pending.extend(data)
        value = self.pending[0]
        del self.pending[0]
        return value

    def key(self):
        value = self.byte()
        if value != 27:
            return chr(value)
        sequence = bytearray()
        try:
            sequence.append(self.byte(0.08))
            if sequence[0] not in (ord("["), ord("O")):
                self.pending[:0] = sequence
                return "ESC"
            while len(sequence) < 16:
                sequence.append(self.byte(0.08))
                if 64 <= sequence[-1] <= 126:
                    break
        except socket.timeout:
            return "ESC"
        codes = {b"[A": "UP", b"[B": "DOWN", b"[C": "RIGHT", b"[D": "LEFT",
                 b"[H": "HOME", b"[F": "END", b"OH": "HOME", b"OF": "END",
                 b"[1~": "HOME", b"[4~": "END", b"[3~": "DELETE",
                 b"[5~": "PAGEUP", b"[6~": "PAGEDOWN"}
        if sequence == b"[200~":
            pasted = bytearray()
            deadline = time.monotonic() + 30
            while not pasted.endswith(b"\x1b[201~"):
                if len(pasted) > MAX_FILE + 6 or time.monotonic() > deadline:
                    raise EOFError("Paste is too large or timed out")
                pasted.append(self.byte(5))
            return ("PASTE", safe_text(pasted[:-6].decode("utf-8", "replace")
                                       .replace("\r\n", "\n").replace("\r", "\n")))
        return codes.get(bytes(sequence), "IGNORE")

    def line(self, prompt, limit=1024):
        text, cursor = "", 0
        def draw():
            # Keep the command on one physical row, including in narrow terminals.
            width = max(20, min(self.dimensions.width, 240))
            room = max(8, width - len(prompt) - 2)
            start = max(0, cursor - room + 1)
            self.send("\r\x1b[2K" + prompt + text[start:start + room])
            self.send(f"\r\x1b[{len(prompt) + cursor - start + 1}G")
        draw()
        while True:
            key = self.key()
            if key in ("\r", "\n"):
                self.send("\r\n")
                return text
            if key == "\x04":
                raise EOFError
            if key == "\x03":
                self.send("^C\r\n")
                return ""
            if key in ("\x7f", "\b"):
                if cursor:
                    text, cursor = text[:cursor - 1] + text[cursor:], cursor - 1
            elif key == "DELETE":
                text = text[:cursor] + text[cursor + 1:]
            elif key == "LEFT":
                cursor = max(0, cursor - 1)
            elif key == "RIGHT":
                cursor = min(len(text), cursor + 1)
            elif key in ("HOME", "\x01"):
                cursor = 0
            elif key in ("END", "\x05"):
                cursor = len(text)
            elif key == "\x15":
                text, cursor = text[cursor:], 0
            elif isinstance(key, tuple):
                if "\n" in key[1]:
                    self.send("\r\nPaste one command at a time; use an editor for file contents.\r\n")
                elif len(text) + len(key[1]) <= limit:
                    text, cursor = text[:cursor] + key[1] + text[cursor:], cursor + len(key[1])
            elif len(key) == 1 and 32 <= ord(key) <= 126 and len(text) < limit:
                text, cursor = text[:cursor] + key + text[cursor:], cursor + 1
            draw()


class Buffer:
    def __init__(self, text):
        self.lines = text.split("\n")
        self.row = self.col = 0

    def text(self):
        return "\n".join(self.lines)

    def insert(self, value):
        if len(self.text()) + len(value) > MAX_FILE:
            raise UserError("The editor limit is 64 KiB.")
        before, after = self.lines[self.row][:self.col], self.lines[self.row][self.col:]
        pieces = value.split("\n")
        replacement = (before + value + after).split("\n")
        self.lines[self.row:self.row + 1] = replacement
        self.row += len(pieces) - 1
        self.col = len(before) + len(pieces[0]) if len(pieces) == 1 else len(pieces[-1])

    def backspace(self):
        if self.col:
            line = self.lines[self.row]
            self.lines[self.row] = line[:self.col - 1] + line[self.col:]
            self.col -= 1
        elif self.row:
            suffix = self.lines.pop(self.row)
            self.row -= 1
            self.col = len(self.lines[self.row])
            self.lines[self.row] += suffix

    def delete(self):
        if self.col < len(self.lines[self.row]):
            line = self.lines[self.row]
            self.lines[self.row] = line[:self.col] + line[self.col + 1:]
        elif self.row < len(self.lines) - 1:
            self.lines[self.row] += self.lines.pop(self.row + 1)

    def move(self, key, page=10):
        if key == "LEFT":
            if self.col:
                self.col -= 1
            elif self.row:
                self.row -= 1
                self.col = len(self.lines[self.row])
        elif key == "RIGHT":
            if self.col < len(self.lines[self.row]):
                self.col += 1
            elif self.row < len(self.lines) - 1:
                self.row += 1
                self.col = 0
        elif key == "HOME":
            self.col = 0
        elif key == "END":
            self.col = len(self.lines[self.row])
        else:
            shift = {"UP": -1, "DOWN": 1, "PAGEUP": -page, "PAGEDOWN": page}.get(key, 0)
            self.row = min(len(self.lines) - 1, max(0, self.row + shift))
            self.col = min(self.col, len(self.lines[self.row]))


class Editor:
    """Nano-like keys and a deliberately small Vim mode over the same safe buffer."""
    def __init__(self, terminal, files, path, mode):
        self.term, self.files, self.path, self.mode = terminal, files, path, mode
        files.can_edit(path)
        original = files.read(path, missing_ok=True)
        self.revision = files.revision(original)
        self.saved = original or ""
        self.buffer = Buffer(safe_text(self.saved))
        self.insert_mode = mode == "nano"
        self.status = "Ctrl+O save | Ctrl+X exit | Ctrl+T clear all (asks first)"
        self.top = self.left = 0

    def draw(self):
        width = max(20, min(self.term.dimensions.width, 240))
        height = max(8, min(self.term.dimensions.height, 80))
        visible = height - 3
        b = self.buffer
        self.top = min(self.top, b.row)
        self.top = max(self.top, b.row - visible + 1)
        self.left = max(0, b.col - width + 2)
        title = f"{self.mode} (sandbox) | {self.path[-1]} | {'INSERT' if self.insert_mode else 'NORMAL'}"
        rows = ["\x1b[7m" + title[:width - 1].ljust(width - 1) + "\x1b[0m"]
        for row in range(self.top, self.top + visible):
            line = b.lines[row] if row < len(b.lines) else ""
            rows.append(line.replace("\t", " ")[self.left:self.left + width - 1])
        rows.extend([self.status[:width - 1], f"Line {b.row + 1}/{len(b.lines)}  Col {b.col + 1}"])
        self.term.send("\x1b[H" + "\r\n".join(r + "\x1b[K" for r in rows))
        self.term.send(f"\x1b[{b.row - self.top + 2};{b.col - self.left + 1}H")

    def ask(self, text):
        self.status = text
        self.draw()
        return self.term.key()

    def save(self):
        try:
            self.revision = self.files.write(self.path, self.buffer.text(), self.revision)
            self.saved = self.buffer.text()
            self.status = "Saved. Ctrl+X exits."
            return True
        except UserError as error:
            self.status = str(error)
            return False

    def run(self):
        self.term.send("\x1b[?1049h\x1b[2J\x1b[H")
        try:
            while True:
                self.draw()
                key = self.term.key()
                if key == "\x0f":
                    self.save()
                elif key == "\x18":
                    if self.buffer.text() == self.saved:
                        return
                    answer = self.ask("Save changes? Y = save and exit, N = discard, any other key = cancel")
                    if answer in ("n", "N") or (answer in ("y", "Y") and self.save()):
                        return
                elif key == "\x14":
                    if self.ask("Clear the entire buffer? Y = clear, any other key = cancel") in ("y", "Y"):
                        self.buffer = Buffer("")
                        self.status = "Buffer cleared. Paste the replacement, then Ctrl+O to save."
                elif key in ("UP", "DOWN", "LEFT", "RIGHT", "HOME", "END", "PAGEUP", "PAGEDOWN"):
                    self.buffer.move(key)
                elif key == "ESC" and self.mode == "vim":
                    self.insert_mode = False
                    self.status = "NORMAL | i insert | :w save | :q quit | :wq save+quit | :q! discard"
                elif not self.insert_mode:
                    if key == "i":
                        self.insert_mode = True
                    elif key == "a":
                        self.buffer.move("RIGHT")
                        self.insert_mode = True
                    elif isinstance(key, str) and len(key) == 1 and key in "hjkl":
                        self.buffer.move({"h": "LEFT", "j": "DOWN", "k": "UP", "l": "RIGHT"}[key])
                    elif key == "x":
                        self.buffer.delete()
                    elif key == ":":
                        self.term.send("\r\x1b[2K")
                        command = self.term.line(":", limit=80)
                        if command == "w":
                            self.save()
                        elif command == "wq":
                            if self.save():
                                return
                        elif command == "q!":
                            return
                        elif command == "q":
                            if self.buffer.text() == self.saved:
                                return
                            self.status = "Unsaved changes. Use :wq or :q!"
                        elif command == "%d":
                            self.buffer = Buffer("")
                        else:
                            self.status = "Only :w, :q, :wq, :q!, and :%d are available."
                elif key in ("\x7f", "\b"):
                    self.buffer.backspace()
                elif key == "DELETE":
                    self.buffer.delete()
                else:
                    value = (key[1] if isinstance(key, tuple) else "\n" if key in ("\n", "\r")
                             else key if len(key) == 1 and (key == "\t" or 32 <= ord(key) <= 126) else "")
                    if value:
                        try:
                            self.buffer.insert(value)
                        except UserError as error:
                            self.status = str(error)
        finally:
            self.term.send("\x1b[?1049l")
