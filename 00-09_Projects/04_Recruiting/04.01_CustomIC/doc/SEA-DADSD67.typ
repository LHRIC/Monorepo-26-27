#import "tids.typ": tids
#import "@preview/digidraw:0.9.3" as dd

#let metadata = (
  title: [SE Advanced Discrete Analogue Data Storage Device],
  product: "SEA-DADSD67RET6",
  product_url: "https://github.com/oldrev/tids",
)

#let features = [
  - RC Locker Device
  - Storage of up to 512 bytes
  - Highly Secure Analogue Key (up to 3 decimal places)
  - Full Ownership Data Retrieval Mode
  - Optimized Flash Storage Usage
  - Backwards Compatible with SE-DAD67 Line
  - ESD Protection Exceeds JESD 22
    - 2000-V Human-Body Model (A114-A)
    - 200-V Machine Model (A115-A)
    - 1000-V Charged-Device Model (C101)
]

#let applications = [
  Common applications of the SEA-DADSD67 include:

  - Military Data Security
  - Aerospace Data Redundancy
  - Password Protected Embedded Secrets
  - Automotive Locking Systems
  - Trade Secret Protection
]

#let description = [
  The SEA-DADSD67, part of the Sawcon Advanced Line, is an upgraded version of the
  SE-DAD67 that improves upon flash usage and utilizes advanced CMOS technology.

  The SEA-DADSD67 enables storage of sensitive data on embedded deices using an
  analogue key system. The user can lock sensitive data behind an arbitrary RC
  time constant ($tau_"lock"$), and later unlock that data with the same constant.

  The SEA-DADSD67 measures time constant using an ultra-precise internal
  reference clock allowing for a time constant with up to 3 decimal places of precision.
]

#let rev_list = (
  (
    rev: [REV2],
    date: [2021/9/3],
    body: [
      - Fixed Typo in @TitlePageFeatures
    ],
  ),
  (
    rev: [REV1],
    date: [2021/8/12],
    body: [
      - Initial release
    ],
  ),
)

#show: doc => tids(
  ds_metadata: metadata,
  features: features,
  applications: applications,
  desc: description,
  rev_list: rev_list,
  doc: doc,
)

= Pin Configuration and Functions

#figure(
  table(
    columns: (auto, 1fr, 1fr),
    align: (center, left, left),
    stroke: 0.5pt,
    table.header([*Pin*], [*Name*], [*Description*]),
    [1], [$overline("RESET")$], [Active-low device reset. Internally pulled high.],
    [2],
    [$"RC Driver"$],
    [Connected to a resistor to the $"RC"$ pin. Charges
      the capacitor during RC measurement.],

    [3], [$"RC"$], [RC time constant input.],

    [4], [$"GND"$], [Ground reference.],
    [5], [$"Mode"$], [Selects which mode the chip will boot into on reset.],

    [6], [$"DATA"$], [Bidirectional serial data input and output.],

    [7],
    [$"CLK"$],
    [Serial clock input. Data is latched and advanced on the rising
      edge.],

    [8], [$V_"CC"$], [Supply voltage.],
  ),
  caption: [SEA-DADSD67RET6 pin functions, 8-pin PDIP],
)

= Absolute Maximum Ratings
_Over operating free-air temperature range (unless otherwise noted). Stresses beyond these ratings may cause permanent damage to the device._

#figure(
  table(
    columns: (1fr, auto, auto),
    align: (left, center, center),
    stroke: 0.5pt,
    table.header([*Parameter*], [*Rating*], [*Unit*]),
    [Supply voltage, $V_"CC"$], [$-0.5$ to $6.0$], [V],
    [Voltage on any pin except RESET], [$-0.5$ to $V_"CC" + 0.5$], [V],
    [Voltage on RESET pin], [$-0.5$ to $13.0$], [V],
    [DC current per I/O pin], [40.0], [mA],
    [DC current, $V_"CC"$ and GND pins], [200.0], [mA],
    [Storage temperature, $T_"STG"$], [$-65$ to $150$], [°C],
    [Lead temperature (soldering, 10 s)], [260], [°C],
    [ESD, Human-Body Model (JS-001)], [2000], [V],
    [ESD, Charged-Device Model (C101)], [1000], [V],
  ),
  caption: [Absolute maximum ratings],
)

= Recommended Operating Conditions

