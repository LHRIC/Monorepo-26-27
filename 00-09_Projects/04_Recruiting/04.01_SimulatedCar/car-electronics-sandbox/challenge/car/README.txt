SIMULATED CAR COMPONENTS

Battery: supplies power. Its voltage can be read on CAN and set with charge_battery.

PDM (Power Distribution Module): checks a simplified shutdown loop. A continuous
loop permits normal operation; a discontinuous loop reports a warning. Inspect
pdm.py if the display mentions this. In this simulation a Boolean models the loop.

Display: receives CAN messages for dashboard readings. Its allow_list in
display.py contains the CAN IDs it is permitted to display. A missing ID shows 0,
even if the ECU is publishing the right value. Use ecu_protocol to identify IDs.

The files are intentionally small Python configurations. You only need to edit
a Boolean or a list of integers. Keep the variable names and simple assignments.
Comments beginning with # are fine. No imports, functions, or other code are needed.
Saving takes effect on the next check; no compiler, service command, or reboot.
