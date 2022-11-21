/************************ (C) COPYRIGHT YICHIP *************************
 * File Name            : usb_core.c
 * Author               : YICHIP
 * Version              : V1.0.0
 * Date                 : 21-May-2019
 * Description          : USB-OTG Core layer.
 *****************************************************************************/
 
/* Include ------------------------------------------------------------------*/
#include "usb_core.h"
#include "usb_bsp.h"
#include "usb_regs.h"
#include "usbd_usr.h"

/* Private typedef ----------------------------------------------------------*/
/* Private define -----------------------------------------------------------*/	
/* Private macro ------------------------------------------------------------*/	
/* Private variables --------------------------------------------------------*/	
/* Ptivate function prototypes ----------------------------------------------*/	

/******************************************************************************
* Function Name  :
* Description    :
* Input          :
* Output         :
* Return         :
*******************************************************************/



/*******************************************************************************
* @brief  USB_OTG_WritePacket : Writes a packet into the Tx FIFO associated 
*         with the EP
* @param  pdev : Selected device
* @param  src : source pointer
* @param  ch_ep_num : end point number
* @param  bytes : No. of bytes
* @retval USB_OTG_STS : status
*/

USB_OTG_STS USB_OTG_WritePacket(USB_OTG_CORE_HANDLE *pdev, 
                                uint8_t             *src, 
                                uint8_t             ch_ep_num, 
                                uint8_t            len)
{
    USB_OTG_STS status = USB_OTG_OK;
    uint8_t i= 0;
  //  MyPrintf("write_len%x\n\r",len);
//	my_delay_ms(1);
 
    #ifdef USB_HID_DEMO_TEST
    if(ch_ep_num == 1)
    {
    	for(uint32_t j=0;j<100000;j++)
		{
			if(USB_OTG_READ_REG8(CORE_USB_FIFO_EMPTY)&0x02)
			{
				break ;
		   	}
		}
    }
   #endif
    if (pdev->cfg.dma_enable == 0)
    {
        for (i = 0; i < len; i++)
        {
        //  * ( pdev->regs.ep[ch_ep_num]) = *src++;
       	//	MyPrintf("src = %02x\n", *src);
          	USB_OTG_WRITE_REG8(pdev->regs.ep[ch_ep_num],*src++);
        }
//        USB_OTG_WRITE_REG8(&pdev->regs.STATUSEGS ->EMPTY, 1<<ch_ep_num);							
    } 

    return status;
}



/**
* @brief  USB_OTG_ReadPacket : Reads a packet from the Rx FIFO
* @param  pdev : Selected device
* @param  dest : Destination Pointer
* @param  bytes : No. of bytes
* @retval None
*/
void USB_OTG_ReadPacket(USB_OTG_CORE_HANDLE *pdev, 
                         uint8_t *dest, 
                         uint8_t ch_ep_num, 
                         uint8_t len)
{
    uint8_t i=0;
    uint8_t count8b = len ;
    uint8_t *data_buff = (uint8_t *)dest;
 //   uint8_t *fifo = pdev->regs.ep[ch_ep_num];
    for (i = 0; i < count8b; i++, data_buff++)
    {
      //  *data_buff = USB_OTG_READ_REG8(pdev->regs.ep[ch_ep_num]);
//				delay_ms(1);
				*data_buff = USB_OTG_READ_REG8(CORE_USB_EP(ch_ep_num));	
    }

		
    /* Return the buffer pointer because if the transfer is composed of several
     packets, the data of the next packet must be stored following the 
     previous packet's data */
//    return ;
}



