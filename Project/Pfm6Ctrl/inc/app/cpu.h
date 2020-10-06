/* Includes ------------------------------------------------------------------*/
#ifndef __CPU_H
#define	__CPU_H

#if		defined		(__F2__)
#include				"stm32f2xx.h"
#elif defined		(__F4__) 
#include				"stm32f4xx.h"
#elif defined		(__F7__) 
#include				"stm32f7xx.h"
#else 
	*** CPU not defined
#endif
#endif
