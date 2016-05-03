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
#include "nPulse.h"

/**
 * Configuration
 *
 */

/**
 * @brief IR Tx pin
 */
#define IR_TX_PIN P1_B1

#define IR_NEC_MOD_FREQ 38000u
#define IR_NEC_TIMER_IRQ_FREQ (IR_NEC_MOD_FREQ * 3ul)
#define IR_NEC_PULSE_BURST_MAGIC_VALUE 65321u
#define IR_NEC_PAUSE_LEADER_CODE_MAGIC_VALUE 56349u
#define IR_NEC_PAUSE_1_MAGIC_VALUE 24595u
#define IR_NEC_PAUSE_0_MAGIC_VALUE 52158u

typedef enum
{
  IR_NEC_PAUSE_LEADER_CODE, IR_NEC_PAUSE_1, IR_NEC_PAUSE_0
} IR_NEC_PAUSE_t;

typedef enum
{
  IR_NEC_TX_STATE_IDLE = 0,
  IR_NEC_TX_STATE_LEADER_CODE_BURST,
  IR_NEC_TX_STATE_LEADER_CODE_PAUSE,
  IR_NEC_TX_STATE_ADDRESS_DATA,
  IR_NEC_TX_STATE_ADDRESS_NOT_DATA,
  IR_NEC_TX_STATE_COMMAND_DATA,
  IR_NEC_TX_STATE_COMMAND_NOT_DATA
//IR_NEC_TX_STATE_
} IR_NEC_TX_STATE_t;

typedef enum
{
  IR_NEC_TX_STATE_NEW_IDLE = 0,
  IR_NEC_TX_STATE_NEW_LEADER_CODE_BURST,
  IR_NEC_TX_STATE_NEW_LEADER_CODE_PAUSE,
  IR_NEC_TX_STATE_NEW_DATA_BURST_1,
  IR_NEC_TX_STATE_NEW_DATA_PAUSE_1,
  IR_NEC_TX_STATE_NEW_DATA_BURST_0,
  IR_NEC_TX_STATE_NEW_DATA_PAUSE_0,
  //IR_NEC_TX_STATE_NEW_DATA_PAUSE_BEFORE_TERMINATION_BURST,
  IR_NEC_TX_STATE_NEW_TERMINATION_BURST
//IR_NEC_TX_STATE_
} IR_NEC_TX_STATE_NEW_t;

typedef struct
{
  uint8_t Data[4];          // TODO: Magic number!!!
  //uint8_t dataArraySize;
  uint8_t byteIndex;
  uint8_t bitIndex;
  uint8_t loadedData;
} IR_NEC_tx_buffer_t;

typedef struct
{
  uint8_t Data[4];          // TODO: Magic number!!!
  //uint8_t dataArraySize;
  uint8_t index;
  uint8_t loadedData;
} IR_NEC_tx_buffer_v3_t;

typedef struct
{
  pulse_t pulse;
  uint8_t isPauseDone;
  IR_NEC_TX_STATE_t state;
} IR_NEC_TX_t;

void
IR_NEC_tx_bit(bool val);

void
IR_NEC_tx_stopBit();

void
IR_NEC_tx_byte(uint8_t u8data);

void
IR_NEC_leader_code();

uint8_t
IR_NEC_tx_frame(uint8_t addr_data, uint8_t cmd_data);

void
IR_NEC_T3_setup();

void
IR_NEC_irq_callback();

void
IR_NEC_pause(IR_NEC_PAUSE_t pause_type);

uint8_t
IR_NEC_tx_buffer_preload(volatile IR_NEC_tx_buffer_v3_t * buffer) reentrant;

void
IR_NEC_tx_stateMachine();

void
IR_NEC_tx_next_state(volatile IR_NEC_TX_t* ir_nec_tx_handle);

// Global variables
extern volatile bool _pause_done;
extern volatile IR_NEC_TX_t ir_nec_tx_handle;
extern volatile IR_NEC_TX_STATE_NEW_t ir_nec_tx_state_new;
extern volatile IR_NEC_tx_buffer_t ir_nec_tx_buffer;
extern volatile IR_NEC_tx_buffer_v3_t ir_nec_tx_buffer_v3;

#endif /* IR_NEC_TX_H_ */
