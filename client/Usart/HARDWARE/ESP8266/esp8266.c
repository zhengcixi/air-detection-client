#include "usart2.h"
#include "delay.h"
#include "string.h"
#include "stdio.h"
#include "usart.h"
#include "esp8266.h"

char* wifista_ssid       = "WERCURY_6698";			//路由器SSID号
char* wifista_encryption = "wpawpa2_aes";	    //wpa/wpa2 aes加密方式
char* wifista_password   = "zxcvbnm147"; 	    //连接密码

/* 配置WiFi模块的复位重启（RST）引脚*/
void esp8266_GPIO_Config( void )
{		
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA, ENABLE );  /*开启GPIOA的外设时钟*/

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;  /*选择要控制的GPIOA0（RST）引脚*/		
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  /*设置引脚模式为通用推挽输出*/
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; /*设置引脚速率为50MHz */   
	GPIO_Init( GPIOA, &GPIO_InitStructure );  /*调用库函数，初始化GPIOA*/	 	
	GPIO_SetBits( GPIOA, GPIO_Pin_1 );  /* 拉高WiFi模块的复位重启引脚	*/
}


/*——————————————————————————————
ESP8266发送命令后,检测接收到的应答
str:期待的应答结果
返回值：0,没有得到期待的应答结果
        其他,期待应答结果的位置(str的位置)
注：strstr(char* str, char* substr) ==>返回字符串str中第一次出现子串substr的地址
———————————————————————————————*/
u8* esp8266_check_cmd(u8 *str)
{
	char* strx=NULL;

	if(USART2_RX_STA & 0X8000)  //接收到一次数据了
	{ 
		USART2_RX_BUF[USART2_RX_STA&0X7FFF] = 0;    //添加结束符
		strx=strstr((const char*)USART2_RX_BUF, (const char*)str);
	} 
	return (u8*)strx;  //返回接收到的数据
}

/*————————————————————————————
向ATK-ESP8266发送命令
cmd:发送的命令字符串
ack:期待的应答结果,如果为空,则表示不需要等待应答
waittime:等待时间(单位:10ms)
返回值:0,发送成功(得到了期待的应答结果)
       1,发送失败
—————————————————————————————*/
u8 esp8266_send_cmd(u8 *cmd, u8 *ack, u16 waittime)
{
	u8 res = 0; 
	USART2_RX_STA = 0;
	u2_printf("%s\r\n", cmd); //发送命令
	if(ack && waittime)		  //需要等待应答，当ack!=null并且waittime!=0时执行if中的语句（很巧妙）
	{
		while(--waittime)	  //等待倒计时
		{
			delay_ms(10);
			if(USART2_RX_STA & 0X8000)  //接收到期待的应答结果（bit15为接收数据标志位）
			{
				if(esp8266_check_cmd(ack))
				{
					printf("ack:%s\r\n",(u8*)ack);  //打印出返回结果
					break;//得到有效数据 
				}
					USART2_RX_STA = 0;  //清空标志位
			} 
		}
		if(waittime==0)
			res=1; 
	}
	return res;
} 

/*—————————————
ATK-ESP8266退出透传模式
返回值:0,退出成功;
       1,退出失败
注：SR:状态寄存器，bit6:TC(发送完成位，发送完成值1)
	DR:数据寄存器，只有bit0--bit8为有效位
退出透传指令：连续发送三个"+++"
—————————————*/
u8 esp8266_quit_trans(void)
{
	while((USART2->SR & 0X40)==0);	//等待发送空
	USART2->DR = '+';      
	delay_ms(15);					//大于串口组帧时间(10ms)
	while((USART2->SR & 0X40)==0);	//等待发送空
	USART2->DR = '+';      
	delay_ms(15);					//大于串口组帧时间(10ms)
	while((USART2->SR & 0X40)==0);	//等待发送空
	USART2->DR = '+';      
	delay_ms(500);					//等待500ms
	return esp8266_send_cmd((u8*)"AT",(u8*)"OK",20);//退出透传判断.
}


/*————————————————————
 * 函数名：ESP8266_JoinAP
 * 描述  ：WF-ESP8266模块连接外部WiFi
 * 输入  ：pSSID，WiFi名称字符串
 *       ：pPassWord，WiFi密码字符串
 * 返回  : 1，连接成功
 *         0，连接失败
 * 调用  ：被外部调用
 ————————————————————*/
u8 ESP8266_JoinAP (char * pSSID, char* pPassWord)
{
	char* cCmd;

	sprintf(cCmd, "AT+CWJAP=\"%s\",\"%s\"", pSSID, pPassWord );
	return esp8266_send_cmd(cCmd, "OK", 1000);
}

/*——————————————————————
 * 函数名：ESP8266_Link_Server
 * 描述  ：WF-ESP8266模块连接外部服务器
 * 输入  ：enumE，网络协议
 *       ：ip，服务器IP字符串
 *       ：ComNum，服务器端口字符串
 *       ：id，模块连接服务器的ID
 * 返回  : 1，连接成功
 *         0，连接失败
 * 调用  ：被外部调用
———————————————————————*/
u8 ESP8266_Link_Server(char* ip, char* ComNum, ENUM_ID_NO_TypeDef id)
{
	char* cStr, *cCmd;

	sprintf ( cStr, "\"%s\",\"%s\",%s", "TCP", ip, ComNum );
	if ( id < 5 )
		sprintf (cCmd, "AT+CIPSTART=%d,%s", id, cStr);
	else
		sprintf (cCmd, "AT+CIPSTART=%s", cStr );  //建立TCP连接

	return esp8266_send_cmd(cCmd, "OK", 400);
}

