/*
CC2500ע�� : 
(1)SPI���ݸ�λ��ǰ , abxx xxxx  ���� : a : Ϊ��д��־λ(1 : ��   0 : д)   b : ͻ�����ʱ�־λ     CS���ͺ󣬱���ȴ�SO�������ͣ����ܿ�ʼSPIͨ�š�
*/
#include "include.h"

#define CC25_CSN_on   {GPIOF -> ODR |=  (1<<6);}
#define CC25_CSN_off  {GPIOF -> ODR &= ~(1<<6);}

#define FRSKYD8_CONFIG_CNTS  36
static const uint8_t cc2500_conf[FRSKYD8_CONFIG_CNTS][2]=
{
	{ CC2500_02_IOCFG0   , 0x06 },	// GDO0 ָʾͬ���뷢��/�������
	{ CC2500_00_IOCFG2   , 0x06 },	// GDO2 ָʾͬ���뷢��/�������
	{ CC2500_17_MCSM1    , 0x0c },  // ������һ�����ݰ��������RXģʽ �� ������һ�����ݰ������ IDLE ģʽ
	{ CC2500_18_MCSM0    , 0x18 },  // �� IDLE ���� TX/RX ģʽǰ���Զ�У׼Ƶ��
	{ CC2500_06_PKTLEN   , 0x19 },  // ���ݰ����� : 25 Byte
	{ CC2500_07_PKTCTRL1 , 0x05 },  // �������ݰ���ʱ�Ӱ����� Byte ����(RSSI ��LQI) �� ȷ�ϵ�ַƥ��
	{ CC2500_08_PKTCTRL0 , 0x05 },  // ���ݰ�������ͬ�����һ���ֽھ���(����CRCУ��)
	{ CC2500_3E_PATABLE  , 0xff },  // PA���� : ����PA����书��
	{ CC2500_0B_FSCTRL1  , 0x08 },  // ��ֵƵ�� :   Fif = 26M/1024 * 10 = 203125Hz
	{ CC2500_0C_FSCTRL0  , 0x00 },	// Ƶ�ʲ��� :   0
	{ CC2500_0D_FREQ2    , 0x5c },	// �ز�Ƶ�� :   Fc  = 26M/65536*(0x5C7627) =  24039998474.412109375Hz         
	{ CC2500_0E_FREQ1    , 0x76 },
	{ CC2500_0F_FREQ0    , 0x27 },
	{ CC2500_10_MDMCFG4  , 0xAA },	// �������� :   BW = 26M / 8*(4+2)*4 = 135417 Hz 
	{ CC2500_11_MDMCFG3  , 0x39 },  // ������ :  Rdata = (256 + 0x39)*1024/268435456 * 26M = 31044 Baud   
	{ CC2500_12_MDMCFG2  , 0x11 },  // GFSK  15/16 Bit ͬ����
	{ CC2500_13_MDMCFG1  , 0x23 },  // 2Byte ����   
	{ CC2500_14_MDMCFG0  , 0x7a },  // Ƶ����� : f = 26M/262144 * (256 + 0x7A) * 8 = 299926 Hz
	{ CC2500_15_DEVIATN  , 0x42 },  // MSK ģʽ��Ч
	{ CC2500_1B_AGCCTRL2 , 0x03 },	// bind ? 0x43 : 0x03	
	{ CC2500_19_FOCCFG   , 0x16 },  // Ƶ��ƫ�Ʋ�������
	{ CC2500_1A_BSCFG    , 0x6c },	// ͬ��λ����
	{ CC2500_1C_AGCCTRL1 , 0x40 },  // AGC����  ���λ����ر�  
	{ CC2500_1D_AGCCTRL0 , 0x91 },  
	{ CC2500_21_FREND1   , 0x56 },
	{ CC2500_22_FREND0   , 0x10 },
	{ CC2500_23_FSCAL3   , 0xa9 },
	{ CC2500_24_FSCAL2   , 0x0A },
	{ CC2500_25_FSCAL1   , 0x00 },
	{ CC2500_26_FSCAL0   , 0x11 },
	{ CC2500_29_FSTEST   , 0x59 },
	{ CC2500_2C_TEST2    , 0x88 },
	{ CC2500_2D_TEST1    , 0x31 },
	{ CC2500_2E_TEST0    , 0x0B },
	{ CC2500_03_FIFOTHR  , 0x07 },
	{ CC2500_09_ADDR     , 0x00 }

};

//==============================================================================
//			         ��ʱ����
//==============================================================================
void DelayUs(uint16_t Us)
{
	for( ; Us ; Us--)
	{
		asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");
	}
}

void DelayMs(uint16_t Ms)
{
	for( ; Ms ; Ms--)
	{
		DelayUs(1000) ;
	}
}

