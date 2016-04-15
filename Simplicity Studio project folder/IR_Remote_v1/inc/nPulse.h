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
} pulse_t;

extern pulse_t pulse;

uint16_t
nPulse(uint16_t pulseCount, volatile pulse_t* pulse);

#endif /* NPULSE_H_ */

