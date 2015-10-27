#!/system/bin/sh

cat << CTAG
{
    name:FS,
    elements:[
    	{ SDescription:{
        	description:"\n\tFor now, this tab just displays the status of the three main partitions.\n\n",
        	background:0
        }},
    	{ SLiveLabel:{
		refresh:10000000,
		title:"Filesystem of /cache Partition",
		style:"normal",
		action:"
		if grep -q 'cache f2fs' /proc/mounts ; then
			echo F2FS;
		else
			echo EXT4;
		fi;"
	}},
	{ SSpacer:{
		height:1
	}},
	{ SLiveLabel:{
		refresh:10000000,
		title:"Filesystem of /data Partition",
		style:"normal",
		action:"
		if grep -q 'data f2fs' /proc/mounts ; then
			echo F2FS;
		else
			echo EXT4;
		fi;"
	}},
	{ SSpacer:{
		height:1
	}},
	{ SLiveLabel:{
		refresh:10000000,
		title:"Filesystem of /system Partition",
		style:"normal",
		action:"
		if grep -q 'system f2fs' /proc/mounts ; then
			echo F2FS;
		else
			echo EXT4;
		fi;"
	}},
	{ SSpacer:{
		height:1
	}},
	{ SPane:{
		title:"Filesystem Controls",
		description:""
        }},
	{ SButton:{
		label:"Remount /system as Writeable",
		action:"mount -o remount,rw \/system;
		echo Remounted \/system as Writable!;"
	}},
	{ SSpacer:{
		height:1
	}},
	{ SButton:{
		label:"Remount /system as Read-Only",
		action:"mount -o remount,ro \/system;
		echo Remounted \/system as Read-Only!;"
	}},
	{ SSpacer:{
		height:1
	}},
	{ SButton:{
		label:"Remount RootFS as Writeable",
		action:"/sbin/busybox mount -t rootfs -o remount,rw rootfs;
		echo Remounted RootFS as Writable!;"
	}},
	{ SSpacer:{
		height:1
	}},
	{ SButton:{
		label:"Remount RootFS as Read-Only",
		action:"/sbin/busybox mount -t rootfs -o remount,ro rootfs;
		echo Remounted RootFS as Read-Only!;"
	}},
	{ SSpacer:{
		height:2
	}},
	{ SPane:{
		title:"Scrolling Cache control",
		description:" Disable to increase in-app scrolling speed and responsiveness. Default is (2). If you experience problems, set to (1). REBOOT REQUIRED !!"
	}},
	{ SDescription:{
		description:"\n 0- force to enable regardless of app setting.\n 1- enable unless app specifies.\n 2- disable unless app specifies.\n 3- force to disable regardless of app setting.\n"
	}},
	{ SOptionList:{
		default:`echo $(/res/synapse/actions/devtools scr_cache)`,
                action:"devtools scr_cache",
		values:[0, 1, 2, 3,]
	}},
	{ SSpacer:{
		height:1
	}},
	{ SButton:{
		label:"Optimize Databases",
		action:"devtools optimizedb"
	}},
	{ SSpacer:{
		height:1
	}},
	{ SPane:{
		title:"Wipe Options",
	}},
	{ SSpacer:{
		height:1
	}},
	{ SButton:{
		label:"Wipe Cache Reboot",
		action:"devtools wipe_cache_reboot"
	}},
	{ SSpacer:{
		height:1
	}},
	{ SButton:{
		label:"Wipe Dalvik-Cache Reboot",
		action:"devtools wipe_dalvik_reboot"
	}},
	{ SSpacer:{
		height:1
	}},
	{ SButton:{
		label:"Wipe Cache & Dalvik-Cache Reboot",
		action:"devtools wipe_cache-dalvik_reboot"
	}},
	{ SDescription:{
		description:""
	}},
	{ SPane:{
		title:"Wipe Junk Folders",
		description:" * clipboard-cache\n * tombstones\n * anr logs\n * dropbox logs\n * lost+found"
	}},
	{ SSpacer:{
		height:1
	}},
	{ SButton:{
		label:"Clean up Junk",
		action:"devtools clean_up"
	}},
	{ SSpacer:{
		height:1
	}},
    ]
}
CTAG
