#include	"pfm.h"
#include <ctype.h>
/**
  ******************************************************************************
  * @file    appmisc.c
  * @author  Fotona d.d.
  * @version V1
  * @date    30-Sept-2013
  * @brief	 Application support 
  *
  */
/** @addtogroup PFM6_Application
* @{
*/
_io	*__dbug;
/*******************************************************************************
* Function Name : ScopeDumpBinary
* Description   :
* Input         :
* Output        :
* Return        :
*******************************************************************************/
int		ScopeDumpBinary(_ADCDMA *buf, int count) {

static _ADCDMA	*p=NULL;
static _io			*io=NULL;
static int			m=0;

int		i,j;
union	{
			_ADCDMA adc;
			unsigned char c[sizeof(_ADCDMA)];
			}	_ADC={0,0};

			if(buf) {														// call from parse !!!
				p=buf;
				m=count;
				io=__stdin.io;
				return 0;
			} else if(io && p && m) {
					io=_stdio(io);
					__print("-! %d,%d\r\n",m,4*pfm->ADCRate);
					io=_stdio(io);
					for(i=0;i<m;++i) {
						_ADC.adc.U += p[i].U;
						_ADC.adc.I += p[i].I;
						if((i%4) == 3) {
							_ADC.adc.U /=4;
							_ADC.adc.I /=4;
							io=_stdio(io);
							for(j=0; j<sizeof(_ADCDMA); ++j)
								while(__stdout.io->put(__stdout.io->tx,_ADC.c[j])==EOF);
									_proc_loop();
							io=_stdio(io);
							_ADC.adc.U =0;
							_ADC.adc.I =0;
						}
					}
				}
			return -1;
}
/*******************************************************************************
* Function Name : str2hex
* Description   : pretvorba stringa v hex stevilo
* Input         : **p, pointer na string array (char *), n stevilo znakov
* Output        : char * se poveca za stevilo znakov
* Return        : hex integer
*******************************************************************************/
int		str2hex(char **p,int n) {
char	q[16];
			strncpy(q,*p,n)[n]='\0';
			*p += n;
			return(strtoul(q,NULL,16));
			}

