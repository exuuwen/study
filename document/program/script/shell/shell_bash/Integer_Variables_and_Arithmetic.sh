#!/bin/bash
#testvar
declare -i a=10 b=2
declare -i result
result=a*b
echo "a*b is $result"
result=a/b
echo "a/b is $result"
result=a+b
echo "a+b is $result"
result=a-b
echo "a-b is $result"
result=++a
echo "++a is $result"
echo ''


c=10
d=2
result1=$((c*d))
echo "c*d is $result1"
result1=$((c/d))
echo "a/b is $result1"
result1=$((c+d))
echo "c+d is $result1"
result1=$((c-d))
echo "c-d is $result1"
result1=$((++c))
echo "++c is $result1"
result1=$(((c>d)&&(3>2)))
echo "(c>d)&&(1>2) is $result1"


for (( i=1; i <= 5 ; i++ ))
do
        for (( j=1 ; j <= 5 ; j++ ))
        do
                echo -ne "$(( j * i ))\t"
        done
        echo
done

