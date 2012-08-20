#!/bin/sh
###################################################################
# Merruk Technology Compress Script For A new Installation/Update #
# For InitRamFS (ramdisk) Envirement .                            #
###################################################################
# Author    : Yahya Lmallas                                       #
# E-Mail    : y.lmallas@merruk.ma, merruk.company@gmail.com       #
# Company   : Merruk Technology, SARL.                            #
# Web-Site  : www.merruk.ma                                       #
# UserName  : Maroc-OS                                            #
###################################################################

# Copy The Compiled Kernel From the Output Directory.

cp ../MerrukTechnology_Output/zImage ./unpack/

# Commpress the RamDisk.

function Help
{
	echo "Positional parameter 1 is empty !"
	echo "How To Use :"
	echo "./Compress.sh [Parameter]"
	echo "  - merruk  =     Use Merruk Technology RamDisk"
	echo "  - stock   =     Use Samsung Stock RamDisk"
	echo "Please specify one of those listed above"
	exit 1
} # end Help

function Make_Img
{
# Making The New Kernel :)
	echo ""
	echo "Making the new kernel..."
	echo ""
	./tools/mkbootimg --kernel ./unpack/zImage --ramdisk ./unpack/boot.img-ramdisk.gz -o ./target_img/boot.img --base `cat ./unpack/boot.img-base`
} # end Make_Img

if [ "$1" == "" ];
then
	Help
else
	echo "Commpressing kernel RamDisk..."
	if [ "$1" == "merruk" ];
	then
		echo ""
		echo "Merruk Technology RamDisk"
		echo ""
		./tools/mkbootfs ./boot | lzma > ./unpack/boot.img-ramdisk.gz
	elif [ "$1" == "stock" ];
	then
		echo ""
		echo "Samsung RamDisk"
		echo ""
		./tools/mkbootfs ./boot | gzip > ./unpack/boot.img-ramdisk.gz
	else
		Help
	fi
fi

Make_Img

# Remove the old TAR Compressed Kernel.
  cd target_img
  rm -Rf *.tar

# Make a Tarball for the Constructed Kernel Image (boot.img).
  tar cvf PDA.$1.tar *
  cd ../
