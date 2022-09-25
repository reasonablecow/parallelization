#!/bin/sh
if [ "$1" = "-h"  -o "$1" = "--help" ]; then
    echo \
'Usage: cp -s job.sh ${PRE}${ver}${S}${deg}${S}${exe}${S}${pro}${S}${thr}${SUF}
  PRE - PREfix
  ver - Number of VERtices
  S   - Separator
  deg - Average vertex DEGree
  exe - EXEcutable solver
  pro - Number of PROcesses
  thr - Number of THReads
  SUF - SUFfix
Example: cp -s job.sh j16_7_pd_1_8.sh && ./j16_7_pd_1_8.sh --dry-run'
    exit 0
fi

PRE='j'
S='_'
SUF='.sh'

name=$(basename $0 $SUF)
details=${name#${PRE}}

ver=$(echo ${details} | cut -d $S -f 1)
deg=$(echo ${details} | cut -d $S -f 2)
exe=$(echo ${details} | cut -d $S -f 3)
pro=$(echo ${details} | cut -d $S -f 4)
thr=$(echo ${details} | cut -d $S -f 5)

if [ "$1" = "--dry-run" ]; then
    echo "# ver=${ver} deg=${deg} exe=${exe} pro=${pro} thr=${thr}"
    echo OMP_NUM_THREADS=${thr} build/${exe} graphs/graph_${ver}_${deg}.txt
    exit 0
fi

OMP_NUM_THREADS=${thr} build/${exe} graphs/graph_${ver}_${deg}.txt
