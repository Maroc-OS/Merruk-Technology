#!/bin/bash
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

# Set Tools Paths

WORKING_DIR="pwd"
export MERRUK_TOOLS=$($WORKING_DIR)"/bin"
export TARBALL_KERNELS=$($WORKING_DIR)"/TarBall_Kernels"
export SOURCE_IMG=$($WORKING_DIR)/"/Source_Img"
export TARGET_IMG=$($WORKING_DIR)"/Target_Img"
export UNPACK=$($WORKING_DIR)"/Unpack"
export BOOT=$($WORKING_DIR)"/Boot"

# Setup the Nedded Directories.

if [ ! -d "$SOURCE_IMG" ];
then
	mkdir -p $SOURCE_IMG
	chmod 777 $SOURCE_IMG
fi
sync

if [ ! -d "$TARGET_IMG" ];
then
	mkdir -p $TARGET_IMG
	chmod 777 $TARGET_IMG
fi
sync

if [ ! -d "$UNPACK" ];
then
	mkdir -p $UNPACK
	chmod 777 $UNPACK
else
	rm -Rf $UNPACK
	mkdir -p $UNPACK
	chmod 777 $UNPACK
fi
sync

if [ ! -d "$BOOT" ];
then
	mkdir -p $BOOT
	chmod 777 $BOOT
else
	rm -Rf $BOOT
	mkdir -p $BOOT
	chmod 777 $BOOT
fi
sync

chmod 777 $MERRUK_TOOLS/*

# Check if there is a Complete Kernel Boot Image (boot.img) in "Source_Img"

function Install_Kernel_Img () {
	if [ -f "$SOURCE_IMG/boot.img" ];
	then
		echo ""
		echo "Boot.Img File Found ! Reinstall the Boot Image ..."
		rm -f $SOURCE_IMG/boot.img
		cd $SOURCE_IMG
		echo ""
		tar -xvf $TARBALL_KERNELS/$1
		chmod 777 *
		cd ..
		echo "Samsung Boot Image Installed With Success."
		echo ""
		$MERRUK_TOOLS/unpackbootimg -i $SOURCE_IMG/boot.img -o $UNPACK
		echo "Boot Image Extracted to '$UNPACK' Directory."
	else
        	echo ""
        	echo "Installing the Boot Image File ..."
        	cd $SOURCE_IMG
        	echo ""
        	tar -xvf $TARBALL_KERNELS/$1
        	chmod 777 *
        	cd ..
        	echo "Samsung Boot Image Installed With Success."
        	echo ""
        	$MERRUK_TOOLS/unpackbootimg -i $SOURCE_IMG/boot.img -o $UNPACK
	        echo "Boot Image Extracted to '$UNPACK' Directory."
	fi
	sync
}

# Decommpress the RamDisk.

function Help
{
<<<<<<< HEAD
	echo "Positional Parameter [1] is Empty !"
	echo ""
	echo "How To Use :"
	echo ""
	echo "./Install.sh [Parameter]"
	echo ""
	echo "  - merruk  =     Use Merruk Technology RamDisk"
	echo "  - stock   =     Use Samsung RamDisk"
	echo ""
	echo "Please specify a parameter from listed above"
	exit 1
} # end Help

# Start Decompression
=======
    echo "Positional parameter 1 is empty !"
    echo "How To Use :"
    echo "./Install.sh [Parameter]"
    echo "    - merruk  =         Use Merruk Technology RamDisk"
    echo "    - stock   =         Use Samsung stock RamDisk"
    echo "Please specify one of those listed above"
    exit 1
}   # end help
>>>>>>> 7a10642bbdf365e73a30e49617fccf19186affcd

if [ "$1" == "" ];
then
	Help
else
<<<<<<< HEAD
	echo "Decommpressing Kernel RamDisk ..."
=======
	echo "Decommpressing kernel RamDisk..."
>>>>>>> 7a10642bbdf365e73a30e49617fccf19186affcd
	if [ "$1" == "merruk" ];
	then
		Install_Kernel_Img "merruk_kernel.tar"
		echo ""
		echo "Merruk Technology RamDisk"
		echo ""
		cd $BOOT
		gzip -dc $UNPACK/boot.img-ramdisk.gz | cpio -i
		rm $UNPACK/boot.img-zImage
		cd ..

	elif [ "$1" == "stock" ];
	then
		Install_Kernel_Img "stock_kernel.tar"
		echo ""
		echo "Samsung RamDisk"
		echo ""
		cd $BOOT
		xz -dc $UNPACK/boot.img-ramdisk.gz | cpio -i
		rm $UNPACK/boot.img-zImage
		cd..

	else
		Help
	fi
fi
sync

# Add Init.d Directory to the RamDisk

if [ -d "$BOOT/system" ];
then
	mkdir $BOOT/system/etc
	mkdir $BOOT/system/etc/init.d
	chmod 777 $BOOT/system/etc/init.d
else
	echo ""
	echo "Boot Directory Not Found !"
	echo ""
fi
sync

# Copy The Compiled Kernel From The Output Directory

if [ -f "../MerrukTechnology_Output/zImage" ];
then
	echo "MerrukTechnology Kernel Found ! Make a Copy into '$($UNPACK)' Direcroty."
	cp ../MerrukTechnology_Output/zImage $UNPACK/
	echo ""
else
	echo "MerrukTechnology Kernel Not Copmiled ! Please Go Back & Run ./Kernel_Make [Parameter]"
	echo ""
fi
sync
