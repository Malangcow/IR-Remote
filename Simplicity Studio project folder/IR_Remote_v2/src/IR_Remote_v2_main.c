// Configure these from project properties
//#define SYSCLK_SPEED  24500000ul
//#define SYS_TICK_FREQ   1000L
//#define SYS_TICK_TIMER_FREQ SYSCLK_SPEED

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------

#include <SI_EFM8BB2_Register_Enums.h>                  // SFR declarations
#include <LIMITS.H>
#include "InitDevice.h"

#include "atomic.h"
#include "systick.h"
#include "IR_NEC_tx.h"
#include "sleep.h"
//#include <my-stdint.h>
// $[Generated Includes]
// [Generated Includes]$

//-----------------------------------------------------------------------------
// Defines
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Functions Forward Declarations
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// main() Routine
// ----------------------------------------------------------------------------
int
main(void)
{
  uint8_t SFRPAGE_save;

  // Call hardware initialization routine
  enter__38kHz_Modulation_from_RESET();

  SFRPAGE_save = SFRPAGE;
  SFRPAGE = SFRPAGE_save;

  while (1)
    {
      uint32_t timeStamp;

      timeStamp = get_sys_tick();
      P1_B6 ^= 1; // For debug purpose

      if (P3_B1 == 0)
        {
          IR_NEC_tx_frame(0x01, 0x0E); // Remote code

          while ((int32_t) (get_sys_tick() - timeStamp < (uint32_t) 200 << 16))
            ; // delay 200 ms
        }

    }
}

