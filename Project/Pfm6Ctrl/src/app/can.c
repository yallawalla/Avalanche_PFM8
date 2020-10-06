/**
  ******************************************************************************
  * @file    can.c
  * @author  Fotona d.d.
  * @version V1
  * @date    30-Sept-2013
  * @brief   CAN initialization
  *
  */
/** @addtogroup PFM6_Setup
* @{
*/
#include	"pfm.h"
_io				*__can;
/*******************************************************************************
* Function Name  : CAN_Initialize
* Description    : Configures the CAN, transmit and receive using interrupt.
* Input          : None
* Output         : None
* Return         : PASSED if the reception is well done, FAILED in other case
*******************************************************************************/
struct { int id, mask; } canFilter[28] =
{
					{_ID_SYS2PFM,0x7ff},
					{_ID_SYS2EC,0x7ff},
					{_ID_SYS2PFMcom,0x7ff},
					{_ID_PFMcom2SYS,0x7ff},
					{_ID_SYS_TRIGG,0x7ff},
					{_PFM_POCKELS,0x7ff},
					{_ID_SYS2ENRG,0x7ff},
					{_ID_ENRG2SYS,0x7ff},
					{_PFM_TAND_CH0,0x7f0}
};
/*******************************************************************************
* Function Name  : CAN_Initialize
* Description    : Configures the CAN, transmit and receive using interrupt.
* Input          : None
* Output         : None
* Return         : PASSED if the reception is well done, FAILED in other case
*******************************************************************************/
void			canFilterConfig(int id, int mask) {
					int i;
					CAN_FilterInitTypeDef		CAN_FilterInitStructure;
					for(i=0; canFilter[i].id && canFilter[i].mask; ++i)
						if(canFilter[i].id==id && canFilter[i].mask==mask)
							return;
					canFilter[i].id= id;
					canFilter[i].mask= mask;

					CAN_FilterInitStructure.CAN_FilterMode=CAN_FilterMode_IdMask;
					CAN_FilterInitStructure.CAN_FilterScale=CAN_FilterScale_16bit;
					CAN_FilterInitStructure.CAN_FilterActivation=ENABLE;
					CAN_FilterInitStructure.CAN_FilterFIFOAssignment=CAN_FIFO0;

					for(i=0; canFilter[i].id && canFilter[i].mask; i+=2) {
						CAN_FilterInitStructure.CAN_FilterIdLow =  canFilter[i].id<<5;
						CAN_FilterInitStructure.CAN_FilterMaskIdLow = canFilter[i].mask<<5;
						
						CAN_FilterInitStructure.CAN_FilterIdHigh= canFilter[i].id<<5;
						CAN_FilterInitStructure.CAN_FilterMaskIdHigh= canFilter[i].mask<<5;
						
						if(canFilter[i+1].id && canFilter[i+1].mask) {
							CAN_FilterInitStructure.CAN_FilterIdHigh= canFilter[i+1].id<<5;
							CAN_FilterInitStructure.CAN_FilterMaskIdHigh= canFilter[i+1].mask<<5;
						}
						
						CAN_FilterInitStructure.CAN_FilterNumber=__FILT_BASE__ + i/2;
						CAN_FilterInit(&CAN_FilterInitStructure);
					}
}
/*******************************************************************************
* Function Name  : CAN_Initialize
* Description    : Configures the CAN, transmit and receive using interrupt.
* Input          : None
* Output         : None
* Return         : PASSED if the reception is well done, FAILED in other case
*******************************************************************************/
_io			 	*Initialize_CAN(int loop) {
	
CAN_InitTypeDef		CAN_InitStructure;
GPIO_InitTypeDef	GPIO_InitStructure;

					GPIO_StructInit(&GPIO_InitStructure);
					GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
					GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;

#if		!defined (__DISC4__) && !defined (__DISC7__)
					GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
					GPIO_Init(GPIOB, &GPIO_InitStructure);
					GPIO_PinAFConfig(GPIOB, GPIO_PinSource5, GPIO_AF_CAN2);
#else
					GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
					GPIO_Init(GPIOB, &GPIO_InitStructure);
					GPIO_PinAFConfig(GPIOB, GPIO_PinSource12, GPIO_AF_CAN2);
#endif
					GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
					GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
					GPIO_Init(GPIOB, &GPIO_InitStructure);
					GPIO_PinAFConfig(GPIOB, GPIO_PinSource13, GPIO_AF_CAN2);

					RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);							// glej opis driverja, šmafu, oba je treba inicializirat
					RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN2, ENABLE);
					CAN_StructInit(&CAN_InitStructure);
					CAN_DeInit(__CAN__);

					CAN_InitStructure.CAN_TTCM=DISABLE;
					CAN_InitStructure.CAN_ABOM=ENABLE;
					CAN_InitStructure.CAN_AWUM=DISABLE;
					CAN_InitStructure.CAN_NART=DISABLE;
					CAN_InitStructure.CAN_RFLM=DISABLE;