#figure(
  table(
    columns: (auto, 1fr, auto, auto, auto, auto),
    align: (left, left, center, center, center, center),
    stroke: 0.5pt,
    table.header([*Symbol*], [*Parameter*], [*Min*], [*Nom*], [*Max*], [*Unit*]),
    [$V_"CC"$], [Supply voltage], [2.7], [5.0], [5.5], [V],
    [$V_"CC(20)"$], [Supply voltage for $f_"CLK" = 20$ MHz operation], [4.5], [5.0], [5.5], [V],
    [$V_"IH"$], [High-level input voltage], [$0.6 times V_"CC" - 0.5$], [], [$V_"CC" + 0.5$], [V],
    [$V_"IL"$], [Low-level input voltage], [$-0.5$], [], [$0.2 times V_"CC" - 0.1$], [V],
    [$f_"CLK"$], [Serial clock frequency], [0], [], [20], [MHz],
    [$T_A$], [Operating free-air temperature], [$-40$], [25], [85], [°C],
  ),
  caption: [Recommended operating conditions],
)

= Electrical Characteristics
_At recommended operating conditions unless otherwise noted._

#figure(
  table(
    columns: (auto, 1fr, auto, auto, auto, auto, auto),
    align: (left, left, left, center, center, center, center),
    stroke: 0.5pt,
    table.header([*Symbol*], [*Parameter*], [*Test Conditions*], [*Min*], [*Typ*], [*Max*], [*Unit*]),
    [$I_"CC"$], [Supply current, active], [$f_"CLK" = 4$ MHz, $V_"CC" = 3$ V], [], [1.5], [3.0], [mA],
    [$I_"CC"$], [Supply current, active], [$f_"CLK" = 8$ MHz, $V_"CC" = 5$ V], [], [5.0], [8.0], [mA],
    [$I_"CC(SB)"$], [Supply current, power-down], [$V_"CC" = 3$ V, WDT disabled], [], [0.1], [1.0], [µA],
    [$V_"OH"$], [High-level output voltage], [$I_"OH" = -20$ mA, $V_"CC" = 5$ V], [$V_"CC" - 0.7$], [], [], [V],
    [$V_"OL"$], [Low-level output voltage], [$I_"OL" = 20$ mA, $V_"CC" = 5$ V], [], [], [0.7], [V],
    [$I_"IL"$], [Input leakage current, I/O pin], [$V_I$ = 0 or $V_"CC"$], [], [], [1], [µA],
    [$R_"PU"$], [Pull-up resistor, I/O pin], [], [20], [], [50], [kΩ],
    [$t_"CYC"$], [Locker read/write cycle time], [], [], [180], [250], [µs],
    [$t_"RET"$], [Data retention time], [$T_A = 55degree$C], [10], [], [], [years],
    [$N_"END"$], [Endurance, write/erase cycles per locker], [], [100000], [], [], [cycles],
  ),
  caption: [DC and locker-access electrical characteristics],
)

= Switching Characteristics
_$V_"CC" = 5.0$ V, $T_A = 25degree$C unless otherwise noted._

#figure(
  table(
    columns: (auto, 1fr, auto, auto, auto, auto),
    align: (left, left, center, center, center, center),
    stroke: 0.5pt,
    table.header([*Symbol*], [*Parameter*], [*Min*], [*Typ*], [*Max*], [*Unit*]),
    [$t_"CH"$], [Clock high time], [25], [], [], [ns],
    [$t_"CL"$], [Clock low time], [25], [], [], [ns],
    [$t_"SU"$], [Data setup time], [10], [], [], [ns],
    [$t_"H"$], [Data hold time], [10], [], [], [ns],
    [$t_"RESET"$], [Minimum reset pulse width], [2.5], [], [], [µs],
  ),
  caption: [AC switching characteristics],
)

= Serial Data Interface
<SerialDataInterface>

The SEA-DADSD67RET6 uses a standard serial data interface. Internally all serial
data is stored in an 8-bit shift register. When sending data to the chip, data is
shifted into the most signifcant bit (MSB) first and out of the least
significant bit (LSB) last. When reading data from the chip, data is shifted out
of the MSB first and LSB last.

When shifting in data, the $"DATA"$ pin is sampled on a rising edge of the
$"CLK"$ pin. This increments the shift register counter, which tracks the number
of shifted bits. After 8 bits have been shifted into the shift register, a full
byte, the shift register counter will overrun and the data stored in the shift
register will be registered as a command or data. At the same time, the value of
the shift register is also zeroed.
#figure(
  dd.wave(
    (
      signal: (
        (wave: "0P.......0.", name: "CLK"),
        (wave: "1..01010.x.", name: "DATA"),
        (wave: "=========..", name: "Shift Register Counter", data: "0 1 2 3 4 5 6 7 0 0"),
        (wave: "=========..", name: "Shift Register Value", data: "x00 x80 xC0 xE0 x70 xB8 x5C xAE x00 x00 x00"),
        (wave: "x.......=x.", name: "Command/Data", data: "x57"),
      ),
    ),
  ),
  caption: "Data In Serial Interface Timing Diagram",
)

If the programmer interfacing the chip looses track of what bit they are on, the
contents of the shift register can be cleared by holding the $"CLK"$ pin high
for at least 1 second.

