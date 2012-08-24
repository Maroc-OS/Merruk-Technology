#
# Makefile which builds Broadcom drivers
#
obj-y					+= cacheops.o
obj-$(CONFIG_BCM_AUXADC)		+= bcm_auxadc.o
obj-$(CONFIG_BCM59038_SARADC)		+=bcm59038-saradc.o
obj-$(CONFIG_BCM59038_AUDIO)		+= bcm59038-audio.o
obj-$(CONFIG_BCM_LCD_BTG222432)		+= lcd_common.o
obj-$(CONFIG_BCM_LCD_HX8352_DF2440V8)	+= lcd_common.o
obj-$(CONFIG_BCM_LCD_HX8347_TFT1P5158)	+= lcd_common.o
obj-$(CONFIG_BCM_LCD_BACKLIGHT)         += lcd_backlight.o
obj-$(CONFIG_BCM_PWM)			+= bcm_pwm.o
ifdef CONFIG_OSDAL_SUPPORT
obj-$(CONFIG_BCM_I2SDAI)       += bcm_i2sdai.o   i2s_drv_osdal.o
obj-$(CONFIG_I2S_TEST_ENABLE)      += i2s_test_osdal.o
else
obj-$(CONFIG_BCM_I2SDAI)		+= bcm_i2sdai.o   i2s_drv.o
obj-$(CONFIG_I2S_TEST_ENABLE)		+=i2s_test.o
endif
obj-$(CONFIG_BCM_OTP)			+= bcm_otp.o

ifeq ($(CONFIG_ATHENA),y)
EXTRA_CFLAGS                += -D_ATHENA_
endif

