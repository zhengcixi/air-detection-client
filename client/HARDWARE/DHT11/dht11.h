#ifndef __DHT11_H
#define __DHT11_H

#include "sys.h"

typedef struct DHT11_Data_TypeDef
{
	uint8_t  humi_int;		//ʪ�ȵ���������
	uint8_t  humi_deci;	 	//ʪ�ȵ�С������
	uint8_t  temp_int;	 	//�¶ȵ���������
	uint8_t  temp_deci;	 	//�¶ȵ�С������
	uint8_t  check_sum;	 	//У���
}DHT11_Data_TypeDef;

#define DHT11_PIN     GPIO_Pin_14
#define DHT11_PORT	  GPIOB

#define DHT11_HIGH()	GPIO_SetBits(DHT11_PORT, DHT11_PIN)    //��������Ϊ�ߵ�ƽ
#define DHT11_LOW() 	GPIO_ResetBits(DHT11_PORT, DHT11_PIN)  //��������Ϊ�͵�ƽ

#define ReadHumValue() 	GPIO_ReadInputDataBit(DHT11_PORT, DHT11_PIN)	//д��ReadValue�����־���

uint8_t Read_DHT11(DHT11_Data_TypeDef *DHT11_Data);

#endif 
