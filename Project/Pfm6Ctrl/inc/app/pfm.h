/* Includes ------------------------------------------------------------------*/
#ifndef WIN32
#include				"cpu.h"
#endif
#include				<string.h>
#include				<stdio.h>
#include				<math.h>
#include				"io.h"
#include 				"proc.h"
#include				"ff.h"
#include				"diskio.h"

#include				"usbh_core.h"
#include				"usbh_msc_usr.h"
#include				"usbd_usr.h"
#include				"usbd_desc.h"
#include				"usbd_msc_core.h"
#include				"usbd_cdc_core.h"
#include				"usbh_msc_core.h"

#include				"usb_conf.h"
#include				"usbh_core.h"
//________SW version string_____________________________	

// TIM1_polarity TIM_OCPolarity_High);

#if		defined		(__PFM6__)
	#define 				SW_version	215		
#elif 		defined		(__PFM8__)
	#define 				SW_version	315		
#else
*** error, define HW platform
#endif
//________global platform dependencies	________________			
#if		defined (__F2__)
	#define					_uS					60
	#define 				ADC_Ts			ADC_SampleTime_3Cycles
	#define					_MAX_BURST	(10*_mS)
#elif	defined (__F4__)
	#define					_uS					60
	#define 				ADC_Ts			ADC_SampleTime_3Cycles
	#define					_MAX_BURST	(10*_mS)
#elif	defined (__F7__)
	#define					_uS					108
	#define 				ADC_Ts			ADC_SampleTime_15Cycles
	#define					_MAX_BURST	(13*_mS)
#else
*** error, define CPU
#endif
//______________________________________________________	
#define					__CAN__				CAN2
#define					__FILT_BASE__	14
//______________________________________________________
#define					_mS						(1000*_uS)
#define					_PWM_RATE_HI	(10*_uS)
#define					_MAX_PWM_RATE	((_PWM_RATE_HI*98)/100)
#define					_MAX_ADC_RATE	(1*_uS)
#define					_FAN_PWM_RATE	(50*_uS)	 

#define					_Rdiv(a,b)		((a)/(a+b))
#define					_Rpar(a,b)		((a)*(b)/(a+b))
#define					_UREF					3.3
#define 				_Ts						1e-6
			          
#if		defined	(__PFM6__)
	#define				_AD2HV(a)		((int)(((a)*_UREF)/4096.0/_AVG3/_Rdiv(7.5e3,2e6)+0.5))
	#define				_HV2AD(a)		((int)(((a)*4096.0*_AVG3*_Rdiv(7.5e3,2e6))/_UREF+0.5))
	#define				_Ifullsc		((int)1200)
	#define				_kmJ				((int)(_V2AD(1000,2000,7.5)*_I2AD(1000)/1000))
#elif	defined		(__PFM8__)
	#define				_AD2HV(a)		((int)(((a)*_UREF)/4096.0/_AVG3/_Rdiv(6.2e3,2e6)+0.5))
	#define				_HV2AD(a)		((int)(((a)*4096.0*_AVG3*_Rdiv(6.2e3,2e6))/_UREF+0.5))
	#define				_Ifullsc		((int)1270)
	#define				_kmJ				((int)(_V2AD(1000,2000,6.2)*_I2AD(1000)/1000))
#else
*** error, define HW platform
#endif

#define					_AD2V(val,rh,rl)	((float)((val)*(rl+rh)/rl*3.3/4096.0))	
#define					_AD2Vn(val,rh,rl)	((float)(((val)-4096)*(rl+rh)/rl*3.3/4096.0 + 3.3))
#define					_V2AD(val,rh,rl)	((int)((val)*4096.0/3.3*rl/(rh+rl)+0.5))
#define					_Vn2AD(val,rh,rl)	((int)(4096+((val)-3.3)*4096.0/3.3*rl/(rh+rl)+0.5))
	
#define					_I2AD(a)					((int)(((a)*4096 + _Ifullsc/2)/_Ifullsc))
#define					_AD2I(a)					((int)(((a)*_Ifullsc + 2048)/4096))

//#define					kVf								(3.3/4096.0*2000.0/7.5)					
//#define					kIf								(3.3/4096.0/2.9999/0.001)
//#define 				kmJ							(int)(0.001/kVf/kIf/_Ts+0.5)
	
#define					__charger6				__i2c1

void						_led(int, int);