/**
* @brief  USB_OTG_SelectCore 
*         Initialize core registers address.
* @param  pdev : Selected device
* @param  coreID : USB OTG Core ID
* @retval USB_OTG_STS : status
*/
USB_OTG_STS USB_OTG_SelectCore(USB_OTG_CORE_HANDLE *pdev, 
                               USB_OTG_CORE_ID_TypeDef coreID)
{
       uint32_t i , baseAddress = 0;
       USB_OTG_STS status = USB_OTG_OK;
        #ifdef  USB_OTG_DMA_ENABLE
        pdev->cfg.dma_enable       = 1; 
        #else
        pdev->cfg.dma_enable       = 0;
        #endif

	 /* at startup the core is in FS mode */
       pdev->cfg.speed            = USB_OTG_SPEED_FULL;
       pdev->cfg.mps              = USB_OTG_FS_MAX_PACKET_SIZE ;    

       
       /* initialize device cfg following its address */
    if (coreID == USB_OTG_FS_CORE_ID)
    {
        baseAddress                = USB_OTG_FS_BASE_ADDR;
        pdev->cfg.coreID           = USB_OTG_FS_CORE_ID;
//        pdev->cfg.host_channels    = 8 ;
        pdev->cfg.dev_endpoints    = 4 ;
        pdev->cfg.TotalFifoSize    = 64; /* in 8-bits */
        pdev->cfg.phy_itface       = USB_OTG_EMBEDDED_PHY;     

        #ifdef USB_OTG_FS_SOF_OUTPUT_ENABLED    
        pdev->cfg.Sof_output       = 1;    
        #endif 

        #ifdef USB_OTG_FS_LOW_PWR_MGMT_SUPPORT    
        pdev->cfg.low_power        = 1;    
        #endif     
    }
    else if (coreID == USB_OTG_HS_CORE_ID)
    {
        baseAddress                = USB_OTG_FS_BASE_ADDR;
        pdev->cfg.coreID           = USB_OTG_HS_CORE_ID;    
        pdev->cfg.host_channels    = 8 ;
        pdev->cfg.dev_endpoints    = 4 ;
        pdev->cfg.TotalFifoSize    = 512;/* in 8-bits */

        #ifdef USB_OTG_ULPI_PHY_ENABLED
            pdev->cfg.phy_itface       = USB_OTG_ULPI_PHY;
        #else    
        #ifdef USB_OTG_EMBEDDED_PHY_ENABLED
            pdev->cfg.phy_itface       = USB_OTG_EMBEDDED_PHY;
        #endif  
        #endif      

        #ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED    
            pdev->cfg.dma_enable       = 1;    
        #endif

        #ifdef USB_OTG_HS_SOF_OUTPUT_ENABLED    
            pdev->cfg.Sof_output       = 1;    
        #endif 

        #ifdef USB_OTG_HS_LOW_PWR_MGMT_SUPPORT    
            pdev->cfg.low_power        = 1;    
        #endif 
    }

	/* Common USB Registers */
	pdev ->regs.CTRLREGS = (USB_OTG_CTRLREGS *)CORE_USB_CONFIG;
	pdev ->regs.xferctl =(USB_OTG_XFERCTL*)CORE_USB_TRIG;
	  /* FIFOS */
    for (i = 0; i < NUM_EP_FIFO; i++)
    {
        pdev->regs.ep[i] = (uint8_t *)(CORE_USB_EP(i));
    }
    /* fifo len*/
     for (i = 0; i < NUM_EP_FIFO; i++)
    {
        pdev->regs.LENREGS[i] = (uint8_t *)(CORE_USB_EP_LEN(i));
    }
	pdev ->regs.STATUSEGS= (USB_OTG_STATUSREGS *)CORE_USB_STALL_STATUS;
//	MyPrintf(" %x\n",&(pdev ->regs.CTRLREGS ->STALL) );
//	MyPrintf(" %x\n",USB_OTG_READ_REG8(pdev ->regs.CTRLREGS));
//	MyPrintf(" %x\n",pdev ->regs.ep[0] );
//	MyPrintf(" %x\n",pdev ->regs.LENREGS[0]);
    return status;
}


