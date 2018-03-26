/**
  ******************************************************************************
  * @file    adc.c
  * @author  Fotona d.d.
  * @version V1
  * @date    30-Sept-2013
  * @brief	 AD & DMA converters initialization
  *
  */
/** @addtogroup PFM6_Setup
* @{
*/
#include	"pfm.h"

_ADC3DMA	ADC3_buf[_AVG3];
_ADCDMA		ADC1_buf[_MAX_BURST/_uS],
					ADC2_buf[_MAX_BURST/_uS],
					ADC1_simmer,
					ADC2_simmer;

int				_ADCRates[]={3,15,28,56,84,112,144,480};

void	TriggerADC(PFM *p) {
//
// trigger tresholds, must be set before ADC's disabled
//
			ADC_ClearITPendingBit(ADC1, ADC_IT_AWD);
			ADC_ClearITPendingBit(ADC2, ADC_IT_AWD);
			ADC_ITConfig(ADC1,ADC_IT_AWD,DISABLE);
			ADC_ITConfig(ADC2,ADC_IT_AWD,DISABLE);	

			if(!_MODE(pfm,_CHANNEL1_DISABLE)) {
				if(p)
					ADC_AnalogWatchdogThresholdsConfig(ADC1,pfm->Burst->max[0],0);
				else
					ADC_AnalogWatchdogThresholdsConfig(ADC1,pfm->Simmer.max,0);
				ADC_ITConfig(ADC1,ADC_IT_AWD,ENABLE);
			}

			if(!_MODE(pfm,_CHANNEL2_DISABLE)) {
				if(p)
					ADC_AnalogWatchdogThresholdsConfig(ADC2,pfm->Burst->max[1],0);
				else
					ADC_AnalogWatchdogThresholdsConfig(ADC2,pfm->Simmer.max,0);
				ADC_ITConfig(ADC2,ADC_IT_AWD,ENABLE);
			}

			ADC_Cmd(ADC1, DISABLE);							ADC_Cmd(ADC2, DISABLE);
			DMA_Cmd(DMA2_Stream4,DISABLE);			DMA_Cmd(DMA2_Stream3,DISABLE);

			while(DMA_GetCmdStatus(DMA2_Stream4) != DISABLE);
			while(DMA_GetCmdStatus(DMA2_Stream3) != DISABLE);

			if(p) {
				DMA_MemoryTargetConfig(DMA2_Stream3,(uint32_t)ADC2_buf,DMA_Memory_0);
				DMA_MemoryTargetConfig(DMA2_Stream4,(uint32_t)ADC1_buf,DMA_Memory_0);
				DMA_SetCurrDataCounter(DMA2_Stream3,__min(_TIM.eint*_uS/_MAX_ADC_RATE*sizeof(_ADCDMA)/sizeof(short),sizeof(ADC1_buf)/sizeof(short)));
				DMA_SetCurrDataCounter(DMA2_Stream4,__min(_TIM.eint*_uS/_MAX_ADC_RATE*sizeof(_ADCDMA)/sizeof(short),sizeof(ADC2_buf)/sizeof(short)));
				DMA_ClearITPendingBit(DMA2_Stream3,DMA_IT_TCIF3|DMA_IT_HTIF3|DMA_IT_TEIF3|DMA_IT_DMEIF3|DMA_IT_FEIF3);
				DMA_ClearITPendingBit(DMA2_Stream4,DMA_IT_TCIF4|DMA_IT_HTIF4|DMA_IT_TEIF4|DMA_IT_DMEIF4|DMA_IT_FEIF4);
				DMA_ITConfig(DMA2_Stream4, DMA_IT_TC, ENABLE);
			} else {
				DMA_MemoryTargetConfig(DMA2_Stream3,(uint32_t)&ADC2_simmer,DMA_Memory_0);
				DMA_MemoryTargetConfig(DMA2_Stream4,(uint32_t)&ADC1_simmer,DMA_Memory_0);
				DMA_SetCurrDataCounter(DMA2_Stream3,sizeof(_ADCDMA)/sizeof(short));
				DMA_SetCurrDataCounter(DMA2_Stream4,sizeof(_ADCDMA)/sizeof(short));
				DMA_ITConfig(DMA2_Stream4, DMA_IT_TC, DISABLE);			
				ADC_DMARequestAfterLastTransferCmd(ADC1, ENABLE);
				ADC_DMARequestAfterLastTransferCmd(ADC2, ENABLE);	
			}

			DMA_ClearFlag(DMA2_Stream3,DMA_FLAG_TCIF3|DMA_FLAG_HTIF3|DMA_FLAG_TEIF3|DMA_FLAG_DMEIF3|DMA_FLAG_FEIF3);
			DMA_ClearFlag(DMA2_Stream4,DMA_FLAG_TCIF4|DMA_FLAG_HTIF4|DMA_FLAG_TEIF4|DMA_FLAG_DMEIF4|DMA_FLAG_FEIF4);

			DMA_Cmd(DMA2_Stream4,ENABLE);				DMA_Cmd(DMA2_Stream3,ENABLE);
			ADC_DMACmd(ADC1, DISABLE);					ADC_DMACmd(ADC2, DISABLE);
			ADC_DMACmd(ADC1, ENABLE);						ADC_DMACmd(ADC2, ENABLE);
			ADC_Cmd		(ADC1, ENABLE);						ADC_Cmd		(ADC2, ENABLE);
}
/*******************************************************************************/
void	DMA2_Stream4_IRQHandler(void) {
			TriggerADC(NULL);
}
/*******************************************************************************/
/**
  * @brief  ADC1 channel init. with DMA
  * @param  None
  * @retval None
  */