#define					_RED1(a)			do if(__time__ > 10000) _led(0,a); while(0)
#define					_GREEN1(a)		do if(__time__ > 10000) _led(1,a); while(0)
#define					_YELLOW1(a)		do if(__time__ > 10000) _led(2,a); while(0)
#define					_BLUE1(a)			do if(__time__ > 10000) _led(3,a); while(0)
#define					_ORANGE1(a)		do if(__time__ > 10000) _led(4,a); while(0)
#define					_RED2(a)			do if(__time__ > 10000) _led(5,a); while(0)
#define					_GREEN2(a)		do if(__time__ > 10000) _led(6,a); while(0)
#define					_YELLOW2(a)		do if(__time__ > 10000) _led(7,a); while(0)
#define					_BLUE2(a)			do if(__time__ > 10000) _led(8,a); while(0)
#define					_ORANGE2(a)		do if(__time__ > 10000) _led(9,a); while(0)   
//________________________________________________________________________
typedef					enum 					{_SIMMER_LOW, _SIMMER_HIGH} SimmerType; 
typedef					enum					{false=0, true} bool;
//________________________________________________________________________
typedef					enum
{								_TRIGGER,
								_PULSE_ENABLED,
								_PULSE_FINISHED,
								_FAN_TACHO,
								_REBOOT=30
} 							_event;

typedef					 enum
{								_DBG_CAN_TX,							//0
								_DBG_CAN_RX,							//1
								_DBG_ERR_MSG,							//2
								_DBG_PULSE_MSG,						//3
								_DBG_ENM_MSG,							//4
								_DBG_SYS_MSG,							//5
								_DBG_I2C_TX,							//6
								_DBG_I2C_RX,							//7
} 							_debug;

typedef					enum
{								_XLAP_SINGLE,							//0
								_XLAP_DOUBLE,							//1
								_XLAP_QUAD,								//2
								_SIMMER_KEEP,							//3
								_SIMULATOR,								//4
								_PULSE_INPROC,						//5
								_LONG_INTERVAL,						//6
								_TRIGGER_PERIODIC,				//7
								__DUMMY2,									//8
								_U_LOOP,									//9
								_P_LOOP,									//10
								_CHANNEL1_DISABLE,				//11
								_CHANNEL2_DISABLE,				//12
								_CH1_SINGLE_TRIGGER,			//13
								_CH2_SINGLE_TRIGGER,			//14
								_CH1_COMMON_TRIGGER,			//15
								_CH2_COMMON_TRIGGER,			//16
								_ALTERNATE_TRIGGER,				//17
								_F2V,											//18
								_ENM_NOTIFY,							//19
								__TEST__									=29,
								_CAN_2_COM								//30
} 							mode;

#define 				PFM_STAT_SIMM1						0x0001
#define 				PFM_STAT_SIMM2						0x0002
#define 				PFM_STAT_DSCHG						0x0004
#define 				PFM_STAT_UBHIGH						0x0008
#define 				PFM_STAT_PSRDY						0x0100
			                                    
#define 				PFM_ERR_SIMM1							0x0001					// simmer 1 error
#define 				PFM_ERR_SIMM2							0x0002					// simmer 2 error
#define 				PFM_ERR_UB  							0x0004					// can message 0x73, _PFM_SetHVmode error, charger not responding
#define 				PFM_ERR_LNG 							0x0008					// flash tube idle voltage error
#define 				PFM_ERR_TEMP							0x0010					// IGBT overheat
#define 				PFM_ERR_DRVERR						0x0020					// dasaturation protection active
#define 				PFM_SCRFIRED  						0x0040					// IGBT not ready, pfm8
#define 				PFM_ERR_PULSEENABLE				0x0080					// crowbar
#define 				PFM_ERR_PSRDYN						0x0100					// pwm threshold error
#define 				PFM_ERR_48V  							0x0200					// 20V igbt supply error
#define 				PFM_ERR_15V 							0x0400					// -5V igbt supply error

#define					PFM_ADCWDG_ERR						0x1000					// adc watchdog fired
#define					PFM_FAN_ERR								0x2000					// igbt fan error
#define					PFM_HV2_ERR								0x4000					// center cap voltaghe out of range
#define					PFM_I2C_ERR								0x8000					// i2c comm. not responding
#define					PFM_ERR_VCAP1							0x10000					//
#define					PFM_ERR_VCAP2							0x20000					//

extern const char *_errStr[];

#if		defined		(__F2__) || defined		(__F4__)
	#define					_BIT(p,n)					(bool)(*(char *)(0x22000000 + ((int)(&p) - 0x20000000) * 32 + 4*n))
	#define					_SET_BIT(p,n)			(*(char *)(0x22000000 + ((int)(&p) - 0x20000000) * 32 + 4*n)) = 1
	#define					_CLEAR_BIT(p,n)		(*(char *)(0x22000000 + ((int)(&p) - 0x20000000) * 32 + 4*n)) = 0
#elif defined		(__F7__)
	#define					_BIT(p,n)					((p) & (1<<(n)))
	#define					_SET_BIT(p,a)			do {					\
										int primask=__get_PRIMASK();	\
										__disable_irq();							\
										(p) |= (1<<(a));							\
										__set_PRIMASK(primask);				\
									} while(0)
	#define					_CLEAR_BIT(p,a)		do {					\
										int primask=__get_PRIMASK();	\
										__disable_irq();							\
										(p) &= ~(1<<(a));							\
										__set_PRIMASK(primask);				\
									} while(0)
