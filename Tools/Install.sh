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

if [ test ! -d "source_img" ];
then
  mkdir -p sources_img
fi
sync

if [ test ! -d "unpack" ];
then
  mkdir -p unpack
else
  rm -Rf unpack
  mkdir -p unpack
fi
sync

if [ test ! -d "boot" ];
then
  mkdir -p boot
else
  rm -Rf boot
  mkdir -p boot
fi
sync

# Check if there is a Complete Kernel (boot.img) in "Source_Img"

if [ test ! -f "source_img/boot.img" ];
then
  cd sources_img
  tar -xvf ../stock_kernel.tar
  cd ..
  tools/unpackbootimg -i source_img/boot.img -o unpack
else
  tools/unpackbootimg -i source_img/boot.img -o unpack
fi
sync

# Decommpress the RamDisk.
# (TODO : Test if Parameters Support Works. Ex: Install.sh "merruk" or "stock" )

function help
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
    $(help)
else
    echo "Decommpressing Kernel RamDisk..."
    if [ "$1" == "merruk" ];
      then
        echo ""
        echo "Merruk Technology RamDisk."
        echo ""
        gzip -dc ../unpack/boot.img-ramdisk.gz | cpio -i
    elif [ "$1" == "stock" ];
      then
        echo ""
        echo "Samsung Stock RamDisk."
        echo ""
        xz -dc ../unpack/boot.img-ramdisk.gz | cpio -i
    else
      $(help)
  fi
fi

# Add Init.d Directory to the RamDisk
mkdir boot/system/etc
mkdir boot/system/etc/init.d

rm unpack/boot.img-zImage

cp ../MerrukTechnology_Output/zImage ./unpack/
