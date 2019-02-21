#ifndef __PMS_H
#define __PMS_H

#include "sys.h"
#include <stdio.h>

#define PMS5003_RST_PIN   GPIO_Pin_9
#define PMS5003_RST_PORT  GPIOB

#define PMS5003_RST_HIGH_LEVEL()  GPIO_SetBits(PMS5003_RST_PORT, PMS5003_RST_PIN )
#define PMS5003_RST_LOW_LEVEL()   GPIO_ResetBits(PMS5003_RST_PORT, PMS5003_RST_PIN )


typedef uint16_t pmsData;
typedef uint8_t  pmsIndex;

typedef struct pms_data_typedef{
	pmsData StartSymbol;          //起始符
	pmsData FrameLength;          //帧长度
	pmsData	PM1dot0CF1;		      //  1  标准颗粒物PM1.0的浓度
	pmsData	PM2dot5CF1;			  //  2  标准颗粒物PM2.5的浓度
	pmsData	PM10dot0CF1;		  //  3  标准颗粒物PM10的浓度
	pmsData	PM1dot0;			  //  4  大气环境下PM1.0的浓度
	pmsData	PM2dot5;			  //  5  大气环境下PM2.5的浓度
	pmsData	PM10dot0;			  //  6  大气环境下PM10的浓度
	pmsData	Particles0dot3;		  //  7  0.1升空气中直径在0.3um以下颗粒物个数
	pmsData	Particles0dot5;		  //  8  0.1升空气中直径在0.5um以下颗粒物个数
	pmsData	Particles1dot0;		  //  9  0.1升空气中直径在1.0um以下颗粒物个数
	pmsData	Particles2dot5;		  // 10  0.1升空气中直径在2.5um以下颗粒物个数
	pmsData	Particles5dot0;		  // 11  0.1升空气中直径在5.0um以下颗粒物个数
	pmsData	Particles10;		  // 12  0.1升空气中直径在10um以下颗粒物个数
	uint8_t version;	          // 13  校验码
	uint8_t error_num;            // 13  错误代码
}pms_data_typedef;


void PMS5003_Init(void);
void Read_PMS5003(pms_data_typedef *PMS5003_Data);

#endif