When shifting out data, the $"DATA"$ pin transparently shows what is in the MSB
of the shift register. Whenever there is a rising edge on the $"CLK"$ pin, the
MSB is shifted out, and the next piece of data becomes available. After one byte
of data has been shifted out, the next byte is immediately loaded into the shift
register. An internal counter keeps track of which byte of data is currently in
the shift register.

#figure(
  dd.wave(
    (
      signal: (
        (wave: "0P.......0", name: "CLK"),
        (wave: "0.10101.0.", name: "DATA"),
        (wave: "=========.", name: "Shift Register Value", data: "x57 xAE x5C xB8 x70 xE0 xC0 x80 x0A"),
        (wave: "=.......=.", name: "Current Byte", data: "x57 x0A"),
      ),
    ),
  ),
  caption: "Data Out Serial Interface Timing Diagram",
)

To indicate how many bytes are part of the data transmission the byte 0x80 is
used as a predicate to indicate one of the following things:

#figure(
  table(
    columns: (auto, auto),
    align: (left, left),
    stroke: 0.5pt,
    table.header([*Byte*], [*Meaning*]),
    [0x80], [The byte 0x80 was sent; ignore the predicate.],
    [0xAA], [Start of Data Transmission.],
    [0xBB], [Stop of Data Transmission.],
    [0xCC], [Loop around, all the data has been shown and will now be shown again.],
  ),
  caption: [AC switching characteristics],
)

Below is an example of a serial data out transmission of the word "Hello" in ascii:

#figure(
  dd.wave(
    (
      signal: (
        (wave: "x==========", name: "Data", data: "0x80 0xAA 'H' 'e' 'l' 'l' 'o' 0x80 0xBB"),
      ),
    ),
  ),
  caption: "Example Data Transmission",
)

In the case that the programmer interfacing with the chip looses track of what
bit they are on, the $"CLK"$ line can be held high for at least 1 second to reset transmission.

= Detailed Description
<DetailedDecscription>

== Startup Operating Mode Selection

Upon startup the $M$ pin is checked to determine what operating mode the system
enable_serial();
will boot into. The following table describes the modes.

#figure(
  table(
    columns: (3em, auto, auto),
    align: (center, left, left),
    stroke: 0.5pt,
    table.header([*$M$*], [*Operating Mode*], [*Description*]),
    [0],
    [Lock Data Mode],
    [Device listens for a RC constant. Upon transmission of the lock signal via
      serial, the RC constant is saved as $tau_"lock"$. Data to be stored can be sent
      via serial.],

    [1],
    [Unlock Data Mode],
    [Device reads the given RC constant and checks for $tau_"lock"$. Upon unlock,
      data can be read out via serial data pins.],
  ),
)

Changes to the $M$ pin after startup do not correspond in changes of operating
mode. If you wish to swtich operating mode, the reset pin should be driven low.

== Lock Data Mode

The Lock Data Mode is used for locking some amount of data ($<= 512$ bytes).
Upon entering the Lock Data Mode, previously stored data is not immediately
destroyed. However, to allow for quick data destruction in certain scenarios,
upon the first $"CLK"$ rising edge, all data currently stored will be erased.

The Lock Data Mode waits for a specific code that acts as the start byte to set
the RC lock constant ($tau_"lock"$). See @SerialDataInterface for explanation on
how to send data and commands to the chip. Once the byte 0x88 has been sent to
the chip, the RC time constant will be measured and recorded. The chip will not
listen for any data until this process has been finished, this will be indicated
by the $"MEASURED"$ pin, being driven high.

#figure(
  table(
    columns: (auto, auto, auto),
    align: (left, left, left),
    table.header(
      [*Time Constant $tau$*],
      [*Resistor Value $R$*],
      [*Capacitor
      Value $C$*],
    ),
    $1m s$, $1k Omega$, $1 mu F$,
    $100 m s$, $10 k Omega$, $10 mu F$,
    $100 m s$, $100 k Omega$, $1 mu F$,
    $1 s$, $1 M Omega$, $1 mu F$,
  ),
  caption: "Common RC Time Constants",
)

After the $tau_"lock"$ has been specified, the chip will begin writing bytes to
it's internal memory as they are shifted in over serial. If the programmer sends
in more than 512 bytes, the chip will wrap around and start writing over the
first set of bytes. Once done transmitting data to be locked, the $"CLK"$ pin
should be pulled high for greater than 1 second to signify the end of
transmission. The chip will not listen to any inputs after this condition, and
the user should reset the chip to boot into unlock mode.

== Unlock Data Mode

The Unlock Data Mode is used for reading data from the chip after it has already
been stored to in Lock Data Mode. Upon boot, the system, by default, starts with
the serial interface fully locked down. After boot up, the system will begin to
test for $tau_"lock"$ repeatadly until it gets the correct value.

