#!/bin/bash
times=$1;
for((n=0;n<times;n++))
do
	./foo libmy.so libmy2.so
done
echo "completed"