# PyOCD Flashing and Debugging

PyOCD is a wrapper around OpenOCD that provides an easy and convenient way of
generally being able to program and debug most chips. It has support for a
variety of debug and flashing interfaces, making it very versatile. While you
can manually write commands to OpenOCD, it is somewhat difficult to setup, and
PyOCD tends to make things a lot easier.

## Installing

Since PyOCD is made in python, it can be installed very easily on a variety of
operating systems by using `pip`. You can install it like so:

```console title="Linux"
$ python3 -m pip install -U pyocd
```

```console title="Windows"
C:\> pip install -U pyocd
```

!!! note "Linux Install in a venv"
    Certain linux distributions don't like it whenever you install something
    globally using pip. The solution for this is to install it in a python
    virtual environment. The tool `pipx` provides any easy way of doing this,
    while making it easy to use the installed tools anywhere. You can install
    PyOCD with `pipx` using the following command:
    ```constole
    $ pipx install pyocd
    ```

To check if you have PyOCD after installation, you can simply run `pyocd` in the
terminal and it should give you a list of commands.

## Flashing with PyOCD

In order to flash something with PyOCD you need two main things:

1. A Debugger that can program the thing you are trying to flash.
2. The PyOCD package that is required for the specific part you are trying to flash.

Most of the MCUs we use in LHR will be STM, so you use an STLink debugger. If
you have one connected to your computer, you can test if it is working by
running the following command:

```console
$ pyocd list
```

This will list out all of the available debuggers connected to your computer. If
I run this command with an STLink-V3 connected I see this:

```console
$ pyocd list
  #   Probe/Board   Unique ID                  Target
-------------------------------------------------------
  0   STLINK-V3     002000203133510837363734   n/a
```

Right now I don't have a board connected, but, for some targets, it will
automatically recognize the part you are trying to program.

The next thing you need is the specific package for the part that you are trying
to program. Say I am trying to program an `stm32l01`, I can run the `pack find`
command to see what package I need to install:

```console
$ pyocd pack find stm32l0
  Part            Vendor               Pack                 Version   Installed
---------------------------------------------------------------------------------
  STM32L010C6Tx   STMicroelectronics   Keil.STM32L0xx_DFP   3.1.0     False
  STM32L010F4Px   STMicroelectronics   Keil.STM32L0xx_DFP   3.1.0     False
  STM32L010K4Tx   STMicroelectronics   Keil.STM32L0xx_DFP   3.1.0     False
  STM32L010K8Tx   STMicroelectronics   Keil.STM32L0xx_DFP   3.1.0     False
  STM32L010R8Tx   STMicroelectronics   Keil.STM32L0xx_DFP   3.1.0     False
  STM32L010RBTx   STMicroelectronics   Keil.STM32L0xx_DFP   3.1.0     False
  STM32L011D3Px   STMicroelectronics   Keil.STM32L0xx_DFP   3.1.0     False
  STM32L011D4Px   STMicroelectronics   Keil.STM32L0xx_DFP   3.1.0     False
  STM32L011E3Yx   STMicroelectronics   Keil.STM32L0xx_DFP   3.1.0     False
  STM32L011E4Yx   STMicroelectronics   Keil.STM32L0xx_DFP   3.1.0     False
  STM32L011F3Px   STMicroelectronics   Keil.STM32L0xx_DFP   3.1.0     False
  STM32L011F3Ux   STMicroelectronics   Keil.STM32L0xx_DFP   3.1.0     False
  STM32L011F4Px   STMicroelectronics   Keil.STM32L0xx_DFP   3.1.0     False
  STM32L011F4Ux   STMicroelectronics   Keil.STM32L0xx_DFP   3.1.0     False
  STM32L011G3Ux   STMicroelectronics   Keil.STM32L0xx_DFP   3.1.0     False
  STM32L011G4Ux   STMicroelectronics   Keil.STM32L0xx_DFP   3.1.0     False
  STM32L011K3Tx   STMicroelectronics   Keil.STM32L0xx_DFP   3.1.0     False
  STM32L011K3Ux   STMicroelectronics   Keil.STM32L0xx_DFP   3.1.0     False
  STM32L011K4Tx   STMicroelectronics   Keil.STM32L0xx_DFP   3.1.0     False
  STM32L011K4Ux   STMicroelectronics   Keil.STM32L0xx_DFP   3.1.0     False
```

As you can see, a lot of options show up, however they all they use the same
pack, so I just need to run

```console
$ pyocd pack install stm32l010c6tx
```

And it will install the Keil.STM32L0xx_DFP package, which will make it so I can
program any of the parts that were listed. The pack install command takes a
specific part number, and not a package name so you can just pick any part
number and it will install the package for that part. Now you're ready to flash
your MCU!

To flash all you need to do is run the following command:

```console
$ pyocd flash -t <target-name> firmware.elf
```

Just fill in the `<target-name>` with whatever chip you are flashing, this is the
same as the target you passed in to install the pack for the chip you wanted,
just make sure you use the exact part for whatever chip you are flashing, as it
does matter in this case. You also need to make sure that `firmware.elf` is the
path to the actual `.elf` output for the code you compiled. This should be
found somewhere in a `build` or `out` directory. After that, your program should
begin to flash and you can begin testing!

!!! tip "Auto Target"
    If when you ran `pyocd list` it showed the target your debugger is connected
    to, then you don't need to pass in the `-t <target-name>` argument, since
    PyOCD already knows what you're trying to flash.

## Debugging

If you would like to open a debugging server with PyOCD, the experience is much
the same as it goes with flashing. Instead of running the `flash` command you
run the `gdbserver` command like so:

```console
$ pyocd gdbserver --target=<target-name>
```

Again, where `<target-name>` is replaced with whatever target you are trying to
debug. You can then connect to the server using gdb, however ensure that your
gdb installation has support for multiple architectures. On windows you'll
likely need gdb-multiarch however on linux, your installation of gdb might be
built with multiarch support already. To check you can run the following command
while in gdb:

```console
$ gdb
(gdb) set architecture
```

This will print out all of the architectures supported by your installation of
gdb. To debug STM32 you will likely need `armv6-m`, though check the specific
architecture of the chip you're debugging.

After validating your gdb installation, you can start debugging by running the
following commands:

```console
$ gdb ./firmware.elf
(gdb) tar ext :3333
```

Which should connect you to your MCU and allow you to start debugging. For more
information on the process, reference [this article](https://pyocd.io/docs/gdbserver.html) from the PyOCD docs.