_QSHAPE			qshape[_MAX_QSHAPE];
_USER_SHAPE	ushape[_MAX_USER_SHAPE];
/*******************************************************************************
* Function Name :  
* Description   : 
* Input         :  
* Return        :
*******************************************************************************/
_TIM_DMA *SetPwmTab00(PFM *p, _TIM_DMA *t) {
int		i,j,n;
int		to			=p->Burst->Time;
int		tpause	=p->Burst->Length/p->Burst->N - p->Burst->Time;							// dodatek ups....
int		Uo=p->Burst->Pmax;
int		dUo=0;																															// modif. 2,3,4... pulza, v %
float	P2V = (float)_AD2HV(p->HVref)/_PWM_RATE_HI;
//-------user shape part -----------																			// 3. koren iz razmerja energij, ajde :)
			if(*(int *)ushape) {
				float e2E=pow(pow(P2V*p->Burst->Pmax,3)/400000.0*p->Burst->Time*p->Burst->N/(*(int *)ushape),1.0/3.0)/P2V;
				p->Burst->Length=0;
				for(i=1; ushape[i].T && i < _MAX_BURST/(10*_uS)-1 && i<_MAX_USER_SHAPE; ++t,++i) {
					t->n=2*ushape[i].T/10-1;
					t->T=e2E*ushape[i].U + p->Burst->Pdelay;
				}
				p->Burst->Ereq=_SHPMOD_OFF;
				_CLEAR_MODE(p, _P_LOOP);																					// current stab. off !!!
				t->n=0;																														// EOF
				return t;																			
			}
//-------DELAY----------------------
			for(n=2*((p->Burst->Delay*_uS)/_PWM_RATE_HI)-1; n>0; n -= 256, ++t) {
				t->T=p->Burst->Pdelay;
				(n > 255) ? (t->n=255) : (t->n=n);
			};
//-------Preludij-------------------
			if(p->Burst->Ereq & (_SHPMOD_CAL | _SHPMOD_QSWCH)) {
				int	du=0,u=0;
				for(i=0; i<_MAX_QSHAPE; ++i)
					if(p->Burst->Time==qshape[i].qref) {
						if(qshape[i].q0 > 0) {
							to=qshape[i].q0;
							Uo=(int)(pow((pow(p->Burst->Pmax,3)*p->Burst->N*qshape[i].qref/to),1.0/3.0)+0.5);
							if(p->Burst->Ereq & _SHPMOD_MAIN) {
								if(Uo > qshape[i].q1)
									Uo = qshape[i].q1;
							} else
								qshape[i].q1=Uo;
//_______________________________________________________________________________________________________
// prePULSE + delay
							for(n=((to*_uS)/_PWM_RATE_HI); n>0; n--,++t) 	{
								du+=(2*Uo-u-2*du)*70/qshape[i].q0;
								u+=du*70/qshape[i].q0;						
								t->T=p->Burst->Pdelay + du + u*qshape[i].q2/100;
								t->n=1;
							}
//_______________________________________________________________________________________________________
// if Uo < q1 or calibrating prePULSE finish prePULSE & return
							if(Uo < qshape[i].q1 || p->Burst->Ereq == _SHPMOD_CAL) {
								while(du > p->Burst->Pdelay) 	{
									du+=(0-u-2*du)*70/qshape[i].q0; 
									u+=du*70/qshape[i].q0;
									t->T=p->Burst->Pdelay + du + u*qshape[i].q2/100;
									t->n=1;
									++t;
									++to;
								}

								for(n=2*((p->Burst->Length*_uS/p->Burst->N-to*_uS)/_PWM_RATE_HI)-1;n>0;n -= 256,++t)	{
									t->T=p->Burst->Pdelay;
									(n > 255) ? (t->n=255) : (t->n=n);
								}
								t->n=0;
								return t;
							}
						}
//_______________________________________________________________________________________________________
// else change parameters & continue to 1.pulse
//
						if(p->Burst->Ereq & _SHPMOD_QSWCH) {
							to=qshape[i].qref;
							Uo=p->Burst->Pmax;
							dUo=pow(p->Burst->Pmax*P2V,3) - 400000000 / qshape[i].qref * qshape[i].q3;			// varianta z zmanjsevanjem za fiksno E(J);
							if(dUo > 0)
								dUo=pow(dUo,1.0/3.0)/P2V - Uo;
							else
								dUo=-Uo;
//						dUo=(int)(pow(pow(Uo,3)*(float)qshape[i].q3/100.0,1.0/3.0))-Uo;									// varianta z zmanjsevanjem v % energije
//						dUo=(Uo * qshape[i].q3)/100 - Uo;																								// varianta z zmanjsevanjem v % napetosti
						} else {
							to=qshape[i].q3;
							tpause=_minmax(Uo,260,550,20,100);
							Uo=(int)(pow((pow(p->Burst->Pmax,3)*p->Burst->N*qshape[i].qref - pow(qshape[i].q1,3)*qshape[i].q0)/qshape[i].qref/p->Burst->N,1.0/3.0)+0.5);
						}
					}				
			}
			if(p->Burst->Ereq & _SHPMOD_MAIN) {
//-------PULSE----------------------
				for(j=0; j<p->Burst->N; ++j) {			
					for(n=2*((to*_uS + _PWM_RATE_HI/2)/_PWM_RATE_HI)-1; n>0; n -= 256, ++t) {
						
						if(j == 0) {
							t->T=Uo+p->Burst->Pdelay;			
						} else {
							t->T=Uo+dUo+p->Burst->Pdelay;					
						}
						
						(n > 255) ? (t->n=255) : (t->n=p->Pockels.trigger=n);
					}
//-------PAUSE----------------------			
					for(n=2*((tpause*_uS)/_PWM_RATE_HI)-1;n>0;n -= 256,++t)	{
						t->T=p->Burst->Pdelay;
						(n > 255) ? (t->n=255) : (t->n=n);
					}
				}
			}
//------- fill seq. till end, except in user mode--------		
			if(p->Burst->Ereq != _SHPMOD_OFF) {
				for(n=2*((p->Burst->Length*_uS - p->Burst->N*(to+tpause)*_uS)/_PWM_RATE_HI)-1;n>0;n -= 256,++t)	{
					t->T=p->Burst->Pdelay;
					(n > 255) ? (t->n=255) : (t->n=n);
				}
			}
			t->n=0;
			return t;
}
/*******************************************************************************
* Function Name : SetPwmTab
* Description   : Selects the pw buffer, according to burst & simmer parameters 
*								: Calls the waveform generator SetPwmTab00
*								: Calculates energy integrating interval to ADC
* Input         : *p, PFM object pointer
* Return        :
*******************************************************************************/
void	SetPwmTab(PFM *p) {
			int n,ch=p->Simmer.active;												// active channel
			while(_MODE(p,_PULSE_INPROC))											// wait the prev setup to finish !!!
				_wait(2,_proc_loop);
			if(ch == PFM_STAT_SIMM1) {				
				_TIM_DMA *t=SetPwmTab00(p,_TIM.pwch1);
				for(n=0; t-- != _TIM.pwch1; n+= t->n);
				_TIM.eint1 = (n)*5;
			}
			else if(ch == PFM_STAT_SIMM2) {
				_TIM_DMA *t=SetPwmTab00(p,_TIM.pwch2);
				for(n=0; t-- != _TIM.pwch2; n+= t->n);
				_TIM.eint2 = (n)*5;
			} else {
				_TIM_DMA *t = SetPwmTab00(p,_TIM.pwch1);
				memcpy(_TIM.pwch2,_TIM.pwch1,sizeof(_TIM_DMA)*_MAX_BURST/_PWM_RATE_HI);
				memcpy(&pfm->burst[1],&pfm->burst[0],sizeof(burst));
				for(n=0; t-- != _TIM.pwch1; n+= t->n);
				_TIM.eint1=_TIM.eint2 = (n)*5;
			}
}
/*______________________________________________________________________________
* Function Name : SetSimmerPw
* Description   : simmer pulse width
* Input         : None
* Output        : None
* Return        : None
*/
static void	_TIMERS_PWM_SET(PFM *p, int rate) {

int 	psimm0=p->Simmer.pw[0];
int 	psimm1=p->Simmer.pw[1];

			if(p->Simmer.active != _STATUS(p, PFM_STAT_SIMM1 | PFM_STAT_SIMM2)) {			
				if(p->Simmer.active & PFM_STAT_SIMM1)
						psimm1=psimm0;
				else
						psimm0=psimm1;
			}
			if(_MODE(p,_XLAP_SINGLE)) {
				if(_STATUS(p, PFM_STAT_SIMM1))
					TIM8->CCR2=TIM8->CCR1=TIM1->CCR2=TIM1->CCR1=psimm0;
				else
					TIM8->CCR2=TIM8->CCR1=TIM1->CCR2=TIM1->CCR1=0;
				if(_STATUS(p, PFM_STAT_SIMM2))
					TIM8->CCR4=TIM8->CCR3=TIM1->CCR4=TIM1->CCR3=psimm1;
				else
					TIM8->CCR4=TIM8->CCR3=TIM1->CCR4=TIM1->CCR3=0;
				TIM_OC2PolarityConfig(TIM1, _PWM_HIGH);
				TIM_OC4PolarityConfig(TIM1, _PWM_HIGH);
				TIM_OC2PolarityConfig(TIM8, _PWM_HIGH);
				TIM_OC4PolarityConfig(TIM8, _PWM_HIGH);
			} else {
				if(_STATUS(p, PFM_STAT_SIMM1))  {
					TIM8->CCR1=TIM1->CCR1=psimm0;
					TIM8->CCR2=TIM1->CCR2=rate-psimm0;
				} else {
					TIM8->CCR1=TIM1->CCR1=0;
					TIM8->CCR2=TIM1->CCR2=rate;
				}
				if(_STATUS(p, PFM_STAT_SIMM2))  {
					TIM8->CCR3=TIM1->CCR3=psimm1;
					TIM8->CCR4=TIM1->CCR4=rate-psimm1;
				} else {
					TIM8->CCR3=TIM1->CCR3=0;
					TIM8->CCR4=TIM1->CCR4=rate;
				}
				TIM_OC2PolarityConfig(TIM1, _PWM_LOW);
				TIM_OC4PolarityConfig(TIM1, _PWM_LOW);
				TIM_OC2PolarityConfig(TIM8, _PWM_LOW);
				TIM_OC4PolarityConfig(TIM8, _PWM_LOW);
			}
#if defined __PFM8__
			if(_MODE(p,_XLAP_SINGLE)) {
				if(_STATUS(p, PFM_STAT_SIMM1))
					TIM2->CCR2=TIM2->CCR1=TIM4->CCR2=TIM4->CCR1=psimm0/2;
				else
					TIM2->CCR2=TIM2->CCR1=TIM4->CCR2=TIM4->CCR1=0;
				if(_STATUS(p, PFM_STAT_SIMM2))
					TIM2->CCR4=TIM2->CCR3=TIM4->CCR4=TIM4->CCR3=psimm1/2;
				else
					TIM2->CCR4=TIM2->CCR3=TIM4->CCR4=TIM4->CCR3=0;
				TIM_OC2PolarityConfig(TIM2, _PWM_HIGH);
				TIM_OC4PolarityConfig(TIM2, _PWM_HIGH);
				TIM_OC2PolarityConfig(TIM4, _PWM_HIGH);
				TIM_OC4PolarityConfig(TIM4, _PWM_HIGH);
			} else {
				if(_STATUS(p, PFM_STAT_SIMM1))  {
					TIM2->CCR1=TIM4->CCR1=psimm0/2;
					TIM2->CCR2=TIM4->CCR2=(rate-psimm0)/2;
				} else {
					TIM2->CCR1=TIM4->CCR1=0;
					TIM2->CCR2=TIM4->CCR2=rate/2;
				}
				if(_STATUS(p, PFM_STAT_SIMM2))  {
					TIM2->CCR3=TIM4->CCR3=psimm1/2;
					TIM2->CCR4=TIM4->CCR4=(rate-psimm1)/2;
				} else {
					TIM2->CCR3=TIM4->CCR3=0;
					TIM2->CCR4=TIM4->CCR4=rate/2;
				}
				TIM_OC2PolarityConfig(TIM4, _PWM_LOW);
				TIM_OC4PolarityConfig(TIM4, _PWM_LOW);
				TIM_OC2PolarityConfig(TIM2, _PWM_LOW);
				TIM_OC4PolarityConfig(TIM2, _PWM_LOW);
			}
#endif
}
/*__________________________________________________________________________
* Function Name : SetSimmerRate
* Description   : simmer pulse width
* Input         : None
* Output        : None
* Return        : None
*/
void	SetSimmerRate(PFM *p, SimmerType type) {	
int		simmrate;
			_CLEAR_MODE(pfm,_XLAP_SINGLE);
			_CLEAR_MODE(pfm,_XLAP_DOUBLE);
			_CLEAR_MODE(pfm,_XLAP_QUAD);
			
			if(type == _SIMMER_HIGH) {
				simmrate = _PWM_RATE_HI;
				_SET_MODE(pfm,pfm->Burst->Mode);
			} else {
				if(p->Simmer.active &  PFM_STAT_SIMM1) {
					simmrate=p->Simmer.rate[0];
					_SET_MODE(pfm,p->Simmer.mode);
				}	else {
					simmrate=p->Simmer.rate[1];
					_SET_MODE(pfm,p->Simmer.mode);
				}
			}
			__disable_irq();			
			while(!(TIM1->CR1 & TIM_CR1_DIR));
			while((TIM1->CR1 & TIM_CR1_DIR));
			_TIMERS_HALT();
			_DISABLE_PWM_OUT();
			__enable_irq();
			if(type == _SIMMER_HIGH) {
				_TIMERS_RESYNC(p,simmrate);
				_TIMERS_PWM_SET(p,simmrate);		
				_TIMERS_ARR_SET(simmrate);
				_TIMERS_PRELOAD_ON();
			} else {
				_TIMERS_PRELOAD_OFF();
				_TIMERS_ARR_SET(simmrate);
				_TIMERS_PWM_SET(p,simmrate);			
				_TIMERS_RESYNC(p,simmrate);
			}
			if(type == _SIMMER_HIGH) {
				_SET_MODE(pfm,_PULSE_INPROC);
				TriggerADC(p);
				TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
				TIM_ITConfig(TIM1,TIM_IT_Update,ENABLE);
			} else {
				TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
				TIM_ITConfig(TIM1,TIM_IT_Update,DISABLE);
				_CLEAR_MODE(pfm,_PULSE_INPROC);
			}
			TIM_Cmd(TIM1,ENABLE);
			_ENABLE_PWM_OUT();
}
/*******************************************************************************/
int		fanPmin=20;
int		fanPmax=95;
int		fanTL=3000;
int		fanTH=4000;
/*******************************************************************************/
/**
  * @brief  temperature interpolation from 	ADC data
  * @param t: ADC readout
  * @retval : temperature (10x deg.C)
  */
