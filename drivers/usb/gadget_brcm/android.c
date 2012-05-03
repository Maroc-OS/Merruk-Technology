/*
 * Gadget Driver for Android
 *
 * Copyright (C) 2008 Google, Inc.
 * Author: Mike Lockwood <lockwood@android.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

/*******************************************************************************************
Copyright 2010 Broadcom Corporation.  All rights reserved.

Unless you and Broadcom execute a separate written software license agreement governing use
of this software, this software is licensed to you under the terms of the GNU General Public
License version 2, available at http://www.gnu.org/copyleft/gpl.html (the "GPL").

Notwithstanding the above, under no circumstances may you combine this software in any way
with any other Broadcom software provided under a license other than the GPL, without
Broadcom's express prior written consent.
*******************************************************************************************/

/* #define DEBUG */
/* #define VERBOSE_DEBUG */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>

#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/utsname.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>

#include <linux/usb/android.h>
#include <linux/usb/ch9.h>
#include <linux/usb/composite.h>
#include <linux/usb/gadget.h>
#include <linux/kthread.h>
#include <linux/mfd/max8986/max8986.h>

#include "f_mass_storage.h"
#include "f_adb.h"
#include "f_ether.h"
#include "f_rndis.h"
#include "f_acm.h"
#include "dwc_otg_cil.h"

#include "gadget_chips.h"
#include "u_ether.h"
#include "u_serial.h"

MODULE_AUTHOR("Mike Lockwood");
MODULE_DESCRIPTION("Android Composite USB Driver");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0");

/* #define TEST_USB_BC10 */
#define BC11_SUPPORT

/* RNDIS Ethernet Settings... */
/*-------------------------------------------------------------------------*/
#define RNDIS_VENDOR_NUM       0x04E8  /* Samsung */
#define RNDIS_PRODUCT_NUM      0x6863  /* Ethernet/RNDIS Gadget */

/* CDC */
/*-------------------------------------------------------------------------*/
#define ACM_VENDOR_NUM       		0x04E8  /* Samsung */
#define ACM_ONLY_PRODUCT_NUM      0x685E  /* ACM only */
//#define ACM_OBEX_PRODUCT_NUM 		0x6860 /* ACM + OBEX */
#define ACM_OBEX_PRODUCT_NUM 		0x685E /* ACM + OBEX */
#define RRCM_PRODUCT_NUM			0xE420
#define SS_MODE

static unsigned char hostaddr[ETH_ALEN];
/*-------------------------------------------------------------------------*/

/* Android Settings.... */
/*-------------------------------------------------------------------------*/
static const char longname[] = "Gadget Android";

/* Default vendor and product IDs, overridden by platform data */
#define VENDOR_ID					0x18D1
#define MSC_PRODUCT_ID				0x0005
#define ADB_PRODUCT_ID				0x0002
#define ENABLE						1
#define DISABLE						0
#define RETRY_ENUM					10
#define RENUM_TIME					3000
#define START_ADB_MODE				4000
#define INT_USB_TIME				1000
#define DISCONNECT_TIME_FOR_PC	10
/* #define SUPPORT_REMOTE_WAKEUP */

enum {
	SET_RNDIS_ETH_USB_CONF = 0,
	SET_ANDROID_USB_CONF,		
	SET_ACM_USB_CONF
};

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

/* USB modes */
enum {
	MSC_ONLY_MOD = 0,
	ADB_MSC_MOD,
	ADB_RNDIS_MOD,
	RESTORE_ADB_MODE,
	CHK_CUR_USB_MOD,
	ADB_RNDIS_MOD_OFF,
	ACM_ONLY_MODE,
#ifdef SS_MODE
	ADB_ENABLE_MODE,
	ADB_DISABLE_MODE,
#endif
	ACM_OBEX_MODE
};

struct android_dev {
	struct usb_gadget *gadget;
	struct usb_composite_dev *cdev;

	int product_id;
	int adb_product_id;
	int version;

	int adb_enabled;
	int nluns;
};

static atomic_t adb_enable_excl;
static struct android_dev *_android_dev;
static unsigned char cur_adb_mode = 0;

/* string IDs are assigned dynamically */

#define STRING_MANUFACTURER_IDX		0
#define STRING_PRODUCT_IDX		1
#define STRING_SERIAL_IDX		2

/* String Table */
static struct usb_string strings_dev[] = {
	/* These dummy values should be overridden by platform data */
	[STRING_MANUFACTURER_IDX].s = "BRCM",
	[STRING_PRODUCT_IDX].s = "Android Phone",
	[STRING_SERIAL_IDX].s = "BRCM2153",
	{}
};

static struct usb_gadget_strings stringtab_dev = {
	.language	= 0x0409,	/* en-us */
	.strings	= strings_dev,
};

static struct usb_gadget_strings *dev_strings[] = {
	&stringtab_dev,
	NULL,
};

