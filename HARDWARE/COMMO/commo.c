#include "commo.h"
#include "spi2.h"
#include "rtc.h"
#include "stm32_dsp.h"
#include "table_fft.h"
#include "rtthread.h"
#include "rthw.h"
#include "openin.h"
#include "SPI1WriteRead.h"

#define _SCB_BASE       (0xE000E010UL)
#define _SYSTICK_LOAD   (*(u32 *)(_SCB_BASE + 0x4))
#define _SYSTICK_VAL    (*(u32 *)(_SCB_BASE + 0x8))

u8 krcnt=0;
float HFconst;
u32 AdjSta,krnum=0;
int32_t adc_buf[7][NPT];
long lBufInArray[7][NPT];
long lBufOutArray[6][NPT];
float lBufMagArray[6][NPT/2];
_ac_time3_t_obj tm[20];
emuInfo_t_obj emuInfo_t;
extern ac_real1_obj ac_real1;
extern ac_real2_obj ac_real2;
extern type_att_para2_obj type_att_para2;
extern u8 YXout[4], YXout_[4], YXin[4];

u32 GetId(void)
{
	u32 id;
	id=ReadSampleRegister(0x00);
	return id;
}

/*void delay_ms(u32 ms)
{
  u16 i;
  while(ms--)
  {
   i=7200;
   while(i--);
  }
}*/
void delay_us(u32 us)//Î¢ÃëÑÓÊ±
{
	 u32 delta;

   us = us * (_SYSTICK_LOAD /(1000000/1000));

   delta = _SYSTICK_VAL;

   while (delta - _SYSTICK_VAL< us);

}
//=============¿ª·½ÔËËã===================================
float SquareRootFloat(float number)
{
    long i;
    float x, y;
    const float f = 1.5F;

    x = number * 0.5F;
    y  = number;
    i  = * ( long * ) &y;
    i  = 0x5f3759df - ( i >> 1 );  //???
  //i  = 0x5f375a86 - ( i >> 1 );  //Lomont
    y  = * ( float * ) &i;
    y  = y * ( f - ( x * y * y ) );
    y  = y * ( f - ( x * y * y ) );
    return number * y;
}
//===============ÕûºÏÊý×é==================================
u32 ArrayData(u8 *temp, u8 len)
{
 	u32 retData;
	u8 i;

	retData=0;
	for(i=0;i<len;i++)
	{
		retData=retData<<8;
		retData=retData+*(temp+i);
	}
	return retData;
}
u32 ArrayData_(u8 *temp, u8 len)
{
	u32 retData;
	u8 i;

	retData=0;
	for(i=len+1;i>1;i--)
	{
		retData=retData<<8;
		retData=retData+*(temp+i-2);
	}
	return retData;
}
//===============Ô­Âë×ª»»³É²¹Âë==============================
void DToBm(float value_, u8 *buff,u8 num)
{
	  u32 value;
	  value = (u32)value_;
    if(value>0)
  	{
  		if(num==3)
  			value=value&0x007fffff;
  		if(num==2)
  			value=value&0x00007fff;
  		if(num==1)
  			value=value&0x0000007f;
  	}
  	else if(value<=0)
  	{
   		if(num==3)
   			value=value|0x800000;
   		if(num==2)
   			value=value|0x8000;
   		if(num==1)
   			value=value|0x80;
  	}
  
  	*(buff+2)=(unsigned char)(value>>16);
  	*(buff+1)=(unsigned char)(value>>8);
  	*buff=(unsigned char)value;
}
u32 adjudata(signed short data)
{
	u32 value;
	value = ((u32)data)&0x0000ffff;
	return value;
}
long BmToYm(long value)
{
    if((value|0xff7fffff)==0xff7fffff);
    else if((value&0x00800000)==0x00800000)
        value=(~value+1)&0x007fffff;
    
	return value;   
}
//========================================================//Èí¼þ°æ±¾ÐÅÏ¢
void emuInfo(void)
{
	u8 i;
	char a[16] = "SV01.101-P01.003";
	char b[16] = "HV01.002-AT7022E";
	for(i=0; i<16; i++) emuInfo_t.SVersion[i]=a[i];
	for(i=0; i<16; i++) emuInfo_t.HVersion[i]=b[i];
}
//========================================================//ac_real1
//¼ÆÁ¿ÆµÂÊ²¢´æ´¢
void CompuFreq(void)
{
	float Freq;
	Freq = (float)ReadSampleRegister(0x1C);
	ac_real1.freq = Freq/(0x2000);
}
//¼ÆÁ¿µçÑ¹ÓÐÐ§Öµ//µçÑ¹Ð³²¨»û±äÂÊ
void CompuVol(void)
{
	//float Vol, Voln;
	u8 i;
	
	for(i=0; i<3; i++)
	{
	  //Vol = (float)ReadSampleRegister(0x0D+i);
	  ac_real1.Vol[i] = (float)ReadSampleRegister(0x0D+i)/(0x2000);
		//Voln = (float)ReadSampleRegister(0x48+i);
		ac_real1.har_vol[i][1] = (float)ReadSampleRegister(0x48+i)/(0x2000);
		ac_real1.thd_vol[i] = SquareRootFloat(ac_real1.Vol[i]* ac_real1.Vol[i]-ac_real1.har_vol[i][1]*ac_real1.har_vol[i][1])/ac_real1.har_vol[i][1];
    ac_real1.har_vol[i][0]=ac_real1.thd_vol[i]*ac_real1.har_vol[i][1];
	}
	
	//Vol = (float)ReadSampleRegister(0x2B);
	ac_real1.Vol[3] = (float)ReadSampleRegister(0x2B)/(0x1000);
}
//¼ÆÁ¿µçÁ÷ÓÐÐ§Öµ//µçÁ÷Ð³²¨»û±äÂÊ
void ComputCur(void)
{
	//float Cur;//, Curn;
	u8 i;
	
	for(i=0; i<3; i++)
	{
	  //Cur = (float)ReadSampleRegister(0x10+i);
	  ac_real1.Cur[i] = (float)ReadSampleRegister(0x10+i)/(0x2000)/N;
		//Curn = (float)ReadSampleRegister(0x4B+i);
		ac_real1.har_cur[i][1] = (float)ReadSampleRegister(0x4B+i)/(0x2000)/N;
		ac_real1.thd_cur[i] = SquareRootFloat(ac_real1.Cur[i]* ac_real1.Cur[i]-ac_real1.har_cur[i][1]*ac_real1.har_cur[i][1])/ac_real1.har_cur[i][1];
		ac_real1.har_cur[i][0] =ac_real1.thd_cur[i]*ac_real1.har_cur[i][1];
	}
	
	//Cur = (float)ReadSampleRegister(0x13);
	ac_real1.Cur[3] = (float)ReadSampleRegister(0x13)/(0x1000)/N;
}
//ÓÐ¹¦¹¦ÂÊ
void Compupwr_p(void)
{
	u8 i;
	float pwr_p;
  HFconst = 0x6c;
	for(i=0; i<3; i++)
	{
		pwr_p = (float)ReadSampleRegister(0x01+i);
		if(pwr_p > 0x800000)
		{
			pwr_p = pwr_p - 0x1000000;
			ac_real1.pwr_p[i] = pwr_p*K;
		}
		else
		{
			ac_real1.pwr_p[i] = pwr_p*K;
		}
	}
	pwr_p = (float)ReadSampleRegister(0x04);
	if(pwr_p > 0x800000)
	{
		pwr_p = pwr_p - 0x1000000;
		ac_real1.pwr_p[i] = pwr_p*K*2;
	}
	else
	{
		ac_real1.pwr_p[i] = pwr_p*K*2;
	}
}
//ÎÞ¹¦¹¦ÂÊ
void Compupwr_q(void)
{
	u8 i;
	float pwr_q;
	for(i=0; i<3; i++)
	{
		pwr_q = (float)ReadSampleRegister(0x05+i);
		if(pwr_q > 0x800000)
		{
			pwr_q = pwr_q - 0x1000000;
			ac_real1.pwr_q[i] = pwr_q*K;
		}
		else
		{
			ac_real1.pwr_q[i] = pwr_q*K;
		}
	}
	pwr_q = (float)ReadSampleRegister(0x08);
	if(pwr_q > 0x800000)
	{
		pwr_q = pwr_q - 0x1000000;
		ac_real1.pwr_q[i] =pwr_q*K*2;
	}
	else
	{
		ac_real1.pwr_q[i] = pwr_q*K*2;
	}
}
//ÊÓÔÚ¹¦ÂÊ
void Compupwr_s(void)
{
	u8 i;
	float pwr_s;
	for(i=0; i<3; i++)
	{
		pwr_s = (float)ReadSampleRegister(0x09+i);
		if(pwr_s> 0x800000)
		{
			pwr_s = pwr_s - 0x1000000;
			ac_real1.pwr_s[i] = pwr_s*K;
		}
		else
		{
			ac_real1.pwr_s[i] = pwr_s*K;
		}
	}
	pwr_s = (float)ReadSampleRegister(0x0C);
	if(pwr_s > 0x800000)
	{
		pwr_s = pwr_s - 0x1000000;
		ac_real1.pwr_s[i] = pwr_s*K*2;
	}
	else
	{
		ac_real1.pwr_s[i] = pwr_s*K*2;
	}
}
//¹¦ÂÊÒòÊý
void Compupwr_f(void)
{
	u8 i;
	float pwr_f;
	for(i=0; i<4; i++)
	{
		pwr_f = (float)ReadSampleRegister(0x14+i);
		if(pwr_f > 0x800000)
		{
			pwr_f = pwr_f - 0x1000000;
			ac_real1.pwr_f[i] = pwr_f/0x800000;
		}
		else
		{
			ac_real1.pwr_f[i] = pwr_f/0x800000;
		}
	}
}
//µçÁ÷ÏàÎ»
void Compuangel_cur(void)
{
	u8 i;
	float angel_cur;
	for(i=0; i<3; i++)
	{
		angel_cur = (float)ReadSampleRegister(0x18+i);
		if(angel_cur > 0x100000)
		{
			angel_cur = angel_cur - 0x1000000;
			ac_real1.angel_cur[i] = 360+(angel_cur/0x100000)*180;
		}
		else
		{
			ac_real1.angel_cur[i] = (angel_cur/0x100000)*180;
		}
	}
}
//µçÑ¹ÏàÎ»
void Compuangel_vol(void)
{
	u8 i;
	float angel_vol;
	for(i=0; i<3; i++)
	{
		angel_vol = (float)BmToYm(ReadSampleRegister(0x26+i));
		
		ac_real1.angel_vol[i] = (angel_vol/0x100000)*180;

	}
}
//¼ÆËã¸÷´ÎÐ³²¨
void GetPowerMag()//¼ÆËã¸÷´ÎÐ³²¨·ùÖµ
{
    signed short lX,lY;
    float X,Y,Mag;
    unsigned short i, j;
	for(j=0; j<6; j++){
    for(i=0; i<NPT/2; i++)
    {
        lX  = (lBufOutArray[j][i] << 16) >> 16;
        lY  = (lBufOutArray[j][i] >> 16);
        
        X = (float)lX;
        Y = (float)lY;
        Mag = SquareRootFloat(X * X + Y * Y);
        lBufMagArray[j][i] =Mag;
    }
	}
}
void CompuPowerMag(void)//¶ÁÈ¡»º³å¼Ä´æÆ÷²¢×ª»»
{
	float test;
		u8 j, i;
	ATT7022_CS_EN;	
	SPI2_ReadWriteByte(0xC9); 
	SPI2_ReadWriteByte(0);
	SPI2_ReadWriteByte(0);
	SPI2_ReadWriteByte(0x5A);//Ê¹ÄÜ½Ï±í¼Ä´æÆ÷Ð´²Ù×÷
	ATT7022_CS_DIS;
	delay_us(5);
  ATT7022_CS_EN;
	SPI2_ReadWriteByte(0xC5); 
	SPI2_ReadWriteByte(0);
	SPI2_ReadWriteByte(0);
	SPI2_ReadWriteByte(0x00);//Í£Ö¹Í¬²½Êý¾Ý
	ATT7022_CS_DIS;
	delay_us(50);
	ATT7022_CS_EN;
	SPI2_ReadWriteByte(0xC5); 
	SPI2_ReadWriteByte(0);
	SPI2_ReadWriteByte(0);
	SPI2_ReadWriteByte(0x02);//¿ªÆôÍ¬²½Êý¾Ý
	ATT7022_CS_DIS;
	delay_ms(50);
	while(ReadSampleRegister2(0x7e)!=1024)
	ATT7022_CS_EN;
	SPI2_ReadWriteByte(0xC1); 
	SPI2_ReadWriteByte(0);
	SPI2_ReadWriteByte(0);
	SPI2_ReadWriteByte(0);//´Ó0¿ªÊ¼¶Á»º³å¼Ä´æÆ÷
	ATT7022_CS_DIS;
		
	for(j=0; j<64; j++)
	{
		for(i=0; i<7; i++)
		{
			adc_buf[i][j] = ReadSampleRegister2(0x7f);
			
			lBufInArray[i][j] = ((signed short)adc_buf[i][j])<<16;
			delay_us(5);
		}
	}
	for(i=0; i<6; i++)
  {
     cr4_fft_64_stm32(lBufOutArray[i], lBufInArray[i], NPT);
  }
	GetPowerMag();
	for(i=0; i<6; i++)
	{
		if(i<3)
			{
			  for(j=2;j<20;j++)
        {
				   ac_real1.har_vol[i][j]=harGain(i, j)*ac_real1.har_vol[i][1];
				}
			}
		else
			{
			  for(j=2;j<20;j++)
        {
				   ac_real1.har_cur[i-3][j]=harGain(i, j)*ac_real1.har_cur[i-3][1];
				}
			}
	}
	/*WriteAdjustRegister(0x70,0);
	WriteAdjustRegister(0x70,0x0000002a);//¸ÄÎªÐ³²¨²âÁ
	for(i=0; i<3; i++)
	{
	  ac_real1.har_vol[i][0] = (float)ReadSampleRegister(0x48+i)/(0x2000);
	  ac_real1.har_cur[i][0] = (float)ReadSampleRegister(0x4b+i)/(0x2000)/N;
	}
	WriteAdjustRegister(0x70,0);
	WriteAdjustRegister(0x70,0x0000000a);//¸ÄÎª»ù²¨²âÁ¿*/
}
float harGain(u8 i, u8 j)
{
	switch(j)
	{
		case 2: return lBufMagArray[i][j]/lBufMagArray[i][1]*1.00362187060665;
	  case 3: return lBufMagArray[i][j]/lBufMagArray[i][1]*1.00969162604172;
		case 4: return lBufMagArray[i][j]/lBufMagArray[i][1]*1.01825901332331;
		case 5: return lBufMagArray[i][j]/lBufMagArray[i][1]*1.02939520355364;
		case 6: return lBufMagArray[i][j]/lBufMagArray[i][1]*1.04319331488342;
		case 7: return lBufMagArray[i][j]/lBufMagArray[i][1]*1.05977378492696;
		case 8: return lBufMagArray[i][j]/lBufMagArray[i][1]*1.07927443401769;
		case 9: return lBufMagArray[i][j]/lBufMagArray[i][1]*1.10187021533519;
		case 10: return lBufMagArray[i][j]/lBufMagArray[i][1]*1.12776405100967;
		case 11: return lBufMagArray[i][j]/lBufMagArray[i][1]*1.15718387269867;
	  case 12: return lBufMagArray[i][j]/lBufMagArray[i][1]*1.19042304659018;
		case 13: return lBufMagArray[i][j]/lBufMagArray[i][1]*1.22777858879375;
		case 14: return lBufMagArray[i][j]/lBufMagArray[i][1]*1.26962990669109;
		case 15: return lBufMagArray[i][j]/lBufMagArray[i][1]*1.31639900106378;
		case 16: return lBufMagArray[i][j]/lBufMagArray[i][1]*1.36856044411429;
		case 17: return lBufMagArray[i][j]/lBufMagArray[i][1]*1.42671649331348;
		case 18: return lBufMagArray[i][j]/lBufMagArray[i][1]*1.49149037623292;
		case 19: return lBufMagArray[i][j]/lBufMagArray[i][1]*1.56363705732262;
	}
}

