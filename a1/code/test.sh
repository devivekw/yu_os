#!/bin/bash

input="$1"

if [[ $input == "fcfs"  ]] || [[ $input == "sjf"  ]] || [[ $input == "rr"  ]] || [[ $input == "priority"  ]] || [[ $input == "priority_rr"  ]] 
then
    rm -f test/"$input"_out.txt && make "$input" && ./"$input" correct_input.txt > test/"$input"_out.txt && cd test && diff -y "$input"_out.txt "$input"_correct.txt
    echo "code vs correct"
fi