#include "dht11.h"
#include "delay.h"

void dht11_GPIO_COFIG(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = DHT11_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  //推挽输出
	GPIO_Init(DHT11_PORT, &GPIO_InitStructure);

	GPIO_SetBits(DHT11_PORT, DHT11_PIN);	//拉高GPIOB10,总线空闲状为高电平
}

 /* 描述 ：使DHT11-DATA引脚变为上拉输入模式 */
static void DHT11_Mode_IPU(void)
{
 	  GPIO_InitTypeDef GPIO_InitStructure;

	  GPIO_InitStructure.GPIO_Pin = DHT11_PIN;
	  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU ; 
	
	  GPIO_Init(DHT11_PORT, &GPIO_InitStructure);	 
}


 /* 描述 ：使DHT11-DATA引脚变为推挽输出模式 */
static void DHT11_Mode_Out_PP(void)
{
 	GPIO_InitTypeDef GPIO_InitStructure;
															   
  	GPIO_InitStructure.GPIO_Pin = DHT11_PIN;	
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;   
	
  	GPIO_Init(DHT11_PORT, &GPIO_InitStructure);	 	 
}

//主机发送开始信号
void DHT11_Start(void)	
{
	DHT11_LOW();   //拉低引脚
	delay_ms(25);
	DHT11_HIGH();  //拉高引脚，等待DHT11响应
	delay_us(40);
}

/*—————————————————————————
数据按位输出，传递数据的起始信号：12-14us的低电平
传输'0'：保持26-28us的高电平
传输'1'：保持70us的高电平
—————————————————————————*/

//读取一个字节的数据
uint8_t Read_ByteData(void)
{
	uint8_t data, i;
	
	for(i=0; i<8; i++)
	{
		while(ReadHumValue() == Bit_RESET);	//检测低电平
		
		delay_us(60);
		if (ReadHumValue() == Bit_SET)		//数据为'1'
		{
			while(ReadHumValue()==Bit_SET);	//等待数据1的高电平结束,这行很重要,检测一位数据传输完毕
			data |= (uint8_t)(0x01<<(7-i));
		}
		else  //数据为'0'
			data &= (uint8_t)~(0x01<<(7-i));	
	}
	
	return data;
}

uint8_t Read_DHT11(DHT11_Data_TypeDef *DHT11_Data)
{
	dht11_GPIO_COFIG();  //初始化引脚
	DHT11_Mode_Out_PP(); //输出模式  
	DHT11_Start();   	 //主机产生复位信号
	DHT11_Mode_IPU();	 //改变pin口为上拉输入模式，检测电平变化
	
	while(ReadHumValue() == Bit_RESET);	//等待DHT11响应输出
	delay_us(50);
	while(ReadHumValue() == Bit_SET);
	delay_us(50);

/*————————数据包格式————————————————
数据格式：8bit湿度整数数据 + 8bit湿度小数数据
		  + 8bit温度整数数据 + 8bit温度小数数据
		  + 8bit校验和
校验和数据为前四个字节相加，输出的数据是未编码的二进制数据
——————————————————————————————*/	

	
	/*开始接受数据*/
	DHT11_Data->humi_int  = Read_ByteData();	//湿度整数
	DHT11_Data->humi_deci = Read_ByteData();	//湿度小数
	DHT11_Data->temp_int  = Read_ByteData();	//温度整数
	DHT11_Data->temp_deci = Read_ByteData();	//温度小数
	DHT11_Data->check_sum = Read_ByteData();	//校验和
	
	/*读取结束，引脚改为输出模式*/
	DHT11_Mode_Out_PP();
	/*主机拉高*/
	DHT11_HIGH();

	/*检查读取的数据是否正确*/
	if(DHT11_Data->check_sum == DHT11_Data->humi_int + DHT11_Data->humi_deci + DHT11_Data->temp_int+ DHT11_Data->temp_deci)
		return SUCCESS;
	else 
		return ERROR;
}

