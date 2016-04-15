/*
 * sleep.c
 *
 *  Created on: Apr 15, 2016
 *      Author: Seungwon
 */

#include <SI_EFM8BB2_Register_Enums.h>
#include <stdbool.h>
#include "sleep.h"
#include "IR_NEC_tx.h"

// global variable

void
enter_power_mode(POWER_MODE_t pMode)
{
  // PCON0, all
  // PCON1, 0x00
  SFRPAGE = 0x00;
  switch (pMode)
    {
  case POWER_MODE_NORMAL:
    PCON0 &= PCON0_STOP__BMASK | PCON0_IDLE__BMASK;
    PCON1 = 0;
    break;
  case POWER_MODE_IDLE:
    PCON0 |= PCON0_IDLE__IDLE;
    PCON0 = PCON0;         // ... followed by a 3-cycle dummy instruction
    break;
  case POWER_MODE_SUSPEND:
    PCON1 |= PCON1_SUSPEND__SUSPEND;
    PCON1 = PCON1;         // ... followed by a 3-cycle dummy instruction
    break;
  case POWER_MODE_SNOOZE:
    PCON1 |= PCON1_SNOOZE__SNOOZE;
    PCON1 = PCON1;         // ... followed by a 3-cycle dummy instruction
    break;
  case POWER_MODE_SHUTDOWN:
    SFRPAGE = 0x00;
    REG0CN = REG0CN_STOPCF__SHUTDOWN;
    PCON0 |= PCON0_STOP__STOP;
    break;
  default:
    return;
    }
  _pause_done = 1;
}

