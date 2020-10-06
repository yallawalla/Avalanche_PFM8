/**
  ******************************************************************************
  * @file    stm32fxxx_it.c
  * @author  MCD Application Team
  * @version V1.1.0
  * @date    19-March-2012
  * @brief   Main Interrupt Service Routines.
  *          This file provides all exceptions handler and peripherals interrupt
  *          service routine.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2012 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "stm32fxxx_it.h"
#include "usb_core.h"
#include "usbd_core.h"
#include "usb_conf.h"
#include "pfm.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
extern USB_OTG_CORE_HANDLE  USB_OTG_Core;
extern uint32_t USBD_OTG_ISR_Handler (USB_OTG_CORE_HANDLE *);
extern uint32_t USBH_OTG_ISR_Handler (USB_OTG_CORE_HANDLE *);
extern void __OTG_FS_IRQHandler(void),
						__EXTI_IRQHandler(void);


/* Private function prototypes -----------------------------------------------*/

/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*             Cortex-M Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief   This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
	_stdio(__com1);
	__print(":%04d NMI_Handler\r\n>",__time__ % 10000);
	while(1);
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
	_stdio(__com1);
	__print(":%04d HardFault_Handler\r\n>",__time__ % 10000);
	while(1);
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
	_stdio(__com1);
	__print(":%04d MemManage_Handler\r\n>",__time__ % 10000);
	while(1);
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
	_stdio(__com1);
	__print(":%04d BusFault_Handler\r\n>",__time__ % 10000);
	while(1);
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
	_stdio(__com1);
	__print(":%04d UsageFault_Handler\r\n>",__time__ % 10000);
	while(1);
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void) {
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
volatile int	__time__;
void SysTick_Handler(void) {
#if defined (__F7__)
extern void HAL_IncTick(void);
	HAL_IncTick();
#endif
	++__time__;
}

/**
  * @brief  OTG_FS_IRQHandler
  *          This function handles USB-On-The-Go FS global interrupt request.
  *          requests.
  * @param  None
  * @retval None
  */

#if defined (__F2__) || defined (__F4__)
void OTG_FS_IRQHandler(void)
{
  if (USB_OTG_IsHostMode(&USB_OTG_Core)) /* ensure that we are in device mode */
  {
    USBH_OTG_ISR_Handler(&USB_OTG_Core);
  }
  else
  {
    USBD_OTG_ISR_Handler(&USB_OTG_Core);
  }
} 
#endif
/**
  * @brief  Calling common handler for EXTI interrupts
  * @param  None
  * @retval None
  */
void	EXTI9_5_IRQHandler(void)
{
	__EXTI_IRQHandler();
}

void	EXTI15_10_IRQHandler(void)
{
	__EXTI_IRQHandler();
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
