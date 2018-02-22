/**
	******************************************************************************
	* @file		timers.c
	* @author	Fotona d.d.
	* @version V1
	* @date		30-Sept-2013
	* @brief	Timers initialization & ISR
	*
	*/
/** @addtogroup PFM6_Setup
* @{
*/
#include	"pfm.h"
#include	<math.h>
struct 		_TIM _TIM;
// ________________________________________________________________________________
// ________________________________________________________________________________
// ________________________________________________________________________________
// ________________________________________________________________________________
// ________________________________________________________________________________
// ________________________________________________________________________________
// ________________________________________________________________________________
// ________________________________________________________________________________
// ________________________________________________________________________________
// ________________________________________________________________________________
// ________________________________________________________________________________
//
/**********************************************************************************
* Function Name	: Timer_Init
* Description		: 
* Output				: 
* Return				:
**********************************************************************************/
void 		Initialize_TIM() {
TIM_TimeBaseInitTypeDef		TIM_TimeBaseStructure;
TIM_OCInitTypeDef					TIM_OCInitStructure;
TIM_ICInitTypeDef					TIM_ICInitStructure;
GPIO_InitTypeDef					GPIO_InitStructure;
EXTI_InitTypeDef   				EXTI_InitStructure;
// ________________________________________________________________________________
// GPIO setup
		GPIO_StructInit(&GPIO_InitStructure);
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;

#ifdef _VBUS_BIT
		GPIO_InitStructure.GPIO_Pin = _VBUS_BIT;
		GPIO_Init(_VBUS_PORT, &GPIO_InitStructure);
		GPIO_SetBits(_VBUS_PORT,_VBUS_BIT);
#endif
	
		GPIO_StructInit(&GPIO_InitStructure);
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
		GPIO_Init(GPIOA, &GPIO_InitStructure);
		GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_TIM13);
		GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_TIM14);	
// ________________________________________________________________________________
// TRIGGER 1, TRIGGER 2, 
// TRIGGER 3, IGBT Reset, PFM8 only
//
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
		GPIO_InitStructure.GPIO_Pin = _TRIGGER1_BIT;
		GPIO_Init(_TRIGGER1_PORT, &GPIO_InitStructure);
		_TRIGGER1_OFF;
		GPIO_InitStructure.GPIO_Pin = _TRIGGER2_BIT;
		GPIO_Init(_TRIGGER2_PORT, &GPIO_InitStructure);
		_TRIGGER2_OFF;
		GPIO_InitStructure.GPIO_Pin = _TRIGGER3_BIT;
		GPIO_Init(_TRIGGER3_PORT, &GPIO_InitStructure);
		_TRIGGER3_OFF;
		GPIO_InitStructure.GPIO_Pin = _IGBT_RESET_BIT;
		GPIO_Init(_IGBT_RESET_PORT, &GPIO_InitStructure);
		_IGBT_RESET;
// ________________________________________________________________________________
// NRST, BOOT
//
#if defined (_NRST_DISABLE_BIT) && defined (_BOOT_ENABLE_BIT)
		GPIO_ResetBits(_NRST_DISABLE_PORT,_NRST_DISABLE_BIT);
		GPIO_SetBits(_BOOT_ENABLE_PORT,_BOOT_ENABLE_BIT);
		GPIO_InitStructure.GPIO_Pin = _NRST_DISABLE_BIT;
		GPIO_Init(_NRST_DISABLE_PORT, &GPIO_InitStructure);	
		GPIO_InitStructure.GPIO_Pin = _BOOT_ENABLE_BIT;
		GPIO_Init(_BOOT_ENABLE_PORT, &GPIO_InitStructure);
#endif
#if defined (_USB_PIN_BIT) && defined (_USB_DIR_BIT)
		GPIO_InitStructure.GPIO_Pin = _USB_PIN_BIT;
		GPIO_Init(_USB_PIN_PORT, &GPIO_InitStructure);
		GPIO_SetBits(_USB_PIN_PORT,_USB_PIN_BIT);
		
		GPIO_InitStructure.GPIO_Pin = _USB_DIR_BIT;
		GPIO_Init(_USB_DIR_PORT, &GPIO_InitStructure);
		GPIO_ResetBits(_USB_DIR_PORT,_USB_DIR_BIT);
