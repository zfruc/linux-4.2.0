starttime=`date +'%Y-%m-%d %H:%M:%S'`
#make bzImage
make menuconfig
make modules -j8
make modules_install
make
make install
endtime=`date +'%Y-%m-%d %H:%M:%S'`
start_seconds=$(date --date="$starttime" +%s);
end_seconds=$(date --date="$endtime" +%s);
echo "本次运行时间： "$((end_seconds-start_seconds))"s"
