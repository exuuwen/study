#!/bin/bash
#testvar
if [ ! $# -ge 1 ]
then
	echo "$0 usage:one para 1 to 4"
	exit 1
fi
num=$1
name="null"
echo $#
while read num_t name_t
do
	if [ $num_t -eq $num ]
	then
	name=$name_t
	echo "num is $num:name is $name"
	fi
done < ./file
