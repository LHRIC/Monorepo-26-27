# Development Environment

For the most part everything we use for development in LHRIC is based on a Unix
environment. While some programs will work on Windows, you might run into a lot
of issues. Thus, it is recommended that if you are going to stick with Windows,
you should use WSL in order to use a linux environment.

!!! tip "Switch to Linux"
    If you want the ultimate computing experience you should switch to Linux (or
    at least dual boot Linux and windows). Talk to Jack if you want to switch
    over to the best operating system.

## Tools

Here is a list of a few of the tools we use:

- arm-none-eabi-gcc: This is the GNU cross compiler used for compiling C code to
  be run on ARM devices. You need this to compile basically anything in this
  repository.
- ST Cube MX: This is a GUI tool that lets you quickly setup starter code for
  STM devices. This makes things super easy and it is recommended that you use
  this to set up your projects.
- openocd: This is a tool that can be used to flash and debug on
  microcontrollers. There are other tools that are STM specific, but openocd is
  pretty easy to set up and works with more than just ST microcontrollers.
- CMake: Most projects use the CMake build system.
- make: If a project doesn't use CMake it probably uses a makefile for its
  build system. This should come standard with any Linux distro.
- just: This is a command runner that is akin to make, but with some nice extra
  features. I've used it a bit in this repository and it is good way to make
  commonly run commands more easily accessible. There's a few examples in the
  tools area.
- ESP-IDF: This is the development framework for ESP microcontrollers. It is
  also used to flash them.

You're probably fine waiting until you try doing something and it says you're
missing one of these tools, though it isn't a bad idea to install them all now.
