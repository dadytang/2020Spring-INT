#ifndef __SPI1WRITEREAD_H
#define __SPI1WRITEREAD_H
#include "sys.h"
#include "spi2.h"
#include "rtc.h" 

#define ditime 100

typedef struct
{
	vu8 buqi;
	vu8 reslogo;
	vu8 addr;
	vu8 comcode;
	vu8 datalogo;
	vu8 datalogo1;
	vu8 length;
	vu8 length1;
	emuInfo_t_obj emuInfo_t;
	vu8 sum0;
	vu8 sum1;
	vu8 taillogo;
	vu8 buqi1;
}EmuInfoResp_t_obj;//交采板描述信息发送帧

typedef struct
{
	vu8 buqi;
	vu8 reslogo;
	vu8 addr;
	vu8 comcode;
	vu8 datalogo;
	vu8 datalogo1;
	vu8 length;
	vu8 length1;
	ac_real1_obj ac_real1;
	vu8 sum0;
	vu8 sum1;
	vu8 taillogo;
	vu8 buqi1;
}ac_real1_t_obj;//实时数据发送帧1

typedef struct
{
	vu8 buqi;
	vu8 reslogo;
	vu8 addr;
	vu8 comcode;
	vu8 datalogo;
	vu8 datalogo1;
	vu8 length;
	vu8 length1;
	ac_real2_obj ac_real2;
	vu8 sum0;
	vu8 sum1;
	vu8 taillogo;
	vu8 buqi1;
}ac_real2_t_obj;//实时数据发送帧2

typedef struct
{
	vu8 buqi;
	vu8 reslogo;
	vu8 addr;
	vu8 comcode;
	vu8 datalogo;
	vu8 datalogo1;
	vu8 length;
	vu8 length1;
	u32 rdptr;
	u32 num;
	_ac_time3_t_obj tm[20];
	vu8 sum0;
	vu8 sum1;
	vu8 taillogo;
	vu8 buqi1;
}kairu_obj;//开入量响应帧

typedef struct
{
	vu8 reslogo;
	vu8 addr;
	vu8 comcode;
	vu8 datalogo;
	vu8 datalogo1;
	vu8 length;
	vu8 length1;
	vu8 sta;
	vu8 sta1;
	vu8 sum0;
	vu8 sum1;
	vu8 taillogo;
}frm_start_adj_response_t_obj;//开始校表响应帧

typedef struct
{
	vu8 buqi;
	vu8 buqi1;
	vu8 buqi2;
	vu8 reslogo;
	vu8 addr;
	vu8 comcode;
	vu8 datalogo;
	vu8 datalogo1;
	vu8 length;
	vu8 length1;
	vu8 sta;
	vu8 sta1;
	type_att_para2_obj type_att_para2;
	vu8 sum0;
	vu8 sum1;
	vu8 taillogo;
	vu8 buqi3;
}frm_rd_coeff_respose_obj;//读校表参数响应帧


void SPI1ReadAcreal(u16 ac_real);
void SPI1ReadCoeff(void);
void SPI1ReadEvent(void);
void kairuSend(void);
void startadj_respon(void);
void StartAdj(u32 Mode, u32 sn);
void writeadj_respon(void);
void writeadjdata(u16 i);

void FramRec(void);





#endif

