#include "spi1.h"
#include "commo.h"
#include "rtc.h" 	
#include "SPI1WriteRead.h"
#include "rtthread.h"
#include "dma.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK战舰STM32开发板
//SPI驱动 代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//修改日期:2012/9/9
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019
//All rights reserved									  
//////////////////////////////////////////////////////////////////////////////////
 
//以下是主控板与交采板之间的SPI模块的初始化代码，配置成从机模式					  
//SPI口初始化
//这里针是对SPI1的初始化

extern rt_thread_t tid1;
extern rt_sem_t dynamic_sem;
u8 SPI1_REC_BUF_[196];
u8 Response[196]={0xf1,0xf1,0xf1,0xf1,0xf2,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,
                  0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,
                  0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,
                  0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,
                  0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,
                  0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,
                  0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,
                  0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,0xf3,
                  0xf3,0xf3,0xf3,0xf3};

void SPI1_Init(void)
{
 	GPIO_InitTypeDef GPIO_InitStructure;
  SPI_InitTypeDef  SPI_InitStructure;
	DMA_InitTypeDef DMA_InitStruct;


	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOA, ENABLE );//PORTA时钟使能 
	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_SPI1 ,  ENABLE );//SPI1时钟使能 
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1,ENABLE);//使能DMA的时钟
	  
  /* MISO复用推挽 */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  /* SCK与MOSI浮空输入 */ 
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_7;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  /* NSS浮空输入 */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;  //设置SPI单向或者双向的数据模式:SPI设置为双线双向全双工
	SPI_InitStructure.SPI_Mode = SPI_Mode_Slave;		//设置SPI工作模式:设置为从SPI
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;		//设置SPI的数据大小:SPI发送接收8位帧结构
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;		//串行同步时钟的空闲状态为高电平
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;	//串行同步时钟的第二个跳变沿（上升或下降）数据被采样
	SPI_InitStructure.SPI_NSS = SPI_NSS_Hard;		//NSS信号由硬件（NSS管脚）还是软件（使用SSI位）管理:从机硬件模式
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_64;		//定义波特率预分频的值:波特率预分频值为256
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;	//指定数据传输从MSB位还是LSB位开始:数据传输从MSB位开始
	SPI_InitStructure.SPI_CRCPolynomial = 7;	//CRC值计算的多项式
	SPI_Init(SPI1, &SPI_InitStructure);  //根据SPI_InitStruct中指定的参数初始化外设SPIx寄存器
	
	SPI_Cmd(SPI1, ENABLE); //使能SPI外设
	
	
	DMA_DeInit(DMA1_Channel2);
  DMA_InitStruct.DMA_PeripheralBaseAddr = (uint32_t)(&SPI1->DR);    //外设至寄存器
  DMA_InitStruct.DMA_MemoryBaseAddr = (uint32_t) SPI1_REC_BUF_;
  DMA_InitStruct.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStruct.DMA_BufferSize = 196;
  DMA_InitStruct.DMA_DIR = DMA_DIR_PeripheralSRC;
  DMA_InitStruct.DMA_M2M = DMA_M2M_Disable;
  DMA_InitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
  DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
  DMA_InitStruct.DMA_Priority = DMA_Priority_Medium;
  DMA_InitStruct.DMA_Mode = DMA_Mode_Circular;//循环接收模式
  DMA_Init(DMA1_Channel2, &DMA_InitStruct);
	
	
	DMA_Cmd(DMA1_Channel2, ENABLE);
  SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Rx, ENABLE);
	
	MYDMA_Config((u32)&SPI1->DR,(u32)&Response,196);
	SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Tx, ENABLE);//使能DMA发送
 }

void DMA1_Channel2_IRQHandler(void)//DMA中断程序
{
	rt_interrupt_enter();
	
	if(DMA_GetITStatus(DMA1_IT_TC2)==SET)
  {
     DMA_Cmd(DMA1_Channel2,DISABLE);
     DMA1_Channel2->CNDTR=196;
		 MYDMA_Config((u32)&SPI1->DR,(u32)&Response,196);
	   SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Tx, ENABLE);//使能DMA发送
     DMA_Cmd(DMA1_Channel2,ENABLE);
     DMA_ClearITPendingBit(DMA1_IT_TC2);
		 DMA_ITConfig(DMA1_Channel2, DMA_IT_TC, DISABLE);
  }
	
	rt_interrupt_leave();
}

