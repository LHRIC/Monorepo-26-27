# Car electronics SSH sandbox

A complete recruitment activity: password-free SSH, a small command list,
safe built-in editors, a live CAN simulator, four repair stages, and a required
conversation with a facilitator. One container represents one team's shared car.

## Start

Install Docker with Docker Compose. From this project's folder:

```bash
docker compose up --build -d
```

Recruits connect from a terminal on their laptops:

```bash
ssh -p 2222 recruit@YOUR_SERVER_IP
```

Use `localhost` for a connection from the Docker host. There is no password or
key to distribute: the virtual `recruit` account accepts SSH's `none` authentication.
The usual first-connection host-key confirmation can still appear. Give recruits
only the connection command and tell them to run `cat commands.txt`.

The default port binding accepts connections to the host on port 2222. Make that
port reachable from the activity network. Everyone who can reach it can join and
edit this shared car, so use a controlled activity network, not a public service.
To bind only to the host for a local rehearsal:

```bash
SSH_BIND=127.0.0.1 docker compose up --build -d
```

Before the activity, replace `prize/datasheet.txt` with the actual IC datasheet
link or handoff instructions, and update the role labels in `challenge/team.txt`
with your current members. The included reward is explicitly a placeholder,
because no real datasheet was provided. Rebuild after changing the prize.

## What is included

| File or folder | Purpose |
| --- | --- |
| `Dockerfile`, `compose.yaml` | Container build, startup, resource limits, and persistent volumes |
| `challenge/` | All recruit-facing activity content and initial broken files |
| `sandbox/server.py` | Dedicated SSH server and transport restrictions |
| `sandbox/commands.py` | Exact allowlist and simplified command argument rules |
| `sandbox/files.py` | Confined file access, atomic saves, and edit conflict detection |
| `sandbox/terminal.py` | Safe Nano-style and limited Vim-style terminal editors |
| `sandbox/car.py` | Separate CAN publisher process, configuration parsing, staged diagnostics |
| `facilitator/guide.md` | Setup, answer key, hints, pacing, and teamwork observations |
| `facilitator/solve_data.py` | Reference recovery program for the facilitator |
| `prize/datasheet.txt` | Handoff text, revealed after the final passing test |
| `tests/` | Core tests and real SSH integration tests |

## Recruit experience

The virtual home is `/home/recruit`. At login it contains `commands.txt` and
`challenge/`. Recruits can explore that home and edit challenge files; they cannot
access the implementation, command binaries, `/state`, `/proc`, or other host paths.
There is no exposed `bin` folder.

Available commands are exactly:

```text
ls                    cat FILE              cd PATH
echo MESSAGE          man                   help
pwd                   ecu_protocol          check_CAN CAN_ID
check_display         charge_battery VOLTS  test_car
nano FILE             vim FILE
```

All argument counts are enforced. Flags, pipes, redirection, chaining, wildcards,
substitutions, and extra executable names are rejected. `Ctrl+D` disconnects;
`Ctrl+C` cancels a command line. Read-only instructions are protected.

`nano` and `vim` are **built-in simplified editors, not GNU Nano or full Vim**.
This is a deliberate part of the restriction: they have no shell escapes,
file browsers, alternate output paths, startup scripts, or plugins. Nano-style
controls include arrows, typing, `Ctrl+O` to save, `Ctrl+X` to exit, and `Ctrl+T`
then `Y` to clear a buffer before pasting a replacement. Vim-style controls include
`i`, Escape, arrows/hjkl, `x`, `:w`, `:q`, `:wq`, `:q!`, and `:%d`.
The command guide documents all supported controls. No undo or search is implemented.
Each text file is limited to 64 KiB; folders accept up to 128 entries through the
participant write API. Names use ASCII letters, digits, underscores, periods,
and hyphens, and cannot start with a period or hyphen.

Recruit Python files are parsed as simple Boolean/list assignments and **never
executed**. Their data-recovery programs run on their own laptops in any language.
They copy the encoded data out with `cat`, then paste recovered CSV into an editor.
Shell operator characters are permitted as ordinary editor text; they never execute.

## The car and the four stages

The CAN backend is a separate process publishing four times per second. It shares
a locked byte table with SSH sessions, while battery changes are saved under
`/state` and immediately published. Repeated `check_CAN` reads see current values.
RPM, coolant, and throttle vary gently; gear stays at 2. These are simplified,
synthetic training measurements, not a physical vehicle dynamics model.

