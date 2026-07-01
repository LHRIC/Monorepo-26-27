# Custom Display

##Purpose

Custom Display is a driver information display for the car, built on an STM32H7 microcontroller with a Riverdi LCD. The STM32 reads live data from the Haltech ECU over the car's CAN bus and renders it on-screen using [TouchGFX](https://support.touchgfx.com/docs/introduction/what-is-touchgfx), an embedded graphics library for STM32.

**Display hardware:** [https://riverdi.com/product/5-inch-tft-lcd-screen-stm32u5-embedded-display-rvt50hqsfwn01?srsltid=AfmBOoq_PbYSuQUkMhpmtfA5kOqO7L9fX0RrjbtiRxhyNDaXJeP7M8Rh](#)  
> ![picture](assets/driverdisplay.jpg)


---

## Overview

| Detail | Value |
|---|---|
| Microcontroller | STM32H7 |
| Display | Riverdi LCD |
| Graphics Library | TouchGFX |
| Language | C / C++ |
| RTOS | None (simplified loop via `Model::tick`) |
| Data Source | ECU via CAN bus |

---

## Architecture

All code is written in C style but integrates with TouchGFX, a C++ library. There is no RTOS — the update loop runs entirely inside `Model::tick` for simplicity.

### Data Flow

```
CAN Bus (mostly ECU though it can be anything on CAN bus)
    │
    ▼
CAN Interrupt (main.c)
    │  updates
    ▼
can_types.cpp  ──  UI data elements & handlers
-----
Model.cpp      ──  data update tick
    │
    ▼
Screen1Presenter.cpp
    │
    ▼
Screen1View.cpp  ──  UI element rendering
```

### Key Files

| File | Role |
|---|---|
| `main.c` | CAN interrupt setup; `RxFifo0Callback` routes incoming CAN frames |
| `can_types.hpp` | Declares CAN value types, IDs, default values, etc|
| `can_types.cpp` | Defines CAN values and the `CAN_value_ptrs` array |
| `Model.cpp` | Polls updated CAN data each tick and forwards to presenter |
| `Screen1Presenter.cpp` | Mediates between model and view |
| `Screen1View.cpp` | Updates UI elements; handles special visual logic |

---

## Contributing

The TouchGFX tutorials and docs are pretty good for understanding how it works. Also, I used CubeMX to generate code, TouchGFX Designer to generate UI code, CMake to build, and VSCode Arm Cortex Debug for flash/debug. The following section describes how to add an item to the display.

---

## Adding a New Display Item

Follow these steps to add a new value from the CAN bus to the display.

### Step 0 — Design in TouchGFX Designer *(Windows only)*

Use TouchGFX Designer to design and generate UI files (the project is in TouchGFX_Designer folder), then copy the generated `TouchGFX/generated` folder into the project. If someone could find a way to integrate the TouchGFX Designer project with the actual code that would be pretty cool, but it was weird and had buggy build issues in my experience.

### Step 1 — `can_types.hpp`

- Increment `NUM_OF_CAN_VALUES`
- `#define` the new CAN ID (check [Haltech ECU protocol](https://support.haltech.com/portal/api/kbArticles/309315000127455298/locale/en/attachments/8whone12beb10b56646c7893d210fa06b2b86/content?portalId=edbsndab83bda6a605a18494b81368a73ed74e00f5941b9c7dc264955a9257f1b8067&inline=true))
- Declare the new `can_type` in the `extern` list
- Add the new entry to the `CAN_ValueType` enum

### Step 2 — `can_types.cpp`

- Define the new `can_type` with non-C++ initial values
- Add a pointer to it in the `CAN_value_ptrs` array

### Step 3 — `main.c`

- Add a case for the new CAN ID in the `switch` statement inside `RxFifo0Callback`

### Step 4 — `Screen1View.cpp`

- Set C++ starting values (UI-related) in `setupScreen`
- Add any special visual logic to `updateDisplayValue` if needed
