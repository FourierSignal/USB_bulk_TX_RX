/*
 * @brief HID generic example
 *
 * @note
 * Copyright(C) NXP Semiconductors, 2013
 * All rights reserved.
 *
 * @par
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * LPC products.  This software is supplied "AS IS" without any warranties of
 * any kind, and NXP Semiconductors and its licensor disclaim any and
 * all warranties, express or implied, including all implied warranties of
 * merchantability, fitness for a particular purpose and non-infringement of
 * intellectual property rights.  NXP Semiconductors assumes no responsibility
 * or liability for the use of the software, conveys no license or rights under any
 * patent, copyright, mask work right, or any other intellectual property rights in
 * or to any products. NXP Semiconductors reserves the right to make changes
 * in the software without notification. NXP Semiconductors also makes no
 * representation or warranty that such application will be suitable for the
 * specified use without further testing or modification.
 *
 * @par
 * Permission to use, copy, modify, and distribute this software and its
 * documentation is hereby granted, under NXP Semiconductors' and its
 * licensor's relevant copyrights in the software, without fee, provided that it
 * is used in conjunction with NXP Semiconductors microcontrollers.  This
 * copyright, permission, and disclaimer notice must appear in all copies of
 * this code.
 */

#include "board.h"
#include <stdio.h>
#include <string.h>
#include "app_usbd_cfg.h"
#include "hid_generic.h"

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/

static USBD_HANDLE_T g_hUsb;

extern const  USBD_HW_API_T hw_api;
extern const  USBD_CORE_API_T core_api;
extern const  USBD_HID_API_T hid_api;
/* Since this example only uses HID class link functions for that class only */
static const  USBD_API_T g_usbApi = {
	&hw_api,
	&core_api,
	0,
	0,
	&hid_api,
	0,
	0,
	0x02221101,
};
const  USBD_API_T *g_pUsbApi = &g_usbApi;

/*****************************************************************************
 * Private functions
 ****************************************************************************/

/* Initialize pin and clocks for USB port */
static void usb_pin_clk_init(void)
{
	/* enable USB PLL and clocks */
	Chip_USB_Init();
	/* enable USB 1 port on the board */
	Board_USBD_Init(1);
}

/*****************************************************************************
 * Public functions
 ****************************************************************************/

/**
 * @brief	Handle interrupt from USB
 * @return	Nothing
 */
void USB_IRQHandler(void)
{

	int USB_DevIntSt=0;
	int USB_EpIntSt=0;


	USB_DevIntSt = LPC_USB->DevIntSt;
	USB_EpIntSt= LPC_USB->EpIntSt;



	USBD_API->hw->ISR(g_hUsb);
}

/* Find the address of interface descriptor for given class type. */
USB_INTERFACE_DESCRIPTOR *find_IntfDesc(const uint8_t *pDesc, uint32_t intfClass)
{
	USB_COMMON_DESCRIPTOR *pD;
	USB_INTERFACE_DESCRIPTOR *pIntfDesc = 0;
	uint32_t next_desc_adr;

	pD = (USB_COMMON_DESCRIPTOR *) pDesc;
	next_desc_adr = (uint32_t) pDesc;

	while (pD->bLength) {
		/* is it interface descriptor */
		if (pD->bDescriptorType == USB_INTERFACE_DESCRIPTOR_TYPE) {

			pIntfDesc = (USB_INTERFACE_DESCRIPTOR *) pD;
			/* did we find the right interface descriptor */
			if (pIntfDesc->bInterfaceClass == intfClass) {
				break;
			}
		}
		pIntfDesc = 0;
		next_desc_adr = (uint32_t) pD + pD->bLength;
		pD = (USB_COMMON_DESCRIPTOR *) next_desc_adr;
	}

	return pIntfDesc;
}



//char EP_send_data[25]="When an OUT packet sent by the host has been received successfully, an internal hardware FIFO status Buffer_Full flag is set";

