DAQ: DATA ACQUISITION / LOGGING

Once the display is working, check whether the logger stored readable data.
data.txt contains decimal numbers separated by whitespace. Each number represents
one corrupted byte. The whitespace between these numbers is packaging, not part
of the original text. Read the file with cat and copy the numbers to your laptop.

The original file was plain ASCII text containing comma-separated measurements.
The corruption is deterministic: the SAME reversible bit change affected EVERY
original byte, including punctuation and line breaks. None were lost or reordered.
The saved recording is independent of the currently changing CAN measurements.

Find Jack and explain what you have already fixed. He offers to fix the DAQ code
while you recover the existing recording. The DAQ implementation is not available
here: investigate the bytes themselves. Jack recommends looking at a few numbers
in eight-bit binary and comparing what stays the same.

Workflow:
1. Copy the decimal numbers from data.txt into a local text file.
2. On your laptop, write and run a program in any language to reverse the issue.
3. Inspect its output. You should see a header and 64 complete measurement rows.
4. Open nano ~/challenge/daq/data.txt.
5. Press Ctrl+T, then Y, to clear the old text from the buffer.
6. Paste the recovered CSV text itself, not your source code or decimal bytes.
7. Press Ctrl+O to save, then Ctrl+X to exit. Run test_car.

Keep your local program so Jack can ask about your approach. You can ask for a
hint when stuck; bring an example byte and explain what you have noticed.
The simulator never overwrites data.txt, so your edits stay there between tests.
