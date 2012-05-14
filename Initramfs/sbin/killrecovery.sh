#!/system/bin/sh -x

echo " " >> /data/local/CWM-Ver
echo " " >> /data/local/CWM-Ver

FILESIZE=$(/sbin/busybox cat /data/local/CWM-Ver | /sbin/busybox wc -c)
USERNAME=$(/sbin/busybox whoami)

exec > /data/local/CWM_userscript.log 2>&1

# Tmp Permissions for Root
/sbin/busybox chmod 4777 /sbin/su

# Remove Kernel file if it is big
if /sbin/busybox test "$FILESIZE" -ge "10000"; then
	/sbin/busybox rm /data/local/CWM-Ver
fi
/sbin/busybox sync

# Start Logging
echo "  " >> /data/local/CWM-Ver
echo " Starting Merruk Logger Script - ( AT ) - " $(/sbin/busybox date) >> /data/local/CWM-Ver
echo " - " >> /data/local/CWM-Ver
echo " - " >> /data/local/CWM-Ver
echo " " $(/sbin/busybox uname -a) >> /data/local/CWM-Ver
echo " Username : " $USERNAME >> /data/local/CWM-Ver
echo " - " >> /data/local/CWM-Ver
echo " - " >> /data/local/CWM-Ver

############# Start the important things #############

# UnlockBML & Remount Read/Write
echo "	Unlock BML Blocks " >> /data/local/CWM-Ver
/sbin/bmlunlock

echo "	Mount /System as R/W " >> /data/local/CWM-Ver
/sbin/busybox mount -o remount,rw -t ext4 /dev/block/stl9 /system
/sbin/busybox mount -o remount,rw -t ext4 / /
/sbin/busybox mount -a

# Create Needed Directories
echo "	Create /etc directory " >> /data/local/CWM-Ver
#/sbin/busybox rm -f /etc
#/sbin/busybox mkdir -p /etc

if /sbin/busybox test ! -d /sd-ext ; then
	echo "	Create /sd-ext Folder " >> /data/local/CWM-Ver
	/sbin/busybox mkdir -p /sd-ext
fi
/sbin/busybox sync

# Galaxy Y make /sdcard a symlink to /mnt/sdcard, which confuses CWM
echo "	Fix Linked /sdcard to /mnt/sdcard " >> /data/local/CWM-Ver
/sbin/busybox rm -f /sdcard
/sbin/busybox mkdir -p /sdcard

# Fix permissions in /sbin
echo "	Fix Permissions on /sbin " >> /data/local/CWM-Ver
/sbin/busybox chmod 750 /sbin/*
echo "		* Set a Special Permissions for Busybox & Su " >> /data/local/CWM-Ver
/sbin/busybox chmod u+s /sbin/busybox
/sbin/busybox chmod 06755 /sbin/su

# Fix Recovery EXT4 Partitions 
if /sbin/busybox test -f /etc/recovery.fstab ; then
	echo "	Replace Recovery 'fstab' File -Status 1- " >> /data/local/CWM-Ver
	/sbin/busybox rm /etc/recovery.fstab
	/sbin/busybox cp /sbin/recovery.fstab /etc/
else
	echo "	Replace Recovery 'fstab' File -Status 2- " >> /data/local/CWM-Ver
	/sbin/busybox cp /sbin/recovery.fstab /etc/
fi
/sbin/busybox sync

# Fix System EXT4 Partitions
if /sbin/busybox test -f /etc/fstab ; then
	echo "	Replace System 'fstab' File -Status 1- " >> /data/local/CWM-Ver
	/sbin/busybox rm /etc/fstab
	/sbin/busybox cp /sbin/fstab /etc/
else
	echo "	Replace System 'fstab' File -Status 2- " >> /data/local/CWM-Ver
	/sbin/busybox cp /sbin/fstab /etc/
fi
/sbin/busybox sync

# Good Tools For enjoing the system
if /sbin/busybox test -f /etc/sysctl.conf ; then
	echo "	Add SysCtl Config File -Status 1- " >> /data/local/CWM-Ver
	/sbin/busybox rm /etc/sysctl.conf
	/sbin/busybox cp /sbin/sysctl.conf /etc/sysctl.conf
else
	echo "	Add SysCtl Config File -Status 2- " >> /data/local/CWM-Ver
	/sbin/busybox cp /sbin/sysctl.conf /etc/sysctl.conf
fi
/sbin/busybox sync

if /sbin/busybox test -f /etc/mke2fs.conf ; then
	echo "	Add Mke2fs Config File -Status 1- " >> /data/local/CWM-Ver
	/sbin/busybox rm /etc/mke2fs.conf
	/sbin/busybox cp /sbin/mke2fs.conf /etc/mke2fs.conf
else
	echo "	Add Mke2fs Config File -Status 2- " >> /data/local/CWM-Ver
	/sbin/busybox cp /sbin/mke2fs.conf /etc/mke2fs.conf
fi
/sbin/busybox sync
#######################################################

# Also A Shorter
echo '#!/system/bin/sh -x
exec > /data/local/CWM_userscript.log 2>&1

/sbin/busybox cp /sbin/fstab /etc/
/sbin/busybox cp /sbin/fstab /etc/mtab
/sbin/busybox cp /sbin/recovery.fstab /etc/recovery.fstab

# Mount All fstab Mount-Points
echo "	Mount /System as R/W " >> /data/local/CWM_Boot_Log.txt
/sbin/busybox mount -o remount,rw -t ext4 /dev/block/stl9 /system
/sbin/busybox mount -o remount,rw -t ext4 / /
/sbin/busybox mount -a

# succeed to mount the sdcard by default even with broken fstab
/sbin/busybox mount -t vfat -o rw,nosuid,nodev,noexec,uid=1000,gid=1015,fmask=0002,dmask=0002,allow_utime=0020,iocharset=iso8859-1,shortname=mixed,utf8,errors=remount-ro /dev/block/mmcblk0p1 /sdcard
' > /sbin/postrecoveryboot.sh
/sbin/busybox chmod 777 /sbin/postrecoveryboot.sh

# Needed For CWM Recouvery
/sbin/busybox rm /cache/recovery/command
/sbin/busybox rm /cache/update.zip
/sbin/busybox touch /tmp/.ignorebootmessage
kill $(/sbin/busybox ps | /sbin/busybox grep /sbin/adbd)
#kill $(/sbin/busybox ps | /sbin/busybox grep /sbin/recovery)

# Check if we have a luancher first
if /sbin/busybox test -f /sbin/recovery.sh ; then
    /sbin/busybox sh /sbin/recovery.sh &
else
    /sbin/recovery &
fi
/sbin/busybox sync

exit 1
