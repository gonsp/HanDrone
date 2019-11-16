/*******************************************************************************
--------------------------------------------------------------------------------
ADCͨ����Ӧ���� : 
ADC11(PB7) -> Random          ADC13(PB5) -> RUD          ADC14(PB4) -> THR(ELE)
ADC15(PB3) -> ELE(THR)        ADC16(PB2) -> AIL          ADC17(PB1) -> BAT
ADC18(PB0) -> REF             
�ڲ��ο���
--------------------------------------------------------------------------------

--------------------------------------------------------------------------------
*******************************************************************************/
#include "include.h"

#define  ADC1_DR_Address                    (ADC1_BASE + 0x04)
#define  ADC1_Buffer_Size                    8

uint16_t ADC_Value[30] ;							//ADC DMA �ڴ滺����		
bool     ISCJ431OKFlg = true ; 							//431 ״̬��־λ(1: ����   0: �쳣)
float    VoltValue = 0.0f ;							//��ص��?


//==============================================================================
//���� ADC
//(1)���ADC���?(ADCֵ�����Ѿ���λ)�����³�ʼ�� DMA ADC��
//(2)ʹ�ܵ���ADCת����
//==============================================================================
void StartupAgain_ADC(void)
{
  	//�ж�ADC�Ƿ������?
	if(ADC1 -> SR & (1<<2))
	{
		ADC1 -> SR &= ~(1<<2) ; 					//���������־�?
		
		//���� ADC  DMA 
		ADC1 -> CR1 &= ~(1<<1) ; 					//�ȹر�ADC
		ADC_DMACmd(ADC1,DISABLE); 					//��ֹ ADC DMA����
		DMA_DeInit(DMA1_Channel0);
		DMA_Init( DMA1_Channel0 , (uint32_t)ADC_Value,			//DMA�ڴ���?
                                  ADC1_DR_Address,				//DMA ADC������?
				  ADC1_Buffer_Size,				//�������ݸ��� : 8
				  DMA_DIR_PeripheralToMemory,			//���䷽�� : ���� -> �ڴ�
				  DMA_Mode_Circular,				//DMAģʽ : ��������
                                  DMA_MemoryIncMode_Inc,			//�ڴ��ַ�ۼ�?
				  DMA_Priority_High,				//DMA���ȼ� : ��
				  DMA_MemoryDataSize_HalfWord );		//�������ݳߴ� : 16 bit
		DMA_Cmd(DMA1_Channel0,ENABLE);
		
		ADC_DMACmd(ADC1,ENABLE);	//ʹ��ADC  DMA
	}
	//ʹ�ܵ���ADC ת�� 
	ADC1 -> CR1 |= (1<<1) ;
}

