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

//volatile pulse_t* p_pulse;

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

void
nPulse_irq_callback()
{
  if (pulse.isActive)
    {
      // 0: Prepare
      // 1: high
      // 2: low
      // 3: low
      // 4:
      ////////////////////////////////////
      int _intCnt_temp = pulse._intCnt;
      switch (_intCnt_temp)
        {
      case 0:
        ++_intCnt_temp;
        break;
      case 1:
        P2_B0 = 1;
        if (pulse.count > 0)
          { // pulse start, decrease counter
            --pulse.count;
          }
        ++_intCnt_temp;
        break;
      case 2:
        P2_B0 = 0;
        ++_intCnt_temp;
        break;
      case 3:
        if (pulse.count > 0)
          {
            _intCnt_temp = 1;
          }
        else
          {
            _intCnt_temp = 0;
            pulse.isActive = 0;
          }
        break;
      default:
        break;
        }
      pulse._intCnt = _intCnt_temp;

    }
}

