#ifndef __LED_H
#define	__LED_H

#include "sys.h"

//#define ON 0
//#define OFF 1


//LED 端口定义
#define LED1 PBout(0)		// DS1
#define LED2 PFout(7)		// DS2
#define LED3 PFout(8)		// DS3



///* 带参宏，可以像内联函数一样使用 */
//#define LED1(a)	if (a)	\
//					GPIO_SetBits(GPIOB,GPIO_Pin_0);\
//					else		\
//					GPIO_ResetBits(GPIOB,GPIO_Pin_0)

//#define LED2(a)	if (a)	\
//					GPIO_SetBits(GPIOF,GPIO_Pin_7);\
//					else		\
//					GPIO_ResetBits(GPIOF,GPIO_Pin_7)

//#define LED3(a)	if (a)	\
//					GPIO_SetBits(GPIOF,GPIO_Pin_8);\
//					else		\
//					GPIO_ResetBits(GPIOF,GPIO_Pin_8)

void LED_GPIO_Config(void);

#endif
