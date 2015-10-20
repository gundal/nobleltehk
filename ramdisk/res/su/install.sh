set_perm() {
  chown $1.$2 $4
  chown $1:$2 $4
  chmod $3 $4
}

cp_perm() {
  rm $5
  cat $4 > $5
  set_perm $1 $2 $3 $5 $6
}

rootsu () {
	echo $TAG "Checking SU"
	
	if [ ! -f /system/xbin/su ]; then
		echo $TAG "Injecting SU"
		
		set_perm 0 0 0777 /system/bin/.ext
		cp_perm 0 0 06755 /res/su/su /system/bin/.ext/.su
		cp_perm 0 0 06755 /res/su/su /system/xbin/su
		cp_perm 0 0 0755 /res/su/su /system/xbin/daemonsu
		cp_perm 0 0 0755 /res/su/su /system/xbin/sugote
		cp_perm 0 0 0755 /system/bin/sh /system/xbin/sugote-mksh
	
		cp_perm 0 0 0755 /res/su/supolicy /system/xbin/supolicy
		cp_perm 0 0 0644 /res/su/libsupol.so /system/lib64/libsupol.so
		
		rm /system/bin/app_process
		ln -s /system/xbin/daemonsu /system/bin/app_process
		
		mv /system/bin/app_process64 /system/bin/app_process64_original
		ln -s /system/xbin/daemonsu /system/bin/app_process64
		
		cp_perm 0 2000 0755 /system/bin/app_process64_original /system/bin/app_process_init
		
		/system/xbin/su --install
		
		echo $TAG "Finished SU injection"
	else
		echo $TAG "SU is already available"
	fi
}