static struct usb_device_descriptor device_desc = {
	.bLength              = sizeof(device_desc),
	.bDescriptorType      = USB_DT_DEVICE,
	.bcdUSB               = __constant_cpu_to_le16(0x0200),
	.bDeviceClass         = USB_CLASS_PER_INTERFACE,
#ifdef SS_MODE
	.idVendor             = __constant_cpu_to_le16(ACM_VENDOR_NUM),
	.idProduct            = __constant_cpu_to_le16(ACM_ONLY_PRODUCT_NUM),
#else
	.idVendor             = __constant_cpu_to_le16(VENDOR_ID),
	.idProduct            = __constant_cpu_to_le16(MSC_PRODUCT_ID),
#endif
	
	.bcdDevice            = __constant_cpu_to_le16(0xffff),
	.bNumConfigurations   = 1,
};
/*-------------------------------------------------------------------------*/
/* 							External Functions 								     */
/*-------------------------------------------------------------------------*/
extern int dwc_otg_cil_GetPostConfigCurrentIn2maUnit(void);
extern void dwc_otg_cil_USBInitialized(void);
extern void dwc_otg_pcd_stop_usb(void);
extern USB_Charger_Type_t dwc_otg_cil_Usb_Charger_Detection_BC10(void);
extern USB_Charger_Type_t dwc_otg_cil_Get_Usb_Charger_Type(void);
extern void dwc_otg_cil_Reset_Usb_Charger_Type(void);
extern void dwc_otg_cil_get_BC11_Charger_Type(pmu_muic_chgtyp bc11_chargertype);
extern void Disable_MSC_function(void);
extern void usb_composite_stop(struct usb_gadget *gadget);
/*-------------------------------------------------------------------------*/
/* 							Local Functions 								     */
/*-------------------------------------------------------------------------*/
static void android_reenum_thread(void *data);
static void android_init_usb_thread (void *data);

/* Global variables */
static unsigned char g_retry;
static bool g_need_reenum, g_usb_cable_connected;
static wait_queue_head_t       enum_wait;

enum {
	INIT_USB = 0,
	USB_REENUM
};

/**
* android_kernel_thread(unsigned char thread_mode) - Create and run the kernel thread.
* @thread_mode: specify the thread mode to be processed
*
*/
static void android_kernel_thread (unsigned char thread_mode)
{
	struct task_struct *usb_enum_task = NULL; /*Coverity check*/
	
	switch (thread_mode) {
	case INIT_USB:
		usb_enum_task = kthread_run(android_init_usb_thread, 0, "android_init_usb");
		break;
	case USB_REENUM:
		usb_enum_task = kthread_run(android_reenum_thread, 0, "android_usb_enum");
		break;
	default:
		pr_info("Not support...\n");
		break;
	}
	
	if (IS_ERR(usb_enum_task)) {
		pr_err("Failed to create usb_enum_task -- ERROR=%x\n", PTR_ERR(usb_enum_task));
		usb_enum_task = NULL;
	}
}

/**
 * android_start_usb() - Start USB initialization at the kenel driver initialization
 * Note: the default usb config is MSC only mode and it will initiailize the usb gadget driver.
 *
 */
static void android_start_usb (void)
{
	struct android_dev *dev = _android_dev;

#if !defined(CONFIG_BOARD_THUNDERBIRD_EDN31) && !defined(CONFIG_BOARD_THUNDERBIRD_EDN5x) && !defined(CONFIG_BOARD_TOTORO) && !defined(CONFIG_BOARD_LUISA) && !defined(CONFIG_BOARD_TASSVE) && !defined(CONFIG_BOARD_COOPERVE)
	if (dev->cdev && dev->cdev->gadget)
		usb_gadget_connect(dev->cdev->gadget);
	android_kernel_thread(INIT_USB);
#else
	dwc_otg_cil_USBInitialized();
#endif
}

/**
 * android_force_reenum() - Force to enumeration the USB device
 *
 */
