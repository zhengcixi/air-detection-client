#include "esp8266.h"
#include "delay.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include "ds18b20.h"
#include "dht11.h"
#include "pms.h"
#include "mq-7.h"

volatile uint8_t ucTcpClosedFlag = 0;
struct   STRUCT_USARTx_Fram strEsp8266_Fram_Record = {0};

static char* itoa (int value, char * string, int radix );	
/*��������������������������������������������������������������������
 * ��������USART2_printf
 * ����  ����ʽ�������������C���е�printf��������û���õ�C��
 * ����  ��-USARTx ����ͨ��������ֻ�õ��˴���2����USART2
 *		   -Data   Ҫ���͵����ڵ����ݵ�ָ��
 *		   -...    ��������
 * ���  ����
 * ����  ���� 
 * ����  ���ⲿ����
 *         ����Ӧ��USART2_printf( USART2, "\r\n this is a demo \r\n" );
 *            	   USART2_printf( USART2, "\r\n %d \r\n", i );
 *            	   USART2_printf( USART2, "\r\n %s \r\n", s );
 ��������������������������������������������������������������������*/
void USART_printf(USART_TypeDef * USARTx, char * Data, ...)
{
	const char *s;
	int d;   
	char buf[16];

	va_list ap;
	va_start(ap, Data);
	while ( *Data != 0 )     // �ж��Ƿ񵽴��ַ���������
	{				                          
		if ( *Data == 0x5c )  //'\'
		{									  
			switch ( *++Data )
			{
				case 'r':	//�س���
				USART_SendData(USARTx, 0x0d);
				Data ++;
				break;

				case 'n':	//���з�
				USART_SendData(USARTx, 0x0a);	
				Data ++;
				break;

				default:
				Data ++;
				break;
			}			 
		}
		else if ( *Data == '%')
		{									  
			switch ( *++Data )
			{				
				case 's':  //�ַ���
					s = va_arg(ap, const char *);
					for ( ; *s; s++) 
					{
						USART_SendData(USARTx,*s);
						while( USART_GetFlagStatus(USARTx, USART_FLAG_TXE) == RESET );
					}
					Data++;
					break;
				case 'd':  //ʮ����	 	
					d = va_arg(ap, int);
					itoa(d, buf, 10);
					for (s = buf; *s; s++) 
					{
						USART_SendData(USARTx,*s);
						while( USART_GetFlagStatus(USARTx, USART_FLAG_TXE) == RESET );
					}
					Data++;
					break;
				default:
					Data++;
					break;
			}		 
		}
		else
			USART_SendData(USARTx, *Data++);
		while ( USART_GetFlagStatus( USARTx, USART_FLAG_TXE ) == RESET );
	}
}

/*������������������������������������������������
 * ��������itoa
 * ����  ������������ת�����ַ���
 * ����  ��-radix =10 ��ʾ10���ƣ��������Ϊ0
 *         -value Ҫת����������
 *         -buf ת������ַ���
 *         -radix = 10
 * ���  ����
 * ����  ����
 * ����  ����USART2_printf()����
 ������������������������������������������������*/
static char * itoa( int value, char *string, int radix )
{
	int i, d;
	int flag = 0;
	char *ptr = string;

	/* This implementation only works for decimal numbers. */
	if (radix != 10)
	{
		*ptr = 0;
		return string;
	}
	if (!value)
	{
		*ptr++ = 0x30;
		*ptr = 0;
		return string;
	}
	/* if this is a negative value insert the minus sign. */
	if (value < 0)
	{
		*ptr++ = '-';
		/* Make the value positive. */
		value *= -1;
	}
	for (i = 10000; i > 0; i /= 10)
	{
		d = value / i;
		if (d || flag)
		{
			*ptr++ = (char)(d + 0x30);
			value -= (d * i);
			flag = 1;
		}
	}
	/* Null terminate the string. */
	*ptr = 0;
	return string;
} /* NCL_Itoa */