#endif
#if defined (_USB_SENSE_BIT)
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
		GPIO_InitStructure.GPIO_Pin = _USB_SENSE_BIT;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
		GPIO_Init(_USB_SENSE_PORT, &GPIO_InitStructure);
#endif
// ________________________________________________________________________________	
#if defined (_CWBAR_INT_pin)
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
		GPIO_InitStructure.GPIO_Pin = _CWBAR_BIT;					
		GPIO_Init(_CWBAR_PORT, &GPIO_InitStructure);
		
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
		SYSCFG_EXTILineConfig(_CWBAR_INT_port, _CWBAR_INT_pin);
		EXTI_ClearITPendingBit(_CWBAR_INT_line);
		EXTI_InitStructure.EXTI_Line = _CWBAR_INT_line;
		EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
		EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;  
		EXTI_InitStructure.EXTI_LineCmd = ENABLE;
		EXTI_Init(&EXTI_InitStructure);
#endif
// ________________________________________________________________________________		
// 	FAULT port && interrupt, IGBT Ready (PFM8 only)
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP ;

		GPIO_InitStructure.GPIO_Pin = _FAULT_BIT;		
		GPIO_Init(_FAULT_PORT, &GPIO_InitStructure);
		GPIO_InitStructure.GPIO_Pin = _IGBT_READY_BIT;						
		GPIO_Init(_IGBT_READY_PORT, &GPIO_InitStructure);
		
		SYSCFG_EXTILineConfig(_FAULT_INT_port, _FAULT_INT_pin);
		EXTI_ClearITPendingBit(_FAULT_INT_line);
		EXTI_InitStructure.EXTI_Line = _FAULT_INT_line;
		EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
		EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;  
		EXTI_InitStructure.EXTI_LineCmd = ENABLE;
		EXTI_Init(&EXTI_InitStructure);
// ________________________________________________________________________________
// TIM1, TIM8 IGBT pwm outputs
		GPIO_StructInit(&GPIO_InitStructure);
		GPIO_InitStructure.GPIO_PuPd = _PWM_3STATE;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;

		GPIO_PinAFConfig(GPIOE, GPIO_PinSource9,	GPIO_AF_TIM1);
		GPIO_PinAFConfig(GPIOE, GPIO_PinSource11, GPIO_AF_TIM1);
		GPIO_PinAFConfig(GPIOE, GPIO_PinSource13, GPIO_AF_TIM1);
		GPIO_PinAFConfig(GPIOE, GPIO_PinSource14, GPIO_AF_TIM1);
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_11 | GPIO_Pin_13 | GPIO_Pin_14 ;
		GPIO_Init(GPIOE, &GPIO_InitStructure);

		GPIO_PinAFConfig(GPIOC, GPIO_PinSource6, GPIO_AF_TIM8);
		GPIO_PinAFConfig(GPIOC, GPIO_PinSource7, GPIO_AF_TIM8);
		GPIO_PinAFConfig(GPIOC, GPIO_PinSource8, GPIO_AF_TIM8);
		GPIO_PinAFConfig(GPIOC, GPIO_PinSource9, GPIO_AF_TIM8);
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 ;
		GPIO_Init(GPIOC, &GPIO_InitStructure);
		
#if defined __PFM8__

		GPIO_PinAFConfig(GPIOA, GPIO_PinSource0, GPIO_AF_TIM2);
		GPIO_PinAFConfig(GPIOA, GPIO_PinSource1, GPIO_AF_TIM2);
		GPIO_PinAFConfig(GPIOB, GPIO_PinSource10, GPIO_AF_TIM2);
		GPIO_PinAFConfig(GPIOB, GPIO_PinSource11, GPIO_AF_TIM2);
		
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
		GPIO_Init(GPIOA, &GPIO_InitStructure);
		
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11;
		GPIO_Init(GPIOB, &GPIO_InitStructure);

		GPIO_PinAFConfig(GPIOD, GPIO_PinSource12, GPIO_AF_TIM4);
		GPIO_PinAFConfig(GPIOD, GPIO_PinSource13, GPIO_AF_TIM4);
		GPIO_PinAFConfig(GPIOD, GPIO_PinSource14, GPIO_AF_TIM4);
		GPIO_PinAFConfig(GPIOD, GPIO_PinSource15, GPIO_AF_TIM4);
		
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15 ;
		GPIO_Init(GPIOD, &GPIO_InitStructure);
