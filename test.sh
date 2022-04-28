#!/bin/sh
# USAGE: ./test.sh ./solver
GRAPHS=(10_3 10_5 10_6 10_7 12_3 12_5 12_6 12_9 15_4 15_5 15_6 15_8)
# GRAPHS=(10_3 10_5 10_6 10_7 12_3 12_5 12_6 15_4 15_5 15_6)
for graph in "${GRAPHS[@]}"; do
    path=data/graf_${graph}.txt
    time_out=$(/usr/bin/time -f '%e (%U ~ %P)' $1 $path 2>&1 > tmp.txt)
    echo "$(date --iso-8601=s); ${path}; $time_out"
    name=$(basename $path)
    diff data/solutions/${name} tmp.txt
    rm tmp.txt
done
