/*
 * systick.c
 *
 *  Created on: Apr 12, 2016
 *      Author: Seungwon
 */

#include <stdint.h>
#include "systick.h"
#include "atomic.h"

volatile uint32_t _sys_tick;

uint32_t
get_sys_tick()
{
  uint32_t st;
  uint16_t T_reload;
  //uint16_t T_cnt;
  uint8_t SFRPAGE_save;
  ATOMIC_BLOCK_START
      {
        st = _sys_tick;
      }
    ATOMIC_BLOCK_END

    SFRPAGE_save = SFRPAGE;
    SFRPAGE = 0x00;

    T_reload = TMR2RLL;
    T_reload += (TMR2RLH << 8);


    SFRPAGE = SFRPAGE_save;

  st <<= 16;
  return st;
}
