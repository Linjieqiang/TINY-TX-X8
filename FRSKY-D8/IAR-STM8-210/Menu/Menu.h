#ifndef _MENU_H
#define _MENU_H_

//ң��������״̬
//ע��״̬�ȼ� : �߼�״̬���Ը��ǵͼ�״̬����֮��Ч��
typedef enum
{
  	__stNormal	= 0 , 
	__stStaTimeOver = 1 ,	//������,��ֹ����15Min,��ʱ����
	__stTHR_err	= 2 , 	//�ϵ�ʱ,���Ŵ��ڸ�λ(�������ֵ����140 960+140 = 1100)
	__stVolt_err 	= 3 , 	//�͵�������
	__stNOAdjust	= 4 , 	//û�н�����λУ׼
	__stRF_err      = 5 ,	//����ģ�����
}RunStatusTypeDef;
extern RunStatusTypeDef RunStatus ; 

typedef enum
{
	__stPownOn	= 0 , 
	__stSarttUp	= 1 ,
	__stFTDebug	= 2 , 
	__stError	= 3 , 
}RunStepTypeDef ;

typedef enum 
{
	__AmericaPlayer = 0 , 
	__JapanPlayer   = 1 , 
}RFHabitTypeDef ; 
extern RFHabitTypeDef RFHabit ; 

typedef struct
{	
	RunStepTypeDef RunStep ; 
	uint8_t Sub_RunStep ; 
}MenuCtrlTypeDef;
extern MenuCtrlTypeDef MenuCtrl ; 


void menu(void);

#endif
