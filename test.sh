#!/bin/sh
# USAGE: ./test.sh GRAPHS SOLUTIONS SOLVER
PATTERN="${1%/*}/graf_[0-9]+_[0-9]+\.txt"
find $1 -regex $PATTERN | while read path; do
    time_out=$(/usr/bin/time -f '%e (%U ~ %P)' $3 $path 2>&1 > tmp.txt)
    echo "$(date --iso-8601=ns) | $path | $time_out"
    name=$(basename $path)
    diff ${2%/*}/${name} tmp.txt
done
rm tmp.txt
