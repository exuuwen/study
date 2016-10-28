#!/bin/sh
#test shift
set -x  #跟踪脚本
echo  "test the shift"
if [ $# -eq 0 ]
then
	echo "usage: `basename $0` need someting"
	exit 1
fi

while [ $# -ne 0 ]
do
	echo "$1"
	shift 2
done
set +x
exit 0