static void ESP8266_GPIO_Config(void)
{
	/*����һ��GPIO_InitTypeDef���͵Ľṹ��*/
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOB, ENABLE ); 								   
	GPIO_InitStructure.GPIO_Pin = ESP8266_RST_PIN;	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;   
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
	GPIO_Init(ESP8266_RST_PORT, &GPIO_InitStructure);	 
}	

static void USART_NVIC_Configuration( void )
{
	NVIC_InitTypeDef NVIC_InitStructure; 
	
	/* Configure the NVIC Preemption Priority Bits */  
	NVIC_PriorityGroupConfig ( NVIC_PriorityGroup_2);

	/* Enable the USART2 Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;	 
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

}

static void ESP8266_USART_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	
	
	/* config USART clock */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE );
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,  ENABLE );
	
	/* USART GPIO config */
	/* Configure USART Tx as alternate function push-pull */
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);  
  
	/* Configure USART Rx as input floating */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	/* USART1 mode config */
	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART2, &USART_InitStructure);
	
	
	/* �ж����� */
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE ); //ʹ�ܴ��ڽ����ж� 
	USART_ITConfig(USART2, USART_IT_IDLE, ENABLE ); //ʹ�ܴ������߿����ж� 	

	USART_NVIC_Configuration();
	
	USART_Cmd(USART2, ENABLE);
}

void USART2_IRQHandler(void)
{
	uint8_t ucCh;
	
	if (USART_GetITStatus(USART2, USART_IT_RXNE ) != RESET ) //�����ж�
	{
		ucCh  = USART_ReceiveData(USART2);
		if (strEsp8266_Fram_Record.InfBit.FramLength < ( RX_BUF_MAX_LEN - 1 ) )  //Ԥ��1���ֽ�д������
			strEsp8266_Fram_Record.Data_RX_BUF[strEsp8266_Fram_Record.InfBit.FramLength++] = ucCh;
	}	 
	if ( USART_GetITStatus(USART2, USART_IT_IDLE)==SET )  //���߿����жϣ�����֡�������
	{
		strEsp8266_Fram_Record .InfBit .FramFinishFlag = 1;
		ucCh = USART_ReceiveData(USART2);   //�������������жϱ�־λ(�ȶ�USART_SR��Ȼ���USART_DR)
		ucTcpClosedFlag = strstr(strEsp8266_Fram_Record.Data_RX_BUF, "CLOSED\r\n" ) ? 1 : 0;
	}	
}



void ESP8266_Init(void)
{
	ESP8266_GPIO_Config();
	ESP8266_USART_Config();
	ESP8266_RST_HIGH_LEVEL();
}
 
/*������������������������������������
 * ��������ESP8266_Rst
 * ����  ������WF-ESP8266ģ��
 * ����  ����
 * ����  : ��
 * ����  ���� ESP8266_AT_Test ����
 ������������������������������������*/
void ESP8266_Rst ( void )
{
	#if 0
		ESP8266_Cmd ("AT+RST", "OK", "ready", 2500 );   	
	#else
		ESP8266_RST_LOW_LEVEL();
		delay_ms( 500 ); 
		ESP8266_RST_HIGH_LEVEL();
	#endif
}

/*������������������������������������������������������������
 * ��������ESP8266_Cmd
 * ����  ����WF-ESP8266ģ�鷢��ATָ��
 * ����  ��cmd�������͵�ָ��
 *         reply1��reply2���ڴ�����Ӧ��ΪNULL������Ӧ������Ϊ���߼���ϵ
 *         waittime���ȴ���Ӧ��ʱ��
 * ����  : 1��ָ��ͳɹ�
 *         0��ָ���ʧ��
 * ����  �����ⲿ����
 ������������������������������������������������������������*/
