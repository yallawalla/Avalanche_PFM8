/**
  ******************************************************************************
  * @file    app.c
  * @author  Fotona d.d.
  * @version V1
  * @date    30-Sept-2013
  * @brief	 Main PFM6 application functionality
  *
  */
	
/** @addtogroup PFM6_Application
* @{
*/
#include	"pfm.h"
#include	<string.h>
#include 	<stdarg.h>
#include 	<stdio.h>

PFM				*pfm;
_io				*__com1,*__com3,*__com6;
const char *_errStr[]={
					"simmer 1 failed",
					"simmer 2 failed",
					"illegal parameter",
					"illegal flash idle voltage",
					"IGBT overheat",
					"IGBT fault",
					"IGBT not ready",
					"Crowbar fired",
					"PWM overrange",
#if	defined (__PFM6__)
					"20V supply failure",
					"-5V supply failure",
#elif	defined (__PFM8__)
					"24V supply failure",
					"5V supply failure",
#else
					"unspecified....",
					"unspecified....",
#endif
					"(800)...unspecified ",
					"HV out of range",
					"IGBT fan error",
					"HV mid voltage out of range",
					"I2C comm. error",
					"VCAP1 error",
					"VCAP2 error",
					"ext. trigger error",
					"(80000)...unspecified",
					"(100000)...unspecified",
					"(200000)...unspecified",
					"(400000)...unspecified",
					"(800000)...unspecified",
					"err charger status 01",
					"err charger status 02",
					"err charger status 03",
					"err charger status 08",
					"err charger status 10",
					"err charger status 20",
					"err charger status 40",
					"err charger status 80",
	NULL
};
/*______________________________________________________________________________
* Function Name : App_Init
* Description   : Initialize PFM object
* Input         : None
* Output        : None
* Return        : None
*/
void 			App_Init(void) {
	
					RCC_AHB1PeriphClockCmd(
						RCC_AHB1Periph_GPIOA |
						RCC_AHB1Periph_GPIOB |
						RCC_AHB1Periph_GPIOC | 
						RCC_AHB1Periph_GPIOD | 
						RCC_AHB1Periph_GPIOE |
						RCC_AHB1Periph_GPIOF |
						RCC_AHB1Periph_GPIOG, ENABLE);

					SystemCoreClockUpdate();

					pfm=calloc(1,sizeof(PFM));		
	
					pfm->Burst=pfm->burst;
					pfm->Burst->U=0;
					pfm->Burst->Time=100;
					pfm->Burst->Delay=100;
					pfm->Burst->N=1;
					pfm->Burst->Length=1000;
					pfm->Burst->Period=1000;
					pfm->Burst->Ereq=_SHPMOD_MAIN;
					pfm->Burst->Mode=_XLAP_QUAD;
					pfm->Burst->Pdelay=pfm->Burst->PW=_PWM_RATE_HI*0.02;
					pfm->Burst->max[0]=pfm->Burst->max[1]=_I2AD(1000);
					pfm->Burst->pockels.delay=0;
					pfm->Burst->pockels.width=0;
					pfm->Burst->pockels.trigger=0;
					memcpy(&pfm->burst[1],&pfm->burst[0],sizeof(burst));
	
					pfm->Trigger.count=pfm->Trigger.enotify=0;
						
					pfm->Simmer.mode=_XLAP_QUAD;
					pfm->Simmer.max=_I2AD(1000);
					pfm->Simmer.pw[0]=pfm->Simmer.pw[1]=200*_uS/1000;
					pfm->Simmer.rate[0]=pfm->Simmer.rate[1]=50*_uS;

					pfm->HVref=0;
					pfm->ADCRate=_uS;

					Initialize_NVIC();
					__com1=Initialize_USART1(921600);		
#if		defined (__PFM8__)
					__com3=Initialize_USART3(115200);		
					__com6=Initialize_USART6(115200);		
#endif
					__can=Initialize_CAN(0);
					Initialize_ADC();
					Initialize_TIM();
#if		!defined (__DISC4__) && !defined (__DISC7__)
					__charger6=Initialize_I2C(NULL, 0x58,50000);
#endif
					pfm->fatfs=calloc(1,sizeof(FATFS));
		
#define noise (rand()%100 - 50)
#if defined (__PFM6__) && ( defined  (__DISC4__) || defined  (__DISC7__) )
{
int				i,j;
					srand(__time__);
					for(i=0; i<_AVG3; ++i) {
						ADC3_buf[i].HV=_V2AD(700,2000,6.2) + noise;
						ADC3_buf[i].HV2=_V2AD(350,2000,6.2) + noise;
						for(j=0; j<sizeof(ADC3_buf[i].IgbtT)/sizeof(short); ++j)
							ADC3_buf[i].IgbtT[j]=2000 + noise;
						ADC3_buf[i].Um5=_Vn2AD(-6,24,12) + noise;
						ADC3_buf[i].Up20=_V2AD(18,68,12) + noise;
					}
}
#endif
#if defined (__PFM8__) && ( defined  (__DISC4__) || defined  (__DISC7__) )
{
int				i,j;
					srand(__time__);
					for(i=0; i<_AVG3; ++i) {
						ADC3_buf[i].HV=_V2AD(700,2000,8.0) + noise;
						ADC3_buf[i].HV2=_V2AD(350,2000,8.0) + noise;
						for(j=0; j<sizeof(ADC3_buf[i].IgbtT)/sizeof(short); ++j)
							ADC3_buf[i].IgbtT[j]=2000 + noise;
						ADC3_buf[i].VCAP1=_V2AD(350,2000,8.0) + noise;
						ADC3_buf[i].VCAP2=_V2AD(350,2000,8.0) + noise;
						ADC3_buf[i].Up12=_V2AD(12,62,10) + noise;
						ADC3_buf[i].Up5=_V2AD(5,10,10) + noise;
						ADC3_buf[i].Up3=_V2AD(3.3,10,10) + noise;
					}
					
}
#endif
//---------------------------------------------------------------------------------
					if(__com1)
						_proc_add(ParseCom,__com1,						"COM1 parser",0);
					if(__com3)
						_proc_add(ParseCom,__com3,						"COM3 parser",0);
					if(__com6)
						_proc_add(ParseCom,__com6,						"COM6 parser",0);
					
					_proc_add(ParseCanTx,pfm,								"CAN tx",0);
					_proc_add(ParseCanRx,pfm,								"CAN rx",0);
					_proc_add(ProcessingStatus,pfm,					"status",1);
					_proc_add(ProcessingEvents,pfm,					"events",0);
					_proc_add(ProcessingCharger,pfm,				"charger6",1);

#if		defined (__PFM6__) || defined (__PFM8__)
					_proc_add(Watchdog,NULL,								"watchdog",0);
					_proc_add(Lightshow,(void *)&__time__,	"leds",0);
#endif
//---------------------------------------------------------------------------------
					SysTick_init();
					SetSimmerRate(pfm,_SIMMER_LOW);
					SetPwmTab(pfm);
					Watchdog_init(300);	
					Initialize_DAC();
//---------------------------------------------------------------------------------
					_stdio(__com1);
					if(RCC_GetFlagStatus(RCC_FLAG_SFTRST) == SET)
						__print("\r ... SWR reset, %dMHz\r\n>",SystemCoreClock/1000000);
					else if(RCC_GetFlagStatus(RCC_FLAG_IWDGRST) == SET)
						__print("\r ... IWDG reset\r\n>");
					else if(RCC_GetFlagStatus(RCC_FLAG_WWDGRST) == SET)
						__print("\r ... WWDG reset\r\n>");
					else if(RCC_GetFlagStatus(RCC_FLAG_PORRST) == SET)
						__print("\r ... power on reset\r\n>");
					else if(RCC_GetFlagStatus(RCC_FLAG_PINRST) == SET)
					{} else
						{}
					RCC_ClearFlag(); 
					f_chdrive(FS_CPU);
					f_mount(pfm->fatfs,FS_CPU,1);
						ungets("@cfg.ini\r");
					_stdio(NULL);
}
/*______________________________________________________________________________
  * @brief	ISR events polling, main loop
  * @param	PFM object
  * @retval	: None
	*
  */
