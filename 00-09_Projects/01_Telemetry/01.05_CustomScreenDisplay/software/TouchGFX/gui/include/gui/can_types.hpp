#ifndef CAN_TYPES_H
#define CAN_TYPES_H

#include <stdint.h>
#include <stdbool.h>


#ifdef __cplusplus
#include <touchgfx/Unicode.hpp>
#include <touchgfx/Color.hpp>
#include <touchgfx/widgets/TextAreaWithWildcard.hpp>

#define RED touchgfx::Color::getColorFromRGB(255, 0, 0)
#define YELLOW touchgfx::Color::getColorFromRGB(255, 255, 0)
#define WHITE touchgfx::Color::getColorFromRGB(255, 255, 255)

extern "C" {
#endif
//pure-C typedefs and enums here

typedef enum {
    CAN_TYPE_INT8,
    CAN_TYPE_UINT16,
    CAN_TYPE_FLOAT16
} CAN_ValueType;

typedef enum {
    RPM,
    THROTTLE,
    COOLANT,
    GEAR,
    BATTERY,
    FAULT,
    SPEED
} CAN_ValueIdentifier;

typedef union {
    int8_t   i8;
    uint16_t u16;
    float    f;
} num16_t;

#ifdef __cplusplus
typedef union {
    touchgfx::TextAreaWithOneWildcard* wildcard;
    touchgfx::TextArea* noWildcard;
} textAreaPtr_t;
#endif

typedef struct CAN_value_t {
    uint32_t id;
    num16_t  value;
    uint8_t  startByte;
    CAN_ValueType type;
    CAN_ValueIdentifier valueIdentifier;
    float scale;// value = value * scale + offset
    float offset;
    bool textUpdated;
    bool flashing;
    num16_t yellowThreshold;
    num16_t redThreshold;
#ifdef __cplusplus
    uint16_t bufferSize;
    touchgfx::Unicode::UnicodeChar* bufferPtr;
    bool wildcard;
    textAreaPtr_t textAreaPtr;
#endif
} CAN_value_t;

// Shared CAN IDs and extern structs
#define NUM_OF_CAN_VALUES 7

#define CAN_ID_RPM_THROTTLE      0x360
#define CAN_ID_COOLANT  0x3E0
#define CAN_ID_GEAR     0x470
#define CAN_ID_BATTERY  0x372
#define CAN_ID_FAULT  0x600
#define CAN_ID_SPEED 0x370


// These are declared in can_types.cpp
extern CAN_value_t rpm, coolant, battery, throttle, gear, fault, speed;
extern CAN_value_t* CAN_value_ptrs[NUM_OF_CAN_VALUES];

//helper functions to read CAN data
static inline uint16_t CAN_GetData_16(uint8_t startByte, uint8_t rxData[8]){
	return (rxData[startByte] << 8) | rxData[startByte + 1];
}

static inline uint8_t CAN_GetData_8(uint8_t startByte, uint8_t rxData[8]){
	return rxData[startByte];
}

//these more complicated CAN_type update functions are declared in can_types.cpp
extern void CAN_value_updateTextBuffer(CAN_value_t* CAN_val);
//updates if different, returns whether or not the value IS updated (false if needs to update on screen)
extern bool CAN_value_updateValue(CAN_value_t* CAN_val, uint16_t CAN_data);

#ifdef __cplusplus
}
#endif
#endif // CAN_TYPES_H