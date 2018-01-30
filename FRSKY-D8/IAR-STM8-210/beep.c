/*==============================================================================
Beep ����IO TIM1_CH2
��������ʾ���� :
(1)������ʾ�� : Ƶ��:1000Hz  ģʽ : �̽�/����/���ζ̽�/���ζ̽�
(2)΢����ʾ�� : 1000(����) -> 1600(��ǿ) -> 1000(����)  ÿ���� 20HZ * 20 * 2
(3)������ʾ�� : (a)ң������ֹ��ʱ   		(���� ��һ��)
		(b)ҡ�˸�λ����	    		(���� ������)
		(c)�͵���/��λУ׼����          (���� ���ٽ�)
  		(d)CYRF6936���߱��� 		(���� ������)
==============================================================================*/
#include "include.h"

#define ENABLE_PRO_CYLCE	500 					    	//����ɱ������  5S

static uint16_t Beep_ON_cnt = 0 ; 						//����������
static BeepModeTypeDef BeepMode = __stStop ; 					//������ģʽ

//==============================================================================
//��ʼ�� BBEP ���� IO  PD4
//ͨ�� TIM1_CH2 ���Ƶ�ʱ仯 ���� BEEP ����
//==============================================================================
void beep_Init(void)
{
  	//������TIM1ʱ��
	CLK -> PCKENR2 |= (1<<1);
	
	//��ʼ�������� PD4 (TIM1_CH2)
	GPIOD -> CR1 |= (1<<4);
	GPIOD -> CR2 |= (1<<4);
	GPIOD -> DDR |= (1<<4);
	
	//ע��: TIM1��Ƶ���� F = Fsc/(PSCR)=12M/(11+1) = 1M
	TIM1 -> PSCRH = 0x00 ;   						
	TIM1 -> PSCRL = 0x0B ;
	
	//������Ƶ�� : ��ʼ�� 1K
	TIM1 -> ARRH  = (1000000/1000) >> 8 ;					
	TIM1 -> ARRL  = (1000000/1000) & 0xff ;
	
	//�ȽϼĴ���ʼ�տ���ΪƵ�ʵ�һ��
	TIM1 -> CCR2H = (1000000/2000) >> 8 ;
	TIM1 -> CCR2L = (1000000/2000) & 0xff ;
	
	//PWM1���ģʽ(���ϼ��� : CNT < CCR1 ���1  CNT >= CCR1 ���0)
	TIM1 -> CCMR2 = 0x68 ;	
	//ʹ��TIM1_CH2���  
	TIM1 -> CCER1 = 0x30 ; 							
	//���ϼ���
	TIM1 -> CR1  &= ~(1<<0);    						//����ͣ����
	TIM1 -> BKR  &= ~(1<<7);						//��ֹ���	
	
	BeepMode = __stStop ; 
	Beep_ON_cnt = 0 ; 
}

void beepCmd(uint16_t BeepFreCounts , BeepModeTypeDef Mode)
{
	//��ֹ�ظ�����
	if(BeepMode != Mode)
	{
	  	TIM1 -> CR1  &= ~(1<<0);    					
		TIM1 -> BKR  &= ~(1<<7);					//�ر����
	
		//������Ƶ�ʷ�Χ : 1K -> 2K
  		if(BeepFreCounts > 2000)      BeepFreCounts = 2000;
		else if(BeepFreCounts < 1000) BeepFreCounts = 1000 ; 
		
		//��������Ĵ���
		TIM1 -> CNTRH = 0 ; 
		TIM1 -> CNTRL = 0 ; 
		
  		//���÷�����Ƶ��
		TIM1 -> ARRH  = (1000000/BeepFreCounts) >> 8 ;			//������Ƶ�� 
		TIM1 -> ARRL  = (1000000/BeepFreCounts) & 0xff ;
  
  		TIM1 -> CCR2H = (1000000/(BeepFreCounts*2)) >> 8 ;
		TIM1 -> CCR2L = (1000000/(BeepFreCounts*2)) & 0xFF ;
	
		TIM1 -> CR1  |= (1<<0);
		TIM1 -> BKR  |= (1<<7);
	
  		BeepMode = Mode ;
		Beep_ON_cnt = 0 ; 
	}
}