void			ProcessingEvents(_proc *proc) {
PFM				*p=proc->arg;
//______________________________________________________________________________
//______________________________________________________________________________
//_______Fan tacho processing context___________________________________________
//______________________________________________________________________________
					if(_EVENT(p,_FAN_TACHO)) {															// fan timeout counter reset
						_CLEAR_EVENT(p,_FAN_TACHO);
						p->fan_rate = __time__ + 500;
						_BLUE2(20);
					}
					if(__time__ > 10000 &&__time__ > p->fan_rate)
						_SET_ERROR(p,PFM_FAN_ERR);				
//					else
//						_CLEAR_ERROR(p,PFM_FAN_ERR);
//______________________________________________________________________________
//______________________________________________________________________________
//______________________________________________________________________________
//______________________________________________________________________________
//______________________________________________________________________________
#define 	_TRIGGER_RESET p->Trigger.time=p->Trigger.counter=0
//______________________________________________________________________________
//________Processing timed trigger______________________________________________
					do {
						int dt= _MODE(p,_ALTERNATE_TRIGGER) ? p->burst[p->Trigger.counter % 2].Period : p->Burst->Period;	
						if(_EVENT(p,_TRIGGER)) {
							_CLEAR_EVENT(p,_TRIGGER);
							if(p->Trigger.time) {
								if(_MODE(p,_AUTO_TRIGGER)) {
									_TRIGGER_RESET;
									break;
								}
								if(_MODE(p,_CHECK_TRIGGER) && abs(__time__ - p->Trigger.time) > 1) {
									_SET_ERROR(p,PFM_ERR_ETRIG);
									_TRIGGER_RESET;
									break;
								}
							}
							if((p->Error & _CRITICAL_ERR_MASK)) {
								_TRIGGER_RESET;
								break;
							}
							Trigger(p);
							p->Trigger.time = __time__ + dt;
							if(++p->Trigger.counter==p->Trigger.count)	
								_TRIGGER_RESET;
							break;
						} else {
							if(p->Trigger.time==0)
								break;
							if(__time__ >= p->Trigger.time && (_MODE(p,_AUTO_TRIGGER) || p->Trigger.count)) {
								p->Trigger.time=0;
								_SET_EVENT(p,_TRIGGER);
								continue;
							}
							if(__time__ - p->Trigger.time > dt*3 && _MODE(p,_CHECK_TRIGGER))
								_TRIGGER_RESET;
							break;								
						}
					} while(true);
//______________________________________________________________________________
					if(_EVENT(p,_PULSE_FINISHED)) {	
						_CLEAR_EVENT(p,_PULSE_FINISHED);											// end of pulse
						SetSimmerRate(p,_SIMMER_LOW);													// reduce simmer
						if(Eack(p)) {																					// Energ. integrator finished
							_TIM.cref1=_TIM.cref2=0;
							ScopeDumpBinary(NULL,0);														// scope printout, for testing(if enabled ?)
						}
					}
//______________________________________________________________________________
					if(_EVENT(p,_REBOOT)) {	
						_CLEAR_EVENT(p,_REBOOT);															// end of pulse
						WWDG_init();
					}
}
/*______________________________________________________________________________
  *
  *
  * @brief	periodic status/error  polling, main loop call from 1 msec event flag
  * @param  : current PFM object
  * @retval : None
  *
______________________________________________________________________________*/
void			PFM_debug(PFM *p) {
	static	int i,img=0;
					if(!p)
						img = 0;
					else 
						img = (p->Error ^ img) & p->Error;
					if(img) {
						for(i=0; i<32 && _errStr[i]; ++i)
							if(img & (1<<i))
								_DEBUG_(_DBG_ERR_MSG,"error %06X: %s",1<<i,(int)_errStr[i]);
					}
					img=p->Error;
}
/*______________________________________________________________________________
  *
  *
  * @brief	periodic status/error  polling, main loop call from 1 msec event flag
  * @param  : current PFM object
  * @retval : None
  *
______________________________________________________________________________*/
void			ProcessingStatus(_proc *proc) {
PFM				*p=proc->arg;
int 			i,j,k;
static		short	status_image=0; 
static		int		error_image=0;
static		int		bounce=0;
					
					for(i=j=k=0; i<_AVG3; ++i) {
						j+=ADC3_buf[i].HV;
						k+=ADC3_buf[i].HV2;
					}
					p->HV=j;	
					p->HV2=k;
//-------------------------------------------------------------------------------			
					p->Temp=IgbtTemp(T_MIN)/100;
					if((p->Temp > (fanTH+fanTH/2)/100) || (p->Temp < -20))
						_SET_ERROR(p,PFM_ERR_TEMP);
//					else
//						_CLEAR_ERROR(p,PFM_ERR_TEMP);
//-------------------------------------------------------------------------------
#if	defined (__PFM6__)
					p->Up20 += (8*(ADC3_buf[0].Up20) - p->Up20)/8;				
					p->Um5  += (8*(ADC3_buf[0].Um5)  - p->Um5)/8;
//-------------------------------------------------------------------------------								
					if(abs(p->Up20 - 8*_V2AD(20,68,12)) >  8*_V2AD(2,68,12) && __time__ > 3000)                                      
						_SET_ERROR(p,PFM_ERR_48V);
//					else
//						_CLEAR_ERROR(p,PFM_ERR_48V);
					
					if(abs(p->Um5 - 8*_Vn2AD(-5,24,12)) > 8*_V2AD(1,24,12) && __time__ > 3000)       
						_SET_ERROR(p,PFM_ERR_15V);
//					else
//						_CLEAR_ERROR(p,PFM_ERR_15V);

					if(ADC3_buf[0].HV > 100 && abs(ADC3_buf[0].HV-ADC3_buf[0].HV2) > ADC3_buf[0].HV/5 && __time__ > 3000)
						_SET_ERROR(p,PFM_HV2_ERR);
//					else
//						_CLEAR_ERROR(p,PFM_HV2_ERR);
//-------------------------------------------------------------------------------
#elif defined (__PFM8__)
					p->Up12 += (8*(ADC3_buf[0].Up12) - p->Up12)/8;
					p->Up5  += (8*(ADC3_buf[0].Up5)  - p->Up5)/8;
					p->Up3  += (8*(ADC3_buf[0].Up3)  - p->Up3)/8;

					if(abs(p->Up12 - 8*_V2AD(24.0,7500.0,560.0)) >  8*_V2AD(2.0,7500.0,560.0) && __time__ > 3000)                       
						_SET_ERROR(p,PFM_ERR_48V);
//					else
//						_CLEAR_ERROR(p,PFM_ERR_48V);
					
					if(abs(p->Up5 - 8*_V2AD(5.0,560.0,560.0)) >  8*_V2AD(0.5,560.0,560.0) && __time__ > 3000)                       
						_SET_ERROR(p,PFM_ERR_15V);
//					else
//						_CLEAR_ERROR(p,PFM_ERR_15V);
						
					if(!_IGBT_READY)
						_SET_ERROR(p,PFM_SCRFIRED);		
//					else
//						_CLEAR_ERROR(p,PFM_SCRFIRED);
					
					if(ADC3_buf[0].HV > _HV2AD(100.0)) {
						if(abs(ADC3_buf[0].HV/2-ADC3_buf[0].HV2) > ADC3_buf[0].HV/10)
							_SET_ERROR(p,PFM_HV2_ERR);
//						else
//							_CLEAR_ERROR(p,PFM_HV2_ERR);
						
						if(abs(ADC3_buf[0].HV/2-ADC3_buf[0].VCAP1) > ADC3_buf[0].HV/4)
							_SET_ERROR(p,PFM_ERR_VCAP1);
//						else
//							_CLEAR_ERROR(p,PFM_ERR_VCAP1);

						if(abs(ADC3_buf[0].HV/2-ADC3_buf[0].VCAP2) > ADC3_buf[0].HV/4)
							_SET_ERROR(p,PFM_ERR_VCAP2);
//						else
//							_CLEAR_ERROR(p,PFM_ERR_VCAP2);
					}
#else
#endif
					
#if defined (_NRST_DISABLE_BIT) && defined (_BOOT_ENABLE_BIT)
					if(p->boot_timeout && __time__ > p->boot_timeout) {
						GPIO_ToggleBits(_NRST_DISABLE_PORT,_NRST_DISABLE_BIT);
						GPIO_ToggleBits(_BOOT_ENABLE_PORT,_BOOT_ENABLE_BIT);
						p->boot_timeout=0;
					}
#endif
//-------------------------------------------------------------------------------
// - polovicna napetost na banki +/- 20%
// - meris sele od 100V naprej
// - vhod v AD za HV/2 je ze HW mnozen z 2 (pfm6 only)
//			
//-------------------------------------------------------------------------------
//				get current active channel....
//				razlika med HV in napetostjo na flesu mora  biti najmanj 12%, sicer simmer error
//
					if(!_MODE(p,_PULSE_INPROC) && !_TRIGGER1 && !_TRIGGER2) { 
//-------------------------------------------------------------------------------
						if(_STATUS(p,PFM_STAT_SIMM1) && abs(ADC3_buf[0].HV - ADC1_simmer.U) < ADC3_buf[0].HV/8)
							_SET_ERROR(p,PFM_ERR_SIMM1);
//						else
//							_CLEAR_ERROR(p,PFM_ERR_SIMM1);
//-------------------------------------------------------------------------------
						if(_STATUS(p,PFM_STAT_SIMM2) && abs(ADC3_buf[0].HV - ADC2_simmer.U) < ADC3_buf[0].HV/8)
							_SET_ERROR(p,PFM_ERR_SIMM2);
//						else
//							_CLEAR_ERROR(p,PFM_ERR_SIMM2);
					}
//-------------------------------------------------------------------------------
					if(p->Simmer.timeout && __time__ >= p->Simmer.timeout) {	
							_TRIGGER1_OFF;	
							_TRIGGER2_OFF;
							p->Simmer.timeout=0;
					}
//-------------------------------------------------------------------------------
					if((status_image != p->Status) || (error_image != p->Error)) {
						error_image = p->Error;	
						status_image = p->Status;
						bounce=25;
					} else if(bounce && !--bounce) {
						PFM_status_send(p);
					}
//-------------------------------------------------------------------------------
					if(_MODE(p,__TEST__) && !_MODE(p,_PULSE_INPROC) && !(__time__ % 100))
						if(_TIM.Hvref < p->HVref - p->HVref/15) {
							_TIM.Hvref = __min(p->HVref,_TIM.Hvref + _TIM.Icaps*400*4096/880/_TIM.Caps);
							_YELLOW2(20);
						}
						
					PFM_debug(p);
}
/*______________________________________________________________________________
  * @brief	Charger6 control procedure; Disables Charger6 if PFM_ERR_DRVERR,PFM_ERR_PULSEENABLE or 
	* _PFM_ADCWDG_ERR are set (from interrupts). Sets the PFM_STAT_PSRDY status bit, 
	* 
  * @param  : current PFM object
  * @retval : None
  *
____________________________________________________________________________*/
void			ProcessingCharger(_proc *proc) {
PFM				*p=proc->arg;
static
	int			ton=1500,					// _PFC_ON command delay
					toff=1000,				// _PFC_OFF command delay
					terr=0,						// shutdown repetition timer
					tpoll=10000;			// 10Hz pfc status polling timer
//-------------------------------------------------------------------------------
// status polling context
//
					if(__time__ >= tpoll) {
						tpoll = __time__ + 100;
						if(_ERROR(p,PFM_I2C_ERR))
							_CLEAR_ERROR(p,PFM_I2C_ERR);									// clear any previous i2c error 
						else {																					// 100 ms charger6 scan, stop when i2c comms error !
int						i=_STATUS_WORD;
							if(readI2C(__charger6,(char *)&i,2))					// add status word >> error status. byte 3
								p->Error = ((p->Error & ((1<<24)-1)) | ((i & 0xff)<<24)) & ~pfm->Errmask;
						}
					} 
//-------------------------------------------------------------------------------						
//	critical PFM error handling
//
					if(p->Error  & ~pfm->Errmask  & _CRITICAL_ERR_MASK) {
						if(p->Simmer.active)	{													// on error = simmer off
							PFM_command(p,0);
							}
						if(!terr--) {																		// elapsed ?
							int i=_PFC_OFF;																// PFC off
							writeI2C(__charger6,(char *)&i,2);	
							ADC_ITConfig(ADC3,ADC_IT_AWD,DISABLE);	
							terr=500;																			// nest handler delay
							ton=300;																			// recovery delay
							_RED2(100);																		// indicator !!!
						}
						return;
					}
//-------------------------------------------------------------------------------
					terr=0;																						// clear pending handler
					if(p->Error)																			// non crirical error indicator
						_RED2(100);
//-------------------------------------------------------------------------------
					if(abs(p->HV - p->HVref) < p->HVref/15)	{					// HV/15 +/- 50V limits at 750V
						_SET_STATUS(p,PFM_STAT_PSRDY);
						ADC_ClearITPendingBit(ADC3, ADC_IT_AWD);				// enable HW voltage watchdog
						ADC_ITConfig(ADC3,ADC_IT_AWD,ENABLE);					
						_GREEN2(100);																		// PSREADY indicator 
						toff=0;																					// clear any pending PFC off events
					} else {
						_CLEAR_STATUS(p,PFM_STAT_PSRDY);
						if(!ton && !toff) {															// skip. if contdown running
							if(p->HV > p->HVref) {												// set countdown on output overshot...
								toff=300;
								ton=3000;
							} else {																			// set countdown on output too low...
								toff=2700;
								ton=3000;
							}
						}
					}

					if(ton) {
						if(!--ton) {																		// switch on countdown
							int i=_PFC_ON;
							writeI2C(__charger6,(char *)&i,2);	
						}		
						if(ton==10)																			// load output voltage 10 ms prior to switch-on
							SetChargerVoltage(_AD2HV(p->HVref));
					}
						
					if(toff)
						if(!--toff) {																		// switch off countdown
							int i=_PFC_OFF;
							writeI2C(__charger6,(char *)&i,2);	
						}
}
/*******************************************************************************
* Function Name	: 
* Description		: 
* Output				:
* Return				:
*******************************************************************************/
int				Escape(void) {
static	
int 			timeout,seq;
int				i=getchar();
	
					if(i==EOF) {
						if(timeout && (__time__ > abs(timeout))) {
							timeout=0;
							return seq;
							}
					} else if(timeout > 0) {
						seq=(seq<<8) | i;
						if(i=='~' || i=='A' || i=='B' || i=='C' || i=='D') {
							timeout=0;
							return seq;
						}
					} else if(i==_Esc) {
						timeout=__time__+5;
						seq=i;
					} else {
						timeout=0;
						return i;
					}
					return EOF;
}
//______________________________________________________________________________________
void			Parse(int i) {
char 			*c;
					switch(i) {
						case EOF:																				// empty usart
							break;				
						case _CtrlZ:																		// call watchdog reset
							while(1);				
						case _CtrlY:																		// call system reset
							NVIC_SystemReset();				
						case _CtrlE:																		// can console - maintenance only
							CAN_console();	
							break;
						case _f12:																			// can console - maintenance only
						case _F12:																			// can console - maintenance only
							Tandem();
							break;						
//______________________________________________________________________________________
						case _CtrlT:
						{
							#include "tetris.h"
							tetris_run(10,20);
						}			
							break;	
						case _Esc:				
							_SET_EVENT(pfm,_TRIGGER);											// console esc +-	trigger... no ja!!
							break;				
						case _f1:	
						case _F1:	
							pfm->Trigger.counter=0;
							_SET_EVENT(pfm,_TRIGGER);
							break;				
						case _f2:
						case _F2:
							pfm->Trigger.counter=1;
							_SET_EVENT(pfm,_TRIGGER);
							break;				
						default:				
							c=cgets(i,EOF);				
							if(c) {		
								while(*c==' ') ++c;	
								if(stdin->io->arg.parse)
									i=stdin->io->arg.parse(c);
								else
									i=DecodeCom(c);
								if(*c && i)				
									__print("... WTF(%d)",i);									// error message
								if(stdin->io->arg.parse)										// call newline
									i=stdin->io->arg.parse(NULL);
								else
									i=DecodeCom(NULL);
							}
						}
}
//______________________________________________________________________________________
void			ParseCom(_proc *p) {
					if(p) {
						_io *in=stdin->io;
						_io *out=stdout->io;
						_stdio(p->arg);																// recursion lock
						Parse(Escape());
						stdin->io=in;
						stdout->io=out;
					} else
						Parse(Escape());
}
//______________________________________________________________________________________
void			ParseFile(FIL *f) {
_io				*io=stdout->io;
					stdout->io=NULL;
					Parse(f_getc(f));
					stdout->io=io;
}
/*______________________________________________________________________________
  * @brief  CAN transmit parser
  * @param:
  * @retval : None
	*
	*
  */
