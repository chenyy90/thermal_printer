/************************ (C) COPYRIGHT YICHIP *************************
 * File Name            : usb_dcd_int.c
 * Author               : YICHIP
 * Version              : V1.0.0
 * Date                 : 21-May-2019
 * Description          : Peripheral Device interrupt subroutines.
 *****************************************************************************/
 
/* Include ------------------------------------------------------------------*/
#include "usb_dcd_int.h"
#include "usb_defines.h"
#include "usbd_desc.h"
#include "yc_timer.h"
//#include "usbd_ccid_core.h"
#include "usbd_usr.h"
//#include "systick.h"
//#include "usb_std.h"
/** @addtogroup USB_OTG_DRIVER
* @{
*/

/** @defgroup USB_DCD_INT 
* @brief This file contains the interrupt subroutines for the Device mode.
* @{
*/


/** @defgroup USB_DCD_INT_Private_Defines
* @{
*/ 
/**
* @}
*/ 

/* Interrupt Handlers */
static uint32_t DCD_HandleInEP_ISR(USB_OTG_CORE_HANDLE *pdev, uint16_t ep_intr);
static uint32_t DCD_HandleOutEP_ISR(USB_OTG_CORE_HANDLE *pdev, uint16_t ep_intr);
static uint32_t DCD_HandleUsbReset_ISR();

void _delay_(uint32_t t)
{
	((void(*)())(0xc6+1))(t);
}

/**
* @brief  USBD_OTG_ISR_Handler
*         handles all USB Interrupts
* @param  pdev: device instance
* @retval status
*/

