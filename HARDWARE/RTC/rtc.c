#include "sys.h"
#include "rtc.h" 	
#include "spi2.h"
#include "SPI1WriteRead.h"
#include "rtthread.h"
#include "commo.h"

u8 const mon_table[12];	//月份日期数据表
extern _ac_time3_t_obj ac_time3_t;	//日历结构体
extern ac_real1_obj ac_real1;
extern ac_real2_obj ac_real2;

//Mini STM32开发板
//RTC实时时钟 驱动代码			 
//正点原子@ALIENTEK
//2010/6/6
 
//实时时钟配置
//初始化RTC时钟,同时检测时钟是否工作正常
//BKP->DR1用于保存是否第一次配置的设置
//返回0:正常
//其他:错误代码

u8 RTC_Init(void)
{
	//检查是不是第一次配置时钟
	u8 temp=0;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);	//使能PWR和BKP外设时钟   
	PWR_BackupAccessCmd(ENABLE);	//使能后备寄存器访问  			
  BKP_DeInit();	//复位备份区域 	
	RCC_HSEConfig(RCC_HSE_ON);	//设置外部低速晶振(LSE),使用外设低速晶振
	while (RCC_GetFlagStatus(RCC_FLAG_HSERDY) == RESET&&temp<250)	//检查指定的RCC标志位设置与否,等待低速晶振就绪
		{
		temp++;
		delay_ms(10);
		}
	if(temp>=250)return 1;//初始化时钟失败,晶振有问题	    
	RCC_RTCCLKConfig(RCC_RTCCLKSource_HSE_Div128);		//设置RTC时钟(RTCCLK),选择LSE作为RTC时钟    
	RCC_RTCCLKCmd(ENABLE);	//使能RTC时钟  
	RTC_WaitForLastTask();	//等待最近一次对RTC寄存器的写操作完成
	RTC_WaitForSynchro();		//等待RTC寄存器同步  
	RTC_WaitForLastTask();	//等待最近一次对RTC寄存器的写操作完成
	RTC_EnterConfigMode();/// 允许配置	
	RTC_SetPrescaler(62500); //设置RTC预分频的值
	RTC_WaitForLastTask();	//等待最近一次对RTC寄存器的写操作完成
	RTC_Set(0X0001);  //设置时间	
	RTC_ExitConfigMode(); //退出配置模式  
			     
	//RTC_Get();//更新时间	
	return 0; //ok

}		 				    

//判断是否是闰年函数
//月份   1  2  3  4  5  6  7  8  9  10 11 12
//闰年   31 29 31 30 31 30 31 31 30 31 30 31
//非闰年 31 28 31 30 31 30 31 31 30 31 30 31
//输入:年份
//输出:该年份是不是闰年.1,是.0,不是
u8 Is_Leap_Year(u8 syear)
{	
  u16 year = syear + 2000;	
	if(year%4==0) //必须能被4整除
	{ 
		if(year%100==0) 
		{ 
			if(year%400==0)return 1;//如果以00结尾,还要能被400整除 	   
			else return 0;   
		}else return 1;   
	}else return 0;	
}	 			   
//设置时钟
//输入2000.1.1至今的秒数									 
u8 const table_week[12]={0,3,3,6,1,4,6,2,5,0,3,5}; //月修正数据表	  
//平年的月份日期表
const u8 mon_table[12]={31,28,31,30,31,30,31,31,30,31,30,31};
u8 RTC_Set(u32 seccount)//(u8 syear,u8 smon,u8 sday,u8 hour,u8 min,u8 sec)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);	//使能PWR和BKP外设时钟  
	PWR_BackupAccessCmd(ENABLE);	//使能RTC和后备寄存器访问 
  
	RTC_SetCounter(seccount);	//设置RTC计数器的值
	
	RTC_WaitForLastTask();	//等待最近一次对RTC寄存器的写操作完成  	
	return 0;	    
}



//得到当前的时间
//返回值:0,成功;其他:错误代码.
_ac_time3_t_obj RTC_Get(u8 type)
{
	static u16 daycnt=0;
	u32 timecount=0; 
	u32 temp=0;
	u16 temp1=0;	  
  timecount=RTC_GetCounter();	
  ac_time3_t.sec = timecount;	
 	temp=timecount/86400;   //得到天数(秒钟数对应的)
	if(daycnt!=temp)//超过一天了
	{	  
		daycnt=temp;
		temp1=2000;	//从2000年开始
		while(temp>=365)
		{				 
			if(Is_Leap_Year(temp1))//是闰年
			{
				if(temp>=366)temp-=366;//闰年的秒钟数
				else {temp1++;break;}  
			}
			else temp-=365;	  //平年 
			temp1++;  
		}   
		ac_time3_t.year=temp1-2000;//得到年份
		temp1=0;
		while(temp>=28)//超过了一个月
		{
			if(Is_Leap_Year(ac_time3_t.year)&&temp1==1)//当年是不是闰年/2月份
			{
				if(temp>=29)temp-=29;//闰年的秒钟数
				else break; 
			}
			else 
			{
				if(temp>=mon_table[temp1])temp-=mon_table[temp1];//平年
				else break;
			}
			temp1++;  
		}
		ac_time3_t.month=temp1+1;	//得到月份
		ac_time3_t.day=temp+1;  	//得到日期 
	}
	temp=timecount%86400;     		//得到秒钟数   	   
	ac_time3_t.hour=temp/3600;     	//小时
	ac_time3_t.minute=(temp%3600)/60; 	//分钟	
	ac_time3_t.second=(temp%3600)%60; 	//秒钟
	ac_time3_t.week=RTC_Get_Week(ac_time3_t.year,ac_time3_t.month,ac_time3_t.day);//获取星期
  ac_time3_t.usec=(62500-RTC_GetDivider())*1000000/62500;//毫秒	
	//if(type)ac_rea11_t.ac_time3_tfra = ac_time3_t; //开入开出事件时间戳
	//else 
	if(type)ac_real1.ac_time3_tac = ac_time3_t;
	else if(type==2)ac_real2.ac_time3_tac = ac_time3_t;
	else if(type==3)
	return ac_time3_t;
}	 
//获得现在是星期几
//功能描述:输入公历日期得到星期(只允许1901-2099年)
//输入参数：公历年月日 
//返回值：星期号																						 
u8 RTC_Get_Week(u8 syear,u8 month,u8 day)
{	
	u16 temp2;
	u8 yearH,yearL;
	u16 year = syear+2000;
	yearH=year/100;	yearL=year%100; 
	// 如果为21世纪,年份数加100  
	if (yearH>19)yearL+=100;
	// 所过闰年数只算1900年之后的  
	temp2=yearL+yearL/4;
	temp2=temp2%7; 
	temp2=temp2+day+table_week[month-1];
	if (yearL%4==0&&month<3)temp2--;
	return(temp2%7);

}			  
















