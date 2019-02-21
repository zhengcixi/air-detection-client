#ifndef __KEY_H
#define __KEY_H

#include "sys.h"

//#define key1	PAin(0)
//#define key_up	PCin(13)

#define KEY2  GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_13)		//读取按键2 
#define WK_UP   GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0)		//读取按键3(WK_UP)

#define KEY2_PRES	2	//KEY2按下
#define WKUP_PRES   1	//KEY_UP按下(即WK_UP/KEY_UP)

void Key_init(void);
u8 KEY_Scan(u8); 	 	//按键扫描函数	

#endif
