# LHRDB C Library

The LHRDB C Library is a set of functions and headers that are generated
automatically by the python `cantools` CLI. These functions and headers make it
much easier to write programs that output or consume signals that are produced
via CAN.

There are two versions of this C Library a Float version and a No Float version.
The Float version should work fine in most cases, but note that if the
microcontroller you are using doesn't have a floating point unit then it may
hurt performance. Generally if you don't need floats, just use the no float version.

## Adding to an STM32 Project

!!! NOTE "Note"
    The following example uses the Float version, swap the text from "Float" to
    "NoFloat" if you are using the No Float version.

When starting a new STM32 project using STM32CubeMX, make sure that you select
CMake as the build system. The LHRDB C Library uses CMake as it's build system
and can only be included if you are also using CMake in your project (You should
use CMake anyway).

After generating your project, make the following modifications to the
`CMakeLists.txt` that is placed in the root of the project's new directory:

```cmake hl_lines="4-8"
# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cubemx)

# Add CAN Library
add_subdirectory(
   "${CMAKE_CURRENT_SOURCE_DIR}/../12.01_DBC-C-Library/float/"
   "${CMAKE_CURRENT_SOURCE_DIR}/external/LHRDB_Lib_Float"
)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
)
```

This adds the LHRDB-C-Library as a subdirectory and tells CMake where it should
build it local to your project. You shouldn't have to change the second path,
but you will have to change the first path depending on where your project is
located. When in doubt, use `../../ ...` until you're at the root of the
monorepo and then use this path:

`/10-19_Libraries/12_CAN/12.01_DBC-C-Library/float/`

or this path for the no float version:

`/10-19_Libraries/12_CAN/12.01_DBC-C-Library/no_float/`

Then make this change to the end of the `CMakeLists.txt`:

```cmake hl_lines="5"
# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx
    # Add user defined libraries
    LHRDB_Lib_Float
)
```

This makes sure that your project links with the library. If you don't add this
line, you'll simply build the library but never link it with your final
executable.

If you already ran `cmake` to generate your build script, make sure you delete
the `CMakeCache.txt` file and run `cmake` again in order to generate a new build
script that uses the library.

## Using the Library

Below is a sample of how the library can be used in an STM32 Project:

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
```

For every CAN ID, there is a corresponding struct that can be used to fill in
all the fields of the message. For each field there is a corresponding encode
and decode function. These functions will take an actual value and transfer it
to the encoded version that the CAN Signal expect, and vice versa. In general,
each function has commented documentation if you get confused.

Once you've filled in all of your fields, there is then a pack function which
will place all the fields into an array of bytes. You can then use your
platform's equivilent of CAN_Transmit to send the message.

Take notice that I've used headers instead of numbers for things like length and
frame id. Make sure to do this so that your code is easy to read an interpret.
Additionally, it makes it so that you don't have to reference the CAN DB for
sending simple signals.
