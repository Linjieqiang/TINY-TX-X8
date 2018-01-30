/*******************************************************************************
--------------------------------------------------------------------------------
FRSKY-D8 ע��ļ��� : 
(1) ң������ס������ϵ�,�������״̬(��������1000�����ݰ�(18mS*1000 = 18S)���˳�����״̬)��
(2) ����ң���� ID �� ������һ����Ƶ�б�(��47��Ƶ��)��
(3) ����ʱ ��ַ�̶�Ϊ 0x03  , ������ɺ� ��ַ����Ϊ ң����ID�ŵĺ�8λ
(4) ���FRSKY_D16 ���ݰ��ڼ�����CRCУ�� �� ��������CC2500��Ӳ��CRCУ��
--------------------------------------------------------------------------------
FRSKYD8Э�� :  ������ 36mS(���Ķ�) ����ǰ����Ϊ���ݰ����� �� ���һ��Ϊң�����
               ���� : �̶��� ͨ�� 0 
	       ��Ƶ : �̶� 47 ͨ�� �� ͨ��ң�������ɵ��������������Ƶ������

�������ݰ�(30Byte) :  0    1    2    3    4    5    6    7    8    9    10   11   12   13   14   15   16   17   
                      0x11 0x03 0x01 0xAA 0xBB 0xCC 0xDD 0xEE 0xFF 0xGG 0xHH 0x00 0xJJ 0X00 0x00 0x00 0x00 0x00 
���� : 
[0] : 0x11
[1] : 0x03
[2] : 0x01
[3][4] : ң����ID��  
[5]   [6][7][8][9][10]: [5] Ϊͨ��ǰ����  [6][7][8][9][10] Ϊ [5] ��ʼ �� 5(����[5]) ͨ��Ƶ���б�
[11] -> [16] : �̶�Ϊ 0x00
[17] : 0x01)


�������ݰ�(30Byte) :  0    1    2    3    4    5    6    7    8    9    10   11   12   13   14   15   16   17   
                      0x11 0xAA 0xBB 0xCC 0xDD 0xEE 0xFF 0xGG 0x00 0xHH 0xII 0xJJ 0xKK 0xLL 0xMM 0xNN 0xOO 0xPP 
���� :
[0]    : 0x11
[1][2] : ң���� ID ��
[3]    : ң���û�֡���� (counter = (counter + 1) % 188)
[4]    : 0x00 (�ر�ң��)
[5]    : 0x03
[6]  [7]  [8]  [9]  [10] [11] : Ϊǰ��ͨң������
[12] [13] [14] [15] [16] [17] : Ϊ����ͨң������

--------------------------------------------------------------------------------
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//ĳЩ���ջ���ʱ��Ҫ���ر��ϸ� : �����κ�Ӱ��ʱ��Ĳ����޸��ر�С�ġ�(��ȷ�� uS)
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
*******************************************************************************/
#include "include.h"

#define FRSKYD8_PACKET_LEN  18
#define FRSKYD8_BINDCHANNEL 47 							//��Ƶ�б��47��Ƶ�ι̶�Ϊ����Ƶ�� 0 

bool     Bind_flg = false ; 
uint16_t FRSKYD8_BindCounts = 0 ; 						// �������ݰ����͸���

uint8_t	 FRSKYD8_Counts = 0 ;       						// �������ݰ����					
uint8_t  FRSKYD8_HOPChannel[50] ; 						// ��Ƶ�б�(����ң����ID�����47����Ƶ��(������Ƶ����Ч))	
uint8_t  FRSKYD8_calData[50];							// ��¼��Ƶͨ��Ƶ��ֵ

bool CommunicationError_flg = false ; 
bool HighThrottle_flg = true ; 							//�����ű�־λ
uint16_t TransmitterID ; 							//ң����ΨһID
uint8_t  SendPacket[18] ; 							//�������ݰ�����

typedef enum 
{
	FRSKYD8_BIND  		= 0x00 , 
	FRSKYD8_BIND_PASSBACK	= 0x01 , 
	FRSKYD8_DATA  		= 0x02 ,	
    	FRSKYD8_TUNE  		= 0x03 ,
}FRSKYD8PhaseTypeDef ;
FRSKYD8PhaseTypeDef FRSKYD8Phase = FRSKYD8_DATA ; 

//Channel values are 12-bit values between 988 and 2012, 1500 is the middle.
uint16_t FRSKYD8_SendDataBuff[8]  = { 1500 , 1500 , 988 , 1500 , 1500 , 1500 , 1500 , 1500};

//FRSKYD8 ͨ������˳��
const uint8_t  FRSKYD8_CH_Code[8] = {AILERON, ELEVATOR, THROTTLE, RUDDER, AUX1, AUX2, AUX3, AUX4};

