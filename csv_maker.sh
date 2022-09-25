#!/bin/bash
# USAGE: ./csv_maker.sh "$VERTICES_$DEGREE"
# Example file: g_14_8/pd_14_8_16.sh.e169861

PATTERN=".*${1}.*\\.sh\\.e.*"
find -regex $PATTERN | while read path; do
    number=${path##*sh.e}
    echo -e "${path}\t, $(cat ${path})\t, $(cat ${path%.e*}.o$number)"
done
