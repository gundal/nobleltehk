#!/bin/bash
export KERNELDIR=`readlink -f ~/android/nobleltehk`
export RAMFS_SOURCE=`readlink -f ~/android/nobleltehk/ramdisk`

echo "kernerldir = $KERNELDIR"
echo "ramfs_source = $RAMFS_SOURCE"

RAMFS_TMP=`readlink -f ~/android/gundal/tmp/ramdisk`

echo "ramfs_tmp = $RAMFS_TMP"

if [ "${1}" = "skip" ] ; then
	echo "Skipping Compilation"
else
	echo "Compiling Kernel"
	cp arch/arm64/configs/gundal_defconfig .config
	make "$@" -j19 || exit 1
fi

echo "Building new ramdisk"

rm -rf '$RAMFS_TMP'*
rm -rf $RAMFS_TMP
rm -rf $RAMFS_TMP.cpio
cp -ax $RAMFS_SOURCE $RAMFS_TMP
cd $RAMFS_TMP

find . -name '*.sh' -exec chmod 755 {} \;

$KERNELDIR/ramdisk_fix_permissions.sh 2>/dev/null 

#find . -name .git -exec rm -rf {} \;
find . -name EMPTY_DIRECTORY -exec rm -rf {} \;
cd $KERNELDIR
rm -rf $RAMFS_TMP/tmp/*

echo "packing ramdisk"
cd $RAMFS_TMP
find . | fakeroot cpio -H newc -o | lzop -9 > $RAMFS_TMP.cpio.lzo
ls -lh $RAMFS_TMP.cpio.lzo
cd $KERNELDIR

echo "Making new boot image"
~/bin/mkbootimg_tools/mkbootimg --kernel arch/arm64/boot/Image --dt dt.img --ramdisk $RAMFS_TMP.cpio.lzo --base 0x10000000 --pagesize 2048 --ramdisk_offset 0x01000000 --tags_offset 0x00000100 --second_offset 0x00f00000 -o boot.img
echo -n "SEANDROIDENFORCE" >> boot.img
#if echo "$@" | grep -q "CC=\$(CROSS_COMPILE)gcc" ; then
	dd if=/dev/zero bs=$((29360128-$(stat -c %s boot.img))) count=1 >> boot.img
#fi
echo "copying boot.img to ~/android/gundal/boot.img"
cp boot.img /android/boot.img
echo "done"
ls -al /android/boot.img
echo ""
	
