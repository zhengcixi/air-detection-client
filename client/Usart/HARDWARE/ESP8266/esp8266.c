#include "usart2.h"
#include "delay.h"
#include "string.h"
#include "stdio.h"
#include "usart.h"
#include "esp8266.h"

char* wifista_ssid       = "WERCURY_6698";			//·����SSID��
char* wifista_encryption = "wpawpa2_aes";	    //wpa/wpa2 aes���ܷ�ʽ
char* wifista_password   = "zxcvbnm147"; 	    //��������

/* ����WiFiģ��ĸ�λ������RST������*/
void esp8266_GPIO_Config( void )
{		
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA, ENABLE );  /*����GPIOA������ʱ��*/

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;  /*ѡ��Ҫ���Ƶ�GPIOA0��RST������*/		
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  /*��������ģʽΪͨ���������*/
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; /*������������Ϊ50MHz */   
	GPIO_Init( GPIOA, &GPIO_InitStructure );  /*���ÿ⺯������ʼ��GPIOA*/	 	
	GPIO_SetBits( GPIOA, GPIO_Pin_1 );  /* ����WiFiģ��ĸ�λ��������	*/
}


/*������������������������������������������������������������
ESP8266���������,�����յ���Ӧ��
str:�ڴ���Ӧ����
����ֵ��0,û�еõ��ڴ���Ӧ����
        ����,�ڴ�Ӧ������λ��(str��λ��)
ע��strstr(char* str, char* substr) ==>�����ַ���str�е�һ�γ����Ӵ�substr�ĵ�ַ
��������������������������������������������������������������*/
u8* esp8266_check_cmd(u8 *str)
{
	char* strx=NULL;

	if(USART2_RX_STA & 0X8000)  //���յ�һ��������
	{ 
		USART2_RX_BUF[USART2_RX_STA&0X7FFF] = 0;    //��ӽ�����
		strx=strstr((const char*)USART2_RX_BUF, (const char*)str);
	} 
	return (u8*)strx;  //���ؽ��յ�������
}

/*��������������������������������������������������������
��ATK-ESP8266��������
cmd:���͵������ַ���
ack:�ڴ���Ӧ����,���Ϊ��,���ʾ����Ҫ�ȴ�Ӧ��
waittime:�ȴ�ʱ��(��λ:10ms)
����ֵ:0,���ͳɹ�(�õ����ڴ���Ӧ����)
       1,����ʧ��
����������������������������������������������������������*/
u8 esp8266_send_cmd(u8 *cmd, u8 *ack, u16 waittime)
{
	u8 res = 0; 
	USART2_RX_STA = 0;
	u2_printf("%s\r\n", cmd); //��������
	if(ack && waittime)		  //��Ҫ�ȴ�Ӧ�𣬵�ack!=null����waittime!=0ʱִ��if�е���䣨�����
	{
		while(--waittime)	  //�ȴ�����ʱ
		{
			delay_ms(10);
			if(USART2_RX_STA & 0X8000)  //���յ��ڴ���Ӧ������bit15Ϊ�������ݱ�־λ��
			{
				if(esp8266_check_cmd(ack))
				{
					printf("ack:%s\r\n",(u8*)ack);  //��ӡ�����ؽ��
					break;//�õ���Ч���� 
				}
					USART2_RX_STA = 0;  //��ձ�־λ
			} 
		}
		if(waittime==0)
			res=1; 
	}
	return res;
} 

/*��������������������������
ATK-ESP8266�˳�͸��ģʽ
����ֵ:0,�˳��ɹ�;
       1,�˳�ʧ��
ע��SR:״̬�Ĵ�����bit6:TC(�������λ���������ֵ1)
	DR:���ݼĴ�����ֻ��bit0--bit8Ϊ��Чλ
�˳�͸��ָ�������������"+++"
��������������������������*/
u8 esp8266_quit_trans(void)
{
	while((USART2->SR & 0X40)==0);	//�ȴ����Ϳ�
	USART2->DR = '+';      
	delay_ms(15);					//���ڴ�����֡ʱ��(10ms)
	while((USART2->SR & 0X40)==0);	//�ȴ����Ϳ�
	USART2->DR = '+';      
	delay_ms(15);					//���ڴ�����֡ʱ��(10ms)
	while((USART2->SR & 0X40)==0);	//�ȴ����Ϳ�
	USART2->DR = '+';      
	delay_ms(500);					//�ȴ�500ms
	return esp8266_send_cmd((u8*)"AT",(u8*)"OK",20);//�˳�͸���ж�.
}


/*����������������������������������������
 * ��������ESP8266_JoinAP
 * ����  ��WF-ESP8266ģ�������ⲿWiFi
 * ����  ��pSSID��WiFi�����ַ���
 *       ��pPassWord��WiFi�����ַ���
 * ����  : 1�����ӳɹ�
 *         0������ʧ��
 * ����  �����ⲿ����
 ����������������������������������������*/
u8 ESP8266_JoinAP (char * pSSID, char* pPassWord)
{
	char* cCmd;

	sprintf(cCmd, "AT+CWJAP=\"%s\",\"%s\"", pSSID, pPassWord );
	return esp8266_send_cmd(cCmd, "OK", 1000);
}

/*��������������������������������������������
 * ��������ESP8266_Link_Server
 * ����  ��WF-ESP8266ģ�������ⲿ������
 * ����  ��enumE������Э��
 *       ��ip��������IP�ַ���
 *       ��ComNum���������˿��ַ���
 *       ��id��ģ�����ӷ�������ID
 * ����  : 1�����ӳɹ�
 *         0������ʧ��
 * ����  �����ⲿ����
����������������������������������������������*/
u8 ESP8266_Link_Server(char* ip, char* ComNum, ENUM_ID_NO_TypeDef id)
{
	char* cStr, *cCmd;

	sprintf ( cStr, "\"%s\",\"%s\",%s", "TCP", ip, ComNum );
	if ( id < 5 )
		sprintf (cCmd, "AT+CIPSTART=%d,%s", id, cStr);
	else
		sprintf (cCmd, "AT+CIPSTART=%s", cStr );  //����TCP����

	return esp8266_send_cmd(cCmd, "OK", 400);
}