//==============================================================================
//			FRSKYD8 ��ʼ��������ַ
//==============================================================================
static void FRSKYD8_InitDeviceAddr(bool IsBindFlg)
{
	CC2500_WriteReg(CC2500_18_MCSM0,    0x08) ;	
	CC2500_WriteReg(CC2500_09_ADDR , IsBindFlg ? 0x03 : (TransmitterID >> 8)&0xFF );
	CC2500_WriteReg(CC2500_07_PKTCTRL1,0x05);
}

//==============================================================================
//			FRSKYD8 ���÷���ͨ��
//==============================================================================
static void FRSKYD8_TuneChannel(uint8_t Channel)
{
	CC2500_Strobe(CC2500_SIDLE);						//��������״̬
	CC2500_WriteReg(CC2500_25_FSCAL1, FRSKYD8_calData[Channel]);		//���÷���ͨ��
	CC2500_WriteReg(CC2500_0A_CHANNR, FRSKYD8_HOPChannel[Channel]);		//���÷���ͨ��
	CC2500_Strobe(CC2500_SCAL);						//У׼Ƶ�ʺϳ������ر�
}


//==============================================================================
//���� : ��Ϊ�ϵ�ʱ�Ѿ�У׼��Ƶ�ʣ��һض���Ƶ��ֵ�����Դ˴�����ֱ������ǰ��ض���
//Ƶ��ֵ��������Ƶ��У׼ʱ�䡣
//==============================================================================
static void FRSKYD8_tune_chan_fast(void)
{
	CC2500_Strobe(CC2500_SIDLE);
	CC2500_WriteReg(CC2500_0D_FREQ2 , Fre_Carrier_H);
	CC2500_WriteReg(CC2500_0E_FREQ1 , Fre_Carrier_M);
	CC2500_WriteReg(CC2500_0F_FREQ0 , Fre_Carrier_L);
	DelayUs(2);
	CC2500_Strobe(CC2500_SIDLE);
	CC2500_WriteReg(CC2500_0D_FREQ2 , Fre_Carrier_H);
	CC2500_WriteReg(CC2500_0E_FREQ1 , Fre_Carrier_M);
	CC2500_WriteReg(CC2500_0F_FREQ0 , Fre_Carrier_L);
	CC2500_WriteReg(CC2500_25_FSCAL1, FRSKYD8_calData[FRSKYD8_Counts%47]);
	CC2500_WriteReg(CC2500_0A_CHANNR, FRSKYD8_HOPChannel[FRSKYD8_Counts%47]);
}

//==============================================================================
//�������ʽ
//==============================================================================
static void  FRSKYD8_build_Bind_packet(void)
{
  	//�̶���
	SendPacket[0] 	= 0x11;       
	SendPacket[1] 	= 0x03;          
	SendPacket[2] 	= 0x01; 
	
	//ң���� ID ��	
	SendPacket[3]   = (TransmitterID >> 8) & 0xFF  ;
	SendPacket[4]   = TransmitterID & 0xFF ;

	uint8_t  idx 	= (FRSKYD8_BindCounts % 10) * 5 ;
	SendPacket[5]   = idx;	
	SendPacket[6]   = FRSKYD8_HOPChannel[idx++];
	SendPacket[7]   = FRSKYD8_HOPChannel[idx++];
	SendPacket[8]   = FRSKYD8_HOPChannel[idx++];
	SendPacket[9]   = FRSKYD8_HOPChannel[idx++];
	SendPacket[10]  = FRSKYD8_HOPChannel[idx++];
	
	//�̶�Ϊ0
	SendPacket[11]  = 0x00;
	SendPacket[12]  = 0x00; 
	SendPacket[13] 	= 0x00;
	SendPacket[14] 	= 0x00;
	SendPacket[15] 	= 0x00;
	SendPacket[16] 	= 0x00;
	SendPacket[17] 	= 0x01;
}

//==============================================================================
//���ݰ���ʽ
//==============================================================================
static void  FRSKYD8_build_Data_packet(void)
{
	// �̶��� + ң���� ID
	SendPacket[0] 	= 0x11; 
	SendPacket[1]   = (TransmitterID >> 8) & 0xFF  ;
	SendPacket[2]   = TransmitterID & 0xFF ;
	SendPacket[3] 	= FRSKYD8_Counts;
	SendPacket[4] 	= 0x00 ; 
	SendPacket[5] 	= 0x01 ;
	
	SendPacket[10] = 0;
	SendPacket[11] = 0;
	SendPacket[16] = 0;
	SendPacket[17] = 0;
	for(uint8_t i = 0; i < 8; i++)
	{
		uint16_t value = FRSKYD8_SendDataBuff[i] + (uint16_t)((float)FRSKYD8_SendDataBuff[i]/2 + 0.5f) ; 
		if(i < 4)
		{
			SendPacket[6+i] = value & 0xff;
			SendPacket[10+(i>>1)] |= ((value >> 8) & 0x0f) << (4 *(i & 0x01));
		} 
		else
		{
			SendPacket[8+i] = value & 0xff;
			SendPacket[16+((i-4)>>1)] |= ((value >> 8) & 0x0f) << (4 * ((i-4) & 0x01));
		}
	}
}

