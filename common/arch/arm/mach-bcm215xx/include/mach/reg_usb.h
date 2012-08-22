/*******************************************************************************
* Copyright 2010 Broadcom Corporation.  All rights reserved.
* 
* 	@file	arch/arm/mach-bcm116x/include/mach/reg_usb.h
*
* Unless you and Broadcom execute a separate written software license agreement
* governing use of this software, this software is licensed to you under the
* terms of the GNU General Public License version 2, available at
* http://www.gnu.org/copyleft/gpl.html (the "GPL").
* 
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a license
* other than the GPL, without Broadcom's express prior written consent.
*******************************************************************************/

/*
*
*****************************************************************************
*
*  reg_usb.h
*
*  PURPOSE:
*
*     This file contains definitions for the usb on-the-go registers:
*
*       0x082xxxxx
*
*  NOTES:
*
*****************************************************************************/


#if !defined( __ASM_ARCH_REG_USB_H )
#define __ASM_ARCH_REG_USB_H

/* ---- Include Files ---------------------------------------------------- */

#include <mach/hardware.h>
#include <mach/usbctl.h>

/* ---- Constants and Types ---------------------------------------------- */

#define REG_USB_EP_IN_TX_FIFO( epIdx )  ((volatile u32 *)( HW_USBOTG_BASE + 0x00000 + 0x4000 * (epIdx) ))
#define REG_USB_EP_OUT_RX_FIFO          ((volatile u32 *)( HW_USBOTG_BASE + 0x40000 ))

#define REG_USB_EP_IN_TX_CTRL( epIdx )      __REG32( HW_USBOTG_BASE + 0x44000 + ((epIdx) * 0x10 ))

#define REG_USB_TX_CTRL_FIFO_READY      0x08    // e.g. OTG_DEV_TX_FIFOREADY
#define REG_USB_TX_CTRL_FLUSH_FIFO      0x04    // e.g. OTG_DEV_TX_FLUSHFIFO
#define REG_USB_TX_CTRL_SEND_NAK        0x02
#define REG_USB_TX_CTRL_SEND_STALL      0x01

#define REG_USB_EP_IN_TX_STAT( epIdx )      __REG32( HW_USBOTG_BASE + 0x44004 + ((epIdx) * 0x10 ))

#define REG_USB_TX_STAT_ISO_TX_DONE     0x20
#define REG_USB_TX_STAT_DATA_UNDERRUN   0x10    // e.g. OTG_DEV_TX_UNDERRUN
#define REG_USB_TX_STAT_NAK_SENT        0x08    // e.g. OTG_DEV_TX_NAK_INTR
#define REG_USB_TX_STAT_BELOW_THRESH    0x04    // e.g. OTG_DEV_TX_THRESH_INTR
#define REG_USB_TX_STAT_DATA_SENT       0x02    // e.g. OTG_DEV_TX_STATUS_INTR
#define REG_USB_TX_STAT_STATUS          0x01    // e.g. OTG_DEV_TX_GOOD_STATUS

#define REG_USB_EP_IN_TX_FIFO_SIZE( epIdx ) __REG32( HW_USBOTG_BASE + 0x44008 + ((epIdx) * 0x10 ))  // Size in UInt32's
#define REG_USB_EP_IN_TX_XFER_SIZE( epIdx ) __REG32( HW_USBOTG_BASE + 0x4400C + ((epIdx) * 0x10 ))  // Size in bytes
#define REG_USB_EP_OUT_RX_CTRL( epIdx )     __REG32( HW_USBOTG_BASE + 0x44100 + ((epIdx) * 0x10 ))

#define REG_USB_RX_CTRL_FIFO_READY      0x08    // e.g. OTG_DEV_RX_FIFOREADY
#define REG_USB_RX_CTRL_FLUSH_FIFO      0x04    // e.g. OTG_DEV_RX_FLUSHFIFO
#define REG_USB_RX_SEND_STALL           0x01

#define REG_USB_DEV_CONF                    __REG32( HW_USBOTG_BASE + 0x44200 )