void			ParseCanTx(_proc *proc) {	
PFM				*p=proc->arg;
CanTxMsg	tx={0,0,CAN_ID_STD,CAN_RTR_DATA,0,0,0,0,0,0,0,0,0};
static 
	int			timeout=0;

					while(1) {
						if(CAN_TransmitStatus(__CAN__, 0) == CAN_TxStatus_Pending &&
							CAN_TransmitStatus(__CAN__, 1) == CAN_TxStatus_Pending &&
								CAN_TransmitStatus(__CAN__, 2) == CAN_TxStatus_Pending) {
									break;
								}
								
						if(_buffer_pull(__can->tx,&tx,sizeof(CanTxMsg)))	{
							CAN_Transmit(__CAN__,&tx);
						} else if(__can->arg.io && __time__ >= timeout) {
							tx.DLC=_buffer_pull(__can->arg.io->tx,tx.Data,8);
							if(tx.DLC > 0) {
								tx.StdId=_ID_PFMcom2SYS;
								CAN_Transmit(__CAN__,&tx);
								timeout = __time__ + 1;
							} else
								break;
						} else
							break;
						
						if(_DBG(p,_DBG_CAN_TX)) {
							_io *io=_stdio(__dbug);
							int i;
							__print(":%04d >%02X ",__time__ % 10000,tx.StdId);
							for(i=0;i<tx.DLC;++i)
								__print(" %02X",tx.Data[i]);
							__print("\r\n>");
							_stdio(io);
						}
					}
}					
/*______________________________________________________________________________
  * @brief  CAN message parser for PFM data as defined in CAN protocol  ICD 
  * @param rx: pointer to CanRxMsg structure
  * @retval : None
	*
	*
  */
