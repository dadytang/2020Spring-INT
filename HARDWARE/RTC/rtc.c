#include "sys.h"
#include "rtc.h" 	
#include "spi2.h"
#include "SPI1WriteRead.h"
#include "rtthread.h"
#include "commo.h"

u8 const mon_table[12];	//�·��������ݱ�
extern _ac_time3_t_obj ac_time3_t;	//�����ṹ��
extern ac_real1_obj ac_real1;
extern ac_real2_obj ac_real2;

//Mini STM32������
//RTCʵʱʱ�� ��������			 
//����ԭ��@ALIENTEK
//2010/6/6
 
//ʵʱʱ������
//��ʼ��RTCʱ��,ͬʱ���ʱ���Ƿ�������
//BKP->DR1���ڱ����Ƿ��һ�����õ�����
//����0:����
//����:�������

u8 RTC_Init(void)
{
	//����ǲ��ǵ�һ������ʱ��
	u8 temp=0;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);	//ʹ��PWR��BKP����ʱ��   
	PWR_BackupAccessCmd(ENABLE);	//ʹ�ܺ󱸼Ĵ�������  			
  BKP_DeInit();	//��λ�������� 	
	RCC_HSEConfig(RCC_HSE_ON);	//�����ⲿ���پ���(LSE),ʹ��������پ���
	while (RCC_GetFlagStatus(RCC_FLAG_HSERDY) == RESET&&temp<250)	//���ָ����RCC��־λ�������,�ȴ����پ������
		{
		temp++;
		delay_ms(10);
		}
	if(temp>=250)return 1;//��ʼ��ʱ��ʧ��,����������	    
	RCC_RTCCLKConfig(RCC_RTCCLKSource_HSE_Div128);		//����RTCʱ��(RTCCLK),ѡ��LSE��ΪRTCʱ��    
	RCC_RTCCLKCmd(ENABLE);	//ʹ��RTCʱ��  
	RTC_WaitForLastTask();	//�ȴ����һ�ζ�RTC�Ĵ�����д�������
	RTC_WaitForSynchro();		//�ȴ�RTC�Ĵ���ͬ��  
	RTC_WaitForLastTask();	//�ȴ����һ�ζ�RTC�Ĵ�����д�������
	RTC_EnterConfigMode();/// ��������	
	RTC_SetPrescaler(62500); //����RTCԤ��Ƶ��ֵ
	RTC_WaitForLastTask();	//�ȴ����һ�ζ�RTC�Ĵ�����д�������
	RTC_Set(0X0001);  //����ʱ��	
	RTC_ExitConfigMode(); //�˳�����ģʽ  
			     
	//RTC_Get();//����ʱ��	
	return 0; //ok

}		 				    

//�ж��Ƿ������꺯��
//�·�   1  2  3  4  5  6  7  8  9  10 11 12
//����   31 29 31 30 31 30 31 31 30 31 30 31
//������ 31 28 31 30 31 30 31 31 30 31 30 31
//����:���
//���:������ǲ�������.1,��.0,����
u8 Is_Leap_Year(u8 syear)
{	
  u16 year = syear + 2000;	
	if(year%4==0) //�����ܱ�4����
	{ 
		if(year%100==0) 
		{ 
			if(year%400==0)return 1;//�����00��β,��Ҫ�ܱ�400���� 	   
			else return 0;   
		}else return 1;   
	}else return 0;	
}	 			   
//����ʱ��
//����2000.1.1���������									 
u8 const table_week[12]={0,3,3,6,1,4,6,2,5,0,3,5}; //���������ݱ�	  
//ƽ����·����ڱ�
const u8 mon_table[12]={31,28,31,30,31,30,31,31,30,31,30,31};
u8 RTC_Set(u32 seccount)//(u8 syear,u8 smon,u8 sday,u8 hour,u8 min,u8 sec)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);	//ʹ��PWR��BKP����ʱ��  
	PWR_BackupAccessCmd(ENABLE);	//ʹ��RTC�ͺ󱸼Ĵ������� 
  
	RTC_SetCounter(seccount);	//����RTC��������ֵ
	
	RTC_WaitForLastTask();	//�ȴ����һ�ζ�RTC�Ĵ�����д�������  	
	return 0;	    
}



//�õ���ǰ��ʱ��
//����ֵ:0,�ɹ�;����:�������.
_ac_time3_t_obj RTC_Get(u8 type)
{
	static u16 daycnt=0;
	u32 timecount=0; 
	u32 temp=0;
	u16 temp1=0;	  
  timecount=RTC_GetCounter();	
  ac_time3_t.sec = timecount;	
 	temp=timecount/86400;   //�õ�����(��������Ӧ��)
	if(daycnt!=temp)//����һ����
	{	  
		daycnt=temp;
		temp1=2000;	//��2000�꿪ʼ
		while(temp>=365)
		{				 
			if(Is_Leap_Year(temp1))//������
			{
				if(temp>=366)temp-=366;//�����������
				else {temp1++;break;}  
			}
			else temp-=365;	  //ƽ�� 
			temp1++;  
		}   
		ac_time3_t.year=temp1-2000;//�õ����
		temp1=0;
		while(temp>=28)//������һ����
		{
			if(Is_Leap_Year(ac_time3_t.year)&&temp1==1)//�����ǲ�������/2�·�
			{
				if(temp>=29)temp-=29;//�����������
				else break; 
			}
			else 
			{
				if(temp>=mon_table[temp1])temp-=mon_table[temp1];//ƽ��
				else break;
			}
			temp1++;  
		}
		ac_time3_t.month=temp1+1;	//�õ��·�
		ac_time3_t.day=temp+1;  	//�õ����� 
	}
	temp=timecount%86400;     		//�õ�������   	   
	ac_time3_t.hour=temp/3600;     	//Сʱ
	ac_time3_t.minute=(temp%3600)/60; 	//����	
	ac_time3_t.second=(temp%3600)%60; 	//����
	ac_time3_t.week=RTC_Get_Week(ac_time3_t.year,ac_time3_t.month,ac_time3_t.day);//��ȡ����
  ac_time3_t.usec=(62500-RTC_GetDivider())*1000000/62500;//����	
	//if(type)ac_rea11_t.ac_time3_tfra = ac_time3_t; //���뿪���¼�ʱ���
	//else 
	if(type)ac_real1.ac_time3_tac = ac_time3_t;
	else if(type==2)ac_real2.ac_time3_tac = ac_time3_t;
	else if(type==3)
	return ac_time3_t;
}	 
//������������ڼ�
//��������:���빫�����ڵõ�����(ֻ����1901-2099��)
//������������������� 
//����ֵ�����ں�																						 
u8 RTC_Get_Week(u8 syear,u8 month,u8 day)
{	
	u16 temp2;
	u8 yearH,yearL;
	u16 year = syear+2000;
	yearH=year/100;	yearL=year%100; 
	// ���Ϊ21����,�������100  
	if (yearH>19)yearL+=100;
	// ����������ֻ��1900��֮���  
	temp2=yearL+yearL/4;
	temp2=temp2%7; 
	temp2=temp2+day+table_week[month-1];
	if (yearL%4==0&&month<3)temp2--;
	return(temp2%7);

}			  
