#define REG_USB_DEV_CONF_STATUS_1           0x1000
#define REG_USB_DEV_CONF_STATUS             0x0800
#define REG_USB_DEV_CONF_SOFT_DISCONNECT    0x0400
#define REG_USB_DEV_CONF_SCALE_DOWN         0x0200

#define REG_USB_DEV_CONF_HST_MODE           0x0100
#define REG_USB_DEV_CONF_HST_MODE_HOST      0x0000
#define REG_USB_DEV_CONF_HST_MODE_DEVICE    0x0100

#define REG_USB_DEV_CONF_SET_DESC_SUP       0x0080
#define REG_USB_DEV_CONF_CSR_DONE           0x0040
#define REG_USB_DEV_CONF_CSR_PRG_SUP        0x0020
#define REG_USB_DEV_CONF_SYNC_FRAME         0x0010
#define REG_USB_DEV_CONF_SELF_POWERED       0x0008
#define REG_USB_DEV_CONF_REMOTE_WAKEUP      0x0004

#define REG_USB_DEV_CONF_SPEED_MASK         0x0003
#define REG_USB_DEV_CONF_FULL_SPEED         0x0003
#define REG_USB_DEV_CONF_LOW_SPEED          0x0002

#define REG_USB_DEV_STATUS                  __REG32( HW_USBOTG_BASE + 0x44204 )

#define REG_USB_DEV_STATUS_TS( status )     (((status) >> 21 ) & 0x07FF )
#define REG_USB_DEV_STATUS_SUSP             0x00001000
#define REG_USB_DEV_STATUS_ALT( status )    (((status) >> 8 ) & 0x0F )
#define REG_USB_DEV_STATUS_INTF( status )   (((status) >> 4 ) & 0x0F )
#define REG_USB_DEV_STATUS_CFG( status )    ( (status) & 0x0F )

#define REG_USB_DEV_INT_FLAGS               __REG32( HW_USBOTG_BASE + 0x44208 )
#define REG_USB_DEV_INT_ENABLE              __REG32( HW_USBOTG_BASE + 0x4420C )

// The following interrupt indicies are shared with the interrupt flags register
// and the interrupt enable register.

#define REG_USB_DEV_MAX_INT         10

#define REG_USB_DEV_INT_I2C         10      //  matches OTG_DEV_I2C_INTR
#define REG_USB_DEV_INT_OTG         9       //  matches OTG_DEV_OTG_INTR
#define REG_USB_DEV_INT_PORT        8       //  matches OTG_DEV_PORT_INTR
#define REG_USB_DEV_INT_OUT_RX_FIFO 7       //  matches OTG_DEV_RX_FIFO_INTR
#define REG_USB_DEV_INT_SETUP       6       //  matches OTG_DEV_SETUP_INTR
#define REG_USB_DEV_INT_SOF         5       //  matches OTG_DEV_SOF_INTR
#define REG_USB_DEV_INT_US          4       //  matches OTG_DEV_SUSPEND_INTR
#define REG_USB_DEV_INT_UR          3       //  matches OTG_DEV_RESET_INTR
#define REG_USB_DEV_INT_SI          1       //  matches OTG_DEV_SETINF_INTR
#define REG_USB_DEV_INT_SC          0       //  matches OTG_DEV_SETCONF_INTR

#define REG_USB_DEV_OUT_RX_FIFO_SIZE        __REG32( HW_USBOTG_BASE + 0x44210 )
#define REG_USB_DEV_EP_INT_ENABLE           __REG32( HW_USBOTG_BASE + 0x44214 )

#define REG_USB_DEV_OUT_EP_INT_ENABLE( epIdx )  ( 0x10000 << (epIdx) )
#define REG_USB_DEV_IN_EP_INT_ENABLE( epIdx )   ( 1 << (epIdx) )

#define REG_USB_DEV_THRESH                  __REG32( HW_USBOTG_BASE + 0x44218 )