//... pomembn.. da ne zamesa mailboxov in jih oddaja po vrstnem redu vpisovanja... ni default !!!

					CAN_InitStructure.CAN_TXFP=ENABLE;

					if(loop)
						CAN_InitStructure.CAN_Mode=CAN_Mode_LoopBack;
					else
						CAN_InitStructure.CAN_Mode=CAN_Mode_Normal;

#if defined (__F2__) || defined (__F4__)
					CAN_InitStructure.CAN_SJW=CAN_SJW_4tq;
					CAN_InitStructure.CAN_BS1=CAN_BS1_10tq;
					CAN_InitStructure.CAN_BS2=CAN_BS2_4tq;
					CAN_InitStructure.CAN_Prescaler=4;
					CAN_Init(__CAN__,&CAN_InitStructure);

#elif defined (__F7__)
					CAN_InitStructure.CAN_SJW=CAN_SJW_1tq;
					CAN_InitStructure.CAN_BS1=CAN_BS1_8tq;
					CAN_InitStructure.CAN_BS2=CAN_BS2_3tq;
					CAN_InitStructure.CAN_Prescaler=9;
					CAN_Init(__CAN__,&CAN_InitStructure);
#else
*** define CPU
#endif
					canFilterConfig(0,0);	
					CAN_ITConfig(__CAN__, CAN_IT_FMP0 | 
					CAN_IT_EWG, ENABLE);
					if(__can)
						return __can;
					else
						return(_io_init(100*sizeof(CanRxMsg),100*sizeof(CanTxMsg)));
}
/*******************************************************************************/
void 			CAN1_TX_IRQHandler(void)
{
CanTxMsg	tx;
					if(_buffer_pull(__can->tx,&tx,sizeof(CanTxMsg)))
						CAN_Transmit(CAN1,&tx);
					else
						CAN_ITConfig(CAN1, CAN_IT_TME, DISABLE);						
}
/*******************************************************************************/
void 			CAN2_TX_IRQHandler(void)
{
CanTxMsg	tx;
					if(_buffer_pull(__can->tx,&tx,sizeof(CanTxMsg)))
						CAN_Transmit(CAN2,&tx);
					else
						CAN_ITConfig(CAN2, CAN_IT_TME, DISABLE);						
}
/*******************************************************************************/
void 			CAN1_RX0_IRQHandler(void)
{
CanRxMsg	rx;
					CAN_Receive(CAN1, CAN_FIFO0, &rx);
					_buffer_push(__can->rx,&rx,sizeof(CanRxMsg));
}
/*******************************************************************************/
void 			CAN2_RX0_IRQHandler(void)
{
CanRxMsg	rx;
					CAN_Receive(CAN2, CAN_FIFO0, &rx);
					_buffer_push(__can->rx,&rx,sizeof(CanRxMsg));
}
/*******************************************************************************/
void 			CAN1_SCE_IRQHandler(void) {
					CAN1->MSR |= 0x0004;
					}
/*******************************************************************************/
void 			CAN2_SCE_IRQHandler(void) {
					CAN2->MSR |= 0x0004;
					}
/**
* @}
*/ 