/**
* @brief  USB_OTG_CoreInit
*         Initializes the USB_OTG controller registers and prepares the core
*         device mode or host mode operation.
* @param  pdev : Selected device
* @retval USB_OTG_STS : status
*/
USB_OTG_STS USB_OTG_CoreInit(USB_OTG_CORE_HANDLE *pdev)
{
    USB_OTG_STS status = USB_OTG_OK;
//    USB_OTG_GUSBCFG_TypeDef  usbcfg;
//    USB_OTG_GCCFG_TypeDef    gccfg;
//    USB_OTG_GAHBCFG_TypeDef  ahbcfg;

//    usbcfg.d32 = 0;
//    gccfg.d32 = 0;
//    ahbcfg.d32 = 0;
//  
//    /* FS interface*/ 
//    /* Reset after a PHY select and set Host mode */
//    USB_OTG_CoreReset(pdev);
//    /* Deactivate the power down*/
//    gccfg.d32 = 0;
//    gccfg.b.pwdn = 1;
//    
//    gccfg.b.vbussensingA = 1 ;
//    gccfg.b.vbussensingB = 1 ;     
//#ifndef VBUS_SENSING_ENABLED
//    gccfg.b.disablevbussensing = 1; 
//#endif    
//    
//    if(pdev->cfg.Sof_output)
//    {
//        gccfg.b.sofouten = 1;  
//    }
//    
//    USB_OTG_WRITE_REG32 (&pdev->regs.GREGS->GCCFG, gccfg.d32);
//    USB_OTG_BSP_mDelay(20);
//    /* case the HS core is working in FS mode */
//    if(pdev->cfg.dma_enable == 1)
//    {
//        ahbcfg.d32 = USB_OTG_READ_REG32(&pdev->regs.GREGS->GAHBCFG);
//        ahbcfg.b.hburstlen = 5; /* 64 x 32-bits*/
//        ahbcfg.b.dmaenable = 1;
//        USB_OTG_WRITE_REG32(&pdev->regs.GREGS->GAHBCFG, ahbcfg.d32);
//    }
//    /* initialize OTG features */
//#ifdef  USE_OTG_MODE
//    usbcfg.d32 = USB_OTG_READ_REG32(&pdev->regs.GREGS->GUSBCFG);
//    usbcfg.b.hnpcap = 1;
//    usbcfg.b.srpcap = 1;
//    USB_OTG_WRITE_REG32(&pdev->regs.GREGS->GUSBCFG, usbcfg.d32);
//    USB_OTG_EnableCommonInt(pdev);
//#endif
    return status;
}



/**
* @brief  USB_OTG_SetCurrentMode : Set ID line
* @param  pdev : Selected device
* @param  mode :  (Host/device)only device
* @retval USB_OTG_STS : status
*/
USB_OTG_STS USB_OTG_SetCurrentMode(USB_OTG_CORE_HANDLE *pdev , uint8_t mode)
{
    USB_OTG_STS status = USB_OTG_OK;
//    USB_OTG_POWER_TypeDef  power;
//    USB_OTG_DEVCTL_TypeDef devctl;
    
//    power.d8 = 0;
//    devctl.d8 = USB_OTG_READ_REG8(&pdev->regs.DYNFIFOREGS->DEVCTL);
//    
    if ( mode == HOST_MODE)
    {
//        power.b.en_suspendM = 1;
//        USB_OTG_WRITE_REG8(&pdev->regs.COMMREGS->POWER, power.d8);
//        devctl.b.host_mode = 1;
//        devctl.b.session = 1;
//        USB_OTG_WRITE_REG8(&pdev->regs.DYNFIFOREGS->DEVCTL, devctl.d8);
    }
    else if ( mode == DEVICE_MODE)
    {
//        devctl.b.host_mode = 0;
//        power.b.en_suspendM = 1;
//        devctl.b.session = 0;
//        power.b.soft_conn = 1;
//        USB_OTG_WRITE_REG8(&pdev->regs.DYNFIFOREGS->DEVCTL, devctl.d8);
//        USB_OTG_WRITE_REG8(&pdev->regs.COMMREGS->POWER, power.d8);
        #ifdef USE_DEVICE_MODE
        pdev->dev.out_ep[0].xfer_buff = pdev->dev.setup_packet;
        pdev->dev.out_ep[0].xfer_len = 8;
        #endif
    }

	delay_us(50);
   // USB_OTG_BSP_mDelay(50);
	
    return status;
}

