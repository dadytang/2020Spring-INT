#include "spi2.h"
#include "mydefine.h"
#include "commo.h"
#include "rtthread.h"
#include "24cxx.h"
#include "rthw.h"
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
 
//������MCU��ad��SPIģ��ĳ�ʼ�����룬���ó�����ģʽ������SD Card/W25Q64/NRF24L01						  
//SPI�ڳ�ʼ��
//�������Ƕ�SPI2�ĳ�ʼ��

_ac_time3_t_obj ac_time3_t;	//�����ṹ��
ac_real1_obj ac_real1;
ac_real2_obj ac_real2;
type_att_para2_obj type_att_para2;
extern u32 AdjSta;
extern u8 adj_flag;

void SPI2_Init(void)
{
 	GPIO_InitTypeDef GPIO_InitStructure;
  SPI_InitTypeDef  SPI_InitStructure;

	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC, ENABLE );//PORTBʱ��ʹ�� 
	RCC_APB1PeriphClockCmd(	RCC_APB1Periph_SPI2,  ENABLE );//SPI2ʱ��ʹ�� 	
 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  //PB13/14/15����������� 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);//��ʼ��GPIOB
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; //ͨ������        
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  GPIO_SetBits(GPIOB,GPIO_Pin_12);  //Ƭѡ�ź�����
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7; //��λ
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
  GPIO_SetBits(GPIOC,GPIO_Pin_7);

	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;  //����SPI�������˫�������ģʽ:SPI����Ϊ˫��˫��ȫ˫��
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;		//����SPI����ģʽ:����Ϊ��SPI
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;		//����SPI�����ݴ�С:SPI���ͽ���8λ֡�ṹ
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;		//����ͬ��ʱ�ӵĿ���״̬Ϊ�͵�ƽ
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;	//����ͬ��ʱ�ӵĵڶ��������أ��������½������ݱ�����
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;		//NSS�ź���Ӳ����NSS�ܽţ����������ʹ��SSIλ������:�ڲ�NSS�ź���SSIλ����
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;		//���岨����Ԥ��Ƶ��ֵ:������Ԥ��ƵֵΪ256
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;	//ָ�����ݴ����MSBλ����LSBλ��ʼ:���ݴ����MSBλ��ʼ
	SPI_InitStructure.SPI_CRCPolynomial = 7;	//CRCֵ����Ķ���ʽ
	SPI_Init(SPI2, &SPI_InitStructure);  //����SPI_InitStruct��ָ���Ĳ�����ʼ������SPIx�Ĵ���
 
	SPI_Cmd(SPI2, ENABLE); //ʹ��SPI����
	
	SPI2_ReadWriteByte(0xff);//��������		 
 

}   
//SPI �ٶ����ú���
//SpeedSet:
//SPI_BaudRatePrescaler_2   2��Ƶ   
//SPI_BaudRatePrescaler_8   8��Ƶ   
//SPI_BaudRatePrescaler_16  16��Ƶ  
//SPI_BaudRatePrescaler_256 256��Ƶ 
  
void SPI2_SetSpeed(u8 SPI_BaudRatePrescaler)
{
  assert_param(IS_SPI_BAUDRATE_PRESCALER(SPI_BaudRatePrescaler));
	SPI2->CR1&=0XFFC7;
	SPI2->CR1|=SPI_BaudRatePrescaler;	//����SPI2�ٶ� 
	SPI_Cmd(SPI2,ENABLE); 

} 

//SPIx ��дһ���ֽ�
//TxData:Ҫд����ֽ�
//����ֵ:��ȡ�����ֽ�
u8 SPI2_ReadWriteByte(u8 TxData)
{		
	u8 retry=0;				 	
	while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET) //���ָ����SPI��־λ�������:���ͻ���ձ�־λ
		{
		retry++;
		if(retry>200)return 0;
		}			  
	SPI_I2S_SendData(SPI2, TxData); //ͨ������SPIx����һ������
	retry=0;

	while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET) //���ָ����SPI��־λ�������:���ܻ���ǿձ�־λ
		{
		retry++;
		if(retry>200)return 0;
		}						    
	return SPI_I2S_ReceiveData(SPI2); //����ͨ��SPIx������յ�����					    
}

//����������
s32 ReadSampleRegister(u8 adReadAddr)
{
	u32 adBuffer;
	u8 tempdata[3];
	adReadAddr = 0x00 | adReadAddr;
	ATT7022_CS_EN;
	SPI2_ReadWriteByte(adReadAddr);
	//delay_us(2); //���sclk����500kHZ���ӳ�2uS�ٶ�ȡ
	tempdata[0] = SPI2_ReadWriteByte(0xFE);//ѭ������0xFE�õ����
	tempdata[1] = SPI2_ReadWriteByte(0xFE);
	tempdata[2] = SPI2_ReadWriteByte(0xFE);
	ATT7022_CS_DIS;
	adBuffer = (ArrayData(tempdata, 3));
	return adBuffer;
}

