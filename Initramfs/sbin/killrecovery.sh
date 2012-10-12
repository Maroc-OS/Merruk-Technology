#!/system/bin/sh

set -x
export PATH=/:/sbin:/system/xbin:/system/bin:/tmp:$PATH

# Path Informations & Fixes:)
SHELL="/sbin/busybox"
ROOT_ROOT="0.0"
ROOT_RADIO="0.radio"
ROOT_SYSTEM="0.1000"
RADIO_SYSTEM="0.1000"

if $SHELL test -f /sbin/busybox ; then
	LN="/sbin/busybox ln"
	CHOWN="/sbin/busybox chown"
	CHMOD="/sbin/busybox chmod"
	MKDIR="/sbin/busybox mkdir"
	RMDIR="/sbin/busybox rmdir"
fi
$SHELL sync

# General Definitions
CP="$SHELL cp"
LS="$SHELL ls"
RM="$SHELL rm"
TR="$SHELL tr"
WC="$SHELL wc"
CAT="$SHELL cat"
CUT="$SHELL cut"
SED="$SHELL sed"
TAR="$SHELL tar"
TEE="$SHELL tee"
DATE="$SHELL date"
ECHO="$SHELL echo"
EXPR="$SHELL expr"
FIND="$SHELL find"
GREP="$SHELL grep"
GZIP="$SHELL gzip"
SYNC="$SHELL sync"
TEST="$SHELL test"
EGREP="$SHELL egrep"
MOUNT="$SHELL mount"
TOUCH="$SHELL touch"
UNAME="$SHELL uname"
UMOUNT="$SHELL umount"
WHOAMI="$SHELL whoami"

#####################################################################

VERSION="MT v2.5"
SIGNATURE="Merruk Technology, SARL."

USERNAME=$($WHOAMI)
FILESIZE=$($CAT /data/local/CWM-Ver | $WC -c)

# Nedded to Start Merruk Technology Kernels
if $TEST ! -d /data/local ; then
	$MKDIR /data/local
fi
$SYNC

######################################################################

exec > /data/local/CWM_userscript.log 2>&1

# Remove Recovery Debug file if it is big
if $TEST "$FILESIZE" -ge "10000"; then
	$RM -R /data/local/CWM-Ver
fi
$SYNC

# Start Logging
echo "  " >> /data/local/CWM-Ver
echo " Starting Merruk Logger $VERSION Script - ( AT ) - " $($DATE) >> /data/local/CWM-Ver
echo " - " >> /data/local/CWM-Ver
echo " - " >> /data/local/CWM-Ver
echo " " $($UNAME -a) >> /data/local/CWM-Ver
echo " Username : " $USERNAME >> /data/local/CWM-Ver
echo " - " >> /data/local/CWM-Ver
echo " - " >> /data/local/CWM-Ver

############# Start the important things #############

# mount /System Read/Write, we did it in top :)
$ECHO "	Re-Mount /System as R/W " >> /data/local/Kernel-Ver

$CHOWN $ROOT_SYSTEM /system/bin/sh

# Fix System EXT4 Partitions 
if $TEST -f /etc/fstab ; then
	$ECHO "	Replace System 'fstab' File -Status 1- " >> /data/local/Kernel-Ver
	$RM /etc/fstab
	$CP /sbin/fstab /etc/fstab
else
	$ECHO "	Replace System 'fstab' File -Status 2- " >> /data/local/Kernel-Ver
	$CP /sbin/fstab /etc/fstab
fi
$SYNC

# Fix Recovery EXT4 Partitions 
if $TEST -f /etc/recovery.fstab ; then
	$ECHO "	Replace Recovery 'fstab' File -Status 1- " >> /data/local/Kernel-Ver
	$RM /etc/recovery.fstab
	$CP /sbin/recovery.fstab /etc/recovery.fstab
else
	$ECHO "	Replace Recovery 'fstab' File -Status 2- " >> /data/local/Kernel-Ver
	$CP /sbin/recovery.fstab /etc/recovery.fstab
fi
$SYNC

# Galaxy Y make /sdcard a symlink to /mnt/sdcard, which confuses CWM
#if $TEST ! -d /sdcard ; then
#	echo "	Fix Linked /sdcard to /mnt/sdcard " >> /data/local/CWM-Ver
#	/sbin/busybox rm -f /sdcard
#	$SHELL mkdir /sdcard
#fi
#$SYNC

# MK2FS :)
if $TEST -f /etc/mke2fs.conf ; then
	echo "	Add Mke2fs Config File -Status 1- " >> /data/local/CWM-Ver
	$RM /etc/mke2fs.conf
	$CP /sbin/mke2fs.conf /etc/mke2fs.conf
else
	echo "	Add Mke2fs Config File -Status 2- " >> /data/local/CWM-Ver
	$CP /sbin/mke2fs.conf /etc/mke2fs.conf
fi
$SYNC

