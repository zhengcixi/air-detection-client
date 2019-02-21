#ifndef __DS18B20_H
#define __DS18B20_H

#include "sys.h"

#define DS18B20_DQ_OUT		PBout(0)
#define DS18B20_DQ_IN		PBin(0)

#define DS18B20_PORT	GPIOB
#define DS18B20_PIN		GPIO_Pin_0

#define ReadTmpValue() 	GPIO_ReadInputDataBit(DS18B20_PORT, DS18B20_PIN)

//ROM指令表
#define READROM			0x33
#define MATCHROM		0x55
#define SKIPROM			0xcc
#define SEARCHROM		0xf0
#define ALARMROM		0xec

//存储器操作指令表
#define WRITE_SCRATCHPAD	0x4e
#define READ_SCRATCHPAD		0xbe
#define COPY_SCRATCHPAD		0x48
#define CONVERT_T			0x44
#define RECALL_EEPROM		0xb8
#define READ_POWER_SUPPLY	0xb4

float DS18B20_Get_temperature(void);
uint8_t DS18B20_Init(void);

#endif
