#import "@preview/polylux:0.4.0": *
#import "@preview/wavy:0.1.3"

#set page(
  paper: "presentation-16-9",
  margin: 2cm,
)

#show heading: set text(font: "Montserrat")
#set text(size: 18pt, font: "Roboto")

#show raw.where(lang: "wavy"): it => scale(
  wavy.render(it.text),
)

#show raw.where(lang: "dbc"): set raw(
  syntaxes: "./can.sublime-syntax",
)

#show raw.where(block: true): block.with(
  fill: luma(240),
  inset: 10pt,
  radius: 4pt,
)

#slide[
  #set align(horizon)
  = CAN / DAQ Lecture
  #uncover(2)[
    == or, how do things on car talk to each other and make data happen please
  ]
]

#slide[
  = Overview

  #toolbox.all-sections((sections, current) => {
    enum(..sections)
  })
]

#slide[
  #set align(horizon)
  #toolbox.register-section("Digital Communication")
  = Digital Communication
]

#slide[
  = How do digital systems communicate?

  #uncover(2)[
    There are 4 main ways we encode information, when communicating between
    digital systems:

    + Parallel
    + Serial
    + Analogue
    + Time
  ]
]

#slide[
  = Parallel

  #figure(
    image("assets/parallel_interface.svg", width: 50%),
  )

  - Data is available all at once
  - Takes up lots of space/resources, but is faster
  - Good for things like memory, where speed is necessary
]

#slide[
  = Serial

  #figure(
    image("assets/serial_interface.svg", width: 50%),
  )

  - Data is encoded over time
  - Takes up less space/resources, but is limited by the speed of the clock
  - Good for most everything
]

#slide[
  = Analogue

  #figure(
    image("assets/analogue_interface.svg", width: 50%),
  )

  - Data is encoded in a continuous current/voltage
  - Data is always available, but it's up to the MCU how often it checks the value
  - Very susceptible to noise
  - How most sensors work
]

#slide[
  = Time

  #figure(
    image("assets/time_interface.svg", width: 50%),
  )

  - Data is a period, frequency, pulse width, or phase shift
  - Good for things like fans, and controlling power
  - Not really used for much else
]

#slide[
  = Some more on Serial

  Serial is the most widely used way of interfacing between digital systems.
  There are a few terms we use to describe serial interfaces:

  #item-by-item(start: 2)[
    - Synchronous/Asynchronous: Is there a dedicated clock line or not?
      - Baud Rate: When asynchronous, both devices must agree upon the speed
        they want to send and receive data.
    - Full/Half Duplex: Can communication happen between both systems at once
  ]
]

#slide[
  = Serial Peripheral Interface (SPI)

  SPI is an example of a _synchronous_ communication protocol. An SPI
  Transaction might look like this:

  #figure(
    ```wavy
      {
        signal: [
          {name: "SCLK", wave: '0.P.......0.'},
          {name: "PICO", wave: 'x.1010.1.0x.', phase: "0.5"},
          {name: "POCI", wave: 'x...........'},
          {name: "\\CS", wave: '10.........1'}
        ],
      }
    ```,
  )

  SPI also has different modes that change the clock polarity and phase. This
  example showed mode 00.
]

#slide[
  = Universal Asynchronous Receiver Transmitter (UART)

  UART is an example of a _asynchronous_ communication protocol. A UART
  transaction might look like this:

  #figure(
    ```wavy
      {
        signal: [
          {name: "TX", wave: '1.01010.1.01..'},
          {name: "RX", wave: '1.0.10.10..1..'},
        ]
      }
    ```,
  )

]

#slide[
  #toolbox.register-section("CAN")
  #set align(horizon)

  = Controller Area Network
]

#slide[
  = What interface does CAN use?

  #uncover((2, 3, 4))[
    CAN is a serial interface that is is asynchronous. It's not exactly half
    duplex, but it uses a message based system.
  ]

  #uncover((3, 4))[
    There are two connections that CAN uses:

    - CAN High
    - CAN Low

    Because CAN only needs two wires, it is very easy to connect it between
    many devices.
  ]

  #uncover(4)[
    CAN High and Low are routed as a *differential pair*.
  ]
]

#slide[
  = What is a differential pair?

  #uncover(2)[
    A differential pair is a set of two signal lines which are routed next to
    each other and read as a differential.

    This helps to protect our system from noise due to electromagnetic
    interference.
  ]

]

#slide[
  = An EMI Event

  An EMI event might look something like this:

  #figure(
    image("assets/emi_event.svg", width: 50%),
  )

  If our microcontroller was interfacing with another during that period, then
  it would read a 1 when it should've read a 0.
]