$ECHO "	Mount /System as R/W " >> /data/local/Kernel-Ver
$MOUNT -o remount,rw -t ext4 /dev/stl9 /system
$MOUNT -o remount,rw -t ext4 / /
$MOUNT -a

# Fix System EXT4 Partitions 
if $TEST -f /etc/fstab ; then
	$ECHO "	Replace System 'fstab' File -Status 1- " >> /data/local/Kernel-Ver
	$RM /etc/fstab
	$CP /sbin/fstab /etc/fstab
else
	$ECHO "	Replace System 'fstab' File -Status 2- " >> /data/local/Kernel-Ver
	$CP /sbin/fstab /etc/fstab
fi
$SYNC

# Fix Recovery EXT4 Partitions 
if $TEST -f /etc/recovery.fstab ; then
	$ECHO "	Replace Recovery 'fstab' File -Status 1- " >> /data/local/Kernel-Ver
	$RM /etc/recovery.fstab
	$CP /sbin/recovery.fstab /etc/recovery.fstab
else
	$ECHO "	Replace Recovery 'fstab' File -Status 2- " >> /data/local/Kernel-Ver
	$CP /sbin/recovery.fstab /etc/recovery.fstab
fi
$SYNC

# Fix permissions in /sbin
$ECHO "	Fix Permissions on /sbin " >> /data/local/Kernel-Ver
$CHMOD 755 /sbin/*
$CHMOD 777 /sbin/bootanimation
$ECHO "		* Set a Special Permissions for Busybox & Su " >> /data/local/Kernel-Ver
$CHMOD 06755 $SHELL
$CHOWN $ROOT_ROOT /sbin/su
$CHMOD 06755 /sbin/su

# UnlockBML & Remount Read/Write
$ECHO "	Unlock BML Blocks " >> /data/local/Kernel-Ver
/sbin/bmlunlock

# Create Needed Directories
###########################

# Install New BusyBox Version
echo "	Install New BusyBox Version " >> /data/local/CWM-Ver
/sbin/busybox --install -s /sbin/

# Fix permissions in /sbin
echo "	Fix Permissions on /sbin " >> /data/local/CWM-Ver
$CHMOD 755 /sbin/*
echo "		* Set a Special Permissions for Busybox & Su " >> /data/local/CWM-Ver
$CHMOD 06755 /sbin/busybox
$CHOWN $ROOT_ROOT /sbin/su
$CHMOD 06755 /sbin/su

#######################################################

# Also A Shorter

$TOUCH /sbin/postrecoveryboot.sh

echo '#!/system/bin/sh -x
exec > /data/local/CWM_userscript.log 2>&1

/sbin/busybox cp /sbin/fstab /etc/
/sbin/busybox cp /sbin/fstab /etc/mtab
/sbin/busybox cp /sbin/recovery.fstab /etc/recovery.fstab

# succeed to mount the sdcard by default even with broken fstab
$SHELL mount -t vfat -o rw,nosuid,nodev,noexec,uid=1000,gid=1015,fmask=0002,dmask=0002,allow_utime=0020,iocharset=iso8859-1,shortname=mixed,utf8,errors=remount-ro /dev/block/mmcblk0p1 /sdcard
/sbin/busybox sleep 1
$SHELL mount -t vfat -o remount,rw,nosuid,nodev,noexec,uid=1000,gid=1015,fmask=0002,dmask=0002,allow_utime=0020,iocharset=iso8859-1,shortname=mixed,utf8,errors=remount-ro /dev/block/mmcblk0p1 /sdcard
/sbin/busybox sleep 1
' > /sbin/postrecoveryboot.sh
$CHMOD 0777 /sbin/postrecoveryboot.sh

/sbin/busybox rm /system/bin/reboot
$LN -s /sbin/toolbox /system/bin/reboot

# Needed For CWM Recouvery
$RM /cache/recovery/command
$RM /cache/update.zip
touch /tmp/.ignorebootmessage
#kill $(ps | /sbin/busybox grep /sbin/adbd)
#kill $(ps | /sbin/busybox grep /sbin/recovery)

# Remount /System read only and continue
echo "	Re-Mount /System as R/O & Continue Booting... " >> /data/local/CWM-Ver
$MOUNT -o remount,rw -t ext4 /dev/block/stl9 /system
$MOUNT -o remount,rw -t ext4 / /
$SYNC

# Stop Logging
echo " - " >> /data/local/CWM-Ver
echo " - " >> /data/local/CWM-Ver
echo " End of Merruk Logger Script - ( AT ) - " $(date) >> /data/local/CWM-Ver
echo " " >> /data/local/CWM-Ver
echo " --------------------------------------------------------------------" >> /data/local/CWM-Ver
echo "		MerrukTechnology Kernel By Maroc-OS @ XDA-Devolopers		" >> /data/local/CWM-Ver
echo " --------------------------------------------------------------------" >> /data/local/CWM-Ver

# Check if we have a luancher first
#if test -f /sbin/recovery.sh ; then
#    sh /sbin/recovery.sh &
#else
    /sbin/recovery &
#fi
#sync

exit 1 

exit 1
