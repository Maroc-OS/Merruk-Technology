#!/bin/bash
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

# Set Tools Paths

WORKING_DIR="pwd"
export MERRUK_TOOLS=$($WORKING_DIR)"/bin"
export SOURCE_IMG=$($WORKING_DIR)"/Source_Img"
export TARGET_IMG=$($WORKING_DIR)"/Target_Img"
export UNPACK=$($WORKING_DIR)"/Unpack"
export BOOT=$($WORKING_DIR)"/Boot"

# Copy The Compiled Kernel From the Output Directory.

if [ -f "../MerrukTechnology_Output/zImage" ];
then
        echo "MerrukTechnology Kernel Found ! Make a Copy into '$UNPACK' Direcroty."
        cp ../MerrukTechnology_Output/zImage $UNPACK/
        echo ""
else
        echo "MerrukTechnology Kernel Not Copmiled ! Please Run ./Kernel_Make -[Parameter]"
        echo ""
fi
sync

# Commpress the RamDisk.

function Help
{
	echo "Positional parameter [1] is empty !"
	echo ""
	echo "How To Use :"
	echo ""
	echo "./Compress.sh [Parameter]"
	echo ""
	echo "  - merruk  =     Use Merruk Technology RamDisk"
	echo "  - stock   =     Use Samsung RamDisk"
	echo ""
	echo "Please specify one of those listed above"
	exit 1
} # end Help

# Making The New Kernel :)

function Make_Img
{
	echo ""
	echo "Making the new kernel..."
	echo ""
	$MERRUK_TOOLS/mkbootimg --kernel $UNPACK/zImage --ramdisk $UNPACK/boot.img-ramdisk.gz -o $TARGET_IMG/boot.img --base `cat $UNPACK/boot.img-base`
} # end Make_Img

# Start Compression

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
		$MERRUK_TOOLS/mkbootfs $BOOT | lzma > $UNPACK/boot.img-ramdisk.gz
	elif [ "$1" == "stock" ];
	then
		echo ""
		echo "Samsung RamDisk"
		echo ""
		$MERRUK_TOOLS/mkbootfs $BOOT | gzip > $UNPACK/boot.img-ramdisk.gz
	else
		Help
	fi
fi
sync

Make_Img

# Remove the old TAR Compressed Kernel.

	cd $TARGET_IMG
	rm -Rf *.tar

# Make a Tarball for the Constructed Kernel Image (boot.img).

if [ -f "$TARGET_IMG/boot.img" ];
then
	# Remove the old TAR Compressed Kernel.
	echo ""
	echo "Remove Old PDA TarBall File ... "
        cd $TARGET_IMG
        rm -Rf *.tar

	# Make a Tarball for the Constructed Kernel Image (boot.img).

	echo ""
	echo "Making The New Odin Flashable 'PDA.$1.tar' & Raw 'Kernel.$1.Boot.img' Flashable with 'DD' Command ..."
	tar cvf PDA.$1.tar *
	mv  boot.img Kernel.$1.Boot.img
	echo ""
	echo "Go Find The Result Files in '$TARGET_IMG' Directory."
	echo ""
	cd ../
else
	echo ""
	echo "$TARGET_IMG/boot.img Not Found !"
	echo ""
	echo "PDA.$1.tar Not Created !"
	echo ""
fi
sync
