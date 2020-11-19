#include "SPI1WriteRead.h"
#include "spi2.h"
#include "commo.h"
#include "24cxx.h"
#include "spi1.h"
#include "rtc.h" 
#include "dma.h"
#include "rthw.h"

extern type_att_para2_obj type_att_para2;
extern ac_real1_obj ac_real1;
extern ac_real2_obj ac_real2;
extern u8 SPI1_REC_BUF_[196];
extern u8 adj_flag,krcnt;
extern u8 RXstatus;
extern u32 AdjSta,krnum;
extern 	_ac_time3_t_obj tm[20];
extern emuInfo_t_obj emuInfo_t;
u32 adj_m;
frm_rd_coeff_respose_obj frm_rd_coeff_respose;
EmuInfoResp_t_obj EmuInfoResp_t;
ac_real2_t_obj ac_rea12_t;
ac_real1_t_obj ac_rea11_t;
kairu_obj kairu;
frm_start_adj_response_t_obj frm_start_adj_response;

//校验和sum计算函数
u32 GetSum(vu8 *data, u32 length)
{
	u32 i, sum=0;
	for(i=0; i<length; i++)
	{
		sum = sum + *(data+i);
	}
	return sum;
}
//底板信息装填帧
void EmuInfoResp(void)
{
	u32 sum;
	EmuInfoResp_t.reslogo = 0xf3;
	EmuInfoResp_t.addr = 0x01;
	EmuInfoResp_t.comcode = 0x03;
	EmuInfoResp_t.datalogo = 0x0e;
	EmuInfoResp_t.datalogo1 = 0x0e;
	EmuInfoResp_t.length = 0x90;
	EmuInfoResp_t.length1 = 0;
	emuInfo();
	EmuInfoResp_t.emuInfo_t = emuInfo_t;
	sum=GetSum(&EmuInfoResp_t.addr, 0x00000096);
	EmuInfoResp_t.sum0 = sum&0xff;
  EmuInfoResp_t.sum1 = (sum&0xff00)>>8;
}
//实时数据帧1填写
void ac_real1_t(void)
{
	u32 sum;
	ac_rea11_t.reslogo = 0xf3;
	ac_rea11_t.addr = 0x01;
	ac_rea11_t.comcode = 0x03;
	ac_rea11_t.datalogo = 0;
	ac_rea11_t.datalogo1 = 0x01;
	ac_rea11_t.length = 0xfc;
	ac_rea11_t.length1 = 0x02;
	ac_rea11_t.ac_real1 = ac_real1;
	sum = GetSum(&ac_rea11_t.addr, 0x00000302);
  ac_rea11_t.sum0 = sum&0xff;
  ac_rea11_t.sum1 = (sum&0xff00)>>8;
  ac_rea11_t.taillogo=0xf1;	
}
//实时数据帧2填写
void ac_real2_t(void)
{
	u32 sum;
	ac_rea12_t.reslogo = 0xf3;
	ac_rea12_t.addr = 0x01;
	ac_rea12_t.comcode = 0x03;
	ac_rea12_t.datalogo = 0x01;
	ac_rea12_t.datalogo1 = 0x01;
	ac_rea12_t.length = 0x58;
	ac_rea12_t.length1 = 0x01;
	ac_rea12_t.ac_real2 = ac_real2;
	sum = GetSum(&ac_rea12_t.addr, 0x0000015e);
	ac_rea12_t.sum0 = sum&0xff;
  ac_rea12_t.sum1 = (sum&0xff00)>>8;
	ac_rea12_t.taillogo=0xf1;
}
//开入开出帧装填
void SPI1ReadEvent(void)
{
	u8 i;
	u32 sum;
	kairu.reslogo=0xf3;
	kairu.addr=0x01;
	kairu.comcode=0x03;
	kairu.datalogo=0;
	kairu.datalogo1=0x05;
	kairu.length=0x48;
	kairu.length1=0x01;
	kairu.rdptr=krnum;
	kairu.num=krcnt;
	for(i=0;i<20;i++) kairu.tm[i]=tm[i];
	sum = GetSum(&kairu.addr, 0x0000014e);
	kairu.sum0=sum&0xff;;
	kairu.sum1=(sum&0xff00)>>8;
	kairu.taillogo=0xf1;
}