This measurement is performed by the chip driving the $"RC Driver"$ pin. The
intended application circuit involves connecting the $"RC Driver"$ pin to the
$"RC"$ pin through a resistor, the $R$, and connecting the $"RC Driver"$ pin to
ground through a capacitor.

It is recommended that the value of ocompanying resistors and capacitors used
for unlocking the device be obfiscated in the hardware layout. The following
methods can be used:

- Pair multiple resistors and capacitors in series.
- Use components with intentionally large tolerances.
- Route the RC ciruit traces through an interior plane of the PCB.

Once the $"RC"$ pin has measured $tau_"lock"$ sucessfully, the chip will become
unlocked for the remainder of the time it is powered on, or until reset. The
$"UNLOCKED"$ pin will be pulled high to indicate that the chip is ready for data
transmission. Prior to the chip being unlocked the system will not output
anything from its serial data port.

After the system is unlocked, the serial interface will become active and will
repeatedly output the stored value acording to @SerialDataInterface.

== Programmable Fuses

The SEA-DADSD67 has a set of programmable fuses that can change the
functionality of the device. The following table describes the fusing registers:

#figure(
  table(
    columns: (auto, auto, auto),
    align: (left, left, left),
    stroke: 0.5pt,
    table.header([*Bits*], [*Name*], [*Device*]),
    $0$,
    [Disable Lock],
    [Disables the locking functionality of the chip, such
      that it cannot be written to or have it's RC time constant changed.],

    $1$,
    [Enable Circular Flash],
    [Enables circular flash overwriting. Flash
      segments are overwritten in a circular order using a stepped pointer.],

    $2 \- 7$, [Reserved], [Reserved for future use.],
  ),
)

= Package Information

The SEA-DAD67RET6 is offered in the standard 8-pin through-hole package for prototyping and legacy socket compatibility, as well as several 8-pin surface-mount packages for volume production.

#figure(
  table(
    columns: (auto, 1fr, auto, auto, auto),
    align: (left, left, center, center, center),
    stroke: 0.5pt,
    table.header([*Package*], [*Description*], [*Pitch*], [*Body Size*], [*Height*]),
    [PDIP-8], [Plastic dual in-line package], [2.54 mm], [9.81 × 6.35 mm], [4.83 mm],
    [SOIC-8 (NB)], [Small-outline IC, narrow body], [1.27 mm], [4.90 × 3.90 mm], [1.75 mm],
    [TSSOP-8], [Thin shrink small-outline package], [0.65 mm], [3.00 × 4.40 mm], [1.20 mm],
    [MSOP-8], [Micro small-outline package], [0.65 mm], [3.00 × 3.00 mm], [1.10 mm],
    [DFN-8 (2×3)], [Dual flat no-lead package], [0.50 mm], [2.00 × 3.00 mm], [0.75 mm],
  ),
  caption: [Available packages, SEA-DAD67RET6],
)

== Ordering Information

#figure(
  table(
    columns: (1fr, auto, auto, auto),
    align: (left, center, center, center),
    stroke: 0.5pt,
    table.header([*Orderable Part Number*], [*Package*], [*Package Qty*], [*Temp Range*]),
    [SEA-DAD67RET6-P], [PDIP-8], [Tube, 50], [$-40degree$C to $85degree$C],
    [SEA-DAD67RET6-SO], [SOIC-8], [Tape and reel, 2500], [$-40degree$C to $85degree$C],
    [SEA-DAD67RET6-TS], [TSSOP-8], [Tape and reel, 3000], [$-40degree$C to $85degree$C],
    [SEA-DAD67RET6-MS], [MSOP-8], [Tape and reel, 3000], [$-40degree$C to $85degree$C],
    [SEA-DAD67RET6-DF], [DFN-8], [Tape and reel, 5000], [$-40degree$C to $85degree$C],
  ),
  caption: [Ordering information by package option],
)

== PDIP-8 Mechanical Data

#figure(
  table(
    columns: (auto, 1fr, auto),
    align: (left, left, center),
    stroke: 0.5pt,
    table.header([*Symbol*], [*Dimension*], [*Value*]),
    [A], [Overall height], [4.83 mm max],
    [A2], [Body height], [3.30 mm typ],
    [b], [Lead width], [0.46 mm typ],
    [e], [Lead pitch], [2.54 mm],
    [D], [Body length], [9.81 mm typ],
    [E1], [Body width], [6.35 mm typ],
    [E], [Overall width (lead tip to lead tip)], [7.87 mm typ],
    [L], [Lead length], [3.30 mm typ],
  ),
  caption: [PDIP-8 mechanical dimensions],
)

#v(0.5em)
_Package thermal and mechanical data is provided for design reference only. Refer to the package drawing for tolerancing prior to PCB footprint or socket design._
