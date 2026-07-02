/*
 * itm_printf.c
 *
 *  Created on: Oct 18, 2025
 *      Author: ayaan
 */

#include "stm32u5xx_hal.h"   // or your MCU series header
#include <stdio.h>

// Redirect printf to ITM
int _write(int file, char *ptr, int len)
{
    (void)file;
    for (int i = 0; i < len; i++)
    {
        ITM_SendChar(*ptr++);
    }
    return len;
}
