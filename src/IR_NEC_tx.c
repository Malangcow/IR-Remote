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
void
IR_NEC_tx_frame(uint8_t addr_data, uint8_t cmd_data)
{
  IR_NEC_leader_code();

  IR_NEC_tx_byte(addr_data);
  addr_data = ~addr_data;  // Complement
  IR_NEC_tx_byte(addr_data);

  IR_NEC_tx_byte(cmd_data);
  cmd_data = ~cmd_data;     // Complement
  IR_NEC_tx_byte(cmd_data);
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

  ATOMIC_BLOCK_START
      {
        _pause_done = 0;
      }
    ATOMIC_BLOCK_END

  while (!_pause_done) {
      //enter_power_mode(POWER_MODE_IDLE); // This corrupts timing do not enter LPM
  }



}

/**
 * @Note Not used. Yet.
 */
void
IR_NEC_irq_callback()
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
        IR_TX_PIN = 1;
        if (pulse.count > 0)
          { // pulse start, decrease counter
            --pulse.count;
          }
        ++_intCnt_temp;
        break;
      case 2:
        IR_TX_PIN = 0;
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
  _pause_done = 1; // exit pause mode
}

