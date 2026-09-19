Upright CAN boards integrate 4 sensors onto the wheel upright to validate Dynamics' design goals. Each board goes on four separate corners of the car.
Sensor list:
1. IMU (I2C)
2. Hall effect (wheel speed) (analog)
3. Shock pots (potentiometer) (analog)
4. Brake temp (IR sensor) (analog)

All of these sensors feed into an STM32F373CCT6, which broadcasts telemetry across the CAN bus to the ESP-DAQ board.