#ifndef __RTC_H
#define __RTC_H	    
#include "sys.h"
#include "spi2.h"
//Mini STM32������
//RTCʵʱʱ�� ��������			 
//����ԭ��@ALIENTEK
//2010/6/6

#define ac1_t 1
#define ac2_t 2
#define kr_t 3


u8 RTC_Init(void);        //��ʼ��RTC,����0,ʧ��;1,�ɹ�;
u8 Is_Leap_Year(u8 syear);//ƽ��,�����ж�
_ac_time3_t_obj RTC_Get(u8 type);         //����ʱ��   
u8 RTC_Get_Week(u8 syear,u8 month,u8 day);
u8 RTC_Set(u32 seccount);//(u16 syear,u8 smon,u8 sday,u8 hour,u8 min,u8 sec);//����ʱ��	
u8 RTC_Set1(u32 seccount);
#endif


