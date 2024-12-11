/********************************** (C) COPYRIGHT *******************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for 
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************
 */

/******************************************************************************/
/* Header file contains */

#include "octopus_platform.h"
#include "octopus_log.h"
#include "octopus_sif.h"

/********************************** (C) COPYRIGHT *******************************
 * CONSTANTS
 */

#define SIF_LOW                    			0      	//�͵�ƽ
#define SIF_HIGH                    		1      	//�ߵ�ƽ

#define SIF_REV_SYNC_L_TIME_NUM         500     //ͬ���źŵ͵�ƽʱ�䣺30ms = 30000us / 50us = 600

#define SIF_REV_SYNC_H_TIME_NUM_MIN     18      //ͬ���źŸߵ�ƽ��Сʱ�䣺1000-100us = 900us / 50us = 18
#define SIF_REV_SYNC_H_TIME_NUM_MAX     42      //ͬ���źŸߵ�ƽ���ʱ�䣺1000+100us = 1000us / 50us = 22

#define SIF_REV_SHORT_TIME_NUM_MIN      18      //һ���߼������ж̵�ʱ����Сֵ��1000-100us = 900us / 50us = 18
#define SIF_REV_SHORT_TIME_NUM_MAX      22      //һ���߼������ж̵�ʱ�����ֵ��1000+100us = 1000us / 50us = 22

#define SIF_REV_LONG_TIME_NUM_MIN       36      //һ���߼������г���ʱ����Сֵ��2ms-200us = 1800us / 50us = 36
#define SIF_REV_LONG_TIME_NUM_MAX       44      //һ���߼������г���ʱ�����ֵ��2ms+200us = 2200us / 50us = 44

#define SIF_REV_LOGIC_CYCLE_NUM_MIN     54      //һ���߼�������Сʱ�䣺3ms-300us = 2700us / 50us = 54
#define SIF_REV_LOGIC_CYCLE_NUM_MAX     66      //һ���߼��������ʱ�䣺3ms+300us = 3300us / 50us = 66

#define SIF_REV_HALF_LOGIC_CYCLE_MIN    27      //һ���߼����ڵ�1/2��Сʱ�䣺1.5ms-150us = 1350us / 50us = 27
#define SIF_REV_HALF_LOGIC_CYCLE_MAX    33      //һ���߼����ڵ�1/2���ʱ�䣺1.5ms+150us = 1650us / 50us = 33

#define SIF_REV_END_SIGNAL_TIME_NUM     60      //�����źŵ�ƽʱ�䣺5ms�͵�ƽ + Nms�ߵ�ƽ��ʵ�ʼ��5ms�͵�ƽ���У�һ֡���ݷ�����ɺ���5ms�͵�ƽ�ʹ�������ˣ��������ݵ�ʱ����������������

#define SIF_REV_BIT_NUM                 8       //���յ�bitλ������1�ֽ�=8bit

#define SIF_REV_DATA_NUM                12      //���յ����ݸ���

#define SIF_SEND_SYNC_L_TIME_NUM        600     //ͬ���źŵ͵�ƽʱ�䣺30ms = 30000us / 50us = 600
#define SIF_SEND_SYNC_H_TIME_NUM        10      //ͬ���źŸߵ�ƽʱ�䣺500us = 500us / 50us = 10
#define SIF_SEND_SHORT_TIME_NUM         10      //һ���߼������ж̵�ʱ�䣺500us = 500us / 50us = 10
#define SIF_SEND_LONG_TIME_NUM          20      //һ���߼������г���ʱ�䣺1ms = 1000us / 50us = 20

#define SIF_SEND_BIT_NUM                8       //���͵�bitλ������1�ֽ�=8bit

#define SIF_SEND_DATA_MAX_NUM           12      //���͵�����������

/***************************************************************************************************
 *                                           GLOBAL VARIABLES
 ***************************************************************************************************/