#else
	*** error, undefined HW
#endif					
	
#define					_MODE(p,a)					_BIT(p->mode,a)
#define					_SET_MODE(p,a)			_SET_BIT(p->mode,a)
#define					_CLEAR_MODE(p,a)		_CLEAR_BIT(p->mode,a)

#define					_EVENT(p,a)					_BIT(p->events,a)
#define					_SET_EVENT(p,a)			_SET_BIT(p->events,a)
#define					_CLEAR_EVENT(p,a)		_CLEAR_BIT(p->events,a)

#define					_DBG(p,a)						_BIT(p->debug,a)
#define					_SET_DBG(p,a)				_SET_BIT(p->debug,a)
#define					_CLEAR_DBG(p,a)			_CLEAR_BIT(p->debug,a)

#define					_STATUS(p,a)				(p->Status & (a))
#define					_SET_STATUS(p,a)		(p->Status |= (a))
#define					_CLEAR_STATUS(p,a)	(p->Status &= ~(a))

#define					_ERROR(p,a)					(p->Error & (a))
#define					_CLEAR_ERROR(p,a)	do {																	\
									if(p->Error & (a)) {																	\
										p->Error &= ~(a);																		\
									}																											\
								} while(0)

#define					_SET_ERROR(p,a)	do {																		\
									if(!(p->Errmask & (a)) && !(p->Error & (a))) {				\
										if(a & _CRITICAL_ERR_MASK)													\
											_DISABLE_PWM_OUT();																\
										p->Error |= a;																			\
									}																											\
								} while(0)	
//________________________________________________________________________
#define 				_AVG3									1
#define					_MAX_QSHAPE						8
#define					_MAX_USER_SHAPE				1024
extern int			_ADCRates[];	
								
#if	defined (__PFM6__)
typedef struct	{	unsigned short			IgbtT[2],HV2,HV,Up20,Um5;											} _ADC3DMA;
#endif
#if	defined (__PFM8__)
typedef struct	{	unsigned short			IgbtT[4],HV2,HV,Up12,Up5,Up3,VCAP1,VCAP2;			} _ADC3DMA;						
#endif
					

typedef struct	{	unsigned short			U,I;								} _ADCDMA;
typedef struct	{	unsigned short			DAC2,DAC1;					} _DACDMA;
typedef struct	{	unsigned short			addr,speed,ntx,nrx;
									unsigned char				txbuf[4],rxbuf[4];	} _i2c;
typedef struct	{					 short			q0,q1,q2,q3,qref;		}	_QSHAPE;
typedef struct	{					 short			T,U;								}	_USER_SHAPE;
extern					_QSHAPE 		qshape[_MAX_QSHAPE];			
extern					_USER_SHAPE ushape[_MAX_USER_SHAPE];
//________________________________________________________________________
extern  struct _TIM {																			// realtime structure, used with timer stack
	struct _TIM_DMA {
		unsigned short			n,T;
	} pwch1[_MAX_BURST/_PWM_RATE_HI],
		pwch2[_MAX_BURST/_PWM_RATE_HI],												// output tables
		*p1,*p2;																							// pointers to output tables
	int		
		U1off,U2off,																					// flash voltage, idle status
		I1off,I2off,																					// flash current, idle status
		eint,eint1,eint2,																			// adc dma length, usec
		m1,m2,																								// timer repetition rate counter index, DMA table
		active,																								// active channel
		cref1,cref2,																					// current loop reference (after 200usec)
		ci1,ci2,																							// current loop gain
		Hvref,Caps,Icaps;																			// test mode parameters
} _TIM;
typedef struct _TIM_DMA _TIM_DMA; 
//________________________________________________________________________
int 						readI2C(_i2c *,char *, int),
								writeI2C(_i2c *,char *, int),
								str2hex(char **,int n);
//________________________________________________________________________
#define					_SIGN_PAGE						FLASH_Sector_1
#define					_FLASH_TOP						0x08008000
#define					_BOOT_SECTOR					0x08000000
		                                  
#define 				_FW_START							((int *)(_FLASH_TOP-16))
#define 				_FW_CRC								((int *)(_FLASH_TOP-20))
#define 				_FW_SIZE							((int *)(_FLASH_TOP-24))
#define 				_SIGN_CRC							((int *)(_FLASH_TOP-28))
#define					_FLASH_BLANK					((int)-1)
//________________________________________________________________________            
extern int			(*USBH_App)(int);
int							USBH_Iap(int);
//________________________________________________________________________
#define					_ID_SYS2PFM						0x20
#define					_ID_SYS2EC						0x23
#define					_ID_PFM2SYS						0x40
#define					_ID_EC2SYS						0x43
		                                  
#define					_ID_SYS_TRIGG					0x1A
#define					_ID_SYS2PFMcom				0xB0
#define					_ID_PFMcom2SYS				0xB1
		                                  