//==============================================================================
//FRSKYD8 : ���� FRSKYD8 ͨ��(ͨ������õ� 47 ��ͨ�� ����ѯʱ������47��ͨ������Ƶ)
//������Ƶ�μ���� 5 ����
// 1  - 26  : ȡ 16 ��Ƶ��
// 27 - 52  : ȡ 15 ��Ƶ��
// 53 - 76  : ȡ 16 ��Ƶ��
//==============================================================================
void Calc_FRSKYD8_Channel()
{
	uint8_t  idx = 0;
	uint16_t id_tmp = ~ TransmitterID; 					//ID�� ��λȡ��
	
	while(idx < 47)
	{
		uint8_t i;
		uint8_t count_1_26 = 0, count_27_52 = 0, count_53_76 = 0;
		id_tmp = id_tmp * 0x0019660D + 0x3C6EF35F;			// Randomization
		uint8_t next_ch = ((id_tmp >> 8) % 0x4B) + 1;			// Use least-significant byte and must be larger than 1
		for (i = 0; i < idx; i++)
		{
			if(FRSKYD8_HOPChannel[i] == next_ch)    	break;
			if(FRSKYD8_HOPChannel[i] < 27) 			count_1_26++;
			else if (FRSKYD8_HOPChannel[i] < 53)  		count_27_52++;
			else                                    	count_53_76++;
		}
		if (i != idx)  continue;					//˵��û�бȶ���(������һ��Ƶ���ص���������Ƶ��������ѡ��)
		
		//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		// ������Ƶ������Ҫ���� 5 ���� (��һ��Ƶ�������ж�)
		//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		if(idx)
		{
			uint8_t Temp = 0 ; 
			if(next_ch > FRSKYD8_HOPChannel[idx - 1]) 	Temp = next_ch - FRSKYD8_HOPChannel[idx - 1] ; 
			else 						Temp = FRSKYD8_HOPChannel[idx - 1] - next_ch ; 
		  	if(Temp < 5)	continue ; 
		}
		
		if(next_ch == 0)  break; 
		
		// 1  - 26  : ȡ 16 ��Ƶ��
		// 27 - 52  : ȡ 15 ��Ƶ��
		// 53 - 76  : ȡ 16 ��Ƶ��
		if (((next_ch < 27) && (count_1_26 < 16)) || ((next_ch >= 27) && (next_ch < 53) && (count_27_52 < 15)) || ((next_ch >= 53) && (count_53_76 < 16)))
		{
			FRSKYD8_HOPChannel[idx++] = next_ch;
		}
	}
	
	//����Ƶ�ι̶�Ϊ 0 
	FRSKYD8_HOPChannel[FRSKYD8_BINDCHANNEL] = 0 ; 
}

