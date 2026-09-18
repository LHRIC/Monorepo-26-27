---
title: 'How to blink an LED'
theme:
    name: gruvbox-dark
sub_title: 'or Intro to Embedded Systems'
author: 'Jack Saenz & Ayaan Dhuka'
---

The Answer
---


```asm
    LDR R0, =GPIOA_CRL
    LDR R0, [R0]
    LDR R1, [R0]
    MOVS R2, 0xf
    BICS R1, R2
    MOVS R2, 0b001
    ORRS R1, R2
    STR R1, [R0]

    LDR R0, =GPIOA_ODR
    LDR R0, [R0]
    MOVS R2, #1
loop:
    LDR R1, [R0]
    EORS R1, R2
    STR R1, [R0]
    BL delay_100ms
    B loop

```

<!-- pause -->
Questions?
<!-- end_slide -->

The more nuanced answer
---

<!-- incremental_lists: true -->

To blink an LED with a computer you need to understand the following things

- How a computer works
- How a computer changes its state
- How a computer interacts with the world

<!-- end_slide -->

Why you should care
--

<!-- incremental_lists: true -->

If you can just use an Arduino or a HAL (Hardware Abstraction Library) why
should I care about assembly?

- Interfacing with sensors is a lot like interfacing with a computer
- Debugging is easier if you know what's going on with the hardware
- If you understand these principles you can use any microcontroller

<!-- end_slide -->

What is a computer?
---


At it's most basic a computer is a machine that reads instructions from a memory
and performs them, updating the computer's state as well as the tape.

![](./assets/turing.jpg)

<!-- alignment: center -->
_Turing's Model of a Computer_

<!-- end_slide -->

What is an instruction?
---

An instruction is a contract given to the programmer, and it is how we update
the state of the computer.

<!-- pause -->

What's included in the state of the computer?

<!-- incremental_lists: true -->

- Registers (Temporary Variables)
- Current Instruction
- Status Bits
- Hidden State

<!-- end_slide -->

An Example of an Instruction
---

The following instruction adds the contents of two registers, storing the result
into the destination register

```asm
ADD DR, R1, R2
```

This is equivalent to:

```typst +render
$
"DR" = "R1" + "R2"
$
```

<!-- end_slide -->

How are instructions performed?
---

Performing instructions is the key purpose of a computer. How do we actually
move data around and perform operations?

<!-- end_slide -->

The Datapath
---

The Datapath is how all information is moved around in the computer. Here's
an example of a datapath that could perform the previous instruction:

<!-- pause -->

![](./assets/Datapath.png)


<!-- end_slide -->

An example of an instruction being performed
---

<!-- column_layout: [1, 2] -->

<!-- column: 0 -->
```asm
ADD R1, R1, R2

```

<!-- column: 1 -->
![](./assets/Datapath.png)

<!-- end_slide -->

A more complex program
---

<!-- column_layout: [1, 2] -->

<!-- column: 0-->
```
LD R1, data
ADD R1, R1, #2
ST R1, data

data:
.fill #1
```

<!-- column: 1 -->
![](./assets/Datapath.png)

<!-- end_slide -->

Why do we care about changing memory?
---

That assembly program was extremely powerful, however the reason why isn't
obvious.

Any ideas what might have happened?

<!-- pause -->

<!-- new_lines: 3 -->

We just updated the value being displayed by our computer!

![](./assets/exterior_changes.png)

<!-- end_slide -->

Memory Mapped IO
---

Certain locations in memory mean something! This is the core of how computers
interact with the world.

<!-- pause -->
# Why not use an instruction?

We only have room for a certain amount of instructions. If we can reuse the load
and store instructions for input and output, we should do that. Additionally,
the address space of a load and store is huge, so we can add a lot of IO.

There are some architectures that use an instruction for IO, but most modern
microcontrollers and CPU's use Memory Mapped IO

<!-- end_slide -->

An Example of Memory Mapped IO
---

<!-- end_slide -->

Back to the Blink
---

Wasn't this presentation about how you blink an LED?

Any guesses now how we're going to do it?

<!-- end_slide -->

The STM32f103 Datapath
---

<!-- end_slide -->

The STM32f103 Memory Map
---

<!-- end_slide -->

The GPIO Registers
---

<!-- end_slide -->

The CRL Register
---

<!-- end_slide -->

The ODR Register
---

<!-- end_slide -->

The Program Once Again
---

<!-- end_slide -->

Why you shouldn't blink in assembly
---

<!-- end_slide -->

How you blink in C
---

<!-- end_slide -->

Why you wouldn't use bare registers
---

<!-- end_slide -->

How you blink with a HAL
---

<!-- end_slide -->

Once again, why you should care
---

<!-- end_slide -->

Thoughts and Feedback
---

