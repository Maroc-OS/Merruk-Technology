#Makefile for Encrypt and Decrypt sources

EXTRA_CFLAGS += -I ../modules/drivers/char/brcm/fuse_ril/
EXTRA_CFLAGS += -I ../modules/drivers/char/brcm/fuse_log/

obj-$(CONFIG_BRCM_FUSE_RIL_2153SDB) += \
          aes.o \
          brcm_hw_otp.o \
          crypto_api.o

obj-$(CONFIG_BRCM_FUSE_RIL_2157SDB) += \
          aes.o \
          brcm_hw_otp.o \
          crypto_api.o


obj-$(CONFIG_BRCM_FUSE_RIL_CIB) += \
          aes.o \
          brcm_hw_otp.o \
          crypto_api.o