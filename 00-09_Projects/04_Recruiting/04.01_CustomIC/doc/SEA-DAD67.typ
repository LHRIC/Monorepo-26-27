#import "tids.typ": tids


#let metadata = (
  title: [SE Advanced Data Storage Device],
  product: "SEA-DAD67RET6",
  product_url: "https://github.com/oldrev/tids",
)

#let features = [
  - Key-Locker Secure Data Storage Device
  - 256 Lockers With 32-bit Data
  - Highly Secure 128-bit keys.
  - Full Ownership Data Retrieval Mode
  - Lost Retrieval System$trademark$
  - Optimized Flash Storage Usage
  - Backwards Compatible with SE-DAD67 Line
  - ESD Protection Exceeds JESD 22
    - 2000-V Human-Body Model (A114-A)
    - 200-V Mahcine Model (A115-A)
    - 1000-V Charged-Device Model (C101)
]

#let applications = [
  Common applications of the SEA-DAD67 include:

  - Military Data Security
  - Aerospace Data Redundancy
  - Password Protected Embedded Secrets
  - Automotive Locking Systems
  - Trade Secret Protection
]

#let description = [
  The SEA-DAD67, part of the Sawcon Advanced Line, is an upgraded version of the
  SE-DAD67 that improves upon flash usage and utilizes advanced CMOS technology.
  It also adds the proprietary Lost Key Retrieval System$trademark$.

  The SEA-DAD67 enables storage of sensitive data on embedded deices using a
  key-locker system. Locker addresses and locker keys are entered using a shift
  register with accessible clock and stored data can be accessed using the same
  register where data is instead shifted out.

  In the event that a key is lost, the Lost Key Retrieval System$trademark$ can
  be used to retrieve data.
]

#let rev_list = (
  (
    rev: [REV1],
    date: [2012/12/12],
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
    [2], [$M_0$], [Mode Select bit 0.],
    [3], [$M_1$], [Mode Select bit 1.],

    [4], [$"GND"$], [Ground reference.],
    [5], [$"DATA"$], [Bidirectional serial data. Key and address bits shifted in; locker contents shifted out.],
    [6],
    [$overline("VALID")$ / $"RC"_"IN"$],
    [Open-drain valid flag. Low whenever output data or store operation is valid. Acts as the RC input whenever the device
      is in Lost Key Retrieval Mode.],

    [7], [$"CLK"$], [Serial clock input. Data is latched on the rising edge.],
    [8], [$V_"CC"$], [Supply voltage.],
  ),
  caption: [SEA-DAD67RET6 pin functions, 8-pin PDIP],
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
= Detailed Description
<DetailedDecscription>

== Startup Operating Mode Selection

Upon startup the $M_0$ and $M_1$ pins are used to determine what operating mode
the system will boot into. The following table describes the modes.

#figure(
  table(
    columns: (3em, 3em, auto, auto),
    align: (center, left, left, left),
    stroke: 0.5pt,
    table.header([*$M_0$*], [*$M_1$*], [*Operating Mode*], [*Description*]),
    [0],
    [0],
    [Standard Operation],
    [Device acts according to the standard operating
      procedures.],

    [0],
    [1],
    [Data Destruction Mode],
    [Destroys all locker data and boots into
      standard operation mode.],

    [1],
    [0],
    [Lost Key Retrieval System $trademark$],
    [Boots into the Lost Key
      Retrieval operating mode.],

    [1], [1], [Lockout], [All device pins are pulled high.],
  ),
)

== Standard Operation Device Modes

Whenever the system is in standard operation mode, the following modes can be
selected to access the locker. The device operates in one of the following modes depending on the status of the
Mode Select Pins.

#figure(
  table(
    columns: (3em, 3em, auto, auto),
    align: (center, left, left, left),
    stroke: 0.5pt,
    table.header([*$M_0$*], [*$M_1$*], [*Device Mode*], [*Description*]),
    [0],
    [0],
    [Data-Address-In Mode],
    [Data is shifted in to the Data Holding
      Register. The output of the Data Holding Register is shifted into the
      Address Holding Register.],

    [0],
    [1],
    [Key-In Mode],
    [The 128-bit key is shifted in to the key holding
      register.],

    [1],
    [0],
    [Data-Store Mode],
    [Loads the current value of the Data Holding Register into the locker at
      the address stored in the Address Holding Register when clocked. If
      the key was incorrect, then the $overline("VALID")$ bit is not pulled low.],

    [1],
    [1],
    [Data-Read Mode],
    [Shifts the stored data out of the locker pointed to by the address stored
      in the Address Holding Register via the $"DATA"$ pin. If
      the key was incorrect, then the $overline("VALID")$ bit is not pulled low.],
  ),
  caption: "Standard Operation Device Modes",
)

== Lost Key Retrieval Mode

When the device is booted into the Lost Key Retrieval System $trademark$, all
system pins are pulled low excluding the $"CLK"$ pin and the, now reconfigured,
$"RC"_"IN"$ pin.

The following procedure can then be followed to unlock all lockers without using
the respective key.

+ Attach applicable RC time-constant circuit to the $"RC"_"IN"$ pin.
+ Send a pulse on the $"CLK"$ pin.
+ Continue steps 1 and 2, until the full RC sequence has been complete
+ On the last edge, wait for all pins of the IC to be asserted high
+ Await a minum of 5 seconds for the system to reconfigure pins into unlocked mode

Once the system is loaded into unlocked mode, the system acts as though it is in
standard operation mode, however no keys are checked.
