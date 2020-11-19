#include "spi1.h"
#include "commo.h"
#include "rtc.h" 	
#include "SPI1WriteRead.h"
#include "rtthread.h"
#include "dma.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEKս��STM32������
//SPI���� ����	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//�޸�����:2012/9/9
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2009-2019
//All rights reserved									  
//////////////////////////////////////////////////////////////////////////////////
 
//���������ذ��뽻�ɰ�֮���SPIģ��ĳ�ʼ�����룬���óɴӻ�ģʽ					  
//SPI�ڳ�ʼ��
//�������Ƕ�SPI1�ĳ�ʼ��

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


	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOA, ENABLE );//PORTAʱ��ʹ�� 
	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_SPI1 ,  ENABLE );//SPI1ʱ��ʹ�� 
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1,ENABLE);//ʹ��DMA��ʱ��
	  
  /* MISO�������� */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  /* SCK��MOSI�������� */ 
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_7;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  /* NSS�������� */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;  //����SPI�������˫�������ģʽ:SPI����Ϊ˫��˫��ȫ˫��
	SPI_InitStructure.SPI_Mode = SPI_Mode_Slave;		//����SPI����ģʽ:����Ϊ��SPI
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;		//����SPI�����ݴ�С:SPI���ͽ���8λ֡�ṹ
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;		//����ͬ��ʱ�ӵĿ���״̬Ϊ�ߵ�ƽ
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;	//����ͬ��ʱ�ӵĵڶ��������أ��������½������ݱ�����
	SPI_InitStructure.SPI_NSS = SPI_NSS_Hard;		//NSS�ź���Ӳ����NSS�ܽţ����������ʹ��SSIλ������:�ӻ�Ӳ��ģʽ
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_64;		//���岨����Ԥ��Ƶ��ֵ:������Ԥ��ƵֵΪ256
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;	//ָ�����ݴ����MSBλ����LSBλ��ʼ:���ݴ����MSBλ��ʼ
	SPI_InitStructure.SPI_CRCPolynomial = 7;	//CRCֵ����Ķ���ʽ
	SPI_Init(SPI1, &SPI_InitStructure);  //����SPI_InitStruct��ָ���Ĳ�����ʼ������SPIx�Ĵ���
	
	SPI_Cmd(SPI1, ENABLE); //ʹ��SPI����
	
	
	DMA_DeInit(DMA1_Channel2);
  DMA_InitStruct.DMA_PeripheralBaseAddr = (uint32_t)(&SPI1->DR);    //�������Ĵ���
  DMA_InitStruct.DMA_MemoryBaseAddr = (uint32_t) SPI1_REC_BUF_;
  DMA_InitStruct.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStruct.DMA_BufferSize = 196;
  DMA_InitStruct.DMA_DIR = DMA_DIR_PeripheralSRC;
  DMA_InitStruct.DMA_M2M = DMA_M2M_Disable;
  DMA_InitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
  DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
  DMA_InitStruct.DMA_Priority = DMA_Priority_Medium;
  DMA_InitStruct.DMA_Mode = DMA_Mode_Circular;//ѭ������ģʽ
  DMA_Init(DMA1_Channel2, &DMA_InitStruct);
	
	
	DMA_Cmd(DMA1_Channel2, ENABLE);
  SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Rx, ENABLE);
	
	MYDMA_Config((u32)&SPI1->DR,(u32)&Response,196);
	SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Tx, ENABLE);//ʹ��DMA����
 }

void DMA1_Channel2_IRQHandler(void)//DMA�жϳ���
{
	rt_interrupt_enter();
	
	if(DMA_GetITStatus(DMA1_IT_TC2)==SET)
  {
     DMA_Cmd(DMA1_Channel2,DISABLE);
     DMA1_Channel2->CNDTR=196;
		 MYDMA_Config((u32)&SPI1->DR,(u32)&Response,196);
	   SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Tx, ENABLE);//ʹ��DMA����
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
					rt_sem_release(dynamic_sem);//�ͷ�һ���ź���
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
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���
	DMA_ITConfig(DMA1_Channel2, DMA_IT_TC, ENABLE);
}
//SPI �ٶ����ú���
//SpeedSet:
//SPI_BaudRatePrescaler_2   2��Ƶ   
//SPI_BaudRatePrescaler_8   8��Ƶ   
//SPI_BaudRatePrescaler_16  16��Ƶ  
//SPI_BaudRatePrescaler_256 256��Ƶ 
  
//void SPI1_SetSpeed(u8 SPI_BaudRatePrescaler)
//{
//  assert_param(IS_SPI_BAUDRATE_PRESCALER(SPI_BaudRatePrescaler));
//	SPI1->CR1&=0XFFC7;
//	SPI1->CR1|=SPI_BaudRatePrescaler;	//����SPI2�ٶ� 
//	SPI_Cmd(SPI1,ENABLE); 

//} 

//SPIx ��дһ���ֽ�
//TxData:Ҫд����ֽ�
//����ֵ:��ȡ�����ֽ�

/*void uart_init(u32 bound)
{
	//GPIO�˿�����
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	DMA_InitTypeDef DMA_InitStruct;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA, ENABLE);	//ʹ��USART1��GPIOAʱ��
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);      // ʹ��DMA

	//USART1_TX   GPIOA.9
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; //PA.9
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//�����������
	GPIO_Init(GPIOA, &GPIO_InitStructure);//��ʼ��GPIOA.9

	//USART1_RX	  GPIOA.10��ʼ��
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;//PA10
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//��������
	GPIO_Init(GPIOA, &GPIO_InitStructure);//��ʼ��GPIOA.10  

	//Usart1 NVIC ����
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���

	//USART ��ʼ������

	USART_InitStructure.USART_BaudRate = 115200;//���ڲ�����
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ

  USART_Init(USART1, &USART_InitStructure); //��ʼ������1
  USART_ITConfig(USART1, USART_IT_IDLE, ENABLE);//�������߿����ж�
  USART_Cmd(USART1, ENABLE);                    //ʹ�ܴ���1 
	
	DMA_DeInit(DMA1_Channel5);
  DMA_InitStruct.DMA_PeripheralBaseAddr = (uint32_t)(&USART1->DR);    //�������Ĵ���
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


void USART1_IRQHandler(void)                	//����1�жϷ������
{
	static u16 rec_cnt = 0;
	u16 temp, i;
	
	rt_interrupt_enter();
	
	if(USART_GetITStatus(USART1, USART_IT_IDLE) != RESET)  //�����ж�(���յ������ݱ�����0x0d 0x0a��β)
	{
		//USART_ClearFlag(USART1,USART_IT_IDLE);
		temp = USART1->SR;
    temp = USART1->DR;
		DMA_Cmd(DMA1_Channel5,DISABLE);
		
		rt_sem_release(dynamic_sem);//�ͷ�һ���ź���
    //i=rt_thread_control(tid1,RT_THREAD_CTRL_CHANGE_PRIORITY,(void*)1);
		//rt_thread_resume (tid1);	
		DMA_Cmd(DMA1_Channel5,ENABLE);

  }
  
  rt_interrupt_leave();	
} */























