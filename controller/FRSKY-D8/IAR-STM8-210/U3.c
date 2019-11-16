/*
2017-8-16 : Cancel the USART3 DMA send completion interrupt. Now it is changed to stop after sending a packet of data each time, and then turn it off and then on 
			before the next transmission (because it will conflict with the TIM2 wireless transmission interrupt, affecting the wireless transmission time base)
*/
#include "include.h"

#define  BAUD  115200
#define  Fosc  12000000

#define  U3_DR_Address                    (USART3_BASE + 0x01)
#define  U3_Test_Buffer_Size       8
#define  U3_Buffer_Size 		   103

#define BIT(n) (1 << n)

#define U3_Recv_Buffer_Size 8

static uint16_t U3_DMA_RXBuff[U3_Recv_Buffer_Size / 2] = { 1500, 1500, 1000, 1500 };

static uint16_t DMA_Test[U3_Test_Buffer_Size / 2] = { 1500, 1500, 1000, 1500 };

bool DMA1_U3_Tx_Flag = false ; 		  //DMA send flag

static uint8_t U3_DMA_TXBuff[U3_Buffer_Size] = 
{
  	//(7Byte  ??????) Starting Head
	0x4C , 0x44 , 0x41 , 0x54 , 0x58 , 0x31 , 0x30 , 
	//(1Byte  0x45 / 0x44) Device Type
	0x45 , 
	//(3Byte  ?? - ?? - ??) Remote control software version number
	0x00 , 0x00 , 0x00 , 
	//(1Byte)  Wireless protocol version number
	0x00 ,
	//American hand / Japanese hand mode selection
	0x00 ,
	// remote control unique ID number(4Byte)
	0x00 , 0x00 , 0x00 , 0x00 , 
	//16 channel data(????2Byte = 16*2Byte = 32Byte)
	0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 
	0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 ,
	0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 ,
	0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 ,
	//Channel reverse flag(1Byte)
	0x00 , 
	//All buttons + 8-bit DIP switch GPIO value(2Byte)
	0x00 , 0x00 , 0x00 , 
	//Battery voltage value(2Byte)
	0x00 , 0x00 , 
	//Reference 431 + High Frequency Module + Whether the median calibration is successful(1Byte)
	0x00 , 
	//Raw values ??of all channel AD samples(????2Byte = 6*2Byte = 12Byte)
	0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 
	0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 
	//LED??(1Byte)  1????????  0???????
	0x00 , 
	//Alarm status(1Byte)
	0x00 , 
	//Timer crash time(1Byte)
	0x00 , 
	//Four rocker offset value(4Byte)
	0x00 , 0x00 , 0x00 , 0x00 , 
	
	//Four rocker maximum AD value(4*2Byte = 8Byte)
	0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 
	
	//Four rocker median AD value(4*2Byte = 8Byte)
	0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 
	
	//Minimum rocker AD value(4*2Byte = 8Byte)
	0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 
	
	//Packet checksum(2Byte)
	0x00 , 0x00 , 
}; 


void U3_Init(void)
{
  	CLK -> PCKENR3 |= (1<<4) ; 						// Usart3 Clock
	
	//初始化 DMA1_CH1 (Usart3_TX DMA)
	DMA_DeInit(DMA1_Channel1);
	DMA_Init( DMA1_Channel1 , (uint32_t) U3_DMA_TXBuff,			//DMA memory address
                                  U3_DR_Address,				//DMA ADC peripheral address
				  U3_Buffer_Size,				//传输数据个数 : 103
				  DMA_DIR_MemoryToPeripheral,			//传输方向 : 内存 -> 外设
				  DMA_Mode_Normal,				//DMA模式 : 单次传输
                                  DMA_MemoryIncMode_Inc,			//内存地址累加
				  DMA_Priority_High,				//DMA优先级 : 高
				  DMA_MemoryDataSize_Byte );			//Transfer data size : 8 bit
	DMA_Cmd(DMA1_Channel1,DISABLE);						// Turn off DMA first (open it when needed)
	DMA_GlobalCmd(ENABLE);
	
	//Usart3初始化
	uint16_t BRR_Counts = Fosc / BAUD ; 
	
	USART3 -> BRR2  =  BRR_Counts & 0x000F ; 
       	USART3 -> BRR2 |= ((BRR_Counts & 0xf000) >> 8);
    	USART3 -> BRR1  = ((BRR_Counts & 0x0ff0) >> 4); /* First assign BRR2 and finally set BRR1 */  
	USART3 -> CR2 = (BIT(2) | BIT(3)); 					// Send enable
	USART3 -> CR3 = 0 ; 
	USART3 -> CR1 &= ~(1<<5);							// Enable serial port

	DMA_DeInit(DMA1_Channel2);
	DMA_Init( DMA1_Channel2 , (uint32_t)U3_DMA_RXBuff,
                  U3_DR_Address,
				  U3_Recv_Buffer_Size,
				  DMA_DIR_PeripheralToMemory,
				  DMA_Mode_Circular,
                  DMA_MemoryIncMode_Inc,
				  DMA_Priority_High,
				  DMA_MemoryDataSize_Byte );
		
	USART_DMACmd(USART3, USART_DMAReq_RX, ENABLE);
	DMA_Cmd(DMA1_Channel2 ,ENABLE);
	DMA_GlobalCmd(ENABLE); 
}


