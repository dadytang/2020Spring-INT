#ifndef __SPI2_H
#define __SPI2_H
#include "sys.h"

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

#define ATT7022_CS_EN GPIO_ResetBits(GPIOB,GPIO_Pin_12)
#define ATT7022_CS_DIS GPIO_SetBits(GPIOB,GPIO_Pin_12)

//时间结构体
typedef struct 
{
	vu8 year;  //1表示2011年
	vu8 month; //1-12
	vu8 day;	//1-31	
	vu8 hour;//0-23
	vu8 minute;//0-59
	vu8 second;//0-59
	vu8 week;
  vu8 state;//多用途在不同的帧里有不同的含义
  vu32 sec;//从2000.1.1 0分0秒开始的秒数
  vu32 usec;	//微秒 0到1000000-1
}_ac_time3_t_obj;					 

//交采板描述信息
typedef struct
{
	_ac_time3_t_obj ac_time3_tac;
	char Vendor[32];//厂商代码
	char SVersion[32];//软件版本号
	char HVersion[32];//硬件版本号
	u8 adjustDate[6];//校表日期
	u16 portSpeedMax;//通讯接口最大速率
	u16 portSpeedMax1;//通讯接口最大速率1，补齐用
	u16 frameLenMax;//数据帧最大长度
	u8 dataTransEnable;//数据透传上送标识
	u8 dataHarmEnable;//计算数据上送标识
	u8 dataPowerEnable;//电能能计量数据上送标识
	u8 dataWaveEnable;//原始采样点上送标识
	u8 devUpdateEnable;//设备升级标识
	u8 Backup[15];//预留扩展用
}emuInfo_t_obj;

typedef struct
{
	
	vu32 head;
	vu32 sn;//计数器，上电从0开始
	_ac_time3_t_obj ac_time3_tac;//时间戳
	vu32 state;//模块状态
	float freq;
	float Vol[4];//ABC零序电压
	float Cur[4];//电流
	float pwr_p[4];//ABC总有功功率
	float pwr_q[4];//无功功率
	float pwr_s[4];//视在因数
	float pwr_f[4];//功率因数
	float angel_vol[3];//ABC电压相位角
	float angel_cur[3];//电流相位角
	float thd_vol[3];//ABC电压谐波畸变率
	float thd_cur[3];//电流谐波畸变率
	float har_vol[3][20];//ABC总谐波电压-基波-19次谐波总电压
	float har_cur[3][20];//谐波电流
	float p_pos[3];//正向有功
	float p_eng[3];//反向有功
	float q_pos[3];//正向无功
	float q_eng[3];//反向无功
	float r_Eq[4][3];//四象限无功
	vu32 diStatus;//di输入状态，位0对应开关1
	vu32 diEventNum;//mcu上电以来发生时间数，位0对应开关1
	vu32 end;//预留扩展用
	
}ac_real1_obj; //实时数据1结构体	


typedef struct
{
	
	_ac_time3_t_obj ac_time3_tac;//时间戳
	vu32 r_Ept[4];//正向有功脉冲总数
	vu32 r_Ept_[4];//反向有功脉冲总数
	vu32 Eq[4][4];//1象限总无功
	vu32 r_Est[4];//正向视在电能脉冲总数
	vu32 r_Est_[4];//反向
	vu32 r_LineEpt[4];//基波有功电能总数
	vu32 r_LineEpt_[4];//反向
	vu32 backup[41];//预留扩展
	vu32 xor;//以上数据异或
	
}ac_real2_obj;//实时数据2结构体	

typedef struct
{
	
	vu32 w_PhSregApq[3];//相位校正0(0d-0f)点2
	vu32 w_Pgain[3];//有功补偿(10-12)点1
	vu32 w_Qgain[3];//点1
	vu32 w_Sgain[3];//点1
	vu32 w_Ugain[3];//Ugain为0时校正
	vu32 w_Igain[3];//Igain为0时校正
	vu32 w_GainADC7;//输入为0
	vu32 w_Poffset[3];//功率偏移 点3
	vu32 w_PoffsetL[3];//点3
	vu32 w_Qoffset[3];//点3
	vu32 w_QoffsetL[3];//点3
	vu32 w_PhSregAPQ1[3];//相位校正1(10-12)点2
	vu32 w_PhSregAPQ2[3];//相位校正2(61-63)点2
	vu32 w_UaRmsoffse[3];//输入为0
	vu32 w_IaRmsoffse[3];//输入为0
	vu32 w_TPSoffset;//25度

}type_att_para2_obj;//校正参数结构体


#define APhase  0x00
#define BPhase  0x01
#define CPhase  0x02

#define EC 6400


void SPI2_Init(void);			 //初始化SPI口
void SPI2_SetSpeed(u8 SpeedSet); //设置SPI速度   
u8 SPI2_ReadWriteByte(u8 TxData);//SPI总线读写一个字节
s32 ReadSampleRegister(u8 adReadAddr);//读计量寄存器
u32 ReadAdjustRegister(u8 addr);//读校表寄存器
void WriteAdjustRegister(u8 addr,u32 data);//写校表参数寄存器
void ResetATT7022(void);//at7022e软件复位
void ClearAdjustRegister(void);//清除校表寄存器
void ConfigAdjustRegister(void);//初始化校表寄存器
void ReadyToAdjust(float Un, float Ib, u32 sn);//校表之前参数设置
void EndToAdjust(void);//校表后参数设置
void ZeroAdjustRegister(void);//校表参数归零
s16 ReadSampleRegister2(u8 adReadAddr);

#endif