//校表参数装填函数
void frm_rd_coeff(void)
{
	u32 sum;
	frm_rd_coeff_respose.reslogo = 0xf3;
	frm_rd_coeff_respose.addr = 0x01;
	frm_rd_coeff_respose.comcode = 0x03;
	frm_rd_coeff_respose.datalogo = 0;
	frm_rd_coeff_respose.datalogo1 = 0x02;
	frm_rd_coeff_respose.length = 0xb2;
	frm_rd_coeff_respose.length1 = 0x00;
	frm_rd_coeff_respose.sta = AdjSta;////////////////////////////////////////////////////////////////////////////////////待修正
	frm_rd_coeff_respose.sta1 = AdjSta>>8;////////////////////////////////////////////////////////////////////////////
	frm_rd_coeff_respose.type_att_para2 = type_att_para2;
	sum = GetSum(&frm_rd_coeff_respose.addr, 0x000000b8);
	frm_rd_coeff_respose.sum0 = sum&0xff;
  frm_rd_coeff_respose.sum1 = (sum&0xff00)>>8;
}

void startadj_respon(void)//校表启动响应兼发送
{
	frm_start_adj_response.reslogo=0xf3;
	frm_start_adj_response.addr=0x01;
	frm_start_adj_response.comcode=0x10;
	frm_start_adj_response.datalogo=0x00;
	frm_start_adj_response.datalogo1=0x03;
	frm_start_adj_response.length=0x02;
	frm_start_adj_response.length1=0x00;
	frm_start_adj_response.sta=0;
	frm_start_adj_response.sta1=0;
	frm_start_adj_response.sum0=0x16;
	frm_start_adj_response.sum1=0x00;
	MYDMA_Config((u32)&SPI1->DR,(u32)&frm_start_adj_response.addr,0xb);
	SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Tx, ENABLE);//使能DMA发送
	MYDMA_Enable(DMA1_Channel2);//开始一次发送
}

void writeadj_respon(void)//写校表响应兼发送
{
	frm_start_adj_response.reslogo=0xf3;
	frm_start_adj_response.addr=0x01;
	frm_start_adj_response.comcode=0x03;
	frm_start_adj_response.datalogo=0x00;
	frm_start_adj_response.datalogo1=0x02;
	frm_start_adj_response.length=0x02;
	frm_start_adj_response.length1=0x00;
	frm_start_adj_response.sta=0;
	frm_start_adj_response.sta1=0;
	frm_start_adj_response.sum0=0xfb;
	frm_start_adj_response.sum1=0x00;
	frm_start_adj_response.taillogo=0xf1;
	
}
//底板信息发送函数
void SPI1EmuInfoResp_t(void)
{
	EmuInfoResp();
	MYDMA_Config((u32)&SPI1->DR,(u32)&EmuInfoResp_t.addr,0x99);
	SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Tx, ENABLE);//使能DMA发送
	MYDMA_Enable(DMA1_Channel2);//开始一次发送
}
//实时数据装填发送函数
void SPI1ReadAcreal(u16 ac_real)
{
	if(ac_real == 0x0100)//ac_real1数据
	{
		ac_real1_t();
		MYDMA_Config((u32)&SPI1->DR,(u32)&ac_rea11_t.addr,0x305);
		SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Tx, ENABLE);//使能DMA发送
		MYDMA_Enable(DMA1_Channel2);//开始一次发送
	}
	else if(ac_real == 0x0101)//ac_real2数据
	{
		ac_real2_t();
		MYDMA_Config((u32)&SPI1->DR,(u32)&ac_rea12_t.addr,0x161);
		SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Tx, ENABLE);//使能DMA发送
		MYDMA_Enable(DMA1_Channel2);//开始一次发送
	}
}

//开入量装填发送函数
void kairuSend(void)
{
	SPI1ReadEvent();
	MYDMA_Config((u32)&SPI1->DR,(u32)&kairu.addr,0x151);
	SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Tx, ENABLE);//使能DMA发送
	MYDMA_Enable(DMA1_Channel2);//开始一次发送
}