//¸üÐÂÊµÊ±Êý¾Ýac_real1
void UpdateAc_real1(u32 Times)
{
	ac_real1.head=0x060207;
	ac_real1.state=0x0101;
	ac_real1.sn = Times;
	RTC_Get(ac1_t);
	CompuFreq();
	CompuVol();
	ComputCur();
	Compupwr_p();
	Compupwr_q();
	Compupwr_s();
	Compupwr_f();
	Compuangel_cur();
	Compuangel_vol();
	CompuPowerMag();
}


//===============================================================//ac_real2ÊµÊ±Êý¾Ý



void UpdateAc_real2(void)
{
	
}



//===============================================================//¿ªÈëÁ¿
void Openin(void)//¿ªÈëÁ¿¼ì²é
{
	static u32 Hcnt[4]={0},Lcnt[4]={0};
	u8 i;
	YXin[0] = YX0;
	YXin[1] = YX1;
	YXin[2] = YX2;
	YXin[3] = YX3;
	for(i=0;i<4;i++)
	{
		if(YXin[i])
		{
			Hcnt[i]++;
			Lcnt[i]=0;
		}
		else
		{
			Hcnt[i]=0;
			Lcnt[i]++;
		}
		if(Hcnt[i]>ditime) YXout[i]=1;
		if(Lcnt[i]>ditime) YXout[i]=0;
		if(YXout[i] != YXout_[i]) Openin_write(i,YXout[i]);
		YXout_[i]=YXout[i];
	}	
}
void Openin_write(u8 i,u8 YXout)//¿ªÈëÁ¿±ä»¯×°ÔØ
{
	krnum+=1;
	krcnt+=1;
	tm[krcnt]=RTC_Get(kr_t);
	tm[krcnt].state=(1<<i)+(YXout<<4);
}
//===============================================================//Ð£±í²ÎÊý¼ÆËã
//HFConst²ÎÊý¼ÆËã²¢Ð´Èë¼Ä´æÆ÷
void CompuHFconst(float Un, float Ib, float Vu, float Vi)
{
	u32 tmpBuff=0;
	//u8 AdjustBuff[3];
	//HFconst = 25920000000*1.163*1.163*0.22*0.05/(EC*220*1.5);
	//DToBm(HFconst,AdjustBuff,3);            //½«½Ï±íÊý¾Ý×ª»»³É3×Ö½Ú²¹ÂëÐÎÊ½
	HFconst=0x6c;
	WriteAdjustRegister(0x1e,tmpBuff);  //Çå¿ÕÐ£±í´æ´¢Æ÷
	WriteAdjustRegister(0x1e,0x6c); //Ð´µ½Ð£±í¼Ä´æÆ÷
}