char EP_send_data[64]="abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ**0123456789";
int index=0;
char EP_data[8];
/* bulk EP_IN and EP_OUT endpoints handler */
static ErrorCode_t EP_bulk_hdlr(USBD_HANDLE_T hUsb, void *data, uint32_t event)
{

	uint32_t count = 0;
    int USB_EpIntSt = 0;
    int USB_DevIntSt =0;
	USB_EpIntSt=LPC_USB->EpIntSt;
	USB_DevIntSt=LPC_USB->DevIntSt;
	switch (event) {
	/* A transfer from us to the USB host that we queued has completed. */
	case USB_EVT_IN:
	{
		//char EP_data[8]="hellocue";
//		char EP_data[8];
#if 1
		if(index < 64)
		{
		   memcpy(EP_data,(EP_send_data+index),8);
		   index = index + 8;
		   USBD_API->hw->WriteEP(g_hUsb, CUSTOM_EP_IN, EP_data,8);
		}
		else
		{
			return ERR_USBD_UNHANDLED;
		}

#endif
		USB_EpIntSt=LPC_USB->EpIntSt;
		USB_DevIntSt=LPC_USB->DevIntSt;
        break;
	}

	case USB_EVT_OUT:
	{
		char usb_rx_data[8]={0,0,0,0,0,0,0,0};
        int ret=0;
	    ret=USBD_API->hw->ReadEP(g_hUsb,CUSTOM_EP_OUT,usb_rx_data);
		break;
	}

	case USB_EVT_IN_NAK :
	{
		int ret=0;
		if(index < 64)
		{
		   memcpy(EP_data,(EP_send_data+index),8);
		   index = index + 8;
		   USBD_API->hw->WriteEP(g_hUsb, CUSTOM_EP_IN, EP_data,8);
		}
		break;
	}

	case USB_EVT_OUT_NAK:
	{
		int ret=0;
		break;
	}

	case USB_EVT_OUT_STALL:
	{
		int ret=0;
		break;
	}



	default:
		break;
	}

	return LPC_OK;
}


/**
 * @brief	main routine for blinky example
 * @return	Function should not exit.
 */