//==============================================================================
//���߷�����ת����
//==============================================================================
uint16_t ReadFRSKYD8(void)
{
  	static bool Test_flg = false ; 
  	static uint8_t Cnts = 0 ; 
	switch(FRSKYD8Phase)
	{
		//���Ͷ������ݰ�
		case FRSKYD8_BIND : 
		  	if(FRSKYD8_BindCounts < 1200)
			{
				FRSKYD8_TuneChannel(FRSKYD8_BINDCHANNEL) ; 
				CC2500_SetPower(CC2500_POWER_3); 		//����ʱ�����书�ʽ���
				CC2500_Strobe(CC2500_SFRX);
		  		FRSKYD8_build_Bind_packet() ;
				CC2500_Strobe(CC2500_SIDLE);
				CC2500_Strobe(CC2500_SFTX);
				CC2500_WriteData(SendPacket, FRSKYD8_PACKET_LEN);
				++FRSKYD8_BindCounts ; 
				Cnts = 0 ; 
				FRSKYD8Phase = FRSKYD8_BIND_PASSBACK ; 
			}  
			else
			{
			  	Bind_flg = false ; 
				FRSKYD8_BindCounts = 0 ; 
				FRSKYD8_Counts = 0 ;
				CC2500_SetPower(CC2500_POWER_17);
				FRSKYD8_InitDeviceAddr(Bind_flg) ;
				
				//ע�� : ֻ����������� �� �������ָʾ����˸ ; �������У׼�� �� ��������һ���ط�����
				if(MenuCtrl.RunStep == __stSarttUp)
				{
					LED_State_Shake &= ~LED_BIND  ; 
					LED_State_ON    |= LED_BIND   ; 
					
					if(RunStatus == __stNormal)
					{
						beepCmd(BindFreCounts , __stStop);
					}
				}
				
				FRSKYD8Phase = FRSKYD8_DATA ; 
				
			}
			return 2443 ;
		
		//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		// ���ڽ��������� 9mS������ 4�� 2mS(����ң����ʱ��)
		// ʵ��Э���в�����Ҫ�˲�����
		//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		case FRSKYD8_BIND_PASSBACK :
		  	if(Cnts < 3)
			{
				++Cnts ; 
			}
			else 
			{
				FRSKYD8Phase = FRSKYD8_BIND ;
			}
		  	return 2634 ;
		  
		//�������ݰ�
		case FRSKYD8_DATA : 
		  	if(Test_flg == false) 
			{
				Test_flg = true ; 
				GPIOF -> ODR |= (1<<5) ; 
			}
			else
			{
				Test_flg = false ; 
				GPIOF -> ODR &= ~(1<<5) ; 
			}
		  
			FRSKYD8_Counts = (FRSKYD8_Counts + 1) % 188 ; 
			FRSKYD8_tune_chan_fast();
			FRSKYD8_build_Data_packet();
			CC2500_Strobe(CC2500_SIDLE);	
			CC2500_WriteData(SendPacket, FRSKYD8_PACKET_LEN);
			FRSKYD8Phase = FRSKYD8_TUNE ;
			Cnts = 0 ; 
			if((FRSKYD8_Counts % 4) == 2)
			{
				return 680 ;  
			}
			else if((FRSKYD8_Counts % 4) == 3)
			{
				return 2340 ;  
			}
			else 	return 1520 ;  
			
		//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		// ���ڽ����ݰ���� 9mS������ 4�� 2mS(����ң����ʱ��)
		// ʵ��Э���в�����Ҫ�˲�����
		//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		case FRSKYD8_TUNE :
		  	if(Cnts < 3)
			{
				++Cnts ; 
			}
			else 
			{
				FRSKYD8Phase = FRSKYD8_DATA ;
			}
			return 2550 ;
		  
		  
	}
	return 0 ; 
}


void SetBind(void)
{
  	FRSKYD8_BindCounts = 0 ; 
	FRSKYD8Phase = FRSKYD8_BIND ;
	FRSKYD8_TuneChannel(FRSKYD8_HOPChannel[FRSKYD8_BINDCHANNEL]) ; 
}

//==============================================================================
//			FRSKYD16 ��ʼ��
//==============================================================================
void initFRSKYD8(void)
{
  	//��ȡ ң���� ID ��
  	TransmitterID = GetUniqueID();

	//ͨ�� ң���� ID �ţ������ 47 ����Ƶ�� 
	Calc_FRSKYD8_Channel();
	
	//��ʼ�� CC2500 , ���س�ʼ���Ƿ�ɹ���־λ
	CommunicationError_flg = CC2500_Init() ; 
	if(CommunicationError_flg == true)
	{
	  	//���߳�ʼ��ʧ�ܣ����ϱ���
	  	if(RunStatus < __stRF_err)				//״̬����ǰ��Ҫ�ж�״̬�ȼ����Ƿ����(���򲻸���,����ʾ)
		{
			beepCmd(NormalFreCounts , __stRFModelLostWarning);
			RunStatus = __stRF_err ; 
			LED_State_ON = LED_NONE ; 
			LED_State_Shake = LED_ALL ; //����ͨ��LED��˸
		}
		
		//======================================================
		//��ת�����ϱ���״̬
		//======================================================
		MenuCtrl.RunStep = __stError ;
		MenuCtrl.Sub_RunStep = 0 ; 
	}
	else
	{
		// У׼ ��ͨ��Ƶ��ֵ
		for (uint8_t i = 0 ; i < 48 ; i++)
		{
			CC2500_Strobe(CC2500_SIDLE);
			CC2500_WriteReg(CC2500_0A_CHANNR , FRSKYD8_HOPChannel[i]);
			CC2500_Strobe(CC2500_SCAL);
			DelayUs(1000);
			FRSKYD8_calData[i]  =  CC2500_ReadReg(CC2500_25_FSCAL1);
		}
		
		
		//��ס���밴���ϵ� �������ģʽ������
		if(!(GPIOE -> IDR & (1<<7)))
		{
			Bind_flg = true ;
		}
		
		FRSKYD8_InitDeviceAddr(Bind_flg);
		FRSKYD8Phase = FRSKYD8_DATA ;
		FRSKYD8_TuneChannel(FRSKYD8_HOPChannel[0]) ; 
	}
	
	
	
}