#slide[
  = An EMI Event with a differential pair

  A similar EMI Event can happen with a differential pair, and the following happens:

  #figure(
    image("assets/differential.svg", width: 45%),
  )

  As you can see, since both signals are affected by the EMI event in the same
  way, since they are physically close to each other, therefore the differential
  between them stays the same.
]

#slide[
  = The Different CAN Bus States

  #figure(
    image("assets/can_states.svg", width: 70%),
  )
]

#slide[
  = CAN Data Flow

  CAN Bus uses a broadcasting model, which means that all messages sent over the
  bus are made available to everyone attached to the bus. It is up to individual
  devices to see if they want to listen.

  Messages contain an ID, and 0 to 8 bytes of data.

]



#slide[
  = CAN Packet Structure

  #figure(
    image("assets/can_packet.png", width: 80%),
  )

  #item-by-item()[
    - Start the frame by putting the bus into the dominant state
    - Start sending frame ID, lower values take priority
    - Send how many bytes you intend to send plus some other information
    - Send the data bytes
    - Send a CRC
    - Send the end of frame
  ]
]

#slide[
  = Quick Message on Arbitration

  Say multiple controllers want to use the bus at once, who wins?

  #uncover((2, 3))[
    The lowest ID takes priority, because it will drive the bus for longer.
    Remember that a driven bus is a logical 0.
  ]

  #uncover(3)[
    Multiple devices should not send messages using the same ID.
  ]
]

#slide[
  = Physical Setup of CAN

  CAN High and Low are routed as twisted pairs throughout the car. At each end
  of the bus we have a 120 Ohm resistor placed across CAN High and Low referred
  to as the *terminating resistor*#footnote[This has to do with preventing
    signal reflections].

  CAN is a long daisy chain from device to device. You can
  have stubs that branch from the bus, with no terminating resistor, but they
  should be under a foot in length.
]

#slide[
  = Debugging CAN

  If you aren't able to get data over CAN you can debug these things:

  - Is there 60 Ohms across High and Low?
  - Are High and Low shorted?
  - When the bus is active: is CAN High and Low in the recessive state (2.5V)?
]

#slide[
  #toolbox.register-section("Using CAN with an MCU")

  #set align(horizon)

  = Using CAN with an MCU
]

#slide[
  = The CAN Transceiver

  Most MCU's, and definitively 3.3V logic MCU's, don't have a CAN transceiver on
  the chip.

  Thus we need an external CAN Transceiver.
]

#slide[
  = Setting up the Transceiver

  A typical setup looks something like this:

  #figure(
    image("assets/can_to_mcu.svg", width: 60%),
  )

  CAN RX and TX are the same signals from CAN, but converted to our
  microcontroller's logic level.
]

#slide[
  = Routing

  Route CAN as a differential pair and follow the same rules you would normally:

  #item-by-item(start: 2)[
    - Keep traces equal in length
    - Avoid vias, and if you use a via do so on both CAN high and low
    - Keep away from noise (buck converters)
  ]
]

#slide[
  #figure(
    image("assets/differential_routing.png"),
  )
]

#slide[
  = CAN FD

  We specifically use CAN-FD which is a superset of regular CAN. It allows for
  the following things:

  #item-by-item(start: 2)[
    - Larger Payloads: Up to 64 bytes per frame
    - Bit Rate Switching: Standard speed is used for arbitration, further
      transactions can be done at up to 8Mbit/s
    - Backward compatible with regular CAN
  ]
]

#slide[
  = Programming with CAN

  Make sure that you've got the correct setup to run CAN at a 1Mbit/s baud rate.
  \
  I use: www.bittiming.can-wiki.info

  A quick demo!
]

#slide[
  = CAN DBC

  Each CAN message has a specific ID. This ID tells you what the bytes
  associated with the CAN message mean.

  The CAN DBC is a big list that tells you what every message ID means. This
  exists for cars, boats, machines, and, of course, the LHR car.
]

#slide[
  = An Example DBC Entry

  Here's what the DBC Entry on the car for ID 360 looks like:

  ```dbc
  BO_ 864 id_360: 8 Vector__XXX
    SG_ Coolant_Pressure : 55|16@0+ (0.1,-101.3) [-101.3|6452.2] "kPa" Vector__XXX
    SG_ Throttle_Position : 39|16@0+ (0.1,0) [0|6553.5] "%" Vector__XXX
    SG_ Manifold_Pressure : 23|16@0+ (0.1,0) [0|6553.5] "kPa (Abs)" Vector__XXX
    SG_ RPM : 7|16@0+ (1,0) [0|65535] "RPM" Vector__XXX
  ```
]

