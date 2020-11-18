#ifndef __COMMO_H
#define __COMMO_H
#include "sys.h"

/*typedef union
{
    float f;
    u32 u;
}float_u32_obj;*/


#define NPT 64

#define N  0x05   //电流有效值计算中的N
#define K  25920000000/(HFconst*EC*0x800000) //功率系数
#define delay_ms rt_thread_mdelay

u32 GetId(void);
//void delay_ms(u32 ms);
void delay_us(u32 us);
u32 ArrayData(u8 *temp, u8 len);
u32 ArrayData_(u8 *temp, u8 len);
void emuInfo(void);//软件信息
void UpdateAc_real1(u32 Times);//更新实时数据ac_real1
void UpdateAc_real2(void);//更新实时数据ac_real2
float harGain(u8 i, u8 j);//谐波增益调整
void Openin(void);//开入量检查
void Openin_write(u8 i, u8 YXout);//开入量变化装载
void CompuHFconst(float Un, float Ib, float Vu, float Vi);
void CompuAdjustPQSgain(float StandValue, u8 PhaseType);//PQS功率增益//第一个校准点：功率因数为1
void CompuAdjustUgain(float StandValue, u8 PhaseType);//电压增益校正//Ugain=0时校准
void CompuAdjustIgain(float StandValue, u8 PhaseType);//电流增益校正//Igain=0时校准

void CompuAdjustPhSregpq(u8 PhaseType, u32 Mode);//相位矫正//功率增益校正之后//第二、三个校准点：功率因数为0.5
void CompuAdjustRmsoffse(u8 PhaseType);//IaRmsoffse校正//1点校正
void CompuAdjustPoffset(float StandValue, u8 PhaseType);//功率offset校准//功率增益校正与相位校正之后//第四个校准点时校准：220V 0.25A 0.5L

#endif


