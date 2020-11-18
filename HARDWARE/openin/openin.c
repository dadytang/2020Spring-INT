#include "openin.h"

u8 YXout[4], YXout_[4], YXin[4];
void openinit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
  RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOA, ENABLE );//PORTA时钟使能 
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOA, &GPIO_InitStructure);//设置浮空输入
	
	YXout_[0] = YX0;
	YXout_[1] = YX1;
	YXout_[2] = YX2;
	YXout_[3] = YX3;
	
}