//PQS¹¦ÂÊÔöÒæ//µÚÒ»¸öÐ£×¼µã£º¹¦ÂÊÒòÊýÎª1
void CompuAdjustPQSgain(float StandValue, u8 PhaseType)
{
	//u32 tmpBuff=0;
	//signed short data;
	float RmsMesurePower ;  //ÕæÊµ²âÁ¿Öµ
	float ERR, PgainX, Pgain;
  //u8 AdjustBuff[3];
	RmsMesurePower = (float)ReadSampleRegister(0x01+PhaseType);
	if(RmsMesurePower > 0x800000)
		{
			RmsMesurePower = RmsMesurePower - 0x1000000;
			RmsMesurePower = RmsMesurePower*K;
		}
		else
		{
			RmsMesurePower = RmsMesurePower*K;
		}

	ERR=(RmsMesurePower-StandValue)/StandValue; //¼ÆËã²âÁ¿Îó²î
	PgainX=(0-ERR)/(1+ERR);

	if(PgainX>=0)Pgain=PgainX*8388608; //¼ÆËã½Ï±í¼Ä´æÆ÷²ÎÊý
  else Pgain=16777216+PgainX*8388608;

	//DToBm(Pgain,AdjustBuff,3);//½«½Ï±íÊý¾Ý×ª»»³É3×Ö½Ú²¹ÂëÐÎÊ½
  //WriteAdjustRegister(0x04+PhaseType,tmpBuff);  //Çå¿ÕÐ£±í´æ´¢Æ÷
	WriteAdjustRegister(0X04+PhaseType,(u32)Pgain>>8); //Ð´µ½Ð£±í¼Ä´æÆ÷
	//WriteAdjustRegister(0x07+PhaseType,tmpBuff);  //Çå¿ÕÐ£±í´æ´¢Æ÷
	WriteAdjustRegister(0X07+PhaseType,(u32)Pgain>>8); //Ð´µ½Ð£±í¼Ä´æÆ÷

	type_att_para2.w_Pgain[PhaseType]=(u32)Pgain>>8;
	type_att_para2.w_Qgain[PhaseType]=(u32)Pgain>>8;
	
}

