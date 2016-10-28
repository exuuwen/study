#!/bin/bash
if (($# < 1))
then
	echo "usage: `basename $0` need someting"
	exit 1
fi
num=$#
while ((num > 0))
do
	echo "$num"
	((num--))
done




