#ifndef __ESP8266_H
#define __ESP8266_H

#include "sys.h"


void esp8266_init();
void esp8266_GPIO_Config( void );
void esp8266_tcp_client();

//WIFI STA模式,设置要去连接的路由器无线参数,请根据你自己的路由器设置,自行修改.


#define esp8266_RST_PORT  GPIOB
#define esp8266_RST_PIN   GPIO_Pin_9
#define esp8266_RST_HIGH_LEVEL()  GPIO_SetBits(esp8266_RST_PORT, esp8266_RST_PIN)
#define esp8266_RST_LOW_LEVEL()   GPIO_ResetBits(esp8266_RST_PORT, esp8266_RST_PIN)

#define User_ESP8266_TcpServer_IP   "192.168.0.101"   //要连接的服务器的 IP，即电脑的IP
#define User_ESP8266_TcpServer_Port "8088"  //要连接的服务器的端口

typedef enum{
	Multiple_ID_0 = 0,
	Multiple_ID_1 = 1,
	Multiple_ID_2 = 2,
	Multiple_ID_3 = 3,
	Multiple_ID_4 = 4,
	Single_ID_0 = 5,
} ENUM_ID_NO_TypeDef;

#endif