#define REG_USB_DEV_RX_THRESH_MASK          0x03FF0000
#define REG_USB_DEV_TX_THRESH_MASK          0x000003FF
#define REG_USB_DEV_RX_THRESH_SHIFT         16
#define REG_USB_DEV_TX_THRESH_SHIFT         0

#define REG_USB_DEV_GET_RX_THRESH( reg )    (( (reg) >> 16 ) & 0xFFFF )
#define REG_USB_DEV_GET_TX_THRESH( reg )    (  (reg) & 0xFFFF )

#define REG_USB_DEV_OUT_RX_FIFO_STATUS      __REG32( HW_USBOTG_BASE + 0x4421C )

#define REG_USB_DEV_OUT_RX_FIFO_XFER_SIZE( reg )    (((reg) >> 22 ) & 0x3FF )
#define REG_USB_DEV_OUT_RX_FIFO_EP_NUM( reg )       (((reg) >> 16 ) & 0x0F )
#define REG_USB_DEV_OUT_RX_FIFO_OVERRUN         0x000000010     // e.g. OTG_DEV_RX_UNDERRUN
#define REG_USB_DEV_OUT_RX_FIFO_ABOVE_THRESH    0x000000004     // e.g. OTG_DEV_RX_THRESH_INTR
#define REG_USB_DEV_OUT_RX_FIFO_STATUS_COMPLETE 0x000000002     // e.g. OTG_DEV_RX_STATUS_INTR
#define REG_USB_DEV_OUT_RX_FIFO_STATUS_SUCCESS  0x000000001     // e.g. OTG_DEV_RX_GOOD_STATUS


#define REG_USB_DEV_SETUP_RX_FIFO_STATUS    __REG32( HW_USBOTG_BASE + 0x44220 )

#define REG_USB_DEV_SETUP_RX_FIFO_EP_NUM( reg )    (((reg) >> 16 ) & 0x0F )
#define REG_USB_DEV_SETUP_RX_FIFO_SETUP_AFTER_OUT       0x00008000  // e.g. OTG_DEV_SETUP_AFTER_OUT
#define REG_USB_DEV_SETUP_RX_FIFO_SETUP_STATUS_COMPLETE 0x00000002  // e.g. OTG_DEV_SETUP_STATUS_INTR
#define REG_USB_DEV_SETUP_RX_FIFO_SETUP_STATUS          0x00000001  // e.g. OTG_DEV_SETUP_GOOD_STATUS


#define REG_USB_DEV_EP_INT                  __REG32( HW_USBOTG_BASE + 0x44224 )

#define REG_USB_DEV_OUT_EP_INT( epIdx )     ( 0x10000 << (epIdx) )
#define REG_USB_DEV_IN_EP_INT( epIdx )      ( 1 << (epIdx) )

#define REG_USB_DEV_FRAME_NUMBER            __REG32( HW_USBOTG_BASE + 0x44228 )
#define REG_USB_DEV_SETUP_DATA_1            __REG32( HW_USBOTG_BASE + 0x44300 )
#define REG_USB_DEV_SETUP_DATA_2            __REG32( HW_USBOTG_BASE + 0x44304 )

#define REG_USB_DEV_BIU_CTRL                __REG32( HW_USBOTG_BASE + 0x44308 )

#define REG_USB_DEV_BIU_CTRL_WAIT_1         0x01
#define REG_USB_DEV_BIU_CTRL_WAIT_2         0x02

#define REG_USB_DEV_I2C                     __REG32( HW_USBOTG_BASE + 0x4430C )

#define REG_USB_DEV_I2C_BUSY_DONE           0x80000000
#define REG_USB_DEV_I2C_BUSY                0x80000000
#define REG_USB_DEV_I2C_DONE                0

#define REG_USB_DEV_I2C_RW                  0x40000000
#define REG_USB_DEV_I2C_READ                0x40000000
#define REG_USB_DEV_I2C_WRITE               0

#define REG_USB_DEV_I2C_BYTE_2_VALID        0x20000000
#define REG_USB_DEV_I2C_NEW_REG_ADDR        0x10000000

