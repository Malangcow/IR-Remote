/*
 * button.h
 *
 *  Created on: May 3, 2016
 *      Author: Seungwon
 */

#ifndef BUTTON_H_
#define BUTTON_H_

#define STOP_BUTTON P2_B1
#define TIME_BUTTON P2_B0
#define DIRECTION_BUTTON P3_B1
#define SLEEP_WIND_BUTTON P2_B3
#define WIND_POWER_BUTTON P2_B2


#define LIGHT_ADDRESS              0x01
#define LIGHT_COMMAND_TOP_LEFT     0x06
#define LIGHT_COMMAND_TOP_RIGHT    0x0E
#define LIGHT_COMMAND_BOTTOM_LEFT  0x26
#define LIGHT_COMMAND_BOTTOM_RIGHT 0x1D


typedef enum {
  BUTTON_PRESSED_NONE,
  BUTTON_PRESSED_STOP_BUTTON,
  BUTTON_PRESSED_TIME_BUTTON,
  BUTTON_PRESSED_DIRECTION_BUTTON,
  BUTTON_PRESSED_SLEEP_WIND_BUTTON,
  BUTTON_PRESSED_WIND_POWER_BUTTON,
} BUTTON_PRESSED_t;


void button_handler(BUTTON_PRESSED_t button);


/**
 * @note Global variable
 */
extern volatile BUTTON_PRESSED_t button;

#endif /* BUTTON_H_ */