static void android_force_reenum(void)
{
	struct android_dev *dev = _android_dev;

	pr_info("android_force_reenum\n");
	
	if (!g_usb_cable_connected ) {
		pr_info("USB cable is disconnected.....\n");
		return;
	}

	if(g_retry > RETRY_ENUM) {
		pr_info("g_retry > RETRY_ENUM\n");
		return;
	}
	
	/* This is for USB cable unplugged during the driver initialization...
	     So the USB gadget speed is UNKNOWN....*/
	if (dev->cdev && dev->cdev->gadget &&
		dev->cdev->gadget->speed == USB_SPEED_UNKNOWN) {		
#if !defined(CONFIG_BOARD_THUNDERBIRD_EDN31) && !defined(CONFIG_BOARD_THUNDERBIRD_EDN5x) && !defined(CONFIG_BOARD_TOTORO) && !defined(CONFIG_BOARD_LUISA) && !defined(CONFIG_BOARD_TASSVE) && !defined(CONFIG_BOARD_COOPERVE)
		if ( dwc_otg_cil_Usb_Charger_Detection_BC10() == DEDICATED_CHARGER) {
			dwc_otg_pcd_stop_usb();
			dwc_otg_cil_USBInitialized();
			return;	
		}	
		msleep(INT_USB_TIME);
#endif		
	}
	
#if defined(CONFIG_BOARD_THUNDERBIRD_EDN31) || defined(CONFIG_BOARD_THUNDERBIRD_EDN5x) || defined(CONFIG_BOARD_TOTORO) || defined(CONFIG_BOARD_LUISA) || defined(CONFIG_BOARD_TASSVE) || defined(CONFIG_BOARD_COOPERVE)
	if (dev->cdev && dev->cdev->gadget) {
		g_need_reenum = TRUE;
		msleep(DISCONNECT_TIME_FOR_PC); 
		usb_gadget_connect(dev->cdev->gadget);		
		android_kernel_thread(USB_REENUM);
	}
	else
		pr_info("USB drvier is not initialized successfully!!!!!!\n");
#else
	if (dev->cdev && dev->cdev->gadget ) {
		pr_info("Performing USB enum...\n");
		usb_gadget_disconnect(dev->cdev->gadget);
		/* should have the enough delay for some PCs to release the drivers*/
		msleep(DISCONNECT_TIME_FOR_PC); 
		g_need_reenum = TRUE;
		android_kernel_thread(USB_REENUM);
		usb_gadget_connect(dev->cdev->gadget);
	} else
		pr_info("USB driver is not initialized successfully!!!!!!\n");
#endif

}

/**
 * android_init_usb_thread() - Initialize the USB setting for the Android ADB/MSC device.
 * Note: the default usb config is MSC only mode and it will initiailize the usb gadget driver.
 *
 */
static void android_init_usb_thread (void *data)
{

	struct android_dev *dev = _android_dev;
	pr_info("android_init_usb_thread ...\n");
	/* Init the USB.....*/
	if (dev->cdev) { /*Coverity check*/
		if (dev->cdev->gadget) {
		msleep(INT_USB_TIME);
		if (dev->cdev->gadget->speed == USB_SPEED_UNKNOWN) {
			pr_info("speed == USB_SPEED_UNKNOWN...\n");
			usb_gadget_disconnect(dev->cdev->gadget);
			dwc_otg_cil_USBInitialized();
		} else {
			pr_info("speed == %d\n", dev->cdev->gadget->speed);
			g_need_reenum = TRUE;
			android_kernel_thread(USB_REENUM);
			dwc_otg_cil_USBInitialized();
		}

	} else {
		pr_info("Android_init_usb -- failed ...\n");
		pr_info("dev->cdev = %x, dev->cdev->gadget = %x\n",
				dev->cdev, dev->cdev->gadget);
		return;
	}
	}
}

/**
 * android_reenum_thread() - the thread will perform the re-enum if the USB REST is comming after 3 seconds.
 *
 * Note:  Doing so, it is because we want to prevent the current dwc otg driver to handle the IN EP disable interrupt case.
 *		In the IN EP diable interrupt case, it will re-xmit the data but it does not handle well and sometimes, it will go to the suspend state.
 */
static void android_reenum_thread(void *data)
{
	DECLARE_WAITQUEUE(waita, current);
	struct android_dev *dev = _android_dev;
	
	pr_info("android_reenum_thread\n");

	 /* for the WALL charger or USB cable unplugged suddendly... */
	 if (!g_usb_cable_connected) {
		pr_info("g_usb_cable_connected=%d\n", g_usb_cable_connected);				
		usb_gadget_disconnect(dev->cdev->gadget);		
		g_need_reenum = FALSE;
		return;
	}	
	 
	if (g_retry > RETRY_ENUM) {
		pr_info("g_retry > RETRY_ENUM\n");
		usb_gadget_disconnect(dev->cdev->gadget);
		g_need_reenum = FALSE;
		return;
	}		

	if (g_need_reenum == FALSE) {
		pr_info("g_need_reenum = FALSE\n");	
		return;
	}
	
	/* Interruptible timeout so We can stop it at any time before 
		kicking in the new enumeration process*/
	add_wait_queue(&enum_wait, &waita);
	set_current_state(TASK_INTERRUPTIBLE);
	schedule_timeout(3 * HZ);
	remove_wait_queue(&enum_wait, &waita);
	
	if (g_need_reenum) {		
		g_retry++;
		pr_info("g_retry = %d\n", g_retry);		
#if defined(CONFIG_BOARD_THUNDERBIRD_EDN31) || defined(CONFIG_BOARD_THUNDERBIRD_EDN5x) || defined(CONFIG_BOARD_TOTORO) || defined(CONFIG_BOARD_LUISA) || defined(CONFIG_BOARD_TASSVE) || defined(CONFIG_BOARD_COOPERVE)
		usb_gadget_disconnect(dev->cdev->gadget);
#endif
		android_force_reenum();
	} 
	
}