/* ���Ͷ��� -------------------------------------------------------------------*/
typedef enum
{
	SIF_REV_STATE_INITIAL = 0,    //��ʼ״̬���ȴ�����ͬ���ź�
	SIF_REV_STATE_SYNC_L = 1,    //����ͬ���͵�ƽ�ź�״̬
	SIF_REV_STATE_SYNC_H = 2,    //����ͬ���ߵ�ƽ�ź�״̬
	SIF_REV_STATE_REV_BIT = 3,    //��ȡ�������ƽ״̬
	SIF_REV_STATE_BUILD_DATA = 4,    //�����ֽ�����
	SIF_REV_STATE_END_SIGNAL = 5,    //���ս�����ƽ�ź�״̬
	SIF_REV_STATE_IDLE_SIGNAL = 6,
	SIF_REV_STATE_RESTART = 7,     //���չ��̳������½���״̬
	SIF_REV_STATE_STOP = 8
} SIF_REV_STATE_T;   //��������״̬ö��

typedef enum
{
	SIF_SEND_STATE_INITIAL = 0,    //��ʼ״̬���ȴ��������ݱ�־
	SIF_SEND_STATE_SYNC_L = 1,    //����ͬ���͵�ƽ�ź�״̬
	SIF_SEND_STATE_SYNC_H = 2,    //����ͬ���ߵ�ƽ�ź�״̬
	SIF_SEND_STATE_GET_BIT = 3,    //��ȡ����λ
	SIF_SEND_STATE_BIT_L = 4,    //����������͵�ƽ�ź�״̬
	SIF_SEND_STATE_BIT_H = 5,    //����������ߵ�ƽ�ź�״̬
	SIF_SEND_STATE_CHECK_BIT = 6,    //�������λ״̬
	SIF_SEND_STATE_END_SIGNAL = 7,    //���ͽ�����ƽ�ź�״̬

} SIF_SEND_STATE_T;  //��������״̬ö��

/* ���ձ������� -------------------------------------------------------------------*/
uint32_t sif_receive_H_L_Level_time_cnt = 0; //�ߵ͵�ƽʱ�����
uint8_t sif_receive_start_H_L_Level_timming_flag = 0; //��ʼ�ߵ͵�ƽ��ʱ���

uint8_t sif_receive_state = SIF_REV_STATE_IDLE_SIGNAL;      //��������״̬
uint8_t sif_receive_bit_num = 0;    //���յ�bitλ����
uint8_t sif_receive_data_num = 0;   //���յ����ݸ���

//�������ݻ�������-��һ���������������ݣ�51�������ֽ�
uint8_t sif_receive_data_buf[SIF_REV_DATA_NUM] = { 0 };
uint8_t sif_receive_buf[SIF_REV_DATA_NUM] = { 0 };

uint8_t sif_receive_has_read_bit = 0;               //1-�Ѿ���ȡһ��bitλ
uint8_t sif_receive_check_OK = 0;                   //1-У�����ȷ��0-У���ʧ��
uint8_t sif_receive_read_success = 0;                 //һ֡�����Ƿ��ȡ�ɹ���0-���ɹ���1-�ɹ�

/* ���ͱ������� -------------------------------------------------------------------*/
uint32_t sif_send_H_L_Level_time_cnt = 0; //�ߵ͵�ƽʱ�����
uint8_t sif_send_start_H_L_Level_timming_flag = 0; //��ʼ�ߵ͵�ƽ��ʱ���

uint8_t sif_send_state = 0;      //��������״̬
uint8_t sif_send_bit_num = 0;    //���յ�bitλ����
uint8_t sif_send_data_num = 0;   //���յ����ݸ���

uint8_t sif_send_bit = 0;               //1-�Ѿ���ȡһ��bitλ

uint8_t sif_send_data_buf[SIF_SEND_DATA_MAX_NUM] = { 0 };
uint8_t sif_send_data_num_target = 0;
uint8_t sif_send_req_flag = 0;               //���ڷ�������

static bool sif_has_inited = false;
/*************************************************************************************************
 * GLOBAL FUNCTIONS DECLEAR
 */
/*************************************************************************************************/

static void Sif_GPIO_Init(void);                //GPIO��ʼ������
static uint8_t Sif_Get_Input_Pin_Data(void);    //��ȡSIF���ն˿ڵ�ƽ������������
static void Sif_Receive_Data_Handle(void);      //�������ݴ�������У׼λ��������������Ӧ
static void Sif_Send_Data_Handle(void);         //�������ݴ�������У׼λ��������������Ӧ
static void Sif_Checksum_Handle(void);         //У��ʹ���

bool Sif_Is_Idle(void);
/***************************************************************************************************
 *                                            LOCAL FUNCTION
 ***************************************************************************************************/
