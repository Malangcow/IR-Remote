/*
 * sleep.h
 *
 *  Created on: Apr 15, 2016
 *      Author: Seungwon
 */

#ifndef SLEEP_H_
#define SLEEP_H_
#include <stdbool.h>



typedef enum
{
  POWER_MODE_NORMAL,
  POWER_MODE_IDLE,
  POWER_MODE_SUSPEND,
  POWER_MODE_SNOOZE,
  POWER_MODE_SHUTDOWN
} POWER_MODE_t;

void
enter_power_mode(POWER_MODE_t pMode);

/**
 * @note I don't think I need this one.\n
 * Code will resume automatically after wake up event.
 */
#define exit_low_power_mode() enter_power_mode(POWER_MODE_NORMAL)

#endif /* SLEEP_H_ */