void Android_usb_cable_connection(bool is_connected)
{
	pr_info("Android_USB_Cable_Connection = %d\n", is_connected);
	g_usb_cable_connected = is_connected;
}

/**
 * Android_cancel_reenum() - USB RESET interrupt routine called this function to stop the re-enum if USB RESET
 *							interrupt is triggerred less than 3 seconds. After cable unplugged, call it to stop
 *                                              the re-enum.
 *
 * Note: If USB cable is unplugged, call this function first to stop any potential issue.
 */
void Android_cancel_reenum(void)
{
	pr_info("Android_cancel_reenum\n");
	g_need_reenum = FALSE;
	wake_up(&enum_wait);
}

/**
 * Android_PMU_USB_Start() - PMU call this function to start the USB opertion
 *
 */
void Android_PMU_USB_Start(void)
{		
#ifdef TEST_USB_BC10	
	if ( dwc_otg_cil_Usb_Charger_Detection_BC10() == DEDICATED_CHARGER)	
		return;	
#endif	
#ifdef BC11_SUPPORT
	dwc_otg_cil_get_BC11_Charger_Type(max8986_muic_get_charger_type());	
#endif
	g_retry = 0;
	android_force_reenum();		
}

/**
 * Android_PMU_USB_Stop() - PMU call this function to stop the USB opertion
 *
 */
void Android_PMU_USB_Stop(void)
{		
	struct android_dev *dev = _android_dev;

	pr_info("Android_PMU_USB_Stop\n");
	g_need_reenum = FALSE;
	wake_up(&enum_wait);
	usb_gadget_disconnect(dev->cdev->gadget);
	usb_composite_stop(dev->cdev->gadget);
}

/*
  *Android_Perform_BC10_Dectection() - PMU call this function to detect the USB BC type.
  * 
  *@return the current BC type
  *
  *Note: It must be called before calling Android_PMU_USB_Start()
  *
 */
USB_Charger_Type_t Android_Perform_BC10_Dectection(void)
{
	return dwc_otg_cil_Usb_Charger_Detection_BC10();	
}

/*
  *Android_Get_USB_Charger_type() - PMU call this function to get the current USB BC type 
  * 									without performing hardware detection.
  * 
  *@return the current USB BC type
  *
  *
 */
USB_Charger_Type_t Android_Get_USB_BC_Type(void)
{
	return dwc_otg_cil_Get_Usb_Charger_Type();
}

/*
  *Android_Get_USB_Charger_type() - PMU call this function to set the current USB BC type to 
  *									"NO_CHARGER".
  * 									
 */
void Android_Reset_USB_BC_Type(void)
{
	dwc_otg_cil_Reset_Usb_Charger_Type();
}

/**
 * Android_switch_usb_conf() - switch the USB configuration
 * @new_usb_conf: The value of the USB configuration
 *
 */