bool ESP8266_Cmd( char* cmd, char* reply1, char* reply2, u32 waittime )
{    
	strEsp8266_Fram_Record .InfBit .FramLength = 0; //���¿�ʼ�����µ����ݰ�
	ESP8266_Usart("%s\r\n", cmd );
	if ( ( reply1 == 0 ) && ( reply2 == 0 ) )  //����Ҫ��������
		return true;
	delay_ms(waittime);  //��ʱ
	strEsp8266_Fram_Record.Data_RX_BUF[strEsp8266_Fram_Record .InfBit .FramLength] = '\0';
	printf("%s", strEsp8266_Fram_Record .Data_RX_BUF ); 
	if ( ( reply1 != 0 ) && ( reply2 != 0 ) )
		return ( ( bool ) strstr ( strEsp8266_Fram_Record .Data_RX_BUF, reply1 ) || 
				( bool ) strstr ( strEsp8266_Fram_Record .Data_RX_BUF, reply2 ) ); 
	else if ( reply1 != 0 )
		return ( ( bool ) strstr ( strEsp8266_Fram_Record .Data_RX_BUF, reply1 ) );
	else
		return ( ( bool ) strstr ( strEsp8266_Fram_Record .Data_RX_BUF, reply2 ) );	
}


/*������������������������������������������
 * ��������ESP8266_AT_Test
 * ����  ����WF-ESP8266ģ�����AT��������
 * ����  ����
 * ����  : ��
 * ����  �����ⲿ����
 ������������������������������������������*/
void ESP8266_AT_Test(void)
{
	char count = 0;
	
	ESP8266_RST_HIGH_LEVEL();	
	delay_ms(1000);
	while ( count < 10 )
	{
		if(ESP8266_Cmd ( "AT", "OK", NULL, 500 ) ) return;
		ESP8266_Rst();
		++count;
	}
}

/*����������������������������������������
 * ��������ESP8266_Net_Mode_Choose
 * ����  ��ѡ��WF-ESP8266ģ��Ĺ���ģʽ
 * ����  ��enumMode������ģʽ
 * ����  : 1��ѡ��ɹ�
 *         0��ѡ��ʧ��
 * ����  �����ⲿ����
 ����������������������������������������*/
bool ESP8266_Net_Mode_Choose(ENUM_Net_ModeTypeDef enumMode )
{
	switch ( enumMode )
	{
		case STA:
			return ESP8266_Cmd( "AT+CWMODE=1", "OK", "no change", 2500 ); 
		case AP:
			return ESP8266_Cmd( "AT+CWMODE=2", "OK", "no change", 2500 ); 
		case STA_AP:
			return ESP8266_Cmd( "AT+CWMODE=3", "OK", "no change", 2500 ); 
		default:
			return false;
  }
}

/*��������������������������������������
 * ��������ESP8266_JoinAP
 * ����  ��WF-ESP8266ģ�������ⲿWiFi
 * ����  ��pSSID��WiFi�����ַ���
 *       ��pPassWord��WiFi�����ַ���
 * ����  : 1�����ӳɹ�
 *         0������ʧ��
 * ����  �����ⲿ����
 ��������������������������������������*/
bool ESP8266_JoinAP( char * pSSID, char * pPassWord )
{
	char cCmd [120];

	sprintf( cCmd, "AT+CWJAP=\"%s\",\"%s\"", pSSID, pPassWord );
	return ESP8266_Cmd( cCmd, "OK", NULL, 5000 );
}

/*��������������������������������������������
 * ��������ESP8266_Enable_MultipleId
 * ����  ��WF-ESP8266ģ������������
 * ����  ��enumEnUnvarnishTx�������Ƿ������
 * ����  : 1�����óɹ�
 *         0������ʧ��
 * ����  �����ⲿ����
 ��������������������������������������������*/
bool ESP8266_Enable_MultipleId( FunctionalState enumEnUnvarnishTx )
{
	char cStr [20];
	
	sprintf( cStr, "AT+CIPMUX=%d", ( enumEnUnvarnishTx ? 1 : 0 ) );
	return ESP8266_Cmd( cStr, "OK", 0, 500 );
	
}

/*����������������������������������������
 * ��������ESP8266_Link_Server
 * ����  ��WF-ESP8266ģ�������ⲿ������
 * ����  ��enumE������Э��
 *       ��ip��������IP�ַ���
 *       ��ComNum���������˿��ַ���
 *       ��id��ģ�����ӷ�������ID
 * ����  : 1�����ӳɹ�
 *         0������ʧ��
 * ����  �����ⲿ����
 ����������������������������������������*/
