#ifndef RC_H
#define RC_H

#include <stdbool.h>
#include <stdint.h>

/*
 *  Initializes Required Peripherals for measuring an RC Time Constant.
 */
void rc_measurement_init(void);

/*
 *  Begins an RC measurement
 */
void rc_measurement_begin(void);

/*
 *  Checks if the RC measurement has completed and is valid.
 */
bool rc_measurement_check_valid(void);

/*
 *  Get the most recent RC measurement
 */
uint32_t rc_measurement_get(void);

/*
 *  Ensure Cap is discharged
 */
void rc_discharge_cap(void);

void comparator_init(void);
void timer0_init(void);

#endif
