/*
 * atomic.h
 *
 *  Created on: Apr 3, 2016
 *      Author: Seungwon
 */

#ifndef ATOMIC_H_
#define ATOMIC_H_

#include <stdint.h>
#include <SI_EFM8BB2_Register_Enums.h>

#define ATOMIC_BLOCK_START { uint8_t IE_State = IE_EA;\
  IE_EA = 0;



#define ATOMIC_BLOCK_END IE_EA = IE_State;\
  }

#endif /* ATOMIC_H_ */
