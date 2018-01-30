#ifndef  __STARTUP_H_
#define  __STARTUP_H_

#define MAXDAT 0 
#define MIDDAT 1
#define MINDAT 2 

//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//��ֵУ׼ʱ�����ֵ����Сֵ��Чֵ���Լ���λֵ����Ч��Χ
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
#define MAXValue_Min  3095
#define MINValue_Max  1000


//2017��12��6���ٸģ��ſ�AD��λ���ޣ���֮ǰ��2047��200�ſ�2047��350��
//#define AD_MidValue_Min 1847
//#define AD_MidValue_Max 2247
#define AD_MidValue_Min (2047-350)
#define AD_MidValue_Max (2047+350)


//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// �������ֵ ������� ������Сֵ
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
#define Output_Max 2012 
#define Output_Mid 1500
#define Output_Min 988

//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//���������С����ֵ
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
#define THR_Output_Max 1995 
#define THR_Output_Mid 1500
#define THR_Output_Min 1005

//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// �������ֵ ������� ������Сֵ
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
#define Input_Max  4094   //Ϊ��ʹ��λֵ�Ǹ�����(4095��λֵ��С��)
#define Input_Mid  2047
#define Input_Min  0

//�����ת��(ʵ������988+212 = 1200)
#define ESC_StartUp_Value 212

extern uint16_t Sampling_Data[12];
extern uint16_t Sampling_MaxMinData[4][3] ; 
extern uint8_t  Sampling_Offset [4] ; 
extern uint8_t  ChannelInversion_flg ; 

void StartUp(void);
uint16_t Get_SendValue(ChannelTypeDef Channel);
void Init_ChannelDis(bool ONOFF_flg);
void SetOffSet(void);
#endif
