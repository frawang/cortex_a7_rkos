#! /bin/bash

usage() {
	echo "usage: ./packimage.sh" 
}

CUR_DIR=$(pwd)
IMAGE=$(pwd)/Bin

rm -rf $IMAGE/RKOS.bin $IMAGE/rkos.img $IMAGE/Firmware*

cp -r $CUR_DIR/Debug/RKOS.bin $IMAGE/

$IMAGE/kernelimage --pack --kernel $IMAGE/RKOS.bin $IMAGE/rkos.img 0x62000000 > /dev/null

echo 'Image: rkos image is ready'
$IMAGE/firmwareMerger -p $IMAGE/setting.ini $IMAGE/