int Android_switch_usb_conf (unsigned char new_usb_conf)
{
	static unsigned char cur_usb_conf = 0xff;
	struct android_dev *dev = _android_dev;

	if (new_usb_conf == CHK_CUR_USB_MOD) {
                pr_info("The current usb configuration is %d", cur_usb_conf);
                return cur_usb_conf;
        }

	if (new_usb_conf == RESTORE_ADB_MODE) {
                pr_info("restore adb mode enable = %d", cur_adb_mode );
                new_usb_conf = cur_adb_mode;
        }

	if (cur_usb_conf == new_usb_conf) {
		pr_info("The usb configuration has not been changed!\n");
		return cur_usb_conf;
	}

	g_retry = 0;		
#if defined(CONFIG_BOARD_THUNDERBIRD_EDN31) || defined(CONFIG_BOARD_THUNDERBIRD_EDN5x) || defined(CONFIG_BOARD_TOTORO) || defined(CONFIG_BOARD_LUISA) || defined(CONFIG_BOARD_TASSVE) || defined(CONFIG_BOARD_COOPERVE)
	if (!dev->cdev)  /*Coverity check*/
		return cur_usb_conf;
	if (dev->cdev->gadget) { /*Coverity check*/
			usb_gadget_disconnect(dev->cdev->gadget);	
			msleep(DISCONNECT_TIME_FOR_PC); 
	}
#endif

	switch (new_usb_conf) {
	case ADB_RNDIS_MOD:
		device_desc.idVendor = __constant_cpu_to_le16(RNDIS_VENDOR_NUM);
		device_desc.idProduct = __constant_cpu_to_le16(RNDIS_PRODUCT_NUM);
		device_desc.bDeviceClass         = USB_CLASS_COMM;
		Disable_MSC_function();
		set_current_usb_config(SET_RNDIS_ETH_USB_CONF);
		rndis_interface_enable(ENABLE);		
		dev->adb_enabled = DISABLE;
		dev->cdev->desc.bDeviceClass         = USB_CLASS_COMM;
		break;	
	case MSC_ONLY_MOD:
		device_desc.idVendor = __constant_cpu_to_le16(VENDOR_ID);
		device_desc.idProduct = __constant_cpu_to_le16(MSC_PRODUCT_ID);
		device_desc.bDeviceClass         = USB_CLASS_MASS_STORAGE;
		set_current_usb_config(SET_ANDROID_USB_CONF);
		adb_interface_enable(DISABLE);		
		dev->cdev->desc.bDeviceClass         = USB_CLASS_MASS_STORAGE;
		dev->adb_enabled = DISABLE;
		adb_function_enable(DISABLE);
		acm_interface_enable(DISABLE);
		obex_interface_enable(DISABLE);	
		obex_function_enable(DISABLE);
		cur_adb_mode = 0;
		break;
	case ADB_MSC_MOD:
#ifdef SS_MODE
		device_desc.idVendor = __constant_cpu_to_le16(ACM_VENDOR_NUM);
		device_desc.idProduct = __constant_cpu_to_le16(ACM_OBEX_PRODUCT_NUM);
		device_desc.bDeviceClass         = USB_CLASS_PER_INTERFACE;
		set_current_usb_config(SET_ANDROID_USB_CONF);
		adb_interface_enable(ENABLE);		
		dev->adb_enabled = ENABLE;
		adb_function_enable(ENABLE);
		acm_interface_enable(ENABLE);
		obex_interface_enable(ENABLE);	
		obex_function_enable(ENABLE);			
#else
		device_desc.idVendor = __constant_cpu_to_le16(VENDOR_ID);
		device_desc.idProduct = __constant_cpu_to_le16(ADB_PRODUCT_ID);
		device_desc.bDeviceClass         = USB_CLASS_PER_INTERFACE;
		set_current_usb_config(SET_ANDROID_USB_CONF);
		adb_interface_enable(ENABLE);		
		dev->adb_enabled = ENABLE;
		adb_function_enable(ENABLE);		
#endif
		dev->cdev->desc.bDeviceClass         = USB_CLASS_PER_INTERFACE;
		cur_adb_mode = 1;
		break;	
	case ACM_ONLY_MODE:
#ifdef SS_MODE
		device_desc.idVendor = __constant_cpu_to_le16(ACM_VENDOR_NUM);
		device_desc.idProduct = __constant_cpu_to_le16(ACM_ONLY_PRODUCT_NUM);
		device_desc.bDeviceClass         = USB_CLASS_PER_INTERFACE;
		set_current_usb_config(SET_ANDROID_USB_CONF);
		adb_interface_enable(ENABLE);		
		dev->adb_enabled = ENABLE;
		adb_function_enable(ENABLE);
		dev->cdev->desc.bDeviceClass         = USB_CLASS_PER_INTERFACE;
		acm_interface_enable(ENABLE);
		obex_interface_enable(DISABLE);	
		obex_function_enable(DISABLE);		
		cur_adb_mode = 1;
#else
		device_desc.idVendor = __constant_cpu_to_le16(ACM_VENDOR_NUM);
		device_desc.idProduct = __constant_cpu_to_le16(ACM_ONLY_PRODUCT_NUM);
		device_desc.bDeviceClass         = USB_CLASS_COMM;
		Disable_MSC_function();
		set_current_usb_config(SET_ACM_USB_CONF);
		acm_interface_enable(ENABLE);
		obex_interface_enable(DISABLE);	
		obex_function_enable(DISABLE);		
		dev->adb_enabled = DISABLE;		
		dev->cdev->desc.bDeviceClass         = USB_CLASS_COMM;
		cur_adb_mode = 0;
#endif		
		break;	
	case ACM_OBEX_MODE:
#ifdef SS_MODE
		device_desc.idVendor = __constant_cpu_to_le16(ACM_VENDOR_NUM);
		device_desc.idProduct = __constant_cpu_to_le16(ACM_OBEX_PRODUCT_NUM);
		device_desc.bDeviceClass         = USB_CLASS_PER_INTERFACE;
		set_current_usb_config(SET_ANDROID_USB_CONF);
		adb_interface_enable(ENABLE);		
		dev->adb_enabled = ENABLE;
		adb_function_enable(ENABLE);
		acm_interface_enable(ENABLE);
		obex_interface_enable(ENABLE);	
		obex_function_enable(ENABLE);
		dev->cdev->desc.bDeviceClass         = USB_CLASS_PER_INTERFACE;
		cur_adb_mode = 1;			
#else
		device_desc.idVendor = __constant_cpu_to_le16(ACM_VENDOR_NUM);
		device_desc.idProduct = __constant_cpu_to_le16(ACM_OBEX_PRODUCT_NUM);
		device_desc.bDeviceClass         = USB_CLASS_PER_INTERFACE;
		Disable_MSC_function();
		set_current_usb_config(SET_ACM_USB_CONF);		
		acm_interface_enable(ENABLE);		
		obex_interface_enable(ENABLE);	
		obex_function_enable(ENABLE);
		dev->adb_enabled = DISABLE;
		dev->cdev->desc.bDeviceClass         = USB_CLASS_PER_INTERFACE;
		cur_adb_mode = 0;		
#endif		
		break;	
#ifdef SS_MODE
	case ADB_DISABLE_MODE:
		pr_info("ADB DISABLE MODE\n");
		set_current_usb_config(SET_ANDROID_USB_CONF);
		adb_interface_enable(DISABLE);
		dev->adb_enabled = DISABLE;
		adb_function_enable(DISABLE);
		break;

	case ADB_ENABLE_MODE:
		pr_info("ADB ENABLE MODE\n");
		set_current_usb_config(SET_ANDROID_USB_CONF);
		adb_interface_enable(ENABLE);
		dev->adb_enabled = ENABLE;
		adb_function_enable(ENABLE);
		break;
#endif 
	default:
		pr_info("The USB configuration is not supported.....\n");
		break;
	}

	dev->cdev->desc.idProduct = device_desc.idProduct;
	dev->cdev->desc.idVendor = device_desc.idVendor;
	cur_usb_conf = new_usb_conf;
	
	if (g_usb_cable_connected == 0) {
		pr_info("USB cable is disconnected.....\n");
		return new_usb_conf;
	}
	if (g_need_reenum == FALSE)
	android_force_reenum();
	else
		pr_info("USB is in the reenum mode...\n");
	
	return new_usb_conf;
}
EXPORT_SYMBOL(Android_switch_usb_conf);

