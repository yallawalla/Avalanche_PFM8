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

static		enum 	{_BOTH,_ALTER,_Er,_Nd} 																triggerMode	=_BOTH;
static		enum 	{_ErSetup,_NdSetup,_Pockels, _STANDBY,_READY,_LASER}	state				=_STANDBY;
static		enum	{_OFF,_SIMM1,_SIMM2,_SIMM_ALL}												simm				=_OFF;
static		int		idx;
//______________________________________________________________________________________
static
	void		LoadSettings() {
FIL				f;
FATFS			fs;
UINT			bw;
					if(f_chdrive(FS_CPU) == FR_OK &&
						f_mount(&fs,FS_CPU,1) == FR_OK && 
						f_open(&f,"/tandem.bin",FA_READ) == FR_OK) {
							f_read(&f,pfm->burst,2*sizeof(burst),&bw);
							f_read(&f,&triggerMode,sizeof(triggerMode),&bw);
							f_close(&f);
					}
}
//______________________________________________________________________________________
static
	void		SaveSettings() {
FIL				f;
FATFS			fs;
UINT			bw;
					if(f_chdrive(FS_CPU) == FR_OK &&
						f_mount(&fs,FS_CPU,1) == FR_OK && 
							f_open(&f,"/tandem.bin",FA_WRITE | FA_OPEN_ALWAYS) == FR_OK) {
								f_write(&f,pfm->burst,2*sizeof(burst),&bw);
								f_write(&f,&triggerMode,sizeof(triggerMode),&bw);
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
							__print("\rEr     : %6du,%6dV,  n=%3d,%6du",
								pfm->Burst->Time,pfm->Burst->PW*_AD2HV(pfm->HVref)/_PWM_RATE_HI,pfm->Burst->N,pfm->Burst->Length);
							break;
						case _NdSetup:							
							__print("\rNd     : %6du,%6dV,  n=%3d,%6du",
								pfm->Burst->Time,pfm->Burst->PW*_AD2HV(pfm->HVref)/_PWM_RATE_HI,pfm->Burst->N,pfm->Burst->Length);
							break;
						case _Pockels:							
							__print("\rpockels: %6.1fu,%6.1fu,%6.1fu,%6.1fu",
								(float)pfm->burst[0].pockels.delay/10,(float)pfm->burst[0].pockels.width/10,
									(float)pfm->burst[1].pockels.delay/10,(float)pfm->burst[1].pockels.width/10);
							break;
						case _LASER:
							__dbug=__stdin.io;
							_SET_DBG(pfm,_DBG_PULSE_MSG);
							_SET_DBG(pfm,_DBG_ENM_MSG);
							return;
						default:				
							switch(triggerMode) {
								case _BOTH:
									_CLEAR_MODE(pfm,_ALTERNATE_TRIGGER);
									__print("\rtrigger:    BOTH,%6dm,%6du",pfm->burst[1].Period,pfm->burst[1].Delay-pfm->burst[0].Delay);
									break;
								case _ALTER:
									_SET_MODE(pfm,_ALTERNATE_TRIGGER);
									__print("\rtrigger:   ALTER,%6dm,%6dm",i,i-pfm->burst[1].Period);
									break;
								case _Er:
									_CLEAR_MODE(pfm,_ALTERNATE_TRIGGER);
									__print("\rtrigger:      Er,%6dm,%6du",pfm->Burst->Period,pfm->burst[1].Delay-pfm->burst[0].Delay);
									break;
								case _Nd:
									_CLEAR_MODE(pfm,_ALTERNATE_TRIGGER);
									__print("\rtrigger:      Nd,%6dm,%6du",pfm->Burst->Period,pfm->burst[1].Delay-pfm->burst[0].Delay);
									break;
							}
							if(state==_STANDBY)
								__print(",STANDBY");
							if(state==_READY)
								__print(",  READY");
							for(i=8*(3-idx)+1; i--; __print("\b"));
							return;
					}
					for(i=8*(3-idx)+1; i--; __print("\b"));
}
//______________________________________________________________________________________
static 
	void		IncrementTrigger(int a) {
					switch(idx) {
						case -1:
							idx=0;
							break;
						case 0:
							triggerMode += a;
							triggerMode = __max(_BOTH,__min(_Nd, triggerMode));
						break;
						case 1:
							pfm->burst[1].Period = __max(2,__min(2000, pfm->burst[1].Period + a));
							if(!_MODE(pfm,_ALTERNATE_TRIGGER))
								pfm->burst[0].Period=pfm->burst[1].Period;
							break;
						case 2:
							if(_MODE(pfm,_ALTERNATE_TRIGGER)) {
								pfm->burst[0].Period +=a;
								pfm->burst[1].Period -=a;
								if(pfm->burst[0].Period < 2) {
									pfm->burst[0].Period = pfm->burst[0].Period + pfm->burst[1].Period - 2;
									pfm->burst[1].Period = 2;
								}
								if(pfm->burst[1].Period < 2) {
									pfm->burst[1].Period = pfm->burst[0].Period + pfm->burst[1].Period - 2;
									pfm->burst[0].Period = 2;
								}
							} else {
								int d = pfm->burst[1].Delay - pfm->burst[0].Delay + a;
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
									_CLEAR_MODE(pfm,_AUTO_TRIGGER);
									if(pfm->burst[0].pockels.width || pfm->burst[1].pockels.width)
										PFM_pockels(pfm);
									_SetPwmTab(pfm,_SIMM1);
									_SetPwmTab(pfm,_SIMM2);
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
									if(pfm->Trigger.enotify)
										CanReply("wwwwX",0xC101,pfm->Simmer.active,40000,pfm->Burst->Length,_ID_SYS2ENRG);
									break;
								case _LASER:
									if(!_MODE(pfm,_AUTO_TRIGGER)) {
										__print("\r\n>");
										_wait(5,_proc_loop);
										_SET_MODE(pfm,_AUTO_TRIGGER);
										_SET_EVENT(pfm,_TRIGGER);	
									}
									break;
								default:
									break;
							}
							return;
						default:
							idx=3;
							break;

					}
					state=_STANDBY;
					simmerMode(_OFF);	
}
//______________________________________________________________________________________
static 
	void		IncrementPockels(int a) {
					switch(idx) {
						case -1:
							idx=0;
							break;
						case 0:
							pfm->burst[0].pockels.delay	= __max(0,__min(49999,pfm->burst[0].pockels.delay + a));
							break;
						case 1:
							pfm->burst[0].pockels.width	= __max(0,__min(49999,pfm->burst[0].pockels.width +a));
							break;
						case 2:
							pfm->burst[1].pockels.delay	= __max(0,__min(49999,pfm->burst[1].pockels.delay + a));
							break;
						case 3:
							pfm->burst[1].pockels.width	= __max(0,__min(49999,pfm->burst[1].pockels.width +a));
							break;
						default:
							idx=3;
							break;	
						}
}
//______________________________________________________________________________________
static 
	void		Increment(int a) {
					if(state==_STANDBY || state==_READY || state==_LASER) 
						IncrementTrigger(a);
					else if(state==_Pockels) 
						IncrementPockels(a);
					else {
						switch(idx) {
							case -1:
								idx=0;
								break;
							case 0:
								pfm->Burst->Time		= __max(50,__min(2000,pfm->Burst->Time +a));
								break;
							case 1:
								pfm->Burst->U				= __max(0,__min(8000,pfm->Burst->U +10*a));
								break;
							case 2:
								pfm->Burst->N				= __max(1,__min(10,pfm->Burst->N +a));
								break;
							case 3:
								pfm->Burst->Length	= __max(pfm->Burst->Time,__min(5000,pfm->Burst->Length +100*a));
								break;

							default:
								idx=3;
								break;
								
						}
						if(state==_ErSetup)
							_SetPwmTab(pfm,PFM_STAT_SIMM1);
						if(state==_NdSetup)
							_SetPwmTab(pfm,PFM_STAT_SIMM2);
					}
}
//______________________________________________________________________________________
int				Tandem() {
int 			kspeed=0, ktimeout=0;
int				i,cnt=0,timeout=0;
					triggerMode=_BOTH;
					state=_STANDBY;
					simmerMode(_OFF);
					LoadSettings();

					__print("\r\n[F1]  - Er parameters");
					__print("\r\n[F2]  - Nd parameters");
					__print("\r\n[F3]  - trigger settings");
					__print("\r\n[F4]  - pockels settings");
					__print("\r\n[F11] - save settings");
					__print("\r\n[F12] - exit");
					__print("\r\n:");


					while(1) {
						i=Escape();
						switch(i) {
							case EOF:
								if(ktimeout && __time__ > ktimeout)
									kspeed=ktimeout=0;
								if(simm != pfm->Simmer.active) {
									__print("\r\n:simmer error...\r\n:");
									state=_STANDBY;
									simmerMode(_OFF);
								}
								if(pfm->Trigger.counter != cnt) {
									cnt=pfm->Trigger.counter;
									timeout=__time__ + 5;
								}
								if(state==_LASER && timeout && __time__ > timeout && pfm->Trigger.enotify) {
									timeout = 0;
									switch(triggerMode) {
										case _Er:
											CanReply("wwwwX",0xC101,1,40000,pfm->Burst->Length,_ID_SYS2ENRG);
										break;
										case _Nd:
											CanReply("wwwwX",0xC101,2,40000,pfm->Burst->Length,_ID_SYS2ENRG);
										break;
										case _BOTH:
										case _ALTER:
											if(pfm->Trigger.enotify == 3)
												CanReply("wwwwX",0xC101,(cnt % 2) + 1,40000,pfm->Burst->Length,_ID_SYS2ENRG);
											else
												CanReply("wwwwX",0xC101,pfm->Trigger.enotify,40000,pfm->Burst->Length,_ID_SYS2ENRG);
										break;
										}
								}								
								_proc_loop();
								continue;
							case _CtrlZ:
								while(1);				
							case _CtrlY:
								NVIC_SystemReset();				
							case _f1: 
							case _F1:
								if(state != _ErSetup)
									__print("\r\n");
								state=_ErSetup;
								simmerMode(_OFF);
								Increment(0);
								break;								
							case _f2: 
							case _F2:
								if(state != _NdSetup)
									__print("\r\n");
								state=_NdSetup;
								simmerMode(_OFF);
								Increment(0);
								break;								
							case _f3: 
							case _F3:
								if(state != _STANDBY)
									__print("\r\n");
								state=_STANDBY;
								simmerMode(_OFF);
								Increment(0);
								break;								
							case _f4: 
							case _F4:
								if(state != _Pockels)
									__print("\r\n");
								state=_Pockels;
								simmerMode(_OFF);
								Increment(0);
								break;								
							case _Up:
								Increment(kspeed/50 + 1);
								break;
							case _Down:
								Increment(-kspeed/50 - 1);
								break;
							case _PageUp:
								Increment(10*(kspeed/50 + 1));
								break;
							case _PageDown:
								Increment(10*(kspeed/50 - 1));
								break;
							case _Left:
									--idx;
								Increment(0);
								break;
							case _Right:
									++idx;
								Increment(0);
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
								_CLEAR_MODE(pfm,_AUTO_TRIGGER);
								_CLEAR_DBG(pfm,_DBG_PULSE_MSG);
								_CLEAR_DBG(pfm,_DBG_ENM_MSG);
								__dbug=NULL;
								__print("\r\n: bye...\r\n>");
								return _PARSE_OK;	
							default:
								_proc_loop();
								continue;
					}
					ktimeout = __time__ + 200;
					++kspeed;
					showCLI();
				}
}
