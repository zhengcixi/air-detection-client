#include "dht11.h"
#include "delay.h"

void dht11_GPIO_COFIG(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = DHT11_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  //�������
	GPIO_Init(DHT11_PORT, &GPIO_InitStructure);

	GPIO_SetBits(DHT11_PORT, DHT11_PIN);	//����GPIOB10,���߿���״Ϊ�ߵ�ƽ
}

 /* ���� ��ʹDHT11-DATA���ű�Ϊ��������ģʽ */
static void DHT11_Mode_IPU(void)
{
 	  GPIO_InitTypeDef GPIO_InitStructure;

	  GPIO_InitStructure.GPIO_Pin = DHT11_PIN;
	  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU ; 
	
	  GPIO_Init(DHT11_PORT, &GPIO_InitStructure);	 
}


 /* ���� ��ʹDHT11-DATA���ű�Ϊ�������ģʽ */
static void DHT11_Mode_Out_PP(void)
{
 	GPIO_InitTypeDef GPIO_InitStructure;
															   
  	GPIO_InitStructure.GPIO_Pin = DHT11_PIN;	
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;   
	
  	GPIO_Init(DHT11_PORT, &GPIO_InitStructure);	 	 
}

//�������Ϳ�ʼ�ź�
void DHT11_Start(void)	
{
	DHT11_LOW();   //��������
	delay_ms(25);
	DHT11_HIGH();  //�������ţ��ȴ�DHT11��Ӧ
	delay_us(40);
}

/*��������������������������������������������������
���ݰ�λ������������ݵ���ʼ�źţ�12-14us�ĵ͵�ƽ
����'0'������26-28us�ĸߵ�ƽ
����'1'������70us�ĸߵ�ƽ
��������������������������������������������������*/

//��ȡһ���ֽڵ�����
uint8_t Read_ByteData(void)
{
	uint8_t data, i;
	
	for(i=0; i<8; i++)
	{
		while(ReadHumValue() == Bit_RESET);	//���͵�ƽ
		
		delay_us(60);
		if (ReadHumValue() == Bit_SET)		//����Ϊ'1'
		{
			while(ReadHumValue()==Bit_SET);	//�ȴ�����1�ĸߵ�ƽ����,���к���Ҫ,���һλ���ݴ������
			data |= (uint8_t)(0x01<<(7-i));
		}
		else  //����Ϊ'0'
			data &= (uint8_t)~(0x01<<(7-i));	
	}
	
	return data;
}

uint8_t Read_DHT11(DHT11_Data_TypeDef *DHT11_Data)
{
	dht11_GPIO_COFIG();  //��ʼ������
	DHT11_Mode_Out_PP(); //���ģʽ  
	DHT11_Start();   	 //����������λ�ź�
	DHT11_Mode_IPU();	 //�ı�pin��Ϊ��������ģʽ������ƽ�仯
	
	while(ReadHumValue() == Bit_RESET);	//�ȴ�DHT11��Ӧ���
	delay_us(50);
	while(ReadHumValue() == Bit_SET);
	delay_us(50);

/*�������������������ݰ���ʽ��������������������������������
���ݸ�ʽ��8bitʪ���������� + 8bitʪ��С������
		  + 8bit�¶��������� + 8bit�¶�С������
		  + 8bitУ���
У�������Ϊǰ�ĸ��ֽ���ӣ������������δ����Ķ���������
������������������������������������������������������������*/	

	
	/*��ʼ��������*/
	DHT11_Data->humi_int  = Read_ByteData();	//ʪ������
	DHT11_Data->humi_deci = Read_ByteData();	//ʪ��С��
	DHT11_Data->temp_int  = Read_ByteData();	//�¶�����
	DHT11_Data->temp_deci = Read_ByteData();	//�¶�С��
	DHT11_Data->check_sum = Read_ByteData();	//У���
	
	/*��ȡ���������Ÿ�Ϊ���ģʽ*/
	DHT11_Mode_Out_PP();
	/*��������*/
	DHT11_HIGH();

	/*����ȡ�������Ƿ���ȷ*/
	if(DHT11_Data->check_sum == DHT11_Data->humi_int + DHT11_Data->humi_deci + DHT11_Data->temp_int+ DHT11_Data->temp_deci)
		return SUCCESS;
	else 
		return ERROR;
}