//==============================================================================
//CS���ͺ󣬱���ȴ�SO���ͣ��ٿ�ʼͨ��
//==============================================================================
static void CC2500_WaitingReady(void)
{
	uint16_t cnts = 0 ; 
	while ((GPIOG->IDR & (1<<7)) && (++cnts < 10000));
}

//==============================================================================
//			    д�����Ĵ�������
//==============================================================================
void CC2500_WriteReg(uint8_t address, uint8_t data)
{
	CC25_CSN_off;  		DelayUs(2) ;
	CC2500_WaitingReady();	 
	SPI2_wr_Byte(address); 	DelayUs(2) ;
	SPI2_wr_Byte(data);     DelayUs(2) ;
	CC25_CSN_on;
} 

//==============================================================================
//			   д����Ĵ�������
//==============================================================================
static void CC2500_WriteRegisterMulti(uint8_t address, const uint8_t data[], uint8_t length)
{
	CC25_CSN_off;		DelayUs(2) ;
	CC2500_WaitingReady();
	SPI2_wr_Byte(CC2500_WRITE_BURST | address);
	for(uint8_t i = 0; i < length; i++) SPI2_wr_Byte(data[i]);DelayUs(2) ;
	CC25_CSN_on;
}


//==============================================================================
//			    �������Ĵ�������
//==============================================================================
uint8_t CC2500_ReadReg(uint8_t address)
{ 
	uint8_t result;
	CC25_CSN_off;		DelayUs(2) ;	
	CC2500_WaitingReady();
	SPI2_wr_Byte(CC2500_READ_SINGLE | address);
	result = SPI2_rd_Byte(); 
	CC25_CSN_on;
	return(result); 
} 

//==============================================================================
//			     CC2500����
//==============================================================================
void CC2500_Strobe(uint8_t state)
{
	CC25_CSN_off;		
	CC2500_WaitingReady();	DelayUs(2) ;
	SPI2_wr_Byte(state); 	DelayUs(2) ;
	CC25_CSN_on;
}

//==============================================================================
//			   д��������
//==============================================================================
void CC2500_WriteData(uint8_t *dpbuffer, uint8_t len)
{
	CC2500_Strobe(CC2500_SFTX);
	CC2500_WriteRegisterMulti(CC2500_3F_TXFIFO, dpbuffer, len);
	CC2500_Strobe(CC2500_STX);
}

//==============================================================================
//		             ����CC2500����
//==============================================================================
void CC2500_SetPower(uint8_t power)
{
  	static uint8_t prev_power = CC2500_POWER_1 ; 
	if(prev_power != power)
	{	
	  	prev_power = power ; 
		CC2500_WriteReg(CC2500_3E_PATABLE, power);
	}
}

//==============================================================================
//			       CC2500��ʼ��
//==============================================================================
bool  CC2500_Init()
{
  	bool CC2500RestError_flg = false ; 
	SPI2_Init();
	
	//CS_CC2500(PF6)	
	GPIOF -> CR1 |= (1<<6) ; 
	GPIOF -> CR2 |= (1<<6) ; 
	GPIOF -> DDR |= (1<<6) ; 
	CC25_CSN_on;
	
	
	GPIOF -> CR1 |= (1<<5) ; 
	GPIOF -> CR2 |= (1<<5) ; 
	GPIOF -> DDR |= (1<<5) ; 
	
	DelayMs(15);

	//��λ CC2500 
	CC2500_Strobe(CC2500_SRES);
	DelayMs(15);
	
	//��ȡ�Ĵ���ֵ�ͳ�ʼ��ֵ�ȶԣ��ж�оƬ�Ƿ��ʼ���ɹ�
	if(CC2500_ReadReg(CC2500_0E_FREQ1) != 0xC4)  CC2500RestError_flg = true ;
	DelayMs(1);
	DelayMs(10);
	//�����λ�ɹ� , ��ʼ��CC2500 ; ���ʧ�ܣ����ó�ʼ������λ��λʧ�ܱ�־λ��
	if(!CC2500RestError_flg)
	{
		for (uint8_t i = 0 ; i < FRSKYD8_CONFIG_CNTS ; ++i) 
		{
			CC2500_WriteReg(cc2500_conf[i][0], cc2500_conf[i][1]);	//��ʼ�� CC2500 �Ĵ���
			DelayUs(20);
		}
		CC2500_Strobe(CC2500_SIDLE);					//��������״̬
		DelayUs(20);
		CC2500_SetPower(CC2500_POWER_17);				//��ʼ�� ����书��
		CC2500_Strobe(CC2500_SIDLE);					//��������״̬
		DelayUs(20);
		
		//���üĴ�����ɺ�SPI��ȡ�ٶ��л���6Mhz
		SPI2 -> CR1 &= ~(1<<6) ; 
		SPI2 -> CR1 &= ~((1<<5)|(1<<4)|(1<<3)) ; 
		DelayMs (10);
		SPI2 -> CR1 |= (1<<6) ; 
		
	}
	return CC2500RestError_flg ; 
}