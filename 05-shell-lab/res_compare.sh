#/bin/bash


# result compare script

if [[ $# -lt 1  ]]; then
echo "arg end required"
exit 2
fi

if [[ $# -eq 1 ]]; then
	id=$1
	if [[ id -lt 10 ]]; then
                gcc compare.c && ./a.out res_trace0$id.txt res_ref_trace0$id.txt
        else
                gcc compare.c && ./a.out res_trace$id.txt res_ref_trace$id.txt
        fi
exit 0
fi

echo "verify results"

end=$2

for (( i=1; i<=16; i++))
do
	echo "round: $i"
	if [[ i -lt 10 ]]; then
		gcc compare.c && ./a.out res_trace0$i.txt res_ref_trace0$i.txt
	else
		gcc compare.c && ./a.out res_trace$i.txt res_ref_trace$i.txt
	fi
done
