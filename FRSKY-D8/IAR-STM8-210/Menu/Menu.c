#include "include.h"

RFHabitTypeDef RFHabit = __AmericaPlayer ; 

MenuCtrlTypeDef  MenuCtrl = 
{
	.RunStep = __stPownOn , 
	.Sub_RunStep = 0 ,
};
RunStatusTypeDef RunStatus = __stNormal ; 					//����״̬(����/��ֹ��ʱ����/����ҡ�˸��ϵ籨��/CYRF6936���߱���/�͵�ѹ����)


//==============================================================================
//�������ع��Ϻ�ͣ�����ﲻ����ִ��(CYRF6936���� / δ������λУ׼(��λУ׼��������))
//==============================================================================
static void Error(void)
{
  	MenuCtrl.RunStep	= __stError ; 
	MenuCtrl.Sub_RunStep 	=  0 ; 
}

static void (*pMENU[])(void) = 
{
	PowerOn , 								// 0   �ϵ���(�ȴ�����ѹ������ҡ��λ�� ���Լ�Ӳ���Ƿ�����)
	StartUp , 								// 1   ң������������
	FTDebug , 								// 2   ��������ģʽ
	Error , 								// 3   ����(ͣ�����ﲻ����ִ��)
};

void menu(void)
{
  	static uint8_t Menu_cnt = 0 ; 
	
	if(Menu_cnt < 5) ++Menu_cnt ; 
	else Menu_cnt = 0 ; 
	
	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	//�ر�ע�� : Ϊ��ʹÿ���ж���Ӧʱ��һ�£�Ӧ��֤ÿ��Ƭ��ִ��ʱ�䣬����С��
	//���жϼ�� , ���Ҵ˳����жϼ����һ�¡����ҪҲ��С�ģ�����
	//�ж��е� SFHSS_CAL ִ�д���Ҳ��Ӱ�� ���˳��
	//0 : Լ 4.5mS        1 : Լ : 287us(�˴���Ҫִ�к���)    2 : Լ : 1.5mS
	//3 : Լ 4.5mS        4 : Լ : 287us(�˴���Ҫִ�к���)    5 : Լ : 1.5mS
	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	if(Menu_cnt == 0)      { StartupAgain_ADC(); beepScan() ; }		//��ʼһ��ADת�� �Լ� ����������				 
	else if(Menu_cnt == 2) { LED_Usart1Send()  ; key_Scan() ; }  		//״ָ̬ʾ����ʾ �Լ� ����ɨ�� 	
	else if(Menu_cnt == 3)   GetADValue() ; 
	else if(Menu_cnt == 5) { BuildUsart3Data() ; Volt_LED(VoltValue) ;} 	//������ �Լ� ���� 3 ��������
	
	pMENU[MenuCtrl.RunStep]();
	ChannelKeyValue = __stEmpty_Key ; 
	OffSetKeyValue  = __stOffset_EmptyKey ;
}


