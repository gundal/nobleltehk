#!/system/bin/sh
BB=/sbin/busybox


mount -o remount,rw /
mount -o remount,rw /system /system

if [ ! -f /sdcard/GuNDaL/profile_backup ]; then
	echo none > /sdcard/GuNDaL/profile_backup
fi

if [ ! -f /system/app/Synapse/Synapse.apk ]; then
	mount -o remount,rw /system
	mkdir /system/app/Synapse
	cp /res/synapse/Synapse.apk /system/app/Synapse/
	chown 0.0 /system/app/Synapse
	chmod 755 /system/app/Synapse
	chown 0.0 /system/app/Synapse.apk
	chmod 644 /system/app/Synapse.apk
	mount -o ro,remount /system
fi
	#make necessary folders
	mkdir /system/etc/init.d

	#cp /sbin/su/99SuperSUDaemon /system/etc/init.d/


	chmod 755 /system/etc/init.d
	chmod 755 /system/etc/init.d/*


#enforce init.d script perms on any post-root added files
#chmod 755 /system/etc/init.d
#chmod 755 /system/etc/init.d/*

#inject busybox if not present
#if [ ! -f /system/xbin/busybox ]; then
#	cp /sbin/busybox /system/xbin/
#	chmod 755 /system/xbin/busybox
#	/system/xbin/busybox --install -s /system/xbin
#fi

#kill securitylogagent
rm -rf /system/app/SecurityLogAgent

$BB mount -t rootfs -o remount,rw rootfs

echo 30 > /sys/module/zswap/parameters/max_pool_percent
echo 10 > /sys/module/menu/parameters/perfect_cstate_ms

echo "1 1200000 1200000 0 0 1" > /sys/class/input_booster/key/freq
echo "1 0 500 0" > /sys/class/input_booster/key/time

echo "1 1200000 1200000 0 0 1" > /sys/class/input_booster/touchkey/freq
echo "1 0 500 0" > /sys/class/input_booster/touchkey/time

echo "1 800000 800000 0 0 1" > /sys/class/input_booster/touch/freq
echo "2 800000 800000 0 0 0" > /sys/class/input_booster/touch/freq
echo "3 0 800000 0 0 0" > /sys/class/input_booster/touch/freq

echo "1 130 500 0" > /sys/class/input_booster/touch/time
echo "2 130 500 0" > /sys/class/input_booster/touch/time

sleep 20;

echo "0x0FF3 0x041E 0x0034 0x1FC8 0xF035 0x040D 0x00D2 0x1F6B 0xF084 0x0409 0x020B 0x1EB8 0xF104 0x0409 0x0406 0x0E08 0x0782 0x2ED8" > /sys/class/misc/arizona_control/eq_A_freqs
echo "0x0C47 0x03F5 0x0EE4 0x1D04 0xF1F7 0x040B 0x07C8 0x187D 0xF3B9 0x040A 0x0EBE 0x0C9E 0xF6C3 0x040A 0x1AC7 0xFBB6 0x0400 0x2ED8" > /sys/class/misc/arizona_control/eq_B_freqs


mount -o remount,ro /system



#init.d
echo "/system/etc/init.d: start" > /dev/kmsg
for f in /system/etc/init.d/*;
do
	echo "${f}: start" > /dev/kmsg;
	/system//bin/sh ${f}; 
done

