#!/bin/bash


./ref_populate.sh
./populate.sh
./res_compare.sh > final_result.txt