#define					_ID_IAP_GO						0xA0
#define					_ID_IAP_ERASE					0xA1
#define					_ID_IAP_ADDRESS				0xA2
#define					_ID_IAP_DWORD					0xA3
#define					_ID_IAP_ACK						0xA4
#define					_ID_IAP_SIGN					0xA5
#define					_ID_IAP_STRING				0xA6
#define					_ID_IAP_PING					0xA7
//________________________________________________________________________
#define					_PFM_status_req				0x00
#define					_PFM_command					0x02
#define					_PFM_set							0x03
#define					_PFM_reset						0x04
#define					_PFM_IgbtTemp_req			0x05
#define					_PFM_simmer_set				0x06
										                  
#define					_PFM_IgbtTemp_ack			0x03
#define					_PFM_U_req						0x08
#define					_PFM_E_ack						0x07
										                  
#define					_PFM_RevNum_req				0x0A
#define					_PFM_Ping							0x0C
#define					_PFM_Iap							0xAA
//________________________________________________________________________
// add.	Pfm6
//
#define					_PFM_CurrentLimit			0x10
#define					_PFM_SetPwm						0x11
#define					_PFM_HV_req						0x12
#define					_PFM_SetLoop					0x13
		                                  
#define 				_PFM_TRIGG						0x71
#define					_PFM_SetHVmode				0x72
#define 				_PFM_POCKELS					0x73

#define 				_PFM_TAND_CH0					0x100
#define 				_PFM_TAND_CH1					0x101
#define 				_PFM_TAND_DLY					0x102

#define					_ID_SYS2ENRG					0x1f
#define					_ID_ENRG2SYS					0x3f
//________________________________________________________________________
#define					_EC_status_req				0x00
#define					_EC_command						0x02
#define					_EC_RevNum_req				0x0A
#define					_EC_Ping							0x0C
//________________________________________________________________________

#define	 				_Esc									0x1b
					                            
#define					_CtrlA								0x01
#define					_CtrlB								0x02
#define					_CtrlC								0x03
#define					_CtrlD								0x04
#define					_CtrlE								0x05
#define					_CtrlF								0x06
					                            
#define					_CtrlI								0x09
#define					_CtrlO								0x0f
#define					_CtrlV								0x16
#define					_CtrlZ								0x1a
#define					_CtrlY								0x19
#define					_CtrlT								0x14
					                            
#define	 				_f1										0x001B4F50
#define	 				_f2										0x001B4F51
#define	 				_f3										0x001B4F52
#define	 				_f4										0x001B4F53
#define	 				_f5										0x001B4F54
#define	 				_f6										0x001B4F55
#define	 				_f7										0x001B4F56
#define	 				_f8										0x001B4F57
#define	 				_f9										0x001B4F58
#define	 				_f10									0x001B4F59
#define	 				_f11									0x001B4F5A
#define	 				_f12									0x001B4F5B
					                            
#define	 				_F1										0x5B31317E
#define	 				_F2 									0x5B31327E
#define	 				_F3										0x5B31337E
#define	 				_F4										0x5B31347E
#define	 				_F5										0x5B31357E
#define	 				_F6										0x5B31377E
#define	 				_F7										0x5B31387E
#define	 				_F8										0x5B31397E
#define	 				_F9										0x5B32307E
#define	 				_F10									0x5B32317E
#define	 				_F11									0x5B32337E
#define	 				_F12									0x5B32347E
#define	 				_Home									0x1B5B317E
#define	 				_End									0x1B5B347E
#define	 				_Insert								0x1B5B327E
#define	 				_PageUp								0x1B5B357E
#define	 				_Delete								0x1B5B337E
#define	 				_PageDown							0x1B5B367E
#define	 				_Up										0x001B5B41
#define	 				_Left									0x001B5B44
#define	 				_Down									0x001B5B42
#define	 				_Right								0x001B5B43

//________________________________________________________________________
typedef 				struct {
short						N,										// burst pulse count 
								Length,					      // burst length, us
								U,										// pulse voltage
								Time,
								Period;								// _PFM_reset command parameters, ms
char						Ereq;		              
short						Pmax,			            
								Pdelay,								// burst interval	pwm
								Delay,								// -"- delay
								max[2];								// burst time current limit
mode						Mode;									// burst time mode
} burst;
//________________________________________________________________________
typedef 				struct {
short						pw[2],								// simmer pwm, izracunan iz _PFM_simmer_set
								rate[2],							// simmer pwm rate
								max,									// simmer current limits
								active;
mode						mode;									// simmer time mode
unsigned int		timeout;
} simmer;
//________________________________________________________________________
typedef 				struct {
unsigned int		erpt,									// send energy on every ....
								counter,							// counter for multiple  triggers sequence	
								count,								// number of multiple  triggers
								time,									// next trigger
								timeout;							// trigger timeout, _F2V mode 
} trigger;
//________________________________________________________________________
typedef 				struct {
burst						*Burst,burst[2];
simmer					Simmer;
trigger					Trigger;
int							Error,	
								Errmask;
short						Status,	
								HVref,								// req. reference HV
								HV,										// Cap1+Cap2	ADC value x _AVG3
								HV2,									// Cap1			ADC value x _AVG3								
								Temp,									// Igbt temp,	degrees
#if	defined (__PFM6__)
								Up20,				
								Um5,				
#elif	defined (__PFM8__)					
								Up12,				
								Up5,				
								Up3,				
#endif
								ADCRate;
volatile unsigned int		
								events,
								debug,
								mode,
								fan_rate,
								boot_timeout;
struct {
	short					delay,
								width,
								trigger;
} Pockels;
FATFS						*fatfs;
} PFM;				  
//________________________________________________________________________
extern					PFM										*pfm;
														