/**
* @brief  USB_OTG_EPActivate : Activates an EP
* @param  pdev : Selected device
* @retval USB_OTG_STS : status
*/
USB_OTG_STS USB_OTG_EPActivate(USB_OTG_CORE_HANDLE *pdev , USB_OTG_EP *ep)
{
    USB_OTG_STS status = USB_OTG_OK;
    USB_OTG_IRQ_MASK1_TypeDef intr_rxtxe;
//    volatile uint16_t *addr;
    /* Read DEPCTLn register */
    if (ep->is_in == 1)
    {
//        addr = &pdev->regs.COMMREGS->INTRTXE;
        intr_rxtxe.d8 = 1 << ep->num;
        USB_OTG_MODIFY_REG8(&pdev->regs.CTRLREGS->IRQ_MASK1, intr_rxtxe.d8, 0);
    }
    else
    {
//        addr = &pdev->regs.COMMREGS->INTRRXE;
        intr_rxtxe.d8 = 1 << ep->num;
        USB_OTG_MODIFY_REG8(&pdev->regs.CTRLREGS ->IRQ_MASK1, intr_rxtxe.d8, 0);
    }  
    return status;
}


/**
* @brief  USB_OTG_EPDeactivate : Deactivates an EP
* @param  pdev : Selected device
* @retval USB_OTG_STS : status
*/
USB_OTG_STS USB_OTG_EPDeactivate(USB_OTG_CORE_HANDLE *pdev , USB_OTG_EP *ep)
{
    USB_OTG_STS status = USB_OTG_OK;
    USB_OTG_IRQ_MASK1_TypeDef intr_rxtxe;
    USB_OTG_IRQ_MASK2_TypeDef empty_rxtxe;
//    volatile uint16_t *addr;
    /* Read DEPCTLn register */
    if (ep->is_in == 1)
    {
//        addr = &pdev->regs.COMMREGS->INTRTXE;
	 empty_rxtxe.d8 = 1 << ep->num;
        USB_OTG_MODIFY_REG8(&pdev->regs.CTRLREGS ->IRQ_MASK1, 0, empty_rxtxe.d8);
      
    }
    else
    {
//        addr = &pdev->regs.COMMREGS->INTRRXE;
        intr_rxtxe.d8 = 1 << ep->num;
        USB_OTG_MODIFY_REG8(&pdev->regs.CTRLREGS ->IRQ_MASK1, 0, intr_rxtxe.d8);
     }  
    return status;
}



/**
* @brief  USB_OTG_GetMode : Get current mode
* @param  pdev : Selected device
* @retval current mode
*/
uint8_t USB_OTG_GetMode(USB_OTG_CORE_HANDLE *pdev)
{
    return DEVICE_MODE;
}


/**
* @brief  USB_OTG_IsDeviceMode : Check if it is device mode
* @param  pdev : Selected device
* @retval num_in_ep
*/
uint8_t USB_OTG_IsDeviceMode(USB_OTG_CORE_HANDLE *pdev)
{
    return (USB_OTG_GetMode(pdev) != HOST_MODE);
}

/**
* @brief  USB_OTG_EPStartXfer : Handle the setup for data xfer for an EP and 
*         starts the xfer
* @param  pdev : Selected device
* @retval USB_OTG_STS : status
*/

