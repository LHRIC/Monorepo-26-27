# Adding New Display Item

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