s16 ReadSampleRegister2(u8 adReadAddr)
{
	return (s16)ReadSampleRegister(adReadAddr);
}

//��У�����
u32 ReadAdjustRegister(u8 addr)
{
	u32 adBuffer;
	u8 tempdata[3];
	ATT7022_CS_EN;
	SPI2_ReadWriteByte(0xC6);  //ʹ�ܽϱ�Ĵ���������
	SPI2_ReadWriteByte(0);
	SPI2_ReadWriteByte(0);
	SPI2_ReadWriteByte(0x5A);
	ATT7022_CS_DIS;
  delay_ms(1);
	addr = 0x00 | addr;
	ATT7022_CS_EN;
	SPI2_ReadWriteByte(addr);//������
	//delay_us(2); //���sclk����500kHZ���ӳ�2uS�ٶ�ȡ
	tempdata[0] = SPI2_ReadWriteByte(0xFE);//ѭ������0xFE�õ����
	tempdata[1] = SPI2_ReadWriteByte(0xFE);
	tempdata[2] = SPI2_ReadWriteByte(0xFE);
	ATT7022_CS_DIS;
	adBuffer = (ArrayData(tempdata, 3))&0x00ffffff;
	ATT7022_CS_EN;
	SPI2_ReadWriteByte(0xC6);  //�ر�У��Ĵ���������ʹ�ܼ����Ĵ���������
	SPI2_ReadWriteByte(0);
	SPI2_ReadWriteByte(0);
	SPI2_ReadWriteByte(0xaa);
	ATT7022_CS_DIS;
	delay_ms(1);
	return adBuffer;
}
//дУ�����
void WriteAdjustRegister(u8 addr,u32 data)
{
  u8 i, tempdata[3];
	for(i=0; i<3; i++)
	{
		tempdata[i] = data&0x000000ff;
		data = data >> 8;
	}
	ATT7022_CS_EN;	
	SPI2_ReadWriteByte(0xC9); 
	SPI2_ReadWriteByte(0);
	SPI2_ReadWriteByte(0);
	SPI2_ReadWriteByte(0x5A);//ʹ�ܽϱ�Ĵ���д����
	ATT7022_CS_DIS;
  delay_us(2);
	
	addr = 0x80 | addr;
	ATT7022_CS_EN;
  SPI2_ReadWriteByte(addr);  //д����
	SPI2_ReadWriteByte(tempdata[2]);
	SPI2_ReadWriteByte(tempdata[1]);
	SPI2_ReadWriteByte(tempdata[0]);
	ATT7022_CS_DIS;
  delay_us(2);
	
	ATT7022_CS_EN;
	SPI2_ReadWriteByte(0xC9); 
	SPI2_ReadWriteByte(0);
	SPI2_ReadWriteByte(0);
	SPI2_ReadWriteByte(0);//�رսϱ�Ĵ���д������ֹ���޸�
	ATT7022_CS_DIS;
  delay_us(2);
	
}

//att7022�����λ
void ResetATT7022(void)
{
	GPIO_ResetBits (GPIOC, GPIO_Pin_7); 
  delay_us(30);
	GPIO_SetBits(GPIOC,GPIO_Pin_7);
	delay_ms(30);
}

//���У��Ĵ���
void ClearAdjustRegister(void)
{
	
	ATT7022_CS_EN;
	SPI2_ReadWriteByte(0xC3); 
	SPI2_ReadWriteByte(0);
	SPI2_ReadWriteByte(0);
	SPI2_ReadWriteByte(0);
	ATT7022_CS_DIS;
  delay_ms(10);
}

void ReadyToAdjust(float Un, float Ib, u32 sn)//У��֮ǰ��������/////////////////////////////////////////////////////////////////////////////////////������
{
	float Vu, Vi;////////////////////////////////////
	u32 tmbuff=0;
	//ClearAdjustRegister();//���У��Ĵ���
	WriteAdjustRegister(0x01,tmbuff);
	WriteAdjustRegister(0x01,0x00B97E);//ģʽ����
	WriteAdjustRegister(0x03,tmbuff);
	WriteAdjustRegister(0x03,0x00F804);//emu��Ԫ����
	WriteAdjustRegister(0x31,tmbuff);
	WriteAdjustRegister(0x31,0x003427);//ģ��ʹ��
	delay_ms(1000);
	UpdateAc_real1(sn);
	delay_ms(3000);
	UpdateAc_real1(sn+1);
	delay_ms(3000);
	UpdateAc_real1(sn+2);
	delay_ms(3000);
	/*Vu=(float)ReadSampleRegister(0x0D);////////////////////////////////////////////////
	Vi=(float)ReadSampleRegister(0x10);////////////////////////////////////////////////////
	CompuHFconst(Un, Ib, Vu, Vi);*/
}