extern USBD_DCD_INT_cb_TypeDef USBD_DCD_INT_cb;
uint8_t setup_cnt =0;
uint8_t SetAddress_Flag = 0;
uint8_t Address_Value = 0;
uint8_t out0_data_len=0;
uint32_t USBD_OTG_ISR_Handler (USB_OTG_CORE_HANDLE *pdev)
{
   USB_OTG_IRQ1_TypeDef gintr_status;
   USB_OTG_trig_TypeDef trig;
   USB_OTG_IRQ2_TypeDef fifo_empty_irq;
   
   
    uint32_t retval = 0;
    USB_OTG_EP *ep;
    uint8_t rx_DataLength = 0;
      if (USB_OTG_IsDeviceMode(pdev)) /* ensure that we are in device mode */
      {
				gintr_status.d8 = USB_OTG_READ_REG8(&(pdev ->regs.STATUSEGS ->STATUS));
				fifo_empty_irq.d8 = USB_OTG_READ_REG8(&pdev ->regs.STATUSEGS ->EMPTY);
//	MyPrintf("\n%x",gintr_status.d8);
//	MyPrintf("empty =%x\n",fifo_empty_irq.d8  );
//	MyPrintf(" status =%x\n",(&(pdev ->regs.STATUSEGS ->STATUS)));
			if(! gintr_status.d8)
				return 0;


//	if (gintr_status.b.suspend)
//        {
//            retval |= DCD_HandleUSBSuspend_ISR(pdev);
//        }
//	if(gintr_status.b.nak)
//	{
//	  retval |= DCD_HandleSof_ISR(pdev);
//	}
	if(gintr_status.b.reset)
	{
	  retval |= DCD_HandleUsbReset_ISR(pdev);
	}
	USB_OTG_WRITE_REG8(&pdev->regs.STATUSEGS ->STATUS, 0xe0);

  if(gintr_status.d8 &0x0E)
    {
    //	MyPrintf("bulkout\r\n");
	 retval |= DCD_HandleOutEP_ISR(pdev, gintr_status.d8 &0x0E);
	 USB_OTG_WRITE_REG8(&pdev->regs.STATUSEGS ->STATUS, gintr_status.d8 &0x0E);
    }
    
     if(gintr_status.b.outep0_irq)
     {
	  USB_OTG_WRITE_REG8(&pdev->regs.STATUSEGS ->STATUS, 0x11);
	 if (SetAddress_Flag)
            {
               USB_OTG_MODIFY_REG8(&pdev->regs.CTRLREGS->FADDR,0,0x80);
                SetAddress_Flag = 0;
//                MyPrintf("set address \n");
            }  
          if(gintr_status.b.setup)
          {
          	   delay_ms(1);
//               MyPrintf("cnt = %d\n",setup_cnt);
//               MyPrintf("YICHIP SCPU USB CCID setup packet start .\n");
//           USB_OTG_WRITE_REG8(&pdev->regs.STATUSEGS ->STATUS, 0x11);
           rx_DataLength = USB_OTG_READ_REG8(pdev->regs.LENREGS[0]);
//           MyPrintf("len =%d\n",rx_DataLength );
                if ((rx_DataLength < pdev->dev.out_ep[0].maxpacket) || \
                    (pdev->dev.out_ep[0].xfer_count + rx_DataLength == pdev->dev.out_ep[0].xfer_len))
                {
                    /* Copy the setup packet received in FIFO into the setup buffer in RAM */
                    USB_OTG_ReadPacket(pdev , 
                                       pdev->dev.out_ep[0].xfer_buff + pdev->dev.out_ep[0].xfer_count,
                                       0, 
                                       USB_SETUP_PACKET_LEN);
                    pdev->dev.out_ep[0].xfer_count =0;
                    if (pdev->dev.out_ep[0].xfer_buff != pdev->dev.setup_packet)
                    {
//                        if (pdev->dev.out_ep[0].xfer_len == 7)
//                        {
//                            csr0l.b.serviced_rxpktrdy = 1;
//                            USB_OTG_WRITE_REG8(&pdev->regs.INDEXREGS->CSRL.CSR0L, csr0l.d8);
                            
                            pdev->dev.out_ep[0].xfer_buff = pdev->dev.setup_packet;
                            pdev->dev.out_ep[0].xfer_count = 0;
                            pdev->dev.out_ep[0].xfer_len = 8;
//                            /* RX COMPLETE */
//                            USBD_DCD_INT_fops->DataOutStage(pdev , 0);
//                            pdev->dev.device_state = USB_OTG_EP0_STATUS_IN;
//                            DCD_EP_Tx (pdev,
//                                       0,
//                                       NULL, 
//                                       0); 
//                            /* RX COMPLETE */
//                            USBD_DCD_INT_fops->DataOutStage(pdev , 0);
//                        }
                    }
                    else{
                        /* deal with receive setup packet */
//                   MyPrintf("req ->bmRequest =%x\n",*(uint8_t *)  (pdev->dev.setup_packet));
				if(rx_DataLength >USB_SETUP_PACKET_LEN)
				{
					out0_data_len = rx_DataLength -USB_SETUP_PACKET_LEN;
				}
               	    USBD_DCD_INT_fops->SetupStage(pdev);
//               	      MyPrintf("setup packet receive.\n");
                    }
              }  
	else{
                    /* Copy the setup packet received in FIFO into the setup buffer in RAM */
                    USB_OTG_ReadPacket(pdev , 
//                                       pdev->dev.setup_packet + pdev->dev.out_ep[0].xfer_count, 
                                       pdev->dev.out_ep[0].xfer_buff + pdev->dev.out_ep[0].xfer_count,
                                       0, 
                                       rx_DataLength);
                    pdev->dev.out_ep[0].xfer_count += rx_DataLength;
//                    USB_OTG_WRITE_REG8(&pdev->regs.INDEXREGS->CSRL.CSR0L, csr0l.d8);
                }
          }

          
          //ep0 dispose

     }

//    if(gintr_status.d8 &0x0E)
//    {
//    	MyPrintf("bulkout\r\n");
//	 retval |= DCD_HandleOutEP_ISR(pdev, gintr_status.d8 &0x0E);
//	 USB_OTG_WRITE_REG8(&pdev->regs.STATUSEGS ->STATUS, gintr_status.d8 &0x0E);
//    }

    //     tx
    if(fifo_empty_irq.d8 &0x0F)
    {
    	  _delay_(300);
	   retval |= DCD_HandleInEP_ISR(pdev, fifo_empty_irq.d8 &0x0F);
            USB_OTG_WRITE_REG8(&pdev->regs.STATUSEGS ->EMPTY, fifo_empty_irq.d8 &0xFF);
            USB_OTG_WRITE_REG8(&pdev->regs.STATUSEGS ->FULL, 0xFF);
            
//             MyPrintf("fifo empty irq.\n");
    }
// if(fifo_empty_irq.d8 &0xF0)
// {
//	 MyPrintf("outfifo empty irq.\n");
//	retval |= DCD_HandleOutEP_ISR(pdev,fifo_empty_irq.d8 &0x20);
//	USB_OTG_WRITE_REG8(&pdev->regs.STATUSEGS ->EMPTY, fifo_empty_irq.d8 &0xF0);
// }
      }
      return retval;
}


