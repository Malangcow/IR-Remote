/*
 * nPulse.c
 *
 *  Created on: Apr 15, 2016
 *      Author: Seungwon
 */

#include <stdint.h>
#include <stdbool.h>
#include "atomic.h"
#include "nPulse.h"

volatile pulse_t pulse =
  { 0, 0 };


uint16_t
nPulse(uint16_t pulseCount, volatile pulse_t* pulse)
{
  if (pulseCount == 0)
    return 1;

  ATOMIC_BLOCK_START
      {
        if (pulse->isActive == 0)
          {
            pulse->count = pulseCount;
            pulse->isActive = 1;
            pulse->_intCnt = 0;
          }
      }
    ATOMIC_BLOCK_END

  return pulseCount;

  //return 0;
}
