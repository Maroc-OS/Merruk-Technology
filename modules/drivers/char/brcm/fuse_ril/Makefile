#Makefile for KRIL & CAPI2
#EXTRA_CFLAGS := -Werror

ifdef CONFIG_MFD_MAX8986
EXTRA_CFLAGS += -DPMU_MAX8986
else
EXTRA_CFLAGS += -DPMU_BCM59038
endif

# common definitons for all capi2 related include paths
include ../modules/drivers/char/brcm/fuse_ril/CAPI2_CIB/capi2_include_paths.gmk

EXTRA_CFLAGS += -I ../modules/drivers/char/brcm/fuse_log/
EXTRA_CFLAGS += -I ../modules/drivers/char/brcm/fuse_ipc/ipc_CIB/public/
EXTRA_CFLAGS += -I ../modules/drivers/char/brcm/fuse_rpc/rpc_CIB/public/
EXTRA_CFLAGS += -I ../modules/drivers/char/brcm/fuse_rpc/rpc_CIB/arpc/inc/
#EXTRA_CFLAGS += -I ../modules/drivers/char/brcm/fuse_rpc/rpc_CIB/srpc/inc/
EXTRA_CFLAGS += -I ../modules/drivers/char/brcm/fuse_ril/crypto/

# used to prevent redefinition of U_CHAR in various CIB header files
EXTRA_CFLAGS += -D_BSDTYPES_DEFINED -DTYPEDEF_U_CHAR -DTYPEDEF_U_SHORT
EXTRA_CFLAGS += -DFUSE_APPS_PROCESSOR -DFUSE_DUAL_PROCESSOR_ARCHITECTURE

# video telephony support
ifdef CONFIG_VIDEO_TELEPHONY_ENABLE
EXTRA_CFLAGS += -DVIDEO_TELEPHONY_ENABLE
endif

ifdef CONFIG_OEM_RIL_ENABLE
EXTRA_CFLAGS += -DOEM_RIL_ENABLE
endif


EXTRA_CFLAGS += -DRPC_INCLUDED
EXTRA_CFLAGS += -DUNDER_LINUX
EXTRA_CFLAGS += -D_BCM2153_
EXTRA_CFLAGS += -DUMTS

# AGPS control plane support
ifdef CONFIG_BRCM_AGPS_CONTROL_PLANE_ENABLE
EXTRA_CFLAGS += -DBRCM_AGPS_CONTROL_PLANE_ENABLE
endif

# BCM_EAP_SIM
# 1. CONFIG_BCM_RIL_FOR_EAP_SIM := true , BoardConfig.mk in device/samsung/model_name
# 2. CONFIG_BCM_EAP_SIM in android.mk of wpa_supplicant
CONFIG_BCM_RIL_FOR_EAP_SIM=y

ifdef CONFIG_BCM_RIL_FOR_EAP_SIM
EXTRA_CFLAGS += -DBCM_RIL_FOR_EAP_SIM
endif

obj-$(CONFIG_BRCM_FUSE_RIL_CIB) += \
	bcm_kril_main.o \
	bcm_kril_ioctl.o \
	bcm_kril_cmd_handler.o \
	bcm_kril_capi2_handler.o \
	bcm_kril_notify_handler.o \
	bcm_kril_callctrl_handler.o \
	bcm_kril_suppsvc_handler.o \
	bcm_kril_msg_handler.o \
	bcm_kril_network_handler.o \
	bcm_kril_misc_handler.o \
	bcm_kril_sim_handler.o \
	bcm_kril_stk_handler.o \
	bcm_kril_pdp_handler.o \
    bcm_kril_simlockfun.o \
    bcm_kril_sysrpc_handler.o \
	bcm_cp_cmd_handler.o \
	bcm_kril_agps_handler.o \
	bcm_kril_adcrpc_handler.o \
	k_sril_svcmode_handler.o \
	k_sril_personalization_handler.o \
	k_sril_gprs_handler.o \
	k_sril_imei_handler.o \
	CAPI2_CIB/ \
	crypto/

obj-$(CONFIG_OEM_RIL_ENABLE) += \
	k_sril_handler.o 
	

