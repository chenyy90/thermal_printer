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




#include <string.h>
#include <stdio.h>
#include "usb_dcd_int.h"
#include "usbd_cdc_core.h"
#include "usbd_usr.h"
#include "usbd_desc.h"
#include "misc.h"


USB_OTG_CORE_HANDLE  USB_OTG_dev;
extern struct APP_DATA_STRUCT_DEF APP_Gdata_param; 
extern CDC_IF_Prop_TypeDef  APP_FOPS;


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
 extern uint8_t CDC_Recv_Flag;

uint8_t Rxbuf[1024*2] = {0};
void usb_main(void)
{
  int rxlen = 0;
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
              &USBD_CDC_cb, 
              &USRD_cb);
	
    MyPrintf("YICHIP SCPU USB VCP Demo V1.0.\n");
	
	while(1)
	{
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
                                            &USBD_CDC_cb, 
                                            &USRD_cb);
					temp1=temp2=cnt=0;
					USB_OTG_WRITE_REG8(CORE_USB_CLEAR,0x40);
					USB_OTG_WRITE_REG8(USB_SOFCNT,0);
					usb_init();
				}
		if (CDC_Recv_Flag)
		{
			rxlen = VCP_GetRxBuflen();
			if(rxlen>0)
			{
				MyPrintf("rxlen =%d\n\r",rxlen);
				for(int i=0; i<rxlen; i++)
				{
					Rxbuf[i] = VCP_GetRxChar();	
				}

				CDC_Recv_Flag = 0;
				DCD_EP_Tx(&USB_OTG_dev,CDC_IN_EP,(uint8_t*)Rxbuf,rxlen);	
			}
		}
	}
  }  







void USB_IRQHandler(void)
{
    USBD_OTG_ISR_Handler (&USB_OTG_dev);
 //   NVIC_ClearPendingIRQ(USB_IRQn);
 //   MyPrintf("YICHIP SCPU USB CCID handler startV1.0.\n");
}









 