//读取校表数据装填
void SPI1ReadCoeff(void)
{
	frm_rd_coeff();
	MYDMA_Config((u32)&SPI1->DR,(u32)&frm_rd_coeff_respose.addr,0xbb);
	SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Tx, ENABLE);//使能DMA发送
	MYDMA_Enable(DMA1_Channel2);//开始一次发送
}


void clear(unsigned char *pta, int size )
{
    while(size>0)
    {
        *pta++ = 0;
        size --;
    }
}
//启动校表按点校表
void StartAdj(u32 Mode, u32 sn)
{
	u8 i;
	if(AT24CXX_Check())//检测不到24c02
	{
		AdjSta = 0x0002;//读eeprom失败
		//ClearAdjustRegister();//清除校表寄存器
		adj_flag=0;
	}
	else
	{
	  if(Mode == 1)//------------------------------------------------点1
	  {
			clear((u8 *)&type_att_para2.w_PhSregApq[0]-3,176);
			ClearAdjustRegister();//清除校表寄存器
			WriteAdjustRegister(0x02,0x0055);
			WriteAdjustRegister(0x70,0xa);
			WriteAdjustRegister(0x1e,0x6c);
		  WriteAdjustRegister(0x60,0xe6);
		  WriteAdjustRegister(0x37,0x326);
	    WriteAdjustRegister(0x1e,0x6c);
			/*for(i=0; i<3; i++)
			{
				WriteAdjustRegister(0x17+i,0);  //清空校表存储器
				WriteAdjustRegister(0x1A+i,0);  //清空校表存储器
				WriteAdjustRegister(0x27+i,0);  //清空校表存储器
				WriteAdjustRegister(0x04+i,0);  //清空校表存储器
				WriteAdjustRegister(0x07+i,0);  //清空校表存储器
			}
			WriteAdjustRegister(0x20,0);  //清空校表存储器*/
			ReadyToAdjust(220, 5, sn);//校表之前参数设置
		  for(i=0; i<3; i++)
		  {
				CompuAdjustUgain(220, i);//电压增益校正//Ugain=0时校准
        CompuAdjustIgain(5, i);//电流增益校正//Igain=0时校准
				CompuAdjustRmsoffse(i);//IaRmsoffse校正
		  }
			delay_ms(3000);
			for(i=0; i<3; i++)
		  {
		  	CompuAdjustPQSgain(1100, i);
		  }
		  EndToAdjust();
	  }
	  else if(Mode == 2)//------------------------------------------------点2
	  {
			/*for(i=0;i<3;i++)
			{
				WriteAdjustRegister(0x0d+i,0);  //清空校表存储器
				WriteAdjustRegister(0x10+i,0);  //清空校表存储器
				WriteAdjustRegister(0x61+i,0);  //清空校表存储器
			}*/
			ReadyToAdjust(220, 5, sn);//校表之前参数设置
		  for(i=0; i<3; i++)
		  {
			  CompuAdjustPhSregpq(i,Mode);//相位矫正//功率增益校正之后//第二个校准点：功率因数为0.5
		  }
		  EndToAdjust();
	  }
	  else if(Mode == 3)//------------------------------------------------点3
	  {
			/*for(i=0;i<3;i++)
			{
				WriteAdjustRegister(0x10+i,0);  //清空校表存储器
				WriteAdjustRegister(0x61+i,0);  //清空校表存储器
			}*/
			ReadyToAdjust(220, 1, sn);//校表之前参数设置
		  for(i=0; i<3; i++)
		  {
			   CompuAdjustPhSregpq(i,Mode);//相位矫正1 2
	  	}
		  EndToAdjust();
	  }
		else if(Mode == 4)//------------------------------------------------点3
	  {
			/*for(i=0;i<3;i++)
			{
				WriteAdjustRegister(0x21+i,0);  //清空校表存储器
				WriteAdjustRegister(0x67+i,0);  //清空校表存储器
			}*/
			ReadyToAdjust(220, 0.25, sn);//校表之前参数设置
		  for(i=0; i<3; i++)
		  {
			   CompuAdjustPoffset(27.5, i);
	  	}
		  EndToAdjust();
	  }
  }
}