int		IgbtTemp(temp_ch n) {
const int Ttab[]={											// tabela kontrolnih toèk
			2500,								  						// za interpolacijo tempreature
			5000,								  						// zaradi prec izracuna x 100
			8000,
			12500
			};
#if		defined (__PFM6__)

const	int Rtab[]={
			(4096.0*_Rdiv(47000.0,22000.0)),	// tabela vhodnih toèk za
			(4096.0*_Rdiv(18045.0,22000.0)),	// interpolacijo (readout iz ADC !!!)
			(4096.0*_Rdiv(6685.6,22000.0)),
			(4096.0*_Rdiv(1936.6,22000.0))
			};
	
int		cc,t=__max( __fit(ADC3_buf[0].IgbtT[0],Rtab,Ttab),
									__fit(ADC3_buf[0].IgbtT[1],Rtab,Ttab));

#elif	defined (__PFM8__)
																				// IgbtTemp()
const	int Rtab[]={
			(4096.0*_Rdiv(3000.0,10000.0)),		// tabela vhodnih toèk za
			(4096.0*_Rdiv(3000.0,3598.7)),		// interpolacijo (readout iz ADC !!!)
			(4096.0*_Rdiv(3000.0,1251.8)),
			(4096.0*_Rdiv(3000.0,336.7))
			};

int		cc,t=__max( __max(__fit(ADC3_buf[0].IgbtT[0],Rtab,Ttab),
												__fit(ADC3_buf[0].IgbtT[1],Rtab,Ttab)),
									__max(__fit(ADC3_buf[0].IgbtT[2],Rtab,Ttab),
												__fit(ADC3_buf[0].IgbtT[3],Rtab,Ttab)));
#else

const	int Rtab[]={
			(4096.0*_Rdiv(47000.0,22000.0)),	// tabela vhodnih toèk za
			(4096.0*_Rdiv(18045.0,22000.0)),	// interpolacijo (readout iz ADC !!!)
			(4096.0*_Rdiv(6685.6,22000.0)),
			(4096.0*_Rdiv(1936.6,22000.0))
			};
int		cc,t=(_FAN_PWM_RATE*fanPmin)/200;
#endif
			if(__time__ > 5000) {
				if(t<fanTL)
					cc=(_FAN_PWM_RATE*fanPmin)/200;
				else {
					if (t>fanTH)
						cc=(_FAN_PWM_RATE*fanPmax)/200;
					else
						cc=(_FAN_PWM_RATE*(((t-fanTL)*(fanPmax-fanPmin))/(fanTH-fanTL)+fanPmin	))/200;
				}
				cc=__min(_FAN_PWM_RATE/2-5,__max(5,cc));
				if(TIM_GetCapture1(TIM13) < cc)
					TIM_SetCompare1(TIM13,TIM_GetCapture1(TIM13)+1);
				else
					TIM_SetCompare1(TIM13,TIM_GetCapture1(TIM13)-1);
			}
			if(n==T_MIN)
				return(t);
			else
				return __fit(ADC3_buf[0].IgbtT[n-1],Rtab,Ttab);
}
/*******************************************************************************/
/**
  * @brief	: fit 2 reda, Bronštajn str. 670
  * @param t:
  * @retval :
  */
