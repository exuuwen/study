#!/bin/sh
#!/bin/sh

if [ $# -eq 0 ];then
 	echo ” Usage: ip2int.sh ip”
	exit
fi

LINE=$1

#while read LINE
#do
	#echo $LINE
	a=`echo $LINE|cut -d\. -f1`
	b=`echo $LINE|cut -d\. -f2`
	c=`echo $LINE|cut -d\. -f3`
	d=`echo $LINE|cut -d\. -f4`

	#echo $a
	#echo $b
	#echo $c
	#echo $d

	itip=`expr  $a \* 256 \* 256 \* 256 + $b \* 256 \* 256 + $c \* 256 + $d `
	echo  "$itip"
#done < ./xj.lst
