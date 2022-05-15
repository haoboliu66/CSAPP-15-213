#!/bin/bash

echo "populate reference result"
for (( i=1; i<=16; i++))
do
	
	if [[ i -lt 10 ]]; then
		echo "0$i"
	./do_ref_test.sh trace0$i.txt > res_ref_trace0$i.txt
	else
		echo "$i"
	./do_ref_test.sh trace$i.txt > res_ref_trace$i.txt
	fi
done	
