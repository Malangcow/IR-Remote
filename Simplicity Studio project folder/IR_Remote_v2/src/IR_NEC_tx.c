/*
 * IR_NEC_tx.c
 *
 *  Created on: Apr 15, 2016
 *      Author: Seungwon
 */
#include <stdint.h>
#include <stdbool.h>
#include <board-conf.h>
#include "IR_NEC_tx.h"
#include "nPulse.h"
#include "sleep.h"
#include "atomic.h"

/**
 * @brief Flag variable for IR_NEC_pause function.
 */
volatile bool _pause_done;

/**
 * @brief Buffer for IR NEC Tx.\n
 * Used for communication between IRQ and main.
 */
volatile IR_NEC_tx_buffer_t ir_nec_tx_buffer;
volatile IR_NEC_tx_buffer_v3_t ir_nec_tx_buffer_v3;

volatile IR_NEC_TX_STATE_NEW_t ir_nec_tx_state_new = 0;

/**
 * @brief Transmits 1 bit
 * @param val value of bit data to transmit
 */
void
IR_NEC_tx_bit(bool val)
{
  IR_NEC_T3_setup();       // Setup T3 for modulation
  if (val == 1)
    { // Transmit "1"
      // 22 pulse burst
      nPulse(22, &pulse);
      // wait for pulse burst ends
      while (pulse.isActive)
        ;
      // pause 1671.05263 us
      IR_NEC_pause(IR_NEC_PAUSE_1);

    }
  else
    { // Transmit "0"
      // 22 pulse burst
      nPulse(22, &pulse);
      while (pulse.isActive)
        ;
      // pause  546.05263 us
      IR_NEC_pause(IR_NEC_PAUSE_0);

    }
}
void
IR_NEC_tx_stopBit()
{
  IR_NEC_T3_setup();       // Setup T3 for modulation
  nPulse(22, &pulse);
}

void
IR_NEC_tx_byte(uint8_t u8data)
{
  uint8_t i;
  for (i = 0; i < 8; ++i)
    {
      IR_NEC_tx_bit(u8data & 0x01);
      u8data >>= 1;
    }
}

void
IR_NEC_leader_code()
{
  IR_NEC_T3_setup();       // Setup T3 for modulation
  // pulse burst 9 ms (342 pulse)
  nPulse(342, &pulse);
  while (pulse.isActive)
    ;
  // pause 4.5 ms
  IR_NEC_pause(IR_NEC_PAUSE_LEADER_CODE);
}

/**
 * @brief Transmits NEC frame
 *
 * Transmits in the order of Address, ~Address, Command, ~Command\n
 *
 * @note LSB is Transmitted first.
 *
 * @param addr_data
 * @param cmd_data
 */

uint8_t
IR_NEC_tx_frame(uint8_t addr_data, uint8_t cmd_data)
{
  IR_NEC_TX_STATE_NEW_t state_now;
  // check state

  state_now = ir_nec_tx_state_new;

  if (state_now != IR_NEC_TX_STATE_NEW_IDLE)
    return -1; // It's busy

  // initialize buffer
  ir_nec_tx_buffer_v3.Data[0] = addr_data;
  ir_nec_tx_buffer_v3.Data[1] = ~addr_data;
  ir_nec_tx_buffer_v3.Data[2] = cmd_data;
  ir_nec_tx_buffer_v3.Data[3] = ~cmd_data;

  ir_nec_tx_buffer_v3.index = 0;

  // Doesn't need to preload. It will be done in the ISR
  //IR_NEC_tx_buffer_preload(&ir_nec_tx_buffer_v3);

  // setup timer for leader code burst
  IR_NEC_T3_setup();       // Setup T3 for modulation
  // pulse burst 9 ms (342 pulse)

  // TODO: Is this critical section?
  nPulse(342, &pulse);

  ATOMIC_BLOCK_START
      {
        ir_nec_tx_state_new = IR_NEC_TX_STATE_NEW_LEADER_CODE_BURST;
      }
    ATOMIC_BLOCK_END

  return 0;

}