//ÏàÎ»½ÃÕý//¹¦ÂÊÔöÒæÐ£ÕýÖ®ºó//µÚ¶þ¸öÐ£×¼µã£º¹¦ÂÊÒòÊýÎª0.5
void CompuAdjustPhSregpq(u8 PhaseType, u32 Mode)///////////////////////////////////////////////////////´ýÐÞÕý
{
	//u32 tmpBuff=0;
	float RmsMesurePha ;  //ÕæÊµ²âÁ¿Öµ
	float ERR, AngelX, Angel;
  //u8 AdjustBuff[3];
	/*RmsMesurePha = (float)ReadSampleRegister(0x18+PhaseType);
	
	ERR=(RmsMesurePha-(60+PhaseType*120))/(60+PhaseType*120); //¼ÆËã²âÁ¿Îó²î
	AngelX=(0-ERR)/1.732;
	
	if(AngelX>=0)Angel=AngelX*32768; //¼ÆËã½Ï±í¼Ä´æÆ÷²ÎÊý
  else Angel=65536+AngelX*32768;
	
	//DToBm(Angel,AdjustBuff,3);//½«½Ï±íÊý¾Ý×ª»»³É3×Ö½Ú²¹ÂëÐÎÊ½
	
  WriteAdjustRegister(0x0d+PhaseType,tmpBuff);  //Çå¿ÕÐ£±í´æ´¢Æ÷
	WriteAdjustRegister(0X0d+PhaseType,(u32)Angel); //Ð´µ½Ð£±í¼Ä´æÆ÷*/
	
    RmsMesurePha = (float)ReadSampleRegister(0x18+PhaseType);
		if(RmsMesurePha > 0x100000)
		{
			RmsMesurePha = RmsMesurePha - 0x1000000;
			RmsMesurePha = 360+(RmsMesurePha/0x100000)*180;
		}
		else
		{
			RmsMesurePha = (RmsMesurePha/0x100000)*180;
		}
  ERR=(RmsMesurePha-PhaseType*120-60)/(PhaseType*120+60); //¼ÆËã²âÁ¿Îó²î
	//else ERR=RmsMesurePha;
	AngelX=(0-ERR)/1.732;
	
	if(AngelX>=0)Angel=AngelX*32768; //¼ÆËã½Ï±í¼Ä´æÆ÷²ÎÊý
  else Angel=65536+AngelX*32768;
	
	//DToBm(Angel,AdjustBuff,3);//½«½Ï±íÊý¾Ý×ª»»³É3×Ö½Ú²¹ÂëÐÎÊ½
	if(Mode == 2)
	{
    //WriteAdjustRegister(0x0d+PhaseType,tmpBuff);  //Çå¿ÕÐ£±í´æ´¢Æ÷
	  WriteAdjustRegister(0X0d+PhaseType,(long)Angel); //Ð´µ½Ð£±í¼Ä´æÆ÷
		type_att_para2.w_PhSregApq[PhaseType]=(u32)Angel;
	}
  // WriteAdjustRegister(0x10+PhaseType,tmpBuff);  //Çå¿ÕÐ£±í´æ´¢Æ÷
	WriteAdjustRegister(0X10+PhaseType,(u32)Angel); //Ð´µ½Ð£±í¼Ä´æÆ÷
	//WriteAdjustRegister(0x61+PhaseType,tmpBuff);  //Çå¿ÕÐ£±í´æ´¢Æ÷
	WriteAdjustRegister(0X61+PhaseType,(u32)Angel); //Ð´µ½Ð£±í¼Ä´æÆ÷
	
	type_att_para2.w_PhSregAPQ1[PhaseType]=(u32)Angel;
	type_att_para2.w_PhSregAPQ2[PhaseType]=(u32)Angel;

}