int  	__fit(int to, const int t[], const int ft[]) {
int		f3=(ft[3]*(t[0]-to)-ft[0]*(t[3]-to)) / (t[0]-t[3]);
int		f2=(ft[2]*(t[0]-to)-ft[0]*(t[2]-to)) / (t[0]-t[2]);
int		f1=(ft[1]*(t[0]-to)-ft[0]*(t[1]-to)) / (t[0]-t[1]);
			f3=(f3*(t[1]-to) - f1*(t[3]-to)) / (t[1]-t[3]);
			f2=(f2*(t[1]-to)-f1*(t[2]-to)) / (t[1]-t[2]);
			return(f3*(t[2]-to)-f2*(t[3]-to)) / (t[2]-t[3]);
}
/*******************************************************************************
* Function Name : __lin2f
* Description   :	ADP1047 linear to float converter
* Input         :
* Output        :
* Return        :
*******************************************************************************/
float	__lin2f(short i) {
			return((i&0x7ff)*pow(2,i>>11));
}
/*******************************************************************************
* Function Name : __f2lin
* Description   : ADP1047 float to linear converter
* Input         :
* Output        :
* Return        :
*******************************************************************************/
short	__f2lin(float u, short exp) {
			return ((((int)(u/pow(2,exp>>11)))&0x7ff)  | (exp & 0xf800));
}
/*******************************************************************************
* Function Name : SetChargerVoltage
* Description   :	ADP1047 output voltage setup, using the default format
* Input         :
* Output        :
* Return        :
*******************************************************************************/
int		SetChargerVoltage(int u) {
struct	{ signed int e:3; } 
e3;
int		i=_VOUT_MODE;

			if(readI2C(__charger6,(char *)&i,1))	{
				e3.e=i;
				i = _VOUT+((u<<(8-e3.e))&~0xff);
					if(writeI2C(__charger6,(char *)&i,3))
						return _PARSE_OK;
			}
			return _PARSE_ERR_NORESP;
}
/*******************************************************************************
* Function Name : batch
* Description   :	
* Input         :
* Output        :
* Return        :
*******************************************************************************/
int		batch(char *filename) {
			FIL		f;
			if(f_open(&f,filename,FA_READ)==FR_OK) {
				while(!f_eof(&f))
					ParseFile(&f);
				f_close(&f);
				return _PARSE_OK;
			} else
				return _PARSE_ERR_OPENFILE;
}
/*******************************************************************************
* Function Name : batch
* Description   :	ADP1047 output voltage setup, using the default format
* Input         :
* Output        :
* Return        :
*******************************************************************************/
void	CAN_console(void) {
char	c[128];
int		i,j;
			__dbug=__stdin.io;
			_SET_MODE(pfm,_CAN_2_COM);
			__print("\r\n remote console open... \r\n>");
			sprintf(c,">%03X %02X %02X",_ID_SYS2PFMcom,'v','\r');
			DecodeCom(c);
			do {
				for(i=0; i<8; ++i) {
					j=getchar();
					if(j == EOF || j == _CtrlE)
						break;
					sprintf(&c[3*i+4]," %02X",j);
				}
				if(i > 0)
					DecodeCom(c);
				_proc_loop();
			} while (j != _CtrlE);
			sprintf(c,">%03X",_ID_SYS2PFMcom);
			DecodeCom(c);
			_CLEAR_MODE(pfm,_CAN_2_COM);
			__dbug=NULL;
			__print("\r\n ....remote console closed\r\n>");
}
/*******************************************************************************
* Function Name : batch
* Description   :	ADP1047 output voltage setup, using the default format
* Input         :
* Output        :
* Return        :
*******************************************************************************/
int		__print(char *format, ...) {
			char 		buf[128],*p;
			va_list	aptr;
			int			ret;
			
			va_start(aptr, format);
			ret = vsnprintf(buf, sizeof(buf), format, aptr);
			va_end(aptr);
			for(p=buf; *p; ++p)
				while(fputc(*p,&__stdout)==EOF)
					_wait(2,_proc_loop);
			return(ret);
}
/**
  * @}
  */ 
