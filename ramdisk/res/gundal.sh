#!/system/bin/sh

chmod 666 -R /sys/module/lowmemorykiller/parameters/*

	if [ ! -f /data/media/0/GuNDaL/profile_backup ]; then
		echo None > /data/media/0/GuNDaL/profile_backup
	fi

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

#sleep 20;

#echo "0x0FF3 0x041E 0x0034 0x1FC8 0xF035 0x040D 0x00D2 0x1F6B 0xF084 0x0409 0x020B 0x1EB8 0xF104 0x0409 0x0406 0x0E08 0x0782 0x2ED8" > /sys/class/misc/arizona_control/eq_A_freqs
#echo "0x0C47 0x03F5 0x0EE4 0x1D04 0xF1F7 0x040B 0x07C8 0x187D 0xF3B9 0x040A 0x0EBE 0x0C9E 0xF6C3 0x040A 0x1AC7 0xFBB6 0x0400 0x2ED8" > /sys/class/misc/arizona_control/eq_B_freqs


