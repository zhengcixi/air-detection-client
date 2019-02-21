#ifndef __ESP8266_H
#define __ESP8266_H

#pragma anon_unions

#include "sys.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>

#define ESP8266_RST_PIN   GPIO_Pin_8
#define ESP8266_RST_PORT  GPIOB

#define RX_BUF_MAX_LEN     1024     //�����ջ����ֽ���
#define TX_BUF_MAX_LEN     1024     //����ͻ����ֽ���

#define macUser_ESP8266_ApSsid "MERCURY_6698"     //Ҫ���ӵ��ȵ������
#define macUser_ESP8266_ApPwd  "zxcvbnm123"   //Ҫ���ӵ��ȵ����Կ

#define macUser_ESP8266_TcpServer_IP    "192.168.1.104"  //Ҫ���ӵķ�������IP
#define macUser_ESP8266_TcpServer_Port  "8090"           //Ҫ���ӵķ������Ķ˿�

#define ESP8266_RST_HIGH_LEVEL()  GPIO_SetBits(ESP8266_RST_PORT, ESP8266_RST_PIN )
#define ESP8266_RST_LOW_LEVEL()   GPIO_ResetBits(ESP8266_RST_PORT, ESP8266_RST_PIN )
#define ESP8266_Usart(fmt, ... )  USART_printf(USART2, fmt, ##__VA_ARGS__ ) 

extern struct STRUCT_USARTx_Fram    //��������֡�Ĵ���ṹ��
{
	char Data_RX_BUF[RX_BUF_MAX_LEN];
	
	union 
	{
		__IO u16 InfAll;
		struct 
		{
			 __IO u16 FramLength       :15;   // 14:0 
			 __IO u16 FramFinishFlag   :1;    // 15 
		}InfBit;
	}; 
}strEsp8266_Fram_Record;

typedef enum{  //ģʽѡ��
	STA,
	AP,
	STA_AP  
}ENUM_Net_ModeTypeDef;

typedef enum{  //Э������ѡ��
	enumTCP,
	enumUDP,
}ENUM_NetPro_TypeDef;

typedef enum{
	Multiple_ID_0 = 0,
	Multiple_ID_1 = 1,
	Multiple_ID_2 = 2,
	Multiple_ID_3 = 3,
	Multiple_ID_4 = 4,
	Single_ID_0 = 5,
} ENUM_ID_NO_TypeDef;


void ESP8266_Init(void);
void ESP8266_StaTcpClient_Test(void);

#endif

