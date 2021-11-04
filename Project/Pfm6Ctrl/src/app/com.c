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
//___________________________________________________________________________
void		USB_MSC_host(void);
void		USB_MSC_device(void);
void		USB_VCP_device(void);
FRESULT iapRemote(char *);

//___________________________________________________________________________
//___________________________________________________________________________
//___________________________________________________________________________
//___________________________________________________________________________
//___________________________________________________________________________
//___________________________________________________________________________
//___________________________________________________________________________
//___________________________________________________________________________
//___________________________________________________________________________
//___________________________________________________________________________
//___________________________________________________________________________
//___________________________________________________________________________
//___________________________________________________________________________
//___________________________________________________________________________
//___________________________________________________________________________
//___________________________________________________________________________
//___________________________________________________________________________
//___________________________________________________________________________
//___________________________________________________________________________
//___________________________________________________________________________
int			DecodeMinus(char *c) {
				char		*cc[8];
				int			m,n,*p;
				switch(*c) {
//__________________________________________________usb host/file/serial_____
				case 'u':
				if(strscan(c,cc,' ')==2) {
					USB_MSC_host();
					_wait(200,_proc_loop);
					switch(*cc[1]) {
						case 'h':
							break;
						case 'f':
							USB_MSC_device();
							break;
						case 's':
							USB_VCP_device();
							break;
						default:
							return _PARSE_ERR_SYNTAX;
					}
					break;
				}
				return _PARSE_ERR_SYNTAX;
//__________________________________________________i2c disable _______
				case 'i':
					__charger6=NULL;
					break;
//__________________________________________________ em notification off _________
				case 'n':
					pfm->Trigger.enotify = 0;
				break;
//__________________________________________________can loop/net_____________
				case 'c':
					n=strscan(c,cc,' ');
				if(n > 1)
					switch(*cc[1]) {
						case 'l':
							if(*cc[2]== 'y')
								__can=Initialize_CAN(true);
							else if(*cc[2]== 'n')
								__can=Initialize_CAN(false);
							else
								return _PARSE_ERR_SYNTAX;		
							break;
						case 'f':
							if(n % 2)
								return(_PARSE_ERR_MISSING); 
							else while (n-2) {
								canFilterConfig(strtoul(cc[2],&cc[2],16),strtoul(cc[3],&cc[3],16));
								n-=2;
							}
						break;
						default:
							return _PARSE_ERR_SYNTAX;							
					}
				break;
//__________________________________________________Pfm8, V to F ___________
				case 'f':
					Initialize_F2V(NULL);
					_SET_MODE(pfm,_F2V);
				break;
//__________________________________________________soft crowbar reset______
				case 'r':
					_SET_STATUS(pfm,_PFM_CWBAR_STAT);
					_CLEAR_ERROR(pfm,EOF);
					_ENABLE_PWM_OUT();
					_IGBT_RESET;
					PFM_debug(NULL);
				break;
//__________________________________________________boot disable____________
				case 'B':
#if defined (_NRST_DISABLE_BIT) && defined (_BOOT_ENABLE_BIT)
					pfm->boot_timeout = atoi(strchr(c,' '));
					if(pfm->boot_timeout)
						pfm->boot_timeout += __time__;
					GPIO_SetBits(_NRST_DISABLE_PORT,_NRST_DISABLE_BIT);
					GPIO_ResetBits(_BOOT_ENABLE_PORT,_BOOT_ENABLE_BIT);
#endif
				break;
//__________________________________________________defragment _____________
				case 'P':
				{
					int i;
					i=Defragment(0);
					__print("\r\n>Packing sectors... %d%c",i,'%');
					if(i>10) {
						Watchdog_init(4000);
						Defragment(-1);
					}
					__print(" Done");
					Watchdog_init(300);
				}
				break;
				
//__________________________________________________formatting flash drive__
				case 'F':
					if(strscan(c,cc,' ')<2)
						return _PARSE_ERR_MISSING;
					switch(*cc[1]) {
						case 'u':
							__print("\rFormat usb ...[y/n]");
							do {
								_wait(5,_proc_loop);
								m=fgetc(&__stdin);
							} while(m==-1);
							if(m == 'y') {
int							wbuf[SECTOR_SIZE];
								f_mount(pfm->fatfs,FS_USB,1);
								f_mkfs(FS_USB,FM_ANY,0,wbuf,SECTOR_SIZE*sizeof(int));
//								f_mount(NULL,FS_USB,1);
							}
							break;

						case 'f':
							__print("\rFormat flash ...[y/n]");
							Watchdog_init(4000);
							do {
								_wait(5,_proc_loop);
								m=fgetc(&__stdin);
							} while(m==-1);
							if(m == 'y') {
int							wbuf[SECTOR_SIZE];
FATFS						fs_cpu;				
								__print(" erasing ");
								for(n=0; n<PAGE_COUNT; ++n)
									if(FLASH_Erase(FATFS_SECTOR+n*FLASH_Sector_1)==FLASH_COMPLETE)
										__print(".");
									else
										__print("?");
								f_mount(&fs_cpu,FS_CPU,1);
								f_mkfs(FS_CPU,FM_ANY,0,wbuf,SECTOR_SIZE*sizeof(int));
//								f_mount(NULL,FS_CPU,1);
							}
							break;
							
						default:
							return _PARSE_ERR_ILLEGAL;
						}
						break;
				case 'l':
						Initialize_LED(cc,strscan(++c,cc,','));
						break;
//__________________________________________________querying flash sector_______________
				case 'q':
					n=strscan(++c,cc,',');
					if(n) {
						for(p=(int *)FATFS_ADDRESS; p[512/4] != -1; p=&p[512/4+1]) {
							if(p[512/4] == getHEX(cc[0],EOF)) {
								for(m=0; m<512; m+=16) {
									__print("\r\n");
									for(n=0,c=(char *)p; n<16; ++n)
										__print("%02X ",(~(c[m+n]))&0xff);
									for(n=0; n<16; ++n) {
										if((((~(c[m+n]))&0xff)>0x20) && (((~(c[m+n]))&0xff)<0x7f))
											__print("%c",(~(c[m+n]))&0xff);
										else
											__print(".");
									}
								}
								__print("\r\n----------------------------------------------------------------");
								for(m=-1;m==-1;m=fgetc(&__stdin))
									_proc_loop();
								if(m==0x1b)
									break;
							}
						}
						break;
					} else
					SectorQuery();
				break;
//______________________________________________________________________________________
				case '?':
					n=strscan(++c,cc,',');

					if(n==1 && *cc[0]=='1') {
					ScopeDumpBinary(ADC1_buf,pfm->Burst->Length*_uS / _MAX_ADC_RATE);
						break;
					}
					if(n==1 && *cc[0]=='2') {
					ScopeDumpBinary(ADC2_buf,pfm->Burst->Length*_uS / _MAX_ADC_RATE);
						break;
					}
					if(n==2) {
						ScopeDumpBinary((_ADCDMA *)atoi(cc[0]),atoi(cc[1]));
						break;
					}
					return _PARSE_ERR_SYNTAX;
//__________________________________________________ disable error ___________
				case 'E':
					for(c=strchr(c,' '); c && *c;) {
						pfm->Errmask |= strtoul(++c,&c,16);
						_CLEAR_ERROR(pfm, pfm->Errmask);
					}
					break;
//__________________________________________________ mode setup _____________
				case 'm':
					n=numscan(++c,cc,',');
					while(n--)
						_CLEAR_MODE(pfm,atoi(cc[n]));	
					break;
//__________________________________________________ debug setup _____________
				case 'D':
					__dbug=__stdin.io;
					n=numscan(++c,cc,',');
					while(n--)
						_CLEAR_DBG(pfm,strtol(cc[n],NULL,0));
					break;
//__________________________________________________ delay execution ________
				case 'd':
					if(strscan(c,cc,' ')==2) {
						_wait(atoi(cc[1]),_proc_loop);
					}
					break;
//___________________________________________________________________________
				default:
					return _PARSE_ERR_SYNTAX;
				}
				return _PARSE_OK;
}
//___________________________________________________________________________
//___________________________________________________________________________
//___________________________________________________________________________
//___________________________________________________________________________
//___________________________________________________________________________
//___________________________________________________________________________
//___________________________________________________________________________
//___________________________________________________________________________
//___________________________________________________________________________
//___________________________________________________________________________
//___________________________________________________________________________
//___________________________________________________________________________
//___________________________________________________________________________
//___________________________________________________________________________
//___________________________________________________________________________
//___________________________________________________________________________
//___________________________________________________________________________
//___________________________________________________________________________
//___________________________________________________________________________
//___________________________________________________________________________
//___________________________________________________________________________
//___________________________________________________________________________
//___________________________________________________________________________
int			DecodePlus(char *c) {

				char		*cc[8];
				int			n;
				switch(*c) {
//__________________________________________________ enable error ___________
				case 'E':
					for(c=strchr(c,' '); c && *c;)
						pfm->Errmask &= ~strtoul(++c,&c,16);
					break;
//__________________________________________________ mode setup _____________
				case 'm':
					n=numscan(++c,cc,',');
					while(n--)
						_SET_MODE(pfm,atoi(cc[n]));
					break;
//__________________________________________________ em notification mode ___________
				case 'n':
					for(c=strchr(c,' '); c && *c;)
						pfm->Trigger.enotify = strtoul(++c,&c,16);
					break;
//__________________________________________________ watchdog setup _________
				case 'w':
					if(strscan(++c,cc,','))
						Watchdog_init(atoi(cc[0]));
					else
						Watchdog_init(300);
					break;
//__________________________________________________ debug setup _____________
				case 'D':
					__dbug=__stdin.io;
					n=numscan(++c,cc,',');
					while(n--)
						_SET_DBG(pfm,atoi(cc[n]));
					break;
//__________________________________________________ I2C setup _______________
				case 'i':
					switch(strscan(++c,cc,',')) {
						case 0:
							__charger6=Initialize_I2C(NULL, 0x58,50000);
						break;
						case 1:
							__charger6=Initialize_I2C(NULL, getHEX(cc[0],2), 50000);
						break;
						case 2:
							__charger6=Initialize_I2C(NULL, getHEX(cc[0],2),atoi(cc[1]));
						break;
					}
					break;
//__________________________________________________boot enable____________
				case 'B':
#if defined (_NRST_DISABLE_BIT) && defined (_BOOT_ENABLE_BIT)
					pfm->boot_timeout = atoi(strchr(c,' '));
					if(pfm->boot_timeout)
						pfm->boot_timeout += __time__;
					GPIO_ResetBits(_NRST_DISABLE_PORT,_NRST_DISABLE_BIT);
					GPIO_SetBits(_BOOT_ENABLE_PORT,_BOOT_ENABLE_BIT);
#endif				
					break;
//______________________________________________________________________________________
				default:
					return _PARSE_ERR_SYNTAX;
				}
				return _PARSE_OK;
}
//___________________________________________________________________________
//___________________________________________________________________________
//___________________________________________________________________________
//___________________________________________________________________________
//___________________________________________________________________________
//___________________________________________________________________________
//___________________________________________________________________________
//___________________________________________________________________________
//___________________________________________________________________________
//___________________________________________________________________________
//___________________________________________________________________________
//___________________________________________________________________________
//___________________________________________________________________________
//___________________________________________________________________________
int			DecodeEq(char *c) {
				char		*cc[8];
				int			n;
//______________________________________________________________________________________
				switch(*c) {
//______________________________________________________________________________________
				case 'C':
// scale fakt. za C v mF pri 880V/1100A full scale, 100kHz sample rate in _AVG3 = 4 pride 20... ni placa za izpeljavo
					_TIM.Caps=__max(100,1000.0*atof(++c));	
					__print(" ... bank capacity set to %6.1lf mF",(double)_TIM.Caps/1000.0);
					break;
//______________________________________________________________________________________
				case 'I':					
					_TIM.Icaps=__max(10,1000.0*atof(++c));
					__print(" ... charge current set to %5.1lf A",(double)_TIM.Icaps/1000.0);
					break;
//______________________________________________________________________________________
				case 'E':
					n=strscan(++c,cc,',');
					while(n--)
						_SET_ERROR(pfm,getHEX(cc[n],-1));
					break;
//______________________________________________________________________________________
				default:
					return _PARSE_ERR_SYNTAX;
				}
				return _PARSE_OK;
}
//___________________________________________________________________________
//___________________________________________________________________________
//___________________________________________________________________________
//___________________________________________________________________________
//___________________________________________________________________________
//___________________________________________________________________________
//___________________________________________________________________________
//___________________________________________________________________________
//___________________________________________________________________________
//___________________________________________________________________________
//___________________________________________________________________________
//___________________________________________________________________________
//___________________________________________________________________________
//___________________________________________________________________________
//___________________________________________________________________________
//___________________________________________________________________________
//___________________________________________________________________________
//___________________________________________________________________________
//___________________________________________________________________________
//___________________________________________________________________________
//___________________________________________________________________________
//___________________________________________________________________________
//___________________________________________________________________________
//___________________________________________________________________________
//___________________________________________________________________________
//___________________________________________________________________________
//___________________________________________________________________________
//___________________________________________________________________________
int			DecodeWhat(char *c) {
				int			k,m,n;
				void 		*v;
				switch(*c) {
				case 'f':
					__print("%d,%d\r\n",TIM_GetCapture1(TIM3),TIM_GetCapture2(TIM3));
				break;
//______________________________________________________________________________________
				case 't':
				{
					int p1=0,p2=0,t=0;
					int n1=5*(_TIM.pwch1[p1].n+1),
							n2=5*(_TIM.pwch2[p2].n+1);
					while(_TIM.pwch1[p1].n || _TIM.pwch2[p2].n) {
						int d=__min(n1,n2);
						if(d==0)
							d=__max(n1,n2);
						n1=__max(-1,n1-d);
						n2=__max(-1,n2-d);
						__print("\r\n%4d,%4d,%4d",t,_TIM.pwch1[p1].T,_TIM.pwch2[p2].T);
						t+=d;
						__print("\r\n%4d,%4d,%4d",t,_TIM.pwch1[p1].T,_TIM.pwch2[p2].T);
						if(_TIM.pwch1[p1].n && n1==0)
							n1=5*(_TIM.pwch1[++p1].n+1);
						if(_TIM.pwch2[p2].n && n2==0)
							n2=5*(_TIM.pwch2[++p2].n+1);
					}
					__print("\r\n");
				}
				break;
//______________________________________________________________________________________
				case '#':
					__print("%d",*(int *)ushape);
					for(n=1; ushape[n].T && n<_MAX_USER_SHAPE;++n)
						__print("\r\n#%d,%d",ushape[n].T,ushape[n].U);
					break;
//______________________________________________________________________________________
				case 'm':
					__print(" %08X",pfm->mode);
					break;
//______________________________________________________________________________________
				case 'D':
					if(__dbug)
						__print(" %08X",pfm->debug);
					else
						__print(" ...");
					break;
//______________________________________________________________________________________
				case 'E':
					if(strchr(c,' ') && strchr(c,'m')) {
						__print("\r\nerror mask=%08X",pfm->Errmask);
						__print("\r\ncritical  =%08X",pfm->Errmask & _CRITICAL_ERR_MASK);
					} else {
						k=pfm->Error & ~pfm->Errmask  & _CRITICAL_ERR_MASK;
						for(n=0; k; k >>= 1, ++n)
							if(_BIT(k,0))
								__print("\r\n%06X: %s",1<<n,_errStr[n]);
						}
					break;
//______________________________________________________________________________________
				case 'W':
					k=pfm->Error & ~pfm->Errmask & ~_CRITICAL_ERR_MASK;
					for(n=0; k; k >>= 1, ++n)
						if(_BIT(k,0))
							__print("\r\n%06X: %s",1<<n,_errStr[n]);
					break;
//______________________________________________________________________________________
				case 'P':
					_proc_list();
					break;
//______________________________________________________________________________________
				case 'B':
#if defined (_NRST_DISABLE_BIT) && defined (_BOOT_ENABLE_BIT)
					if(GPIO_ReadOutputDataBit(_BOOT_ENABLE_PORT,_BOOT_ENABLE_BIT)==SET)
						__print (" ..boot enabled");
					else
						__print (" ..boot disabled");
					if(pfm->boot_timeout)
						__print(", timeout=%d ms",pfm->boot_timeout - __time__);
#endif
					break;
//______________________________________________________________________________________
				case 'H':
					k=0;
					while(1) {
						v=malloc(0x400 * k);
						if(!v)
							break;
						free(v);
						++k;
						__print(".");
					}
					m=(int)__heap_limit;
					n=(int)__heap_base;
					__print("\r\n%d of %dk heap left, ",k,(m - n)/0x400);
					m=(int)__initial_sp;
					n=(int)__heap_limit;
					for(k=0; k<(m - n)/sizeof(int); ++k)
						if(__heap_limit[k])
							break;
					__print("\r\n%d of %dk stack clean",k*sizeof(int)/0x400,(m-n)/0x400);					
					break;
//______________________________________________________________________________________
				default:
					return _PARSE_ERR_SYNTAX;
				}
				return _PARSE_OK;
}
//______________________________________________________________________________________
//______________________________________________________________________________________
//______________________________________________________________________________________
//______________________________________________________________________________________
//______________________________________________________________________________________
//______________________________________________________________________________________
//______________________________________________________________________________________
//______________________________________________________________________________________
//______________________________________________________________________________________
int			EnterFile(char *c) {
static
FIL			*f=NULL;																// file object pointer												
				if(!f) {																// first alloc
					f=calloc(1,sizeof(FIL));	
					if(!f)																// mem error, exit
						return _PARSE_ERR_MEM;
					if(f_open(f,c,FA_READ|FA_WRITE|FA_OPEN_ALWAYS)==FR_OK &&
						f_lseek(f,f_size(f))==FR_OK) {			// open & pointer to eof
							fprintf((FILE *)f,"\r");					// init. needed kwdf???
							__stdin.io->arg.parse=EnterFile;
							return _PARSE_OK;									// parser redirect
						} else {
							free(f);													// free & eroor exit othw
							return _PARSE_ERR_OPENFILE;
						}	
				}
				if(!c)																	// eol parser entry...
					__print("\r\n");	
				else
//______________________________________________________________________________________
				switch(*c) {
				case 0x04:
					f_sync(f);														// buffer flush, close file and memfree ...
					f_close(f);
					free(f);
					f=NULL;																// null pointer
					__stdin.io->arg.parse=DecodeFs;				// parser redirect
					break;
//______________________________________________________________________________________
				default:
					fprintf((FILE *)f,"%s\r\n",c);
				}
			return _PARSE_OK;
}
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
//______________________________________________________________________________________
//______________________________________________________________________________________
//______________________________________________________________________________________
//______________________________________________________________________________________
//__list directory______________________________________________________________________
char			*findName(char *p, DIR *dir, FILINFO *fno) {
char			*q,*qq;
					if(!dir->sect && f_readdir(dir,NULL)!=FR_OK)
						return NULL;
					while(1) {
						if(f_readdir(dir,fno) != FR_OK || !dir->sect)
							return NULL;
							q=qq=fno->fname;
						while(*q) {
							if(*p == *q) {
								++p;
								++q;
							} else {
								if(*p == '*') {
									if(!p[1])
										return qq;
									if(p[1]==*q)
										++p;	
									else
										++q;
								} else
									return NULL;
							}
						}
						return qq;
					}
}
//______________________________________________________________________________________
int				DecodeFs(char *c) {
					char	*p,*sc[8];

static 		DIR		dir;
FILINFO		fno;
TCHAR			buf[128];

					if(!c) {
						__print("\r\n");
						if(f_getcwd(buf,_MAX_LFN)==FR_OK && f_opendir(&dir,buf)==FR_OK) {
							if(buf[strlen(buf)-1]=='/')
								__print("%s",buf);
							else
								__print("%s/",buf);
						} else
							__print("?:/"); 		
					} else if(*c) {
//______________________________________________________________________________________
						int n=strscan(c,sc,' ');
						int len=strlen(sc[0]);
//______________________________________________________________________________________
						if(!(strncmp("0:",sc[0],len) && strncmp("1:",sc[0],len) && strncmp("2:",sc[0],len))) {
							if(f_chdrive(c)!=FR_OK ||
								f_mount(pfm->fatfs,c,1)!=FR_OK ||
									f_getcwd(buf,_MAX_LFN)!=FR_OK ||
										f_opendir(&dir,buf)!=FR_OK)
											return _PARSE_ERR_SYNTAX;
							__stdin.io->arg.parse=DecodeFs;
						}
//__delete file________________________________________________________________________
						else if(!strncmp("delete",sc[0],len)) {
							if(n == 2 && f_readdir(&dir,NULL)==FR_OK) {
								while(dir.sect) {
									char *p=findName(sc[1],&dir,&fno);
									if(p && f_unlink(p) != FR_OK)
											return _PARSE_ERR_OPENFILE;
								}
							}	else
								return _PARSE_ERR_SYNTAX;
						}
//__rename file_________________________________________________________________________
						else if(!strncmp("rename",sc[0],len)) {
							if(n == 2)
								return _PARSE_ERR_SYNTAX;
							if(f_rename(sc[1],sc[2]) != FR_OK)
							return _PARSE_ERR_SYNTAX;
						}
//__type file_________________________________________________________________________
						else if(!strncmp("type",sc[0],len)) {
							if(n == 2) {
								FIL	f;
								if(f_open(&f,sc[1],FA_READ)==FR_OK) {
									__print("\r\n");
									while(!f_eof(&f)) 
										__print("%c",f_getc(&f));
									f_close(&f);
								} else
								return _PARSE_ERR_OPENFILE;
							}
						}
//__make directory_____________________________________________________________________
						else if(!strncmp("mkdir",sc[0],len)) {
							if(n == 2) {
								if(f_mkdir(sc[1]) != FR_OK)
									return _PARSE_ERR_OPENFILE;
							} else
								return _PARSE_ERR_MISSING;
						}
//__change directory_____________________________________________________________________
						else if(!strncmp("cdir",sc[0],len)) {
							if(n == 2) {
								if(f_chdir(sc[1]) != FR_OK )
									return _PARSE_ERR_OPENFILE;
							} else
								return _PARSE_ERR_MISSING;
						}
//__copy file_________________________________________________________________________
						else if(!strncmp("copy",sc[0],len)) {
							char f[256];
							FIL f1,f2;
							if(n == 2) {
								p=strchr(sc[1],':');
								if(p++) {
									if(*p=='/')
										++p;
									strcpy(f,p);
								} else
									strcpy(f,sc[1]);
							}
							else
							if(n == 3) {
								strcpy(f,sc[2]);	
							} else
								return _PARSE_ERR_SYNTAX;
							
							if(!strcmp(sc[1],f))
								strcat(f,"_Copy");

							if(f[strlen(f)-1]==':')
								strcat(f,sc[1]);
							if(f_open(&f1,sc[1],FA_READ)!=FR_OK)
								return _PARSE_ERR_OPENFILE;
							if(f_open(&f2,f,FA_CREATE_ALWAYS | FA_WRITE)!=FR_OK) {
								f_close(&f1);
								return _PARSE_ERR_OPENFILE;
							}
							while(!f_eof(&f1))
								if(f_putc(f_getc(&f1),&f2)==EOF)
									break;
							if(!f_eof(&f1)) {
								f_close(&f1);
								f_close(&f2);
								return _PARSE_ERR_OPENFILE;
							}
							f_close(&f1);
							f_close(&f2);
						}

//__entering new file__________________________________________________________________
						else if(!strncmp("file",sc[0],len)) {
							if(n == 2)
								return(EnterFile(sc[1]));
							else
								return _PARSE_ERR_SYNTAX;
						}

//__entering new file__________________________________________________________________
						else if(!strncmp("format",sc[0],len)) {
							char *c,fs[256];
							FIL f1,f2;

							if(n < 3)
								return(_PARSE_ERR_MISSING);
							if(f_open(&f1,sc[1],FA_READ)!=FR_OK)
								return _PARSE_ERR_OPENFILE;
							if(f_open(&f2,sc[2],FA_WRITE | FA_OPEN_ALWAYS)!=FR_OK) {
								f_close(&f1);
								return _PARSE_ERR_OPENFILE;
							};
							
							while(fgets(fs,sizeof(fs),(FILE *)&f1))
								for(c=fs;c < fs + strlen(fs)-2; f_putc(strtol(c,&c,16),&f2));
							
							f_close(&f1);
							f_close(&f2);
							return _PARSE_OK;						
						}

//__list directory______________________________________________________________________
						else if(!strncmp("dir",sc[0],len)) {
							union {
								uint16_t fdate;
								struct { 
									int	day:5;
									int	month:4;
									int	year:7;
								} date;
							} f;
							
							if(n==2)
								if(f_opendir(&dir,sc[1])!=FR_OK || f_readdir(&dir,NULL)!=FR_OK)
									return _PARSE_ERR_OPENFILE;					
							while(1) {
								f_readdir(&dir,&fno);
								if(!dir.sect)
									break;
								__print("\r\n%-28s",fno.fname);
								if (fno.fattrib & AM_DIR)
									__print("%16s","/");
								else
									__print("%16d",(int)fno.fsize);
								f.fdate=fno.fdate;
								__print("   %2d-%2d-%4d",f.date.day, f.date.month, 1980+f.date.year);
							}
						}
//__iap_________________________________________________________________________________
						else if(!strncmp("iap",sc[0],len)) {
							return iapRemote(sc[1]);
						}
//______________________________________________________________________________________
						else if(*sc[0]=='>') {
							__stdin.io->arg.parse=DecodeCom;
							return(DecodeCom(NULL));
						}
//______________________________________________________________________________________
						else return _PARSE_ERR_ILLEGAL;						
		}
		return _PARSE_OK;
}
//______________________________________________________________________________________
//______________________________________________________________________________________
//______________________________________________________________________________________
//______________________________________________________________________________________
//______________________________________________________________________________________
//______________________________________________________________________________________
//______________________________________________________________________________________
//______________________________________________________________________________________
//int		DecodeSpi(char *c) {
//void	*InitializeSpi(void);
//int		ExchgSpi(int, int);		
//int		i,j,k;
//static void
//			*spi=NULL;
//			if(!spi)
//				spi=InitializeSpi();
//			if(!spi) {
//				__print(" not initialized ");
//				return _PARSE_ERR_NORESP;
//			}
//			switch(*c) {
//				case '\x0':
//					break;
//				case ' ':
//					return DecodeSpi(++c);
////________read register_________________________________________________________________
//				case 'r':
//					sscanf(++c,"%X",&i);
//					i=((i<<16) | i) | 0x00a000a0;
//					ExchgSpi(i, 4);
//					_wait(2,_proc_loop);
//					i=ExchgSpi(0x00000000, 4);
//					__print(" >> %02X,%02X",i&0xff,(i>>16)&0xff);
//					break;
////________reset register________________________________________________________________
//				case '!':
//					sscanf(++c,"%X",&i);
//					i=((i<<16) | i) | 0x00c000c0;
//					ExchgSpi(i,4);
//					break;
////________write register________________________________________________________________
//				case 'w':
//					if(sscanf(++c,"%X,%X,%X",&i,&j,&k)==3) {
//						i=((i<<16) | i) | 0x00800080;
//						j=((k<<16) | j);
//					} else {
//						i=((i<<16) | i) | 0x00800080;
//						j=((j<<16) | j);
//					}
//					ExchgSpi(i,4);
//					ExchgSpi(j,4);
//					break;
////______command___________________2w_______________________________________
//				default:
//						sscanf(c,"%X",&i);
//						i=((i<<16) | i);
//						ExchgSpi(i,4);
//						break;
//			}
//			return _PARSE_OK;
//}
//___________________________________________________________________________
int	DecodeCom(char *c) {
		char 		*cc[8];
		int		 	m,n,k;
//__________________________________________________Prompt only response ____
		if(!c)
			__print("\r\n>");
		else
//___________________________________________________________________________
		switch(*c) {
//__________________________________________________SW version query_________
				case 'v':
				case 'V':
					RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_CRC, ENABLE);
					CRC_ResetDR();
					__print(" %d.%02d %s, <%08X>",
						SW_version/100,SW_version%100,
						__DATE__,
							CRC_CalcBlockCRC(__Vectors, (FATFS_ADDRESS-(int)__Vectors)/sizeof(int)));			//crc od vektorjev do zacetka FS
					break;
//__________________________________________________single interger read/write__________
				case 'B':
					n=strscan(++c,cc,',');
					if(n) {
						if(n>1)
							*(char *)strtol(cc[0],NULL,16)=(char)strtol(cc[1],NULL,16);
						else
							__print(",%02X",*(unsigned char *)strtol(cc[0],NULL,16));
						break;
					}
					return _PARSE_ERR_SYNTAX;
//__________________________________________________single interger read/write__________
				case 'W':
					n=strscan(++c,cc,',');
					if(n) {
						if(n>1)
							*(short *)strtol(cc[0],NULL,16)=(short)strtol(cc[1],NULL,16);
						else
							__print(",%04X",*(unsigned short *)strtol(cc[0],NULL,16));
						break;
					}
					return _PARSE_ERR_SYNTAX;
//__________________________________________________single interger read/write__________
				case 'L':
					n=strscan(++c,cc,',');
					if(n) {
						if(n>1)
							*(int *)strtol(cc[0],NULL,16)=(int)strtol(cc[1],NULL,16);
						else
							__print(",%08X",*(unsigned int *)strtol(cc[0],NULL,16));
						break;
					}
					return _PARSE_ERR_SYNTAX;
//__________________________________________________S-records read_____________________
				case 'S':
					return(sLoad(c));
//__________________________________________________S-records/trace dump________________
				case 'D':
					n=strscan(++c,cc,',');
					if(n<2)
						return _PARSE_ERR_SYNTAX;
					if(n==3 && *cc[2]=='i') {
						n = (int)getHEX(cc[1],EOF);
						iDump((int *)getHEX(cc[0],EOF),n);
					}
					else {
						n = (int)getHEX(cc[1],EOF);
						sDump((char *)getHEX(cc[0],EOF),n);
					}
					break;
//__________________________________________________submit CAN message(SYS to PFM)______
				case '.': {
CanTxMsg	tx={_ID_SYS2PFM,0,CAN_ID_STD,CAN_RTR_DATA,0,0,0,0,0,0,0,0,0};
int				n;
					if(*++c) {
						for(n=0; n<strlen(c);++n,++n)
							tx.Data[n/2]=getHEX(&c[n],2);
						tx.DLC=n/2;
						CAN_ITConfig(__CAN__, CAN_IT_FMP0, DISABLE);
						_buffer_push(__can->rx,&tx,sizeof(CanTxMsg));
						CAN_ITConfig(__CAN__, CAN_IT_FMP0, ENABLE);
						CanReply("I",stdin->io);	
					} else
						CanReply("I",NULL);	
					break;
				}
//__________________________________________________submit CAN message(SYS to __)______
				case '<': {
CanRxMsg	buf={0,0,CAN_ID_STD,CAN_RTR_DATA,0,0,0,0,0,0,0,0,0};
					buf.StdId=strtol(++c,&c,16);
					if(!buf.StdId)
						return(_PARSE_ERR_MISSING); 
					do {
						while(*c == ' ') ++c;
						for(buf.DLC=0; *c && buf.DLC < 8; ++buf.DLC)
							buf.Data[buf.DLC]=strtol(c,&c,16);
							CAN_ITConfig(__CAN__, CAN_IT_FMP0, DISABLE);
							_buffer_push(__can->rx,&buf,sizeof(CanTxMsg));
							CAN_ITConfig(__CAN__, CAN_IT_FMP0, ENABLE);
					} while(*c);
					break;
				}				
//__________________________________________________submit CAN message(SYS to __)______
				case '>': {
CanTxMsg	buf={0,0,CAN_ID_STD,CAN_RTR_DATA,0,0,0,0,0,0,0,0,0};			
					buf.StdId=strtol(++c,&c,16);
					if(!buf.StdId)
						return(_PARSE_ERR_MISSING); 
					do {
						while(*c == ' ') ++c;
						for(buf.DLC=0; *c && buf.DLC < 8; ++buf.DLC)
							buf.Data[buf.DLC]=strtol(c,&c,16);
						_buffer_push(__can->tx,&buf,sizeof(CanTxMsg));
					} while(*c);
					break;
				}				
//__________________________________________________i2c read_________________
				case 'r':
					n=strscan(++c,cc,',');
					if(n) {
						if(n>1)
							m=atoi(cc[1]);
						else
							m=1;
						n=getHEX(cc[0],EOF);
						if(n>255)
							n=((n>>8)&0x00ff) | ((n<<8)&0xff00);
						if(!readI2C(__charger6,(char *)(&n),m))
						{
							__print(" ...I2C error(%02X)",_PARSE_ERR_NORESP);
						} else {
							__print(",");
							__print("%0*X",2*m,n & ((1<<(m*8))-1));
						}
						break;
					}
					return _PARSE_ERR_SYNTAX;
//__________________________________________________i2c write_________________
				case 'w':
					n=strscan(++c,cc,',');
					if(n==2) {
						n=getHEX(cc[0],EOF);
						if(n>255) {
							n=((n>>8)&0x00ff) | ((n<<8)&0xff00);
							n|=getHEX(cc[1],EOF)<<16;
						}
						else
							n |= (getHEX(cc[1],EOF)<<8);
						if(!writeI2C(__charger6,(char *)(&n),strlen(cc[0])/2+strlen(cc[1])/2))
							__print(" ...I2C error(%02X)",_PARSE_ERR_NORESP);	
					} else
						return _PARSE_ERR_SYNTAX;
					break;
//_vpis user shape _____________________________________________________________________
				case '#':
					n=strscan(++c,cc,',');
					if(!n) {
						for(n=0; n < _MAX_USER_SHAPE; ++n)
							ushape[n].T=ushape[n].U=0;	
						break;
					}
					if(n==2) {
						for(n=1; n < _MAX_USER_SHAPE; ++n)
							if(ushape[n].T==0) {
								ushape[n].T=__max(10*((atoi(cc[0])+5)/10),0);
								ushape[n].U=__max(atoi(cc[1]),0);
								*(int *)ushape += pow(ushape[n].U,3)*ushape[n].T/400000.0 + 0.5;			// mJ !!!
								break;
							}
						break;
					}
					return _PARSE_ERR_SYNTAX;
//______________________________________________________________________________________
				case 'P':
				case 'p':
					n=numscan(++c,cc,',');
					if(n==0) {
						__print("\r>p(ulse)  T,U           ... %dus,%dV",pfm->Burst->Time,pfm->Burst->PW*_AD2HV(pfm->HVref)/_PWM_RATE_HI);
						if(pfm->Burst->pockels.delay || pfm->Burst->pockels.width)
							__print("\r\n>q(swch)  delay,width   ... %.1fus,%.1fus",(float)pfm->Burst->pockels.delay/10,(float)pfm->Burst->pockels.width/10);
						break;
					}
//__________________________________
					if(n>0)
						pfm->Burst->Time=atoi(cc[0]);
//__________________________________
					if(n>1)
						pfm->Burst->U=10*atof(cc[1]);
//__________________________________
					if(n==3)
						pfm->Burst->Ereq = atoi(cc[2]);
					else if(*--c == 'P')
						pfm->Burst->Ereq=7;
					else
						pfm->Burst->Ereq=1;
//__________________________________
					if(n==4) {																		// dodatek za vnos pockelsa 
						pfm->Burst->pockels.delay=10*atof(cc[2]);					// ndc673476iopj
						pfm->Burst->pockels.width=10*atof(cc[3]);
						PFM_pockels(pfm);
					}
//__________________________________
					SetPwmTab(pfm);
					break;
//______________________________________________________________________________________
				case 'd':
					n=numscan(++c,cc,',');
					if(!n) {
						__print("\r>d(elay)  T,PW          ... %dus,%.2f",pfm->Burst->Delay,(float)pfm->Burst->Pdelay/_PWM_RATE_HI);
						break;
						}
					if(n>0)
						pfm->Burst->Delay=atoi(cc[0]);
					if(n>1)
						pfm->Burst->Pdelay=_PWM_RATE_HI*atof(cc[1]);
					SetPwmTab(pfm);
					break;
//______________________________________________________________________________________
				case 'b':
					n=numscan(++c,cc,',');
					if(n==0) {
						__print("\r>b(urst)  N,len,per     ... %d,%dus,%dms",pfm->Burst->N, pfm->Burst->Length,pfm->Burst->Period);
						break;
					}
					if(n>0 && atoi(cc[0]) > 0)
						pfm->Burst->N=atoi(cc[0]);
					else
						return _PARSE_ERR_ILLEGAL;
					if(n>1)
						pfm->Burst->Length=atoi(cc[1]);
					if(n>2) {
						pfm->Burst->Period=atoi(cc[2]);
					}
					if(n>3) {
						pfm->Trigger.count=atoi(cc[3]);
						_CLEAR_MODE(pfm,_AUTO_TRIGGER);
					}
					else
						pfm->Trigger.count=0;
					SetPwmTab(pfm);
					break;
//______________________________________________________________________________________
				case 'q':
				{
					n=numscan(++c,cc,',');
					if(!n) {
							__print("\r>q(shape) w,U,td,to,tref ...");
							for(k=0; qshape[k].qref && k<_MAX_QSHAPE; ++k)
								__print("\r\n%d,%d,%d,%d,%d",				qshape[k].q0,
																										(qshape[k].q1*_AD2HV(pfm->HVref) + _PWM_RATE_HI/2)/_PWM_RATE_HI,
																										qshape[k].q2,
																										qshape[k].q3,
																										qshape[k].qref);		
					break;
					} 
					if(n==5) {
						for(k=0; qshape[k].qref && qshape[k].qref != atoi(cc[4]); ++k);
						qshape[k].q0=atoi(cc[0]);
						qshape[k].q1=(_PWM_RATE_HI*atoi(cc[1]))/_AD2HV(pfm->HVref);
						qshape[k].q2=atoi(cc[2]);
						qshape[k].q3=atoi(cc[3]);
						qshape[k].qref=atoi(cc[4]);	
					}
					else
						return _PARSE_ERR_SYNTAX; 
				}
					break;
//______________________________________________________________________________________
				case 's':
					switch(numscan(++c,cc,',')) {
						case 0:
							__print("\r>s(immer) n // t1,t2,f1,f2..%dns,%dns,%dus,%dus",
									(int)(1000*pfm->Simmer.pw[0])/_uS,
										(int)(1000*pfm->Simmer.pw[1])/_uS,
											pfm->Simmer.rate[0]/_uS,
												pfm->Simmer.rate[1]/_uS);
							break;
						case 1:
							PFM_command(pfm,atoi(cc[0]) & 0x3);
							break;
						case 2:
							pfm->Simmer.pw[0]=atoi(cc[0])*_uS/1000;
							pfm->Simmer.pw[1]=atoi(cc[1])*_uS/1000;
							SetSimmerRate(pfm, _SIMMER_LOW);
							break;
						case 3:
							pfm->Simmer.pw[0]=atoi(cc[0])*_uS/1000;
							pfm->Simmer.pw[1]=atoi(cc[1])*_uS/1000;
							if(atoi(cc[2])<10 || atoi(cc[2])>100)
								return _PARSE_ERR_ILLEGAL;
							pfm->Simmer.rate[0]=pfm->Simmer.rate[1]=atoi(cc[2])*_uS;
							SetSimmerRate(pfm,_SIMMER_LOW);		
							break;
						case 4:																																					// #kerer734hf
							if(!_MODE(pfm,_CHANNEL1_DISABLE) && !_MODE(pfm,_CHANNEL2_DISABLE))						// 4 parameter not allowed in double channel cfg !!!
									return _PARSE_ERR_SYNTAX;
							if(atoi(cc[0])> 500 || atoi(cc[1])> 500 || atoi(cc[2])<10 || atoi(cc[2])>100 || atoi(cc[3])<10 || atoi(cc[3])>100)
								return _PARSE_ERR_ILLEGAL;
							pfm->Simmer.pw[0]=atoi(cc[0])*_uS/1000;
							pfm->Simmer.pw[1]=atoi(cc[1])*_uS/1000;
							pfm->Simmer.rate[0]=atoi(cc[2])*_uS;
							pfm->Simmer.rate[1]=atoi(cc[3])*_uS;
							SetSimmerRate(pfm,_SIMMER_LOW);		
							break;
						default:
							return _PARSE_ERR_SYNTAX;
					}
					break;
//______________________________________________________________________________________
				case 'i':							
					switch(numscan(++c,cc,',')) {
						case 0:
							__print("\r\n");
							__print("current limits(l/h)         ... %dA,%dA,%dA\r\n",_AD2I(pfm->Burst->max[0]),_AD2I(pfm->Burst->max[1]),_AD2I(pfm->Simmer.max));
							__print("voltage limits(l/h)         ... %dV,%dV\r\n",_AVG3*_AD2HV(ADC3->LTR),_AVG3*_AD2HV(ADC3->HTR));
						break;
						case 3:	
							pfm->Simmer.max=_I2AD(atoi(cc[2]));
						case 2:
							pfm->Burst->max[0]=_I2AD(atoi(cc[0]));
							pfm->Burst->max[1]=_I2AD(atoi(cc[1]));
							break;
						default:
							return _PARSE_ERR_SYNTAX;
					}
				break;
//______________________________________________________________________________________
				case 'I':							
					switch(strscan(++c,cc,',')) {
						case 0:
							__print("\r>i(DAC)   i1,i2         ... %d%c,%d%c",(DAC_GetDataOutputValue(DAC_Channel_1)*100+0x7ff)/0xfff,'%',(DAC_GetDataOutputValue(DAC_Channel_2)*100+0x7ff)/0xfff,'%');
							break;
						case 2:
							DAC_SetDualChannelData(DAC_Align_12b_R,(atoi(cc[1])*0xfff+50)/100,(atoi(cc[0])*0xfff+50)/100);
							DAC_DualSoftwareTriggerCmd(ENABLE);		
							break;
						default:
							return _PARSE_ERR_SYNTAX;
					}
				break;
//______________________________________________________________________________________
				case 'a':
					switch(numscan(++c,cc,',')) {
						case 0:
							__print("  \r>a(dc)    U1,I1,U2,I2   ... %dV,%dA,%dV,%dA",_AD2HV(_AVG3*ADC1_simmer.U),_AD2I(ADC1_simmer.I-_TIM.I1off),
																																					_AD2HV(_AVG3*ADC2_simmer.U),_AD2I(ADC2_simmer.I-_TIM.I2off));
							__print("\n\r>a(dc)    idle          ... %dV,%dA,%dV,%dA",_AD2HV(_AVG3*_TIM.U1off),_AD2I(_TIM.I1off),_AD2HV(_AVG3*_TIM.U2off),_AD2I(_TIM.I2off));
							break;
						default:
							return _PARSE_ERR_SYNTAX;
					}
					break;	
//__________________________________________________ events trigger ___________________
				case 'e':
					n=numscan(++c,cc,',');
					while(n--)
						_SET_EVENT(pfm,atoi(cc[n]));
					break;
//______________________________________________________________________________________
				case ':':
					break;
//______________________________________________________________________________________
				case 'x':
					switch(atoi(++c)) {
					case 1:
						pfm->Simmer.mode=_XLAP_SINGLE;
						break;
					case 2:
						pfm->Simmer.mode=_XLAP_DOUBLE;
						break;
					case 4:
						pfm->Simmer.mode=_XLAP_QUAD;
						break;
					default:
						return _PARSE_ERR_SYNTAX;
					}
					SetSimmerRate(pfm,_SIMMER_LOW);
					break;
//______________________________________________________________________________________
				case 'X':
					switch(atoi(++c)) {
					case 1:
						pfm->Burst->Mode=_XLAP_SINGLE;
						break;
					case 2:
						pfm->Burst->Mode=_XLAP_DOUBLE;
						break;
					case 4:
						pfm->Burst->Mode=_XLAP_QUAD;
						break;
					default:
						return _PARSE_ERR_SYNTAX;
					}
					break;
//______________________________________________________________________________________
				case 't':
				{
#if defined (__F2__) || defined (__F4__) 
					short *t30	=(short *)0x1FFF7A2C;
					short *t110	=(short *)0x1FFF7A2E;
#endif
#ifdef __F7__
					short *t30	=(short *)0x1FF0F44C ;
					short *t110	=(short *)0x1FF0F44E;
#endif
					ADC_SoftwareStartInjectedConv(ADC1);
					_wait(2,_proc_loop);
					__print(" %d, %d",(110-30)*(ADC_GetInjectedConversionValue(ADC1, ADC_InjectedChannel_1) - *t30)/(*t110 - *t30) + 30,
						(ADC_GetInjectedConversionValue(ADC1, ADC_InjectedChannel_1)*3300/4096-760)*4/10+25
					);
				}
					break;
//______________________________________________________________________________________
				case 'f':
					n=numscan(++c,cc,',');
					if(!n) {
						if(_MODE(pfm,_CHANNEL1_DISABLE))		
							__print("\r>f(an)   Tl,Th,min,max,T... %d,%3d,%3d%c,%3d%c, -- ,%5.1f",fanTL/100,fanTH/100,fanPmin,'%',fanPmax,'%',(double)IgbtTemp(TH2)/100.0);
						else if(_MODE(pfm,_CHANNEL2_DISABLE))		
							__print("\r>f(an)   Tl,Th,min,max,T... %d,%3d,%3d%c,%3d%c,%5.1f, -- ",fanTL/100,fanTH/100,fanPmin,'%',fanPmax,'%',(double)IgbtTemp(TH1)/100.0);
						else	
#ifdef	__PFM6__
							__print("\r>f(an)   Tl,Th,min,max,T... %d,%3d,%3d%c,%3d%c,%5.1f,%5.1f",fanTL/100,fanTH/100,fanPmin,'%',fanPmax,'%',(double)IgbtTemp(TH1)/100.0,(double)IgbtTemp(TH2)/100.0);
#elif		__PFM8__
//							__print("\r>f(an)   Tl,Th,min,max,T... %d,%3d,%3d%c,%3d%c,%5.1f,%5.1f,%5.1f,%5.1f",fanTL/100,fanTH/100,fanPmin,'%',fanPmax,'%',
//								(double)IgbtTemp(TL1)/100.0,(double)IgbtTemp(TH1)/100.0,(double)IgbtTemp(TL2)/100.0,(double)IgbtTemp(TH2)/100.0);
							__print("\r>f(an)   Tl,Th,min,max,T... %d,%3d,%3d%c,%3d%c,%5.1f,%5.1f",fanTL/100,fanTH/100,fanPmin,'%',fanPmax,'%',(double)IgbtTemp(TH1)/100.0,(double)IgbtTemp(TH2)/100.0);
#else
*** error, define platform
#endif
						break;
					} else {
						if(n==4) {
							fanTL=100*atoi(cc[0]);
							fanTH=100*atoi(cc[1]);
							fanPmin=atoi(cc[2]);
							fanPmax=atoi(cc[3]);
							break;
						}
					}
					return _PARSE_ERR_SYNTAX;
//______________________________________________________________________________________
				case 'u':
{
int			u=0,umax=0,umin=0;
					switch(numscan(++c,cc,',')) {
						case 0:
#ifdef	__PFM8__
							__print("(bank) HV,HV/2,Vc1,Vc2... %.0fV,%5.0fV,%5.0fV,%5.0fV\r\n",_AD2V(ADC3_buf[0].HV,2000.0,8.0),
																																								_AD2V(ADC3_buf[0].HV2,2000.0,8.0),
																																								_AD2V(ADC3_buf[0].VCAP1,2000.0,8.0),
																																								_AD2V(ADC3_buf[0].VCAP2,2000.0,8.0));
							__print("         V24,V5,V3      ... %.1fV,%5.1fV,%5.1fV",				_AD2V(ADC3_buf[0].Up12,7500.0,560.0),
																																								_AD2V(ADC3_buf[0].Up5,560.0,560.0),
																																								_AD2V(ADC3_buf[0].Up3,560.0,560.0));
#elif 	__PFM6__
							__print("(bank)  Uc,Uc/2,20,-5 ... %.0fV,%5.0fV,%5.1fV,%5.1fV",	_AD2V(ADC3_buf[0].HV,2000,7.5),
																																								_AD2V(ADC3_buf[0].HV2,1000,7.5),
																																								_AD2V(ADC3_buf[0].Up20,68,12),
																																								_AD2Vn(ADC3_buf[0].Um5,24,12));
#else
*** error, define platform
#endif
							return _PARSE_OK;
						case 3:
							umax=atoi(cc[2]);
						case 2:
							umin=atoi(cc[1]);
						case 1:
							u=atoi(cc[0]);
							break;
						default:
							return _PARSE_ERR_ILLEGAL;
					}
					if(!umax)
						umax=u+u/10;
					if(!umin)
						umin=u-u/3;
					if(u>800 || u<0 || umin>=u || umax<=u)										
						return _PARSE_ERR_ILLEGAL;
					pfm->HVref=_HV2AD(u);
						
					ADC_ITConfig(ADC3,ADC_IT_AWD,DISABLE);
					ADC_AnalogWatchdogThresholdsConfig(ADC3,_HV2AD(umax)/_AVG3,_HV2AD(umin)/_AVG3);
					return SetChargerVoltage(u);
}
//______________________________________________________________________________________
				case 'U':
					m=_READ_VIN;
					if(readI2C(__charger6,(char *)&m,2))	{
						__print("\r>Vin=%.1f, ",__lin2f(m));
						m=_READ_VOUT;
						if(readI2C(__charger6,(char *)&m,2)) {
							__print("Vout=%.1f, ",__lin2f(m));
							m=_READ_PIN;
							if(readI2C(__charger6,(char *)&m,2)) {
								__print("Pout=%.1f",__lin2f(m));
								break;
							}
						}
					}
					return _PARSE_ERR_NORESP;
//______________________________________________________________________________________
				case '!':
					CanReply("wwwwX",0xC101,pfm->Simmer.active,40000,pfm->Burst->Time,_ID_SYS2ENRG);
					_wait(20,_proc_loop);
					CanReply("X",0x1A,_ID_SYS2ENRG);
					_SET_EVENT(pfm,_TRIGGER);
				break;
//______________________________________________________________________________________
				case '$':
				{
					int n=atoi(++c);
					int i=0,U=0,I=0;
					CanReply("I",NULL);
					__print(",%d\r\n",_TIM.eint);		
					while(i < __min(_TIM.eint*_uS/_MAX_ADC_RATE-1,_MAX_BURST/_uS-1)) {
						if(n==1) {
							U+=ADC1_buf[i].U;
							I+=ADC1_buf[i].I;
						}
						if(n==2) {
							U+=ADC2_buf[i].U;
							I+=ADC2_buf[i].I;
						}
						if(++i % 4 == 0) {
							U/=4;
							I/=4;
							while(fputc(U%256,stdout)==EOF)
								_wait(2,_proc_loop);
							while(fputc(U/256,stdout)==EOF)
								_wait(2,_proc_loop);
							while(fputc(I%256,stdout)==EOF)
								_wait(2,_proc_loop);
							while(fputc(I/256,stdout)==EOF)
								_wait(2,_proc_loop);
							U=I=0;
						}
					}
					CanReply("I",stdin->io);
				}
				break;
//______________________________________________________________________________________
				case '@':
					return batch(++c);
//______________________________________________________________________________________
				case '-':
					return DecodeMinus(++c);
//______________________________________________________________________________________
				case '+':
					return DecodePlus(++c);
//______________________________________________________________________________________
				case '?':
					return DecodeWhat(++c);
//______________________________________________________________________________________
				case '=':
					return DecodeEq(++c);
//______________________________________________________________________________________
				case '0':
				case '1':
				case '2':
					return DecodeFs(c);
//_______________________________________________________________________________________
				default:
					return _PARSE_ERR_SYNTAX;
				}
			return _PARSE_OK;
}
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
//______________________________________________________________________________________
//______________________________________________________________________________________
//______________________________________________________________________________________
#define			_ID_IAP_GO			0xA0
#define			_ID_IAP_ERASE		0xA1
#define			_ID_IAP_ADDRESS	0xA2
#define			_ID_IAP_DWORD		0xA3
#define			_ID_IAP_ACK			0xA4
#define			_ID_IAP_SIGN		0xA5
#define			_ID_IAP_STRING	0xA6
#define			_ID_IAP_PING		0xA7
/*******************************************************************************
* Function Name  : AckWait
* Description    : Stop the main CAN RX process and wait for the remote iap ack 
* Input          : CAN TX msg, waiting time (ms) 
* Return         : IAP message or EOF (not successful...)
*******************************************************************************/
int					AckWait(CanTxMsg *tx, int t) {
						int to;
						_proc	*p= _proc_find(ParseCanRx,pfm);	
						CanRxMsg	rx={0,0,CAN_ID_STD,CAN_RTR_DATA,0,0,0,0,0,0,0,0,0};

						p->t = __time__ + 1000; 												// zakasni CAN RX process za sekundo
						to = __time__ + t;															// nastavi cakalni nterval
						_buffer_push(__can->tx,tx,sizeof(CanTxMsg));
						if(t == 0)
							return 0;
						while(__time__ < to)
							if(_buffer_pull(__can->rx,&rx,sizeof(CanRxMsg)) && rx.StdId == _ID_IAP_ACK) {
								return rx.Data[0];
							}	else
								_proc_loop();
						return EOF;
}
/*******************************************************************************
* Function Name  : HexChecksumError
* Description    : preverja konsistentnost vrstice iz hex fila
* Input          : pointer na string 
* Return         : 0 ce je OK
*******************************************************************************/
int					HexChecksumError(char *p) {
int	 				err,n=str2hex(&p,2);
						for(err=n;n-->-5;err+=str2hex(&p,2));
						return(err & 0xff);
}
/*******************************************************************************
* Function Name  : CanHexProg request, server
* Description    : dekodira in razbije vrstice hex fila na 	pakete 8 bytov in jih
*								 : pošlje na CAN bootloader
* Input          : pointer na string, zaporedne vrstice hex fila, <cr> <lf> ali <null> niso nujni
* Output         : 
* Return         : 0 ce je checksum error sicer eof(-1). bootloader asinhrono odgovarja z ACK message
*				 				 : za vsakih 8 bytov !!!
*******************************************************************************/
int					CanHexProg(char *p) {
CanTxMsg		tx={0,0,CAN_ID_STD,CAN_RTR_DATA,0,0,0,0,0,0,0,0,0};
static int	ExtAddr=0;
int	 				n,a,i=FLASH_COMPLETE;
int					*d=(int *)tx.Data;

						if(HexChecksumError(++p))
							return 0;
						n=str2hex(&p,2);
						a=(ExtAddr<<16)+str2hex(&p,4);
						switch(str2hex(&p,2)) {
							case 00:
								if(a<_FLASH_TOP)
									return 0;
								tx.StdId=_ID_IAP_ADDRESS;
								d[0]=a;
								tx.DLC=sizeof(int);
								AckWait(&tx,0);

								tx.StdId=_ID_IAP_DWORD;
								for(i=0; n--;) {
									tx.Data[i++]=str2hex(&p,2);
									if(i==8 || !n) {	
										tx.DLC=i;
										i=0;
										if(AckWait(&tx,100) != 0)
											return 0;
									}	
								}
								break;
							case 01:
								break;
							case 02:
								break;
							case 04:
							case 05:
								ExtAddr=str2hex(&p,4);
								break;
						}
						return(EOF);
}
/*******************************************************************************
* Function Name  : CanHexProg request, server
* Description    : dekodira in razbije vrstice hex fila na 	pakete 8 bytov in jih
*								 : pošlje na CAN bootloader
* Input          : pointer na string, zaporedne vrstice hex fila, <cr> <lf> ali <null> niso nujni
* Output         : 
* Return         : 0 ce je checksum error sicer eof(-1). bootloader asinhrono odgovarja z ACK message
*				 				 : za vsakih 8 bytov !!!
*******************************************************************************/
FRESULT			iapRemote(char * filename) {
						FIL	f;																					// hex file object
						TCHAR hexline[128];															// line buffer
						int n,k;																				// misc
						CanTxMsg tx ={0,0,CAN_ID_STD,CAN_RTR_DATA,0,0,0,0,0,0,0,0,0};		

						FRESULT err= f_open(&f,filename,FA_READ);				// open file
						if(err)
							return err;																		// return error code if not succ..
						for(n=0; !f_eof(&f); ++n) {											// count lines >>>> n
							f_gets(hexline,sizeof(hexline),&f);
							Watchdog();
						}
						f_close(&f);
						
						canFilterConfig(_ID_IAP_GO, 0x7f0);							// config. can filters for IAP protocol
						
						tx.StdId=_ID_SYS2PFM;														// send iap req. as from sys
						tx.Data[0]=_PFM_Iap;
						tx.DLC=1;
						if(AckWait(&tx,3000) != 0) {										// wait for ping
							tx.StdId=_ID_IAP_PING;												// send ping
							tx.DLC=0;
							if(AckWait(&tx,100) != 0)											// wait for response '-'
								return FR_NOT_READY;
						}
						__print("\r\niap ping received...");
						__print("\r\nerasing");													// erase 5 pages
						for(k=FLASH_Sector_1; k<FLASH_Sector_6; k+=FLASH_Sector_1) {
							tx.StdId=_ID_IAP_ERASE;
							tx.DLC=sizeof(int);
							*(int *)tx.Data=k;
							if(AckWait(&tx,2000) != 0)										// send erase page, wait for ack
								return FR_NOT_READY;
							__print(".");
						}
						
						err= f_open(&f,filename,FA_READ);								// open file again...
						__print("\r\nprogramming");
						for(k=0; (!f_eof(&f)); ++k) {
							f_gets(hexline,sizeof(hexline),&f);						// read line
							if(CanHexProg(hexline) == 0)									// send prog. message
								return FR_NOT_READY;
							if((k % (n/20)) == 0)													// echo after 20 lines...
								__print(".%3d%c%c%c%c%c",100*k/n,'%','\x8','\x8','\x8','\x8');
						}
						
						tx.StdId=_ID_IAP_SIGN;													// send sign command
						tx.DLC=0;
						if(AckWait(&tx,300) != 0)												// wait for ack...
							return FR_NOT_READY;
						__print("\r\nsign ...");
						tx.StdId=_ID_IAP_GO;														// send <go> command
						tx.DLC=0;
						AckWait(&tx,0);
						f_close(&f);
						__print("and RUN :)");
						return FR_OK;
}
/*******************************************************************************
* Function Name  : CanHexProg request, server
* Description    : dekodira in razbije vrstice hex fila na 	pakete 8 bytov in jih
*								 : pošlje na CAN bootloader
* Input          : pointer na string, zaporedne vrstice hex fila, <cr> <lf> ali <null> niso nujni
* Output         : 
* Return         : 0 ce je checksum error sicer eof(-1). bootloader asinhrono odgovarja z ACK message
*				 				 : za vsakih 8 bytov !!!
*******************************************************************************/
int					USBH_Iap(int call) {	
FATFS				fs0,fs1;
DIR					dir;
FIL					f0,f1;
BYTE 				buffer[128];   																										//	file copy buffer */
FRESULT 		fr;          																											//	FatFs function common result code	*/
UINT 				br, bw;         																									//	File read/write count */
FILINFO			fno;
TCHAR				*t;
static
int					state=0;

						if(state==0 && call==0) {
							++state;
							Watchdog_init(4000);
							_RED1(0);_GREEN1(0);_BLUE1(0);_YELLOW1(0);
							if(f_mount(&fs0,FS_USB,1)==FR_OK)																// mount usb 
								if(f_mount(&fs1,FS_CPU,1)==FR_OK)															// mount flash
									if(f_chdrive(FS_USB)== FR_OK)																// go to usb drive
										if(f_chdir("/sync")==FR_OK)																// goto /sync directory
											if(f_opendir(&dir,fno.fname)==FR_OK) {									// & open it !
												while(f_readdir(&dir,&fno)==FR_OK && dir.sect) {			// scan the files, if end, exit
													if (fno.fattrib & AM_DIR)														// skip if it is a subdirectory 
														continue;
													t = *fno.fname ? fno.fname : fno.fname;							// check for long filenames
													
													if(f_chdrive(FS_USB)== FR_OK && f_open(&f0,t,FA_OPEN_EXISTING | FA_READ)!=FR_OK) 
														continue;
													if(f_chdrive(FS_CPU)== FR_OK && f_open(&f1,t,FA_CREATE_ALWAYS | FA_WRITE)!=FR_OK) 
														continue;												
													for (;;) {
														if((__time__ / 100) % 2)
															_YELLOW1(1000);
														else
															_YELLOW1(0);															
														fr = f_read(&f0, buffer, sizeof buffer, &br);			/* Read a chunk of source file */
														if (fr || br == 0) break; 												/* error or eof */
														fr = f_write(&f1, buffer, br, &bw);								/* Write it to the destination file */
														if (fr || bw < br) break;													/* error or disk full */
														Watchdog();
													}
													++state;
													f_close(&f0);																				// close both files
													f_close(&f1);	
													f_utime(t,&fno);
												}
//							f_mount(NULL,FS_USB,1);																				// dismount both drives
//							f_mount(NULL,FS_CPU,1);
							}
							if(state>1) {
								_GREEN1(3000);
							} else {
								_RED1(3000);
							}
						}
						
						if(call==EOF) {
							if(state>1)
								WWDG_init();
							state=0;
						}
						return(0);
					}