void DMA1_Channel2_Check(void)
{
	static u8 i=0;
	static u32 state=0;
		if(DMA1_Channel2->CNDTR<196)
		{
			if(state!=DMA1_Channel2->CNDTR)
			{
			  state = DMA1_Channel2->CNDTR;
			}
			else
			{
				if(i>4)
				{
					rt_sem_release(dynamic_sem);//释放一个信号量
          i=0;
				}
				else i+=1;
			}
		}
}

void NVIC_DMAinit(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
	DMA_ITConfig(DMA1_Channel2, DMA_IT_TC, ENABLE);
}
//SPI 速度设置函数
//SpeedSet:
//SPI_BaudRatePrescaler_2   2分频   
//SPI_BaudRatePrescaler_8   8分频   
//SPI_BaudRatePrescaler_16  16分频  
//SPI_BaudRatePrescaler_256 256分频 
  
//void SPI1_SetSpeed(u8 SPI_BaudRatePrescaler)
//{
//  assert_param(IS_SPI_BAUDRATE_PRESCALER(SPI_BaudRatePrescaler));
//	SPI1->CR1&=0XFFC7;
//	SPI1->CR1|=SPI_BaudRatePrescaler;	//设置SPI2速度 
//	SPI_Cmd(SPI1,ENABLE); 

//} 

//SPIx 读写一个字节
//TxData:要写入的字节
//返回值:读取到的字节

/*void uart_init(u32 bound)
{
	//GPIO端口设置
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	DMA_InitTypeDef DMA_InitStruct;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA, ENABLE);	//使能USART1，GPIOA时钟
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);      // 使能DMA

	//USART1_TX   GPIOA.9
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; //PA.9
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
	GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA.9

	//USART1_RX	  GPIOA.10初始化
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;//PA10
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
	GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA.10  

	//Usart1 NVIC 配置
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器

	//USART 初始化设置

	USART_InitStructure.USART_BaudRate = 115200;//串口波特率
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式

  USART_Init(USART1, &USART_InitStructure); //初始化串口1
  USART_ITConfig(USART1, USART_IT_IDLE, ENABLE);//开启总线空闲中断
  USART_Cmd(USART1, ENABLE);                    //使能串口1 
	
	DMA_DeInit(DMA1_Channel5);
  DMA_InitStruct.DMA_PeripheralBaseAddr = (uint32_t)(&USART1->DR);    //外设至寄存器
  DMA_InitStruct.DMA_MemoryBaseAddr = (uint32_t) SPI1_REC_BUF;
  DMA_InitStruct.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStruct.DMA_BufferSize = 512;
  DMA_InitStruct.DMA_DIR = DMA_DIR_PeripheralSRC;
  DMA_InitStruct.DMA_M2M = DMA_M2M_Disable;
  DMA_InitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
  DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
  DMA_InitStruct.DMA_Priority = DMA_Priority_Medium;
  DMA_InitStruct.DMA_Mode = DMA_Mode_Normal;
  DMA_Init(DMA1_Channel5, &DMA_InitStruct);
	
	DMA_Cmd(DMA1_Channel5, ENABLE);
  USART_DMACmd(USART1, USART_DMAReq_Rx, ENABLE);

}


void USART1_IRQHandler(void)                	//串口1中断服务程序
{
	static u16 rec_cnt = 0;
	u16 temp, i;
	
	rt_interrupt_enter();
	
	if(USART_GetITStatus(USART1, USART_IT_IDLE) != RESET)  //接收中断(接收到的数据必须是0x0d 0x0a结尾)
	{
		//USART_ClearFlag(USART1,USART_IT_IDLE);
		temp = USART1->SR;
    temp = USART1->DR;
		DMA_Cmd(DMA1_Channel5,DISABLE);
		
		rt_sem_release(dynamic_sem);//释放一个信号量
    //i=rt_thread_control(tid1,RT_THREAD_CTRL_CHANGE_PRIORITY,(void*)1);
		//rt_thread_resume (tid1);	
		DMA_Cmd(DMA1_Channel5,ENABLE);

  }
  
  rt_interrupt_leave();	
} */























