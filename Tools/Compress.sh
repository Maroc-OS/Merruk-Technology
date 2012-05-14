#For Stock Kernel
#tools/mkbootfs boot | gzip > unpack/boot.img-ramdisk.gz

#For MerrukTechnology Kernel
tools/mkbootfs boot | lzma > unpack/boot.img-ramdisk.gz

cp ../MerrukTechnology_Output/zImage ./unpack/

tools/mkbootimg --kernel unpack/zImage --ramdisk unpack/boot.img-ramdisk.gz -o target_img/boot.img --base `cat unpack/boot.img-base`
cd target_img
rm -Rf boot.tar
tar cvf boot.tar *
cd ../