uint8_t data_len;
USB_OTG_STS USB_OTG_EPStartXfer(USB_OTG_CORE_HANDLE *pdev , USB_OTG_EP *ep)
{
    USB_OTG_STS status = USB_OTG_OK;
    uint8_t  rx_count;
    /* IN endpoint */
	
    if (ep->is_in == 1)
		{
					 if ((pdev->cfg.dma_enable == 0) || ((USB_OTG_DEV_DMA_EP_NUM & 0x07) != ep->num))
					 {
										ep->rem_data_len = ep->xfer_len - ep->xfer_count;
									if(ep->rem_data_len == ep->maxpacket)
									{
//										MyPrintf("max\r\n");
										USB_OTG_WritePacket(pdev, 
																							ep->xfer_buff + ep->xfer_count, 
																							ep->num, 
																							ep->maxpacket); 
													ep->xfer_count += ep->maxpacket;
//													USB_OTG_WRITE_REG8(CORE_USB_FIFO_EMPTY,1 << ep ->num);
//													USB_OTG_WRITE_REG8(CORE_USB_TRIG,1 << ep ->num);
													USB_OTG_TRIG(pdev,ep);
													ep->xfer_count = ep->xfer_len; 
													ep->rem_data_len = 0;
													pdev->dev.zero_replay_flag =1;
									
									}
									/* Zero Length Packet? */
									else if (ep->rem_data_len == 0)
									{
//											MyPrintf("rem_zero\r\n");
											USB_OTG_WritePacket(pdev, 
																					ep->xfer_buff + ep->xfer_count, 
																					ep->num, 
																					0);
											ep->xfer_count = ep->xfer_len;
											ep->rem_data_len = 0; 
			//								tx_csrl.b.tx_pkt_rdy = 1;
				//							USB_OTG_WRITE_REG8(&pdev->regs.CSRREGS[ep->num]->TXCSRL,tx_csrl.d8);
									}
									 else
									{
											if (ep->rem_data_len >ep->maxpacket)
											{
//													  GPIO_CONFIG(3) = GPCFG_OUTPUT_LOW;
													USB_OTG_WritePacket(pdev, 
																							ep->xfer_buff + ep->xfer_count, 
																							ep->num, 
																							ep->maxpacket); 
													USB_OTG_TRIG(pdev,ep);
//													GPIO_CONFIG(3) = GPCFG_OUTPUT_HIGH;
													ep->xfer_count += ep->maxpacket;
//												MyPrintf("x\r\n");
													if (ep->xfer_len >= ep->xfer_count)
													{
															ep->rem_data_len = ep->xfer_len - ep->xfer_count;
													}		 
													else		 
													{
															ep->rem_data_len = 0;
															ep->xfer_count = ep->xfer_len;
													}										
			//										tx_csrl.b.tx_pkt_rdy = 1;
			//										USB_OTG_WRITE_REG8(&pdev->regs.CSRREGS[ep->num]->TXCSRL,tx_csrl.d8);
											}
											else
											{
//													MyPrintf("tx_len %x\n",ep->rem_data_len);
													USB_OTG_WritePacket(pdev, 
																							ep->xfer_buff + ep->xfer_count, 
																							ep->num, 
																							ep->rem_data_len); 
													USB_OTG_TRIG(pdev,ep);
													ep->xfer_count = ep->xfer_len; 
													ep->rem_data_len = 0;								
			//										tx_csrl.b.tx_pkt_rdy = 1;
			//										USB_OTG_WRITE_REG8(&pdev->regs.CSRREGS[ep->num]->TXCSRL,tx_csrl.d8);
											}
									}
						}
		}
		else
		{
				/* OUT endpoint */					
								rx_count = USB_OTG_READ_REG8(pdev->regs.LENREGS[ep ->num]); 
								data_len   = rx_count;
//								MyPrintf("out_packetlen  %d\n",rx_count);
								USB_OTG_ReadPacket(pdev, ep->xfer_buff + ep->xfer_count,ep->num, rx_count);																 
								ep->xfer_count += rx_count;
								if (ep->xfer_len <= ep->xfer_count)
								{
										ep->rem_data_len = ep->xfer_count - ep->xfer_len;
								}
								else
								{
										ep->rem_data_len = 0;
				//						ep->xfer_count = ep->xfer_len;
										ep->xfer_len = ep->xfer_count;
								}
//								rx_csrl.b.rx_pkt_rdy = 0;
//								USB_OTG_WRITE_REG8(&pdev->regs.CSRREGS[ep->num]->RXCSRL,rx_csrl.d8);
//							}
//						}
		}
		return status;
}


