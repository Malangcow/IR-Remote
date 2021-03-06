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
#include "button.h"

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
  // P1_B7 ^= 1;
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

  // Check for burst mode
  if (pulse.isActive)
    {
      // 0: high
      // 1: low
      // 2: low
      ////////////////////////////////////
      int _intCnt_temp = pulse._intCnt;
      switch (_intCnt_temp)
        {
      case 0:
        IR_TX_PIN = 1;
        if (pulse.count > 0)
          { // pulse start, decrease counter
            --pulse.count;
          }
        ++_intCnt_temp;
        break;
      case 1:
        IR_TX_PIN = 0;
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

  if (!pulse.isActive)
    {
      // NOTE: Will enter here if pulse.isActive = 0
      // But, what if isActive just became 0 from executing code right above?
      // Whouldn't that make this take additional 1/38k s and 1 IRQ to reach here?
      // So I added additional if conditional here
      switch (ir_nec_tx_state_new)
        {
      case IR_NEC_TX_STATE_IDLE:
        // nothing to do
        break;
      case IR_NEC_TX_STATE_NEW_LEADER_CODE_BURST:
        // Was leader burst. So, now for the leader code pause
        // set 4.5 ms timer interrupt
        // T CLK: SYSCLK/12
        // Initial value: 56349

        SFRPAGE = 0x10;
        TMR3CN0 = 0;   // Stop and resets timer. This also make T3XCLK SYSCLK/12
        // CLK source EXTCLK, SYSCLK/12
        CKCON0 &= ~(CKCON0_T3MH__BMASK | CKCON0_T3ML__BMASK);

        // Set initial value
        TMR3H = IR_NEC_PAUSE_LEADER_CODE_MAGIC_VALUE >> 8;
        TMR3L = IR_NEC_PAUSE_LEADER_CODE_MAGIC_VALUE;

        // Since we will use this timer as one shot timer,
        // reload value doesn't matters
        //TMR3RLH = 0;
        //TMR3RLL = 0;

        TMR3CN0 |= TMR3CN0_TR3__RUN;            // Run Timer

        // Renew state
        ir_nec_tx_state_new = IR_NEC_TX_STATE_NEW_LEADER_CODE_PAUSE;

        // After leader code pause data burst is expected
        // Preload data to the buffer
        IR_NEC_tx_buffer_preload(&ir_nec_tx_buffer_v3);

        break;

      case IR_NEC_TX_STATE_NEW_LEADER_CODE_PAUSE:
        {
          // Was leader code pause. Now for the data burst
          //uint16_t reloadVal;
          //uint8_t TMR3CN0_TR3_save;
          uint8_t regTemp;
          SFRPAGE = 0x10;
          // Save Timer Run Configuration
          //TMR3CN0_TR3_save = TMR3CN0 & TMR3CN0_TR3__BMASK;
          // Stop and resets Timer
          TMR3CN0 = 0;

          // Timer uses SYSCLK for the clock
          regTemp = CKCON0 & ~(CKCON0_T3MH__BMASK | CKCON0_T3MH__BMASK);
          CKCON0 = regTemp | CKCON0_T3MH__SYSCLK | CKCON0_T3ML__SYSCLK;

          // Calculate reload value, 65321(0xFF29) expected at 24.5 MHz
          //      reloadVal = 65536u
          //          - (SYS_TICK_TIMER_FREQ + IR_NEC_TIMER_IRQ_FREQ / 2)
          //              / IR_NEC_TIMER_IRQ_FREQ;

          //reloadVal = 65536u - SYS_TICK_TIMER_FREQ / IR_NEC_TIMER_IRQ_FREQ;

          TMR3RLH = IR_NEC_PULSE_BURST_MAGIC_VALUE >> 8;
          TMR3RLL = IR_NEC_PULSE_BURST_MAGIC_VALUE;
          TMR3H = 0xFF;
          TMR3L = 0xFF;

          // Interrupts
          //    SFRPAGE = 0x10;
          //    EIE1 |= EIE1_ET3__ENABLED; // Enable T3 IRQ
          //    EIP1H |= EIP1H_PHT3__HIGH; // T3 priority: 3(highest)
          //    EIP1 |= EIP1_PT3__HIGH;

          TMR3CN0 |= TMR3CN0_TR3__RUN;                // Run Timer

          // 22 pulses
          if (pulse.isActive == 0)
            {
              pulse.count = 22;                   // TODO: Magic number!!!
              pulse.isActive = 1;
              pulse._intCnt = 0;
            }

          if (ir_nec_tx_buffer_v3.loadedData == 1)
            ir_nec_tx_state_new = IR_NEC_TX_STATE_NEW_DATA_BURST_1;
          else
            ir_nec_tx_state_new = IR_NEC_TX_STATE_NEW_DATA_BURST_0;

          break;
        }
      case IR_NEC_TX_STATE_NEW_DATA_BURST_1:
        // Was data burst "1". Now for data pause "1"
        // T CLK: SYSCLK
        // Initial value: 24595
        SFRPAGE = 0x10;
        TMR3CN0 = 0;            // Stop and resets timer.

        // CLK source, SYSCLK
        CKCON0 |= CKCON0_T3MH__EXTERNAL_CLOCK | CKCON0_T3ML__EXTERNAL_CLOCK;
        // Set initial value
        TMR3H = IR_NEC_PAUSE_1_MAGIC_VALUE >> 8;
        TMR3L = IR_NEC_PAUSE_1_MAGIC_VALUE;

        // No need to configure reload value.
        //    TMR3RLH = 0;
        //    TMR3RLL = 0;
        // Run Timer
        TMR3CN0 |= TMR3CN0_TR3__RUN;

        // After data pause, data burst or termination burst is expected
        // Preload data to the buffer
          {
            uint8_t p_data = IR_NEC_tx_buffer_preload(&ir_nec_tx_buffer_v3);
            //        if (p_data == -1)
            //          {
            //            ir_nec_tx_state_new =
            //                IR_NEC_TX_STATE_NEW_DATA_PAUSE_BEFORE_TERMINATION_BURST;
            //            break;
            //          }
          }
        ir_nec_tx_state_new = IR_NEC_TX_STATE_NEW_DATA_PAUSE_1;
        break;

      case IR_NEC_TX_STATE_NEW_DATA_BURST_0:
        // Was data burst "0". Now for data pause "0"

        // T CLK: SYSCLK
        // Initial value: 52158
        SFRPAGE = 0x10;
        TMR3CN0 = 0;            // Stop and resets timer.

        // CLK source, SYSCLK
        CKCON0 |= CKCON0_T3MH__EXTERNAL_CLOCK | CKCON0_T3ML__EXTERNAL_CLOCK;
        // Set initial value
        TMR3H = IR_NEC_PAUSE_0_MAGIC_VALUE >> 8;
        TMR3L = IR_NEC_PAUSE_0_MAGIC_VALUE;

        // No need to configure reload value.
        //        TMR3RLH = 0;
        //        TMR3RLL = 0;

        // Run Timer
        TMR3CN0 |= TMR3CN0_TR3__RUN;

        // After data pause, data burst or termination burst is expected
        // Preload data to the buffer
          {
            uint8_t p_data = IR_NEC_tx_buffer_preload(&ir_nec_tx_buffer_v3);
            //        if (p_data == -1)
            //          {
            //            ir_nec_tx_state_new =
            //                IR_NEC_TX_STATE_NEW_DATA_PAUSE_BEFORE_TERMINATION_BURST;
            //            break;
            //          }
          }

        ir_nec_tx_state_new = IR_NEC_TX_STATE_NEW_DATA_PAUSE_0;

        break;

      case IR_NEC_TX_STATE_NEW_DATA_PAUSE_1:
      case IR_NEC_TX_STATE_NEW_DATA_PAUSE_0:
        // Was data pause. Now for the Data Burst or Termination Burst
        {

          uint8_t regTemp;
          SFRPAGE = 0x10;
          // Stop and resets Timer
          TMR3CN0 = 0;

          // Timer uses SYSCLK for the clock
          regTemp = CKCON0 & ~(CKCON0_T3MH__BMASK | CKCON0_T3MH__BMASK);
          CKCON0 = regTemp | CKCON0_T3MH__SYSCLK | CKCON0_T3ML__SYSCLK;

          TMR3RLH = IR_NEC_PULSE_BURST_MAGIC_VALUE >> 8;
          TMR3RLL = IR_NEC_PULSE_BURST_MAGIC_VALUE;
          TMR3H = 0xFF;
          TMR3L = 0xFF;

          TMR3CN0 |= TMR3CN0_TR3__RUN;                // Run Timer

          // 22 pulses
          if (pulse.isActive == 0)
            {
              pulse.count = 22;                   // TODO: Magic number!!!
              pulse.isActive = 1;
              pulse._intCnt = 0;
            }

          switch (ir_nec_tx_buffer_v3.loadedData)
            {
          case 0:
            ir_nec_tx_state_new = IR_NEC_TX_STATE_NEW_DATA_BURST_0;
            break;
          case 1:
            ir_nec_tx_state_new = IR_NEC_TX_STATE_NEW_DATA_BURST_1;
            break;
          case -1:
            ir_nec_tx_state_new = IR_NEC_TX_STATE_NEW_TERMINATION_BURST;
            break;
            }
          break;
        }

        //  case IR_NEC_TX_STATE_NEW_DATA_PAUSE_BEFORE_TERMINATION_BURST:
        //    break;

      case IR_NEC_TX_STATE_NEW_TERMINATION_BURST:
        // Termination burst sent. Now idle mode
        SFRPAGE = 0x10;
        TMR3CN0 = 0;            // Stop and resets Timer

        ir_nec_tx_state_new = IR_NEC_TX_STATE_NEW_IDLE;

        break;

      default:
        break;
        } // end of switch case

    }

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
//-----------------------------------------------------------------------------
// PMATCH_ISR
//-----------------------------------------------------------------------------
//
// PMATCH ISR Content goes here. Remember to clear flag bits:

//
//-----------------------------------------------------------------------------
SI_INTERRUPT (PMATCH_ISR, PMATCH_IRQn)
{
  // Mismatch event occurred

  // Check port
  if (STOP_BUTTON == 0)
    button = BUTTON_PRESSED_STOP_BUTTON;
  else if(TIME_BUTTON == 0)
    button = BUTTON_PRESSED_TIME_BUTTON;
  else if(DIRECTION_BUTTON == 0)
    button = BUTTON_PRESSED_DIRECTION_BUTTON;
  else if(SLEEP_WIND_BUTTON == 0)
    button = BUTTON_PRESSED_SLEEP_WIND_BUTTON;
  else if(WIND_POWER_BUTTON == 0)
    button = BUTTON_PRESSED_WIND_POWER_BUTTON;
  else
    button = BUTTON_PRESSED_NONE;

  // Disable Port Match interrupts to prevent multiple interrupts from
  // occurring while the switches are pressed
  EIE1 &= ~EIE1_EMAT__BMASK;
}