/*-------------------------------------------------------------------------*/

/*
 * We may not have an RNDIS configuration, but if we do it needs to be
 * the first one present.  That's to make Microsoft's drivers happy,
 * and to follow DOCSIS 1.0 (cable modem standard).
 */
static int __init rndis_do_config(struct usb_configuration *c)
{
	/* FIXME alloc iConfiguration string, set it in c->strings */
	int ret;
	pr_info("rndis_do_config: \n");

	ret = rndis_bind_config(c, hostaddr);
	 if (ret)
		return ret;
	return ret;
}

static struct usb_configuration rndis_config_driver = {
	 .label			= "RNDIS",
	.bind			= rndis_do_config,
	.bConfigurationValue	= 2,
	/* .iConfiguration = DYNAMIC */
	.bmAttributes		= USB_CONFIG_ATT_ONE | USB_CONFIG_ATT_SELFPOWER,
	.bMaxPower		= 0xFA, /* 500ma */
};

/*-------------------------------------------------------------------------*/
/*
 * 	CDC Composite Devices 
 */
static int __init cdc_do_config(struct usb_configuration *c)
{
	struct android_dev *dev = _android_dev;
	int	status;

	pr_info("cdc_do_config: cdc composite devices \n");

	status = acm_bind_config(c, 0);
	if (status < 0)
		return status;

	return obex_bind_config(dev->cdev, c, 1);
}

static struct usb_configuration cdc_config_driver = {
	.label			= "CDC Composite Devices",
	.bind			= cdc_do_config,
	.bConfigurationValue	= 3,
	/* .iConfiguration = DYNAMIC */
	.bmAttributes		= USB_CONFIG_ATT_ONE | USB_CONFIG_ATT_SELFPOWER,
	.bMaxPower		= 0xFA, /* 500ma */
};

/*-------------------------------------------------------------------------*/

/*
 * ADB + MSC functions.
 */

static int __init android_bind_config(struct usb_configuration *c)
{
	struct android_dev *dev = _android_dev;
	int ret;
	pr_debug("android_bind_config\n");
	
	ret = mass_storage_function_add(dev->cdev, c, dev->nluns);
	if (ret)
		return ret;

#ifdef SS_MODE
	ret = acm_bind_config(c, 0);
	if (ret)
		return ret;
	ret = obex_bind_config(dev->cdev, c, 1);
	if (ret)
		return ret;
#endif
	return adb_function_add(dev->cdev, c);
}

static struct usb_configuration android_config = {
	.label		= "android",
	.bind		= android_bind_config,
	.bConfigurationValue = 1,
	.bmAttributes	= USB_CONFIG_ATT_ONE | USB_CONFIG_ATT_SELFPOWER,
	.bMaxPower	= 0xFA, /* 500ma */
};