bool Sif_IsInit(void)
{
  return sif_has_inited;
}

void Sif_GPIO_Init(void)
{
	SIF_SEND_DATA_BIT_LOW();
}

void Sif_Init(void)
{
if (!sif_has_inited)
	{
		Sif_GPIO_Init();
		///Sif_TIM2_Int_Init(50 - 1, 96 - 1);
		sif_has_inited = true;
	}
}

void Sif_DeInit(void)
{
if (sif_has_inited)
	{
		///TIM_ITConfig(TIM2, TIM_IT_Update, DISABLE);
		///TIM_Cmd(TIM2, DISABLE);
		sif_has_inited = false;
	}
}

void Sif_ReStart()
{
	sif_receive_state = SIF_REV_STATE_INITIAL;
}

bool Sif_Is_Idle(void)
{
	if (sif_receive_state == SIF_REV_STATE_STOP)
		return 1;
	else
		return 0;
}

void Sif_Delay_us(uint32_t us) {
  DELAY_US(us);  
}

uint8_t Sif_ReadData(uint8_t* data, uint8_t maxlen)
{
	assert(data);
	if (!sif_has_inited)
	{
		return 0;
	}

	if (sif_receive_read_success && sif_receive_check_OK)
	{
		uint8_t len = maxlen < SIF_REV_DATA_NUM ? maxlen : SIF_REV_DATA_NUM;
		for (int i = 0; i < len; i++)
		{
			data[i] = sif_receive_buf[i];
		}
		sif_receive_read_success = false;
		return len;
	}
	return 0;
}

uint8_t Sif_SendData_Sync(uint8_t* data, uint8_t len)
{
	assert(data);
	assert(len);

	if (!sif_has_inited)
	{
		return 0;
	}

	uint8_t byte = 0;
	uint8_t flag = 0;
	//ͬ���ź�
	SIF_SEND_DATA_BIT_LOW();
	Sif_Delay_us(20000);
	SIF_SEND_DATA_BIT_HIGH();
	Sif_Delay_us(500);
	for (int i = 0; i < len; i++)
	{
		byte = data[i];
		for (int j = 0; j < 8; j++)
		{
			flag = byte & (0x80 >> j);
			if (flag)
			{
				SIF_SEND_DATA_BIT_LOW();
				Sif_Delay_us(500);
				SIF_SEND_DATA_BIT_HIGH();
				Sif_Delay_us(1000);
			}
			else
			{
				SIF_SEND_DATA_BIT_LOW();
				Sif_Delay_us(1000);
				SIF_SEND_DATA_BIT_HIGH();
				Sif_Delay_us(500);
			}
		}
	}
	SIF_SEND_DATA_BIT_LOW();
	Sif_Delay_us(5000);
	return len;
}

uint8_t Sif_SendData(uint8_t* data, uint8_t len)
{
	#if 1
	LOG_LEVEL("Sif_SendData:");
	for (int i = 0; i < len; i++)
	{
		LOG_LEVEL("%02x ", data[i]);
	}
	LOG_LEVEL("\r\n");
	#endif
	assert(data);
	assert(len);
	assert(len <= SIF_SEND_DATA_MAX_NUM);
	if (!sif_has_inited)
	{
		return 0;
	}

	if (sif_send_req_flag == false)
	{
		for (int i = 0; i < len; i++)
		{
			sif_send_data_buf[i] = data[i];
		}
		sif_send_data_num_target = len;
		sif_send_req_flag = true;
		return len;
	}
	return 0;
}

/*********************************************************************
 * @fn      TIM2_IRQHandler
 *
 * @brief   This function handles TIM2 global interrupt request.
 *
 * @return  none
 */

void SIF_IO_IRQHandler(void)
{
		//SIF ���մ���
		if (sif_receive_start_H_L_Level_timming_flag == 1) //��ʼ�ߵ͵�ƽ��ʱ���
		{
			sif_receive_H_L_Level_time_cnt++;     //�ߵ͵�ƽά��ʱ���������
		}
		/////////////////////////////////////////////////////////////////////////////////////

		Sif_Receive_Data_Handle();      //�������ݴ���

		/////////////////////////////////////////////////////////////////////////////////////
		//SIF ���ʹ���
		#if 1
		if (sif_send_start_H_L_Level_timming_flag == 1)
		{
			sif_send_H_L_Level_time_cnt++;     //�ߵ͵�ƽά��ʱ���������
		}
		Sif_Send_Data_Handle();     //�������ݴ���
		#endif
    /////////////////////////////////////////////////////////////////////////////////////
}

