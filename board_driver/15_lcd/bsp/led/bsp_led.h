#ifndef __BSP_LED_H
#define __BSP_LED_H
#include "imx6ull.h"

#define LED0	0

/* 函数声明 */
void led_init(void);
void led_switch(int led, int status);
#endif