/**
 * @brief Setup Timer to generate 38 kHz pulse burst
 *
 * 16-bit auto reload, 38 kHz * 3 = 114 KHz,
 * enable Timer interrupt,
 *
 * @note All interrupts enable(EA bit) should be enabled.\n
 * Assumes SYSCLK equals 24.5 MHz.
 */
void
IR_NEC_T3_setup()
{
#define IR_NEC_MOD_FREQ 38000u
#define IR_NEC_TIMER_IRQ_FREQ (IR_NEC_MOD_FREQ * 3ul)
  uint16_t reloadVal;
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
  reloadVal = 65536u
      - (SYS_TICK_TIMER_FREQ + IR_NEC_TIMER_IRQ_FREQ / 2)
          / IR_NEC_TIMER_IRQ_FREQ;

  //reloadVal = 65536u - SYS_TICK_TIMER_FREQ / IR_NEC_TIMER_IRQ_FREQ;

  TMR3RLH = reloadVal >> 8;
  TMR3H = reloadVal >> 8;
  TMR3RLL = reloadVal;
  TMR3L = reloadVal;

  // Interrupts
  //SFRPAGE = 0x10;
  EIE1 |= EIE1_ET3__ENABLED; // Enable T3 IRQ
  EIP1H |= EIP1H_PHT3__HIGH; // T3 priority: 3(highest)
  EIP1 |= EIP1_PT3__HIGH;

  TMR3CN0 |= TMR3CN0_TR3__RUN; // Run Timer

}

/**
 * @brief Setup wake up source and goto low power mode.
 *
 * Idle mode is used because in other modes like suspend mode or snooze mode
 * wake up source is T4 clocked by slow LFOSC0 which means low resolution.
 *
 * @bug Currently entering low power mode while pause is not working.
 * Entering idle mode while pause corrupts the timing.
 *
 * @note T3 IRQ is used for wake up source.\n
 * Also assumes T3 IRQ is enabled.
 */
void
IR_NEC_pause(IR_NEC_PAUSE_t pauseType)
{
  // IR_NEC_PAUSE_LEADER_CODE:    4.5      ms
  // IR_NEC_PAUSE_1          : 1671.052632 us
  // IR_NEC_PAUSE_0          :  546.052632 us

  //uint8_t regTemp;

  switch (pauseType)
    {
  case IR_NEC_PAUSE_LEADER_CODE: // 4.5 ms
    // T CLK: SYSCLK/12
    // Initial value: 56349
#define IR_NEC_PAUSE_LEADER_CODE_MAGIC_VALUE 56349u

    SFRPAGE = 0x10;
    TMR3CN0 = 0; // Stop and resets timer. This also make T3XCLK SYSCLK/12
    // CLK source EXTCLK, SYSCLK/12
    CKCON0 &= ~(CKCON0_T3MH__BMASK | CKCON0_T3ML__BMASK);

    // Set initial value
    TMR3H = IR_NEC_PAUSE_LEADER_CODE_MAGIC_VALUE >> 8;
    TMR3L = IR_NEC_PAUSE_LEADER_CODE_MAGIC_VALUE;

    TMR3RLH = 0;
    TMR3RLL = 0;

    // Since we will use this timer as one shot timer,
    // reload value doesn't matters

    // Run Timer
    TMR3CN0 |= TMR3CN0_TR3__RUN;

    break;
  case IR_NEC_PAUSE_1:
    // T CLK: SYSCLK
    // Initial value: 24595
#define IR_NEC_PAUSE_1_MAGIC_VALUE 24595u
    SFRPAGE = 0x10;
    TMR3CN0 = 0; // Stop and resets timer.

    // CLK source, SYSCLK
    CKCON0 |= CKCON0_T3MH__EXTERNAL_CLOCK | CKCON0_T3ML__EXTERNAL_CLOCK;
    // Set initial value
    TMR3H = IR_NEC_PAUSE_1_MAGIC_VALUE >> 8;
    TMR3L = IR_NEC_PAUSE_1_MAGIC_VALUE;

    TMR3RLH = 0;
    TMR3RLL = 0;

    // No need to configure reload value.

    // Run Timer
    TMR3CN0 |= TMR3CN0_TR3__RUN;

    break;
  case IR_NEC_PAUSE_0:
    // T CLK: SYSCLK
    // Initial value: 52158
#define IR_NEC_PAUSE_0_MAGIC_VALUE 52158u
    SFRPAGE = 0x10;
    TMR3CN0 = 0; // Stop and resets timer.

    // CLK source, SYSCLK
    CKCON0 |= CKCON0_T3MH__EXTERNAL_CLOCK | CKCON0_T3ML__EXTERNAL_CLOCK;
    // Set initial value
    TMR3H = IR_NEC_PAUSE_0_MAGIC_VALUE >> 8;
    TMR3L = IR_NEC_PAUSE_0_MAGIC_VALUE;

    TMR3RLH = 0;
    TMR3RLL = 0;

    // No need to configure reload value.

    // Run Timer
    TMR3CN0 |= TMR3CN0_TR3__RUN;

    break;

  default:
    return;
    }

//  ATOMIC_BLOCK_START
//      {
//        _pause_done = 0;
//      }
//    ATOMIC_BLOCK_END
//
//  while (!_pause_done)
//    {
////enter_power_mode(POWER_MODE_IDLE); // This corrupts timing do not enter LPM
//    }

}

