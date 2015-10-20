#!/system/bin/sh

echo 400000 > /sys/devices/system/cpu/cpu0/cpufreq/scaling_min_freq
echo 1400000 > /sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq
echo 2100000 > /sys/devices/system/cpu/cpu4/cpufreq/scaling_max_freq

	if [ ! -f /data/media/0/GuNDaL/profile_backup ]; then
		echo None > /data/media/0/GuNDaL/profile_backup
	fi