///__attribute__((section(".highcode")))
void Sif_Receive_Data_Handle(void)
{

	switch (sif_receive_state)													//��⵱ǰ��������״̬
	{
	case SIF_REV_STATE_INITIAL:                         //��ʼ״̬��δ���յ�ͬ����Ϣ������ͬ���ж�
		if (Sif_Get_Input_Pin_Data() == SIF_LOW)          //�жϽ������ŵĵ�ƽ״̬���������͵�ƽʱ����ʼ��ʱ
		{
			sif_receive_bit_num = 0;                				//����bitλ������
			sif_receive_data_num = 0;               				//���ý������ݸ���
			sif_receive_H_L_Level_time_cnt = 0;             //�ߵ͵�ƽ��ʱ������0
			sif_receive_start_H_L_Level_timming_flag = 1;   //��ʼ�ߵ͵�ƽ��ʱ
			sif_receive_state = SIF_REV_STATE_SYNC_L;       //�����ȡͬ���͵�ƽ�ź�״̬

			memset(sif_receive_data_buf, 0, SIF_REV_DATA_NUM);
		}
		break;

	case SIF_REV_STATE_SYNC_L:                          //�ڶ�ȡͬ���͵�ƽ�ź��ڼ�
		if (Sif_Get_Input_Pin_Data() == SIF_HIGH)         //ͬ���źŵ͵�ƽ����ڼ�����ߵ�ƽ
		{
			if (sif_receive_H_L_Level_time_cnt >= SIF_REV_SYNC_L_TIME_NUM) //���ͬ���źŵ͵�ƽʱ��>=SYNC_L_TIME_NUM
			{                                       				//ͬ���źŵ͵�ƽʱ��Ҫ>=10ms
				sif_receive_H_L_Level_time_cnt = 0;         	//�ߵ͵�ƽ��ʱ������0
				sif_receive_state = SIF_REV_STATE_SYNC_H;   	//�����ȡͬ���źŸߵ�ƽ״̬
			}
			else
			{
				sif_receive_state = SIF_REV_STATE_IDLE_SIGNAL; //�������½���״̬
			}
		}
		break;

	case SIF_REV_STATE_SYNC_H:                          //�ڶ�ȡͬ���źŸߵ�ƽ�ڼ�
		if (Sif_Get_Input_Pin_Data() == SIF_LOW)          //ͬ���źŸߵ�ƽ����ڼ�����͵�ƽ
		{
			//�ж�ͬ���źŸߵ�ƽʱ���Ƿ���1ms��100us֮��
			if (sif_receive_H_L_Level_time_cnt >= SIF_REV_SYNC_H_TIME_NUM_MIN
					&& sif_receive_H_L_Level_time_cnt <= SIF_REV_SYNC_H_TIME_NUM_MAX)
			{
				sif_receive_H_L_Level_time_cnt = 0;          //�ߵ͵�ƽ��ʱ������0
				sif_receive_has_read_bit = 0;
				sif_receive_state = SIF_REV_STATE_REV_BIT;   //�����ȡ����״̬
			}
			else
			{
				sif_receive_state = SIF_REV_STATE_IDLE_SIGNAL;//�������½���״̬
			}
		}
		else            																	//�����ͬ���źŸߵ�ƽ����ڼ䣬ʱ�䳬��2ms��200us����Ϊ��ʱ
		{
			//�ж�ʱ���Ƿ�ʱ 2ms��200us
			if (sif_receive_H_L_Level_time_cnt >= SIF_REV_LOGIC_CYCLE_NUM_MAX)
			{
				sif_receive_state = SIF_REV_STATE_IDLE_SIGNAL;      //�������½���״̬
			}
		}
		break;

	case SIF_REV_STATE_REV_BIT:          //�ڶ�ȡ�������ƽ�ڼ�
		if (Sif_Get_Input_Pin_Data() == SIF_HIGH)               //ͬ���źŵ͵�ƽ����ڼ�����ߵ�ƽ
		{
			//�ж�ʱ���Ƿ�ʱ 2ms��200us
			if (sif_receive_H_L_Level_time_cnt >= SIF_REV_LOGIC_CYCLE_NUM_MAX)
			{
				sif_receive_state = SIF_REV_STATE_IDLE_SIGNAL;      //�������½���״̬
			}
			else
			{
				if (sif_receive_has_read_bit == 0)
				{
					if ((sif_receive_H_L_Level_time_cnt < (SIF_REV_HALF_LOGIC_CYCLE_MIN)))
					{
						sif_receive_data_buf[sif_receive_data_num] |= 0x01;
					}
					sif_receive_has_read_bit = 1;
					sif_receive_state = SIF_REV_STATE_BUILD_DATA;
				}
			}
		}
		else
		{
			if (sif_receive_H_L_Level_time_cnt >= SIF_REV_LOGIC_CYCLE_NUM_MAX)
			{
				sif_receive_state = SIF_REV_STATE_IDLE_SIGNAL;      //�������½���״̬
			}
		}
		break;

	case SIF_REV_STATE_BUILD_DATA:
		if (Sif_Get_Input_Pin_Data() == SIF_LOW)                //ͬ���źŸߵ�ƽ����ڼ�����͵�ƽ
		{
			//�ж�ͬ���źŸߵ�ƽʱ���Ƿ���1ms��100us֮��
			if (sif_receive_has_read_bit == 1)
			{
				sif_receive_H_L_Level_time_cnt = 0;         //�ߵ͵�ƽ��ʱ������0

				sif_receive_has_read_bit = 0;                   //��0����ȡ��һ��bitλ
				sif_receive_bit_num++;                  //���յ�bit��++

				if (sif_receive_bit_num == SIF_REV_BIT_NUM)   //���һ���ֽ�8��bitλ�������
				{
					sif_receive_data_num++;             //���յ����ݸ���++
					sif_receive_bit_num = 0;            //����bitλ������0���½���
					if (sif_receive_data_num == SIF_REV_DATA_NUM)   //������ݲɼ����
					{
						sif_receive_state = SIF_REV_STATE_END_SIGNAL; //������ս����͵�ƽ�ź�״̬
					}
					else
					{
						sif_receive_state = SIF_REV_STATE_REV_BIT; //�����ȡ����״̬
					}
				}
				else                                //���һ���ֽ�8��bitλ��û�н������
				{
					//���������ݻ�������һλ�����ݴӵ�bitλ��ʼ����
					sif_receive_data_buf[sif_receive_data_num] =
							sif_receive_data_buf[sif_receive_data_num] << 1;
					sif_receive_state = SIF_REV_STATE_REV_BIT; //�����ȡ����״̬
				}
			}
		}
		else            //�����ͬ���źŸߵ�ƽ����ڼ䣬ʱ�䳬��2ms��200us����Ϊ��ʱ
		{
			//�ж�ʱ���Ƿ�ʱ 2ms��200us
			if (sif_receive_H_L_Level_time_cnt >= SIF_REV_LOGIC_CYCLE_NUM_MAX)
			{
				sif_receive_state = SIF_REV_STATE_IDLE_SIGNAL;      //�������½���״̬
			}
		}
		break;

	case SIF_REV_STATE_END_SIGNAL:                              //�ڽ��ս����źŵ͵�ƽ�ڼ�
		if (Sif_Get_Input_Pin_Data() == SIF_LOW)
		{
			if (sif_receive_H_L_Level_time_cnt >= SIF_REV_END_SIGNAL_TIME_NUM) //��������͵�ƽʱ��>=5ms
			{

				memcpy(sif_receive_buf, sif_receive_data_buf, SIF_REV_DATA_NUM);
				Sif_Checksum_Handle();

				sif_receive_read_success = 1;                   //һ֡���ݶ�ȡ�ɹ�
				sif_receive_start_H_L_Level_timming_flag = 0;   //ֹͣ�ߵ͵�ƽ��ʱ
				sif_receive_H_L_Level_time_cnt = 0;             //��ʱ������ֵ��0
				sif_receive_state = SIF_REV_STATE_INITIAL;      //����״̬��0
			}
		}
		else    //�����źŵ͵�ƽ����ڼ�һֱΪ��
		{
			if (sif_receive_H_L_Level_time_cnt >= SIF_REV_LOGIC_CYCLE_NUM_MAX) //��������͵�ƽʱ��>=10ms����Ϊ��ʱ
			{                               //һ֡���ݷ�����ɺ���Ҫ���50ms�ŷ��͵ڶ�֡���ݣ��ڼ�϶��ᱻ����
				sif_receive_state = SIF_REV_STATE_IDLE_SIGNAL;      //�������½���״̬
			}
		}
		break;
	#if 1
	case SIF_REV_STATE_IDLE_SIGNAL:
		//sif_receive_state = SIF_REV_STATE_RESTART;
		//break;
	case SIF_REV_STATE_RESTART:                      		//���½�������״̬
		sif_receive_start_H_L_Level_timming_flag = 0;     //ֹͣ�ߵ͵�ƽ��ʱ
		sif_receive_H_L_Level_time_cnt = 0;          			//��ʱ������ֵ��0
		sif_receive_has_read_bit = 0;                			//��0����ȡ��һ��bitλ
		sif_receive_bit_num = 0;
		sif_receive_data_num = 0;
		sif_receive_state = SIF_REV_STATE_INITIAL;        //����״̬��0
		break;
	#endif

	}

}

