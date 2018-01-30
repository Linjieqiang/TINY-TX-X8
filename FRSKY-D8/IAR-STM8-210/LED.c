#include "include.h"

#define FOSC  12000000
#define BAUD  115200

uint8_t LED_State_ON     = 0x00 ;						//LED����
uint8_t LED_State_Shake  = 0x00 ;						//LED��˸

uint8_t LED_Status_SendDat = 0 ; 

//==============================================================================
//LED���Ƴ�ʼ��(UART1_TX)
//==============================================================================
void LED_UART1_Init(void)
{
  	//��USART1 ʱ��(�͹���STM8����Ҫ)
  	CLK -> PCKENR1 |= (1<<5) ;
	
  	USART_DeInit(USART1);
	
  	//���ô���1: 115200    12M 
	uint16_t BRR_Counts = 0 ; 
	BRR_Counts = FOSC / BAUD ; 
	USART1 -> BRR2  =  BRR_Counts & 0x000F ; 
       	USART1 -> BRR2 |= ((BRR_Counts & 0xf000) >> 8);
    	USART1 -> BRR1  = ((BRR_Counts & 0x0ff0) >> 4);     			/*�ȸ�BRR2��ֵ ���������BRR1*/  
	
	USART1 -> CR1  |= (1<<6)|(1<<4)|(1<<2)|(1<<1);				//9λ���� 1λֹͣλ ��У�� ���ж�
	USART1 -> CR2  |= (1<<3) ;						//����ʹ��
	USART1 -> CR3  = 0 ; 
	
	USART1->CR1 &= (uint8_t)(~USART_CR1_USARTD);
}



//==============================================================================
//���ڷ���3Byte ���ݸ��ư壬���� 1Byte ����ͷ + 1Byte��ʾ���� + 1Byte��ʾ����ȡ��
//��ʾ���� (8Bit) : �ֱ��Ӧ��ʾ��8��LED
//�ư��յ����ݺ���Ҫ���ж�������Ч�ԡ���Ч����LED��ʾ״̬����Ч������һ��״̬��ʾ
//����Ƶ�� : 5mS/��       LED��˸Ƶ��50mS/��(���е�Դָʾ�Ʋ��ܴ���˸Ƶ������)
//==============================================================================
void LED_Usart1Send(void)
{
	static uint16_t Shake_Rate_Counts = 0 ; 
	
	
	if(Shake_Rate_Counts < 40) ++Shake_Rate_Counts ;
	else Shake_Rate_Counts = 0 ; 
	
	
	if(Shake_Rate_Counts < 20)
	{
	  	//��̬��ʾ : ֻ���� ��������LED
		LED_Status_SendDat = LED_State_ON ; 
	}
	else
	{	
	  	//��˸��ʾ : ���� �������� + ��˸����
	  	LED_Status_SendDat = LED_State_ON | LED_State_Shake ; 
	}
	
  	while(!(USART1 -> SR & (1<<7))); USART_SendData8(USART1, 0x55); 
	while(!(USART1 -> SR & (1<<7))); USART_SendData8(USART1, LED_Status_SendDat); 
	while(!(USART1 -> SR & (1<<7))); USART_SendData8(USART1, ~LED_Status_SendDat);   
}


