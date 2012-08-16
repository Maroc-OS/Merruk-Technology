rm -Rf unpack
rm -Rf boot

mkdir -p unpack
mkdir -p source_img
cd sources_img
tar -xvf stock_kernel*
cd ..
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

cp ../MerrukTechnology_Output/zImage ./unpack/

rm unpack/boot.img-zImage

