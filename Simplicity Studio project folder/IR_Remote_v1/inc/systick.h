/*
 * systic.h
 *
 *  Created on: Apr 12, 2016
 *      Author: Seungwon
 */

#ifndef SYSTIC_H_
#define SYSTIC_H_

#include <stdint.h>

#ifndef SYS_TICK_FREQ
#warning "Please define SYS_TICK_FREQ value!"
#define SYS_TICK_FREQ 1000L
#endif

#ifndef SYS_TICK_TIMER_FREQ
#warning "Please define SYS_TICK_TIMER_FREQ value!"
#define SYS_TICK_TIMER_FREQ 24500000UL
#endif



extern volatile uint32_t _sys_tick;

extern uint32_t
get_sys_tick();

#endif /* SYSTIC_H_ */