//¹¦ÂÊoffsetÐ£×¼//¹¦ÂÊÔöÒæÐ£ÕýÓëÏàÎ»Ð£ÕýÖ®ºó//µÚËÄ¸öÐ£×¼µãÊ±Ð£×¼£º220V 0.25A 0.5L
void CompuAdjustPoffset(float StandValue, u8 PhaseType)
{
	//u32 tmpBuff=0;
	float RmsMesure_i;  //ÕæÊµ²âÁ¿Öµ
	float ERR, Poffset;
  u8 AdjustBuff[3];
	RmsMesure_i = (float)ReadSampleRegister(0x05+PhaseType);;
	if(RmsMesure_i > 0x800000)
		{
			RmsMesure_i = RmsMesure_i - 0x1000000;
			RmsMesure_i = RmsMesure_i*K;
		}
		else
		{
			RmsMesure_i = RmsMesure_i*K;
		}
	ERR=(RmsMesure_i-StandValue)/StandValue; //¼ÆËã²âÁ¿Îó²î
	
  Poffset = (float)StandValue*EC*HFconst*2147483648*(-ERR)/(2.592*10000000000);
	
	DToBm(Poffset,AdjustBuff,3);//½«½Ï±íÊý¾Ý×ª»»³É3×Ö½Ú²¹ÂëÐÎÊ½
	
  /*WriteAdjustRegister(0x13+PhaseType,tmpBuff);  //Çå¿ÕÐ£±í´æ´¢Æ÷
	WriteAdjustRegister(0X13+PhaseType,ArrayData(AdjustBuff,2)); //Ð´µ½Ð£±í¼Ä´æÆ÷
	WriteAdjustRegister(0x64+PhaseType,tmpBuff);  //Çå¿ÕÐ£±í´æ´¢Æ÷
	WriteAdjustRegister(0X64+PhaseType,(ArrayData(AdjustBuff,3)&0x000000ff)); //Ð´µ½Ð£±í¼Ä´æÆ÷*/
  //WriteAdjustRegister(0x21+PhaseType,tmpBuff);  //Çå¿ÕÐ£±í´æ´¢Æ÷
	WriteAdjustRegister(0X21+PhaseType,ArrayData(AdjustBuff,2)); //Ð´µ½Ð£±í¼Ä´æÆ÷
	//WriteAdjustRegister(0x67+PhaseType,tmpBuff);  //Çå¿ÕÐ£±í´æ´¢Æ÷
	WriteAdjustRegister(0X67+PhaseType,(ArrayData(AdjustBuff,3)&0x000000ff)); //Ð´µ½Ð£±í¼Ä´æÆ÷
	
	//type_att_para2.w_Poffset[PhaseType]=ReadAdjustRegister(0x13+PhaseType);
	//type_att_para2.w_PoffsetL[PhaseType]=ReadAdjustRegister(0x64+PhaseType);
	type_att_para2.w_Qoffset[PhaseType]=ArrayData(AdjustBuff,2);
	type_att_para2.w_QoffsetL[PhaseType]=(ArrayData(AdjustBuff,3)&0x000000ff);
}

