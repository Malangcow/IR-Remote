/*
 * button.c
 *
 *  Created on: May 3, 2016
 *      Author: Seungwon
 */

#include <stdint.h>
#include "button.h"
#include "IR_NEC_tx.h"

volatile BUTTON_PRESSED_t button;

void
button_handler(BUTTON_PRESSED_t b)
{
//  uint8_t addr;
//  uint8_t cmd;

  switch (b)
    {
  case BUTTON_PRESSED_NONE:
    break;
  case BUTTON_PRESSED_STOP_BUTTON:
    IR_NEC_tx_frame(LIGHT_ADDRESS, LIGHT_COMMAND_TOP_RIGHT);
    break;
  case BUTTON_PRESSED_TIME_BUTTON:
    IR_NEC_tx_frame(LIGHT_ADDRESS, LIGHT_COMMAND_BOTTOM_LEFT);
    break;
  case BUTTON_PRESSED_DIRECTION_BUTTON:
    /**
     * @todo P3.1 is not port match available.\n
     * Need to hardware pin change required.
     */
    break;
  case BUTTON_PRESSED_SLEEP_WIND_BUTTON:
    IR_NEC_tx_frame(LIGHT_ADDRESS, LIGHT_COMMAND_BOTTOM_RIGHT);
    break;
  case BUTTON_PRESSED_WIND_POWER_BUTTON:
    IR_NEC_tx_frame(LIGHT_ADDRESS, LIGHT_COMMAND_TOP_LEFT);

    break;
  default:
    break;

    }


  // before enter LPM check if transmission is done
  while (ir_nec_tx_state_new != IR_NEC_TX_STATE_NEW_IDLE)
    ;

  // Port match event handled. Re-enable PM interrupt.
  SFRPAGE = 0x0;
  EIE1 |= EIE1_EMAT__BMASK;
}
