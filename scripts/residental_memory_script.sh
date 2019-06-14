#!/bin/bash

# Basic while loop
resmem=0
summary=0
counter=0
while [ $resmem -ne 1296 ]
do
resmem=$(top -b -n1 | grep "app" | head -1 | awk '{print $6}')
echo $resmem
summary=$((summary+resmem))
((counter++))
done

echo All done
echo $s
echo Average
echo $((summary/counter))
