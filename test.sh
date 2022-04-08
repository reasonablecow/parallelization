#!/bin/sh
# USAGE: ./test.sh GRAPHS SOLUTIONS SOLVER
PATTERN="${1%/*}/graf_[0-9]+_[0-9]+\.txt"
find $1 -regex $PATTERN | while read path; do
    echo $(date -Iseconds) $path
    $3 $path > tmp.txt
    name=$(basename $path)
    diff ${2%/*}/${name} tmp.txt
done
rm tmp.txt