/*==============================================================================
﮵�� 30% - 100% ��ѹ : ����3.77V��             ����  
       10% - 30%  ��ѹ : 3.65V  - 3.77V  1S      ����
       0%  - 10%  ��ѹ : ���� 3.65V      0.25S   ����

��Դָʾ����ʾ : ��ʾ��ص������(����Խ�ͣ���˸Խ�죬ֱ������)
==============================================================================*/
typedef enum 
{
	__stAbove30Percent 	= 1 , 
	__stAbove10Percent	= 2 , 
	__stUnder10Percent 	= 3 , 
}VoltStatusTypeDef ; 
void Volt_LED(float Volt_Value)
{
  	//��ص������ : Ϊ��ֹ����ѹ�����ٽ��ʱ���������������������ȼ�������Ŵ���
  	//��ѹ��Ҫ������������ 2S ����ͬһ״̬����Ϊ������Ч��
	static VoltStatusTypeDef VoltStatus = __stAbove30Percent ;
	static uint16_t VoltStatusUpCNT   = 0 ; 
	static uint16_t VoltStatusDownCNT = 0 ; 
	
	//��Դָʾ����˸Ƶ�ʼ���
  	static uint8_t VoltLED_ShakeRateCnt = 0 ; 
	static uint8_t VoltLED_Counts = 0 ;  
	
	//======================================================================
	//��ص�������ж�:Ĭ�ϵ�ѹֻ��һֱ�½�(�������������)
	//(1)30% - 100%���� : ����ѹ С�� 3.77V ����3S ���������� 10% - 30% ״̬   
	//(2)10% - 30% ���� : ����ѹ С�� 3.65V ����3S ���������� 10%         ���� 3.79V ����3S ����Ϊ������� �� 30% - 100%״̬
	//(3)0%  - 10% ���� : ����ѹ ���� 3.68V ��Ϊ������� ��   10% - 30%״̬
	//======================================================================
	if(VoltStatus == __stAbove30Percent) 
	{
	  	//����ѹС��3.77Vʱ����Ϊ���������� 10% - 30%
		if(Volt_Value < 3.77f) 
		{
			if(VoltStatusDownCNT < 400) ++VoltStatusDownCNT ; 
			else 
			{
				VoltStatus = __stAbove10Percent ; 
				VoltStatusDownCNT = 0 ; 
			}
		}
		else VoltStatusDownCNT = 0 ;
		
		VoltStatusUpCNT = 0 ; 						//����(��������30% ����Ҫ���� �������������)
	}
	else if(VoltStatus == __stAbove10Percent) 
	{
	  	//����ѹ���� 3.79Vʱ ������Ϊ���������� 30%(Ϊ�˱����ѹ������ɵ�״̬���ȶ�)
		if(Volt_Value > 3.79f) 
		{
		  	VoltStatusDownCNT = 0 ; 
			if(VoltStatusUpCNT < 400) ++VoltStatusUpCNT ; 
			else 
			{
				VoltStatus = __stAbove30Percent ; 
				VoltStatusUpCNT = 0 ;
			}
		}
		
		//����ѹС�� 3.65V ��Ϊ�������� 10 %
		else if(Volt_Value < 3.65f) 
		{
		  	VoltStatusUpCNT = 0 ; 
			if(VoltStatusDownCNT < 400) ++VoltStatusDownCNT ; 
			else 
			{
				VoltStatus = __stUnder10Percent ; 
				VoltStatusDownCNT = 0 ;
			}
		}
		
		//��ص�ѹ �� (3.65 - 3.79V ֮�� )
		else
		{
		  	VoltStatusUpCNT = 0 ; 
			VoltStatusDownCNT = 0 ;
		}
	}
	else
	{
		//����ص�ѹ���� 3.68������Ϊ��������10%����
	  	if(Volt_Value > 3.68f) 
		{
			if(VoltStatusUpCNT < 400) ++VoltStatusUpCNT ; 
			else
			{
				VoltStatus = __stAbove10Percent ; 
				VoltStatusUpCNT = 0 ;
			}
		}
		else
		{
			VoltStatusUpCNT = 0 ;
		}
		  	 
		VoltStatusDownCNT = 0 ;
	}
	
	
	//======================================================================
	//���ݵ���״̬�����õ��ָʾ����ʾ(���� / ���� / ����)
	//======================================================================
	if(VoltStatus == __stAbove30Percent)
	{
	  	VoltLED_Counts = 0 ; 		//30% - 100% : ����
		//���������������ѹ����
		if(RunStatus == __stVolt_err)
		{
			beepCmd(NormalFreCounts , __stStop);
			RunStatus = __stNormal ;
		}
		
	}
	else if(VoltStatus == __stAbove10Percent)
	{
		VoltLED_Counts = 200 ; 		//10% - 30% : ����
		//���������������ѹ����
		if(RunStatus == __stVolt_err)
		{
			beepCmd(NormalFreCounts , __stStop);
			RunStatus = __stNormal ;
		}
	}
	else 
	{
	  	VoltLED_Counts = 50 ;		//����30%   : ����
		//�͵�������
		if(RunStatus < __stVolt_err)
		{
		  	RunStatus = __stVolt_err ;
			beepCmd(NormalFreCounts , __stFastContinumWarning);
		}
	}
	
	if(VoltLED_ShakeRateCnt < VoltLED_Counts) ++VoltLED_ShakeRateCnt ; 
	else VoltLED_ShakeRateCnt = 0 ; 
	
	if(VoltLED_ShakeRateCnt <= (VoltLED_Counts/2))
	{
		LED_State_ON |= LED_VOLT ; 	//�ر�  ��Դָʾ��
	}
	else
	{
		LED_State_ON &= ~LED_VOLT ; 	//�� ��Դָʾ��
	}
}