#!/bin/bash
#testvar
IFS=:  #This sets the IFS to be a colon

for dir in $PATH

do

    echo   "$dir"

done

for data in $@
do
    echo "$data"
done
