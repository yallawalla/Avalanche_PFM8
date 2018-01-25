/**
  ******************************************************************************
  * @file    com.c
  * @author  Fotona d.d.
  * @version V1
  * @date    30-Sept-2013
  * @brief	 COM port parsing related functionality
  *
  */

/** @addtogroup PFM6_Application
* @{
*/


#include	"pfm.h"
#include	<ctype.h>
#include	<math.h>
#include	<string.h>
#include	<stdio.h>
#include	<ff.h>
#include	"limits.h"

#define		simmerMode(a)			\
					simm=a;						\
					PFM_command(pfm,a);

static		enum 	{_BOTH,_ALTER,_Er,_Nd} 											triggerMode;
static		enum 	{_ErSetup,_NdSetup,_STANDBY,_READY,_LASER}	state;
static		enum	{_OFF,_SIMM1,_SIMM2,_SIMM_ALL}							simm;
static		int		idx;
//______________________________________________________________________________________
static
	void		LoadSettings() {
FIL				f;
FATFS			fs;
UINT			bw;
					if(f_mount(&fs,FS_CPU,1) == FR_OK && 
						f_open(&f,"/tandem.ini",FA_READ) == FR_OK) {
						f_read(&f,pfm->burst,2*sizeof(burst),&bw);
						f_close(&f);
					}
}
//______________________________________________________________________________________
static
	void		SaveSettings() {
FIL				f;
FATFS			fs;
UINT			bw;
					if(f_mount(&fs,FS_CPU,1) == FR_OK && 
						f_open(&f,"/tandem.ini",FA_WRITE | FA_OPEN_ALWAYS) == FR_OK) {
						f_write(&f,pfm->burst,2*sizeof(burst),&bw);
						f_sync(&f);
						f_close(&f);
					}
}
//______________________________________________________________________________________
static 
	void		showCLI() {
int				i=pfm->burst[0].Period+pfm->burst[1].Period;	

					switch(state) {
						case _ErSetup:							
							__print("\rEr     : %5du,%5dV, n=%3d,%5du",pfm->Burst->Time,pfm->Burst->Pmax*_AD2HV(pfm->HVref)/_PWM_RATE_HI,pfm->Burst->N,pfm->Burst->Length);
							break;
						case _NdSetup:							
							__print("\rNd     : %5du,%5dV, n=%3d,%5du",pfm->Burst->Time,pfm->Burst->Pmax*_AD2HV(pfm->HVref)/_PWM_RATE_HI,pfm->Burst->N,pfm->Burst->Length);
							break;
						default:				
							switch(triggerMode) {
								case _BOTH:
									_CLEAR_MODE(pfm,_ALTERNATE_TRIGGER);
									__print("\rtrigger:   BOTH,%5dm,%5du",pfm->Burst->Period,pfm->burst[1].Delay-pfm->burst[0].Delay);
									break;
								case _ALTER:
									_SET_MODE(pfm,_ALTERNATE_TRIGGER);
									__print("\rtrigger:  ALTER,%5dm,%5dm",i,i-pfm->Burst->Period);
									break;
								case _Er:
									_CLEAR_MODE(pfm,_ALTERNATE_TRIGGER);
									__print("\rtrigger:     Er,%5dm,%5du",pfm->Burst->Period,pfm->burst[1].Delay-pfm->burst[0].Delay);
									break;
								case _Nd:
									_CLEAR_MODE(pfm,_ALTERNATE_TRIGGER);
									__print("\rtrigger:     Nd,%5dm,%5du",pfm->Burst->Period,pfm->burst[1].Delay-pfm->burst[0].Delay);
									break;
							}
							if(state==_STANDBY)
								__print(", STDBY");
							if(state==_READY)
								__print(", READY");
							if(state==_LASER) {
								__print(", LASER\r\n");
								__dbug=__stdin.io;
								_SET_DBG(pfm,_DBG_PULSE_MSG);
								_SET_DBG(pfm,_DBG_ENM_MSG);
								return;
							}
							break;
					}
					for(i=7*(3-idx)+1; i--; __print("\b"));
}
//______________________________________________________________________________________
static 
	void		IncrementTrigger(int a) {
					switch(idx) {
						case 0:
							triggerMode += a;
							triggerMode = __max(_BOTH,__min(_Nd, triggerMode));
						break;
						case 1:
							pfm->burst[1].Period = __max(10,__min(2000, pfm->burst[1].Period + 10*a));
							if(!_MODE(pfm,_ALTERNATE_TRIGGER))
								pfm->burst[0].Period=pfm->burst[1].Period;
							break;
						case 2:
							if(_MODE(pfm,_ALTERNATE_TRIGGER)) {
								pfm->burst[0].Period +=a;
								pfm->burst[1].Period -=a;
								if(pfm->burst[0].Period < 5) {
									pfm->burst[0].Period = pfm->burst[0].Period + pfm->burst[1].Period - 5;
									pfm->burst[1].Period = 5;
								}
								if(pfm->burst[1].Period < 5) {
									pfm->burst[1].Period = pfm->burst[0].Period + pfm->burst[1].Period - 5;
									pfm->burst[0].Period = 5;
								}
							} else {
								int d = pfm->burst[1].Delay - pfm->burst[0].Delay + 10*a;
								if( d >= 0) {
									pfm->burst[0].Delay=300;
									pfm->burst[1].Delay=d+300;
								} else {
									pfm->burst[1].Delay=300;
									pfm->burst[0].Delay=-d+300;
								}
							}
							break;
						case 3:
							state += a;
							state = __max(_STANDBY,__min(_LASER, state));
							switch(state) {
								case _STANDBY:
									simmerMode(_OFF);	
									break;
								case _READY:
									_CLEAR_MODE(pfm,_TRIGGER_PERIODIC);
									simmerMode(_SIMM1);	
									SetPwmTab(pfm);
									simmerMode(_SIMM2);	
									SetPwmTab(pfm);
									switch(triggerMode) {
										case _BOTH:
										case _ALTER:
											simmerMode(_SIMM_ALL);	
										break;
										case _Er:
											simmerMode(_SIMM1);	
										break;
										case _Nd:
											simmerMode(_SIMM2);	
										break;
									}
									_SET_MODE(pfm,_ENM_NOTIFY);
									CanReply("wwwwX",0xC101,pfm->Simmer.active,40000,pfm->Burst->Length,_ID_SYS2ENRG);
									break;
								case _LASER:
								_SET_MODE(pfm,_TRIGGER_PERIODIC);
								_SET_EVENT(pfm,_TRIGGER);	
								break;
							default:
								break;
							}
							return;
					}
					state=_STANDBY;
					simmerMode(_OFF);	
}
//______________________________________________________________________________________
static 
	void		Increment(int a) {
					if(state==_STANDBY || state==_READY || state==_LASER) 
						IncrementTrigger(a);
					else
						switch(idx) {
							case 0:
								pfm->Burst->Time		= __max(50,__min(2000,pfm->Burst->Time +10*a));
								break;
							case 1:
								pfm->Burst->Pmax		= __max(0,__min(_PWM_RATE_HI,pfm->Burst->Pmax +a));
								break;
							case 2:
								pfm->Burst->N				= __max(1,__min(10,pfm->Burst->N +a));
								break;
							case 3:
								pfm->Burst->Length	= __max(pfm->Burst->Time,__min(5000,pfm->Burst->Length +100*a));
								break;
						}
}
//______________________________________________________________________________________
int				Tandem() {
int				i,cnt=0,timeout=0;
					LoadSettings();

					__print("\r\n[F1]  - Er parameters");
					__print("\r\n[F2]  - Nd parameters");
					__print("\r\n[F3]  - trigger parameters");
					__print("\r\n[F11] - save settings");
					__print("\r\n[F12] - exit");
					__print("\r\n:");

					triggerMode=_BOTH;
					state=_STANDBY;
					simmerMode(_OFF);

					while(1) {
						i=Escape();
						switch(i) {
							case EOF:
								if(simm != pfm->Simmer.active) {
									__print("\r\n:simmer error...\r\n:");
									state=_STANDBY;
									simmerMode(_OFF);
								}
								if(pfm->Trigger.counter != cnt) {
									cnt=pfm->Trigger.counter;
									timeout=__time__ + 5;
								}
								if(state==_LASER && timeout && __time__ > timeout &&
									(triggerMode == _BOTH || triggerMode == _ALTER)) {
										CanReply("wwwwX",0xC101,(cnt % 2) + 1,40000,pfm->Burst->Length,_ID_SYS2ENRG);
										timeout = 0;
								}									
								_proc_loop();
								continue;
							case _f1: 
							case _F1:
								if(state != _ErSetup)
									__print("\r\n");
								state=_ErSetup;
								simmerMode(_SIMM1);
								break;								
							case _f2: 
							case _F2:
								if(state != _NdSetup)
									__print("\r\n");
								state=_NdSetup;
								simmerMode(_SIMM2);
								break;								
							case _f3: 
							case _F3:
								if(state != _STANDBY)
									__print("\r\n");
								state=_STANDBY;
								simmerMode(_OFF);
								break;								
							case _Up:
								Increment(1);
								break;
							case _Down:
								Increment(-1);
								break;
							case _Left:
								if(idx)	
									--idx;
								break;
							case _Right:
								if(idx < 3)	
									++idx;
								break;
							case _f11: 
							case _F11:
								SaveSettings();
								__print("\r\n: saved...\r\n:");
								break;								
							case _f12: 
							case _F12:
								state=_STANDBY;
								simmerMode(_OFF);
								_CLEAR_MODE(pfm,_ENM_NOTIFY);
								_CLEAR_MODE(pfm,_TRIGGER_PERIODIC);
								_CLEAR_DBG(pfm,_DBG_PULSE_MSG);
								_CLEAR_DBG(pfm,_DBG_ENM_MSG);
								__dbug=NULL;
								__print("\r\n: bye...\r\n>");
								return _PARSE_OK;	
							default:
								_proc_loop();
								continue;
					}
					showCLI();
				}
}
