#!/system/bin/sh

echo "init.fixdeepsleep.sh: start" > /dev/kmsg

echo "temporary write through" > /sys/class/scsi_disk/0:0:0:1/cache_type
echo "temporary write through" > /sys/class/scsi_disk/0:0:0:2/cache_type
echo "temporary write through" > /sys/class/scsi_disk/0:0:0:3/cache_type
