#!/bin/bash
#testvar
cd_dir=$1;
orgin_dir=$PWD
if [ -z $cd_dir ]
then
	echo "cd_dir is null"
elif cd $cd_dir>/dev/null 2>&1
then
	DIR_STACK="$cd_dir from ${orgin_dir}" # push the directory        
      	echo $DIR_STACK
else
	echo "wrong cd_dir ,still in the $PWD"
fi
