#!/bin/sh
###################################################################
# Merruk Technology Install Script For A new Installation/Update  #
# For InitRamFS (ramdisk) Envirement .                            #
###################################################################
# Author    : Yahya Lmallas                                       #
# E-Mail    : y.lmallas@merruk.ma, merruk.company@gmail.com       #
# Company   : Merruk Technology, SARL.                            #
# Web-Site  : www.merruk.ma                                       #
# UserName  : Maroc-OS                                            #
###################################################################

# Setup the Nedded Directories.

if [ ! -d "source_img" ];
then
	mkdir -p source_img
	chmod 777 source_img
fi
sync

if [ ! -d "target_img" ];
then
	mkdir -p target_img
	chmod 777 target_img
fi
sync

if [ ! -d "unpack" ];
then
	mkdir -p unpack
	chmod 777 unpack
else
	rm -Rf unpack
	mkdir -p unpack
	chmod 777 unpack
fi
sync

if [ ! -d "boot" ];
then
	mkdir -p boot
	chmod 777 boot
else
	rm -Rf boot
	mkdir -p boot
	chmod 777 boot
fi
sync

chmod 777 ./tools/*

# Check if there is a Complete Kernel (boot.img) in "Source_Img"

if [ ! -f "source_img/boot.img" ];
then
	cd source_img
	tar -xvf ../stock_kernel.tar
	chmod 777 *
	cd ..
	./tools/unpackbootimg -i ./source_img/boot.img -o ./unpack
else
	./tools/unpackbootimg -i ./source_img/boot.img -o ./unpack
fi
sync

# Decommpress the RamDisk.

function Help
{
    echo "Positional parameter 1 is empty !"
    echo "How To Use :"
    echo "./Install.sh [Parameter]"
    echo "    - merruk  =         Use Merruk Technology RamDisk"
    echo "    - stock   =         Use Samsung Stock RamDisk"
    echo "Please spesifie a parameter of listed above"
    exit 1
}   # end help

if [ "$1" == "" ];
then
	Help
else
	echo "Decommpressing Kernel RamDisk..."
	if [ "$1" == "merruk" ];
	then
		echo ""
		echo "Merruk Technology RamDisk."
		echo ""
		gzip -dc ../unpack/boot.img-ramdisk.gz | cpio -i
		rm ./unpack/boot.img-zImage

	elif [ "$1" == "stock" ];
	then
		echo ""
		echo "Samsung Stock RamDisk."
		echo ""
		xz -dc ../unpack/boot.img-ramdisk.gz | cpio -i
		rm ./unpack/boot.img-zImage

	else
		Help
	fi
fi

# Add Init.d Directory to the RamDisk

if [ -d "boot/system" ];
then
	mkdir ./boot/system/etc
	mkdir ./boot/system/etc/init.d
	chmod 777 ./boot/system/etc/init.d
else
	mkdir -p boot
	mkdir ./boot/system/etc
	mkdir ./boot/system/etc/init.d
	chmod 777 ./boot/system/etc/init.d
fi
sync

cp ../MerrukTechnology_Output/zImage ./unpack/
