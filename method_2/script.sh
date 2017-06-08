#!/bin/bash
times=$1;
for((n=0;n<times;n++))
do
	./main libmy.so libmy2.so
done
echo "completed"