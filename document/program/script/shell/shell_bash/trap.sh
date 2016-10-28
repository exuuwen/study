#!/bin/bash
loop()
{
trap "echo ' Re_You hit control-C!'" INT	
}
trap "echo ' You hit control-C!'" INT
#trap "echo ' You hit pkill!'" TERM
trap "echo ' You hit control-\!'" QUIT


loop #re_define

while true
do
 sleep 2
 echo "ok"
done