/*��������������������������������������������
 * ��������ESP8266_UnvarnishSend
 * ����  ������WF-ESP8266ģ�����͸������
 * ����  ����
 * ����  : 1�����óɹ�
 *         0������ʧ��
 * ����  �����ⲿ����
 ��������������������������������������������*/
u8 ESP8266_UnvarnishSend ( void )
{
	if ( esp8266_send_cmd("AT+CIPMODE=1", "OK", 50) )
		return 0;
	return 
	  esp8266_send_cmd("AT+CIPSEND", "OK", 50);
}

/*��������������������������������������������������������
 * ��������ESP8266_SendString
 * ����  ��WF-ESP8266ģ�鷢���ַ���
 * ����  ��enumEnUnvarnishTx�������Ƿ���ʹ����͸��ģʽ
 *       ��pStr��Ҫ���͵��ַ���
 *       ��ulStrLength��Ҫ���͵��ַ������ֽ���
 *       ��ucId���ĸ�ID���͵��ַ���
 * ����  : 1�����ͳɹ�
 *         0������ʧ��
 * ����  �����ⲿ����
 ��������������������������������������������������������*/
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


/*��������������������������������������������������������������������
���ܣ����Դ�������STAģʽ��ģ����Ϊ����WIFI STA���ӵ��������磬����TCP�ͻ���
����ֵ��0������
		�������������
ָ����ܣ�AT:����AT����
		  AT+CIPMODE=0�˳�͸��   AT+CIPMODE=2���ó�·��ģʽ
		  ATE0:�رջ���
��������������������������������������������������������������������*/
void esp8266_tcp_client()
{
	u8* p;
	char* cStr;
	u8 res;
	sprintf( cStr, "ABCDEFGHIJKLMNOPQRSTUVWXYZ\r\n" );
	
	printf("\r\nESP8266 WIFIģ�����STAģʽ������TCP�ͻ���\r\n");
	while( !esp8266_send_cmd((u8*)"AT", (u8*)"OK", 10))  //���WIFI�Ƿ����ߣ����������᷵��"OK"
	{
		esp8266_quit_trans();  //�˳�͸��
		esp8266_send_cmd((u8*)"AT+CIPMODE=0", (u8*)"OK", 200); //�ر�͸��ģʽ
		printf("\r\nδ��⵽WIFIģ�飡\r\n");
		delay_ms(800);
		printf("\r\n�������ӵ�ģ�飡\r\n");
	}
	esp8266_send_cmd((u8*)"AT+GMR", (u8*)"OK", 20);
	p = esp8266_check_cmd((u8*)"SDK version:");
	printf("�̼��汾��%s\r\n", p);
	
//	while(esp8266_send_cmd((u8*)"ATE0", (u8*)"OK", 20));   //�رջ���
	esp8266_send_cmd((u8*)"AT+CWMODE=1",(u8*)"OK", 200);   //���ó�STAģʽ
	while ( !ESP8266_JoinAP(wifista_ssid, wifista_password) );  //����WIFI�ȵ�
	res = esp8266_send_cmd("AT+CIPMUX=0", "OK", 50);  //������ģʽ
	printf("res = %d\r\n", res);
	while ( !ESP8266_Link_Server(User_ESP8266_TcpServer_IP, User_ESP8266_TcpServer_Port, Single_ID_0));
	while( !ESP8266_UnvarnishSend() );
	printf ( "\r\n���� ESP8266 ���\r\n" );
	while(1)
	{
		u2_printf("%s", cStr);
		delay_ms(1000);
	}
	
/*
	esp8266_send_cmd((u8*)"AT+RST",(u8*)"OK",20);  	       //����ģ��
	delay_ms(1000);
	delay_ms(1000);
	delay_ms(1000);
	delay_ms(1000);
	sprintf((char*)p, "AT+CWJAP=\"%s\",\"%s\"", wifista_ssid, wifista_password);
	esp8266_send_cmd(p, (u8*)"OK", 1000);  //����wifi�ȵ�
	esp8266_send_cmd("AT+CIPMUX=0", (u8*)"OK", 20);  //���������� 
	esp8266_send_cmd("AT+CIPSTART=\"TCP\",\"192.168.0.101\",8086", (u8*)"OK", 1000);
	esp8266_send_cmd("AT+CIPMODE=1", "OK", 1000);
	esp8266_send_cmd("AT+CIPSEND=2,6", "OK", 1000);
	esp8266_send_cmd("123456", "OK", 1000);
*/
}

void esp8266_init()
{
	u8* p;
	
	esp8266_GPIO_Config();     //��������
	esp8266_RST_HIGH_LEVEL();  //��λ��������Ϊ�ߵ�ƽ
	delay_ms(1000);
	esp8266_send_cmd((u8*)"AT+GMR",(u8*)"OK",20);
	p = esp8266_check_cmd((u8*)"SDK version:");
	printf("�̼��汾��%s\r\n", p);
}
/*
	esp8266_send_cmd((u8*)"AT+CWMODE?",(u8*)"+CWMODE:",20);	//��ȡ����ģʽ
	p = esp8266_check_cmd((u8*)":");
	printf("����ģʽ%s\r\n", p);
	esp8266_check_cmd("AT+CWSAP?","+CWSAP:",20);	//��ȡwifi����
*/
