//µçÑ¹ÔöÒæÐ£Õý//Ugain=0Ê±Ð£×¼===============µÚÒ»¸öÐ£×¼µã
void CompuAdjustUgain(float StandValue, u8 PhaseType)
{
	//u32 tmpBuff=0;
	//u8 AdjustBuff[3];
	float RealVol ;  //ÕæÊµµçÑ¹Öµ
  float Ugain;
  //long  Uadjust;

	RealVol = ReadSampleRegister(0x0D+PhaseType)/8192.00;  //²âÁ¿ÊäÈëµçÑ¹ÓÐÐ§Öµ
	Ugain=(StandValue/RealVol)-1; 
	if(Ugain>0)Ugain=Ugain*8388608;
  else Ugain=Ugain*8388608+16777216; 
	
	//Uadjust=(u32)Ugain;          //µÃµ½Ð£±í¼Ä´æÆ÷Öµ
	//DToBm(Ugain,AdjustBuff,3);  //×ª»»ÎªÊý×é
	
  //WriteAdjustRegister(0x17+PhaseType,tmpBuff);  //Çå¿ÕÐ£±í´æ´¢Æ÷
	WriteAdjustRegister(0x17+PhaseType,((u32)Ugain)>>8);	//Ð´µ½Ð£±í¼Ä´æÆ÷ 
	type_att_para2.w_Ugain[PhaseType]=(u32)Ugain>>8;
}

