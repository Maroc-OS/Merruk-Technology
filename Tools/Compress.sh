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

# Set Tool Paths

WORKING_DIR="pwd"
export MERRUK_TOOLS=$($WORKING_DIR)"/bin"
export TARBALL_KERNELS=$($WORKING_DIR)"/TarBall_Kernels"
export SOURCE_IMG=$($WORKING_DIR)/"/Source_Img"
export TARGET_IMG=$($WORKING_DIR)"/Target_Img"
export UNPACK=$($WORKING_DIR)"/Unpack"
export BOOT=$($WORKING_DIR)"/Boot"

clear

# Copy The Compiled Kernel From the Output Directory.

if [ -f "../MerrukTechnology_Output/zImage" ];
then
	echo "###############################################################################"
        echo "#    MerrukTechnology Kernel Found ! Make a Copy into '/Unpack' Direcroty.    #"
	echo "###############################################################################"
        cp ../MerrukTechnology_Output/zImage $UNPACK/
else
	echo "###############################################################################"
        echo "# MerrukTechnology Kernel Not Copmiled ! Please Run ./Kernel_Make [Parameter] #"
	echo "###############################################################################"
        echo "#                 Samsung Stock zImage used as default...!                    #"
        echo "#                 ----------------------------------------                    #"
	echo "###############################################################################"
	cp $WORKING_DIR/ZMG $UNPACK/zImage
fi
sync

# Commpress the RamDisk.

function Help
{
	clear
        echo "#                 ----------------------------------------                    #"
	echo "###############################################################################"
        echo "#                  MerrukTechnology Images Compressor.sh                      #"
	echo "#                   Positional parameter [1] is empty !                       #"
	echo "###############################################################################"
	echo "# How To Use :                                                                #"
	echo "# ___________                                                                 #"
        echo "#                                                                             #"
	echo "#		./Compress.sh [Parameter]                                           #"
	echo "#                                                                             #"
	echo "#			- merruk  =     Use Merruk Technology RamDisk               #"
	echo "#			- stock   =     Use Samsung RamDisk                         #"
        echo "#                                                                             #"
	echo "#	Please specify one of those listed above                                    #"
        echo "#                                                                             #"
	echo "###############################################################################"
        echo "#                 ----------------------------------------                    #"
	exit 1
} # end Help

# Making The New Kernel :)

function Make_Img
{
        echo "#                 ----------------------------------------                    #"
	echo "###############################################################################"
	echo "# Making the new kernel image...                                              #"
	echo "###############################################################################"
        echo "#                 ----------------------------------------                    #"
	$MERRUK_TOOLS/mkbootimg --kernel $UNPACK/zImage --ramdisk $UNPACK/boot.img-ramdisk.gz -o $TARGET_IMG/boot.img --base `cat $UNPACK/boot.img-base`
} # end Make_Img

# Start Compression

if [ "$1" == "" ];
then
	Help
else
        echo "#                 ----------------------------------------                    #"
	echo "###############################################################################"
	echo "# Commpressing kernel RamDisk...                                              #"
	echo "###############################################################################"
        echo "#                 ----------------------------------------                    #"

	if [ "$1" == "merruk" ];
	then
        	echo "#                 ----------------------------------------                    #"
		echo "#                      Merruk Technology RamDisk                              #"
        	echo "#                 ----------------------------------------                    #"
		$MERRUK_TOOLS/mkbootfs $BOOT | lzma > $UNPACK/boot.img-ramdisk.gz

	elif [ "$1" == "stock" ];
	then
        	echo "#                 ----------------------------------------                    #"
		echo "#                           Samsung RamDisk                                   #"
        	echo "#                 ----------------------------------------                    #"
		$MERRUK_TOOLS/mkbootfs $BOOT | gzip > $UNPACK/boot.img-ramdisk.gz

	else
		Help
	fi
	echo "###############################################################################"
        echo "#                 ----------------------------------------                    #"
fi
sync

# Remove Old Images and make  the kernel
rm -Rf $TARGET_IMG/*.img
Make_Img

# Make a Tarball for the Constructed Kernel Image (boot.img).

if [ -f "$TARGET_IMG/boot.img" ];
then
	# Remove the old TAR Compressed Kernel.
        echo "#                 ----------------------------------------                    #"
	echo "###############################################################################"
	echo "# Remove Old PDA TarBall File...                                              #"
        echo "#                                                                             #"
        cd $TARGET_IMG
        rm -Rf *.tar

	# Make a Tarball for the Constructed Kernel Image (boot.img).

        echo "#                                                                             #"
	echo "# Making The New Odin Flashable 'PDA.$1.tar'                               #"
	echo "#            & Raw 'Kernel.$1.Boot.img' Flashable with 'DD' Command ...    #"
	tar cvf PDA.$1.tar *
	mv  boot.img Kernel.$1.Boot.img
        echo "#                                                                             #"
	echo "#            Go Find The Result Files in '/Target_Img' Directory.             #"
	echo "###############################################################################"
        echo "#                 ----------------------------------------                    #"
	cd ../
else
        echo "#                 ----------------------------------------                    #"
	echo "###############################################################################"
        echo "#                 ----------------------------------------                    #"
	echo "#                     /Target_Img/boot.img Not Found !                        #"
        echo "#                 ----------------------------------------                    #"
	echo "#                         PDA.$1.tar Not Created !                            #"
        echo "#                 ----------------------------------------                    #"
	echo "###############################################################################"

fi
sync
