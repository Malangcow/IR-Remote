//=========================================================
// src/Interrupts.c: generated by Hardware Configurator
//
// This file will be regenerated when saving a document.
// leave the sections inside the "$[...]" comment tags alone
// or they will be overwritten!
//=========================================================

// USER INCLUDES
#include <stdint.h>
#include <SI_EFM8BB2_Register_Enums.h>

#include "nPulse.h"
#include "systick.h"
#include "sleep.h"
#include "IR_NEC_tx.h"

//-----------------------------------------------------------------------------
// TIMER2_ISR
//-----------------------------------------------------------------------------
//
// TIMER2 ISR Content goes here. Remember to clear flag bits:
// TMR2CN0::TF2H (Timer # High Byte Overflow Flag)
// TMR2CN0::TF2L (Timer # Low Byte Overflow Flag)
//
//-----------------------------------------------------------------------------
SI_INTERRUPT (TIMER2_ISR, TIMER2_IRQn)
{
  // Clear T2 high Interrupt Flag
  TMR2CN0_TF2H = 0;
  P1_B7 ^= 1;
  ++_sys_tick;

}

//-----------------------------------------------------------------------------
// TIMER3_ISR
//-----------------------------------------------------------------------------
//
// TIMER3 ISR Content goes here. Remember to clear flag bits:
// TMR3CN0::TF3H (Timer # High Byte Overflow Flag)
// TMR3CN0::TF3L (Timer # Low Byte Overflow Flag)
//
//-----------------------------------------------------------------------------
SI_INTERRUPT (TIMER3_ISR, TIMER3_IRQn)
{
  // Clear Interrupt Flag
  TMR3CN0 &= ~TMR3CN0_TF3H__BMASK;
  //P1_B7 ^= 1;

  // IT SEEMS TO BE USING POINTER OR CALLING FUNCTION WHITHIN IRQ SLOWS DOWN
  // ISR ALOT!!! LET'S NOT USE THEM!

//  if (p_pulse->isActive)
//    {
//      // 0: Prepare
//      // 1: high
//      // 2: low
//      // 3: low
//      // 4:
//      ////////////////////////////////////
//      int _intCnt_temp = p_pulse->_intCnt;
//      switch (_intCnt_temp)
//        {
//      case 0:
//        ++_intCnt_temp;
//        break;
//      case 1:
//        P2_B0 = 1;
//        if (p_pulse->count > 0)
//          { // pulse start, decrease counter
//            --p_pulse->count;
//          }
//        ++_intCnt_temp;
//        break;
//      case 2:
//        P2_B0 = 0;
//        ++_intCnt_temp;
//        break;
//      case 3:
//        if (p_pulse->count > 0)
//          {
//            _intCnt_temp = 1;
//          }
//        else
//          {
//            _intCnt_temp = 0;
//            p_pulse->isActive = 0;
//          }
//        break;
//      default:
//        break;
//        }
//      p_pulse->_intCnt = _intCnt_temp;
//
//    }

  //nPulse_irq_callback();

  if (pulse.isActive)
      {
        // 0: high
        // 1: low
        // 2: low
        // 3:

        ////////////////////////////////////
        int _intCnt_temp = pulse._intCnt;
        switch (_intCnt_temp)
          {
        case 0:
          P2_B0 = 1;
          if (pulse.count > 0)
            { // pulse start, decrease counter
              --pulse.count;
            }
          ++_intCnt_temp;
          break;
        case 1:
          P2_B0 = 0;
          ++_intCnt_temp;
          break;
        case 2:
          if (pulse.count > 0)
            {
              _intCnt_temp = 0;
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

//  if (pulse.isActive)
//    {
//      // 0: Prepare
//      // 1: high
//      // 2: low
//      // 3: low
//      // 4:
//      ////////////////////////////////////
//      int _intCnt_temp = pulse._intCnt;
//      switch (_intCnt_temp)
//        {
//      case 0:
//        ++_intCnt_temp;
//        break;
//      case 1:
//        P2_B0 = 1;
//        if (pulse.count > 0)
//          { // pulse start, decrease counter
//            --pulse.count;
//          }
//        ++_intCnt_temp;
//        break;
//      case 2:
//        P2_B0 = 0;
//        ++_intCnt_temp;
//        break;
//      case 3:
//        if (pulse.count > 0)
//          {
//            _intCnt_temp = 1;
//          }
//        else
//          {
//            _intCnt_temp = 0;
//            pulse.isActive = 0;
//          }
//        break;
//      default:
//        break;
//        }
//      pulse._intCnt = _intCnt_temp;
//
//    }

  _pause_done = 1; // exit pause mode
}

//-----------------------------------------------------------------------------
// TIMER4_ISR
//-----------------------------------------------------------------------------
//
// TIMER4 ISR Content goes here. Remember to clear flag bits:
// TMR4CN0::TF4H (Timer # High Byte Overflow Flag)
// TMR4CN0::TF4L (Timer # Low Byte Overflow Flag)
//
//-----------------------------------------------------------------------------
SI_INTERRUPT (TIMER4_ISR, TIMER4_IRQn)
{
  TMR4CN0_TF4H = 0;
} // of TIMER4_ISR