void			ParseCanRx(_proc *proc) {	
CanRxMsg	rx;
short			n;
char			*q=(char *)rx.Data;
PFM				*p=proc->arg;
//______________________________________________________________________________________					
					if(_buffer_pull(__can->rx,&rx,sizeof(CanRxMsg))) {
						q=(char *)rx.Data;
//______________________________________________________________________________________
						if(_DBG(p,_DBG_CAN_RX))
						{
							_io *io=_stdio(__dbug);
							__print(":%04d <%02X ",__time__ % 10000,rx.StdId);
							for(n=0;n<rx.DLC;++n)
								__print(" %02X",rx.Data[n]);
							__print("\r\n>");
							_stdio(io);
						}
//______________________________________________________________________________________
//
						switch(rx.StdId) {
//______________________________________________________________________________________
//______________________________________________________________________________________
//______________________________________________________________________________________
//______________________________________________________________________________________
//______________________________________________________________________________________
//____________Tandem message block _____________________________________________________
//______________________________________________________________________________________
							case _ID_SYS_TRIGG:
								if(rx.DLC) {
									switch(rx.Data[0]) {
										case PFM_STAT_SIMM1:									// < 1a 01
											p->Trigger.counter=0;
											_SET_MODE(p,_ALTERNATE_TRIGGER);
										break;
										case PFM_STAT_SIMM2:									// < 1a 02
											p->Trigger.counter=1;
											_SET_MODE(p,_ALTERNATE_TRIGGER);
										break;
										case PFM_STAT_SIMM1 | PFM_STAT_SIMM2:	// < 1a 03
											_CLEAR_MODE(p,_ALTERNATE_TRIGGER);
											p->Trigger.counter=0;
										break;
										default:
											_SET_ERROR(p,PFM_ERR_UB);
											return;
									}
								}
								_SET_EVENT(p,_TRIGGER);										// < 1a
								break;
//______________________________________________________________________________________
							case _PFM_TAND_CH0:													// < 100 64 00 00 10 00 04 03 01
								p->Burst=&p->burst[0];
								p->Burst->Time	=*(short *)q++;q++;
								p->Burst->U			=*(short *)q++;q++;
								p->Burst->Length=*(short *)q++;q++;
								p->Burst->N			=*(char *)q++;
								p->Burst->Ereq	=*(char *)q++;
								_SetPwmTab(p,PFM_STAT_SIMM1);														
								break;
//______________________________________________________________________________________
							case _PFM_TAND_CH1:													// < 101 00 04  00 10 00 04 01 01
										p->Burst=&p->burst[1];
								p->Burst->Time	=*(short *)q++;q++;
								p->Burst->U			=*(short *)q++;q++;
								p->Burst->Length=*(short *)q++;q++;
								p->Burst->N			=*(char *)q++;
								p->Burst->Ereq	=*(char *)q++;
								_SetPwmTab(p,PFM_STAT_SIMM2);														
								break;
//______________________________________________________________________________________										
							case _PFM_TAND_DLY:
								p->burst[0].Delay=	*(short *)q++;q++;
								p->burst[1].Delay=	*(short *)q++;q++;
								SetPwmTab(p);
								break;
//______________________________________________________________________________________									
							case _PFM_TAND_POCKELS:
								p->burst[0].pockels.delay=	*(short *)q++;q++;
								p->burst[0].pockels.width=	*(short *)q++;q++;
								p->burst[1].pockels.delay=	*(short *)q++;q++;
								p->burst[1].pockels.width=	*(short *)q++;q++;
								PFM_pockels(p);	
								SetPwmTab(p);
								break;
//______________________________________________________________________________________
//______________________________________________________________________________________
//______________________________________________________________________________________
//______________________________________________________________________________________
//______________________________________________________________________________________
//______________________________________________________________________________________
//______________________________________________________________________________________
//______________________________________________________________________________________
//______________________________________________________________________________________
//______________________________________________________________________________________
							case _ID_PFMcom2SYS:
								if(_MODE(p,_CAN_2_COM))	{
									_io *io=_stdio(__dbug);
									for(n=0;n<rx.DLC;++n)
										__print("%c",rx.Data[n]);
									_stdio(io);
								}
								break;
//______________________________________________________________________________________
							case _ID_SYS2PFMcom:
								if(rx.DLC) {
									if(__can->arg.io == NULL) {
										__can->arg.io=_io_init(128,128);
										_proc_add(ParseCom,__can->arg.io,"ParseCAN-IO",0);
									}
									while(__can->arg.io->rx->size - _buffer_count(__can->arg.io->rx) < 8)
										_wait(2,_proc_loop);
									_buffer_push(__can->arg.io->rx,rx.Data,rx.DLC);
								} else {
									_proc_remove(ParseCom,__can->arg.io);
									__can->arg.io=_io_close(__can->arg.io);
								}
								break;
//__________________________________________________________________
							case _ID_SYS2PFM:
								switch(*(uint8_t *)q++) {
									case _PFM_status_req:
										PFM_status_send(p);
										break;
									case _PFM_IgbtTemp_req:
										CanReply("ccP",_PFM_IgbtTemp_ack,p->Temp);
										break;
									case _PFM_U_req:
										CanReply("cwP",_PFM_U_req,p->Burst->U);
										break;
									case _PFM_command:
										PFM_command(p,rx.Data[1]);
										Eack(NULL);
										break;
									case _PFM_set:				// >20  03 00 10 64 00 01				100u,409.6V 
										p->Burst->U = *(short *)q++;q++;
										p->Burst->Time=*(short *)q++;q++;
										p->Burst->Ereq=*q++;
										SetPwmTab(p);														
										break;
// ______ smafu za preverjanje LW protokola______________________________________________________________
									case _PFM_reset:			// >20 04 00 04 03 01						3x, 1m, 1000m
										p->Burst->Period=*(short *)q++;q++;
										p->Burst->N=*q++;
										p->Burst->Length=*q++*1000;
// _______ 
										if(p->Burst->N==0)
											p->Burst->N=1;
										if(p->Burst->Length==0)
											p->Burst->Length=3000;	
										p->Trigger.erpt = 0;
// _______
										if(_MODE(p,_LONG_INTERVAL)) {
											for(n=0; n<8; ++n)
												if((_ADCRates[n]+12)*(_MAX_BURST/_uS)/15 >  p->Burst->Length)
													break;
													
											ADC_RegularChannelConfig(ADC1, ADC_Channel_8,		1, n);
											ADC_RegularChannelConfig(ADC1, ADC_Channel_2,		2, n);
											ADC_RegularChannelConfig(ADC2, ADC_Channel_11,	1, n);
											ADC_RegularChannelConfig(ADC2, ADC_Channel_12,	2, n);
											p->ADCRate=((_ADCRates[n]+12)*_uS)/15;											
//________								
										} else {																																// NdYag long pulse burst, LW 4x2ms, 15/25 ms burst 
											if(p->Burst->Length > _MAX_BURST/_uS) {
												p->Trigger.erpt = p->Burst->N-1;																		// energy report after N pulses
												p->Burst->Period = (p->Burst->Length / p->Burst->N + 500)/1000;			// repetition rate = burst repetition, rounded to 1ms
												p->Burst->Length=(p->Burst->Period-1)*1000;													// burst length = repetition(us) - 1ms
												p->Burst->N=1;																											// treated as single pulse
											}
										}
//________
										SetPwmTab(p);
										break;
									case _PFM_simmer_set:
										if(rx.DLC==5) {
											p->Simmer.pw[0]=*(short *)q/50 + 7;
											++q;++q;
											p->Simmer.pw[1]=*(short *)q/50 + 7;
											SetSimmerRate(p,_SIMMER_LOW);	
											break;
										}
										else if(rx.DLC==7) {
											short	pw1,pw2;
											char	r1,r2;
											pw1 =*(short *)q++;q++;
											pw2 =*(short *)q++;q++;
											r1=*q++;
											r2=*q++;

											if(pw1 >= 120 && pw1 <= 500 &&
													pw2 >= 120 && pw2 <= 500 &&
														r1 >= 10 && r1 <= 100 &&
															r2 >= 10 && r2 <= 100) {
																p->Simmer.pw[0]=pw1*_uS/1000;
																p->Simmer.pw[1]=pw2*_uS/1000;
																p->Simmer.rate[0]=r1*_uS;
																p->Simmer.rate[1]=r2*_uS;
																SetSimmerRate(pfm,_SIMMER_LOW);
																break;
															}
										}
										_SET_ERROR(p,PFM_ERR_UB);
										break;
									case _PFM_RevNum_req:
										n=*(short *)q;
										CanReply("cwwP",_PFM_RevNum_req,SW_version,n);
										break;
									case _PFM_Ping:
										CanReply("cP",_PFM_Ping);
										break;
									case _PFM_Iap:
										while(1);
//___________________________________________________________________________________________________________								
//
// Pfm6 add..
//
//___________________________________________________________________________________________________________								
									case _PFM_POCKELS: 																					// 0x72, _PFM_POCKELS
										p->Burst->pockels.delay=	*(short *)q++;q++;							// 0.1uS delay , 0.1uS width	(short)
										p->Burst->pockels.width=	*(short *)q++;q++;
										p->Burst->Length=	*(short *)q++;q++;											// interval energije v uS			(short)
										p->Burst->N=*q;																						// stevilo pulzov v intervalu	(byte)
										PFM_pockels(p);																						// pockels timer setup
										SetPwmTab(p);																							// pulse buildup...
//										Eack(NULL);																								// reset integratorja energije
									break;

									case _PFM_SetHVmode:																				// 0x72, _PFM_SetHVmode 
									{																														// HV & mode configuration modif. by host
										char c[16];
										sprintf(c,"u %d",*(short *)q);
										if(*(short *)q == 0 || DecodeCom(c) == _PARSE_OK) {				// ignore zero voltage, left to default
											_CLEAR_ERROR(p,PFM_ERR_UB);															// clear previous error
											if(rx.DLC > 4) {																				// only if config. parameters are there...
												++q;++q;
												if(*q & 1)
													_CLEAR_MODE(p,_CHANNEL1_DISABLE);										// single2-disable2-single1-disable1
												else
													_SET_MODE(p,_CHANNEL1_DISABLE);
												if(*q & 2)
													_CLEAR_MODE(p,_CH1_SINGLE_TRIGGER);
												else
													_SET_MODE(p,_CH1_SINGLE_TRIGGER);
												if(*q & 4)
													_CLEAR_MODE(p,_CHANNEL2_DISABLE);
												else
													_SET_MODE(p,_CHANNEL2_DISABLE);
												if(*q & 8)
													_CLEAR_MODE(p,_CH2_SINGLE_TRIGGER);
												else
													_SET_MODE(p,_CH2_SINGLE_TRIGGER);
											}
										} else
											_SET_ERROR(p,PFM_ERR_UB);
									}
									break;

									case _PFM_CurrentLimit:																			// .10
									{
										int dac1,dac2;
										dac1=*(short *)q;
										++q;++q;
										dac2=*(short *)q;
										DAC_SetDualChannelData(DAC_Align_12b_R,dac1,dac2);
										DAC_DualSoftwareTriggerCmd(ENABLE);									
									}
										break;	
									case _PFM_HV_req:																						// .12
										CanReply("cwwwP",_PFM_HV_req,p->HV,p->HV2,p->Temp);
										break;	
									case _PFM_TRIGG:																						// .71
										_SET_EVENT(p,_TRIGGER);
										break;
								}
								break;
//______________________________________________________________________________________
								case _ID_ENRG2SYS: 																						// energometer-2-system message 
								{
									union {short w[4];} *e = (void *)q; 
									if((unsigned short)e->w[0]==0xD103)
										_DEBUG_(_DBG_ENM_MSG,"e1=%d.%dmJ, e2=%d.%dmJ",e->w[2]/10,e->w[2]%10,e->w[3]/10,e->w[3]%10);
								}
								break;
//______________________________________________________________________________________
								case _ID_SYS2ENRG: 																						// system-2-energometer message 
								{

								}
								break;
//______________________________________________________________________________________
								case _ID_SYS2EC:
								switch(*q++) {
									case _EC_status_req:
										CanReply("cwwE",_EC_status_req,0,0);
										break;
									case _EC_command:
										CanReply("cwwE",_EC_status_req,0,0);
										break;
									case _EC_RevNum_req:
										n=*(short *)q;
										CanReply("cwwE",_EC_RevNum_req,SW_version,n);
										break;
									case _EC_Ping:
										CanReply("cE",_EC_Ping);
										break;
								}
								break;
							}
						}
}
/*______________________________________________________________________________
  * @brief  CAN data transmission reply
  * @param format: message format string
  * @param ....  : list of parameters, acc. to format specifier
  * @retval : None
  */
