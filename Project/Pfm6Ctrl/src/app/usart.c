/**
  ******************************************************************************
  * @file    serial.c
  * @author  Fotona d.d.
  * @version V1
  * @date    30-Sept-2013
  * @brief	 Serial port initialization & ISR
  *
  */

/** @addtogroup PFM6_Setup
* @{
*/
#include	"cpu.h"
#include	<string.h>
#include	<stdio.h>
#include	"io.h"
/*******************************************************************************
* Function Name  : DMA_Configuration
* Description    : Configures the DMA.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
#define RxBufferSize		128
#define TxBufferSize		128
/* Private variables ---------------------------------------------------------*/
USART_InitTypeDef USART_InitStructure;
#if	!defined (__DISC4__)	&& !defined (__DISC7__)
void DMA_Configuration(_io *io)
{	
	DMA_InitTypeDef DMA_InitStructure;
	DMA_StructInit(&DMA_InitStructure);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);

/*DMA2 channel4, stream7  USART1 TX*/
	DMA_DeInit(DMA2_Stream7);
	DMA_InitStructure.DMA_Channel = DMA_Channel_4;
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)USART1_BASE+4;
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)(io->tx->_buf);
	DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
	DMA_InitStructure.DMA_BufferSize = 0;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;

	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;

	DMA_Init(DMA2_Stream7, &DMA_InitStructure);
	DMA_Cmd(DMA2_Stream7, ENABLE);

/*DMA2 channel4, stream2 USART1 RX*/
	DMA_DeInit(DMA2_Stream2);
	DMA_InitStructure.DMA_Channel = DMA_Channel_4;
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)USART1_BASE+4;
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)(io->rx->_buf);
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
	DMA_InitStructure.DMA_BufferSize = RxBufferSize;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;

	DMA_Init(DMA2_Stream2, &DMA_InitStructure);
	DMA_Cmd(DMA2_Stream2, ENABLE);

	USART_DMACmd(USART1, USART_DMAReq_Rx | USART_DMAReq_Tx, ENABLE);
}
//______________________________________________________________________________________
int	__putDMA(_buffer *tx, int	c) {
static
	int n=0;

	if(DMA2_Stream7->NDTR==0)																					// end of buffer reached
		n=0;																														// reset static index
	if(n == TxBufferSize-1)																						// buffer full ?
		return(EOF);	
	
	DMA2_Stream7->CR &= ~0x00000001;																	// stream 7 off
	while(DMA2_Stream7->CR & 0x00000001);															// wait for HW

	DMA2_Stream7->M0AR=(uint32_t)(&tx->_buf[n-DMA2_Stream7->NDTR]);		// set DMA pointer to new character
	DMA2_Stream7->NDTR++;																							// increase DMA counter
	tx->_buf[n++]=c;																									// enter new character
	
	DMA2->HIFCR = 0x0F400000;																					// clear all flags
	DMA2_Stream7->CR |= 0x00000001;																		// stream 7 on
	return(c);
}
//______________________________________________________________________________________
int	__getDMA(_buffer *rx) {
int	i=0;
	if(DMA2_Stream2->NDTR)
		rx->_push = &(rx->_buf[rx->size - DMA2_Stream2->NDTR]);
	else
		rx->_push = rx->_buf;
	if(_buffer_pull(rx,&i,1))
		return i;
	else
		return EOF;
}
#else
//______________________________________________________________________________________
volatile int32_t  ITM_RxBuffer=ITM_RXBUFFER_EMPTY; 
void DMA_Configuration(_io *io) {	

}
//______________________________________________________________________________________
int	__putDMA(_buffer *tx, int	c) {
	ITM_SendChar(c);
	return  c;
}
//______________________________________________________________________________________
int	__getDMA(_buffer *rx) {
int	i=0;
	if(_buffer_pull(rx,&i,1))
		return i;
	else if(ITM_CheckChar())
		return ITM_ReceiveChar();
	else
		return EOF;
}
#endif
//______________________________________________________________________________________
_io *Initialize_USART1(int speed) {

USART_InitTypeDef 			USART_InitStructure;
USART_ClockInitTypeDef  USART_ClockInitStructure;
GPIO_InitTypeDef				GPIO_InitStructure;
_io 										*io;

	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
 	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
 	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

 	GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);		
 	GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1);		
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
 
	USART_ClockInitStructure.USART_Clock = USART_Clock_Disable;
	USART_ClockInitStructure.USART_CPOL = USART_CPOL_Low;
	USART_ClockInitStructure.USART_CPHA = USART_CPHA_2Edge;
	USART_ClockInitStructure.USART_LastBit = USART_LastBit_Disable;
	USART_ClockInit(USART1, &USART_ClockInitStructure);
 
	USART_InitStructure.USART_BaudRate = speed;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART1, &USART_InitStructure);
	USART_Cmd(USART1, ENABLE);
	
	io=_io_init(RxBufferSize,TxBufferSize);
	io->get= __getDMA;	
	io->put= __putDMA;
	DMA_Configuration(io);
	return io;
}
//______________________________________________________________________________________
extern _io* __com3;
int	putCOM3(_buffer *p, int	c) {
	if(_buffer_push(__com3->tx,&c,1)==1)
		USART_ITConfig(USART3, USART_IT_TXE, ENABLE);
	else
		c=EOF;
	return(c);
}
//______________________________________________________________________________________
_io *Initialize_USART3(int speed) {

USART_InitTypeDef 			USART_InitStructure;
USART_ClockInitTypeDef  USART_ClockInitStructure;
_io 										*io;
GPIO_InitTypeDef				GPIO_InitStructure;
	
	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
 	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
 	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

 	GPIO_PinAFConfig(GPIOC, GPIO_PinSource10, GPIO_AF_USART3);		
 	GPIO_PinAFConfig(GPIOC, GPIO_PinSource11, GPIO_AF_USART3);		
	
	io=_io_init(RxBufferSize,TxBufferSize);
	io->put = putCOM3;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
 
	USART_ClockInitStructure.USART_Clock = USART_Clock_Disable;
	USART_ClockInitStructure.USART_CPOL = USART_CPOL_Low;
	USART_ClockInitStructure.USART_CPHA = USART_CPHA_2Edge;
	USART_ClockInitStructure.USART_LastBit = USART_LastBit_Disable;
	USART_ClockInit(USART3, &USART_ClockInitStructure);
 
	USART_InitStructure.USART_BaudRate = speed;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART3, &USART_InitStructure);

	/* Enable USART3 */
	
	USART_Cmd(USART3, ENABLE);
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);		
	return io;
}
/*******************************************************************************
* Function Name  : USART1_IRQHandler
* Description    : This function handles USART3  interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void USART3_IRQHandler(void) {
int i=0;
	if(USART_GetFlagStatus(USART3, USART_FLAG_RXNE) != RESET) {
		USART_ClearITPendingBit(USART3, USART_IT_RXNE);
		i=USART_ReceiveData(USART3);
		_buffer_push(__com3->rx,&i,1);
		}
	if(USART_GetFlagStatus(USART3, USART_FLAG_TXE) != RESET) {
		USART_ClearITPendingBit(USART3, USART_IT_TXE);
		if (_buffer_pull(__com3->tx,&i,1))															// if data available
			USART_SendData(USART3, i);																		// send!
		else																														// else
			USART_ITConfig(USART3, USART_IT_TXE, DISABLE);								// disable interrupt
	}
}

/**
* @}
*/ 
