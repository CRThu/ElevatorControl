#include "led.h"

void LED_Init(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
        
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB, ENABLE);	 //使能PB端口时钟

    // PB12-15
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
    GPIO_Init(GPIOB, &GPIO_InitStructure);					 //根据设定参数初始化GPIOB.12 13 14 15
    GPIO_ResetBits(GPIOB,GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15);
    
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_8|GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;           // pull up
 	GPIO_Init(GPIOB, &GPIO_InitStructure);
}

uint8_t KEY_Scan(void)
{
	static uint8_t key_up=1;//按键按松开标志
	if(key_up && (F3_DOWN==0 || F2_UP==0 || F2_DOWN==0 || F1_UP==0
        || ELEVATOR_F3==0 || ELEVATOR_F2==0 || ELEVATOR_F1==0))
	{
		delay_ms(10);//去抖动 
		key_up = 0;
		if(F3_DOWN == 0)            return KEY_F3_DOWN;
		else if(F2_UP == 0)         return KEY_F2_UP;
		else if(F2_DOWN == 0)       return KEY_F2_DOWN;
		else if(F1_UP == 0)         return KEY_F1_UP;
		else if(ELEVATOR_F3 == 0)   return KEY_ELEVATOR_F3;
		else if(ELEVATOR_F2 == 0)   return KEY_ELEVATOR_F2;
		else if(ELEVATOR_F1 == 0)   return KEY_ELEVATOR_F1;
	}
    else if(F3_DOWN==1 && F2_UP==1 && F2_DOWN==1 && F1_UP==1
            && ELEVATOR_F3==1 && ELEVATOR_F2==1 && ELEVATOR_F1==1)
        key_up = 1;
    
 	return 0;// 无按键按下
}