extern 					_ADCDMA								ADC1_buf[], ADC2_buf[],ADC1_simmer,ADC2_simmer;
extern 					_ADC3DMA							ADC3_buf[];
				        
void						App_Init(void),
				        
								SetSimmerRate(PFM *, SimmerType),
								SetPwmTab(PFM *),
								Trigger(PFM *),
								TriggerADC(PFM *),
								CanReply(char *, ...);
void						PFM_debug(PFM *);
				        
typedef	enum 		{T_MIN=0,TH1,TH2,TL1,TL2} temp_ch;
int							IgbtTemp(temp_ch);


int							Eack(PFM *),
								PFM_pockels(PFM *),
								PFM_status_send(PFM *);
								
void 						USBD_Storage_Init(void),
								PFM_command(PFM *, int);
				        
#define					_VOUT_MODE						0x20
#define					_VOUT									0x21
#define					_READ_VIN							0x88
#define					_READ_IIN							0x89
#define					_READ_VOUT						0x8B
#define					_READ_PIN							0x97
#define					_VIN_ON								0x35
#define					_VIN_OFF							0x36
#define					_PFC_ON								0x8401
#define					_PFC_OFF							0x0401
#define					_READ_RATE						0xFE3A
#define					_STATUS_WORD 					0x79

#define 				_SHPMOD_OFF			0
#define 				_SHPMOD_MAIN		1
#define 				_SHPMOD_CAL			2
#define 				_SHPMOD_QSWCH		4

#define					_minmax(x,x1,x2,y1,y2) 	__min(__max(((y2-y1)*(x-x1))/(x2-x1)+y1,y1),y2)


int							FLASH_Program(uint32_t, uint32_t); 
int							FLASH_Erase(uint32_t);
	
void 						ParseCanRx(_proc *),
								ParseCanTx(_proc *),
								ParseCom(_proc *),
								ParseFile(FIL *f),
								ProcessingEvents(_proc *),
								ProcessingCharger(_proc *),
								ProcessingStatus(_proc *),
								Lightshow(_proc *);
				        
void						SysTick_init(void),
								Watchdog_init(int),
								Watchdog(void),
								WWDG_init(void),
										
								USBD_Vcp_Init(void),
								Initialize_ADC(void),
								Initialize_DAC(void),
								Reset_I2C(_i2c *),
								Initialize_LED(char *[], int),
								Initialize_TIM(void),
								Initialize_NVIC(void);
										
_io 						*Initialize_USART1(int),
								*Initialize_USART3(int),
								*Initialize_CAN(int);
void						canFilterConfig(int, int);

void	 					*Initialize_F2V(_proc *);
_i2c*						Initialize_I2C(int, int);
				
extern int			fanPmin,fanPmax,fanTL,fanTH;

extern _io			*__com1,
								*__com3,
								*__dbug,
								*__can;
								
extern _i2c			*__i2c1,
								*__i2c2;
								
						
char						*cgets(int, int);
int							DecodeCom(char *),
								DecodeFs(char *),
								Tandem();
int							Escape(void);


int							ScopeDumpBinary(_ADCDMA *, int);
							
int							getHEX(char *, int);
int							hex2asc(int);
int							strscan(char *,char *[],int),
								numscan(char *,char *[],int);
int							hex2asc(int);
int							sLoad(char *);
int							iDump(int *,int);
int							sDump(char *,int);
int							hex2asc(int);
int							asc2hex(int);

#ifndef	__max				
#define __max(a,b)  (((a) > (b)) ? (a) : (b))	
#endif
#ifndef	__min				
#define __min(a,b)  (((a) < (b)) ? (a) : (b))	
#endif

int			__fit(int,const int[],const int[]);
float		__lin2f(short);
short		__f2lin(float, short);

int			batch(char *);	        
void		CAN_console(void);

extern	uint32_t	__Vectors[],
									__heap_base[],
									__heap_limit[],
									__initial_sp[];

extern	int				_PWM_RATE_LO;

void		SectorQuery(void);
int 		Defragment(int);
int			SetChargerVoltage(int);