#endif
// ________________________________________________________________________________
// TIMebase setup
		TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
		TIM_OCStructInit(&TIM_OCInitStructure);
		TIM_TimeBaseStructure.TIM_Prescaler = 0;
		TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_CenterAligned1;
//		TIM_TimeBaseStructure.TIM_RepetitionCounter=1;
// ________________________________________________________________________________
// TIM 1,8
		TIM_TimeBaseStructure.TIM_Period = _PWM_RATE_HI;
		TIM_DeInit(TIM1);
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
		TIM_TimeBaseInit(TIM1,&TIM_TimeBaseStructure);
		TIM_DeInit(TIM8);
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM8, ENABLE);
		TIM_TimeBaseInit(TIM8,&TIM_TimeBaseStructure);
#if defined __PFM8__
		TIM_TimeBaseStructure.TIM_Period = _PWM_RATE_HI/2;
		TIM_DeInit(TIM2);
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
		TIM_TimeBaseInit(TIM2,&TIM_TimeBaseStructure);
		TIM_DeInit(TIM4);
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
		TIM_TimeBaseInit(TIM4,&TIM_TimeBaseStructure);
#endif
		TIM_SetCounter(TIM1,0);
		TIM_SetCounter(TIM8,_SIMMER_HIGH/2);
#if defined __PFM8__		
		TIM_SetCounter(TIM2,_SIMMER_HIGH/8);
		TIM_SetCounter(TIM4,3*_SIMMER_HIGH/8);
#endif
// ________________________________________________________________________________
// TIM13,14, fan pwm, tacho
		TIM_TimeBaseStructure.TIM_Period = _FAN_PWM_RATE/2;
 		TIM_DeInit(TIM13);
 		TIM_DeInit(TIM14);
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM13, ENABLE);
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM14, ENABLE);
		TIM_TimeBaseInit(TIM13,&TIM_TimeBaseStructure);
		TIM_TimeBaseInit(TIM14,&TIM_TimeBaseStructure);
// ________________________________________________________________________________
// Output Compares	TIM1,8
		TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
		TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
		TIM_OCInitStructure.TIM_Pulse=0;
		TIM_OCInitStructure.TIM_OCPolarity = _PWM_HIGH;

		TIM_OC1Init(TIM1, &TIM_OCInitStructure);
		TIM_OC1Init(TIM8, &TIM_OCInitStructure);
		TIM_OC3Init(TIM1, &TIM_OCInitStructure);
		TIM_OC3Init(TIM8, &TIM_OCInitStructure);
#if defined __PFM8__
		TIM_OC1Init(TIM2, &TIM_OCInitStructure);
		TIM_OC1Init(TIM4, &TIM_OCInitStructure);
		TIM_OC3Init(TIM2, &TIM_OCInitStructure);
		TIM_OC3Init(TIM4, &TIM_OCInitStructure);
#endif
		TIM_OCInitStructure.TIM_Pulse=_PWM_RATE_HI;
		TIM_OCInitStructure.TIM_OCPolarity = _PWM_LOW;

		TIM_OC2Init(TIM1, &TIM_OCInitStructure);
		TIM_OC2Init(TIM8, &TIM_OCInitStructure);
		TIM_OC4Init(TIM1, &TIM_OCInitStructure);
		TIM_OC4Init(TIM8, &TIM_OCInitStructure);
#if defined __PFM8__
		TIM_OC2Init(TIM2, &TIM_OCInitStructure);
		TIM_OC2Init(TIM4, &TIM_OCInitStructure);
		TIM_OC4Init(TIM2, &TIM_OCInitStructure);
		TIM_OC4Init(TIM4, &TIM_OCInitStructure);
#endif
// ________________________________________________________________________________
// Output Compares, CH1	TIM13
		TIM_OCInitStructure.TIM_Pulse=1;
		TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
		TIM_OC1Init(TIM13, &TIM_OCInitStructure);

// Input Captures CH1 TIM14

		TIM_ICStructInit(&TIM_ICInitStructure);												// Input Capture channels
		TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Falling;	// Falling edge capture
		TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
		TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV8;
		TIM_ICInitStructure.TIM_ICFilter = 15;

		TIM_ICInitStructure.TIM_Channel = TIM_Channel_1;
		
		TIM_ICInit(TIM14, &TIM_ICInitStructure);
		TIM_ITConfig(TIM14, TIM_IT_CC1,ENABLE);

