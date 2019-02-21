#include "pms.h"
#include "usart.h"

pms_data_typedef PMS5003_Data;

static void PMS5003_GPIO_Config(void)
{
	/*定义一个GPIO_InitTypeDef类型的结构体*/
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE ); 								   
	GPIO_InitStructure.GPIO_Pin = PMS5003_RST_PIN;	  	//PB9
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;   
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
	GPIO_Init(PMS5003_RST_PORT, &GPIO_InitStructure);	 
}

static void USART_NVIC_Configuration( void )
{
	NVIC_InitTypeDef NVIC_InitStructure; 
	
	/* Configure the NVIC Preemption Priority Bits */  
	NVIC_PriorityGroupConfig (NVIC_PriorityGroup_2);

	/* Enable the USART3 Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;	 
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

static void PMS5003_USART_Config(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	
	/* config USART clock */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE );
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,  ENABLE );
	
	/* USART GPIO config */
	/* Configure USART Tx as alternate function push-pull */
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;    //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);  
  
	/* Configure USART Rx as input floating */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;  //浮空输入
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	/* USART2 mode config */
	USART_InitStructure.USART_BaudRate = 9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART3, &USART_InitStructure);
	
	USART_ClearITPendingBit(USART3, USART_IT_RXNE);
	/* 中断配置 */
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE ); //使能串口接收中断 	

	USART_NVIC_Configuration();
	
	USART_Cmd(USART3, ENABLE);
}

/*——————————————————————————
 * 根据协议格式解析数据，接收一帧数据完成后调用
 ——————————————————————————*/
void pms5003ReceiveDataAnl(uint8_t *PM_Sensor_RxBuffer)
{
    uint8_t i;
    uint32_t sum = 0;
	
    /* 校验和计算 */
    for (i=0; i<30; i++) {
        sum += PM_Sensor_RxBuffer[i];
    }
    /* 接收数据校验和判断 */
    if (sum != ((PM_Sensor_RxBuffer[30]<<8) | PM_Sensor_RxBuffer[31])){
        return;
    }
	
	/* 标准颗粒物 */
	PMS5003_Data.PM1dot0CF1   = (uint16_t)((PM_Sensor_RxBuffer[4]<<8) | PM_Sensor_RxBuffer[5]);
	PMS5003_Data.PM2dot5CF1   = (uint16_t)((PM_Sensor_RxBuffer[6]<<8) | PM_Sensor_RxBuffer[7]);
	PMS5003_Data.PM10dot0CF1  = (uint16_t)((PM_Sensor_RxBuffer[8]<<8) | PM_Sensor_RxBuffer[9]);
	
	/* 大气环境下 */
	PMS5003_Data.PM1dot0   = (uint16_t)((PM_Sensor_RxBuffer[10]<<8) | PM_Sensor_RxBuffer[11]);
	PMS5003_Data.PM2dot5   = (uint16_t)((PM_Sensor_RxBuffer[12]<<8) | PM_Sensor_RxBuffer[13]);
	PMS5003_Data.PM10dot0  = (uint16_t)((PM_Sensor_RxBuffer[14]<<8) | PM_Sensor_RxBuffer[15]);
	
	/* 0.1升空气中颗粒物的个数 */
	PMS5003_Data.Particles0dot3   = (uint16_t)((PM_Sensor_RxBuffer[16]<<8) | PM_Sensor_RxBuffer[17]);
	PMS5003_Data.Particles0dot5   = (uint16_t)((PM_Sensor_RxBuffer[18]<<8) | PM_Sensor_RxBuffer[19]);
	PMS5003_Data.Particles1dot0   = (uint16_t)((PM_Sensor_RxBuffer[20]<<8) | PM_Sensor_RxBuffer[21]);
	PMS5003_Data.Particles2dot5   = (uint16_t)((PM_Sensor_RxBuffer[22]<<8) | PM_Sensor_RxBuffer[23]);
	PMS5003_Data.Particles5dot0   = (uint16_t)((PM_Sensor_RxBuffer[24]<<8) | PM_Sensor_RxBuffer[25]);
	PMS5003_Data.Particles10      = (uint16_t)((PM_Sensor_RxBuffer[26]<<8) | PM_Sensor_RxBuffer[27]);
	PMS5003_Data.version = (uint8_t)(PM_Sensor_RxBuffer[28]);
	PMS5003_Data.error_num = (uint8_t)(PM_Sensor_RxBuffer[29]);
}


void Read_PMS5003(pms_data_typedef *PMS5003Data)
{
	PMS5003Data->PM1dot0CF1  = PMS5003_Data.PM1dot0CF1;
	PMS5003Data->PM2dot5CF1  = PMS5003_Data.PM2dot5CF1;
	PMS5003Data->PM10dot0CF1 = PMS5003_Data.PM10dot0CF1;
	
	PMS5003Data->PM1dot0  = PMS5003_Data.PM1dot0;
	PMS5003Data->PM2dot5  = PMS5003_Data.PM2dot5;
	PMS5003Data->PM10dot0 = PMS5003_Data.PM10dot0;
	
	PMS5003Data->Particles0dot3 = PMS5003_Data.Particles0dot3;
	PMS5003Data->Particles0dot5 = PMS5003_Data.Particles0dot5;
	PMS5003Data->Particles1dot0 = PMS5003_Data.Particles1dot0;
	PMS5003Data->Particles2dot5 = PMS5003_Data.Particles2dot5;
	PMS5003Data->Particles5dot0 = PMS5003_Data.Particles5dot0;
	PMS5003Data->Particles10    = PMS5003_Data.Particles10;

}


/*————————————————————————————
 * 根据协议格式接收数据，串口每接收一次数据调用一次
 ————————————————————————————*/
void pms5003ReceiveDataDepare(uint8_t data)
{
    static uint8_t state;
    static uint8_t data_len = 30, data_cnt;  // 数据帧长度 = 2*13 + 2 
    static uint8_t rxBuffer[33];

    if (state == 0 && data == 0x42)         // 起始符1判断
	{
        state = 1;
        rxBuffer[0] = data; 
    } else if (state == 1 && data == 0x4d)  // 起始符2判断
	{  
        state = 2;
        rxBuffer[1] = data;
    } else if (state == 2 && data_len > 0)  // 开始接收数据
	{  
        data_len--;
        rxBuffer[2 + data_cnt++] = data;
        if(data_len == 0)   state = 3;  
    } else if (state == 3)  // 数据接收完成
	{
        state = 0;
        data_cnt = 0; 
        data_len = 30; 
        pms5003ReceiveDataAnl(rxBuffer);  
    }
}

void USART3_IRQHandler(void)
{
	uint8_t	USART3_ByteData = 0;
	
	if (USART_GetITStatus(USART3, USART_IT_RXNE) != RESET ) //接收中断
	{
		USART_ClearITPendingBit(USART3, USART_IT_RXNE);
		USART3_ByteData = USART3->DR;
		pms5003ReceiveDataDepare(USART3_ByteData);
	}	
}

void PMS5003_Init(void)
{
	PMS5003_GPIO_Config();
	PMS5003_USART_Config();
	PMS5003_RST_HIGH_LEVEL();
}




