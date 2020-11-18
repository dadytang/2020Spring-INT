#ifndef __SPI2_H
#define __SPI2_H
#include "sys.h"

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

#define ATT7022_CS_EN GPIO_ResetBits(GPIOB,GPIO_Pin_12)
#define ATT7022_CS_DIS GPIO_SetBits(GPIOB,GPIO_Pin_12)

//ʱ��ṹ��
typedef struct 
{
	vu8 year;  //1��ʾ2011��
	vu8 month; //1-12
	vu8 day;	//1-31	
	vu8 hour;//0-23
	vu8 minute;//0-59
	vu8 second;//0-59
	vu8 week;
  vu8 state;//����;�ڲ�ͬ��֡���в�ͬ�ĺ���
  vu32 sec;//��2000.1.1 0��0�뿪ʼ������
  vu32 usec;	//΢�� 0��1000000-1
}_ac_time3_t_obj;					 

//���ɰ�������Ϣ
typedef struct
{
	_ac_time3_t_obj ac_time3_tac;
	char Vendor[32];//���̴���
	char SVersion[32];//����汾��
	char HVersion[32];//Ӳ���汾��
	u8 adjustDate[6];//У������
	u16 portSpeedMax;//ͨѶ�ӿ��������
	u16 portSpeedMax1;//ͨѶ�ӿ��������1��������
	u16 frameLenMax;//����֡��󳤶�
	u8 dataTransEnable;//����͸�����ͱ�ʶ
	u8 dataHarmEnable;//�����������ͱ�ʶ
	u8 dataPowerEnable;//�����ܼ����������ͱ�ʶ
	u8 dataWaveEnable;//ԭʼ���������ͱ�ʶ
	u8 devUpdateEnable;//�豸������ʶ
	u8 Backup[15];//Ԥ����չ��
}emuInfo_t_obj;

typedef struct
{
	
	vu32 head;
	vu32 sn;//���������ϵ��0��ʼ
	_ac_time3_t_obj ac_time3_tac;//ʱ���
	vu32 state;//ģ��״̬
	float freq;
	float Vol[4];//ABC�����ѹ
	float Cur[4];//����
	float pwr_p[4];//ABC���й�����
	float pwr_q[4];//�޹�����
	float pwr_s[4];//��������
	float pwr_f[4];//��������
	float angel_vol[3];//ABC��ѹ��λ��
	float angel_cur[3];//������λ��
	float thd_vol[3];//ABC��ѹг��������
	float thd_cur[3];//����г��������
	float har_vol[3][20];//ABC��г����ѹ-����-19��г���ܵ�ѹ
	float har_cur[3][20];//г������
	float p_pos[3];//�����й�
	float p_eng[3];//�����й�
	float q_pos[3];//�����޹�
	float q_eng[3];//�����޹�
	float r_Eq[4][3];//�������޹�
	vu32 diStatus;//di����״̬��λ0��Ӧ����1
	vu32 diEventNum;//mcu�ϵ���������ʱ������λ0��Ӧ����1
	vu32 end;//Ԥ����չ��
	
}ac_real1_obj; //ʵʱ����1�ṹ��	


typedef struct
{
	
	_ac_time3_t_obj ac_time3_tac;//ʱ���
	vu32 r_Ept[4];//�����й���������
	vu32 r_Ept_[4];//�����й���������
	vu32 Eq[4][4];//1�������޹�
	vu32 r_Est[4];//�������ڵ�����������
	vu32 r_Est_[4];//����
	vu32 r_LineEpt[4];//�����й���������
	vu32 r_LineEpt_[4];//����
	vu32 backup[41];//Ԥ����չ
	vu32 xor;//�����������
	
}ac_real2_obj;//ʵʱ����2�ṹ��	

typedef struct
{
	
	vu32 w_PhSregApq[3];//��λУ��0(0d-0f)��2
	vu32 w_Pgain[3];//�й�����(10-12)��1
	vu32 w_Qgain[3];//��1
	vu32 w_Sgain[3];//��1
	vu32 w_Ugain[3];//UgainΪ0ʱУ��
	vu32 w_Igain[3];//IgainΪ0ʱУ��
	vu32 w_GainADC7;//����Ϊ0
	vu32 w_Poffset[3];//����ƫ�� ��3
	vu32 w_PoffsetL[3];//��3
	vu32 w_Qoffset[3];//��3
	vu32 w_QoffsetL[3];//��3
	vu32 w_PhSregAPQ1[3];//��λУ��1(10-12)��2
	vu32 w_PhSregAPQ2[3];//��λУ��2(61-63)��2
	vu32 w_UaRmsoffse[3];//����Ϊ0
	vu32 w_IaRmsoffse[3];//����Ϊ0
	vu32 w_TPSoffset;//25��

}type_att_para2_obj;//У�������ṹ��


#define APhase  0x00
#define BPhase  0x01
#define CPhase  0x02

#define EC 6400


void SPI2_Init(void);			 //��ʼ��SPI��
void SPI2_SetSpeed(u8 SpeedSet); //����SPI�ٶ�   
u8 SPI2_ReadWriteByte(u8 TxData);//SPI���߶�дһ���ֽ�
s32 ReadSampleRegister(u8 adReadAddr);//�������Ĵ���
u32 ReadAdjustRegister(u8 addr);//��У��Ĵ���
void WriteAdjustRegister(u8 addr,u32 data);//дУ������Ĵ���
void ResetATT7022(void);//at7022e�����λ
void ClearAdjustRegister(void);//���У��Ĵ���
void ConfigAdjustRegister(void);//��ʼ��У��Ĵ���
void ReadyToAdjust(float Un, float Ib, u32 sn);//У��֮ǰ��������
void EndToAdjust(void);//У����������
void ZeroAdjustRegister(void);//У���������
s16 ReadSampleRegister2(u8 adReadAddr);

#endif

