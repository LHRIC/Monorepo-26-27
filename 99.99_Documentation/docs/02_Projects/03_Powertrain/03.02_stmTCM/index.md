## Purpose

The STM TCM, is a Transmission Control Module that is implemented using an
STM32 MCU. This board and it's accompanying software are used to control the
motor that enables electronic shifting or E-Shifting.

## Hardware

The STM TCM board incorporates the following systems and IC's:

- A SN65HVD230 CAN transceiver for communicating with the car.
- A TPS62933 buck converter module for powering the logic +3V3 rail.
- A dual NMOS MOSFET IC for driving the ECU Up and Down inputs with +5V logic.

Beyond that, the board is relatively simple and only interacts with CAN and some
GPIO.
