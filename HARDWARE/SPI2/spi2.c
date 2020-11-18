#include "spi2.h"
#include "mydefine.h"
#include "commo.h"
#include "rtthread.h"
#include "24cxx.h"
#include "rthw.h"
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
 
//以下是MCU对ad的SPI模块的初始化代码，配置成主机模式，访问SD Card/W25Q64/NRF24L01						  
//SPI口初始化
//这里针是对SPI2的初始化

_ac_time3_t_obj ac_time3_t;	//日历结构体
ac_real1_obj ac_real1;
ac_real2_obj ac_real2;
type_att_para2_obj type_att_para2;
extern u32 AdjSta;
extern u8 adj_flag;

void SPI2_Init(void)
{
 	GPIO_InitTypeDef GPIO_InitStructure;
  SPI_InitTypeDef  SPI_InitStructure;

	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC, ENABLE );//PORTB时钟使能 
	RCC_APB1PeriphClockCmd(	RCC_APB1Periph_SPI2,  ENABLE );//SPI2时钟使能 	
 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  //PB13/14/15复用推挽输出 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);//初始化GPIOB
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; //通用推挽        
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  GPIO_SetBits(GPIOB,GPIO_Pin_12);  //片选信号上拉
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7; //复位
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
  GPIO_SetBits(GPIOC,GPIO_Pin_7);

	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;  //设置SPI单向或者双向的数据模式:SPI设置为双线双向全双工
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;		//设置SPI工作模式:设置为主SPI
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;		//设置SPI的数据大小:SPI发送接收8位帧结构
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;		//串行同步时钟的空闲状态为低电平
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;	//串行同步时钟的第二个跳变沿（上升或下降）数据被采样
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;		//NSS信号由硬件（NSS管脚）还是软件（使用SSI位）管理:内部NSS信号有SSI位控制
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;		//定义波特率预分频的值:波特率预分频值为256
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;	//指定数据传输从MSB位还是LSB位开始:数据传输从MSB位开始
	SPI_InitStructure.SPI_CRCPolynomial = 7;	//CRC值计算的多项式
	SPI_Init(SPI2, &SPI_InitStructure);  //根据SPI_InitStruct中指定的参数初始化外设SPIx寄存器
 
	SPI_Cmd(SPI2, ENABLE); //使能SPI外设
	
	SPI2_ReadWriteByte(0xff);//启动传输		 
 

}   
//SPI 速度设置函数
//SpeedSet:
//SPI_BaudRatePrescaler_2   2分频   
//SPI_BaudRatePrescaler_8   8分频   
//SPI_BaudRatePrescaler_16  16分频  
//SPI_BaudRatePrescaler_256 256分频 
  
void SPI2_SetSpeed(u8 SPI_BaudRatePrescaler)
{
  assert_param(IS_SPI_BAUDRATE_PRESCALER(SPI_BaudRatePrescaler));
	SPI2->CR1&=0XFFC7;
	SPI2->CR1|=SPI_BaudRatePrescaler;	//设置SPI2速度 
	SPI_Cmd(SPI2,ENABLE); 

} 

//SPIx 读写一个字节
//TxData:要写入的字节
//返回值:读取到的字节
u8 SPI2_ReadWriteByte(u8 TxData)
{		
	u8 retry=0;				 	
	while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET) //检查指定的SPI标志位设置与否:发送缓存空标志位
		{
		retry++;
		if(retry>200)return 0;
		}			  
	SPI_I2S_SendData(SPI2, TxData); //通过外设SPIx发送一个数据
	retry=0;

	while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET) //检查指定的SPI标志位设置与否:接受缓存非空标志位
		{
		retry++;
		if(retry>200)return 0;
		}						    
	return SPI_I2S_ReceiveData(SPI2); //返回通过SPIx最近接收的数据					    
}