/*-----------------------------------------------------------------------*/
void	SectorQuery(void) {
int		i,j,*p,*q;

			p=(int *)FATFS_ADDRESS;
			for(i=0; i<SECTOR_COUNT; ++i) {
				if(!((i%255)%16))
					__print("\r\n");
				if(!(i%255))
					__print("\r\n");		
				if(p[SECTOR_SIZE/4] == -1)
					__print(" --- ");
				else {
					q=&p[SECTOR_SIZE/4+1];
					j=i;
					while(++j<SECTOR_COUNT && p[SECTOR_SIZE/4] != q[SECTOR_SIZE/4])
						q=&q[SECTOR_SIZE/4+1];
					if(j==SECTOR_COUNT)
						__print(" %-04X",p[SECTOR_SIZE/4]);
					else
						__print("%c%-04X",'*',p[SECTOR_SIZE/4]);
				}
				p=&p[SECTOR_SIZE/4+1];
			}
}
/*-----------------------------------------------------------------------*/																		
int		FLASH_Erase(uint32_t FLASH_Sector) {
int		m;
			FLASH_Unlock();
			FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR|FLASH_FLAG_PGSERR);	
			do {
				Watchdog();	
				m=FLASH_EraseSector(FLASH_Sector,VoltageRange_3);
			} while (m==FLASH_BUSY);
			return(m);
}
/*-----------------------------------------------------------------------*/																		
int		Defragment(int mode) {
int 	i,f,e,*p,*q,buf[SECTOR_SIZE/4];
int		c0=0,c1=0;

			f=FATFS_SECTOR;																															// f=koda prvega 128k sektorja
			e=PAGE_SIZE;																																// e=velikost sektorja
			p=(int *)FATFS_ADDRESS;																											// p=hw adresa sektorja
			do {
				do {
					++c0;
					Watchdog();																															//jk822iohfw
					q=&p[SECTOR_SIZE/4+1];																									
					while(p[SECTOR_SIZE/4] != q[SECTOR_SIZE/4] && q[SECTOR_SIZE/4] != -1)		// iskanje ze prepisanih sektorjev
						q=&q[SECTOR_SIZE/4+1];
					if(q[SECTOR_SIZE/4] == -1) {																						// ce ni kopija, se ga prepise na konec fs
						for(i=0; i<SECTOR_SIZE/4;++i)
							buf[i]=~p[i];
						Watchdog();
						if(mode)
							disk_write (*FS_CPU-'0',(uint8_t *)buf,p[SECTOR_SIZE/4],1);					// STORAGE_Write bo po prvem brisanju zacel na
					} else																																	// zacetku !!!
						++c1;
					p=&p[SECTOR_SIZE/4+1]; 
				} while(((int)p)-FATFS_ADDRESS <  e && p[SECTOR_SIZE/4] != -1);						// prepisana cela stran...
				if(mode)
					FLASH_Erase(f);																													// brisi !
				f+=FLASH_Sector_1; 
				e+=PAGE_SIZE;
			} while(p[SECTOR_SIZE/4] != -1);	
			if(mode) {
				FLASH_Erase(f);																														// se zadnja !
				return 0;
			} else 
				return(100*c1/c0);
}
//______________________________________________________________________________________
char	*cgets(int c, int mode)
{
_buffer		*p=__stdin.io->gets;
			
			if(!p)
				p=__stdin.io->gets=_buffer_init(__stdin.io->rx->size);
			switch(c) {
				case EOF:		
				case '\n':
					break;
				case '\r':
					*p->_push = '\0';
					p->_push=p->_pull=p->_buf;
					return(p->_buf);
				case 0x08:
				case 0x7F:
					if(p->_push != p->_pull) {
						--p->_push;
						if(mode)
							__print("\b \b");
					}
					break;
				default:
					if(p->_push != &p->_buf[p->size-1])
						*p->_push++ = c;
					else  {
						*p->_push=c;
						if(mode)
							__print("%c",'\b');
					}
					if(mode) {
						if(c < ' ' || c > 127)
							__print("%c%02X%c",'<',c,'>');
						else
							__print("%c",c);
					}
					break;
			}
			return(NULL);
}
//______________________________________________________________________________________
int		strscan(char *s,char *ss[],int c) {
			int		i=0;
			while(1)
			{
				while(*s==' ') ++s;
				if(!*s)
					return(i);

				ss[i++]=s;
				while(*s && *s!=c)
				{
					if(*s==' ')
						*s='\0';
					s++;
				}
				if(!*s)
					return(i);
				*s++=0;
			}
}
//______________________________________________________________________________________
int		numscan(char *s,char *ss[],int c) {
			while(*s && !isdigit(*s)) 
				++s;
			return(strscan(s,ss,c));
}
//______________________________________________________________________________________
int		hex2asc(int i)
{
			if(i<10)
				return(i+'0');
			else 
				return(i-10+'A');
}
//_____________________________________________________________________________________
int		asc2hex(int i)
{
			if(isxdigit(i))
			{
				if(isdigit(i))
					return(i-'0');
				else
					return(toupper(i)-'A'+0x0a);
			}
			else
				return(0);
}
//______________________________________________________________________________________
int		getHEX(char *p, int n)
{
			int	i=0;
			while(n-- && isxdigit(*p))
				i=(i<<4) | asc2hex(*p++);
			return(i);
}
//_____________________________________________________________________________________
char	*endstr(char *p)
{
			while(*p)
				++p;
			return(p);
}
//_____________________________________________________________________________________
#define		HEXREC3
//_____________________________________________________________________________________
int		sDump(char *p,int n)
{
			int	i,j;

#ifdef	HEXREC1
			i=(int)p + ((int)p >> 8);
			if(n<252)
				j=n;
			else
				j=252;
			n -= j;
			i += (j+3);
			__print("\r\nS1%02X%04X",j+3,(int)p);
#endif

#ifdef	HEXREC2
			i=(int)p + ((int)p >> 8) + ((int)p >> 16);
			if(n<250)
				j=n;
			else
				j=250;
			n -= j;
			i += (j+4);
			__print("\r\nS2%02X%06X",j+4,(int)p);
#endif

#ifdef	HEXREC3
			i=(int)p + ((int)p >> 8)+ ((int)p >>16)+ ((int)p >> 24);
			if(n<248)
				j=n;
			else
				j=248;
			n -= j;
			i += (j+5);
			__print("\r\nS3%02X%08X",j+5,(int)p);
#endif
//_____________________________________________________________________________________
			while(j--)
			{
				i += *p;
				__print("%02X",*p++ & 0xff);
			}
			__print("%02X",~i & 0xff);
			if(n)
				sDump(p,n);
			return(-1);
}
//_____________________________________________________________________________________
int		iDump(int *p,int n)
{
			int		i,j,k;
			union 	{int i; char c[sizeof(int)];} u;

#ifdef	HEXREC1
			i=(int)p + ((int)p >> 8);
			if(n < (255-3)/sizeof(int))
				j=n;
			else
				j=(255-3)/sizeof(int);
			n -= j;
			i += (sizeof(int)*j+3);
			__print("\r\nS1%02X%04X",sizeof(int)*j+3,(int)p);
#endif

#ifdef	HEXREC2
			i=(int)p + ((int)p >> 8) + ((int)p >> 16);
			if(n < (255-5)/sizeof(int))
				j=n;
			else
				j=(255-5)/sizeof(int);
			n -= j;
			i += (sizeof(int)*j+4);
			__print("\r\nS2%02X%06X",sizeof(int)*j+4,(int)p);
#endif

#ifdef	HEXREC3
			i=(int)p + ((int)p >> 8)+ ((int)p >>16)+ ((int)p >> 24);
			if(n < (255-7)/sizeof(int))
				j=n;
			else
				j=(255-7)/sizeof(int);
			n -= j;
			i += (sizeof(int)*j+5);
			__print("\r\nS3%02X%08X",sizeof(int)*j+5,(int)p);
#endif
//_____________________________________________________________________________________
			while(j--)
			{
				u.i=*p++;
				for(k=0; k<sizeof(int); ++k)
				{
					i += u.c[k]; 
					__print("%02X",u.c[k]);
				}
			}
			__print("%02X",~i & 0xff);
			if(n)
				iDump(p,n);
			return(-1);
}
//_____________________________________________________________________________________
int		sLoad(char *p)
{
			int	 err,k,n;
			char *q,*a=NULL;
			k=*++p;												// skip 'S', k='1','2','3'
			++p;
			err=n=getHEX(p,2);
			q=p;
			++q;++q;
			while(--n) {
				err += getHEX(q,2);
				++q;++q;
			}
			if(((~err) & 0xff) != getHEX(q,2))
				__print("...checksum error !");
			else {
				n=getHEX(p,2);				++p;++p;
				switch(k) {
				case '1':
					a=(char *)getHEX(p,4);	++p;++p;++p;++p;
					n=n-3;
					break;
				case '2':
					a=(char *)getHEX(p,6);	++p;++p;++p;++p;++p;++p;
					n=n-4;
					break;
				case '3':
					a=(char *)getHEX(p,8);	++p;++p;++p;++p;++p;++p;++p;++p;
					n=n-5;
					break;
				}
				while(n--) {
					*(char *)a=getHEX(p,2);
					++a;++p;++p;
				}
			}
			return(-1);
}

