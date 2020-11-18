#ifndef __COMMO_H
#define __COMMO_H
#include "sys.h"

/*typedef union
{
    float f;
    u32 u;
}float_u32_obj;*/


#define NPT 64

#define N  0x05   //������Чֵ�����е�N
#define K  25920000000/(HFconst*EC*0x800000) //����ϵ��
#define delay_ms rt_thread_mdelay

u32 GetId(void);
//void delay_ms(u32 ms);
void delay_us(u32 us);
u32 ArrayData(u8 *temp, u8 len);
u32 ArrayData_(u8 *temp, u8 len);
void emuInfo(void);//�����Ϣ
void UpdateAc_real1(u32 Times);//����ʵʱ����ac_real1
void UpdateAc_real2(void);//����ʵʱ����ac_real2
float harGain(u8 i, u8 j);//г���������
void Openin(void);//���������
void Openin_write(u8 i, u8 YXout);//�������仯װ��
void CompuHFconst(float Un, float Ib, float Vu, float Vi);
void CompuAdjustPQSgain(float StandValue, u8 PhaseType);//PQS��������//��һ��У׼�㣺��������Ϊ1
void CompuAdjustUgain(float StandValue, u8 PhaseType);//��ѹ����У��//Ugain=0ʱУ׼
void CompuAdjustIgain(float StandValue, u8 PhaseType);//��������У��//Igain=0ʱУ׼

void CompuAdjustPhSregpq(u8 PhaseType, u32 Mode);//��λ����//��������У��֮��//�ڶ�������У׼�㣺��������Ϊ0.5
void CompuAdjustRmsoffse(u8 PhaseType);//IaRmsoffseУ��//1��У��
void CompuAdjustPoffset(float StandValue, u8 PhaseType);//����offsetУ׼//��������У������λУ��֮��//���ĸ�У׼��ʱУ׼��220V 0.25A 0.5L

#endif


