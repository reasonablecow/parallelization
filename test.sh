#!/bin/sh
# USAGE: ./test.sh "executable solver"
GRAPHS=(10_3 10_5 10_6 10_7 12_3 12_5 12_6 12_9 15_4 15_5 15_6 15_8)

for graph in "${GRAPHS[@]}"; do
    path=data/graf_${graph}.txt
    $1 $path > out.txt 2> err.txt
    echo "${path} , $(cat err.txt)"
    diff "data/solutions/$(basename $path)" out.txt
    rm out.txt err.txt
done