void			CanReply(char *format, ...) {
static 
_io*			io=NULL;
CanTxMsg	tx={_ID_PFM2SYS,0,CAN_ID_STD,CAN_RTR_DATA,0,0,0,0,0,0,0,0,0};
union			{
						void *v;
						int i;
						char c[8];
					} u;
va_list		v;
char*			c;
int				i;				
					va_start(v, format);
					while(*format) { 
						u.i=va_arg(v, int);
						c=u.c;
						i=0;
						switch(*format++) {
							case '.': memcpy(&tx,u.v,sizeof(tx));	break;							
							case 'I':	memcpy(&io,c,sizeof(_io *));return;
							case 'P':	tx.StdId=_ID_PFM2SYS;				break;
							case 'E':	tx.StdId=_ID_EC2SYS;				break;
							case 'X':	tx.StdId=*c++;							break;
							case 'c':	i=sizeof(char);							break;
							case 'w':	i=sizeof(short);						break;
							case 'i':	i=sizeof(int);							break;
						}
						while(i--)			
							tx.Data[tx.DLC++]=*c++;
					}
					va_end(v);	
//______________________________________________________________________________________
					if(io) {
						io=_stdio(io);
						__print("%02X.<<.",tx.StdId);
						for(i=0; i<tx.DLC; ++i)
							__print("%c%02X",'.',tx.Data[i]);
						__print("\r\n>");	
						io=_stdio(io);						
					}
//					CAN_ITConfig(__CAN__, CAN_IT_TME, DISABLE);	
					_buffer_push(__can->tx,&tx,sizeof(CanTxMsg));
//					CAN_ITConfig(__CAN__, CAN_IT_TME, ENABLE);	
}
/*______________________________________________________________________________
	* @brief  : Computes energy across the Eenergy interval
	* @param 	: PFM object
  * @retval : None
  *
  */