#define _FAULT_BIT				GPIO_Pin_8
#define _FAULT_PORT				GPIOE
#define _FAULT_INT_port 	EXTI_PortSourceGPIOE
#define _FAULT_INT_pin		EXTI_PinSource8
#define _FAULT_INT_line		EXTI_Line8
#define _IGBT_READY_BIT 	GPIO_Pin_2
#define _IGBT_READY_PORT 	GPIOE
#define _IGBT_RESET_BIT 	GPIO_Pin_3
#define _IGBT_RESET_PORT 	GPIOE

#define _USB_SENSE_BIT 		GPIO_Pin_10
#define _USB_SENSE_PORT	 	GPIOD
//_________________________________________________________________________________
#ifndef __DISC4_
#define _USB_PIN_BIT 			GPIO_Pin_8
#define _USB_PIN_PORT 		GPIOD
#define _USB_DIR_BIT 			GPIO_Pin_9
#define _USB_DIR_PORT	 		GPIOD
#endif
//_________________________________________________________________________________
#if defined (__PFM6__)
#define _TRIGGER1_BIT GPIO_Pin_12
#define _TRIGGER1_PORT GPIOD
#define _TRIGGER2_BIT GPIO_Pin_13
#define _TRIGGER2_PORT GPIOD

#define _CWBAR_BIT 				GPIO_Pin_14
#define _CWBAR_PORT 			GPIOD
#define _CWBAR_INT_port 	EXTI_PortSourceGPIOD
#define _CWBAR_INT_pin		EXTI_PinSource14
#define _CWBAR_INT_line		EXTI_Line14

#define	_ERROR_OW_BIT			GPIO_Pin_13
#define	_ERROR_OW_PORT		GPIOB
#define	_F2V_OW_BIT				GPIO_Pin_5
#define	_F2V_OW_AF				GPIO_PinSource5
#define	_F2V_OW_PORT			GPIOB
//_________________________________________________________________________________
#elif defined (__PFM8__)
#define _TRIGGER1_BIT GPIO_Pin_4
#define _TRIGGER1_PORT GPIOE
#define _TRIGGER2_BIT GPIO_Pin_5
#define _TRIGGER2_PORT GPIOE

#define _CWBAR_BIT							GPIO_Pin_6
#define _CWBAR_PORT							GPIOE
#define _CWBAR_INT_port					EXTI_PortSourceGPIOE
#define _CWBAR_INT_pin					EXTI_PinSource6
#define _CWBAR_INT_line					EXTI_Line6

#define	_ERROR_OW_BIT						GPIO_Pin_13
#define	_ERROR_OW_PORT					GPIOB
#define	_F2V_OW_BIT							GPIO_Pin_5
#define	_F2V_OW_AF							GPIO_PinSource5
#define	_F2V_OW_PORT						GPIOB
//_________________________________________________________________________________
#define	_NRST_DISABLE_BIT 			GPIO_Pin_13
#define _NRST_DISABLE_PORT			GPIOF
#define	_BOOT_ENABLE_BIT				GPIO_Pin_14
#define _BOOT_ENABLE_PORT				GPIOF			        
//_________________________________________________________________________________
#else
*** error, define platform
#endif

#if defined (__DISC7__)
#define _VBUS_BIT GPIO_Pin_5
#define _VBUS_PORT GPIOD
#endif

#if defined (__DISC4__)
#define _VBUS_BIT GPIO_Pin_0
#define _VBUS_PORT GPIOC
#endif

#define _IGBT_READY		(GPIO_ReadInputDataBit(_IGBT_READY_PORT,_IGBT_READY_BIT)== Bit_SET)				        		        
#define	_PFM_CWBAR		(GPIO_ReadInputDataBit(_CWBAR_PORT, _CWBAR_BIT)== Bit_RESET)

#define	_IGBT_RESET		{ int i; 																							\
												for(i=0; i<10; ++i)		 															\
													GPIO_ResetBits(_IGBT_RESET_PORT,_IGBT_RESET_BIT); \
											}																											\
											GPIO_SetBits(_IGBT_RESET_PORT,_IGBT_RESET_BIT);				

#define _TRIGGER1			(!GPIO_ReadOutputDataBit(_TRIGGER1_PORT,_TRIGGER1_BIT))				        
#define _TRIGGER1_ON	do {															\
							if(!_TRIGGER1)														\
								_DEBUG_(_DBG_SYS_MSG,"trigger 1 enabled");				\
								GPIO_ResetBits(_TRIGGER1_PORT,_TRIGGER1_BIT);			\
							} while(0)
#define _TRIGGER1_OFF	do {															\
							if(_TRIGGER1)															\
								_DEBUG_(_DBG_SYS_MSG,"trigger 1 disabled");				\
								GPIO_SetBits(_TRIGGER1_PORT,_TRIGGER1_BIT);	  		\
							} while(0)

