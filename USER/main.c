#include "stm32f10x.h"
#include "spi2.h"
#include "spi1.h"
#include "rtc.h"
#include "commo.h"
#include "24cxx.h"
#include "SPI1WriteRead.h"
#include "rtthread.h"
#include "dma.h"

#define THREAD_STACK_SIZE 128
#define THREAD_PRIORITY 2
#define THREAD_TIMESLICE 10

u8 adj_flag=0;
rt_thread_t tid1 = RT_NULL;
rt_thread_t tid2 = RT_NULL;
rt_sem_t dynamic_sem = RT_NULL;
extern u32 AdjSta;
extern u32 adj_m;
extern type_att_para2_obj type_att_para2;
u8 test[8192];

void thread_entry_fram(void* parameter);
void thread_entry_updata(void* parameter);


void thread_entry_fram(void* parameter)//线程1帧校验函数入口
{
	static rt_err_t result;
	while(1)
		{
			result = rt_sem_take(dynamic_sem, RT_WAITING_FOREVER);
			if (result == RT_EOK)
	    FramRec();
	    //i=rt_thread_control(tid2,RT_THREAD_CTRL_CHANGE_PRIORITY,(void*)1);
			//rt_thread_suspend(tid1);
			//rt_schedule();
	  }
}

void thread_entry_updata(void* parameter)//线程2帧响应函数入口
{
	u32 ac_real1_sn=0;
	
	while(1)
	{
		if(adj_flag==0)//标志位0，正常工作状态
		{
	    ac_real1_sn++;
		  UpdateAc_real1(ac_real1_sn);//更新实时数据1
		  UpdateAc_real2();//更新实时数据ac_real2
		}
		else if(adj_flag==1) 
	  {  ac_real1_sn++;
		   StartAdj(adj_m, ac_real1_sn);//标志位1，开始校表
		}
	}
}


int main(void)
{
	u32 id;
	
	//uart_init(0x0001C200);//spi1端口
	SPI1_Init();
	SPI2_Init();//spi2端口
	RTC_Init();//初始化RTC
	AT24CXX_Init();//IIC初始化 
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	
	ResetATT7022();//at7022e软件复位
	
	id=GetId();
	
	if(AT24CXX_Check())//检测不到24c02
	{
		delay_ms(50);
		AdjSta = 0x0002;//读eeprom失败
		//ClearAdjustRegister();//清除校表寄存器
	}
	else 
	{
		/*AT24CXX_Write(0,test,8192);
		AT24CXX_Read(0,test,8192);
		MYDMA_Config((u32)&SPI1->DR,(u32)&test,0x2000);
	  SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Tx, ENABLE);//使能DMA发送
	  MYDMA_Enable(DMA1_Channel2);//开始一次发送*/
		
	  AT24CXX_Read(1028,(u8 *)&type_att_para2.w_PhSregApq[0],176);//eeprom里读校表数据
	  //ClearAdjustRegister();//清除校表寄存器

		delay_ms(50);
	}
	ClearAdjustRegister();//清除校表寄存器
	WriteAdjustRegister(0x01,0xbd7f);
	WriteAdjustRegister(0x02,0x0055);
	WriteAdjustRegister(0x03,0xfda2);
	WriteAdjustRegister(0x70,0x0000000a);//改为基波测量
	WriteAdjustRegister(0x1e,0x6c);
	WriteAdjustRegister(0x60,0xe6);
	WriteAdjustRegister(0x37,0x326);
	ConfigAdjustRegister();//读计量数据前先校表
	delay_ms(50);
	
	dynamic_sem = rt_sem_create("dsem", 0, RT_IPC_FLAG_FIFO);//创建一个动态信号量
	
	tid1 = rt_thread_create("thread1", 
	                       thread_entry_fram,
                      	 (void*)1,
												 THREAD_STACK_SIZE,
												 1, 
												 THREAD_TIMESLICE-9);
  if (tid1 != RT_NULL)
	rt_thread_startup(tid1);
	
	tid2 = rt_thread_create("thread2", 
												  thread_entry_updata, 
												  (void*)2, 
													THREAD_STACK_SIZE*4,
													THREAD_PRIORITY, 
													THREAD_TIMESLICE);
  if (tid2 != RT_NULL)
  rt_thread_startup(tid2);
	
	return 0;
}

