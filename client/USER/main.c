#include "led.h"
#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "key.h"
#include "usart2.h"
#include "esp8266.h"
#include "ds18b20.h"
#include "dht11.h"
#include "pms.h"
#include "mq-7.h"



int main(void)
{

	uint16_t ADC_ConvertedValue;
	float CO_mg_m3 = 0.0;	//用于临时存放CO浓度值
	float tmp = 0.0;  		//用于临时存取温度值
	DHT11_Data_TypeDef  DHT11_Data;   //用于存取湿度值
	pms_data_typedef    PMS5003_Data;
	
	LED_GPIO_Config( );
	delay_init( );
	NVIC_PriorityGroupConfig( NVIC_PriorityGroup_2 );
	uart_init( 115200 );
	Adc_Init( );
	PMS5003_Init( );
	if( DS18B20_Init( ) )
		printf("\r\nds18b20初始化失败！\r\n");
//	ESP8266_Init( );
	printf( "\r\n开始测试数据\r\n" );
//	ESP8266_StaTcpClient_Test( );
	
	
	while(1)
	{	
		tmp = DS18B20_Get_temperature( );
		printf("temperature: %0.2f\n", tmp);
		
		ADC_ConvertedValue = Get_Adc_Average( ADC_Channel_11, 10 );	
		if(ADC_ConvertedValue >= 4095)		
			ADC_ConvertedValue = 4095;		
		CO_mg_m3 = (ADC_ConvertedValue-480.0)*((1145.6)/(4095.0-480.0));
		if ( CO_mg_m3 > 1145.6 )
			CO_mg_m3 = 1145.6;
		printf("CO: %0.2f\n", CO_mg_m3);
		
		
		
		Read_PMS5003( &PMS5003_Data );
		
		printf("标准环境下：PM1dot0CF1: %d, PM2dot5CF1: %d, PM10dot0CF1: %d\n", PMS5003_Data.PM1dot0CF1, PMS5003_Data.PM2dot5CF1, PMS5003_Data.PM10dot0CF1);
		
		printf("大气环境下：PM1dot0: %d, PM2dot5: %d, PM10dot0: %d\n", PMS5003_Data.PM1dot0, PMS5003_Data.PM2dot5, PMS5003_Data.PM10dot0);
		
		printf("颗粒物个数Particles0dot3: %d, Particles0dot5：%d Particles1dot0：%d \nParticles2dot5：%d Particles5dot0:%d Particles10:%d\n", \
		PMS5003_Data.Particles0dot3, PMS5003_Data.Particles0dot5, PMS5003_Data.Particles1dot0, \
		PMS5003_Data.Particles2dot5, PMS5003_Data.Particles5dot0, PMS5003_Data.Particles10);
		
		
		if (Read_DHT11(&DHT11_Data))
		{
			printf("shidu: %d.%d\n", DHT11_Data.humi_int, DHT11_Data.humi_deci);
		}
		delay_ms(1000);
	}
}