//µçÁ÷ÔöÒæÐ£Õý//Igain=0Ê±Ð£×¼==================µÚÒ»¸öÐ£×¼µã
void CompuAdjustIgain(float StandValue, u8 PhaseType)
{
	u32 tmpBuff=0;
	u8 AdjustBuff[3];
	float RealCur;  //ÕæÊµµçÑ¹Öµ
  float Igain;
  //long  Iadjust;

	RealCur = ReadSampleRegister(0x10+PhaseType)/8192.00/N;  //²âÁ¿ÊäÈëµçÁ÷ÓÐÐ§Öµ
	Igain=(StandValue/RealCur)-1; 
	if(Igain>0)Igain=Igain*32678;
  else Igain=Igain*32678+65536; 
	
	//Uadjust=(u32)Ugain;          //µÃµ½Ð£±í¼Ä´æÆ÷Öµ
	DToBm(Igain,AdjustBuff,3);  //×ª»»ÎªÊý×é
  //WriteAdjustRegister(0x1A+PhaseType,tmpBuff);  //Çå¿ÕÐ£±í´æ´¢Æ÷
	WriteAdjustRegister(0x1A+PhaseType,(u32)Igain);	//Ð´µ½Ð£±í¼Ä´æÆ÷ 
	type_att_para2.w_Igain[PhaseType]=(u32)Igain;
	
	WriteAdjustRegister(0x20,tmpBuff);  //Çå¿ÕÐ£±í´æ´¢Æ÷
	WriteAdjustRegister(0x20,(u32)Igain);	//Ð´µ½Ð£±í¼Ä´æÆ÷ 
	type_att_para2.w_GainADC7=(u32)Igain;
	
}

