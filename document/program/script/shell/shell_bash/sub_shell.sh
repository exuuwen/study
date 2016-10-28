#!/bin/bash
aa=ww;
{

    hatter=mad
    aa=ee
    trap "echo 'You hit pkill!'" TERM

}

(

    hatter=childmad
    echo "$aa in sub"
    aa=mm
    trap "echo 'You hit CTRL-C!'" INT

)#child shell 

while true; do

    echo "\$hatter is $hatter"
    echo $aa
    sleep 2

done


