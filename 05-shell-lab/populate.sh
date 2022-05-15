#!/bin/bash

echo "populate my result"
end=$1

for (( i=1; i<=$end; i++))
do
	
	if [[ i -lt 10 ]]; then
		echo "0$i"
	./do_test.sh trace0$i.txt > res_trace0$i.txt
	else
				echo "$i"
	./do_test.sh trace$i.txt > res_trace$i.txt
	fi
done	
