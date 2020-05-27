#ifndef __LED_H
#define __LED_H	 
#include "sys.h"
#include "delay.h"

// PIN MAP  C8T6 BOARD
//
// PB12     F3_FLOOR_LED
// PB13     F2_FLOOR_LED
// PB14     F1_FLOOR_LED
// PB15     ALARM_LED
// PA8      (RESERVED)
// PA9      UART_TX
// PA10     UART_RX
// PA11     (RESERVED:USB)
// PA12     (RESERVED:USB)
// PA15     (RESERVED)
// PB3      F3_DOWN
// PB4      F2_UP
// PB5      F2_DOWN
// PB6      F1_UP
// PB7      ELEVATOR_F3
// PB8      ELEVATOR_F2
// PB9      ELEVATOR_F1


#define F3_FLOOR_LED    PBout(12)   // PB12
#define F2_FLOOR_LED    PBout(13)   // PB13
#define F1_FLOOR_LED    PBout(14)   // PB14
#define ALARM_LED       PBout(15)   // PB15

#define F3_DOWN         PBin(3)     // PB3
#define F2_UP           PBin(4)     // PB4
#define F2_DOWN         PBin(5)     // PB5
#define F1_UP           PBin(6)     // PB6

#define ELEVATOR_F3     PBin(7)     // PB7
#define ELEVATOR_F2     PBin(8)     // PB8
#define ELEVATOR_F1     PBin(9)     // PB9

// Key Scan
#define KEY_F3_DOWN     1
#define KEY_F2_UP       2
#define KEY_F2_DOWN     3
#define KEY_F1_UP       4
#define KEY_ELEVATOR_F3 5
#define KEY_ELEVATOR_F2 6
#define KEY_ELEVATOR_F1 7

void LED_Init(void);//≥ı ºªØ
uint8_t KEY_Scan(void);

#endif
