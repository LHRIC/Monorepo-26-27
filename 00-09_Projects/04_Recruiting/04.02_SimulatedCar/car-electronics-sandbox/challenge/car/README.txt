SIMULATED CAR COMPONENTS

Battery: 
supplies power. Its voltage can be read on the CAN bus and set with charge_battery.

PDM (Power Distribution Module): 
Distributes power to all the components in the car!
Checks a simplified shutdown loop. 
A continuous loop permits normal operation; a discontinuous loop reports a warning.

Display: 
receives CAN messages and displays them for dashboard readings.

DAQ (Data Acquisition):
Records and stores CAN data.
this is not a script for DAQ in this environment, but it exists on the car so I included it here

Sensor Boards:
most data on the CAN bus comes from the ECU, but we also add some sensors to different parts of the car
the sensor boards put the data from these sensors on the CAN bus

These components exist as Python scripts in this simulated car. Most of the code doesn't actually do anything here, but the program configs might have problems you have to fix...