void 	Initialize_ADC1(void)
{
			ADC_InitTypeDef       ADC_InitStructure;
			DMA_InitTypeDef       DMA_InitStructure;
			GPIO_InitTypeDef      GPIO_InitStructure;

/* DMA2 Stream4 channel0 configuration **************************************/
			RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);

			DMA_InitStructure.DMA_Channel = DMA_Channel_0;
			DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(ADC1_BASE+0x4C);
			DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)&ADC1_simmer;
			DMA_InitStructure.DMA_BufferSize = sizeof(_ADCDMA)/sizeof(short);
			DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
			DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
			DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
			DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
			DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
			DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
			DMA_InitStructure.DMA_Priority = DMA_Priority_High;
			DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Enable;
			DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull;
			DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
			DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
			DMA_Init(DMA2_Stream4, &DMA_InitStructure);

			DMA_Cmd(DMA2_Stream4, ENABLE);
//		DMA_ITConfig(DMA2_Stream4, DMA_IT_TC | DMA_IT_HT, ENABLE);
//		DMA_ITConfig(DMA2_Stream4, DMA_IT_TC, ENABLE);

/* Configure ADC1 Channel gpio pin as analog inputs *************************/
			GPIO_StructInit(&GPIO_InitStructure);
			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
			GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
			GPIO_Init(GPIOB, &GPIO_InitStructure);
			
/* ADC1 Init ****************************************************************/
			ADC_StructInit(&ADC_InitStructure);
			ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
			ADC_InitStructure.ADC_ScanConvMode = ENABLE;
			ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
			ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_Rising;	
			
#ifdef	__F7__
			ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T3_CC1;
#else
			ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T8_TRGO;
#endif

			ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
			ADC_InitStructure.ADC_NbrOfConversion = sizeof(_ADCDMA)/sizeof(short);
			ADC_Init(ADC1, &ADC_InitStructure);

/* ADC1 regular channel12 configuration *************************************/
			ADC_RegularChannelConfig(ADC1, ADC_Channel_9, 1, ADC_SampleTime);				// Uflash 1
			ADC_RegularChannelConfig(ADC1, ADC_Channel_8, 2, ADC_SampleTime);				// Iflash 1		

/* Set injected sequencer length */
			ADC_InjectedSequencerLengthConfig(ADC1, 1);
/* ADC1 injected channel Configuration */
			ADC_InjectedChannelConfig(ADC1, ADC_Channel_TempSensor, 1, ADC_SampleTime_144Cycles);
/* ADC1 injected external trigger configuration */
			ADC_ExternalTrigInjectedConvEdgeConfig(ADC1, ADC_ExternalTrigInjecConvEdge_None);

			ADC_AnalogWatchdogSingleChannelConfig(ADC1,ADC_Channel_8);
			ADC_AnalogWatchdogCmd(ADC1,ADC_AnalogWatchdog_SingleRegEnable);

/* Enable DMA request after last transfer (Single-ADC mode) */
			ADC_DMARequestAfterLastTransferCmd(ADC1, ENABLE);
			ADC_Cmd(ADC1, ENABLE);
			ADC_DMACmd(ADC1, ENABLE);
}
/*******************************************************************************/
/**
  * @brief  ADC2 channel init. with DMA
  * @param  None
  * @retval None
  */
