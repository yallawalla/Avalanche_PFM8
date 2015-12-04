/**
  ******************************************************************************
  * @file    io.c
  * @author  Fotona d.d.
  * @version V1
  * @date    30-Sept-2013
  * @brief	 System I/O
  *
  */ 
/** @addtogroup PFM6_Misc
* @brief PFM6 miscellaneous
* @{
*/
#include 	<stdio.h>
#include 	<stdlib.h>
#include 	<string.h>
#include 	"io.h"
//______________________________________________________________________________________
//
//	struct _buffer
//																											,------------------<<push *
//					_buf  *				OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO| 	max. len
//							_pull * <<-------------'															  			 
//______________________________________________________________________________________
// buffer initialization
//
_buffer	*_buffer_init(int length) {
				if(length>0) {
					_buffer	*p=calloc(1,sizeof(_buffer));
					if(p) {
						p->_buf=p->_push=p->_pull=calloc(length,sizeof(char));
						p->len=length;
						if(p->_buf)
							return(p);
						free(p);
					}
				}
				return(NULL);
}
// buffer kill
//______________________________________________________________________________________
_buffer	*_buffer_close(_buffer	*p) {
				if(p) {
					free(p->_buf);
					free(p);
				}
				return NULL;
}
//
//			return number of succesfuly pushed items from q[] into buffer
//______________________________________________________________________________________
int			_buffer_push(_buffer *p, void *q, int n) {
char		*r=q, *t=p->_push;
int			i;
				for(i=0; i<n; ++i) {
					if((int)p->_pull - (int)t == 1)
						break;
					if((int)t - (int)p->_pull == p->len-1)
						break;
					*t++ = *r++;
					if(t == &p->_buf[p->len])
						t = p->_buf;
				}
				p->_push=t;
				return(i);
}
//
//			return number of succesfuly pulled items from buffer onto q[]
//______________________________________________________________________________________
int			_buffer_pull(_buffer *p, void *q, int n) {
int			i=0;
char		*t=p->_pull,
				*r=(char *)q;
				while(n-- && t != p->_push) {
					r[i++] = *t++;
					if(t == &p->_buf[p->len])
						t=p->_buf;
				}
				p->_pull=t;
				return(i);				
}
//___test empty_________________________________________________________________________
int			_buffer_empty	(_buffer *p) {
				if(p->_pull != p->_push)
					return(0);
				else
					return(EOF);
}
//______________________________________________________________________________________
int			_buffer_len	(_buffer *p) {
				if(p) {
					if(p->_pull <= p->_push)
						return((int)p->_push - (int)p->_pull);
					else
						return(p->len - (int)p->_pull + (int)p->_push);
				}
				return(0);
}
//______________________________________________________________________________________
// struct _io
//
// buffer --  rx					.. input buffer
//						tx					.. output buffer
//						put()				.. output API
//						get()				.. input API
//						gets				.. command line buffer
//						parse				.. command line parser
//______________________________________________________________________________________
//
//	stdin prototype
//
int			__getIO (_buffer *p) {
int			i=0;
				if(_buffer_pull(p,&i,1))
					return i;
				else
					return EOF;
}
//______________________________________________________________________________________
//
//	stdout prototype
//
int			__putIO (_buffer *p, int c) {
				return(_buffer_push(p,&c,1));
}
//
//
//	io port init instance
//______________________________________________________________________________________
_io			*_io_init(int rxl, int txl) {
_io			*p=calloc(1,sizeof(_io));
				if(p) {
					p->rx=_buffer_init(rxl);
					p->tx=_buffer_init(txl);
					p->get=__getIO;
					p->put=__putIO;
					p->arg.io=NULL;
					if(p->rx && p->tx)
						return(p);
					if(p->rx)
						free(p->rx);
					if(p->tx)
						free(p->tx);
				}
				return(NULL);
}
//
//	io kill
//______________________________________________________________________________________
_io			*_io_close(_io *io) {
				if(io) {
					_buffer_close(io->rx);
					_buffer_close(io->tx);
					if(io->gets)
						free(io->gets);
					free(io);
				}
				return NULL;
}
//______________________________________________________________________________________
int			_buffer_LIFO(_buffer *p, void *q, int n) {
char		*t=p->_pull;
				while(n--) {
					if(t == p->_buf)
						t = &p->_buf[p->len];
					if(--t == p->_push)
						return EOF;
					*t = ((char *)q)[n];
				}
				p->_pull=t;
				return *(char *)q;
}
//______________________________________________________________________________________
int 		ungetch(int c) {
				if(__stdin.io)
					return _buffer_LIFO(__stdin.io->rx,&c,1);
				else
					return EOF;
}
/*-----------------------------------------------------------------------*/
/* Get a character from the file                                          */
/* add. to FatFs																												 */
/*-----------------------------------------------------------------------*/
int			f_getc (FIL* fil) {						/* Pointer to the file object */
				UINT br;
				BYTE s[4];
				f_read(fil, s, 1, &br);				/* Write the char to the file */
				return (br == 1) ? s[0] : EOF;/* Return the result */
}
/**
* @}
*/ 