| Stage | Initial issue | Repair |
| --- | --- | --- |
| 1 | Battery at 9.1 V; display off below 11.5 V | Decode CAN ID 1 and charge to the documented 12.6 V target |
| 2 | PDM shutdown loop discontinuous | Set `shutdown_continuous = True` in `challenge/car/pdm.py` |
| 3 | Display omits battery and coolant IDs | Include IDs `1, 2, 3, 7, 8` in `allow_list` in `challenge/car/display.py` |
| 4 | Corrupted historical DAQ recording | Speak to Jack; write a local decoder; replace `challenge/daq/data.txt` with recovered CSV |

The display reports battery volts, coolant degrees C, RPM, gear, and throttle
percentage. Classic CAN / CAN FD and scaling differences are explained in the
protocol guide. The historical DAQ recording stays fixed while live values change.
The simulator does not overwrite recruits' data edits.

The human conversation is an actual gate. After talking to the recruits, Jack or
the activity host runs this **on the Docker host**, not inside the recruit session:

```bash
docker compose exec sandbox python -m sandbox.admin approve-jack
```

That marks the conversation complete. Recruits still have to repair the data.
On success, `test_car` records the simulated DAQ fix and creates a read-only
`challenge/reward.txt` containing your handoff. The inaccessible DAQ code is a
narrative element: there is no real hardware logger to patch during the activity.

## Running teams and resetting

All sessions in one container share battery, files, and progression. Working
directories are session-specific. Saving over a teammate's newer edit is blocked;
have one person edit each file at a time. Sessions idle for 15 minutes disconnect.
Reopening an editor loses unsaved edits but preserves previously saved changes.

Create separate Compose projects and ports for independent teams, for example:

```bash
SSH_HOST_PORT=2223 docker compose -p team2 up --build -d
```

The same project name must be supplied to later management commands for that team.

```bash
docker compose exec sandbox python -m sandbox.admin status
docker compose logs --tail 50 sandbox
docker compose restart sandbox
docker compose down
```

Restart and `down` preserve saved work and the SSH host key. Rebuilding alone also
preserves the existing home volume; edits to the seed `challenge/` folder only
appear in a fresh home volume.

To **erase this team's saved work, battery state, approval, and host key**, then
start the original challenge again:

```bash
docker compose down -v
docker compose up --build -d
```

After that intentional reset the SSH host key changes. Confirm the reset with
the host before removing the old entry on the recruit's laptop:

```bash
ssh-keygen -R '[YOUR_SERVER_IP]:2222'
```

## Testing

Build the test target to install dependencies and run the core and real SSH
tests, including an OpenSSH client check:

```bash
docker build --target test -t car-electronics-sandbox-tests .
```

Or with Python 3.12+ and an OpenSSH client available locally:

```bash
python -m pip install -r requirements.txt
python -m unittest discover -s tests -v
```

Tests use temporary homes and ephemeral local ports. They do not reset or touch
an active team's container. The ordinary runtime build does not run tests;
explicitly build the test target before the activity.

Validation in the creation environment: **31 core tests passed**. These cover
stage progression, CAN updates in a real background process, exact data recovery,
the conversation gate, edits, conflict detection, parser restrictions, traversal,
symlinks, hardlinks, special files, file limits, and terminal-control filtering.
Docker was unavailable, and outbound package downloads were blocked, so the
10 dependency-backed SSH integration tests and the Docker build could not be run
there. Those tests are included for execution using the command above.

## Boundary and deployment notes

The SSH account is an application identity, not an operating-system login. The
server never invokes a shell or native editor. SSH remote exec, SFTP/SCP,
forwarding, agent/X11 requests, and environment requests are denied. Filesystem
access walks directories with `O_NOFOLLOW` and directory file descriptors;
symlinks, hardlinks, devices, hidden components, and paths above home are rejected.
The container runs as UID 10001, with dropped capabilities and a read-only root
filesystem under Compose. Only its activity home and state volumes are writable.

This is an application-level restricted environment, not a kernel chroot or
proof against vulnerabilities in Python, the SSH library, Docker, or the host.
Do not mount sensitive host paths or the Docker socket into it. Facilitators
and anyone controlling Docker are trusted; the activity is intentionally shared
and password-free. Keep Docker and dependencies maintained for your deployment.

SSH implementation reference: [Paramiko server interfaces](https://docs.paramiko.org/en/stable/api/server.html).
The pinned version follows the [Paramiko 5.0.0 release](https://www.paramiko.org/changelog.html).