// enable outputs, brez pulzov!!!
		TIM_SelectMasterSlaveMode(TIM1, TIM_MasterSlaveMode_Enable);	// T1 -> master mode
		TIM_SelectOutputTrigger(TIM1, TIM_TRGOSource_Enable); 				// trigger enable event

// trigger T8 za T1,DAC in ADC !!!
		TIM_SelectSlaveMode(TIM8, TIM_SlaveMode_Trigger); 						// T8 -> slave mode
		TIM_SelectInputTrigger(TIM8, TIM_TS_ITR0); 										// T8 started from T1 trigger enable event
		TIM_SelectOutputTrigger(TIM8, TIM_TRGOSource_Update);					// triggers ADC, DAC on update

#if defined __PFM8__
		TIM_SelectSlaveMode(TIM2, TIM_SlaveMode_Trigger); 						// T2 -> slave mode
		TIM_SelectSlaveMode(TIM4, TIM_SlaveMode_Trigger); 						// T4 -> slave mode
		TIM_SelectInputTrigger(TIM2, TIM_TS_ITR0); 										// T2 started from T1 trigger enable event
		TIM_SelectInputTrigger(TIM4, TIM_TS_ITR0); 										// T4 started from T1 trigger enable event
#endif

		TIM_CtrlPWMOutputs(TIM13, ENABLE);
		TIM_Cmd(TIM1,ENABLE);
		TIM_Cmd(TIM13,ENABLE);
		TIM_Cmd(TIM14,ENABLE);
		TIM_CtrlPWMOutputs(TIM1, ENABLE);
		TIM_CtrlPWMOutputs(TIM8, ENABLE);			
		
		_TIM.Hvref=0;
		_TIM.Caps=2000;																								// 2000 u
		_TIM.Icaps=1000;																							// 1A
		}
#if defined __PFM8__
/*******************************************************************************
* Function Name  : Initialize_F2V()
* Description    : reconfigures the CAN rx tx to pfm8 functionality
* Input          : None
* Output         : None
* Return         : 
*******************************************************************************/
void 		*Initialize_F2V(_proc *p) {
TIM_TimeBaseInitTypeDef	TIM_TimeBaseStructure;
GPIO_InitTypeDef				GPIO_InitStructure;
TIM_ICInitTypeDef				TIM_ICInitStructure;
	
	if(!p) {
		GPIO_StructInit(&GPIO_InitStructure);
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
		GPIO_InitStructure.GPIO_Pin = _ERROR_OW_BIT;
		GPIO_Init(_ERROR_OW_PORT, &GPIO_InitStructure);
		GPIO_ResetBits(_ERROR_OW_PORT,_ERROR_OW_BIT);

		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
		GPIO_InitStructure.GPIO_Pin = _F2V_OW_BIT;
		GPIO_Init(_F2V_OW_PORT, &GPIO_InitStructure);
		GPIO_PinAFConfig(_F2V_OW_PORT, _F2V_OW_AF, GPIO_AF_TIM3);

		TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
		TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
		TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
		TIM_TimeBaseStructure.TIM_Prescaler = 0;
		TIM_TimeBaseStructure.TIM_Period = 0xFFFF;
		TIM_DeInit(TIM3);
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
		TIM_TimeBaseInit(TIM3,&TIM_TimeBaseStructure);	

		TIM_ICStructInit(&TIM_ICInitStructure);	
		TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Falling;
		TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
		TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
		TIM_ICInitStructure.TIM_ICFilter = 0;
		TIM_ICInitStructure.TIM_Channel = TIM_Channel_2;
		TIM_PWMIConfig(TIM3, &TIM_ICInitStructure);

		TIM_SelectInputTrigger(TIM3, TIM_TS_TI2FP2);
		TIM_SelectSlaveMode(TIM3, TIM_SlaveMode_Reset);
		TIM_SelectMasterSlaveMode(TIM3,TIM_MasterSlaveMode_Enable);

		TIM_Cmd(TIM3,ENABLE);
		_proc_add((func *)Initialize_F2V,NULL,"F2V",1);
	} else {
		if(pfm->Burst && TIM_GetCapture2(TIM3)) {
			pfm->Burst->Pmax=600000*_PWM_RATE_HI/TIM_GetCapture2(TIM3)/_AD2HV(pfm->HVref);
			if(pfm->Trigger.timeout && __time__ >= pfm->Trigger.timeout) {
				SetPwmTab(pfm);
				pfm->Trigger.timeout=0;
			}
		}
		if(pfm->Error)
			GPIO_SetBits(_ERROR_OW_PORT,_ERROR_OW_BIT);
		else
			GPIO_ResetBits(_ERROR_OW_PORT,_ERROR_OW_BIT);
	}
	return Initialize_F2V;
}
#endif
/**
  ******************************************************************************
  * @file			timers.c
  * @author		Fotona d.d.
  * @version	V1
  * @date			30-Sept-2013
  * @brief   	TIM1/8 handler
  *
  */
