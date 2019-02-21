#include "key.h"
#include "delay.h"

void Key_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode =  GPIO_Mode_IPU;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
}


u8 KEY_Scan(u8 mode)
{	 
	static u8 key_up=1;		//�������ɿ���־
	
	if(mode)key_up=1;  		//֧������		  
	if(key_up && (0==KEY2 || 1==WK_UP))
	{
		delay_ms(10);//ȥ���� 
		key_up=0;
		if(KEY2 == 0)
			return KEY2_PRES;
		else if(WK_UP == 1)
			return WKUP_PRES;
	}
	else if(KEY2==1 && WK_UP==0)
		key_up=1; 	    
 	
	return 0;// �ް�������
}