#define _TRIGGER2			(!GPIO_ReadOutputDataBit(_TRIGGER2_PORT,_TRIGGER2_BIT))			        
#define _TRIGGER2_ON	do {															\
							if(!_TRIGGER2)														\
								_DEBUG_(_DBG_SYS_MSG,"trigger 2 enabled");				\
								GPIO_ResetBits(_TRIGGER2_PORT,_TRIGGER2_BIT);			\
							} while(0)
#define _TRIGGER2_OFF	do {															\
							if(_TRIGGER2)															\
								_DEBUG_(_DBG_SYS_MSG,"trigger 2 disabled");				\
								GPIO_SetBits(_TRIGGER2_PORT,_TRIGGER2_BIT);		  	\
							} while(0)
							      
#define	_CRITICAL_ERR_MASK		(PFM_ERR_DRVERR | PFM_ERR_PULSEENABLE | PFM_ADCWDG_ERR | PFM_ERR_PSRDYN | PFM_ERR_LNG | PFM_HV2_ERR)
#define	_PFM_CWBAR_STAT				PFM_ERR_PULSEENABLE
				        
enum	err_parse	{
								_PARSE_OK=0,
								_PARSE_ERR_SYNTAX,
								_PARSE_ERR_ILLEGAL,
								_PARSE_ERR_MISSING,
								_PARSE_ERR_NORESP,
								_PARSE_ERR_OPENFILE,
								_PARSE_ERR_MEM
								};

static __inline void dbg_2(int n, char *s) {
			if(pfm->debug & (1<<(n))) {
				_io *io=_stdio(__dbug);
				__print(":%04d %s\r\n>",__time__ % 10000, s);
				_stdio(io);
			}
}

static __inline void dbg_3(int n, char *s, int arg1) {
			if(pfm->debug & (1<<(n))) {
				_io *io=_stdio(__dbug);
				__print(":%04d ",__time__ % 10000);
				__print((s),(arg1));
				__print("\r\n>");
				_stdio(io);
			}
}

static __inline void dbg_4(int n,char *s, int arg1, int arg2) {
			if(pfm->debug & (1<<(n))) {
				_io *io=_stdio(__dbug);
				__print(":%04d ",__time__ % 10000);
				__print((s),(arg1),(arg2));
				__print("\r\n>");
				_stdio(io);
			}
}

static __inline void dbg_5(int n,char *s, int arg1, int arg2, int arg3) {
			if(pfm->debug & (1<<(n))) {
				_io *io=_stdio(__dbug);
				__print(":%04d ",__time__ % 10000);
				__print((s),(arg1),(arg2),(arg3));
				__print("\r\n>");
				_stdio(io);
			}
}

static __inline void dbg_6(int n,char *s, int arg1, int arg2, int arg3, int arg4) {
			if(pfm->debug & (1<<(n))) {
				_io *io=_stdio(__dbug);
				__print(":%04d ",__time__ % 10000);
				__print((s),(arg1),(arg2),(arg3),(arg4));
				__print("\r\n>");
				_stdio(io);
			}
}

#define	GET_MAC(_1,_2,_3,_4,_5,_6,NAME,...) NAME
#define	_DEBUG_(...) GET_MAC(__VA_ARGS__,dbg_6,dbg_5,dbg_4,dbg_3,dbg_2)(__VA_ARGS__)

#if defined __PFM8__
	#define _PWM_LOW			TIM_OCPolarity_High
	#define _PWM_HIGH			TIM_OCPolarity_Low
	#define _PWM_3STATE		GPIO_PuPd_UP
#else
	#define _PWM_LOW			TIM_OCPolarity_Low
	#define _PWM_HIGH			TIM_OCPolarity_High
	#define _PWM_3STATE		GPIO_PuPd_DOWN
#endif

static __inline void _TIMERS_HALT(void) {
#if defined __PFM8__
			TIM_Cmd(TIM4,DISABLE);			
			TIM_Cmd(TIM2,DISABLE);
#endif
			TIM_Cmd(TIM8,DISABLE);	
			TIM_Cmd(TIM1,DISABLE);
}

static __inline void _TIMERS_PRELOAD_ON(void) {
			TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Enable);
			TIM_OC2PreloadConfig(TIM1, TIM_OCPreload_Enable);
			TIM_OC3PreloadConfig(TIM1, TIM_OCPreload_Enable);
			TIM_OC4PreloadConfig(TIM1, TIM_OCPreload_Enable);
			TIM_OC1PreloadConfig(TIM8, TIM_OCPreload_Enable);
			TIM_OC2PreloadConfig(TIM8, TIM_OCPreload_Enable);
			TIM_OC3PreloadConfig(TIM8, TIM_OCPreload_Enable);
			TIM_OC4PreloadConfig(TIM8, TIM_OCPreload_Enable);
	#if defined __PFM8__
			TIM_OC1PreloadConfig(TIM2, TIM_OCPreload_Enable);
			TIM_OC2PreloadConfig(TIM2, TIM_OCPreload_Enable);
			TIM_OC3PreloadConfig(TIM2, TIM_OCPreload_Enable);
			TIM_OC4PreloadConfig(TIM2, TIM_OCPreload_Enable);
			TIM_OC1PreloadConfig(TIM4, TIM_OCPreload_Enable);
			TIM_OC2PreloadConfig(TIM4, TIM_OCPreload_Enable);
			TIM_OC3PreloadConfig(TIM4, TIM_OCPreload_Enable);
			TIM_OC4PreloadConfig(TIM4, TIM_OCPreload_Enable);
