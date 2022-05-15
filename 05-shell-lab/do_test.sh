#!/bin/bash

if [[ $#  -ne 1 ]] ; then
	return;
fi

tracefile=$1

./sdriver.pl -t $tracefile -s ./tsh -a "-p"