//校验接收帧并发送帧
void FramRec(void)
{
	u16 isum, iend, sum = 0;
	u16 i=0,j;
	u8 SPI1_REC_BUF[196];
	while(i<196 && SPI1_REC_BUF_[i]!=0xfe)
	{
		j=SPI1_REC_BUF_[i];
		i++;
	}
	if(i!=196)
	for(j=0;j<196;j++)
	{
		if(i>196)i=0;
		SPI1_REC_BUF[j]=SPI1_REC_BUF_[i];
		SPI1_REC_BUF_[i]=0;
		i++;
	}
	
	i=0;
	
	if(ArrayData(&SPI1_REC_BUF[i], 4) == 0xfefefefe)//如果已收入至少四个字节判断帧头
	  {
			iend = SPI1_REC_BUF[i+9]*256+SPI1_REC_BUF[i+8]+12+i;

		  if(ArrayData(&SPI1_REC_BUF[iend], 4) == 0xfcfcfcfc)//判断帧尾是否到达
			{

				for(isum=i;isum<iend-2;isum++){sum=sum+SPI1_REC_BUF[isum];}//累加校验
				
				if(SPI1_REC_BUF[iend-1]*256+SPI1_REC_BUF[iend-2] == sum) //验证校验和
				{
					sum = 0;
				  RTC_Set(ArrayData_(&SPI1_REC_BUF[i+18], 4));//校对时间
					if(SPI1_REC_BUF[i+5] == 0x03) //读命令
					{
						switch(ArrayData_(&SPI1_REC_BUF[i+6], 2))
						{
							case 0x0e0e:{//读底板信息
							SPI1EmuInfoResp_t();
							break;}
							
							case 0x0100:{//读实时数据1
							SPI1ReadAcreal(0x0100);//组装帧并发送
							//SPI1_REC_BUF[i]=0; i=i+32;
							break;}
						
							case 0x0101:{//读实时数据2
							SPI1ReadAcreal(0x0101);//组装帧并发送
							break;}
						
							case 0x0500:{//读开入量事件
					    kairuSend();
							break;}
						
						  case 0x0200:{//读校表参数
							SPI1ReadCoeff();//组装帧并发送
							break;}
							
							//default: i++;
						}
					}
					else if(SPI1_REC_BUF[i+5] == 0x10)//写命令
					{
						if(ArrayData_(&SPI1_REC_BUF[i+6], 2) == 0x0300)//启动校表
						{
							switch(ArrayData_(&SPI1_REC_BUF[i+26], 4))
							{
								case 0x0001:{adj_m=1;AdjSta =0;adj_flag=1;startadj_respon();break;}
								
								case 0x0002:{adj_m=2;AdjSta =0;adj_flag=1;startadj_respon();break;}
								
								case 0x0003:{adj_m=3;AdjSta =0;adj_flag=1;startadj_respon();break;}
								
								case 0x0004:{adj_m=4;AdjSta =0;adj_flag=1;startadj_respon();break;}
								
								default: {adj_flag =0;}
							}
						}
						
						else if(ArrayData_(&SPI1_REC_BUF[i+6], 2) == 0x0200)//写校表参数
            {
							writeadjdata(i);
							AT24CXX_Write(0,(u8 *)&type_att_para2.w_PhSregApq[0],176);
							writeadj_respon();
							SPI1_REC_BUF[i]=0; i=i+208;
						}
					}
					else return;
				}
				else
				{
					sum = 0;
					return;
					//return 0;//校验和与帧数据不匹配
				}
			}else return;
	  }else return;

}
	
		
void writeadjdata(u16 i)//帧数据data写入校表数据结构体
{
	u8 j;
	for(j=0;j<176;j++)
	{
		*(u8 *)(&type_att_para2.w_PhSregApq[0]+j)=SPI1_REC_BUF_[i+26+j];
	}
}