//读计量参数
s32 ReadSampleRegister(u8 adReadAddr)
{
	u32 adBuffer;
	u8 tempdata[3];
	adReadAddr = 0x00 | adReadAddr;
	ATT7022_CS_EN;
	SPI2_ReadWriteByte(adReadAddr);
	//delay_us(2); //如果sclk高于500kHZ则延迟2uS再读取
	tempdata[0] = SPI2_ReadWriteByte(0xFE);//循环输入0xFE得到输出
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

//读校表参数
u32 ReadAdjustRegister(u8 addr)
{
	u32 adBuffer;
	u8 tempdata[3];
	ATT7022_CS_EN;
	SPI2_ReadWriteByte(0xC6);  //使能较表寄存器读操作
	SPI2_ReadWriteByte(0);
	SPI2_ReadWriteByte(0);
	SPI2_ReadWriteByte(0x5A);
	ATT7022_CS_DIS;
  delay_ms(1);
	addr = 0x00 | addr;
	ATT7022_CS_EN;
	SPI2_ReadWriteByte(addr);//读数据
	//delay_us(2); //如果sclk高于500kHZ则延迟2uS再读取
	tempdata[0] = SPI2_ReadWriteByte(0xFE);//循环输入0xFE得到输出
	tempdata[1] = SPI2_ReadWriteByte(0xFE);
	tempdata[2] = SPI2_ReadWriteByte(0xFE);
	ATT7022_CS_DIS;
	adBuffer = (ArrayData(tempdata, 3))&0x00ffffff;
	ATT7022_CS_EN;
	SPI2_ReadWriteByte(0xC6);  //关闭校表寄存器读操作使能计量寄存器读操作
	SPI2_ReadWriteByte(0);
	SPI2_ReadWriteByte(0);
	SPI2_ReadWriteByte(0xaa);
	ATT7022_CS_DIS;
	delay_ms(1);
	return adBuffer;
}
//写校表参数
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
	SPI2_ReadWriteByte(0x5A);//使能较表寄存器写操作
	ATT7022_CS_DIS;
  delay_us(2);
	
	addr = 0x80 | addr;
	ATT7022_CS_EN;
  SPI2_ReadWriteByte(addr);  //写数据
	SPI2_ReadWriteByte(tempdata[2]);
	SPI2_ReadWriteByte(tempdata[1]);
	SPI2_ReadWriteByte(tempdata[0]);
	ATT7022_CS_DIS;
  delay_us(2);
	
	ATT7022_CS_EN;
	SPI2_ReadWriteByte(0xC9); 
	SPI2_ReadWriteByte(0);
	SPI2_ReadWriteByte(0);
	SPI2_ReadWriteByte(0);//关闭较表寄存器写操作防止勿修改
	ATT7022_CS_DIS;
  delay_us(2);
	
}

//att7022软件复位
void ResetATT7022(void)
{
	GPIO_ResetBits (GPIOC, GPIO_Pin_7); 
  delay_us(30);
	GPIO_SetBits(GPIOC,GPIO_Pin_7);
	delay_ms(30);
}

//清除校表寄存器
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

void ReadyToAdjust(float Un, float Ib, u32 sn)//校表之前参数设置/////////////////////////////////////////////////////////////////////////////////////待修正
{
	float Vu, Vi;////////////////////////////////////
	u32 tmbuff=0;
	//ClearAdjustRegister();//清除校表寄存器
	WriteAdjustRegister(0x01,tmbuff);
	WriteAdjustRegister(0x01,0x00B97E);//模式配置
	WriteAdjustRegister(0x03,tmbuff);
	WriteAdjustRegister(0x03,0x00F804);//emu单元配置
	WriteAdjustRegister(0x31,tmbuff);
	WriteAdjustRegister(0x31,0x003427);//模块使能
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

void EndToAdjust(void)//校表后参数设置
{
	adj_flag=0;
  AT24CXX_Write(1028,(u8 *)&type_att_para2.w_PhSregApq[0],176);
  AdjSta=0x0100;
	//ClearAdjustRegister();//清除校表寄存器
	//ConfigAdjustRegister();
	WriteAdjustRegister(0x01,0);
	WriteAdjustRegister(0x01,0xbd7f);
	WriteAdjustRegister(0x03,0);
	WriteAdjustRegister(0x03,0xfda2);
}

//初始化校表寄存器
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
	
	WriteAdjustRegister(0x70, 0x00000008);//相角选择为电流电压
	
}

