/**
* @brief  DCD_HandleInEP_ISR
*         Indicates that an IN EP has a pending Interrupt
* @param  pdev: device instance
* @retval status
*/
static uint32_t DCD_HandleInEP_ISR(USB_OTG_CORE_HANDLE *pdev, uint16_t ep_intr)
{
	  USB_OTG_EP *ep;
	  uint16_t epnum = 0;
 
	 while ( ep_intr )
    {
    	  ep = &pdev->dev.in_ep[epnum];
          /* Setup and start the Transfer */
          ep->is_in = 1;
          ep->num = epnum;  
        if (ep_intr & 0x01) /* In ITR */
       {
	if (pdev->dev.in_ep[epnum].rem_data_len == 0)
	{
			
			if(pdev->dev.zero_replay_flag )
			{
				USB_OTG_EPReply_Zerolen(pdev,ep);
				pdev->dev.zero_replay_flag =0;
			}
		return 0;
	}
   else
   {
		 if (pdev->dev.in_ep[epnum].xfer_len - pdev->dev.in_ep[epnum].xfer_count >= pdev->dev.in_ep[epnum].maxpacket)
                    {
//                    	MyPrintf("tg\n\r");
                        USB_OTG_WritePacket (pdev, 
                                             pdev->dev.in_ep[epnum].xfer_buff + pdev->dev.in_ep[epnum].xfer_count,
                                             epnum, 
                                             pdev->dev.in_ep[epnum].maxpacket);
                         USB_OTG_TRIG(pdev ,ep );
                        pdev->dev.in_ep[epnum].xfer_count += pdev->dev.in_ep[epnum].maxpacket;
                        pdev->dev.in_ep[epnum].rem_data_len = pdev->dev.in_ep[epnum].xfer_len - pdev->dev.in_ep[epnum].xfer_count;
												pdev->dev.zero_replay_flag =1;
//                        txcsrl.b.tx_pkt_rdy = 1;
//                        USB_OTG_WRITE_REG8(&pdev->regs.INDEXREGS->CSRL.CSR0L,txcsrl.d8);
//		      USB_OTG_TRIG(pdev ,ep );
//												MyPrintf("tx xfer go on.\n\r");
                    }
                    else
                    {
//                       MyPrintf("tc\n\r");	
                        USB_OTG_WritePacket (pdev, 
                                             pdev->dev.in_ep[epnum].xfer_buff + pdev->dev.in_ep[epnum].xfer_count,  
                                             epnum,
                                             pdev->dev.in_ep[epnum].xfer_len - pdev->dev.in_ep[epnum].xfer_count);
                            USB_OTG_TRIG(pdev ,ep );                 
                        pdev->dev.in_ep[epnum].xfer_count = pdev->dev.in_ep[epnum].xfer_len;
                        pdev->dev.in_ep[epnum].rem_data_len = 0;
//                        txcsrl.b.tx_pkt_rdy = 1;
//                        USB_OTG_WRITE_REG8(&pdev->regs.INDEXREGS->CSRL.CSR0L,txcsrl.d8);
                        /* TX COMPLETE */
                        USBD_DCD_INT_fops->DataInStage(pdev , epnum);
												pdev->dev.zero_replay_flag =0;
//                        USB_OTG_TRIG(pdev ,ep );
//                       MyPrintf("tx xfer continue.\n\r");
                        
                    }
        }
    }
   
   epnum++;
   ep_intr >>= 1;
   }
   return 1;
}


