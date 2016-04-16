/*
 * nPulse.h
 *
 *  Created on: Apr 3, 2016
 *      Author: Seungwon
 */

#ifndef NPULSE_H_
#define NPULSE_H_

#include <stdint.h>
#include <stdbool.h>

typedef struct
{
  uint16_t count;
  uint8_t isActive;
  uint8_t _intCnt;
  void (*pulseDone_callback)();
} pulse_t;


uint16_t
nPulse(uint16_t pulseCount, volatile pulse_t* pulse);

void
nPulse_irq_callback();

// Global variables
extern pulse_t pulse;
//extern volatile pulse_t* p_pulse;

#endif /* NPULSE_H_ */