void 	Initialize_ADC2(void)
{
			ADC_InitTypeDef       ADC_InitStructure;
			DMA_InitTypeDef       DMA_InitStructure;
			GPIO_InitTypeDef      GPIO_InitStructure;

/* DMA2 Stream3 channel1 configuration **************************************/
			RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);

			DMA_InitStructure.DMA_Channel = DMA_Channel_1;
			DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(ADC2_BASE+0x4C);
			DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)&ADC2_simmer;
			DMA_InitStructure.DMA_BufferSize = sizeof(_ADCDMA)/sizeof(short);
			DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
			DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
			DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
			DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
			DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
			DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
			DMA_InitStructure.DMA_Priority = DMA_Priority_High;
			DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Enable;
			DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull;
			DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
			DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
			DMA_Init(DMA2_Stream3, &DMA_InitStructure);

			DMA_Cmd(DMA2_Stream3, ENABLE);

/* Configure ADC2 Channel gpio pin as analog inputs *************************/
			GPIO_StructInit(&GPIO_InitStructure);
			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5;
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
			GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
			GPIO_Init(GPIOC, &GPIO_InitStructure);

/* ADC2 Init ****************************************************************/
			ADC_StructInit(&ADC_InitStructure);
			ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
			ADC_InitStructure.ADC_ScanConvMode = ENABLE;
			ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
			ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_Rising;

#ifdef	__F7__
			ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T3_CC1;
#else
			ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T8_TRGO;
#endif

			ADC_InitStructure.ADC_NbrOfConversion = sizeof(_ADCDMA)/sizeof(short);
			ADC_Init(ADC2, &ADC_InitStructure);

/* ADC2 regular channel12 configuration *************************************/
			ADC_RegularChannelConfig(ADC2, ADC_Channel_15, 1, ADC_SampleTime);		// Uflash 2
			ADC_RegularChannelConfig(ADC2, ADC_Channel_14, 2, ADC_SampleTime);		// Iflash 2

			ADC_AnalogWatchdogSingleChannelConfig(ADC2,ADC_Channel_14);	
			ADC_AnalogWatchdogCmd(ADC2,ADC_AnalogWatchdog_SingleRegEnable);

/* Enable DMA request after last transfer (Single-ADC mode) */
			ADC_DMARequestAfterLastTransferCmd(ADC2, ENABLE);
			ADC_Cmd(ADC2, ENABLE);
			ADC_DMACmd(ADC2, ENABLE);
}
/*******************************************************************************/
/**
  * @brief  ADC3 channel init. with DMA
  * @param  None
  * @retval None
  */
void 	Initialize_ADC3(void)
{
			ADC_InitTypeDef       ADC_InitStructure;
			DMA_InitTypeDef       DMA_InitStructure;
			GPIO_InitTypeDef      GPIO_InitStructure;

/* Enable ADC3, DMA2 and GPIO clocks ****************************************/
			RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2 | RCC_AHB1Periph_GPIOC, ENABLE);
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC3, ENABLE);

/* DMA2 Stream0 channel0 configuration **************************************/
			DMA_InitStructure.DMA_Channel = DMA_Channel_2;
			DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(ADC3_BASE+0x4C);			
			DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)ADC3_buf;
			DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
			DMA_InitStructure.DMA_BufferSize = sizeof(ADC3_buf)/sizeof(short);
			DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
			DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
			DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
			DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
			DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
			DMA_InitStructure.DMA_Priority = DMA_Priority_High;
			DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Enable;
			DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
			DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
			DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
			DMA_Init(DMA2_Stream0, &DMA_InitStructure);
			DMA_Cmd(DMA2_Stream0, ENABLE);

/* Configure ADC3 Channel12 PC3,PA0,PA1 pin as analog input ******************************/

			GPIO_StructInit(&GPIO_InitStructure);
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
			GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
#if	defined (__PFM6__)
			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2;																							
			GPIO_Init(GPIOA, &GPIO_InitStructure);																			// temp 1, temp 2, HV/2		
			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3;										
			GPIO_Init(GPIOC, &GPIO_InitStructure);																			// HV,20V,-5V
#endif
#if	defined (__PFM8__)
			GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_2 | GPIO_Pin_3;																													
			GPIO_Init(GPIOA, &GPIO_InitStructure);																			// HV/2, VCAP1sense		
			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1; 																											
			GPIO_Init(GPIOC, &GPIO_InitStructure);																			// VCAP2sense, HV		
			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_4  | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 	| GPIO_Pin_9 ;										
			GPIO_Init(GPIOF, &GPIO_InitStructure);																			// 12V, 5V, 3.3V, TH1,TH2,TL1,TL2
#endif
/* ADC3 Init ****************************************************************/
			ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
			ADC_InitStructure.ADC_ScanConvMode = ENABLE;
			ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
			ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
			ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
			ADC_InitStructure.ADC_NbrOfConversion = sizeof(_ADC3DMA)/sizeof(short);
			ADC_Init(ADC3, &ADC_InitStructure);

