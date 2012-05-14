ifeq ($(CONFIG_ATHENA),y)
EXTRA_CFLAGS                            += -D_ATHENA_
EXTRA_CFLAGS				+= -DCHAL_NDEBUG_BUILD
endif

ifeq ($(CONFIG_ARCH_BCM21553_A0),y)
EXTRA_CFLAGS				+= -DCHIP_REVISION=10
endif
ifeq ($(CONFIG_ARCH_BCM21553_B0),y)
EXTRA_CFLAGS				+= -DCHIP_REVISION=20
endif
ifeq ($(CONFIG_ARCH_BCM21553_B1),y)
EXTRA_CFLAGS				+= -DCHIP_REVISION=20
endif

obj-y	+= chal_dma.o
obj-y   += chal_lcdc.o
obj-y   += chal_cam.o
