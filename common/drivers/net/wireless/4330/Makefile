ifeq ($(SRCBASE),)
SRCBASE := $(shell /bin/pwd)/src
endif

DHDCFLAGS = -Dlinux -DLINUX -DBDC -DBCMDRIVER -DBCMDONGLEHOST -DDHDTHREAD \
	-DDHD_GPL -DDHD_SCHED -DBCMSDIO -DBCMLXSDMMC -DBCMPLATFORM_BUS -DDHD_BCMEVENTS \
	-DSHOW_EVENTS

# BCM21553 specific
DHDCFLAGS += -DDHD_SDALIGN=64 -DMAX_HDR_READ=64 -DDHD_FIRSTREAD=64 \
  	-DMMC_SDIO_ABORT -DCUSTOMER_HW -DCUSTOM_OOB_GPIO_NUM=18 -DXTAL_PU_TIME_MOD \
  	-DCUSTOMER_HW_BCM2155X

# Samsung platform specific
DHDCFLAGS += -DBCMWLAN_SAMSUNG
ifeq ($(CONFIG_BROADCOM_WIFI_RESERVED_MEM),y)
DHDCFLAGS += -DCUSTOMER_HW_SAMSUNG
endif

DHDCFLAGS += -DCHECK_CHIP_REV
DHDCFLAGS += -DRDWR_MACADDR

# SDIO_ISR_THREAD is mutually exclusive with OOB_INTR_ONLY
#DHDCFLAGS += -DSDIO_ISR_THREAD

# For OOB only
DHDCFLAGS += -DOOB_INTR_ONLY

# For HW_OOB
#DHDCFLAGS += -DHW_OOB

# For SOFTAP
DHDCFLAGS += -DSOFTAP

# For Debug
DHDCFLAGS += -DDHD_DEBUG -DSRCBASE=\"$(SRCBASE)\"

# HOST WAKEUP
DHDCFLAGS += -DBCM_HOSTWAKE

# STATIC MEMORY ALLOCATION FEATURE

ifeq ($(CONFIG_BROADCOM_WIFI_RESERVED_MEM),y)
DHDCFLAGS += -DDHD_USE_STATIC_BUF
endif


# Disable PowerSave mode for OTA or certification test
#DHDCFLAGS += -DBCMDISABLE_PM
DHDCFLAGS += -DCONFIG_CONTROL_PM

# P2P ENABLE
#DHDCFLAGS += -DWLP2P

# BTAMP
DHDCFLAGS += -DWLBTAMP
DHDCFLAGS += -DDHD_BTAMP_DBG

# COMBO SCAN
DHDCFLAGS += -DCSCAN

# For Keepalive
DHDCFLAGS += -DUSE_KEEP_ALIVE

#For implementation Broadcast filter
# Packet Filter
DHDCFLAGS += -DPKT_FILTER_SUPPORT

# ARP offloading
#DHDCFLAGS += -DARP_OFFLOAD_SUPPORT

# DTIM counter
DHDCFLAGS += -DDTIM_CNT1

#MobileAP fw_reload
DHDCFLAGS += -DMOBILEAP_RELOAD

###############################################################################################

EXTRA_CFLAGS +=	-I$(src)/src/include/ \
	-I$(src)/src/dhd/sys/ \
	-I$(src)/src/dongle/ \
	-I$(src)/src/bcmsdio/sys/ \
	-I$(src)/src/wl/sys/ \
	-I$(src)/src/shared/

DHDOFILES = src/dhd/sys/dhd_linux.o \
	src/dhd/sys/dhd_common.o \
	src/dhd/sys/dhd_cdc.o \
	src/dhd/sys/dhd_linux_sched.o \
	src/dhd/sys/dhd_sdio.o   \
	src/dhd/sys/dhd_custom_gpio.o \
	src/dhd/sys/dhd_custom_sec.o \
	src/dhd/sys/dhd_bta.o \
	src/shared/aiutils.o \
	src/shared/bcmutils.o \
	src/shared/bcmwifi.o \
	src/shared/hndpmu.o \
	src/shared/linux_osl.o \
	src/shared/sbutils.o \
	src/shared/siutils.o \
	src/shared/bcmevent.o \
	src/bcmsdio/sys/bcmsdh_sdmmc.o \
	src/bcmsdio/sys/bcmsdh.o \
	src/bcmsdio/sys/bcmsdh_linux.o    \
	src/bcmsdio/sys/bcmsdh_sdmmc_linux.o \
	src/wl/sys/wl_iw.o

obj-m += bcm4330.o
bcm4330-objs += $(DHDOFILES)
EXTRA_CFLAGS += $(DHDCFLAGS)
EXTRA_LDFLAGS += --strip-debug
