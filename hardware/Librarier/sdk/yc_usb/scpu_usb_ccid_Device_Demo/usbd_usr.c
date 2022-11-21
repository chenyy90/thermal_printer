/**
  ******************************************************************************
  * @file    usbd_usr.c
  * @author  YICHIP
  * @version V1.0.0
  * @date    21-October-2014
  * @brief   This file includes the user application layer
  ******************************************************************************
  * @attention
  *
  * 
  ******************************************************************************
  */ 

/* Includes ------------------------------------------------------------------*/
#include "usbd_usr.h"
#include "usbd_ioreq.h"

#include "usb_conf.h"

/** @addtogroup STM32_USB_OTG_DEVICE_LIBRARY
* @{
*/

/** @defgroup USBD_USR 
* @brief    This file includes the user application layer
* @{
*/ 

/** @defgroup USBD_USR_Private_TypesDefinitions
* @{
*/ 
/**
* @}
*/ 


/** @defgroup USBD_USR_Private_Defines
* @{
*/ 
/**
* @}
*/ 


/** @defgroup USBD_USR_Private_Macros
* @{
*/ 
/**
* @}
*/ 


/** @defgroup USBD_USR_Private_Variables
* @{
*/ 

USBD_Usr_cb_TypeDef USRD_cb =
{
  USBD_USR_Init,
  USBD_USR_DeviceReset,
  USBD_USR_DeviceConfigured,
  USBD_USR_DeviceSuspended,
  USBD_USR_DeviceResumed,
  
  USBD_USR_DeviceConnected,
  USBD_USR_DeviceDisconnected,  
  
  
};



/**
* @}
*/

/** @defgroup USBD_USR_Private_Constants
* @{
*/ 

/**
* @}
*/



/** @defgroup USBD_USR_Private_FunctionPrototypes
* @{
*/ 
/**
* @}
*/ 


/** @defgroup USBD_USR_Private_Functions
* @{
*/ 

#define USER_INFORMATION1      "[Key]:RemoteWakeup"
#define USER_INFORMATION2      "[Joystick]:Mouse emulation"


/**
* @brief  USBD_USR_Init 
*         Displays the message on LCD for host lib initialization
* @param  None
* @retval None
*/
void USBD_USR_Init(void)
{  
  
}

/**
* @brief  USBD_USR_DeviceReset 
*         Displays the message on LCD on device Reset Event
* @param  speed : device speed
* @retval None
*/
void USBD_USR_DeviceReset(uint8_t speed )
{
// switch (speed)
// {
//   case USB_OTG_SPEED_HIGH: 
//     LCD_LOG_SetFooter ("     USB Device Library v1.1.0 [HS]" );
//     break;

//  case USB_OTG_SPEED_FULL: 
//     LCD_LOG_SetFooter ("     USB Device Library v1.1.0 [FS]" );
//     break;
// default:
//     LCD_LOG_SetFooter ("     USB Device Library v1.1.0 [??]" );
//     
// }
}


/**
* @brief  USBD_USR_DeviceConfigured
*         Displays the message on LCD on device configuration Event
* @param  None
* @retval Staus
*/
void USBD_USR_DeviceConfigured (void)
{
//    MyPrintf("> HID Interface started.\n");
}


/**
* @brief  USBD_USR_DeviceConnected
*         Displays the message on LCD on device connection Event
* @param  None
* @retval Staus
*/
void USBD_USR_DeviceConnected (void)
{
//    MyPrintf("> USB Device Connected.\n");
}


/**
* @brief  USBD_USR_DeviceDisonnected
*         Displays the message on LCD on device disconnection Event
* @param  None
* @retval Staus
*/
void USBD_USR_DeviceDisconnected (void)
{
//    MyPrintf("> USB Device Disconnected.\n");
}

/**
* @brief  USBD_USR_DeviceSuspended 
*         Displays the message on LCD on device suspend Event
* @param  None
* @retval None
*/
void USBD_USR_DeviceSuspended(void)
{
    MyPrintf("\n> USB Device in Suspend Mode.\n");
    /* Users can do their application actions here for the USB-Reset */
}


/**
* @brief  USBD_USR_DeviceResumed 
*         Displays the message on LCD on device resume Event
* @param  None
* @retval None
*/
void USBD_USR_DeviceResumed(void)
{
    MyPrintf("\n> USB Device in Idle Mode.\n");
    /* Users can do their application actions here for the USB-Reset */
}


void my_delay_ms(int ms)
{
	int delay_num;
	while(ms-->0)
	{
		delay_num=(CPU_MHZ/1000)/8+400;
		while(delay_num-->0)
		{
			__asm__("nop");
		}
	}
}


/**
* @}
*/ 

/**
* @}
*/ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/






























