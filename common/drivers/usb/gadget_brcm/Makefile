#
# Makefile for DWC_otg Highspeed USB controller driver
#

#CPPFLAGS	+= -DDEBUG

# Use one of the following flags to compile the software in host-only or
# device-only mode based on the configuration selected by the user

#ifeq ($(CONFIG_USB_DWC_MODE_HOST_ONLY),y)
#	CPPFLAGS        += -DDWC_HOST_ONLY
#else
#ifeq ($(CONFIG_USB_DWC_MODE_DEVICE_ONLY),y)
#	CPPFLAGS        += -DDWC_DEVICE_ONLY
#else
#	CPPFLAGS        += -DDWC_OTG_MODE
#endif
#endif

# don't enable high-power workaround
#KBUILD_CPPFLAGS    += -DOTG_EXT_CHG_PUMP

# Note: By defining DWC_SLAVE, internal DMA mode is disabled.
#       When DWC_SLAVE is not defined, the internal DMA mode is used
#       and even if OTG_PLB_DMA or OTG_PLB_DMA_TASKLET are defined,
#       the PLB DMA mode is not used.

ifeq ($(CONFIG_405EZ),y)
KBUILD_CPPFLAGS        += -Dlinux -DDWC_DEVICE_ONLY
else
# !CONFIG_405EZ -> currently CONFIG_405EX || CONFIG_460EX
ifeq ($(CONFIG_USB_DWC_OTG_INTERNAL_DMA),y)
# with internal DMA
KBUILD_CPPFLAGS	+= -Dlinux
else
# without internal DMA, without PLB DMA
KBUILD_CPPFLAGS	+= -Dlinux -DDWC_SLAVE
endif
# without internal DMA, with PLB DMA
#KBUILD_CPPFLAGS	+= -Dlinux -DDWC_SLAVE -DOTG_PLB_DMA -DOTG_PLB_DMA_TASKLET
endif

KBUILD_CPPFLAGS        += -DCONFIG_PPC4xx_EDMA

obj-$(CONFIG_USB_DWC_OTG)	+= dwc_otg.o

dwc_otg-objs	:= dwc_otg_driver.o dwc_otg_attr.o
dwc_otg-objs	+= dwc_otg_cil.o dwc_otg_cil_intr.o
dwc_otg-objs	+= dwc_otg_pcd.o dwc_otg_pcd_intr.o
dwc_otg-objs	+= dwc_otg_hcd.o dwc_otg_hcd_intr.o dwc_otg_hcd_queue.o
#dwc_otg-objs	+= ppc4xx_dma.o

clean:
	rm -rf   *.o *.ko .*cmd *.mod.c .tmp_versions cscope.out