void		TIM1_UP_TIM10_IRQHandler(void) {
//static
//int			e1=0,
//				e2=0;
static 
int			x=0,y=0;
int 		hv,j,k,ki=30,kp=0;

				TIM_ClearITPendingBit(TIM1, TIM_IT_Update);								// brisi ISR flage
				if(TIM1->CR1 & TIM_CR1_DIR)
					return;
				
				TIM8->CCR1 = TIM1->CCR1 = x;			
				TIM8->CCR3 = TIM1->CCR3 = y;
#if defined __PFM8__
				TIM4->CCR1 = TIM2->CCR1 = TIM1->CCR1/2;			
				TIM4->CCR3 = TIM2->CCR3 = TIM1->CCR3/2;
#endif
				if(_MODE(pfm,_XLAP_SINGLE)) {															//----- x1, x2, x4 ------------
					TIM8->CCR2 = TIM1->CCR2 = TIM1->CCR1;
					TIM8->CCR4 = TIM1->CCR4 = TIM1->CCR3;
#if defined __PFM8__
					TIM4->CCR2 = TIM2->CCR2 = TIM2->CCR1;
					TIM4->CCR4 = TIM2->CCR4 = TIM2->CCR3;
#endif
				} else {
					TIM8->CCR2 = TIM1->CCR2 = TIM1->ARR - TIM1->CCR1;
					TIM8->CCR4 = TIM1->CCR4 = TIM1->ARR - TIM1->CCR3;
#if defined __PFM8__
					TIM4->CCR2 = TIM2->CCR2 = TIM2->ARR - TIM2->CCR1;
					TIM4->CCR4 = TIM2->CCR4 = TIM2->ARR - TIM2->CCR3;
#endif
				}

#if defined __DISC4__ && defined __PFM8__
				DAC_SetDualChannelData(DAC_Align_12b_R,5*y,5*x);
				DAC_DualSoftwareTriggerCmd(ENABLE);		
#endif
				if(_TIM.active & PFM_STAT_SIMM1)
					x=pfm->Simmer.pw[0];
				else
					x=0;
				if(_TIM.active & PFM_STAT_SIMM2)
					y=pfm->Simmer.pw[1];
				else
					y=0;
				
				if(!_TIM.p1 && !_TIM.p2) {																//----- end of burst, stop IT, notify main loop ---------------------------				
					TIM_ITConfig(TIM1,TIM_IT_Update,DISABLE);
					_SET_EVENT(pfm,_PULSE_FINISHED);
					_CLEAR_MODE(pfm,_PULSE_INPROC);
#if !defined __PFM8__
					TIM_SelectOnePulseMode(TIM4, TIM_OPMode_Single);				//----- Qswitch pasus
#endif					
					return;
				}

//----- HV voltage averaging, calc. active ADC DMA index----------------------------------

				for(hv=j=0;j<_AVG3;++j)																		// 
					hv+=(unsigned short)(ADC3_buf[j].HV);
																																	// --- on first entry, compute DMA index 
				k = _TIM.eint*_uS/_MAX_ADC_RATE;
				k-= DMA_GetCurrDataCounter(DMA2_Stream4) / sizeof(_ADCDMA)*sizeof(short);

				if(!_MODE(pfm,__TEST__))																	// ---  on __TEST__ mode, leave virt. reference for HV
					_TIM.Hvref=pfm->HVref;
				
				if(_TIM.p1) {																							//----- channel 1 -----------
					x=_TIM.p1->T;

//----- mode 9, forward voltage stab. ---------------------------------------------------	

					if(_MODE(pfm,_U_LOOP)) {
						x = (x * _TIM.Hvref + hv/2)/hv;

//					if(m && z1 > pfm->Burst->Pdelay*2) {
//						for(i=4;i<8;++i)
//							e1+=(short)(ADC1_buf[k-i].U) * (short)(ADC1_buf[k-i].I-_TIM.I1off);
//					}
//					else if (!_E1ref) {
//						_E1ref=e1;
//						e1=0;
//					}

//					if(m && z2 > pfm->Burst->Pdelay*2) {
//						for(i=4;i<8;++i)
//							e2+=(short)(ADC2_buf[k-i].U) * (short)(ADC2_buf[k-i].I-_TIM.I2off);
//					} else if (!_E2ref) {
//						_E2ref=e2;
//						e2=0;
//					}
//----- mode 10, current stab. ---------------------------------------------------									
//
						if(pfm->Burst->Time > 200 && _MODE(pfm,_P_LOOP)) {	
//----- current loop for ch1, if cref1 present !----------------------------------									
							if(_TIM.cref1 && _TIM.p1->T > pfm->Burst->Pdelay*2) {
								int dx=(_TIM.cref1 - ADC1_buf[k-5].U * ADC1_buf[k-5].I)/4096;
								_TIM.ci1 += dx*ki;
								x += _TIM.ci1/4096 + dx*kp/4096;
							}
//----- calc. cref1 after 200 us for ch1 -----------------------------------------									
							if(k > 200 + pfm->Burst->Delay) {
								if(!_TIM.cref1 && _TIM.p1->T > 2*pfm->Burst->Pdelay) {
									int n,jU=0,jI=0;
									for(n=5;n<13;++n)	{
										jU+=ADC1_buf[k-n].U;
										jI+=ADC1_buf[k-n].I;
									}
									_TIM.cref1=jU*jI/64;
									_TIM.ci1=0;
								}
							}		
						}
					}					
//----- vpis v OC registre ---------------------------------------------------------------
					if(_TIM.p1->n) {																				// set simmer pw on last sample !
						x = __max(pfm->Burst->Pdelay,__min(_MAX_PWM_RATE, x));			
						if(_TIM.m1++ == _TIM.p1->n/2) {												//----- pwch tabs increment ---
							_TIM.m1=0;
							++_TIM.p1;
						}
					} else {
						x = pfm->Simmer.pw[0];
						_TIM.p1=NULL;
					}
				}
				if(_TIM.p2) {																							//----- channel 2 -----------
					y=_TIM.p2->T;
//----- mode 9, forward voltage stab. ---------------------------------------------------	
					if(_MODE(pfm,_U_LOOP)) {
						y = (y * _TIM.Hvref + hv/2)/hv;
				
//					if(m && z1 > pfm->Burst->Pdelay*2) {
//						for(i=4;i<8;++i)
//							e1+=(short)(ADC1_buf[k-i].U) * (short)(ADC1_buf[k-i].I-_TIM.I1off);
//					}
//					else if (!_E1ref) {
//						_E1ref=e1;
//						e1=0;
//					}

//					if(m && z2 > pfm->Burst->Pdelay*2) {
//						for(i=4;i<8;++i)
//							e2+=(short)(ADC2_buf[k-i].U) * (short)(ADC2_buf[k-i].I-_TIM.I2off);
//					} else if (!_E2ref) {
//						_E2ref=e2;
//						e2=0;
//					}
//----- mode 10, current stab. ---------------------------------------------------									
//
						if(pfm->Burst->Time>200 && _MODE(pfm,_P_LOOP)) {	
//----- current loop for ch2, if cref2 present !----------------------------------									
							if(_TIM.cref2 && _TIM.p2->T > pfm->Burst->Pdelay*2) {
								int dx=(_TIM.cref2 - ADC2_buf[k-5].U * ADC2_buf[k-5].I)/4096;
								_TIM.ci2 += dx*ki;
								y += _TIM.ci2/4096 + dx*kp/4096;
							}
//----- calc. cref2 after 200 us for ch2 -----------------------------------------									
							if(k > 200 + pfm->Burst->Delay) {
								if(!_TIM.cref2 && _TIM.p2->T > 2*pfm->Burst->Pdelay) {
									int n,jU=0,jI=0;
									for(n=5;n<13;++n)	{
										jU+=ADC2_buf[k-n].U;
										jI+=ADC2_buf[k-n].I;
									}
									_TIM.cref2=jU*jI/64;
									_TIM.ci2=0;
								}
							}		
						}
					}					
//----- vpis v OC registre ---------------------------------------------------------------
					if(_TIM.p2->n)	{																				// set simmer pw on last sample !
						y = __max(pfm->Burst->Pdelay,__min(_MAX_PWM_RATE, y));
						
						if(_TIM.m2++ == _TIM.p2->n/2) {												//----- pwch tabs increment ---	
							_TIM.m2=0;
							++_TIM.p2;
						}
					} else {
						y = pfm->Simmer.pw[1];
						_TIM.p2=NULL;
					}
				}

				if(_MODE(pfm,__TEST__)) {																	//----- mode  29, simulacija klasicnega odziva, =C cap (mF), =P current(A)
					_SET_MODE(pfm,_U_LOOP);																	// obveze U stab. ker modificiramo HV referenco
					if(k>5)																									// scale fakt. za C v uF pri 880V/1100A full scale, 100kHz sample rate pride ~80... ni placa za izpeljavo		
						_TIM.Hvref -= (ADC1_buf[k-5].I+ADC2_buf[k-5].I)/80*1000/_TIM.Caps;	
				}
#if !defined __PFM8__
				if(_TIM.p1 && _TIM.p1->T > pfm->Burst->Pdelay && 					//----- Qswitch pasus, dela samo na ch 1 -------------------------------------------------
					(_TIM.p1->n == pfm->Pockels.trigger || 
						_TIM.p1->n == 255)) {																	// #jhw9847duhd		dodatek za qswitch	
					TIM_SelectOnePulseMode(TIM4, TIM_OPMode_Repetitive);		// triganje na kakrsnokoli stanje nad delay x 2
					TIM_Cmd(TIM4,ENABLE);																		// trigger !!!
				} else
					TIM_SelectOnePulseMode(TIM4, TIM_OPMode_Single);				// single pulse, timer se disabla po izteku				
#endif									
				if(TIM1->CCR1==_MAX_PWM_RATE || TIM1->CCR3==_MAX_PWM_RATE)// duty cycle 100% = PSRDYN error
 					_SET_ERROR(pfm,PFM_ERR_PSRDYN);
}
/*******************************************************************************/
/**
	* @brief	TIM13,14 IC2 ISR
	* @param	: None
	* @retval : None
	*/
