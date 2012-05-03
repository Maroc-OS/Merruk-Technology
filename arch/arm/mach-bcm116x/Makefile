#
# Makefile for the linux kernel.
#

# Object file lists.
obj-$(CONFIG_ARCH_BCM2157)  = clock-2157.o
obj-y  +=cpu-bcm215x.o device.o mm.o
obj-$(CONFIG_BOARD_BRAVA_EDN10) += board-brava.o
obj-$(CONFIG_BCM215X_PM) += bcm215x_sleep.o bcm215x_pm.o
obj-$(CONFIG_USB_GADGET_DWC_OTG) += lm.o