//==============================================================================
//��ʼ�� ADC  DMA_CH0
//==============================================================================
void adc_Init(void)
{
	//��ʼ��GPIO ��������(�� 7 ͨ��  �����ڲ��ο���   ��8ͨ��)
  	GPIOB -> CR1 &= ~((1<<7)|(1<<5)|(1<<4)|(1<<3)|(1<<2)|(1<<1)|(1<<0));
	GPIOB -> CR2 &= ~((1<<7)|(1<<5)|(1<<4)|(1<<3)|(1<<2)|(1<<1)|(1<<0));
	GPIOB -> DDR &= ~((1<<7)|(1<<5)|(1<<4)|(1<<3)|(1<<2)|(1<<1)|(1<<0));
	CLK -> PCKENR2 |= (1<<4)|(1<<0); 					//�� ADC1  DMA1 ʱ�� 
	
	//��ʼ�� DMA1_CH0 
	DMA_DeInit(DMA1_Channel0);
	DMA_Init( DMA1_Channel0 , (uint32_t)ADC_Value,				//DMA�ڴ���?
                                  ADC1_DR_Address,				//DMA ADC������?
				  ADC1_Buffer_Size,				//�������ݸ��� : 8
				  DMA_DIR_PeripheralToMemory,			//���䷽�� : ���� -> �ڴ�
				  DMA_Mode_Circular,				//DMAģʽ : ��������
                                  DMA_MemoryIncMode_Inc,			//�ڴ��ַ�ۼ�?
				  DMA_Priority_High,				//DMA���ȼ� : ��
				  DMA_MemoryDataSize_HalfWord );		//�������ݳߴ� : 16 bit
	DMA_Cmd(DMA1_Channel0,ENABLE);
	DMA_GlobalCmd(ENABLE);
		
        ADC1 -> CR1 = (1<<0);       						//�ֱ��� : 12bit  �ر�����ADC����ж�?  ����ת��  ʹ��ADC                              		
        ADC1 -> CR2 = (1<<2)|(1<<1);						//����ǰ24ͨ��    ������ 192ADC Clock cycles
	ADC1 -> CR3 = (1<<7)|(1<<6);						//�����ڲ��ο���  ������ 192ADC Clock cycles
	ADC_VrefintCmd(ENABLE);
	
	//Turn off external 7 channel GPIO Schmitt trigger
	ADC_SchmittTriggerConfig(ADC1, ADC_Channel_11, DISABLE) ; 
	ADC_SchmittTriggerConfig(ADC1, ADC_Channel_13, DISABLE) ; 
	ADC_SchmittTriggerConfig(ADC1, ADC_Channel_14, DISABLE) ; 
	ADC_SchmittTriggerConfig(ADC1, ADC_Channel_15, DISABLE) ; 
	ADC_SchmittTriggerConfig(ADC1, ADC_Channel_16, DISABLE) ; 
	ADC_SchmittTriggerConfig(ADC1, ADC_Channel_17, DISABLE) ; 
	ADC_SchmittTriggerConfig(ADC1, ADC_Channel_18, DISABLE) ; 
	
	//Enable all 8-channel ADCs (7 external GPIO channels + 1 internal reference channel)
	ADC_ChannelCmd(ADC1, ADC_Channel_11, ENABLE) ; 
	ADC_ChannelCmd(ADC1, ADC_Channel_13, ENABLE) ; 
	ADC_ChannelCmd(ADC1, ADC_Channel_14, ENABLE) ; 
	ADC_ChannelCmd(ADC1, ADC_Channel_15, ENABLE) ; 
	ADC_ChannelCmd(ADC1, ADC_Channel_16, ENABLE) ; 
	ADC_ChannelCmd(ADC1, ADC_Channel_17, ENABLE) ; 
	ADC_ChannelCmd(ADC1, ADC_Channel_18, ENABLE) ; 
	ADC_ChannelCmd(ADC1, ADC_Channel_Vrefint, ENABLE) ; 
	
	SYSCFG -> RMPCR1 = 0x0C ; 						//ӳ�� DMA1 Channel0 -> ADC1 
	ADC_DMACmd(ADC1,ENABLE);
	ADC1 -> CR1 |= (1<<1) ;							//��ʼһ�� ADC  
}

