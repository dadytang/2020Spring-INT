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


void thread_entry_fram(void* parameter)//�߳�1֡У�麯�����
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

void thread_entry_updata(void* parameter)//�߳�2֡��Ӧ�������
{
	u32 ac_real1_sn=0;
	
	while(1)
	{
		if(adj_flag==0)//��־λ0����������״̬
		{
	    ac_real1_sn++;
		  UpdateAc_real1(ac_real1_sn);//����ʵʱ����1
		  UpdateAc_real2();//����ʵʱ����ac_real2
		}
		else if(adj_flag==1) 
	  {  ac_real1_sn++;
		   StartAdj(adj_m, ac_real1_sn);//��־λ1����ʼУ��
		}
	}
}


int main(void)
{
	u32 id;
	
	//uart_init(0x0001C200);//spi1�˿�
	SPI1_Init();
	SPI2_Init();//spi2�˿�
	RTC_Init();//��ʼ��RTC
	AT24CXX_Init();//IIC��ʼ�� 
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	
	ResetATT7022();//at7022e�����λ
	
	id=GetId();
	
	if(AT24CXX_Check())//��ⲻ��24c02
	{
		delay_ms(50);
		AdjSta = 0x0002;//��eepromʧ��
		//ClearAdjustRegister();//���У��Ĵ���
	}
	else 
	{
		/*AT24CXX_Write(0,test,8192);
		AT24CXX_Read(0,test,8192);
		MYDMA_Config((u32)&SPI1->DR,(u32)&test,0x2000);
	  SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Tx, ENABLE);//ʹ��DMA����
	  MYDMA_Enable(DMA1_Channel2);//��ʼһ�η���*/
		
	  AT24CXX_Read(1028,(u8 *)&type_att_para2.w_PhSregApq[0],176);//eeprom���У������
	  //ClearAdjustRegister();//���У��Ĵ���

		delay_ms(50);
	}
	ClearAdjustRegister();//���У��Ĵ���
	WriteAdjustRegister(0x01,0xbd7f);
	WriteAdjustRegister(0x02,0x0055);
	WriteAdjustRegister(0x03,0xfda2);
	WriteAdjustRegister(0x70,0x0000000a);//��Ϊ��������
	WriteAdjustRegister(0x1e,0x6c);
	WriteAdjustRegister(0x60,0xe6);
	WriteAdjustRegister(0x37,0x326);
	ConfigAdjustRegister();//����������ǰ��У��
	delay_ms(50);
	
	dynamic_sem = rt_sem_create("dsem", 0, RT_IPC_FLAG_FIFO);//����һ����̬�ź���
	
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

