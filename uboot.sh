#!/bin/sh

BOOTLOADERIMAGE=u-boot.bin
BC_DM365=/home/klaus/LeopardBoardDM365sdkEVAL2011Q2/bootloader/u-boot-2010.12-rc2-psp03.01.01.39/ti-flash-utils/src/DM36x/GNU/bc_DM36x.exe 

make distclean CROSS_COMPILE=arm-none-linux-gnueabi- 

make davinci_dm365leopard_config CROSS_COMPILE=arm-none-linux-gnueabi- RELFLAGS="-Wall -O2 -mcpu=arm926ej-s"

make  -j 1 CROSS_COMPILE=arm-none-linux-gnueabi- CONFIG_SYS_TEXT_BASE="0x82000000" EXTRA_CPPFLAGS="-DCONFIG_SPLASH_ADDRESS="0x80800000" -DCONFIG_SPLASH_LCD=1"  

mono ${BC_DM365} -uboot -pageSize 2048 -blockNum 25 \
	   -startAddr 0x82000000 -loadAddr 0x82000000 ${BOOTLOADERIMAGE} -o ${BOOTLOADERIMAGE}.nandbin 
