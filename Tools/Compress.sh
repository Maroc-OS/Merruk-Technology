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

# Setup the Nedded Directories.

if [ test ! -d "target_img" ];
  then
    mkdir -p target_img
fi
sync

# Commpress the RamDisk.
# (TODO : Test the Support of Parameters Ex: ramdisk= "merruk" or "stock" )

function help
{
    echo "Positional parameter 1 is empty !"
    echo "How To Use :"
    echo "./Compress.sh [Parameter]"
    echo "  - merruk  =     Use Merruk Technology RamDisk"
    echo "  - stock   =     Use Samsung Stock RamDisk"
    echo "Please spesifie a parameter of listed above"
    exit 1
} # end help

if [ "$1" == "" ];
  then
    $(help)
else
  echo "Commpressing Kernel RamDisk..."
      if [ "$1" == "merruk" ];
        then
          echo ""
          echo "Merruk Technology RamDisk."
          echo ""
          tools/mkbootfs boot | lzma > unpack/boot.img-ramdisk.gz
      elif [ "$1" == "stock" ];
        then
          echo ""
          echo "Samsung Stock RamDisk."
          echo ""
          tools/mkbootfs boot | gzip > unpack/boot.img-ramdisk.gz
      else
        $(help)
    fi
fi

# Copy The Compiled Kernel From the Output Directory.
cp ../MerrukTechnology_Output/zImage ./unpack/

# Making The New Kernel :)
  echo ""
  echo "Making the New Kernel..."
  echo ""

tools/mkbootimg --kernel unpack/zImage --ramdisk unpack/boot.img-ramdisk.gz -o target_img/boot.img --base `cat unpack/boot.img-base`

# Remove the old TAR Compressed Kernel.
  cd target_img
  rm -Rf *.tar

# Make a Tarball for the Constructed Kernel Image (boot.img).
  tar cvf PDA.$1.$$.tar *
  cd ../