#define REG_USB_DEV_I2C_SET_ADDR( addr )    (( (addr) & 0x7F ) << 16 )
#define REG_USB_DEV_I2C_SET_DATA2( data )   (( (data) & 0xFF ) << 8 )
#define REG_USB_DEV_I2C_SET_DATA1( data )   (  (data) & 0xFF )

#define REG_USB_DEV_I2C_GET_DATA2( reg )    (( (reg) >> 8 ) & 0xFF )
#define REG_USB_DEV_I2C_GET_DATA1( reg )    (  (reg) & 0xFF )

#define REG_USB_DEV_EP_INFO( epIdx )        __REG32( HW_USBOTG_BASE + 0x44404 + ( 4 * (epIdx) ))

#define REG_USB_DEV_EP_SET_MAX_PKT_SIZE( mp )   (( (mp) & 0x3FF ) << 19 )
#define REG_USB_DEV_EP_SET_ALT_SETTING( as )    (( (as) & 0x0F ) << 15 )
#define REG_USB_DEV_EP_SET_INTF_NUM( in )       (( (in) & 0x0F ) << 11 )
#define REG_USB_DEV_EP_SET_CFG_NUM( in )        (( (in) & 0x0F ) << 7 )

#define REG_USB_DEV_EP_GET_MAX_PKT_SIZE( reg )   (( (reg) >> 19 ) & 0x3FF )
#define REG_USB_DEV_EP_GET_ALT_SETTING( reg )    (( (reg) >> 15 ) & 0x0F )
#define REG_USB_DEV_EP_GET_INTF_NUM( reg )       (( (reg) >> 11 ) & 0x0F )
#define REG_USB_DEV_EP_GET_CFG_NUM( reg )        (( (reg) >>  7 ) & 0x0F )

#define REG_USB_DEV_EP_SET_TYPE( typ )        (( (typ) & 0x03 ) << 5 )
#define REG_USB_DEV_EP_GET_TYPE( reg )        (( (reg) >> 5 ) & 0x03 )

#define REG_USB_DEV_EP_TYPE_CONTROL         0
#define REG_USB_DEV_EP_TYPE_ISOCHRONOUS     1
#define REG_USB_DEV_EP_TYPE_BULK            2
#define REG_USB_DEV_EP_TYPE_INTERRUPT       3

#define REG_USB_DEV_EP_SET_DIR( num )       (( (num) & 1 ) << 4 )
#define REG_USB_DEV_EP_GET_DIR( reg )       (( (reg) >> 4 ) & 1 )

#define REG_USB_DEV_EP_DIR_IN               1
#define REG_USB_DEV_EP_DIR_OUT              0

#define REG_USB_DEV_EP_SET_NUM( num )       ( (num) & 0x0F )
#define REG_USB_DEV_EP_GET_NUM( reg )       ( (reg) & 0x0F )

#define REG_USB_HOST_INT_FLAGS              __REG32( HW_USBOTG_BASE + 0x44800 )
#define REG_USB_HOST_INT_ENABLE             __REG32( HW_USBOTG_BASE + 0x44804 )

// The following interrupt indicies are shared with the interrupt flags register
// and the interrupt enable register.

#define REG_USB_HOST_MAX_INT            10

#define REG_USB_HOST_INT_I2C            10
#define REG_USB_HOST_INT_OTG            9
#define REG_USB_HOST_INT_PORT           8
#define REG_USB_HOST_INT_SOF_DUE        3
#define REG_USB_HOST_INT_BELOW_THRESH   2
#define REG_USB_HOST_INT_ABOVE_THRESH   1
#define REG_USB_HOST_INT_STATUS         0

#define REG_USB_MAC_FRAME_INTERVAL          __REG32( HW_USBOTG_BASE + 0x44C34 )
#define REG_USB_MAC_FRAME_REMAINING         __REG32( HW_USBOTG_BASE + 0x44C38 )
#define REG_USB_MAC_FRAME_NUM               __REG32( HW_USBOTG_BASE + 0x44C3C )
#define REG_USB_MAC_PORT_STATUS_CC( port )  __REG32( HW_USBOTG_BASE + 0x44C50 + ( 4 * (port) ))
#define REG_USB_MAC_HOST_TOKEN              __REG32( HW_USBOTG_BASE + 0x44C90 )

