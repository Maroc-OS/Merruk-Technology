#
# Makefile for misc devices that really don't fit anywhere else.
#
EXTRA_CFLAGS += -I ../modules/drivers/char/brcm/fuse_log/

obj-$(CONFIG_IBM_ASM)		+= ibmasm/
obj-$(CONFIG_AD525X_DPOT)	+= ad525x_dpot.o
obj-$(CONFIG_AD525X_DPOT_I2C)	+= ad525x_dpot-i2c.o
obj-$(CONFIG_AD525X_DPOT_SPI)	+= ad525x_dpot-spi.o
obj-$(CONFIG_ATMEL_PWM)		+= atmel_pwm.o
obj-$(CONFIG_ATMEL_SSC)		+= atmel-ssc.o
obj-$(CONFIG_ATMEL_TCLIB)	+= atmel_tclib.o
obj-$(CONFIG_ICS932S401)	+= ics932s401.o
obj-$(CONFIG_LKDTM)		+= lkdtm.o
obj-$(CONFIG_TIFM_CORE)       	+= tifm_core.o
obj-$(CONFIG_TIFM_7XX1)       	+= tifm_7xx1.o
obj-$(CONFIG_PHANTOM)		+= phantom.o
obj-$(CONFIG_ANDROID_PMEM)	+= pmem.o
obj-$(CONFIG_SGI_IOC4)		+= ioc4.o
obj-$(CONFIG_ENCLOSURE_SERVICES) += enclosure.o
obj-$(CONFIG_KERNEL_DEBUGGER_CORE)	+= kernel_debugger.o
obj-$(CONFIG_KGDB_TESTS)	+= kgdbts.o
obj-$(CONFIG_SGI_XP)		+= sgi-xp/
obj-$(CONFIG_SGI_GRU)		+= sgi-gru/
obj-$(CONFIG_CS5535_MFGPT)	+= cs5535-mfgpt.o
obj-$(CONFIG_HP_ILO)		+= hpilo.o
obj-$(CONFIG_ISL29003)		+= isl29003.o
obj-$(CONFIG_SENSORS_TSL2550)	+= tsl2550.o
obj-$(CONFIG_EP93XX_PWM)	+= ep93xx_pwm.o
obj-$(CONFIG_DS1682)		+= ds1682.o
obj-$(CONFIG_TI_DAC7512)	+= ti_dac7512.o
obj-$(CONFIG_UID_STAT)		+= uid_stat.o
obj-$(CONFIG_IFACE_STAT)	+= iface_stat.o
obj-$(CONFIG_C2PORT)		+= c2port/
obj-$(CONFIG_IWMC3200TOP)      += iwmc3200top/
obj-y				+= eeprom/
obj-y				+= cb710/
obj-$(CONFIG_VMWARE_BALLOON)	+= vmware_balloon.o
obj-$(CONFIG_WL127X_RFKILL)	+= wl127x-rfkill.o
obj-$(CONFIG_APANIC)		+= apanic.o
obj-y				+= bcm_reset_utils.o
obj-$(CONFIG_BRCM_CKBLOCK_READER) += ckblock_reader.o
obj-$(CONFIG_BRCM_CP_CRASH_DUMP) += cp_crash.o
obj-$(CONFIG_SENSORS_AK8975)	+= akm8975.o
obj-$(CONFIG_SENSORS_BMA222)	+= bma222_driver.o bma222.o 
obj-$(CONFIG_SENSORS_GP2A)	+= gp2a_prox.o 
obj-$(CONFIG_SENSORS_TAOS)	+= taos.o 
obj-$(CONFIG_SENSORS_MMC328X) += mmc328x.o mecs.o
obj-$(CONFIG_INPUT_YAS_MAGNETOMETER) += yas_mag_kernel_driver.o 
obj-$(CONFIG_INPUT_YAS_ACCELEROMETER) += yas_acc_kernel_driver.o 
obj-$(CONFIG_INPUT_YAS_ORIENTATION) += yas_ori_kernel_driver.o 
obj-$(CONFIG_BLX)		+= blx.o
obj-$(CONFIG_MAX8986_AUDIO)	+= max8986-audio.o
obj-$(CONFIG_BCM59035_PWM)	+= bcm59035_pwm.o
obj-$(CONFIG_MAX8986_MUIC)	+= max8986-muic.o