#slide[
  = A Better way of viewing the DBC

  The LHRIC Monorepo has a nice tool for viewing the DBC!

  A quick demo!
]

#slide[
  = Putting this into code

  #set text(size: 14pt)

  #figure(
    ```C
    struct lhrdb_id_360_t message;
      message.coolant_pressure = lhrdb_id_360_coolant_pressure_encode(120);
      message.manifold_pressure = lhrdb_id_360_manifold_pressure_encode(120);
      message.rpm = lhrdb_id_360_rpm_encode(3000);
      message.throttle_position = lhrdb_id_360_throttle_position_encode(200);

      uint8_t data[LHRDB_ID_360_LENGTH];
      lhrdb_id_360_pack(data, &message, LHRDB_ID_360_LENGTH);
      uint32_t mailbox;
      CAN_TxHeaderTypeDef header = {
          .DLC = LHRDB_ID_360_LENGTH,
          .StdId = LHRDB_ID_360_FRAME_ID,
          .IDE = CAN_ID_STD,
      };

      HAL_CAN_AddTxMessage(&hcan, &header, data, &mailbox);
    ```,
  )
]

#slide[
  = When to make a new ID

  In general, when making a new CAN ID, keep the following in mind:

  #item-by-item(start: 2)[
    - Does a CAN ID for this exist already? *Check the DBC*
    - What priority should this message be? *Remember Arbitration*
    - Will adding this ID mess up other systems?

  ]

  #uncover(5)[
    If you added a new can ID be sure to *regenerate the libraries*
  ]
]

#slide[
  #toolbox.register-section("DAQ")
  #set align(horizon)

  = DAQ

  How it works, in brief
]

#slide[
  = The Job of DAQ

  #uncover((2, 3))[
    DAQ's only purpose is to log any important data during a drive of the car.
  ]

  #uncover(3)[
    The only means for it to gather data is through CAN.

    Therefore, DAQ is essentially a fancy CAN logger.
  ]
]

#slide[
  = Current DAQ's workflow

  The current iteration of DAQ works by using a double buffering system.

  One buffer gets written to by the TWAI#footnote([
    Two Wire Automotive Interface, because ESP didn't want to pay for
    CAN naming rights.
  ]) peripheral.
  Another buffer is read from and written to the SD Card. The buffers get
  swapped whenever the SD Card has exhausted all of the data in its buffer.

  This is how DAQ deals with lots of CAN Data.
]

#slide[
  = What gets logged by DAQ?

  #uncover(2)[
    #toolbox.big[
      Everything on CAN
    ]
  ]
]

#slide[
  = How do I see what DAQ Logged?

  The directory structure of DAQ looks like this:

  #figure(
    ```txt
    .
    ├── logs
    │   └── notime
    └── meta
        └── index.txt
    ```,
  )

]

#slide[
  = index.txt

  The `meta/index.txt` file tells you which log file is the most recent. It
  looks like this:

  #figure(
    ```txt
    ...
    START 0 /sdcard/logs/notime/session_63cc18ab.txt
    START 0 /sdcard/logs/notime/session_2a70ca41.txt
    START 0 /sdcard/logs/notime/session_e1c3a6b7.txt
    START 0 /sdcard/logs/notime/session_21306483.txt
    START 0 /sdcard/logs/notime/session_68e1905d.txt
    START 0 /sdcard/logs/notime/session_b9839ffe.txt
    ```,
  )

  The last line in the file, is the last log that DAQ made. Don't look at the
  number, I have no clue what it means.
]

#slide[
  = A session file
  Here's what an actual log looks like:

  ```txt
    0000000000.300000,372,009C0000FC0B03F5
    0000000000.300000,3E2,00000000FFFF0000
    0000000000.300000,3EC,0000000000000000
    0000000000.300000,471,085300EA03F50000
  ```

  The first part has the time stamp that the CAN message was collected, then the
  CAN ID, and then the raw bytes from the CAN message. This can be parsed into a
  json file with all of the data.
]

#slide[
  = DAQ Issues

  Sometimes DAQ no work. Here's a quick list of things to check:

  #item-by-item(start: 2)[
    - Is DAQ plugged in? *really double check*
    - Is CAN continuous? *display should show data*
    - Does DAQ have an SD card in? *make sure it's all the way in*
  ]

  #uncover(5)[
    If all those possibilities are covered, then DAQ should work, if you're
    still not getting any logs, hit up electronics.
  ]
]

#slide[
  #set align(horizon)

  = Thank you!
]