#if !defined (__DISC7__)

extern 	USBD_DEVICE 						USR_MSC_desc,	USR_VCP_desc;
extern 	USBD_Class_cb_TypeDef  	USBD_MSC_cb,	USBD_CDC_cb;
extern 	USBH_HOST								USB_Host;
extern 	USB_OTG_CORE_HANDLE  		USB_OTG_Core;
typedef enum {off, on} vbus;
void	 	Vbus(vbus on) {
	
#if defined (_VBUS_BIT) && defined (_VBUS_PORT)
				GPIO_InitTypeDef					GPIO_InitStructure;
				GPIO_StructInit(&GPIO_InitStructure);
				GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
				GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
				GPIO_InitStructure.GPIO_Pin = _VBUS_BIT;
				GPIO_Init(_VBUS_PORT, &GPIO_InitStructure);
				if(on)
					GPIO_ResetBits(_VBUS_PORT,_VBUS_BIT);
				else
					GPIO_SetBits(_VBUS_PORT,_VBUS_BIT);
#endif
}
#endif

void	 	USB_MSC_device(void) {
#ifdef _VBUS_BIT
				Vbus(off);
#endif
#if defined (_USB_DIR_BIT) && defined (_USB_DIR_PORT)
				GPIO_SetBits(_USB_DIR_PORT,_USB_DIR_BIT);
#endif
				USBD_Init(&USB_OTG_Core, USB_OTG_FS_CORE_ID, &USR_MSC_desc, &USBD_MSC_cb, &USR_MSC_cb);
}

