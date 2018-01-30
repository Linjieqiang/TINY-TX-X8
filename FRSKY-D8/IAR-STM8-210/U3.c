/*
2017-8-16 : ȡ�� USART3 DMA��������жϡ����ڸ�Ϊÿ�η���һ�����ݺ�ֹͣ���´η���ǰ�ȹر��ٴ�(��Ϊ��� TIM2 ���߷����ж��໥��ͻ��Ӱ�����߷���ʱ��)
*/
#include "include.h"

#define  BAUD  115200
#define  Fosc  12000000

#define  U3_DR_Address                    (USART3_BASE + 0x01)
#define  U3_Buffer_Size 		   103

bool DMA1_U3_Tx_Flag = false ; 		  //DMA���ͱ�־λ

static uint8_t U3_DMA_TXBuff[U3_Buffer_Size] = 
{
  	//��ʼͷ(7Byte  ���ݹ̶�)
	0x4C , 0x44 , 0x41 , 0x54 , 0x58 , 0x31 , 0x30 , 
	//ң��������(1Byte  0x45 / 0x44)
	0x45 , 
	//ң��������汾��(3Byte  �� - �� - ��)
	0x00 , 0x00 , 0x00 , 
	//����Э��汾��(1Byte)
	0x00 ,
	//������/�ձ���ģʽѡ��
	0x00 ,
	//ң����ΨһID��(4Byte)
	0x00 , 0x00 , 0x00 , 0x00 , 
	//16ͨ������(ÿͨ��2Byte = 16*2Byte = 32Byte)
	0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 
	0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 ,
	0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 ,
	0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 ,
	//ͨ�������־λ(1Byte)
	0x00 , 
	//���а��� + 8λ���뿪��GPIOֵ(2Byte)
	0x00 , 0x00 , 0x00 , 
	//��ص�ѹֵ(2Byte  ��ص�ѹ�Ŵ�100��)
	0x00 , 0x00 , 
	//�ο���431 + ��Ƶģ�� + ��λУ׼�Ƿ�ɹ�  ��־λ(1Byte)
	0x00 , 
	//����ͨ��AD����ԭʼֵ(ÿͨ��2Byte = 6*2Byte = 12Byte)
	0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 
	0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 
	//LED״̬(1Byte)  1�������  0����Ϩ��
	0x00 , 
	//����״̬(1Byte)
	0x00 , 
	//��ʱ����������(1Byte)
	0x00 , 
	//��ҡ��ƫ��ֵ(4Byte)
	0x00 , 0x00 , 0x00 , 0x00 , 
	
	//��ҡ�����ADֵ(4*2Byte = 8Byte)
	0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 
	
	//��ҡ����λADֵ(4*2Byte = 8Byte)
	0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 
	
	//��ҡ����СADֵ(4*2Byte = 8Byte)
	0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 
	
	//���ݰ�У���(2Byte)
	0x00 , 0x00 , 
}; 


void U3_Init(void)
{
  	CLK -> PCKENR3 |= (1<<4) ; 						//�� usart3 ʱ�� 
	
	//��ʼ�� DMA1_CH1 (Usart3_TX DMA)
	DMA_DeInit(DMA1_Channel1);
	DMA_Init( DMA1_Channel1 , (uint32_t)U3_DMA_TXBuff,			//DMA�ڴ��ַ
                                  U3_DR_Address,				//DMA ADC�����ַ
				  U3_Buffer_Size,				//�������ݸ��� : 103
				  DMA_DIR_MemoryToPeripheral,			//���䷽�� : �ڴ� -> ����
				  DMA_Mode_Normal,				//DMAģʽ : ���δ���
                                  DMA_MemoryIncMode_Inc,			//�ڴ��ַ�ۼ�
				  DMA_Priority_High,				//DMA���ȼ� : ��
				  DMA_MemoryDataSize_Byte );			//�������ݳߴ� : 8 bit
	DMA_Cmd(DMA1_Channel1,DISABLE);						//�ȹر�DMA(��Ҫʱ�ٴ�)
	DMA_GlobalCmd(ENABLE);
	
	//Usart3��ʼ��
	uint16_t BRR_Counts = Fosc / BAUD ; 
	
	USART3 -> BRR2  =  BRR_Counts & 0x000F ; 
       	USART3 -> BRR2 |= ((BRR_Counts & 0xf000) >> 8);
    	USART3 -> BRR1  = ((BRR_Counts & 0x0ff0) >> 4);     			/*�ȸ�BRR2��ֵ ���������BRR1*/  
	USART3 -> CR2   = (1<<3) ; 						//����ʹ��
	USART3 -> CR3   = 0 ; 
	USART3 -> CR1 &= ~(1<<5);						//ʹ�ܴ���
	
	DMA1_U3_Tx_Flag = false ; 
}