/**
 *
 * @param buffer
 * @return -1: no more data to preload/n
 *  0: preloaded "0"
 *  1: preloaded "1"
 *
 * This function will load next data to loadedData.
 * @note Assumes buffer index is initialized.
 */
// TODO: Is there any other way not to use this "reentrant" attribute?
uint8_t
IR_NEC_tx_buffer_preload(volatile IR_NEC_tx_buffer_v3_t * buffer) reentrant
{
#warning  "Not tested"
  uint8_t index_temp;
  uint8_t byteIndex;
  uint8_t data_temp;
  index_temp = buffer->index;
  byteIndex = index_temp >> 3;
  if (byteIndex < 4)  // TODO: Magic number!!!
    {
      data_temp = buffer->Data[byteIndex];                 // get data byte
      data_temp = (data_temp >> (index_temp & 0x7)) & 0x1; // extract bit
      buffer->loadedData = data_temp;                      // preload data
      ++index_temp;
      buffer->index = index_temp;
      return data_temp;
    }
  else
    {
      // index is equal or greater than 4 which means no more data to Tx.
      buffer->loadedData = -1;              // preload data with -1
      return -1;
    }

}

/**
 * @brief State machine.
 * @note This function is not called.\n
 *  Inline code of this function to the Timer ISR.
 */
void
IR_NEC_irq_callback()
{
// clear IRQ flag

  //
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
    TMR3CN0 = 0; // Stop and resets timer. This also make T3XCLK SYSCLK/12
    // CLK source EXTCLK, SYSCLK/12
    CKCON0 &= ~(CKCON0_T3MH__BMASK | CKCON0_T3ML__BMASK);

    // Set initial value
    TMR3H = IR_NEC_PAUSE_LEADER_CODE_MAGIC_VALUE >> 8;
    TMR3L = IR_NEC_PAUSE_LEADER_CODE_MAGIC_VALUE;

    // Since we will use this timer as one shot timer,
    // reload value doesn't matters
    //TMR3RLH = 0;
    //TMR3RLL = 0;

    TMR3CN0 |= TMR3CN0_TR3__RUN; // Run Timer

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

      TMR3CN0 |= TMR3CN0_TR3__RUN; // Run Timer

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
    TMR3CN0 = 0; // Stop and resets timer.

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
    TMR3CN0 = 0; // Stop and resets timer.

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

      TMR3CN0 |= TMR3CN0_TR3__RUN; // Run Timer

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
    TMR3CN0 = 0; // Stop and resets Timer

    ir_nec_tx_state_new = IR_NEC_TX_STATE_NEW_IDLE;

    break;

  default:
    break;
    } // end of switch case

}