static int __init android_bind(struct usb_composite_dev *cdev)
{
	struct android_dev *dev = _android_dev;
	struct usb_gadget	*gadget = cdev->gadget;
	int			gcnum;
	int			id;
	int			ret;

	pr_info("android_bind %d\n", dev->adb_enabled);

	/* set up network link layer */
	ret = gether_setup(cdev->gadget, hostaddr); /* debug.... */

	if (ret < 0)
		goto fail0;

	/* set up serial link layer */
	ret = gserial_setup(cdev->gadget, 2);

	if (ret < 0)
		goto fail1;

	/* Allocate string descriptor numbers ... note that string
	 * contents can be overridden by the composite_dev glue.
	 */
	id = usb_string_id(cdev);
	if (id < 0)
		return id;
	strings_dev[STRING_MANUFACTURER_IDX].id = id;
	device_desc.iManufacturer = id;

	id = usb_string_id(cdev);
	if (id < 0)
		return id;
	strings_dev[STRING_PRODUCT_IDX].id = id;
	device_desc.iProduct = id;

	id = usb_string_id(cdev);
	if (id < 0)
		return id;
	strings_dev[STRING_SERIAL_IDX].id = id;
	device_desc.iSerialNumber = id;
	
#ifdef SUPPORT_REMOTE_WAKEUP
	if (gadget->ops->wakeup)
		android_config.bmAttributes |= USB_CONFIG_ATT_WAKEUP;
#endif

	/* Get the Max power */
	android_config.bMaxPower = dwc_otg_cil_GetPostConfigCurrentIn2maUnit();

	/* register our configuration */
	ret = usb_add_config(cdev, &android_config);
	if (ret) {
		pr_err("usb_add_config failed -- android_config\n");
		goto fail1;
	}

	ret = usb_add_config(cdev, &rndis_config_driver);
	if (ret) {
		pr_err("usb_add_config failed -- rndis_config_driver\n");
		goto fail1;
	}
#ifndef SS_MODE
	ret = usb_add_config(cdev, &cdc_config_driver);
	if (ret) {
		pr_err("usb_add_config failed -- cdc_config_driver\n");
		goto fail1;
	}	
#endif
	gcnum = usb_gadget_controller_number(gadget);
	if (gcnum >= 0)
		device_desc.bcdDevice = cpu_to_le16(0x0400 + gcnum);
	else {
		/* gadget zero is so simple (for now, no altsettings) that
		 * it SHOULD NOT have problems with bulk-capable hardware.
		 * so just warn about unrcognized controllers -- don't panic.
		 *
		 * things like configuration and altsetting numbering
		 * can need hardware-specific attention though.
		 */
		pr_warning("%s: controller '%s' not recognized\n",
			longname, gadget->name);
		device_desc.bcdDevice = __constant_cpu_to_le16(0x9999);
	}

	usb_gadget_set_selfpowered(gadget);
	dev->cdev = cdev;
	g_need_reenum = FALSE;
	g_retry = 0;
	g_usb_cable_connected = FALSE;
	init_waitqueue_head(&enum_wait);
	android_start_usb();
	return 0;

fail1:
	gserial_cleanup();
fail0:
	gether_cleanup();
	return ret;
}

static void enable_adb(struct android_dev *dev, int enable)
{	
	pr_info("enable_adb: enable = %d\n", enable);
	
	if (enable == ADB_RNDIS_MOD) 
		return;		
	
	/* If the USB is in the enumeration process, clean it up before moving forward... */
	if (g_need_reenum) {
		g_need_reenum = FALSE;
		g_retry = RETRY_ENUM + 1;
		wake_up(&enum_wait);
		msleep(START_ADB_MODE);
	}
	
	/* set product ID to the appropriate value */
	if (enable)
	 	Android_switch_usb_conf(ADB_MSC_MOD);
	else
#ifdef SS_MODE
		Android_switch_usb_conf(ADB_DISABLE_MODE);
#else
		Android_switch_usb_conf(MSC_ONLY_MOD);
#endif

	cur_adb_mode = enable;
}


void android_enable_function(struct usb_function *f, int enable)
{
	struct android_dev *dev = _android_dev;
	
	pr_info("android_enable_function enable = %d\n", enable);
	
	if (!strcmp(f->name, "rndis")) {		
		if (enable)
			Android_switch_usb_conf(ADB_RNDIS_MOD);
		else
			enable_adb(_android_dev, 2);		
	}

}

static struct usb_composite_driver android_usb_driver = {
	.name		= "android_usb",
	.dev		= &device_desc,
	.strings	= dev_strings,
	.bind		= android_bind,
	.enable_function = android_enable_function,
};

static int adb_enable_open(struct inode *ip, struct file *fp)
{
	pr_info("adb_enable_open\n");
	if (atomic_inc_return(&adb_enable_excl) != 1) {
		atomic_dec(&adb_enable_excl);
		return -EBUSY;
	}
	return 0;
}