bool ESP8266_Link_Server( ENUM_NetPro_TypeDef enumE, char * ip, char * ComNum, ENUM_ID_NO_TypeDef id)
{
	char cStr [100] = { 0 }, cCmd [120];

	switch( enumE )
	{
		case enumTCP:
		  sprintf( cStr, "\"%s\",\"%s\",%s", "TCP", ip, ComNum ); break;
		case enumUDP:
		  sprintf( cStr, "\"%s\",\"%s\",%s", "UDP", ip, ComNum ); break;
		default:
			break;
	}
	if ( id < 5 )
		sprintf ( cCmd, "AT+CIPSTART=%d,%s", id, cStr);
	else
	  sprintf( cCmd, "AT+CIPSTART=%s", cStr );

	return ESP8266_Cmd( cCmd, "OK", "ALREAY CONNECT", 4000 );	
}

/*����������������������������������������
 * ��������ESP8266_UnvarnishSend
 * ����  ������WF-ESP8266ģ�����͸������
 * ����  ����
 * ����  : 1�����óɹ�
 *         0������ʧ��
 * ����  �����ⲿ����
 ����������������������������������������*/
bool ESP8266_UnvarnishSend(void)
{
	if ( ! ESP8266_Cmd ( "AT+CIPMODE=1", "OK", 0, 500 ) )
		return false;
	return 
	  ESP8266_Cmd ( "AT+CIPSEND", "OK", ">", 500 );
}

/*������������������������������������������������������
 * ��������ESP8266_SendString
 * ����  ��WF-ESP8266ģ�鷢���ַ���
 * ����  ��enumEnUnvarnishTx�������Ƿ���ʹ����͸��ģʽ
 *       ��pStr��Ҫ���͵��ַ���
 *       ��ulStrLength��Ҫ���͵��ַ������ֽ���
 *       ��ucId���ĸ�ID���͵��ַ���
 * ����  : 1�����ͳɹ�
 *         0������ʧ��
 * ����  �����ⲿ����
 ������������������������������������������������������*/
bool ESP8266_SendString( FunctionalState enumEnUnvarnishTx, char * pStr, u32 ulStrLength, ENUM_ID_NO_TypeDef ucId )
{
	char cStr [20];
	bool bRet = false;
		
	if ( enumEnUnvarnishTx )
	{
		ESP8266_Usart( "%s", pStr );
		bRet = true;
	}
	else
	{
		if ( ucId < 5 )
			sprintf( cStr, "AT+CIPSEND=%d,%d", ucId, ulStrLength + 2 );
		else
			sprintf( cStr, "AT+CIPSEND=%d", ulStrLength + 2 );
		ESP8266_Cmd( cStr, "> ", 0, 1000 );
		bRet = ESP8266_Cmd( pStr, "SEND OK", 0, 1000 );
  }
	return bRet;
}

/*������������������������������������������
 * ��������ESP8266_ExitUnvarnishSend
 * ����  ������WF-ESP8266ģ���˳�͸��ģʽ
 * ����  ����
 * ����  : ��
 * ����  �����ⲿ����
 ������������������������������������������*/
void ESP8266_ExitUnvarnishSend ( void )
{
	delay_ms ( 1000 );
	ESP8266_Usart("+++");
	delay_ms ( 500 ); 
}

/*����������������������������������������������������
 * ��������ESP8266_Get_LinkStatus
 * ����  ����ȡ WF-ESP8266 ������״̬�����ʺϵ��˿�ʱʹ��
 * ����  ����
 * ����  : 2�����ip
 *         3����������
 *         3��ʧȥ����
 *         0����ȡ״̬ʧ��
 * ����  �����ⲿ����
 ����������������������������������������������������*/
uint8_t ESP8266_Get_LinkStatus ( void )
{
	if ( ESP8266_Cmd("AT+CIPSTATUS", "OK", 0, 500) )
	{
		if ( strstr(strEsp8266_Fram_Record .Data_RX_BUF, "STATUS:2\r\n") )
			return 2; 
		else if ( strstr(strEsp8266_Fram_Record .Data_RX_BUF, "STATUS:3\r\n") )
			return 3;
		else if ( strstr(strEsp8266_Fram_Record .Data_RX_BUF, "STATUS:4\r\n") )
			return 4; 
	}
	return 0;
}