/**
* @brief  DCD_HandleOutEP_ISR
*         Indicates that an OUT EP has a pending Interrupt
* @param  pdev: device instance
* @retval status
*/
static uint32_t DCD_HandleOutEP_ISR(USB_OTG_CORE_HANDLE *pdev, uint16_t ep_intr)
{
//    USB_OTG_DEPXFRSIZ_TypeDef  deptsiz;
//   USB_OTG_EP_LENREGS rx_count;
    uint32_t epnum = 1;
    uint8_t rx_count;
    ep_intr >>= 1;
    while ( ep_intr )
    {
        if (ep_intr & 0x1)
        {
            rx_count = USB_OTG_READ_REG8(&pdev ->regs.LENREGS[epnum]);
              if (rx_count )
             {
             	USBD_DCD_INT_fops->DataOutStage(pdev , epnum);
             }
             
//                {
//                    USB_OTG_ReadPacket (pdev, 
//                                        pdev->dev.out_ep[epnum].xfer_buff + pdev->dev.out_ep[epnum].xfer_count,
//                                        epnum, 
//                                        pdev->dev.out_ep[epnum].maxpacket);
//                    pdev->dev.out_ep[epnum].xfer_count += pdev->dev.out_ep[epnum].maxpacket;
 //                   rxcsrl.b.rx_pkt_rdy = 0;
//                    USB_OTG_WRITE_REG8(&pdev->regs.CSRREGS[epnum]->RXCSRL,rxcsrl.d8);
//                    if (pdev->dev.out_ep[epnum].xfer_count >= pdev->dev.out_ep[epnum].xfer_len)
//                    {
//		MyPrintf(" Enter USBD_DCD_INT_fops->DataOutStage(pdev , epnum);\r\n");
//                        /* RX COMPLETE */
//                        USBD_DCD_INT_fops->DataOutStage(pdev , epnum);
//                    }
//                }
//              else
//                {
//                    USB_OTG_ReadPacket (pdev, 
//                                        pdev->dev.out_ep[epnum].xfer_buff + pdev->dev.out_ep[epnum].xfer_count,
//                                        epnum, 
//                                        rx_count);
//                    pdev->dev.out_ep[epnum].xfer_count += rx_count;
//                    if (pdev->dev.out_ep[epnum].xfer_len >=  pdev->dev.out_ep[epnum].xfer_count)
//                    {
//                        pdev->dev.out_ep[epnum].xfer_len = pdev->dev.out_ep[epnum].xfer_count;
//                    }
//                    else
//                    {
//                        pdev->dev.out_ep[epnum].xfer_count = pdev->dev.out_ep[epnum].xfer_len;
//                    }
//              //      rxcsrl.b.rx_pkt_rdy = 0;
//            //        USB_OTG_WRITE_REG8(&pdev->regs.CSRREGS[epnum]->RXCSRL, rxcsrl.d8);
//                    /* RX COMPLETE */
//                    USBD_DCD_INT_fops->DataOutStage(pdev , epnum);
//                }
	 
      }
                 /* Endpoint disable  */
         epnum++;
         ep_intr >>= 1;
    }
    return 1;
}



/**
* @brief  DCD_HandleUsbReset_ISR
*         This interrupt occurs when a USB Reset is detected
* @param  pdev: device instance
* @retval status
*/
extern USB_OTG_CORE_HANDLE  USB_OTG_dev;
static uint32_t DCD_HandleUsbReset_ISR()
{
	
		 USB_OTG_WRITE_REG8(CORE_USB_ADDR,0x00);
		 USB_OTG_WRITE_REG8(CORE_USB_STATUS,0xFF);
//	 MyPrintf("YICHIP SCPU USB CCID demo V1.0.  reset \n");
	 return 1;
}