//====================================================================================================================
// ADC_Value[1]     -> RUD      ADC_Value[2]    -> THR         ADC_Value[3]     -> ELE        ADC_Value[4]     -> AIL      
// ADC_Value[5]     -> BAT      ADC_Value[6]    -> CJ_431      ADC_Value[7]     -> IR_REF     ADC_Value[0]     -> Random    
// Ϊ��ֹADֵ�������� �������� AD ֵ ��ƽ���˲�
//====================================================================================================================
void GetADValue(void)
{
	////////////////////////////////////////////////////////////////////////
 	// MCU internal reference electrical ADC (used to determine whether CJ431 is normal, CJ431 is not normal when using internal reference power as a benchmark)
	// STM8L052R8 internal reference voltage: 1.224V * (4095/3.3V) �� 1519
	////////////////////////////////////////////////////////////////////////
	static uint16_t IRV_ADValue  = 1519 ; 				
	static uint16_t ADJ_ADValue  = 3102 ; 				        //CJ431��׼��ѹ  2.5V * (4095/3.3V) 
	static uint16_t Volt_ADValue = 3309 ; 				        //��ʼ����ص��? 4.0V * (2/3)* (4095/3.3V)
		
	uint16_t TempAD = 0 ; 
	uint16_t LimitTemp = 0 ; 
	
	//Calculate internal reference power (1.224V)
	TempAD =  ADC_Value[7] ;
	if(IRV_ADValue > TempAD)  LimitTemp = IRV_ADValue - TempAD ;  
	else			  LimitTemp = TempAD - IRV_ADValue ;  
	if(LimitTemp > 30)//Change more than a certain range, respond immediately
	
	{
		IRV_ADValue = TempAD ; 
	}
	else
	{
		IRV_ADValue = (uint16_t)(IRV_ADValue*0.8f + TempAD *0.2f) ;
	}
	if((IRV_ADValue > 1719) || (IRV_ADValue < 1319)) IRV_ADValue  = 1519 ;
		
	//Calculate the scale factor of CJ431 
	TempAD =  ADC_Value[6] ;
	if(ADJ_ADValue > TempAD)  LimitTemp = ADJ_ADValue - TempAD ;  
	else			  LimitTemp = TempAD - ADJ_ADValue ;  
	if(LimitTemp > 30)//Change more than a certain range, respond immediately

	{
		ADJ_ADValue = TempAD ; 
	}
	else
	{
		ADJ_ADValue = (uint16_t)(ADJ_ADValue*0.8f + TempAD *0.2f) ;
	}
	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// Internal reference power is used to determine the validity of the CJ431 reference. If the CJ431 decision is invalid,
	// Use the internal reference power to calculate the remaining AD values
	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	TempAD = (uint16_t)((1519.0f/IRV_ADValue)*ADJ_ADValue) ; 
	if(TempAD < 3002)        { ISCJ431OKFlg = false ; ADJ_ADValue = 3102 ;} //��Ϊ�ο���ѹADֵ����������Ϊ������(���ܵ�����,��������ܷ�?)
	else if(TempAD > 3202)   { ISCJ431OKFlg = false ; ADJ_ADValue = 3102 ;}
		
	//Calculate battery voltage
	TempAD =  ADC_Value[5] ;
	if(ISCJ431OKFlg == true)TempAD = (uint16_t)(TempAD * ( 3102 / (float)ADJ_ADValue)) ;
	else			TempAD = (uint16_t)(TempAD * ( 1519 / (float)IRV_ADValue)) ;
	if(Volt_ADValue > TempAD)  LimitTemp = Volt_ADValue - TempAD ;  
	else			   LimitTemp = TempAD - Volt_ADValue ;  
	if(LimitTemp > 30)
	{
		Volt_ADValue = TempAD ; 
	}
	else
	{
		Volt_ADValue = (uint16_t)(Volt_ADValue * 0.8f + TempAD * 0.2f) ; 	
	}
	VoltValue =  (float)Volt_ADValue/Input_Max *3.3f* 1.51f ;		//5.1K + 10K��ѹ(﮵�ص��?)
		
	//Calculate RUDDER
	TempAD = ADC_Value[1] ;
	if(ISCJ431OKFlg == true) TempAD = (uint16_t)(TempAD * ( 3102 / (float)ADJ_ADValue)) ;
	else			 TempAD = (uint16_t)(TempAD * ( 1519 / (float)IRV_ADValue)) ;
	if(Sampling_Data[RUDDER] > TempAD)  	LimitTemp = Sampling_Data[RUDDER] - TempAD  ;  
	else			   		LimitTemp = TempAD - Sampling_Data[RUDDER]  ;  
	if(LimitTemp < 20)
	{
		TempAD = (uint16_t)(Sampling_Data[RUDDER] * 0.8f + TempAD * 0.2f); 	
	}
	if(TempAD > Input_Max) 	Sampling_Data[RUDDER]   = Input_Max ; 
	else              	Sampling_Data[RUDDER]   = TempAD ;
		
	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	//!!!!!  Note: American hand and Japanese hand THROTTLE and ELEVATOR are interchanged   !!!!!
	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	//����Throttle
	if(RFHabit == __AmericaPlayer)
	{
		TempAD =  ADC_Value[2] ;
	}
	else
	{
		TempAD =  ADC_Value[3] ;
	}
	if(ISCJ431OKFlg == true) TempAD = (uint16_t)(TempAD * ( 3102 / (float)ADJ_ADValue)) ;
	else			 TempAD = (uint16_t)(TempAD * ( 1519 / (float)IRV_ADValue)) ;
	if(Sampling_Data[THROTTLE] > TempAD)  	LimitTemp = Sampling_Data[THROTTLE] - TempAD ;  
	else			   		LimitTemp = TempAD - Sampling_Data[THROTTLE] ;  
	if(LimitTemp < 20)
	{
		TempAD = (uint16_t)(Sampling_Data[THROTTLE] * 0.8f + TempAD * 0.2f); 	
	}
	if(TempAD > Input_Max) 	Sampling_Data[THROTTLE]   = Input_Max ; 
	else              	Sampling_Data[THROTTLE]   = TempAD ;
		
	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	//!!!!!  Note: American hand and Japanese hand THROTTLE and ELEVATOR are interchanged  !!!!!
	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	//Calculate ELEVATOR
	if(RFHabit == __AmericaPlayer)
	{
		TempAD =  ADC_Value[3] ;
	}
	else
	{
		TempAD =  ADC_Value[2] ;
	}
	if(ISCJ431OKFlg == true) TempAD = (uint16_t)(TempAD * ( 3102 / (float)ADJ_ADValue)) ;
	else			 TempAD = (uint16_t)(TempAD * ( 1519 / (float)IRV_ADValue)) ;
	if(Sampling_Data[ELEVATOR] > TempAD)  	LimitTemp = Sampling_Data[ELEVATOR] - TempAD ;  
	else			   		LimitTemp = TempAD - Sampling_Data[ELEVATOR] ;  
	if(LimitTemp < 20)
	{
		TempAD = (uint16_t)(Sampling_Data[ELEVATOR] * 0.8f + TempAD * 0.2f); 	
	}
	if(TempAD > Input_Max) 	Sampling_Data[ELEVATOR]   = Input_Max ; 
	else              	Sampling_Data[ELEVATOR]   = TempAD ;
		
	//Calculate AILERON
	TempAD =  ADC_Value[4] ;
	if(ISCJ431OKFlg == true) 
		TempAD = (uint16_t)(TempAD * ( 3102 / (float)ADJ_ADValue)) ;
	else
		TempAD = (uint16_t)(TempAD * ( 1519 / (float)IRV_ADValue)) ;
	if(Sampling_Data[AILERON] > TempAD)  	LimitTemp = Sampling_Data[AILERON] - TempAD ;  
	else 
		LimitTemp = TempAD - Sampling_Data[AILERON] ;  

	if(LimitTemp < 20)
		TempAD = (uint16_t)(Sampling_Data[AILERON] * 0.8f + TempAD * 0.2f); 	

	if(TempAD > Input_Max) 
		Sampling_Data[AILERON]   = Input_Max ; 
	else              	Sampling_Data[AILERON]   = TempAD ;
		
	//Update all wireless transmission channel values in the joystick section
	//tx________________Value_____________________Test ();
	// FRSKYD8_SendDataBuff[RUDDER]   = Get_SendValue((ChannelTypeDef)FRSKYD8_CH_Code[RUDDER]);  
	// FRSKYD8_SendDataBuff[THROTTLE] = Get_SendValue((ChannelTypeDef)FRSKYD8_CH_Code[THROTTLE]); 
	// FRSKYD8_SendDataBuff[ELEVATOR] = Get_SendValue((ChannelTypeDef)FRSKYD8_CH_Code[ELEVATOR]);  
	// FRSKYD8_SendDataBuff[AILERON]  = Get_SendValue((ChannelTypeDef)FRSKYD8_CH_Code[AILERON]); 
	//Switching channel
	FRSKYD8_SendDataBuff[AUX1]     = Get_SendValue((ChannelTypeDef)FRSKYD8_CH_Code[AUX1]);  
	FRSKYD8_SendDataBuff[AUX2]     = Get_SendValue((ChannelTypeDef)FRSKYD8_CH_Code[AUX2]);  
	FRSKYD8_SendDataBuff[AUX3]     = Get_SendValue((ChannelTypeDef)FRSKYD8_CH_Code[AUX3]);  
	FRSKYD8_SendDataBuff[AUX4]     = Get_SendValue((ChannelTypeDef)FRSKYD8_CH_Code[AUX4]);


	
}