uint64_t	eMac(_ADCDMA *,int, int);

int				Eack(PFM *p) {

static		uint64_t	e1=0,e2=0;
static		int				n=0;
					int				i=__min(_TIM.eint*_uS/_MAX_ADC_RATE,_MAX_BURST/_uS);

					if(p) {
#ifdef __F2__
						while(i--) {
							if(ADC1_buf[i].I > _I2AD(20.0))
								e1+=(short)(ADC1_buf[i].U) * (short)(ADC1_buf[i].I-_TIM.I1off);	
							if(ADC2_buf[i].I > _I2AD(20.0))
								e2+=(short)(ADC2_buf[i].U) * (short)(ADC2_buf[i].I-_TIM.I2off);							
						}			
#else
						e1+=eMac(ADC1_buf, i, _TIM.I1off);
						e2+=eMac(ADC2_buf, i, _TIM.I2off);
#endif
						if(n++ == p->Trigger.erpt) {
							e1/=(_kmJ*_uS/p->ADCRate);
							e2/=(_kmJ*_uS/p->ADCRate);
							if(_STATUS(p,PFM_STAT_SIMM1) && !_STATUS(p,PFM_STAT_SIMM2)) {
								CanReply("cicP",_PFM_E_ack,e1,0);	
								_DEBUG_(_DBG_PULSE_MSG,"E1=%d.%dJ",e1/1000,(e1%1000)/100);
							}
							
							if(!_STATUS(p,PFM_STAT_SIMM1) && _STATUS(p,PFM_STAT_SIMM2)) {
								CanReply("cicP",_PFM_E_ack,e2,0);					
								_DEBUG_(_DBG_PULSE_MSG,"E2=%d.%dJ",e2/1000,(e2%1000)/100);
							}
							
							if(_STATUS(p,PFM_STAT_SIMM1) && _STATUS(p,PFM_STAT_SIMM2)) {
								CanReply("cicP",_PFM_E_ack,e1+e2,0);		
								_DEBUG_(_DBG_PULSE_MSG,"E1=%d.%dJ, E2=%d.%dJ",e1/1000,(e1%1000)/100,e2/1000,(e2%1000)/100);
							}
							
							e1=e2=n=0;
							return(-1);
						}
					} else
						e1=e2=n=0;
					return(0);
}
/*______________________________________________________________________________
  * @brief  Composes & sends the PFM status message
  * @param  : active flash=k, active channel=_STATUS()
  * @retval : None
  *
  */
