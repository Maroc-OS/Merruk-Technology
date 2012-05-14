#bcm_hal_cam-objs := hal_cam.o
bcm_hal_cam_still_YUV-objs := cam_cntrl_2153.o hal_cam.o hal_cam_CMI_YUVdrv.o
#obj-$(CONFIG_BRCM_HAL_CAM) += bcm_hal_cam.o
obj-$(CONFIG_BRCM_HAL_CAM_WITH_STILL_YUV) += bcm_hal_cam_still_YUV.o
obj-$(CONFIG_BCM_CAM_MT9T111) += camdrv_mt9t111.o  hal_cam_core.o cam_cntrl_2153.o
obj-$(CONFIG_BCM_CAM_STV0986) += camdrv_stv0986.o hal_cam_CMI_drv.o cam_cntrl_2153.o
obj-$(CONFIG_BCM_CAM_OV2655) += camdrv_ov2655.o
obj-$(CONFIG_BCM_CAM_STV0987) += camdrv_stv0987.o hal_cam_core_ath.o
obj-$(CONFIG_BCM_CAM_SR200PC10) += camdrv_totoro.o hal_cam_core_ath.o
obj-$(CONFIG_BCM_CAM_ISX005) += camdrv_luisa.o hal_cam_core_ath.o
obj-$(CONFIG_BCM_CAM_S5K5CCGX) += camdrv_tassve.o hal_cam_core_ath.o
obj-$(CONFIG_BCM_CAM_S5K4ECGX) += camdrv_cooperve.o hal_cam_core_ath.o
#EXTRA_CFLAGS += -I$(KERNEL_DIR)/../../vendor/samsung/modules/camacq 
EXTRA_CFLAGS += -I../modules/drivers/char/brcm/camacq
EXTRA_CFLAGS += -Idrivers/media/video
EXTRA_CFLAGS += -D_LINUX_