#endif				
}

/*******************************************************************************/
static __inline void	_ENABLE_PWM_OUT(void) {
			GPIOE->MODER |= ((2<<(2*9))  | (2<<(2*11)) | (2<<(2*13)) | (2<<(2*14))); 							//tim1, PE 9,11,13,14		00101000100010000000000000000000
			GPIOC->MODER |= ((2<<(2*6))  | (2<<(2*7))  | (2<<(2*8))  | (2<<(2*9)));	  						//tim8, PC 6,7,8,9			
#if defined __PFM8__
			GPIOA->MODER |= ((2<<(2*0))  | (2<<(2*1)));																						//tim2,	PA 0,1
			GPIOB->MODER |= ((2<<(2*10)) | (2<<(2*11)));																					//PB 8,9
			GPIOD->MODER |= ((2<<(2*12)) | (2<<(2*13)) | (2<<(2*14)) | ((uint32_t)2<<(2*15)));		//tim4	PD 12,13,14,15
			_IGBT_RESET;
#endif
}
/*******************************************************************************/
static __inline void	_DISABLE_PWM_OUT(void) {
			GPIOE->MODER &= ~((3<<(2*9)) | (3<<(2*11)) | (3<<(2*13)) | (3<<(2*14))); 							//tim1, PE 9,11,13,14
			GPIOC->MODER &= ~((3<<(2*6)) | (3<<(2*7))  | (3<<(2*8))  | (3<<(2*9)));								//tim8, PC 6,7,8,9
#if defined __PFM8__
			GPIOA->MODER &= ~((3<<(2*0)) | (3<<(2*1)));																						//tim2,	PA 0,1
			GPIOB->MODER &= ~((3<<(2*10))| (3<<(2*11)));																					//PB 8,9
			GPIOD->MODER &= ~((3<<(2*12))| (3<<(2*13)) | (3<<(2*14)) | ((uint32_t)3<<(2*15)));		//tim4	PD 12,13,14,15
#endif	
}

static __inline void _TIMERS_PRELOAD_OFF(void) {
			TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Disable);
			TIM_OC2PreloadConfig(TIM1, TIM_OCPreload_Disable);
			TIM_OC3PreloadConfig(TIM1, TIM_OCPreload_Disable);
			TIM_OC4PreloadConfig(TIM1, TIM_OCPreload_Disable);
			TIM_OC1PreloadConfig(TIM8, TIM_OCPreload_Disable);
			TIM_OC2PreloadConfig(TIM8, TIM_OCPreload_Disable);
			TIM_OC3PreloadConfig(TIM8, TIM_OCPreload_Disable);
			TIM_OC4PreloadConfig(TIM8, TIM_OCPreload_Disable);
	#if defined __PFM8__
			TIM_OC1PreloadConfig(TIM2, TIM_OCPreload_Disable);
			TIM_OC2PreloadConfig(TIM2, TIM_OCPreload_Disable);
			TIM_OC3PreloadConfig(TIM2, TIM_OCPreload_Disable);
			TIM_OC4PreloadConfig(TIM2, TIM_OCPreload_Disable);
			TIM_OC1PreloadConfig(TIM4, TIM_OCPreload_Disable);
			TIM_OC2PreloadConfig(TIM4, TIM_OCPreload_Disable);
			TIM_OC3PreloadConfig(TIM4, TIM_OCPreload_Disable);
			TIM_OC4PreloadConfig(TIM4, TIM_OCPreload_Disable);
#endif				
}

static __inline void _TIMERS_RESYNC(PFM *p, int simmrate) {
			TIM_SetCounter(TIM1,0);
			if(_MODE(p,_XLAP_QUAD))
				TIM_SetCounter(TIM8,simmrate/2);
			else
				TIM_SetCounter(TIM8,0);
#if defined __PFM8__
			TIM_SetCounter(TIM2,simmrate/8);
			if(_MODE(p,_XLAP_QUAD))
				TIM_SetCounter(TIM4,3*simmrate/8);
			else
				TIM_SetCounter(TIM4,simmrate/8);
#endif
}

static __inline void _TIMERS_ARR_SET(int simmrate) {
			TIM_SetAutoreload(TIM1,simmrate);
			TIM_SetAutoreload(TIM8,simmrate);
#if defined __PFM8__
			TIM_SetAutoreload(TIM2,simmrate/2);
			TIM_SetAutoreload(TIM4,simmrate/2);
#endif
}
