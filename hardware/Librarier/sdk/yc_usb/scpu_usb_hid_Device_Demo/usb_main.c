/**
  ******************************************************************************
  * @file    main.c
  * @author  yichip
  * @version V1.0.0
  * @date    7-9-2018
  * @brief   This file provides all the Application firmware functions.
  ******************************************************************************
  * @attention
  *
  ******************************************************************************
  */ 

/* Includes ------------------------------------------------------------------*/ 



#include "usbd_usr.h"
#include "usbd_desc.h"
#include <string.h>
#include <stdio.h>
#include "usb_dcd_int.h"
#include "usbd_hid_core.h"


uint8_t Buf[4] = {0, 0, 0, 0};

USB_OTG_CORE_HANDLE  USB_OTG_dev;

void usb_init()
{
					USB_OTG_WRITE_REG8(CORE_USB_CONFIG,0x00);
		//			my_delay_ms(1);
					USB_OTG_WRITE_REG8(CORE_USB_TRIG,0xc0); 
					 USB_OTG_WRITE_REG8(CORE_USB_CONFIG,0x3c);
					 USB_OTG_WRITE_REG8(CORE_USB_STATUS,0xFF);
					 USB_OTG_WRITE_REG8(CORE_USB_FIFO_EMPTY,0xFF);
					 USB_OTG_WRITE_REG8(CORE_USB_ADDR,0x00);
					 USB_OTG_WRITE_REG8(CORE_USB_INT_MASK(0),0x70);
					 #ifdef USB_HID_DEMO_TEST
					  USB_OTG_WRITE_REG8(CORE_USB_INT_MASK(1),0xf2);
					  #else
					 USB_OTG_WRITE_REG8(CORE_USB_INT_MASK(1),0xf0);
					 #endif
					 USB_OTG_WRITE_REG8(CORE_USB_INT_MASK(2),0xff);


}

void usb_main(void)
{
	uint8_t ReceiveCode = 0x41;
	int8_t x = 0, y = 0;
    
	enable_clock(CLKCLS_BT);	
	BT_CLKPLL_EN = 0xff;
	enable_clock(CLKCLS_USB);
	SYSCTRL_HCLK_CON |= 1 << 11;
	usb_init();
	
       enable_intr(INTR_USB); 
   memset(&USB_OTG_dev, 0, sizeof(USB_OTG_dev));

  USBD_Init(&USB_OTG_dev,
        #ifdef USE_USB_OTG_HS
              USB_OTG_HS_CORE_ID,
        #else
              USB_OTG_FS_CORE_ID, 
        #endif
              &USR_desc,
              &USBD_HID_cb, 
              &USRD_cb);
    MyPrintf("YICHIP SCPU HID Device Demo V1.0.\n");

  #ifdef USB_HID_MS_DEMO      
      while (1)
 {
 		    #ifndef POWER_FROM_USB
				uint16_t temp1,temp2,cnt;
				temp2 = temp1;
				temp1 = USB_OTG_READ_REG8(USB_SOFCNT)|( USB_OTG_READ_REG8(USB_SOFCNTHI)<<8); 
				if((temp2 == temp1) && (temp1 !=0))
				    cnt++;
				else 
					cnt = 0;
				
				if(cnt ==10000)
				{
					MyPrintf("reset ");
					USB_OTG_dev.dev.zero_replay_flag =0;
					memset(&USB_OTG_dev, 0, sizeof(USB_OTG_dev));
					USB_OTG_dev.dev.device_status = USB_OTG_DEFAULT;

                              	 USBD_Init(&USB_OTG_dev,
                                      #ifdef USE_USB_OTG_HS
                                            USB_OTG_HS_CORE_ID,
                                      #else
                                            USB_OTG_FS_CORE_ID, 
                                      #endif
                                            &USR_desc,
                                            &USBD_HID_cb, 
                                            &USRD_cb);
					temp1=temp2=cnt=0;
					USB_OTG_WRITE_REG8(CORE_USB_CLEAR,0x40);
					USB_OTG_WRITE_REG8(USB_SOFCNT,0);
					usb_init();
				
				}
		     #endif
		#ifdef USB_HID_DEMO_TEST
			   if (USB_OTG_dev.dev.device_status_new== USB_OTG_END)
			   {
                              	     x = 0;
                                          y = 0;
                                          if(ReceiveCode == 0x45 )
                                          	{
                              			ReceiveCode = 0x41;
                                          	}
                                          else
                                          	{
                              			ReceiveCode++;
                                          	}
                                          
                              		switch (ReceiveCode)
                                          {
                                              case 0x41:                      /* ио */
                                                  y -= 10;
                                                  Buf[1] = 0;
                                                  Buf[2] = y;
                                                  DCD_EP_Tx ( &USB_OTG_dev,
                                                              HID_IN_EP,
                                                              Buf,
                                                              4);
                                                              
                                                  break;
                                              case 0x42:                      /* об */
                                                  y += 10;
                                                  Buf[1] = 0;
                                                  Buf[2] = y;
                                                  DCD_EP_Tx ( &USB_OTG_dev,
                                                              HID_IN_EP,
                                                              Buf,
                                                              4);
                                                  break;
                                              case 0x43:                      /* ср */
                                                  x += 10;
                                                  Buf[1] = x;
                                                  Buf[2] = 0;
                                                  DCD_EP_Tx ( &USB_OTG_dev,
                                                              HID_IN_EP,
                                                              Buf,
                                                              4);
                                                  break;
                                              case 0x44:                      /* вС */
                                                  x -= 10;
                                                  Buf[1] = x;
                                                  Buf[2] = 0;
                                                  DCD_EP_Tx ( &USB_OTG_dev,
                                                              HID_IN_EP,
                                                              Buf,
                                                              4);
                                                  break;
                                              default:
                                                  break;
                                          }
                             }
                #endif
  }  

#endif
}






void USB_IRQHandler(void)
{
    USBD_OTG_ISR_Handler (&USB_OTG_dev);
 //   NVIC_ClearPendingIRQ(USB_IRQn);
 //   MyPrintf("YICHIP SCPU USB CCID handler startV1.0.\n");
}









 