/*******************************************************************************
* Function Name  : CanHexProg request, server
* Description    : dekodira in razbije vrstice hex fila na 	pakete 8 bytov in jih
*								 : pošlje na CAN bootloader
* Input          : pointer na string, zaporedne vrstice hex fila, <cr> <lf> ali <null> niso nujni
* Output         : 
* Return         : 0 ce je checksum error sicer eof(-1). bootloader asinhrono odgovarja z ACK message
*				 				 : za vsakih 8 bytov !!!
*******************************************************************************/
FRESULT			HexBinWrite(char *p, FIL *f) {
static int	extAddr=0;
uint32_t		n,a,i,j;
FRESULT			fr=FR_OK;

						if(HexChecksumError(++p))
							return FR_INVALID_PARAMETER;
						n=str2hex(&p,2);
						a=(extAddr<<16)+str2hex(&p,4);
						switch(str2hex(&p,2)) {
							case 00:
								if(a<_FLASH_TOP)
									return FR_INVALID_PARAMETER;
								f_write(f, &a, sizeof(uint32_t), &j);
								f_write(f, &n, sizeof(uint8_t), &j);
								while(fr==FR_OK && n--) {
									i=str2hex(&p,2);
									fr=f_write(f, &i, sizeof(uint8_t), &j);
									++p;++p;
								}	
								break;
							case 01:
								break;
							case 02:
								break;
							case 04:
							case 05:
								extAddr=str2hex(&p,4);
								break;
						}
						return(fr);
}
/*******************************************************************************
* Function Name  : CanHexProg request, server
* Description    : dekodira in razbije vrstice hex fila na 	pakete 8 bytov in jih
*								 : pošlje na CAN bootloader
* Input          : pointer na string, zaporedne vrstice hex fila, <cr> <lf> ali <null> niso nujni
* Output         : 
* Return         : 0 ce je checksum error sicer eof(-1). bootloader asinhrono odgovarja z ACK message
*				 				 : za vsakih 8 bytov !!!
*******************************************************************************/
FRESULT			USBH_IapBin(void) {	
FATFS				fs0,fs1;
DIR					dir;
FIL					f0,f1;
TCHAR 			buffer[128];   																										//	file copy buffer */
FRESULT 		fr=FR_OK;          																								//	FatFs function common result code	*/
UINT 				br, bw;         																									//	File read/write count */
FILINFO			fno;
	
						if(f_mount(&fs0,"0:",1) ||
							f_mount(&fs1,"1:",1) ||
							f_chdrive("1:") || 
							f_chdir("/sync") ||
							f_findfirst(&dir,&fno,"/sync","*")) 
								return FR_DISK_ERR;
						
						do {
							if (fno.fattrib & AM_DIR)
								continue;
							fr=FR_DISK_ERR;
							if(f_chdrive("1:") || f_open(&f1,fno.fname,FA_OPEN_EXISTING | FA_READ))
								break;
							if(!strcmp(strchr(fno.fname,'.')+1,"hex")) {
								strcpy(strchr(fno.fname,'.')+1,"bin");
								if(f_chdrive("0:") || f_open(&f0,fno.fname,FA_CREATE_ALWAYS | FA_WRITE))
									break;
								do {
									if(f_gets(buffer,128,&f1))
										fr=HexBinWrite(buffer,&f0);
								} while(!f_eof(&f1) && fr==FR_OK);
							} else {
								if(f_chdrive("0:") || f_open(&f0,fno.fname,FA_CREATE_ALWAYS | FA_WRITE))
									break;
								for (;;) {
									fr = f_read(&f1, buffer, sizeof buffer, &br);	
									if (fr || br == 0) break; 
									fr = f_write(&f0, buffer, br, &bw);
									if (fr || bw < br) break;
									Watchdog();
								}
							}
							f_close(&f0);
							f_close(&f1);
						}  while(f_findnext(&dir,&fno) == FR_OK && *fno.fname);
						f_close(&f0);
						f_close(&f1);
						return(fr);
}
//__________________________________________________________________________________
// hard fault handler in C,
// with stack frame location as input parameter
// called from HardFault_Handler in file eMac.s
//__________________________________________________________________________________
void hard_fault_handler_c (unsigned int * hardfault_args)
{
  unsigned int stacked_r0;
  unsigned int stacked_r1;
  unsigned int stacked_r2;
  unsigned int stacked_r3;
  unsigned int stacked_r12;
  unsigned int stacked_lr;
  unsigned int stacked_pc;
  unsigned int stacked_psr;
	FIL f;

  stacked_r0 = ((unsigned long) hardfault_args[0]);
  stacked_r1 = ((unsigned long) hardfault_args[1]);
  stacked_r2 = ((unsigned long) hardfault_args[2]);
  stacked_r3 = ((unsigned long) hardfault_args[3]);
 
  stacked_r12 = ((unsigned long) hardfault_args[4]);
  stacked_lr = ((unsigned long) hardfault_args[5]);
  stacked_pc = ((unsigned long) hardfault_args[6]);
  stacked_psr = ((unsigned long) hardfault_args[7]);
 
	if(f_open(&f,"fault",FA_OPEN_APPEND | FA_WRITE)==FR_OK) {
	//	f_open(&f,"fault",FA_CREATE_ALWAYS | FA_WRITE);
		f_printf(&f,"\n\r\n\r[Hard fault handler - all numbers in hex]\n\r");
		f_printf(&f,"R0 = %x\n\r", stacked_r0);
		f_printf(&f,"R1 = %x\n\r", stacked_r1);
		f_printf(&f,"R2 = %x\n\r", stacked_r2);
		f_printf(&f,"R3 = %x\n\r", stacked_r3);
		f_printf(&f,"R12 = %x\n\r", stacked_r12);
		f_printf(&f,"LR [R14] = %x  subroutine call return address\n\r", stacked_lr);
		f_printf(&f,"PC [R15] = %x  program counter\n\r", stacked_pc);
		f_printf(&f,"PSR = %x\n\r", stacked_psr);
		f_printf(&f,"BFAR = %x\n\r", (*((volatile unsigned long *)(0xE000ED38))));
		f_printf(&f,"CFSR = %x\n\r", (*((volatile unsigned long *)(0xE000ED28))));
		f_printf(&f,"HFSR = %x\n\r", (*((volatile unsigned long *)(0xE000ED2C))));
		f_printf(&f,"DFSR = %x\n\r", (*((volatile unsigned long *)(0xE000ED30))));
		f_printf(&f,"AFSR = %x\n\r", (*((volatile unsigned long *)(0xE000ED3C))));
		f_printf(&f,"SCB_SHCSR = %x\n\r", SCB->SHCSR);
		f_close(&f);
	}
	NVIC_SystemReset();
}

/**
* @}
*/