/*******************************************************************************/
void		TIM8_TRG_COM_TIM14_IRQHandler(void) {
				if(TIM_GetITStatus(TIM14,TIM_IT_CC1)==SET) {
					TIM_ClearITPendingBit(TIM14, TIM_IT_CC1);
					_SET_EVENT(pfm,_FAN_TACHO);
				}
}
/*******************************************************************************/
/**
  * @brief  - main crowbar interrupt
	*					- IGBT fault driver interrupt
  * @param  None
  * @retval None
  */
void 		__EXTI_IRQHandler(void)
{
				if(EXTI_GetITStatus(_CWBAR_INT_line) == SET) { 						// CROWBAR
					EXTI_ClearITPendingBit(_CWBAR_INT_line);								// clear flag
					if(_MODE(pfm,_F2V)) {																		// F2V mode, pfm8
						if(!_PFM_CWBAR && !pfm->Trigger.timeout) {						// rising edge, trigger
							_SET_EVENT(pfm,_TRIGGER);
						}	else {																							// falling edge, pulse rearm
							pfm->Trigger.timeout=__time__+2;
						}							
					} else {																								// pfm6 mode
						if(_PFM_CWBAR) {																			// rising edge, main error reset & restart
							_SET_STATUS(pfm,_PFM_CWBAR_STAT);
							_CLEAR_ERROR(pfm, _CRITICAL_ERR_MASK);
							_ENABLE_PWM_OUT();
						}	else {																							// falling edge, crowbar fired error
							_CLEAR_STATUS(pfm,_PFM_CWBAR_STAT);
							_SET_ERROR(pfm,PFM_ERR_PULSEENABLE);
						}
					}
				}

				if(EXTI_GetITStatus(_FAULT_INT_line) == SET) { 						// IGBT FAULT					
					EXTI_ClearITPendingBit(_FAULT_INT_line);								// clear flag
					if(_PFM_CWBAR)
						_SET_ERROR(pfm,PFM_ERR_DRVERR);
				}
}
/*******************************************************************************/
/**
	* @brief	Trigger call
	* @param	: None
	* @retval : None
	*/
