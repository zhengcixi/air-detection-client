#include "ds18b20.h"
#include "delay.h"

static void ds18b20_GPIO_COFIG(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB , ENABLE);

	GPIO_InitStructure.GPIO_Pin = DS18B20_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(DS18B20_PORT, &GPIO_InitStructure);

	GPIO_SetBits(DS18B20_PORT, DS18B20_PIN);
}


static void DS18B20_Mode_IPU(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_InitStructure.GPIO_Pin = DS18B20_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;	
	GPIO_Init(DS18B20_PORT, &GPIO_InitStructure);
}


static void DS18B20_Mode_Out_PP(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
													   
	GPIO_InitStructure.GPIO_Pin = DS18B20_PIN;	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;   
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		
	GPIO_Init(DS18B20_PORT, &GPIO_InitStructure);
}

static void ds18b20_Reset()
{
	DS18B20_Mode_Out_PP();	
	DS18B20_DQ_OUT=0;		
	delay_us(650);
	DS18B20_DQ_OUT=1;		
	delay_us(15);
}


static uint8_t ds18b20_Presence(void)
{
	uint8_t pulse_time = 0;
	
	DS18B20_Mode_IPU();		
	
	while(ReadTmpValue() && pulse_time<100) 
	{
		pulse_time++;
		delay_us(1);
	}
	if(pulse_time >= 100)	
		return 1;
	else
		pulse_time=0;
	
	while(!ReadTmpValue() && pulse_time<240)	
	{
		pulse_time++;
		delay_us(1);
	}
	if(pulse_time >= 240)	
		return 1;
	else
		return 0;
}

uint8_t DS18B20_Init(void)
{
	ds18b20_GPIO_COFIG();
	ds18b20_Reset();
	
	return ds18b20_Presence();
}

static uint8_t DS18B20_readBit(void)
{
	uint8_t data;
	
	DS18B20_Mode_Out_PP();
	DS18B20_DQ_OUT=0;		
	delay_us(10);
	DS18B20_DQ_OUT=1;	
	DS18B20_Mode_IPU();
	if( ReadTmpValue() )
		data = 1;
	else
		data = 0;
	
	delay_us(50);
	return data;
}

uint8_t DS18B20_readByte(void)
{
	uint8_t i, j, data=0;
	
	for(i=0; i<8; i++)
	{
		j = DS18B20_readBit();
		data = (data) | (j<<i);
	}
	
	return data;
}


void DS18B20_writeByte(uint8_t data)
{
	uint8_t i, testb;
	
	DS18B20_Mode_Out_PP();
	
	for(i=1; i<=8; i++)
	{
		testb=data&0x01;	
        data=data>>1;
        if (testb) 
        {
            DS18B20_DQ_OUT=0;	
            delay_us(2);                            
            DS18B20_DQ_OUT=1;
            delay_us(60);             
        }
        else 
        {
            DS18B20_DQ_OUT=0;	
            delay_us(60);             
            DS18B20_DQ_OUT=1;
            delay_us(2);                          
        }
	}
}

void DS18B20_Start(void) 
{   						               
    ds18b20_Reset();	   
	ds18b20_Presence();	 
    DS18B20_writeByte(SKIPROM);		// skip rom
    DS18B20_writeByte(CONVERT_T);	// convert
} 


float DS18B20_Get_temperature(void)
{
	uint8_t tpmsb, tplsb;
	short s_tem;
	float f_tem;
	
	ds18b20_Reset();	   
	ds18b20_Presence();	 
	DS18B20_writeByte(SKIPROM);				
	DS18B20_writeByte(CONVERT_T);			

	ds18b20_Reset();
    ds18b20_Presence();
	DS18B20_writeByte(SKIPROM);				
    DS18B20_writeByte(READ_SCRATCHPAD);
	
	tplsb = DS18B20_readByte();		 
	tpmsb = DS18B20_readByte(); 
	
	s_tem = tpmsb<<8;
	s_tem = s_tem | tplsb;
	
	if( s_tem < 0 )		
		f_tem = (~s_tem+1) * 0.0625;	
	else
		f_tem = s_tem * 0.0625;
	
	return f_tem; 	
}


