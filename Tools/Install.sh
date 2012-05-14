rm -Rf unpack
rm -Rf boot

mkdir -p unpack
tools/unpackbootimg -i source_img/boot.img -o unpack

mkdir -p boot
cd boot

#For Stock Kernel
#gzip -dc ../unpack/boot.img-ramdisk.gz | cpio -i

#For MerrukTechnology Kernel
xz -dc ../unpack/boot.img-ramdisk.gz | cpio -i
cd ../

mkdir boot/system/etc
mkdir boot/system/etc/init.d

#cp Worked/sbin/su boot/sbin/
#cp Worked/sbin/busybox boot/sbin/
#cp Worked/sbin/init.data boot/sbin/
#cp Worked/sbin/sysinit.rc boot/sbin/
#cp Worked/sbin/bmlunlock boot/sbin/
#cp Worked/system/etc/init.d/* boot/system/etc/init.d/

cp ../MerrukTechnology_Output/zImage ./unpack/

rm unpack/boot.img-zImage

