/**
  ******************************************************************************
  * @file    USBD_usr.c
  * @author  MCD Application Team
  * @version V1.1.0
  * @date    19-March-2012
  * @brief   This file includes the user application layer
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
#include "usbd_usr.h"
#include "usbd_ioreq.h"
#include "usb_conf.h"
#include "stdio.h"
#include "pfm.h"
/** @addtogroup USBD_USER
* @{
*/

#ifdef __GNUC__
/* With GCC/RAISONANCE, small LCD_UsrLog (option LD Linker->Libraries->Small LCD_UsrLog
set to 'Yes') calls __io_putchar() */
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */

/** @addtogroup USBD_MSC_DEMO_USER_cb
* @{
*/

/** @defgroup USBD_USR 
* @brief    This file includes the user application layer
* @{
*/ 

/** @defgroup USBD_MSC_Private_TypesDefinitions
* @{
*/ 
/**
* @}
*/ 


/** @defgroup USBD_MSC_Private_Defines
* @{
*/ 

/**
* @}
*/ 


/** @defgroup USBD_MSC_Private_Macros
* @{
*/ 
/**
* @}
*/ 


/** @defgroup USBD_MSC_Private_Variables
* @{
*/ 
/*  Points to the DEVICE_PROP structure of current device */
/*  The purpose of this register is to speed up the execution */

USBD_Usr_cb_TypeDef USR_MSC_cb =
{
  USBD_MSC_DeviceInit,
  USBD_MSC_DeviceReset,
  USBD_MSC_DeviceConfigured,
  USBD_MSC_DeviceSuspended,
  USBD_MSC_DeviceResumed,
  
  USBD_MSC_DeviceConnected,
  USBD_MSC_DeviceDisconnected,    
};

USBD_Usr_cb_TypeDef USR_CDC_cb =
{
  USBD_VCP_DeviceInit,
  USBD_VCP_DeviceReset,
  USBD_VCP_DeviceConfigured,
  USBD_VCP_DeviceSuspended,
  USBD_VCP_DeviceResumed,

  USBD_VCP_DeviceConnected,
  USBD_VCP_DeviceDisconnected,    
};

/* wrapperji za inicializacije iz com.c, da se izognes includom
*/
__ALIGN_BEGIN
USB_OTG_CORE_HANDLE							USB_OTG_Core;
__ALIGN_END

/**
* @}
*/

/** @defgroup USBD_MSC_Private_Constants
* @{
*/ 

/**
* @}
*/

/** @defgroup USBD_MSC_Private_FunctionPrototypes
* @{
*/ 
/**
* @}
*/ 


/** @defgroup USBD_MSC_Private_Functions
* @{
*/ 

#define USER_INFORMATION1 "MSC running on High speed."
#define USER_INFORMATION2 "HID running on Full speed."

/**
* @brief  USBD_MSC_Init 
*         Displays the message on LCD for host lib initialization
* @param  None
* @retval None
*/
void USBD_MSC_DeviceInit(void)
{
	_DEBUG_(_DBG_SYS_MSG,"..MSC initialized\r\n");
}

/**
* @brief  USBD_MSC_DeviceReset 
*         Displays the message on LCD on device attached
* @param  None
* @retval None
*/
void USBD_MSC_DeviceReset(uint8_t speed)
{
	_DEBUG_(_DBG_SYS_MSG,"..MSC reset\r\n");
}


/**
* @brief  USBD_DisconnectEvent
*         Device disconnect event
* @param  None
* @retval Staus
*/
void USBD_MSC_DeviceConfigured (void)
{
	_DEBUG_(_DBG_SYS_MSG,"..MSC configured\r\n");
}

/**
* @brief  USBD_MSC_ResetUSBDevice 
* @param  None
* @retval None
*/

void USBD_MSC_DeviceSuspended(void)
{
	_DEBUG_(_DBG_SYS_MSG,"..MSC suspended\r\n");
}


/**
* @brief  USBD_MSC_ResetUSBDevice 
* @param  None
* @retval None
*/
void USBD_MSC_DeviceResumed(void)
{
	_DEBUG_(_DBG_SYS_MSG,"..MSC resumed\r\n");
}

/**
* @brief  USBD_USR_DeviceConnected
*         Displays the message on LCD on device connection Event
* @param  None
* @retval Staus
*/
void USBD_MSC_DeviceConnected (void)
{
	_DEBUG_(_DBG_SYS_MSG,"..MSC connected\r\n");
}


/**
* @brief  USBD_USR_DeviceDisonnected
*         Displays the message on LCD on device disconnection Event
* @param  None
* @retval Staus
*/
void USBD_MSC_DeviceDisconnected (void)
{
	_DEBUG_(_DBG_SYS_MSG,"..MSC disconnected\r\n");
}
/*****************************************************************************/
/**
* @brief  USBD_VCP_Init 
*         Displays the message on LCD for host lib initialization
* @param  None
* @retval None
*/
void USBD_VCP_DeviceInit(void)
{
	_DEBUG_(_DBG_SYS_MSG,"..VCP initialized\r\n");
}

/**
* @brief  USBD_MSC_DeviceReset 
*         Displays the message on LCD on device attached
* @param  None
* @retval None
*/
void USBD_VCP_DeviceReset(uint8_t speed)
{
	_DEBUG_(_DBG_SYS_MSG,"..VCP reset\r\n");
}


/**
* @brief  USBD_DisconnectEvent
*         Device disconnect event
* @param  None
* @retval Staus
*/
void USBD_VCP_DeviceConfigured (void)
{
	_DEBUG_(_DBG_SYS_MSG,"..VCP configured\r\n");
}

/**
* @brief  USBD_MSC_ResetUSBDevice 
* @param  None
* @retval None
*/
void USBD_VCP_DeviceSuspended(void)
{
	_DEBUG_(_DBG_SYS_MSG,"..VCP suspended\r\n");
}


/**
* @brief  USBD_MSC_ResetUSBDevice 
* @param  None
* @retval None
*/
void USBD_VCP_DeviceResumed(void)
{
	_DEBUG_(_DBG_SYS_MSG,"..VCP resumed\r\n");
}

/**
* @brief  USBD_USR_DeviceConnected
*         Displays the message on LCD on device connection Event
* @param  None
* @retval Status
*/
void USBD_VCP_DeviceConnected (void)
{
	_DEBUG_(_DBG_SYS_MSG,"..VCP connected\r\n");
}


/**
* @brief  USBD_USR_DeviceDisonnected
*         Displays the message on LCD on device disconnection Event
* @param  None
* @retval Staus
*/
void USBD_VCP_DeviceDisconnected (void)
{
	_DEBUG_(_DBG_SYS_MSG,"..VCP disconnected\r\n");
}
/**
* @}
*/ 

/**
* @}
*/ 

/**
* @}
*/

/**
* @}
*/

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