int				PFM_status_send(PFM *p) {
					if(p->Error & (PFM_ERR_SIMM1 | PFM_ERR_SIMM2)) {
						if(_STATUS(p,PFM_STAT_SIMM1 | PFM_STAT_SIMM2) == p->Simmer.active)	
							CanReply("cwiP",_PFM_status_req,
								(p->Status & ~(PFM_STAT_SIMM1 | PFM_STAT_SIMM2)) | p->Simmer.active,
								(p->Error & ~(PFM_ERR_SIMM1 | PFM_ERR_SIMM2)) | (p->Simmer.active & p->Error));
						else
							CanReply("cwiP",_PFM_status_req,
								(p->Status & ~(PFM_STAT_SIMM1 | PFM_STAT_SIMM2)) | p->Simmer.active,
								(p->Error & ~(PFM_ERR_SIMM1 | PFM_ERR_SIMM2)) | p->Simmer.active);
					}
					else
						CanReply("cwiP",_PFM_status_req,
							(p->Status & ~(PFM_STAT_SIMM1 | PFM_STAT_SIMM2)) | p->Simmer.active,
								p->Error);		
					
					return p->Simmer.active;
}
/*______________________________________________________________________________
  * @brief  Interprets the PFM command message
  * @param 	pfmcmd: PFM command word as defined in CAN protocol  ICD 
  * @retval : None
  *
  */					