//µçÑ¹µçÁ÷ÓÐÐ§ÖµoffsetÐ£Õý//ÊäÈëÐÅºÅÎª0Ê±Ð£Õý//ÔÚÓÐÐ§ÖµÐ£ÕýÖ®Ç°
void CompuAdjustRmsoffse(u8 PhaseType)
{
	//u32 tmpBuff=0, tempdata;
	/*tempdata = ReadSampleRegister(0x13);
	WriteAdjustRegister(0x6a,tmpBuff);  //Çå¿ÕÐ£±í´æ´¢Æ÷
	WriteAdjustRegister(0x6a,tempdata*tempdata/32768);	//Ð´µ½Ð£±í¼Ä´æÆ÷ 
	type_att_para2.w_UaRmsoffse[PhaseType]=ReadAdjustRegister(0x24+PhaseType);
	
	tempdata = ReadSampleRegister(0x0D+PhaseType);
	WriteAdjustRegister(0x24+PhaseType,tmpBuff);  //Çå¿ÕÐ£±í´æ´¢Æ÷
	WriteAdjustRegister(0x24+PhaseType,tempdata*tempdata/32768);	//Ð´µ½Ð£±í¼Ä´æÆ÷ 
	type_att_para2.w_UaRmsoffse[PhaseType]=ReadAdjustRegister(0x24+PhaseType);*/
	
	//tempdata = ReadSampleRegister(0x10+PhaseType);
	//tempdata*tempdata/32768
	//WriteAdjustRegister(0x27+PhaseType,tmpBuff);  //Çå¿ÕÐ£±í´æ´¢Æ÷
	WriteAdjustRegister(0x27+PhaseType,0x07);	//Ð´µ½Ð£±í¼Ä´æÆ÷ 
	type_att_para2.w_IaRmsoffse[PhaseType]=0x07;	
	
}

//ToffsetÐ£Õý//³£ÎÂ25¶ÈÊ±Ð£Õý
void CompuAdjustToffset(void)
{
	u32 tmpBuff=0;
	WriteAdjustRegister(0x6B,tmpBuff);  //Çå¿ÕÐ£±í´æ´¢Æ÷
	WriteAdjustRegister(0x6B,ReadSampleRegister(0x2A));	//Ð´µ½Ð£±í¼Ä´æÆ÷ 
}

























