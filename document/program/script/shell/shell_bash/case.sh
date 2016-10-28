#!/bin/bash
#testvar
if [ ! $# -ge 1 ]
then
	echo "usage: $0 data(is interge frome a to d)"
	exit 1
fi

echo "$1"
case $1 in
	a|A)
	echo "is a";;
	b|B)
	echo "is b";;
	c|C)
	echo "is c";;
	d|D)
	echo "is d";;
	*)
	echo "is invaild data";;
esac