static int adb_enable_release(struct inode *ip, struct file *fp)
{
	pr_info("disabling adb\n");
	enable_adb(_android_dev, 0);
	atomic_dec(&adb_enable_excl);
	return 0;
}

static int adb_enable_ioctl(struct inode *inode, struct file *fp,
							unsigned int cmd, unsigned long arg)
{
	void __user	*argp = (void __user *)arg;
	int		val;

	
	switch (cmd) {
	case MSC_ONLY_MOD:
		pr_info("enabling adb MSC only mode\n");
		Android_switch_usb_conf(ADB_DISABLE_MODE);
		cur_adb_mode = 0;
		//enable_adb(_android_dev, 0);
		break;
	case ADB_MSC_MOD:
		pr_info("enabling adb ADB mode\n");
		enable_adb(_android_dev, 1);
		break;
	case ADB_RNDIS_MOD:
		pr_info("enabling adb RNDIS mode\n");
		Android_switch_usb_conf(ADB_RNDIS_MOD);
		break;
	case CHK_CUR_USB_MOD:
		pr_info("checking the current USB mod\n");
		val = Android_switch_usb_conf(CHK_CUR_USB_MOD);
		if(put_user(val, (int __user *) argp)) {
			pr_err("put_user for check cur USB mod --- failed\n");
		}			
		break;
	case ADB_RNDIS_MOD_OFF:
		enable_adb(_android_dev, 2);
		break;

	case ADB_DISABLE_MODE:
		Android_switch_usb_conf(ADB_DISABLE_MODE);
		cur_adb_mode = 0;
		break;
	default:
		pr_info("adb ioctl unknown cmd %d\n", cmd);
		break;
	}
	return 0;
}

static struct file_operations adb_enable_fops = {
	.owner =   THIS_MODULE,
	.open =    adb_enable_open,
	.release = adb_enable_release,
	.ioctl = adb_enable_ioctl,
};

static struct miscdevice adb_enable_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "android_adb_enable",
	.fops = &adb_enable_fops,
};

static int __init android_probe(struct platform_device *pdev)
{
	struct android_usb_platform_data *pdata = pdev->dev.platform_data;
	struct android_dev *dev = _android_dev;

	pr_info("android_probe pdata: %p\n", pdata);

	if (pdata) {
		if (pdata->vendor_id)
			device_desc.idVendor =
				__constant_cpu_to_le16(pdata->vendor_id);
		if (pdata->product_id) {
			dev->product_id = pdata->product_id;
			device_desc.idProduct =
				__constant_cpu_to_le16(pdata->product_id);
		}
		if (pdata->adb_product_id)
			dev->adb_product_id = pdata->adb_product_id;
		if (pdata->version)
			dev->version = pdata->version;

		if (pdata->product_name)
			strings_dev[STRING_PRODUCT_IDX].s = pdata->product_name;
		if (pdata->manufacturer_name)
			strings_dev[STRING_MANUFACTURER_IDX].s =
					pdata->manufacturer_name;
		if (pdata->serial_number)
			strings_dev[STRING_SERIAL_IDX].s = pdata->serial_number;
		dev->nluns = pdata->nluns;
	}

	return 0;
}

static struct platform_driver android_platform_driver = {
	.driver = { .name = "android_usb", },
	.probe = android_probe,
};

/*-------------------------------------------------------------------------*/
static int __init android_init(void)
{
	struct android_dev *dev;
	int ret;

	pr_info("android init\n");

	dev = kzalloc(sizeof(*dev), GFP_KERNEL);
	if (!dev)
		return -ENOMEM;

	/* set default values, which should be overridden by platform data */
#ifdef SS_MODE
	dev->product_id = ACM_ONLY_PRODUCT_NUM;
	dev->adb_product_id = ACM_OBEX_PRODUCT_NUM;
#else
	dev->product_id = MSC_PRODUCT_ID;
	dev->adb_product_id = ADB_PRODUCT_ID;
#endif
	_android_dev = dev;

	pr_info("adb android enabled  %d\n", dev->adb_enabled);

	ret = platform_driver_register(&android_platform_driver);
	if (ret)
		return ret;
	ret = misc_register(&adb_enable_device);
	if (ret) {
		platform_driver_unregister(&android_platform_driver);
		return ret;
	}

	ret = usb_composite_register(&android_usb_driver);
	if (ret) {
		misc_deregister(&adb_enable_device);
		platform_driver_unregister(&android_platform_driver);
	}
	return ret;
}

static void  __exit android_cleanup(void)
{
	pr_info("cleanup\n");
	usb_composite_unregister(&android_usb_driver);
	misc_deregister(&adb_enable_device);
	platform_driver_unregister(&android_platform_driver);
	kfree(_android_dev);
	_android_dev = NULL;

}

module_init(android_init);
module_exit(android_cleanup);