/*
WIFIģ���STAģʽ����ΪTCP Cilent����TCP Server������������
*/
void ESP8266_StaTcpClient_Test(void)
{
	uint8_t  ucStatus;
	uint16_t ADC_ConvertedValue;
	float CO_mg_m3 = 0.0;	//������ʱ���COŨ��ֵ
	float tmp = 0.0;  		//������ʱ��ȡ�¶�ֵ
	char  cStr[400] = {0};
	DHT11_Data_TypeDef  DHT11_Data;   //���ڴ�ȡʪ��ֵ
	pms_data_typedef    PMS5003_Data;
	
	printf ("\r\n�������� ESP8266 ......\r\n" );
	ESP8266_AT_Test();
	ESP8266_Net_Mode_Choose(STA);
    while( !ESP8266_JoinAP( macUser_ESP8266_ApSsid, macUser_ESP8266_ApPwd ) );	
	ESP8266_Enable_MultipleId( DISABLE );
	while ( !ESP8266_Link_Server( enumTCP, macUser_ESP8266_TcpServer_IP, macUser_ESP8266_TcpServer_Port, Single_ID_0 ) );
	while ( !ESP8266_UnvarnishSend() );
	printf( "\r\n���� ESP8266 ���\r\n" );
	while(1)
	{	
		tmp = DS18B20_Get_temperature( );
		Read_PMS5003( &PMS5003_Data );
		ADC_ConvertedValue = Get_Adc_Average( ADC_Channel_11, 10 );	
		if(ADC_ConvertedValue >= 4095)		
			ADC_ConvertedValue = 4095;		
		CO_mg_m3 = (ADC_ConvertedValue-480.0)*((1145.6)/(4095.0-480.0));
		if ( CO_mg_m3 > 1145.6 )
			CO_mg_m3 = 1145.6;

		if (Read_DHT11(&DHT11_Data))
		{
			sprintf(cStr, "%0.2f %0.2f %d.%d %d %d %d %d %d %d %d %d %d %d %d %d ", tmp, CO_mg_m3, \
			DHT11_Data.humi_int, DHT11_Data.humi_deci, \
			PMS5003_Data.PM1dot0CF1, PMS5003_Data.PM2dot5CF1, PMS5003_Data.PM10dot0CF1,\
			PMS5003_Data.PM1dot0, PMS5003_Data.PM2dot5, PMS5003_Data.PM10dot0,\
			PMS5003_Data.Particles0dot3, PMS5003_Data.Particles0dot5, PMS5003_Data.Particles1dot0, \
			PMS5003_Data.Particles2dot5, PMS5003_Data.Particles5dot0, PMS5003_Data.Particles10);
		}
		
		ESP8266_SendString(ENABLE, cStr, 0, Single_ID_0); //�����¶���Ϣ�������������
		delay_ms(1000);
		
		if ( ucTcpClosedFlag ) //����Ƿ�ʧȥ����
		{
			ESP8266_ExitUnvarnishSend();  //�˳�͸��ģʽ
			do ucStatus = ESP8266_Get_LinkStatus(); //��ȡ����״̬
			while ( !ucStatus );
			if ( ucStatus == 4 )  //ȷ��ʧȥ���Ӻ�����
			{
				printf("\r\n���������ȵ�ͷ����� ......\r\n");
				while ( !ESP8266_JoinAP(macUser_ESP8266_ApSsid, macUser_ESP8266_ApPwd) );
				while ( !ESP8266_Link_Server(enumTCP, macUser_ESP8266_TcpServer_IP, macUser_ESP8266_TcpServer_Port, Single_ID_0) );
				printf ( "\r\n�����ȵ�ͷ������ɹ�\r\n" );
			}
			while ( !ESP8266_UnvarnishSend() );		
		}
	}
}