void EndToAdjust(void)//У����������
{
	adj_flag=0;
  AT24CXX_Write(1028,(u8 *)&type_att_para2.w_PhSregApq[0],176);
  AdjSta=0x0100;
	//ClearAdjustRegister();//���У��Ĵ���
	//ConfigAdjustRegister();
	WriteAdjustRegister(0x01,0);
	WriteAdjustRegister(0x01,0xbd7f);
	WriteAdjustRegister(0x03,0);
	WriteAdjustRegister(0x03,0xfda2);
}

//��ʼ��У��Ĵ���
void ConfigAdjustRegister(void)
{
	
	WriteAdjustRegister(0x04+APhase, type_att_para2.w_Pgain[0]);
	WriteAdjustRegister(0x04+BPhase, type_att_para2.w_Pgain[1]);
	WriteAdjustRegister(0x04+BPhase, type_att_para2.w_Pgain[2]);
	
	WriteAdjustRegister(0x07+APhase, type_att_para2.w_Qgain[0]);
	WriteAdjustRegister(0x07+BPhase, type_att_para2.w_Qgain[1]);
	WriteAdjustRegister(0x07+CPhase, type_att_para2.w_Qgain[2]);
	
	WriteAdjustRegister(0x0A+APhase, type_att_para2.w_Sgain[0]);
	WriteAdjustRegister(0x0A+BPhase, type_att_para2.w_Sgain[1]);
	WriteAdjustRegister(0x0A+CPhase, type_att_para2.w_Sgain[2]);
	
	WriteAdjustRegister(0x10+APhase, type_att_para2.w_PhSregApq[0]);
	WriteAdjustRegister(0x10+BPhase, type_att_para2.w_PhSregApq[1]);
	WriteAdjustRegister(0x10+CPhase, type_att_para2.w_PhSregApq[2]);
	
	WriteAdjustRegister(0x17+APhase, type_att_para2.w_Ugain[0]);
	WriteAdjustRegister(0x17+BPhase, type_att_para2.w_Ugain[1]);
	WriteAdjustRegister(0x17+CPhase, type_att_para2.w_Ugain[2]);
	
	WriteAdjustRegister(0x1A+APhase, type_att_para2.w_Igain[0]);
	WriteAdjustRegister(0x1A+BPhase, type_att_para2.w_Igain[1]);
	WriteAdjustRegister(0x1A+CPhase, type_att_para2.w_Igain[2]);
	
	WriteAdjustRegister(0x13+APhase, type_att_para2.w_Poffset[0]);
	WriteAdjustRegister(0x13+BPhase, type_att_para2.w_Poffset[1]);
	WriteAdjustRegister(0x13+CPhase, type_att_para2.w_Poffset[2]);
	
	WriteAdjustRegister(0x21+APhase, type_att_para2.w_Qoffset[0]);
	WriteAdjustRegister(0x21+BPhase, type_att_para2.w_Qoffset[1]);
	WriteAdjustRegister(0x21+CPhase, type_att_para2.w_Qoffset[2]);
	
	WriteAdjustRegister(0x64+APhase, type_att_para2.w_PoffsetL[0]);
	WriteAdjustRegister(0x64+BPhase, type_att_para2.w_PoffsetL[1]);
	WriteAdjustRegister(0x64+CPhase, type_att_para2.w_PoffsetL[2]);
	
	WriteAdjustRegister(0x67+APhase, type_att_para2.w_QoffsetL[0]);
	WriteAdjustRegister(0x67+BPhase, type_att_para2.w_QoffsetL[1]);
	WriteAdjustRegister(0x67+CPhase, type_att_para2.w_QoffsetL[2]);
	
	WriteAdjustRegister(0x20, type_att_para2.w_GainADC7);
	
	WriteAdjustRegister(0x24+APhase, type_att_para2.w_UaRmsoffse[0]);
	WriteAdjustRegister(0x24+BPhase, type_att_para2.w_UaRmsoffse[1]);
	WriteAdjustRegister(0x24+CPhase, type_att_para2.w_UaRmsoffse[2]);
	
	WriteAdjustRegister(0x27+APhase, type_att_para2.w_IaRmsoffse[0]);
	WriteAdjustRegister(0x27+BPhase, type_att_para2.w_IaRmsoffse[1]);
	WriteAdjustRegister(0x27+CPhase, type_att_para2.w_IaRmsoffse[2]);
	
	WriteAdjustRegister(0x6B, type_att_para2.w_TPSoffset);
	
	WriteAdjustRegister(0x70, 0x00000008);//���ѡ��Ϊ������ѹ
	
}

























