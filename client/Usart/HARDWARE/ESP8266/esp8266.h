#ifndef __ESP8266_H
#define __ESP8266_H

#include "sys.h"


void esp8266_init();
void esp8266_GPIO_Config( void );
void esp8266_tcp_client();

//WIFI STAģʽ,����Ҫȥ���ӵ�·�������߲���,��������Լ���·��������,�����޸�.


#define esp8266_RST_PORT  GPIOB
#define esp8266_RST_PIN   GPIO_Pin_9
#define esp8266_RST_HIGH_LEVEL()  GPIO_SetBits(esp8266_RST_PORT, esp8266_RST_PIN)
#define esp8266_RST_LOW_LEVEL()   GPIO_ResetBits(esp8266_RST_PORT, esp8266_RST_PIN)

#define User_ESP8266_TcpServer_IP   "192.168.0.101"   //Ҫ���ӵķ������� IP�������Ե�IP
#define User_ESP8266_TcpServer_Port "8088"  //Ҫ���ӵķ������Ķ˿�

typedef enum{
	Multiple_ID_0 = 0,
	Multiple_ID_1 = 1,
	Multiple_ID_2 = 2,
	Multiple_ID_3 = 3,
	Multiple_ID_4 = 4,
	Single_ID_0 = 5,
} ENUM_ID_NO_TypeDef;

#endif
