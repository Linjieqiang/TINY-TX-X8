#ifndef _BEEP_H_
#define _BEEP_H_

#define NormalFreCounts      1000
#define PowerOnFreCounts     1650
#define BindFreCounts        512

typedef enum
{
  	__stStop			= 0 , 
	
	__stKeyClick 			= 1 , 					//������ʾ��
	__stMidOffSet  			= 2 , 					//΢����ֵ��
	__stExitStaticOverTime		= 3 , 					//�˳���ֹ��ʱ״̬
	__stMidADAdjustDown		= 4 , 					//��λУ׼���
	
	__stBindWarning 		= 5 , 					//������ʾ��
	__stStaticOverTimeWarning	= 6 , 					//��ʱ����
	__stRFModelLostWarning   	= 7 , 					//����ģ����߱���
	__stThrottleHighWarning		= 8 , 					//����ҡ�˸��ϵ籨��
	__stFastContinumWarning 	= 9 , 					//������������(������У׼���� �� �͵���)
}BeepModeTypeDef ;

void beep_Init(void);
void beepCmd(uint16_t BeepFreCounts , BeepModeTypeDef Mode);
void beepScan(void);

#endif