/**
* @brief  USB_OTG_EP0StartXfer : Handle the setup for a data xfer for EP0 and 
*         starts the xfer
* @param  pdev : Selected device
* @retval USB_OTG_STS : status
*/
extern uint8_t out0_data_len;
USB_OTG_STS USB_OTG_EP0StartXfer(USB_OTG_CORE_HANDLE *pdev , USB_OTG_EP *ep)
{
    USB_OTG_STS  status = USB_OTG_OK;
    uint8_t rx_count;
//    USB_OTG_CSR0L_IN_PERIPHERAL_TypeDef csr0l;
    uint32_t fifoemptymsk = 0;
//    csr0l.d8 = USB_OTG_READ_REG8(&pdev->regs.INDEXREGS->CSRL.CSR0L);
    /* IN endpoint */
    if (ep->is_in == 1)
    {
        ep->rem_data_len = ep->xfer_len - ep->xfer_count;
        /* Zero Length Packet? */
        if (ep->rem_data_len == 0)
        {
//        	//	send zero packet
           USB_OTG_EPReply_Zerolen(pdev,ep);
            ep->xfer_count = ep->xfer_len;
            ep->rem_data_len = 0;
//            csr0l.b.tx_pkt_rdy = 1;
//            csr0l.b.data_end = 1;
//            USB_OTG_WRITE_REG8(&pdev->regs.INDEXREGS->CSRL.CSR0L,csr0l.d8);
        }
        else
        {
            if (ep->rem_data_len > ep->maxpacket)
            {
                USB_OTG_WritePacket(pdev, 
                                    ep->xfer_buff + ep->xfer_count, 
                                    0, 
                                    ep->maxpacket); 
                ep->xfer_count += ep->maxpacket;
                ep->rem_data_len = ep->xfer_len - ep->xfer_count;
                USB_OTG_TRIG(pdev , ep);
//                csr0l.b.tx_pkt_rdy = 1;
//                USB_OTG_WRITE_REG8(&pdev->regs.INDEXREGS->CSRL.CSR0L,csr0l.d8);
            }
            else
            {
      //          MyPrintf("txbuf = %x\n", ep->xfer_buff);
                USB_OTG_WritePacket(pdev, 
                                    ep->xfer_buff + ep->xfer_count, 
                                    0, 
                                    ep->rem_data_len);  
                ep->xfer_count = ep->xfer_len;    
                ep->rem_data_len = 0;   
                USB_OTG_TRIG(pdev , ep);
//                MyPrintf("usb0 tx\n");
//                csr0l.b.tx_pkt_rdy = 1;
//                csr0l.b.data_end = 1;
//                USB_OTG_WRITE_REG8(&pdev->regs.INDEXREGS->CSRL.CSR0L,csr0l.d8);
            }
        }
//        if (pdev->cfg.dma_enable == 1)
//        {

//        }

        if (pdev->cfg.dma_enable == 0)
        {
            /* Enable the Tx FIFO Empty Interrupt for this EP */
            if (ep->xfer_len > 0)
            {
            }
        }
    }
    else
    {
//        /* Program the transfer size and packet count as follows:
//        * xfersize = N * (maxpacket + 4 - (maxpacket % 4))
        if (ep->xfer_len == 0)
        {

        }
        else
        {
	//		rx_count = USB_OTG_READ_REG8(pdev->regs.LENREGS[0]); 
			if(out0_data_len)
			{
								 rx_count = out0_data_len;
			}
			else
			{
				rx_count = USB_OTG_READ_REG8(pdev->regs.LENREGS[0]);
			}
//								MyPrintf("out_packetlen  %d\n",rx_count);
								USB_OTG_ReadPacket(pdev, ep->xfer_buff + ep->xfer_count,ep->num, ep->xfer_len);	
						        ep->xfer_count =0;
							 ep->xfer_buff = pdev->dev.setup_packet;
			
				
        }

    }
    return status;
}

