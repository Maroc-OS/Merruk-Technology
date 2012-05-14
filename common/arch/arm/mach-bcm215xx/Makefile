#
# Makefile for the linux kernel, BCM21553 machine.
#
obj-$(CONFIG_COMMON_CLKDEV)			+= clock-21553.o
obj-$(CONFIG_BOARD_ATHENARAY_EDN20)		+= board-athenarayEDN20.o
obj-$(CONFIG_BOARD_THUNDERBIRD) += board-thunderbird.o
obj-$(CONFIG_BOARD_THUNDERBIRD_EDN31) += board-thunderbirdEDN31.o
obj-$(CONFIG_BOARD_THUNDERBIRD_EDN5x) += board-thunderbirdEDN5x.o
obj-$(CONFIG_BOARD_TOTORO) += board-totoro.o
obj-$(CONFIG_BOARD_LUISA) += board-luisa.o
obj-$(CONFIG_BOARD_TASSVE) += board-tassve.o
obj-$(CONFIG_BOARD_COOPERVE) += board-cooperve.o
obj-$(CONFIG_BCM215XX_PM)      += bcm215xx_sleep.o bcm215xx_pm.o
obj-$(CONFIG_OSDAL_SUPPORT) += osdal_21553_clock.o
obj-y  += cpu-bcm21553.o device.o mm.o

obj-$(CONFIG_USB_GADGET_DWC_OTG) += lm.o
obj-$(CONFIG_ARM1136JFS_CP15_DEBUG)	+= arm1136jfs_cp15_debug.o
obj-$(CONFIG_CPU_FREQ_GOV_BCM21553)	+= cpufreq_bcm21553.o
obj-$(CONFIG_BCM21553_CPU_IDLE_DRV)	+= cpuidle_bcm21553.o
obj-$(CONFIG_SEC_DEBUG)	+= sec_debug.o

