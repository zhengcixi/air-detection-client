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
	pmsData StartSymbol;          //��ʼ��
	pmsData FrameLength;          //֡����
	pmsData	PM1dot0CF1;		      //  1  ��׼������PM1.0��Ũ��
	pmsData	PM2dot5CF1;			  //  2  ��׼������PM2.5��Ũ��
	pmsData	PM10dot0CF1;		  //  3  ��׼������PM10��Ũ��
	pmsData	PM1dot0;			  //  4  ����������PM1.0��Ũ��
	pmsData	PM2dot5;			  //  5  ����������PM2.5��Ũ��
	pmsData	PM10dot0;			  //  6  ����������PM10��Ũ��
	pmsData	Particles0dot3;		  //  7  0.1��������ֱ����0.3um���¿��������
	pmsData	Particles0dot5;		  //  8  0.1��������ֱ����0.5um���¿��������
	pmsData	Particles1dot0;		  //  9  0.1��������ֱ����1.0um���¿��������
	pmsData	Particles2dot5;		  // 10  0.1��������ֱ����2.5um���¿��������
	pmsData	Particles5dot0;		  // 11  0.1��������ֱ����5.0um���¿��������
	pmsData	Particles10;		  // 12  0.1��������ֱ����10um���¿��������
	uint8_t version;	          // 13  У����
	uint8_t error_num;            // 13  �������
}pms_data_typedef;


void PMS5003_Init(void);
void Read_PMS5003(pms_data_typedef *PMS5003_Data);

#endif

