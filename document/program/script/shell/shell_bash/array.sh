#!/bin/bash
#testvar

echo ${!name[@]}
name[0]=aaa
name[1]=bbbb
name[5]=cccccc
name[2]=ee
echo ${name[0]}  #打印第0个元素
echo ${name[1]}
echo ${name[5]}

echo ${!name[@]} #打印出有数据的标号
echo ${name[@]} #打印所有元素

echo ${#name[5]} #打印出该元素的长度
echo ${#name[@]} #打印出元素的个数