void			PFM_command(PFM *p, int n) {
//________________________________________________________________________________
					while(_MODE(p,_PULSE_INPROC))																			// no change during pulse
						_wait(2,_proc_loop);
//________________________________________________________________________________
					if(p->Simmer.active != n) {																				// simmer status changed ???
						_TRIGGER1_OFF;																									// kill both triggers
						_TRIGGER2_OFF;
						_CLEAR_STATUS(p,PFM_STAT_SIMM1);																// clear status
						_CLEAR_STATUS(p,PFM_STAT_SIMM2);
						SetSimmerRate(p, _SIMMER_LOW); 																	// kill both simmers

						_wait(100,_proc_loop);																					// wait 100 msecs	
						p->Simmer.active=n & (PFM_STAT_SIMM1 | PFM_STAT_SIMM2);					// mask filter command

						if(!_MODE(p,_CHANNEL1_DISABLE)) {																// if not Erbium  single channel
int						u=p->HV/7;
							if(_MODE(p,_CH1_SINGLE_TRIGGER))															// single trigger config.. as from V1.11
								u=2*p->HV/7;
#ifdef __PFM8__
							u=_HV2AD(50);
#endif
							_TIM.I1off=ADC1_simmer.I;																			// get current sensor offset
							_TIM.U1off=ADC1_simmer.U;																			// check idle voltage
							if(abs(u - _AVG3*ADC1_simmer.U) > _HV2AD(50)) {								// HV +/- 50 range ???
								_SET_ERROR(p,PFM_ERR_LNG);																	// if not, PFM_STAT_UBHIGH error 
							}
						}

						if(!_MODE(p,_CHANNEL2_DISABLE)) {																// same for NdYAG channel
int						u=p->HV/7;
							if(_MODE(p,_CH2_SINGLE_TRIGGER))															// single trigger config.. as from V1.11
								u=2*p->HV/7;
#ifdef __PFM8__
							u=_HV2AD(50);
#endif
							_TIM.I2off=ADC2_simmer.I;
							_TIM.U2off=ADC2_simmer.U;
							if(abs(u - _AVG3*ADC2_simmer.U) > _HV2AD(50)) {
								_SET_ERROR(p,PFM_ERR_LNG);
							}
						}
					}
//__________________________________________________________________________________________________________
					if(p->Simmer.active && _MODE(p,_CHANNEL1_DISABLE))								// if single channel request
						_SET_STATUS(p, PFM_STAT_SIMM2);																	// modify status
					else if(p->Simmer.active && _MODE(p,_CHANNEL2_DISABLE))						// same for other channel
						_SET_STATUS(p, PFM_STAT_SIMM1);
					else
						_SET_STATUS(p, p->Simmer.active);																// else set status as requested
//__________________________________________________________________________________________________________
					if(p->Simmer.active && _MODE(p,_CH1_COMMON_TRIGGER))
						_TRIGGER1_ON;
					else if(p->Simmer.active && _MODE(p,_CH2_COMMON_TRIGGER))
						_TRIGGER2_ON;
					else {
						if(p->Simmer.active & PFM_STAT_SIMM1)
							_TRIGGER1_ON;
						if(p->Simmer.active & PFM_STAT_SIMM2)
							_TRIGGER2_ON;
					}
//__________________________________________________________________________________________________________
					SetSimmerRate(p,_SIMMER_LOW);																			// set low simmer
					p->Simmer.timeout=__time__+500;																		// set trigger burst timeout
//__________________________________________________________________________________________________________

					if(_STATUS(p,PFM_STAT_SIMM1 | PFM_STAT_SIMM2) == PFM_STAT_SIMM1)	// preklopi na aktivni objekt samo, ce je eksplicitno dolocen
						p->Burst = &p->burst[0];																				// ce je simm. 0 oz. 3 se uporabi burst 1 na __SetPwmTab
					if(_STATUS(p,PFM_STAT_SIMM1 | PFM_STAT_SIMM2) == PFM_STAT_SIMM2)
						p->Burst = &p->burst[1];
					p->Trigger.time=0;																								// reset trigger process		
}
/*______________________________________________________________________________
  * @brief  Pockels cell driver setup
  * @param 	q == can buffer (short delay, short pw, short n-pulses
	* if q==0 >> call from interrupt
  * @retval : None
  *
  */
int				PFM_pockels(PFM *p) {

					TIM_TimeBaseInitTypeDef		TIM_TimeBaseStructure;
					TIM_OCInitTypeDef					TIM_OCInitStructure;
					GPIO_InitTypeDef					GPIO_InitStructure;
					
					TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
					TIM_OCStructInit(&TIM_OCInitStructure);
					GPIO_StructInit(&GPIO_InitStructure);
	
					TIM_DeInit(TIM4);																					// reset TIM4
					RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
					
					GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;							// to prevent glitch, back to GPIO mode first
					GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
					GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;						
					GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;	
					GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
					GPIO_SetBits(GPIOD,GPIO_Pin_12);													// preset
					GPIO_Init(GPIOD, &GPIO_InitStructure);										// activate GPIO mode
	
					GPIO_PinAFConfig(GPIOD, GPIO_PinSource12, GPIO_AF_TIM4);	// this one, causes glitch !!!!
					GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;							// then, change config
					GPIO_Init(GPIOD, &GPIO_InitStructure);
									
					TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
					TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
					TIM_OCInitStructure.TIM_Pulse=1;
					TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
					TIM_OC1Init(TIM4, &TIM_OCInitStructure);
					
					TIM_TimeBaseStructure.TIM_Period = 1000;
					TIM_TimeBaseStructure.TIM_Prescaler = _uS/10-1;
					TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
					TIM_TimeBaseInit(TIM4,&TIM_TimeBaseStructure);
					
					TIM_Cmd(TIM4, DISABLE);
					return 0;
}
/**
* @}
*/ 
