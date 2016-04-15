/*
 * IR_NEC_tx.h
 *
 *  Created on: Apr 15, 2016
 *      Author: Seungwon
 */

#ifndef IR_NEC_TX_H_
#define IR_NEC_TX_H_
#include <stdbool.h>
#include <stdint.h>
#include <board-conf.h>

/**
 * Configuration
 *
 */
#define IR_TX_PIN P2_B0

extern volatile bool _pause_done;

typedef enum
{
  IR_NEC_PAUSE_LEADER_CODE, IR_NEC_PAUSE_1, IR_NEC_PAUSE_0
} IR_NEC_PAUSE_t;

void
IR_NEC_tx_bit(bool val);

void
IR_NEC_tx_byte(uint8_t u8data);

void
IR_NEC_leader_code();

void
IR_NEC_tx_frame(uint8_t addr_data, uint8_t cmd_data);

void
IR_NEC_T3_setup();

void
IR_NEC_irq_callback();

void IR_NEC_pause(IR_NEC_PAUSE_t pause_type);

#endif /* IR_NEC_TX_H_ */
