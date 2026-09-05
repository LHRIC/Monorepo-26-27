# Facilitator guide — contains answers

Keep this folder away from recruits. It is not copied into the runtime image.
Use one container per team so teams cannot solve one another's car.

## Prepare

1. Replace `prize/datasheet.txt` with your IC datasheet link or handoff.
2. Put the actual current team members in `challenge/team.txt`.
3. Build the Docker test target, then launch a fresh Compose project.
4. Rehearse from a recruit laptop, including copy/paste into the editor.
5. Give recruits the SSH command and only `cat commands.txt` as the starting hint.

Suggested pilot duration: 45–60 minutes. Allow roughly 5 minutes to explore,
5–10 for the battery and PDM, 5–10 for the display, and 20–30 for data recovery
and discussion. These are planning estimates; pilot with a novice team and
adjust the hint timing. The simple corruption may be quick for an experienced
programmer; ask everyone to explain the diagnosis and verify the output together.

## Stages and hints

**Battery:** CAN ID 1 initially returns `01011011`, decimal 91. Divide by 10:
9.1 V. The documented normal target is 12.6 V. `charge_battery 12.6` repairs stage 1.
The display's turn-on threshold is 11.5 V, so other values at or above it also pass.
Ask: “What supplies the display?” then “Which command reads the underlying data?”
If necessary, walk through one unrelated binary conversion example.

**PDM:** `check_display` shows “PDM Shutdown Discontinuous.” In
`challenge/car/pdm.py`, change `shutdown_continuous = False` to `True`.
This Boolean represents the simulator's loop condition; recruits are not being
taught to bypass a real vehicle safety system. Ask what a continuous loop means.

**Display:** battery and coolant show zero while CAN reports valid measurements.
In `challenge/car/display.py`, include `[1, 2, 3, 7, 8]` in `allow_list`.
Order and duplicate IDs do not matter. Ask how one could distinguish an ECU
measurement failure from a display filtering problem.

**DAQ / required conversation:** `test_car` directs them to Jack. Ask them to
explain their previous tests and to ask at least one question about the logger.
Tell them the intended file is ASCII CSV with a header and 64 measurement rows.
The damaged file contains decimal representations of bytes, not decimal
measurements. A single bit fault affected every byte, including line breaks.
Then record the conversation from the host:

```bash
docker compose exec sandbox python -m sandbox.admin approve-jack
```

This command requires host/container management access. It is unavailable through
the recruit command interface, and repairing the data without approval will not pass.
For a named team use `docker compose -p team2 exec ...`.

Jack's prompt: “That's a silly logger bug. I'll fix the DAQ code while you recover
the old recording. Look at a few bytes in binary. You can use any language on
your laptop. Replace data.txt with the recovered text and show me how you did it.”

Give hints gradually, based on what they have tried:

1. Print several values as eight-bit binary. Is one bit always the same?
2. Compare the values with the ASCII range and the binary for a common character.
3. ASCII uses values below 128, so its high bit is zero. What is the high bit here?
4. Clear bit 7 for every byte. XOR with 128, AND with 127, or subtracting 128 all
   recover this particular dataset, because every encoded byte has bit 7 set.
5. Parse whitespace-separated decimal tokens, transform each byte, turn bytes
   into text, and preserve punctuation and newlines.

The reference program is `facilitator/solve_data.py`. On your laptop:

```bash
python facilitator/solve_data.py challenge/daq/data.txt recovered.csv
```

The first recovered line is:

```text
sample,battery_v,coolant_c,rpm,gear,throttle_pct
```

There are exactly 64 data rows and 1,383 original bytes. The validator checks
the full original recording; it accepts CRLF or LF and an optional final newline.
It does not accept partially repaired data or just plausible-looking values.
Current CAN values do not have to match the historical recording.

Recruits replace the file using `nano ~/challenge/daq/data.txt`, `Ctrl+T`, `Y`,
paste, `Ctrl+O`, `Ctrl+X`. Their local source code is not uploaded or executed.
They should keep the program open for discussion. A passing test releases the
handoff and marks the simulated logger repair complete. There is no real DAQ
source patch required from Jack: the activity models that outcome.

## Observe teamwork

Record concrete examples: a hypothesis before a command, a useful comparison,
a clear request for help, a shared explanation, a teammate invited to contribute,
or recovery after a mistaken edit. Do not use speed, prior Linux knowledge, or
the number of hints as the primary score. Ask each recruit to explain one step.
Rotate the keyboard after the display works so the coding step is not monopolized.

Useful debrief questions: “Which observation ruled out your first idea?”
“How did you know your recovery was correct?” “What would you test next if one
measurement still looked wrong?” “What did someone else notice that you missed?”

## Common operational fixes

- Wrong SSH username: use `recruit`. There is no root/admin SSH login.
- Remote command/SFTP failure: expected; enter an interactive SSH session.
- Can't paste: use the terminal's Paste menu. Mac uses Command+V; other terminal
  shortcuts differ. Open an editor before pasting a whole file.
- Vim confusion: `Ctrl+X` works there too. Use Nano next time.
- Concurrent save error: preserve the edits locally, exit without saving, reopen
  the teammate's latest version, and agree on a merged edit.
- Python syntax error: keep only the supplied assignment and comments. There is
  no need to add imports or functions to a car configuration file.
- Correct-looking data still fails: check the entire file, header, all 64 rows,
  punctuation, and extra whitespace. Confirm the Jack conversation is approved.
- Restart did not reset: expected; volumes retain state. The README documents
  the explicit destructive reset for use between teams.
- Datasheet placeholder appears: update `prize/datasheet.txt`, rebuild/recreate
  the service, then have recruits run `test_car` again to refresh the handoff.
