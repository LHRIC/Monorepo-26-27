# Custom IC

## Purpose

Custom IC was designed as a fun embedded challenge to test how recruits handle
an embedded task. This project includes ATTiny85 firmware, a data sheet for our
custom IC, and some code of the expected software solution. The ATTiny85 was
chosen because it looks a lot like the typical Op-Amp 8DIP package.

The circuit itself checks to see if recruits set up a circuit with the correct
RC time constant to then unlock a serial interface that will output the message:
"Congrats! You won trial workday!"

## Core Details

Most of the serial interface and core functionality is laid out in the data
sheet that can be found in the `doc/` folder. The main functionality not laid
out in that doc is how the ATTiny85 actually measures the time constant. Here is
that explanation:

The working idea is this, the ATTiny85 uses a comparator peripheral to trigger
an interrupt when the capacitor has charged past the ATTiny's internal reference
voltage(1.1V). Whenever we wish to measure the RC time constant, we discharge the
capacitor for some fixed time[^1]. Then we begin to charge the capacitor, by
pulling a pin tied through the resistor of the RC circuit to the pin that
measures the capacitor voltage. At the same time, we start a timer that measures
the amount of charging time in microseconds. Once the interrupt is triggered, we
stop the timer. At that point the timer measurement, using the following
formula,
$$
    \tau = -\frac{t_{timer}}{\ln(2/3)}
$$
yields the RC time constant.

The system simply checks for that constant and then begins transmitting the
success message via the serial interface.

## Issues

Some recruits this trial workday seemingly had the correct circuit and software
to interface with the device, yet they couldn't get it to work. This might be an
issue with the firmware placed on the chip. Worth looking into if this activity
is used in future trial workdays.

[^1]: There is a function that uses the ADC to ensure that the capacitor fully
    discharges before the next measurement. However, there was issues where this
    function would work *sometimes*, so it was scrapped for time.
