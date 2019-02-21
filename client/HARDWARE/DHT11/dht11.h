#ifndef __DHT11_H
#define __DHT11_H

#include "sys.h"

typedef struct DHT11_Data_TypeDef
{
	uint8_t  humi_int;		//湿度的整数部分
	uint8_t  humi_deci;	 	//湿度的小数部分
	uint8_t  temp_int;	 	//温度的整数部分
	uint8_t  temp_deci;	 	//温度的小数部分
	uint8_t  check_sum;	 	//校验和
}DHT11_Data_TypeDef;

#define DHT11_PIN     GPIO_Pin_14
#define DHT11_PORT	  GPIOB

#define DHT11_HIGH()	GPIO_SetBits(DHT11_PORT, DHT11_PIN)    //设置引脚为高电平
#define DHT11_LOW() 	GPIO_ResetBits(DHT11_PORT, DHT11_PIN)  //设置引脚为低电平

#define ReadHumValue() 	GPIO_ReadInputDataBit(DHT11_PORT, DHT11_PIN)	//写成ReadValue将出现警告

uint8_t Read_DHT11(DHT11_Data_TypeDef *DHT11_Data);

#endif 
