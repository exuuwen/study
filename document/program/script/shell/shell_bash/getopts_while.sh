#!/bin/sh
#test shift
echo  "test the getopts"
if [ $# -eq 0 ]
then
	echo "usage: `basename $0` need [-h][-a][-d path][-n val]"
	exit 1
fi
ALL=false
NUM=0
DEVICE=`pwd`
while getopts ":d:han:" opt  #the first : used for if -d -n no argument ,it will not print error message.or it will be
			     #only accept -x,not accetp x,store the x in the "$opt" ,if x is not a,h,d,n  it store ? to "$opt"
do
	echo "left is $OPTIND" #$OPTIND  num option been handle  already
	case $opt in
	a)ALL=true
	echo "ALL is $ALL"
	;;
	d)DEVICE=$OPTARG
	echo "devce path is $DEVICE"
	;;
	n)NUM=$OPTARG
	echo "NUM is $NUM"
	;;
	h)
	echo "help do noting"
	;;	
	\?)
	echo "usage: `basename $0` wrong should [-h][-a][-d path][-n val]" #when we set -o it cannot find o .  it translate the OPTION to ?  only for -X wrong situation.
	;;
	esac	
done
 
exit 0