int main(void)
{
	USBD_API_INIT_PARAM_T usb_param;
	USB_CORE_DESCS_T desc;
	ErrorCode_t ret = LPC_OK;

    /* Initialize board and chip */
	SystemCoreClockUpdate();
	Board_Init();

	/* enable clocks and pinmux */
//	usb_pin_clk_init();

	Chip_Clock_EnablePeriphClock(SYSCTL_CLOCK_USB);
	Chip_Clock_SetupPLL(SYSCTL_USB_PLL, 3, 1);	/* Multiply by 4, Divide by 2 */
	/* Enable PLL1 */
	Chip_Clock_EnablePLL(SYSCTL_USB_PLL, SYSCTL_PLL_ENABLE);
	/* Wait for PLL1 to lock */
	while (!Chip_Clock_IsUSBPLLLocked()) {}
	/* Connect PLL1 */
	Chip_Clock_EnablePLL(SYSCTL_USB_PLL, SYSCTL_PLL_ENABLE | SYSCTL_PLL_CONNECT);
	/* Wait for PLL1 to be connected */
	while (!Chip_Clock_IsUSBPLLConnected()) {}

	LPC_USB->USBClkCtrl = 0x12;					/* Dev, PortSel, AHB clock enable */
	while ((LPC_USB->USBClkSt & 0x12) != 0x12) ;
// Enable the USB pin functions by writing to the corresponding PINSEL register.
	Chip_IOCON_PinMux(LPC_IOCON, 0, 29, IOCON_MODE_INACT, IOCON_FUNC1);	/* P0.29 D1+, P0.30 D1- */
	Chip_IOCON_PinMux(LPC_IOCON, 0, 30, IOCON_MODE_INACT, IOCON_FUNC1);
	Chip_IOCON_PinMux(LPC_IOCON, 2, 9, IOCON_MODE_INACT, IOCON_FUNC1); //USB_CONNECT
	/* initialize call back structures */
	memset((void *) &usb_param, 0, sizeof(USBD_API_INIT_PARAM_T));
	usb_param.usb_reg_base = LPC_USB_BASE + 0x200;
//	usb_param.usb_reg_base = LPC_USB_BASE ;  //??? Hardfault
	usb_param.max_num_ep = 6;
	usb_param.mem_base = USB_STACK_MEM_BASE;
	usb_param.mem_size = USB_STACK_MEM_SIZE;

	/* Set the USB descriptors */
	desc.device_desc = (uint8_t *) USB_DeviceDescriptor;
	desc.string_desc = (uint8_t *) USB_StringDescriptor;

	/* Note, to pass USBCV test full-speed only devices should have both
	 * descriptor arrays point to same location and device_qualifier set
	 * to 0.
	 */
	desc.high_speed_desc = USB_FsConfigDescriptor;
	desc.full_speed_desc = USB_FsConfigDescriptor;
	desc.device_qualifier = 0;

	//Endpoint-No as per USB spec:
	/*
				bEndpointAddress  Bits 7    |  Bits 4..6 |  Bits 0..3
				                  Direction |  Reserved  |  Endpoint Number.
				Direction 0 = Out, 1 = In (Ignored for Control Endpoints)

	Phy-EP 4 --- Bulk OUT ===> 04h
	Phy-EP 5 --- Bulk IN  ===> 85h
	*/

	/* USB Initialization */
	ret = USBD_API->hw->Init(&g_hUsb, &desc, &usb_param);
	if (ret == LPC_OK)
	{
			{
				uint32_t ep_indx;
 				/* register endpoint interrupt handler */
     			ep_indx = (((CUSTOM_EP_IN & 0x0F) << 1) + 1);
	    		ret = USBD_API->core->RegisterEpHandler(g_hUsb, ep_indx, EP_bulk_hdlr, NULL);
	    		if (ret == LPC_OK)
	    		{
	    		   /* register endpoint interrupt handler */
			       ep_indx = ((CUSTOM_EP_OUT & 0x0F) << 1);
	    		   ret = USBD_API->core->RegisterEpHandler(g_hUsb, ep_indx, EP_bulk_hdlr, NULL);
		    	   if (ret != LPC_OK)
		    		{
		    			while(1);
		    		}
	    		}
	    		else
	    		{
	    			while(1);
	    		}
			}
	 }
	else
	{
		while(1);
	}


//comment this if INTR on ACK is not needed.
#if 1
			//  ErrorCode_t  (*EnableEvent)(USBD_HANDLE_T hUsb, uint32_t EPNum, uint32_t event_type, uint32_t enable);
			// this API causing USB_EVT_IN_NAK event when no data is sent
				ret = USBD_API->hw->EnableEvent(g_hUsb, CUSTOM_EP_IN, USB_EVT_IN_NAK,1);
				if (ret != LPC_OK)
				{
					while(1);
				}
#endif
			/*  enable USB interrupts */
			NVIC_EnableIRQ(USB_IRQn);
			/* now connect */
			USBD_API->hw->Connect(g_hUsb, 1);

//comment  below and check : you may not be able to send data if INTR on NAK is disabled.
#if 1
             //delay for Enumeration of device to complete.
			//Ensure Enumeration and configuration by host is completed before writing to Ep.
			//	other wise you will be writing to / reading from an unrealized Ep.
			  {
				  int i ,j ;
			      for(i=1000;i>0;i--) {
	                 for(j=10000; j>0;j--)
	            	;
			       }
			  }
#endif


/*
Configure Device (Command: 0xD8, Data: write 1 byte)
A value of 1 written to the register indicates that the device is configured and all the
enabled non-control endpoints will respond. Control endpoints are always enabled and
respond even if the device is not configured, in the default state.

CONF_DEVICE : Device is configured.
All enabled non-control endpoints will respond. This bit is
cleared by hardware when a bus reset occurs. When set, the UP_LED signal is
driven LOW if the device is not in the suspended state (SUS=0)

GPIO port Pin value register FIOxPIN (FIO0PIN to FIO4PIN
			  This register provides the value of port pins that are configured to perform only digital
			  functions. The register will give the logic value of the pin regardless of whether the pin is
			  configured for input or output, or as GPIO or an alternate digital function.

			  If a pin has an analog function as one of its options, the pin state cannot be read if the
			  analog configuration is selected. Selecting the pin as an A/D input disconnects the digital
			  features of the pin. In that case, the pin value read in the FIOxPIN register is not valid

*/


//by the time you reach this point device should be configured by host and Eps should be realized.
//before reaching this point If you send IN/OUT request from host : Write/read of EP may not be successfull.

//if INTR on NAK is  Enabled : comment this section :
#if 0
		  memcpy(EP_data,(EP_send_data+index),8);
		  index= index + 8;
		  USBD_API->hw->WriteEP(g_hUsb, CUSTOM_EP_IN, EP_data, 8);
#endif

for(;;);

}