void beepScan(void)
{
	if(BeepMode == __stStop) 
	{
	  	Beep_ON_cnt = 0 ; 
		TIM1 -> BKR  &= ~(1<<7);
		TIM1 -> CR1  &= ~(1<<0); 
	}
	
	//������ʾ��(�̽�һ��0.025S)
	else if(BeepMode == __stKeyClick)
	{
		if(Beep_ON_cnt < 3) ++Beep_ON_cnt ; 
		else
		{
		  	BeepMode = __stStop ; 
		}
	}
	
	//����λ�� ΢����ֵ ���� ��ֵ(����һ��0.2S)
	else if(BeepMode == __stMidOffSet)
	{
		if(Beep_ON_cnt < 20) ++Beep_ON_cnt ; 
		else
		{
		  	BeepMode = __stStop ; 
		}
	}
	
	//�˳���ֹ��ʱ����(��������)
	else if(BeepMode == __stExitStaticOverTime)
	{
		if(Beep_ON_cnt < 30) ++Beep_ON_cnt ; 
		else
		{
		  	BeepMode = __stStop ; 
		}
		
		if((Beep_ON_cnt < 10) || (Beep_ON_cnt > 20))
		{
		  	if(!(TIM1->BKR & (1<<7))) TIM1 -> BKR |=  (1<<7) ;
		}
		else
		{
			if(TIM1->BKR & (1<<7))    TIM1 -> BKR &= ~(1<<7) ;
		}
	}
	
	//��λУ׼��� ��ʾ��(����1��)
	else if(BeepMode == __stMidADAdjustDown)
	{
		if(Beep_ON_cnt < 40) ++Beep_ON_cnt ; 
		else
		{
		  	BeepMode = __stStop ; 
		}
		
		if(!(TIM1->BKR & (1<<7))) TIM1 -> BKR |=  (1<<7) ;
	}
	
	//======================================================================
	//			������ʾ��
	//======================================================================
	//������ʾ��
	else if(BeepMode == __stBindWarning)
	{
		if(Beep_ON_cnt < ENABLE_PRO_CYLCE)
		{
		  	++Beep_ON_cnt ; 
			//Ƶ�� : 512
			if(((Beep_ON_cnt < 90) && (Beep_ON_cnt >= 40)) || ((Beep_ON_cnt < 150) && (Beep_ON_cnt >= 100)) || ((Beep_ON_cnt < 210) && (Beep_ON_cnt >= 160)) || ((Beep_ON_cnt < 330) && (Beep_ON_cnt >= 280)) || ((Beep_ON_cnt < 500) && (Beep_ON_cnt >= 400)))
			{
			  	if((TIM1 -> ARRH != ((1000000/512) >> 8)) || (TIM1 -> ARRL  != ((1000000/512) & 0xff)) || (!(TIM1 -> BKR & (1<<7))))
				{
					TIM1 -> ARRH  = (1000000/512) >> 8 ;			//������Ƶ�� 
					TIM1 -> ARRL  = (1000000/512) & 0xff ;
  
  					TIM1 -> CCR2H = (1000000/(512*2)) >> 8 ;
					TIM1 -> CCR2L = (1000000/(512*2)) & 0xFF ;	
					
					TIM1 -> BKR |=  (1<<7) ;
				}
			}
			
			//Ƶ�� : 413
			else if(((Beep_ON_cnt < 255) && (Beep_ON_cnt >= 220)) || ((Beep_ON_cnt < 375) && (Beep_ON_cnt >= 340)))
			{
				if((TIM1 -> ARRH != ((1000000/413) >> 8)) || (TIM1 -> ARRL  != ((1000000/413) & 0xff)) || (!(TIM1 -> BKR & (1<<7))))
				{
					TIM1 -> ARRH  = (1000000/413) >> 8 ;			//������Ƶ�� 
					TIM1 -> ARRL  = (1000000/413) & 0xff ;
  
  					TIM1 -> CCR2H = (1000000/(413*2)) >> 8 ;
					TIM1 -> CCR2L = (1000000/(413*2)) & 0xFF ;	
					
					TIM1 -> BKR |=  (1<<7) ;
				}
			}
			
			//Ƶ�� : 610
			else if(((Beep_ON_cnt < 280) && (Beep_ON_cnt >= 255)) || ((Beep_ON_cnt < 400) && (Beep_ON_cnt >= 375)))
			{
				if((TIM1 -> ARRH != ((1000000/610) >> 8)) || (TIM1 -> ARRL  != ((1000000/610) & 0xff)) || (!(TIM1 -> BKR & (1<<7))))
				{
					TIM1 -> ARRH  = (1000000/610) >> 8 ;			//������Ƶ�� 
					TIM1 -> ARRL  = (1000000/610) & 0xff ;
  
  					TIM1 -> CCR2H = (1000000/(610*2)) >> 8 ;
					TIM1 -> CCR2L = (1000000/(610*2)) & 0xFF ;	
					
					TIM1 -> BKR |=  (1<<7) ;
				}
			}
			
			//��������� Ϩ��
			else
			{
				if(TIM1->BKR & (1<<7))    TIM1 -> BKR &= ~(1<<7) ;
			}
		}
		else
		{
		  	Beep_ON_cnt = 0 ;  
		}
	}
	
	//ң������ֹ��ʱ : ������һ��
	else if(BeepMode == __stStaticOverTimeWarning)
	{
		if(Beep_ON_cnt < 100) ++Beep_ON_cnt;
		else Beep_ON_cnt = 0 ; 
		
		if(Beep_ON_cnt < 5)
		{
		  	if(!(TIM1->BKR & (1<<7))) TIM1 -> BKR |=  (1<<7) ;
		}
		else
		{
			if(TIM1->BKR & (1<<7))    TIM1 -> BKR &= ~(1<<7) ;
		}
	}
	//CYRF6936���߱��� : ����������
	else if(BeepMode == __stRFModelLostWarning)
	{
		if(Beep_ON_cnt < 100) ++Beep_ON_cnt;
		else Beep_ON_cnt = 0 ; 
		
		if((Beep_ON_cnt < 5) || ((Beep_ON_cnt > 10)&&(Beep_ON_cnt < 15)) || ((Beep_ON_cnt > 20)&&(Beep_ON_cnt < 25)))
		{
		  	if(!(TIM1->BKR & (1<<7))) TIM1 -> BKR |=  (1<<7) ;
		}
		else
		{
			if(TIM1->BKR & (1<<7))    TIM1 -> BKR &= ~(1<<7) ;
		}
	}
	//���Ÿ��ϵ籨�� : ����������
	else if(BeepMode == __stThrottleHighWarning)
	{
		if(Beep_ON_cnt < 100) ++Beep_ON_cnt;
		else Beep_ON_cnt = 0 ; 
		
		if((Beep_ON_cnt < 5) || ((Beep_ON_cnt > 10)&&(Beep_ON_cnt < 15)) || ((Beep_ON_cnt > 20)&&(Beep_ON_cnt < 25)) || ((Beep_ON_cnt > 30)&&(Beep_ON_cnt < 35)))
		{
		  	if(!(TIM1->BKR & (1<<7))) TIM1 -> BKR |=  (1<<7) ;
		}
		else
		{
			if(TIM1->BKR & (1<<7))    TIM1 -> BKR &= ~(1<<7) ;
		}
	}
	//���������� : �����͵���������λУ׼����
	else if(BeepMode == __stFastContinumWarning)
	{
		if(Beep_ON_cnt < 10) ++Beep_ON_cnt;
		else Beep_ON_cnt = 0 ; 
		
		if(Beep_ON_cnt < 5) 
		{
		  	if(!(TIM1->BKR & (1<<7))) TIM1 -> BKR |=  (1<<7) ;
		}
		else
		{
			if(TIM1->BKR & (1<<7))    TIM1 -> BKR &= ~(1<<7) ;
		}
	}
}