///__attribute__((section(".highcode")))
void Sif_Send_Data_Handle(void)
{
	switch (sif_send_state)															 //��⵱ǰ��������״̬
	{
	case SIF_SEND_STATE_INITIAL:                         //��ʼ״̬��δ���յ�ͬ����Ϣ������ͬ���ж�
		if (sif_send_req_flag == true)             			   //�жϽ������ŵĵ�ƽ״̬���������͵�ƽʱ����ʼ��ʱ
		{
			sif_send_bit_num = 0;                						 //����bitλ������
			sif_send_data_num = 0;               						 //���ý������ݸ���
			sif_send_H_L_Level_time_cnt = 0;                 //�ߵ͵�ƽ��ʱ������0
			sif_send_start_H_L_Level_timming_flag = 1;       //��ʼ�ߵ͵�ƽ��ʱ
			sif_send_state = SIF_SEND_STATE_SYNC_L;          //�����ȡͬ���͵�ƽ�ź�״̬
			SIF_SEND_DATA_BIT_LOW();
		}
		break;

	case SIF_SEND_STATE_SYNC_L:                          //�ڶ�ȡͬ���͵�ƽ�ź��ڼ�
		if (sif_send_H_L_Level_time_cnt >= SIF_SEND_SYNC_L_TIME_NUM) //���ͬ���źŵ͵�ƽʱ��>=SYNC_L_TIME_NUM
		{                                                  //ͬ���źŵ͵�ƽʱ��Ҫ>=10ms
			sif_send_H_L_Level_time_cnt = 0;                 //�ߵ͵�ƽ��ʱ������0
			SIF_SEND_DATA_BIT_HIGH();
			sif_send_state = SIF_SEND_STATE_SYNC_H;          //�����ȡͬ���źŸߵ�ƽ״̬
		}
		break;

	case SIF_SEND_STATE_SYNC_H:                          //�ڶ�ȡͬ���źŸߵ�ƽ�ڼ�
		//�ж�ͬ���źŸߵ�ƽʱ���Ƿ���1ms��100us֮��
		if (sif_send_H_L_Level_time_cnt >= SIF_SEND_SYNC_H_TIME_NUM)
		{
			sif_send_H_L_Level_time_cnt = 0;                 //�ߵ͵�ƽ��ʱ������0
			SIF_SEND_DATA_BIT_LOW();
			sif_send_state = SIF_SEND_STATE_GET_BIT;         //�����ȡ����״̬
		}
		break;

	case SIF_SEND_STATE_GET_BIT:                         //�ڶ�ȡ�������ƽ�ڼ�
		sif_send_H_L_Level_time_cnt = 0;
		sif_send_bit = sif_send_data_buf[sif_send_data_num] & (0x80 >> sif_send_bit_num);
		sif_send_state = SIF_SEND_STATE_BIT_L; 						 //�����ȡ����״̬
		break;

	case SIF_SEND_STATE_BIT_L:
		if (sif_send_bit)
		{
			if (sif_send_H_L_Level_time_cnt >= SIF_SEND_SHORT_TIME_NUM)
			{
				sif_send_H_L_Level_time_cnt = 0;
				SIF_SEND_DATA_BIT_HIGH();
				sif_send_state = SIF_SEND_STATE_BIT_H;      	 //�������½���״̬
			}
		}
		else
		{
			if (sif_send_H_L_Level_time_cnt >= SIF_SEND_LONG_TIME_NUM)
			{
				sif_send_H_L_Level_time_cnt = 0;
				SIF_SEND_DATA_BIT_HIGH();
				sif_send_state = SIF_SEND_STATE_BIT_H;      		//�������½���״̬
			}
		}
		break;

	case SIF_SEND_STATE_BIT_H:
		if (sif_send_bit)
		{
			if (sif_send_H_L_Level_time_cnt >= SIF_SEND_LONG_TIME_NUM)
			{
				sif_send_H_L_Level_time_cnt = 0;
				SIF_SEND_DATA_BIT_LOW();
				sif_send_state = SIF_SEND_STATE_CHECK_BIT;      //�������½���״̬
			}
		}
		else
		{
			if (sif_send_H_L_Level_time_cnt >= SIF_SEND_SHORT_TIME_NUM)
			{
				sif_send_H_L_Level_time_cnt = 0;
				SIF_SEND_DATA_BIT_LOW();
				sif_send_state = SIF_SEND_STATE_CHECK_BIT;      //�������½���״̬
			}
		}
		break;

	case SIF_SEND_STATE_CHECK_BIT:
		sif_send_bit_num++;
		if (sif_send_bit_num == SIF_SEND_BIT_NUM)   				//���һ���ֽ�8��bitλ�������
		{
			sif_send_data_num++;             									//���յ����ݸ���++
			sif_send_bit_num = 0;            									//����bitλ������0���½���
			if (sif_send_data_num == sif_send_data_num_target)//������ݲɼ����
			{
				sif_send_state = SIF_SEND_STATE_END_SIGNAL;   	//������ս����͵�ƽ�ź�״̬
			}
			else
			{
				SIF_SEND_DATA_BIT_LOW();
				sif_send_state = SIF_SEND_STATE_GET_BIT; 				//�����ȡ����״̬
			}
		}
		else                                								//���һ���ֽ�8��bitλ��û�н������
		{
			SIF_SEND_DATA_BIT_LOW();
			sif_send_state = SIF_SEND_STATE_GET_BIT; 					//�����ȡ����״̬
		}
		break;

	case SIF_SEND_STATE_END_SIGNAL:                       //�ڽ��ս����źŵ͵�ƽ�ڼ�
		sif_send_req_flag = false;
		sif_send_start_H_L_Level_timming_flag = 0;       		//ֹͣ�ߵ͵�ƽ��ʱ
		sif_send_H_L_Level_time_cnt = 0;                 		//��ʱ������ֵ��0
		sif_send_bit_num = 0;
		sif_send_data_num = 0;
		sif_send_state = SIF_SEND_STATE_INITIAL;          	//����״̬��0
		break;
	}
}