void		USB_VCP_device(void) {
#ifdef _VBUS_BIT
				Vbus(off);
#endif
#if defined (_USB_DIR_BIT) && defined (_USB_DIR_PORT)
				GPIO_SetBits(_USB_DIR_PORT,_USB_DIR_BIT);
#endif
				USBD_Init(&USB_OTG_Core, USB_OTG_FS_CORE_ID, &USR_VCP_desc, &USBD_CDC_cb, &USR_CDC_cb);
}

void		USBHost (_proc *p) {
				USBH_Process(&USB_OTG_Core, p->arg);
}

void		USB_MSC_host(void) {
#if defined (_USB_DIR_BIT) && defined (_USB_DIR_PORT)
				GPIO_ResetBits(_USB_DIR_PORT,_USB_DIR_BIT);
#endif
#ifdef _VBUS_BIT
				Vbus(on);
#endif
	
				USBH_App=USBH_Iap;
				USBH_Init(&USB_OTG_Core, USB_OTG_FS_CORE_ID, &USB_Host, &USBH_MSC_cb, &USR_USBH_MSC_cb);
				if(!_proc_find((func *)USBHost,&USB_Host))
					_proc_add((func *)USBHost,&USB_Host,"host USB",0);
}

/*						VBUS
PFM6 F4					N
PFM6 F7					N
PFM8 F4					N
PFM8 F7					N
DISC F4					Y
DISC F746				Y
DISC F769				Y
*/



/**
* @}
*/
/*
>1a
+e 0
*/