/**
* @brief  USB_OTG_TRIG : Handle start xfer and set tx trig
* @param  pdev : Selected device
* @retval USB_OTG_STS : status
*/
USB_OTG_STS  USB_OTG_TRIG(USB_OTG_CORE_HANDLE *pdev , USB_OTG_EP *ep)
{
   int i;
   USB_OTG_STS status = USB_OTG_OK;
 //  USB_OTG_trig_TypeDef trig;
 //  trig.d8 = 1 << ep ->num;
 //  USB_OTG_MODIFY_REG8(&pdev->regs.xferctl ->TRIG, 0,trig.d8);

USB_OTG_WRITE_REG8(CORE_USB_STATUS,USB_STATUS_NAK);
//	MyPrintf("status = %x\n",USB_OTG_READ_REG8(CORE_USB_STATUS));
	for(i = 0;i < 10000;i++)
	{
//	MyPrintf("status1 = %x\n",USB_OTG_READ_REG8(CORE_USB_STATUS));
		if(USB_OTG_READ_REG8(CORE_USB_STATUS) & USB_STATUS_NAK)  break;
	}
//	USB_FIFO_EMPTY = 1 << ep;
//	MyPrintf("status2 = %x\n",USB_OTG_READ_REG8(CORE_USB_STATUS));
	USB_OTG_WRITE_REG8(CORE_USB_FIFO_EMPTY,1 << ep ->num);
//	for(i = 0;i < 10000;i++) {
//		USB_TRG =  1 << ep;
	USB_OTG_WRITE_REG8(CORE_USB_TRIG,1 << ep ->num);
 return status;
}
/**
* @brief  USB_OTG_EPSetStall : Set the EP STALL
* @param  pdev : Selected device
* @retval USB_OTG_STS : status
*/
USB_OTG_STS USB_OTG_EPSetStall(USB_OTG_CORE_HANDLE *pdev , USB_OTG_EP *ep)
{
    USB_OTG_STS status = USB_OTG_OK;
   USB_OTG_stall_TypeDef stcsrl;
	if(ep->num ==0)
	{
		USB_OTG_WRITE_REG8(&pdev->regs.xferctl ->STALL, 0x01);
		USB_OTG_EPReply_Zerolen(pdev,ep);
	}
//    if (ep->is_in == 1)
//    {
//        if (ep->num)
//        {
//            stcsrl.d8 = 1 <<  2 * (ep ->num) -1;
//            /* set the stall bit */
//            USB_OTG_MODIFY_REG8(&pdev->regs.xferctl ->STALL, 0,stcsrl.d8);
//        }
//        else
//        {
//            
//            USB_OTG_WRITE_REG8(&pdev->regs.xferctl ->STALL, 0x01);
//        }
//    }
//    else    /* OUT Endpoint */
//    {
//        if (ep->num)
//        {
//             stcsrl.d8 =1 << 2 * (ep ->num) ;
//            USB_OTG_MODIFY_REG8(&pdev->regs.xferctl ->STALL, 0,stcsrl.d8);
//        }
//        else
//        {
//            USB_OTG_WRITE_REG8(&pdev->regs.xferctl ->STALL, 0x01);
//        }
//    }
    
    return status;
}

/**
* @brief  USB_OTG_EPSetStall : ack zero  length packet
* @param  pdev : Selected device
* @retval USB_OTG_STS : status
*/

USB_OTG_STS USB_OTG_EPReply_Zerolen(USB_OTG_CORE_HANDLE *pdev , USB_OTG_EP *ep)
{
	USB_OTG_STS status = USB_OTG_OK;
           USB_OTG_WRITE_REG8(&pdev->regs.xferctl ->TRIG, 0x10 <<(ep ->num) );
    return status;
}



/**
* @brief  USB_OTG_RemoteWakeup : active remote wakeup signalling
* @param  None
* @retval : None
*/
void USB_OTG_ActiveRemoteWakeup(USB_OTG_CORE_HANDLE *pdev)
{
    USB_OTG_CTL_TypeDef power;
    /* Note: If CLK has been stopped,it will need be restarted before 
     * this write can occur. 
     */
    power.d8 = USB_OTG_READ_REG8(&pdev->regs.CTRLREGS ->USB_CTL); 
    power.b.resume = 1;
    power.b.wakeup_enable = 1;
    USB_OTG_WRITE_REG8(&pdev->regs.CTRLREGS ->USB_CTL, power.d8);
    /* The software should leave then this bit set for approximately 10ms
     * (minimum of 2ms, a maximum of 15ms) before resetting it to 0.    
     */
   delay_ms(2);
   delay_us(500);
    power.b.resume = 0;
    power.b.wakeup_enable = 0;
    USB_OTG_WRITE_REG8(&pdev->regs.CTRLREGS ->USB_CTL, power.d8);
}