/*******************************************************************************
 *�������� : Check_Sum_Handle
 *�������� : У��ʹ���
 *������� : void
 *������� : void
 *******************************************************************************/
///__attribute__((section(".highcode")))
void Sif_Checksum_Handle(void)
{
	uint8_t checkByte = 0;
	uint64_t checkXor = 0;
	for (int i = 0; i < (SIF_REV_DATA_NUM - 1); i++)
	{
		checkXor = checkXor ^ sif_receive_buf[i];
	}

	checkByte = (unsigned char) checkXor;
	if (checkByte == sif_receive_buf[SIF_REV_DATA_NUM - 1])//У�����ȷ
	{
		sif_receive_check_OK = 1;           								 //���У��ɹ�
	}
	else
	{
		sif_receive_check_OK = 0;           								 //���У��ʧ��
	}
}

///__attribute__((section(".highcode")))
uint8_t Sif_Get_Input_Pin_Data(void)
{
	static uint8_t flag = 0;
	static uint8_t value = 0;
	static uint8_t ret = 0;
	uint8_t cur = SIF_RECEIVE_DATA_BIT();
	if (value != cur)
	{
		flag = 3;
		value = cur;
	}
	if (flag)
	{
		flag--;
		if (flag == 0)
		{
			ret = value;
		}
	}
	return ret;
}