/* ADC3 regular channel12 configuration ************************************
*/
#if	defined (__PFM6__)
			ADC_RegularChannelConfig(ADC3, ADC_Channel_0,  1, ADC_SampleTime);				// temp 1
			ADC_RegularChannelConfig(ADC3, ADC_Channel_1,  2, ADC_SampleTime);				// temp 2
			ADC_RegularChannelConfig(ADC3, ADC_Channel_2,  3, ADC_SampleTime);				// HV/2
			ADC_RegularChannelConfig(ADC3, ADC_Channel_11, 4, ADC_SampleTime);				// HV
			ADC_RegularChannelConfig(ADC3, ADC_Channel_12, 5, ADC_SampleTime);				// +20V
			ADC_RegularChannelConfig(ADC3, ADC_Channel_13, 6, ADC_SampleTime);				// -5V
#endif						                                                                    
#if	defined (__PFM8__)						                                                    
			ADC_RegularChannelConfig(ADC3, ADC_Channel_4,	 1, ADC_SampleTime);				// TH1
			ADC_RegularChannelConfig(ADC3, ADC_Channel_5,	 2, ADC_SampleTime);				// TH2
			ADC_RegularChannelConfig(ADC3, ADC_Channel_6,  3,	ADC_SampleTime);				// TL1
			ADC_RegularChannelConfig(ADC3, ADC_Channel_7,  4,	ADC_SampleTime);				// TL2
			ADC_RegularChannelConfig(ADC3, ADC_Channel_2,  5, ADC_SampleTime);				// HV/2
			ADC_RegularChannelConfig(ADC3, ADC_Channel_11, 6, ADC_SampleTime);				// HV
			ADC_RegularChannelConfig(ADC3, ADC_Channel_9,  7, ADC_SampleTime);				// 12V
			ADC_RegularChannelConfig(ADC3, ADC_Channel_14, 8, ADC_SampleTime);				// 5V
			ADC_RegularChannelConfig(ADC3, ADC_Channel_15, 9, ADC_SampleTime);				// 3.3V
			ADC_RegularChannelConfig(ADC3, ADC_Channel_3,  10, ADC_SampleTime);				// VCAP1sense	
			ADC_RegularChannelConfig(ADC3, ADC_Channel_10, 11, ADC_SampleTime);				// VCAP2sense
#endif
#if		!defined (__DISC4__) && !defined (__DISC7__)
			ADC_AnalogWatchdogSingleChannelConfig(ADC3,ADC_Channel_11);
			ADC_AnalogWatchdogCmd(ADC3,ADC_AnalogWatchdog_SingleRegEnable);	

			ADC_DMARequestAfterLastTransferCmd(ADC3, ENABLE);
			ADC_DMACmd(ADC3, ENABLE);
			ADC_Cmd(ADC3, ENABLE);
			ADC_SoftwareStartConv(ADC3);
#endif
}
/**
  * @brief  ADC	common init
  * @param  None
  * @retval None
  */
/*******************************************************************************/
void 	Initialize_ADC(void)
{
			ADC_CommonInitTypeDef ADC_CommonInitStructure;
			RCC_APB2PeriphClockCmd(	RCC_APB2Periph_ADC |
									RCC_APB2Periph_ADC1 |
									RCC_APB2Periph_ADC2 |
									RCC_APB2Periph_ADC3, ENABLE);

			ADC_CommonStructInit(&ADC_CommonInitStructure);
			ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
			ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div2;
			ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
			ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
			ADC_CommonInit(&ADC_CommonInitStructure);

			ADC_TempSensorVrefintCmd(ENABLE);
			Initialize_ADC1();
			Initialize_ADC2();
			Initialize_ADC3();
}
/*******************************************************************************
* Function Name  : ADC_IRQHandler
* Description    : ADC3 analog watchdog ISR
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void	ADC_IRQHandler(void)	{
			if(ADC_GetITStatus(ADC1, ADC_IT_AWD) != RESET) {
				ADC_ClearITPendingBit(ADC1, ADC_IT_AWD);
				_SET_ERROR(pfm,PFM_ADCWDG_ERR);
				ADC_ITConfig(ADC1,ADC_IT_AWD,DISABLE);
			}
			if(ADC_GetITStatus(ADC2, ADC_IT_AWD) != RESET) {
				ADC_ClearITPendingBit(ADC2, ADC_IT_AWD);
				_SET_ERROR(pfm,PFM_ADCWDG_ERR);
				ADC_ITConfig(ADC2,ADC_IT_AWD,DISABLE);
			}
			if(ADC_GetITStatus(ADC3, ADC_IT_AWD) != RESET) {
				ADC_ClearITPendingBit(ADC3, ADC_IT_AWD);
				_SET_ERROR(pfm,PFM_ADCWDG_ERR);
			}
}

/**
* @}
*/ 