//==============================================================================
//���ʹ������ݰ�
//==============================================================================
void U3_DMATX_ONOFF(void)
{
  	//������ɱ�־λ
  	if (DMA1_Channel1 -> CSPR & (1<<1))
	{
		DMA1_Channel1 -> CSPR &= ~(1<<1) ; 	//����жϱ��
		USART3 -> CR5   &= ~(1<<7) ; 		//�ǳ���Ҫ �ر�UART1 DMA��������
		DMA1_Channel1->CCR &= ~(1<<0);		//�ر�DMA
		DMA1_U3_Tx_Flag = false ; 		//��λ��־λ
	}
  
  	//������ɺ󣬲ŻῪʼ����
    	if (DMA1_U3_Tx_Flag == false)
	{
		//���÷��͸���
		DMA1_Channel1 -> CNBTR = U3_Buffer_Size;
		
		//����UART3 DMA���ͣ�
		USART3 -> CR5   = (1<<7) ; 					//ʹ�� USART3 DMA TX
		DMA1_Channel1 -> CCR |= (1<<0) ;
		
		DMA1_U3_Tx_Flag = true;
	}
}

//==============================================================================
//��Ϊ���ݰ�̫��Ϊ�˲�Ӱ�������������Ӧ�ٶȣ���һ���������ݲ��5�����롣
//�������������ݺ���ʹ�ܷ��͡�
//==============================================================================
void BuildUsart3Data(void)
{
	static uint8_t Phase = 0 ; 
	uint8_t i = 0 ; 
	uint16_t DataTemp = 0 ; 
	
	//����ǰ 17 Byte ����
	if(Phase == 0)
	{
	  	//                  L                         D                          A                         T                         X                          1                        0
		U3_DMA_TXBuff[0]  = 0x4C ; U3_DMA_TXBuff[1] = 0X44 ; U3_DMA_TXBuff[2] = 0x41 ; U3_DMA_TXBuff[3] = 0x54 ; U3_DMA_TXBuff[4] = 0X58 ; U3_DMA_TXBuff[5] = 0x31 ; U3_DMA_TXBuff[6] = 0x30 ; //��ʼͷ(7Byte)
		U3_DMA_TXBuff[7]  = RF_TypeVersion ;											//ң��������(FRSKYD8)
		U3_DMA_TXBuff[8]  = 18   ; U3_DMA_TXBuff[9] =  1 ; U3_DMA_TXBuff[10] = 25 ; 						//�������ʱ�� : 2018 - 1 - 25
		U3_DMA_TXBuff[11] = MasterInitProtocolVersion ; 									//����Э��汾�� (1Byte)
		U3_DMA_TXBuff[12] = RFHabit ;												//������/�ձ���ѡ�� (0:������ / 1:�ձ���)
		U3_DMA_TXBuff[13] = 0x00 ; //(TransmitterID >> 24)&0xff ; 								//ң����ID��(SFHSS ֻ��16Byte ���Ը�16λ�̶�Ϊ0)
		U3_DMA_TXBuff[14] = 0x00 ; //(TransmitterID >> 16)&0xff ;
		U3_DMA_TXBuff[15] = (TransmitterID >> 8)&0xff ;
		U3_DMA_TXBuff[16] = TransmitterID &0xff ; 
	}
	//���� 16ͨ�� ����
	else if(Phase == 4)
	{
	  	for(i = 0 ; i < PTOTOCOL_MAX_CHANNEL ; i++)
		{
			if(i < TRANSMITTER_CHANNEL) DataTemp = FRSKYD8_SendDataBuff[i];
			else DataTemp = 0;
			
			U3_DMA_TXBuff[17 + 2*i] = DataTemp >> 8;
			U3_DMA_TXBuff[18 + 2*i] = DataTemp & 0xFF;
		}
	}
	//���� ͨ�������־λ(1Byte) + ���а��� + 8λ���뿪��GPIOֵ(2Byte) + ��ص�ѹֵ(2Byte) + �ο���431 ��Ƶģ�� ��λУ׼�ɹ� ��־λ
	else if(Phase == 7)
	{
		U3_DMA_TXBuff[49] = ChannelInversion_flg ; 			//ͨ�������־λ
		
		DataTemp  = 0x0000 ; 
		if(GPIOC -> IDR & (1<<4)) DataTemp |= 0x2000 ; 
		if(GPIOC -> IDR & (1<<5)) DataTemp |= 0x1000 ; 
		
		if(GPIOE -> IDR & (1<<5)) DataTemp |= 0x0800 ; 
		if(GPIOE -> IDR & (1<<4)) DataTemp |= 0x0400 ; 
		
		if(GPIOD -> IDR & (1<<1)) DataTemp |= 0x0200 ; 
		if(GPIOD -> IDR & (1<<0)) DataTemp |= 0x0100 ; 
		if(GPIOC -> IDR & (1<<7)) DataTemp |= 0x0080 ; 
		if(GPIOC -> IDR & (1<<6)) DataTemp |= 0x0040 ; 
		if(GPIOE -> IDR & (1<<7)) DataTemp |= 0x0020 ; 
		
		if(GPIOE -> IDR & (1<<3)) DataTemp |= 0x0010 ; 
		if(GPIOE -> IDR & (1<<2)) DataTemp |= 0x0008 ; 
		if(GPIOE -> IDR & (1<<0)) DataTemp |= 0x0004 ; 
		if(GPIOE -> IDR & (1<<1)) DataTemp |= 0x0002 ; 
		if(GPIOE -> IDR & (1<<6)) DataTemp |= 0x0001 ; 
		U3_DMA_TXBuff[50] = DataTemp >> 8 ;
		U3_DMA_TXBuff[51] = DataTemp & 0xFF ;
		
		
		DataTemp = 0x0000 ; 
		if(GPIOA -> IDR & (1<<4)) DataTemp |= 0x80 ;  // T
		if(GPIOA -> IDR & (1<<5)) DataTemp |= 0x40 ;  // A
		if(GPIOA -> IDR & (1<<6)) DataTemp |= 0x20 ;  // E
		if(GPIOA -> IDR & (1<<7)) DataTemp |= 0x10 ;  // R 
		
		if(GPIOF -> IDR & (1<<0)) DataTemp |= 0x08 ;  //CH5
		if(GPIOF -> IDR & (1<<1)) DataTemp |= 0x04 ;  //CH6
		if(GPIOG -> IDR & (1<<2)) DataTemp |= 0x02 ;  //CH7
		if(GPIOG -> IDR & (1<<3)) DataTemp |= 0x01 ;  //CH8
		U3_DMA_TXBuff[52] = DataTemp&0xFF ;
		
		DataTemp = (uint16_t)(VoltValue * 100) ; 
		U3_DMA_TXBuff[53] = DataTemp >> 8 ;
		U3_DMA_TXBuff[54] = DataTemp & 0xFF ;
		
		DataTemp = 0 ; 
		if(ISCJ431OKFlg)           		DataTemp |= (1<<2) ;
		if(CommunicationError_flg == false) 	DataTemp |= (1<<1) ;
		if(FTDebug_err_flg == false) 	        DataTemp |= (1<<0) ;
		U3_DMA_TXBuff[55] = DataTemp & 0xFF ;
	}
	//���� ����ͨ��AD����ԭʼֵ + LED״̬ + ����״̬ + ��ʱ���������� + ��ҡ��ƫ��ֵ(4Byte) + ��ҡ����λADֵ
	else if(Phase == 8)
	{
	  	//7ͨ��ADֵ
		for(i = 0 ; i < 7 ; i++)
		{
			DataTemp = ADC_Value[i] ; 
			U3_DMA_TXBuff[56 + 2*i] = DataTemp >> 8 ;
			U3_DMA_TXBuff[57 + 2*i] = DataTemp & 0xFF ;
		}
		//LED״̬
		U3_DMA_TXBuff[70] = LED_Status_SendDat ; 
		//����״̬
		U3_DMA_TXBuff[71] = RunStatus ; 
		//��ʱ����������
		U3_DMA_TXBuff[72] = TIM2_ErrorCnt ; 
		//��ҡ��ƫ��ֵ + 4ҡ�˼�ֵ(�����λ����СADֵ)
		for(i = 0 ; i < 4 ; i++)
		{
			U3_DMA_TXBuff[73+i] = Sampling_Offset[i];
			
			DataTemp = Sampling_MaxMinData[i][MAXDAT];
			U3_DMA_TXBuff[77 + 2*i] = DataTemp >> 8;
			U3_DMA_TXBuff[78 + 2*i] = DataTemp & 0xFF;
			
			DataTemp = Sampling_MaxMinData[i][MIDDAT];
			U3_DMA_TXBuff[85 + 2*i] = DataTemp >> 8;
			U3_DMA_TXBuff[86 + 2*i] = DataTemp & 0xFF;
			
			DataTemp = Sampling_MaxMinData[i][MINDAT];
			U3_DMA_TXBuff[93 + 2*i] = DataTemp >> 8;
			U3_DMA_TXBuff[94 + 2*i] = DataTemp & 0xFF;
		}
	}
	else if(Phase == 9)
	{
	  	//�������ۼӺ�
	  	DataTemp = 0 ; 
		for(i = 0 ; i < (U3_Buffer_Size - 2) ; i++)
		{
			DataTemp += U3_DMA_TXBuff[i] ; 
		}
		U3_DMA_TXBuff[101] = DataTemp >> 8;
		U3_DMA_TXBuff[102] = DataTemp & 0xFF;
	}
	
	if(++Phase > 10) { Phase = 0 ; U3_DMATX_ONOFF() ;} //��ʼ����
}