/*——————————————————————
 * 函数名：ESP8266_UnvarnishSend
 * 描述  ：配置WF-ESP8266模块进入透传发送
 * 输入  ：无
 * 返回  : 1，配置成功
 *         0，配置失败
 * 调用  ：被外部调用
 ——————————————————————*/
u8 ESP8266_UnvarnishSend ( void )
{
	if ( esp8266_send_cmd("AT+CIPMODE=1", "OK", 50) )
		return 0;
	return 
	  esp8266_send_cmd("AT+CIPSEND", "OK", 50);
}

/*————————————————————————————
 * 函数名：ESP8266_SendString
 * 描述  ：WF-ESP8266模块发送字符串
 * 输入  ：enumEnUnvarnishTx，声明是否已使能了透传模式
 *       ：pStr，要发送的字符串
 *       ：ulStrLength，要发送的字符串的字节数
 *       ：ucId，哪个ID发送的字符串
 * 返回  : 1，发送成功
 *         0，发送失败
 * 调用  ：被外部调用
 ————————————————————————————*/
/*
u8 ESP8266_SendString (char * pStr, u32 ulStrLength, ENUM_ID_NO_TypeDef ucId )
{
	char cStr [20];
	u8 bRet = 0;
	
		
	if ( enumEnUnvarnishTx )
	{
		u2_printf ( "%s", pStr );
		
		bRet = 1;
		
	}
	return bRet;
}
*/


/*——————————————————————————————————
功能：测试串口无线STA模式，模块作为无线WIFI STA连接到无线网络，用作TCP客户端
返回值：0，正常
		其它，错误代码
指令介绍：AT:测试AT启动
		  AT+CIPMODE=0退出透传   AT+CIPMODE=2设置成路由模式
		  ATE0:关闭回显
——————————————————————————————————*/
void esp8266_tcp_client()
{
	u8* p;
	char* cStr;
	u8 res;
	sprintf( cStr, "ABCDEFGHIJKLMNOPQRSTUVWXYZ\r\n" );
	
	printf("\r\nESP8266 WIFI模块测试STA模式，用作TCP客户端\r\n");
	while( !esp8266_send_cmd((u8*)"AT", (u8*)"OK", 10))  //检查WIFI是否在线，如果在线则会返回"OK"
	{
		esp8266_quit_trans();  //退出透传
		esp8266_send_cmd((u8*)"AT+CIPMODE=0", (u8*)"OK", 200); //关闭透传模式
		printf("\r\n未检测到WIFI模块！\r\n");
		delay_ms(800);
		printf("\r\n尝试连接到模块！\r\n");
	}
	esp8266_send_cmd((u8*)"AT+GMR", (u8*)"OK", 20);
	p = esp8266_check_cmd((u8*)"SDK version:");
	printf("固件版本：%s\r\n", p);
	
//	while(esp8266_send_cmd((u8*)"ATE0", (u8*)"OK", 20));   //关闭回显
	esp8266_send_cmd((u8*)"AT+CWMODE=1",(u8*)"OK", 200);   //设置成STA模式
	while ( !ESP8266_JoinAP(wifista_ssid, wifista_password) );  //连接WIFI热点
	res = esp8266_send_cmd("AT+CIPMUX=0", "OK", 50);  //单连接模式
	printf("res = %d\r\n", res);
	while ( !ESP8266_Link_Server(User_ESP8266_TcpServer_IP, User_ESP8266_TcpServer_Port, Single_ID_0));
	while( !ESP8266_UnvarnishSend() );
	printf ( "\r\n配置 ESP8266 完毕\r\n" );
	while(1)
	{
		u2_printf("%s", cStr);
		delay_ms(1000);
	}
	
/*
	esp8266_send_cmd((u8*)"AT+RST",(u8*)"OK",20);  	       //重启模块
	delay_ms(1000);
	delay_ms(1000);
	delay_ms(1000);
	delay_ms(1000);
	sprintf((char*)p, "AT+CWJAP=\"%s\",\"%s\"", wifista_ssid, wifista_password);
	esp8266_send_cmd(p, (u8*)"OK", 1000);  //连接wifi热点
	esp8266_send_cmd("AT+CIPMUX=0", (u8*)"OK", 20);  //开启单连接 
	esp8266_send_cmd("AT+CIPSTART=\"TCP\",\"192.168.0.101\",8086", (u8*)"OK", 1000);
	esp8266_send_cmd("AT+CIPMODE=1", "OK", 1000);
	esp8266_send_cmd("AT+CIPSEND=2,6", "OK", 1000);
	esp8266_send_cmd("123456", "OK", 1000);
*/
}

void esp8266_init()
{
	u8* p;
	
	esp8266_GPIO_Config();     //引脚配置
	esp8266_RST_HIGH_LEVEL();  //复位引脚设置为高电平
	delay_ms(1000);
	esp8266_send_cmd((u8*)"AT+GMR",(u8*)"OK",20);
	p = esp8266_check_cmd((u8*)"SDK version:");
	printf("固件版本：%s\r\n", p);
}
/*
	esp8266_send_cmd((u8*)"AT+CWMODE?",(u8*)"+CWMODE:",20);	//获取网络模式
	p = esp8266_check_cmd((u8*)":");
	printf("网络模式%s\r\n", p);
	esp8266_check_cmd("AT+CWSAP?","+CWSAP:",20);	//获取wifi配置
*/
