/*******************************************************************************/
void		Trigger(PFM *p) {
//_______________________________________________________________________________
				if(!_MODE(p,_PULSE_INPROC)) {
					if(_MODE(p,_ENM_NOTIFY)) {
						CanTxMsg tx = {_ID_SYS_TRIGG,0,CAN_ID_STD,CAN_RTR_DATA,0,0,0,0,0,0,0,0,0};
						while(CAN_TransmitStatus(__CAN__, 0) == CAN_TxStatus_Pending &&
							CAN_TransmitStatus(__CAN__, 1) == CAN_TxStatus_Pending &&
								CAN_TransmitStatus(__CAN__, 2) == CAN_TxStatus_Pending) {
									_proc_loop();
								}
						_YELLOW1(50);
						CAN_Transmit(__CAN__,&tx);
					}
//_______________________________________________________________________________
					_TIM.active=p->Simmer.active;														// find active channel
					if(_MODE(pfm,_CHANNEL1_DISABLE)) {											// single channel 2 mode
						if(_MODE(pfm,_ALTERNATE_TRIGGER)) {										// altenate trigger
							if(p->Trigger.counter % 2) {
								_TIM.eint=_TIM.eint2;
								if(_TIM.active & PFM_STAT_SIMM2)
									_TIM.p2 = _TIM.pwch2;
							} else {
								_TIM.eint=_TIM.eint1;
								if(_TIM.active & PFM_STAT_SIMM1)
									_TIM.p2 = _TIM.pwch1;
							} 
						} else if(_TIM.active & PFM_STAT_SIMM1)	{							// simult. trigger
							_TIM.p2 = _TIM.pwch1;
							_TIM.eint=_TIM.eint1;
						} else if(_TIM.active & PFM_STAT_SIMM2) {
							_TIM.p2 = _TIM.pwch2;
							_TIM.eint=_TIM.eint2;
						}
					} else if(_MODE(pfm,_CHANNEL2_DISABLE)) {								// single channel 1 mode
							if(_MODE(pfm,_ALTERNATE_TRIGGER)) {									// altenate trigger
								if(p->Trigger.counter % 2) {
									_TIM.eint=_TIM.eint2;
									if(_TIM.active & PFM_STAT_SIMM2)
										_TIM.p1 = _TIM.pwch2;
								} else {
									_TIM.eint=_TIM.eint1;
									if(_TIM.active & PFM_STAT_SIMM1)
										_TIM.p1 = _TIM.pwch1;
								} 
						} else if(_TIM.active & PFM_STAT_SIMM1) {							// simult. trigger
							_TIM.p1 = _TIM.pwch1;
							_TIM.eint=_TIM.eint1;
						}	else if(_TIM.active & PFM_STAT_SIMM2) {
							_TIM.p1 = _TIM.pwch2;
							_TIM.eint=_TIM.eint2;
						}
					} else {																								// dual channel mode
						if(_MODE(pfm,_ALTERNATE_TRIGGER)) {										// altenate trigger
							if(p->Trigger.counter % 2) {
								_TIM.eint=_TIM.eint2;
								if(_TIM.active & PFM_STAT_SIMM2)
									_TIM.p2 = _TIM.pwch2;
							} else {
								_TIM.eint=_TIM.eint1;
								if(_TIM.active & PFM_STAT_SIMM1)
									_TIM.p1 = _TIM.pwch1;
							}
						} else {																							// simult. trigger
							if(_TIM.active & PFM_STAT_SIMM1)
								_TIM.p1 = _TIM.pwch1;
							if(_TIM.active & PFM_STAT_SIMM2)
								_TIM.p2 = _TIM.pwch2;
							_TIM.eint=__max(_TIM.eint1,_TIM.eint2);
						}
					}					
					
					ADC_DMARequestAfterLastTransferCmd(ADC1, DISABLE);			// at least ADC conv. time before ADC/DMA change 
					ADC_DMARequestAfterLastTransferCmd(ADC2, DISABLE);
					_SET_MODE(p,_PULSE_INPROC);
					SetSimmerRate(p,_SIMMER_HIGH);
				} else					
					_DEBUG_(_DBG_SYS_MSG,"trigger aborted...");
}
/**
* @}
*/ 