#define REG_USB_MAC_CTRL_STATUS             __REG32( HW_USBOTG_BASE + 0x44C94 )

#define REG_USB_MAC_HNP_CAPABLE             ( 1 << 21 )
#define REG_USB_MAC_SRP_CAPABLE             ( 1 << 20 )
#define REG_USB_MAC_HNP_ENABLED             ( 1 << 19 )
#define REG_USB_MAC_HOST_SET_HNP_ENABLED    ( 1 << 18 )
#define REG_USB_MAC_HNP_REQUEST             ( 1 << 17 )
#define REG_USB_MAC_SESSION_REQUEST         ( 1 << 16 )
#define REG_USB_MAC_DEVICE_MODE             ( 1 << 10 )
#define REG_USB_MAC_CONN_ID_CHANGE          ( 1 << 9 )
#define REG_USB_MAC_CONN_ID                 ( 1 << 8 )
#define REG_USB_MAC_HN_DETECT_CHANGE        ( 1 << 7 )
#define REG_USB_MAC_HN_DETECTED             ( 1 << 6 )
#define REG_USB_MAC_SR_DETECT_CHANGE        ( 1 << 5 )
#define REG_USB_MAC_SR_DETECTED             ( 1 << 4 )
#define REG_USB_MAC_HN_SUCCESS_CHANGE       ( 1 << 3 )
#define REG_USB_MAC_HN_SUCCESS              ( 1 << 2 )
#define REG_USB_MAC_SR_SUCCESS_CHANGE       ( 1 << 1 )
#define REG_USB_MAC_SR_SUCCESS              ( 1 << 0 )

// Magic register - has something to do with i2c. Currently, we need to write
// the magic value 0xc10d into this register for i2c to work properly.

#define REG_USB_CONTROL_REG                 __REG32( HW_USBOTG_BASE + 0x80000 )

#define REG_USB_CONTROL_SUSPEND_SOFTMODE                                 (1 << 31)
#define REG_USB_CONTROL_SUSPEND_SOFT                                     (1 << 30)
#define REG_USB_CONTROL_OTGTOP_SPEED_O                                   (1 << 19)
#define REG_USB_CONTROL_OTGTOP_DEV_HOST_N_O                              (1 << 18)
#define REG_USB_CONTROL_OTGTOP_HNP_O                                     (1 << 17)
#define REG_USB_CONTROL_OTGTOP_SRP_O                                     (1 << 16)
#define REG_USB_CONTROL_USB_ON                                           (1 << 15)
#define REG_USB_CONTROL_EX_XVER_SEL                                      (1 << 14)
#define REG_USB_CONTROL_SOFT_12M_RST                                     (1 << 13)
#define REG_USB_CONTROL_SOFT_FIFO_RST                                    (1 << 12)
#define REG_USB_CONTROL_SOFT_48M_RST                                     (1 << 11)
#define REG_USB_CONTROL_SOFT_RXPLL_RST                                   (1 << 10)
#define REG_USB_CONTROL_BI_DIRECTION_EXTERNAL_TRANSCEIVER_BUS_ENABLE     (1 << 8)

#define REG_USB_CONTROL_INTERNAL_TRANSCEIVER_PUP_IDLE                    (1 << 7)
#define REG_USB_CONTROL_INTERNAL_TRANSCEIVER_PUP_ACT                     (1 << 6)
#define REG_USB_CONTROL_INTERNAL_TRANSCEIVER_PDN                         (1 << 5)
#define REG_USB_CONTROL_TRANSCEIVER_I2C_SDA_INVERTED                     (1 << 3)
#define REG_USB_CONTROL_TRANSCEIVER_I2C_SCL_INVERTED                     (1 << 2)
#define REG_USB_CONTROL_APP_DEV_RESUME_I                                 (1)

#endif  /* __ASM_ARCH_REG_USB_H */