//==============================================================================
//Send serial port packet
//==============================================================================
void U3_DMATX_ONOFF(void)
{
  	// Send completion flag
  	if (DMA1_Channel1 -> CSPR & (1<<1))
	{
		DMA1_Channel1 -> CSPR &= ~(1<<1) ; 	// Clear interrupt flag
		USART3 -> CR5   &= ~(1<<7) ; 		// Turn off DMA UART1 send request
		DMA1_Channel1->CCR &= ~(1<<0);		// Turn off DMA
		DMA1_U3_Tx_Flag = false ; 		// Set flag
	}
  
  	//Sending will not start until the send is complete
	if (DMA1_U3_Tx_Flag == false)
	{
		//设置发送个数
		DMA1_Channel1 -> CNBTR = U3_Buffer_Size;
		
		//启动UART3 DMA发送！
		USART3 -> CR5   |= (1<<7) ; 					// USART3 DMA TX Enable
		DMA1_Channel1 -> CCR |= (1<<0) ;
		
		DMA1_U3_Tx_Flag = true;
	}
}


static uint8_t done = 0;

//==============================================================================
//Because the data packet is too large, in order not to affect the response speed of other programs, a packet of sent data is split into 5 loads.
// After loading all the data, enable it again.
//==============================================================================
void BuildUsart3Data(void)
{
	// if(U3_DMA_RXBuff[0] != done) {
	// 	done = U3_DMA_RXBuff[0];
	// 	DMA_Test[0] = done;
	// 	U3_DMATX_ONOFF();
	// }

	FRSKYD8_SendDataBuff[RUDDER]   = U3_DMA_RXBuff[RUDDER];
	FRSKYD8_SendDataBuff[THROTTLE] = U3_DMA_RXBuff[THROTTLE];
	FRSKYD8_SendDataBuff[ELEVATOR] = U3_DMA_RXBuff[ELEVATOR];  
	FRSKYD8_SendDataBuff[AILERON]  = U3_DMA_RXBuff[AILERON]; 

	static uint8_t Phase = 0 ; 
	uint8_t i = 0 ; 
	uint16_t DataTemp = 0 ; 
	
	//载入前 17 Byte 数据
	if(Phase == 0)
	{
	  	//                  L                         D                          A                         T                         X                          1                        0
		U3_DMA_TXBuff[0]  = 0x4C ; U3_DMA_TXBuff[1] = 0X44 ; U3_DMA_TXBuff[2] = 0x41 ; U3_DMA_TXBuff[3] = 0x54 ; U3_DMA_TXBuff[4] = 0X58 ; U3_DMA_TXBuff[5] = 0x31 ; U3_DMA_TXBuff[6] = 0x30 ; //起始头(7Byte)
		U3_DMA_TXBuff[7]  = RF_TypeVersion ;											//遥控器类型(FRSKYD8)
		U3_DMA_TXBuff[8]  = 18   ; U3_DMA_TXBuff[9] =  1 ; U3_DMA_TXBuff[10] = 25 ; 						//软件编译时间 : 2018 - 1 - 25
		U3_DMA_TXBuff[11] = MasterInitProtocolVersion ; 									//无线协议版本号 (1Byte)
		U3_DMA_TXBuff[12] = RFHabit ;												//美国手/日本手选择 (0:美国手 / 1:日本手)
		U3_DMA_TXBuff[13] = 0x00 ; //(TransmitterID >> 24)&0xff ; 								//遥控器ID号(SFHSS 只有16Byte 所以高16位固定为0)
		U3_DMA_TXBuff[14] = 0x00 ; //(TransmitterID >> 16)&0xff ;
		U3_DMA_TXBuff[15] = (TransmitterID >> 8)&0xff ;
		U3_DMA_TXBuff[16] = TransmitterID &0xff ; 
	}
	//载入 16通道 数据
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
	//载入 通道反向标志位(1Byte) + 所有按键 + 8位拨码开关GPIO值(2Byte) + 电池电压值(2Byte) + 参考电431 高频模块 中位校准成功 标志位
	else if(Phase == 7)
	{
		U3_DMA_TXBuff[49] = ChannelInversion_flg ; 			//通道反向标志位
		
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
	//载入 所有通道AD采样原始值 + LED状态 + 报警状态 + 定时器死机次数 + 四摇杆偏置值(4Byte) + 四摇杆中位AD值
	else if(Phase == 8)
	{
	  	//7通道AD值
		for(i = 0 ; i < 7 ; i++)
		{
			DataTemp = ADC_Value[i] ; 
			U3_DMA_TXBuff[56 + 2*i] = DataTemp >> 8 ;
			U3_DMA_TXBuff[57 + 2*i] = DataTemp & 0xFF ;
		}
		//LED状态
		U3_DMA_TXBuff[70] = LED_Status_SendDat ; 
		//报警状态
		U3_DMA_TXBuff[71] = RunStatus ; 
		//Tiempo de bloqueo del temporizador
		U3_DMA_TXBuff[72] = TIM2_ErrorCnt ; 
		// 4 valores de desplazamiento del eje de balancín + 4 valores extremos del joystick (valor AD máximo, medio, mínimo)
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
	  	//Se suman todos los números
	  	DataTemp = 0 ; 
		for(i = 0 ; i < (U3_Buffer_Size - 2) ; i++)
		{
			DataTemp += U3_DMA_TXBuff[i] ; 
		}
		U3_DMA_TXBuff[101] = DataTemp >> 8;
		U3_DMA_TXBuff[102] = DataTemp & 0xFF;
	}
	
	if(++Phase > 10) { Phase = 0 ; /* U3_DMATX_ONOFF(); */ } //开始发送  
}
