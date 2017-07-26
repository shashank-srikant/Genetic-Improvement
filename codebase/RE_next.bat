#!/bin/tcsh

if( -e sources_$CPU/STOP ) then
  echo "sources_$CPU/STOP" `date`
  exit 1;
endif
if( -e STOP ) then
  echo "STOP" `date`
  exit 1;
endif

echo "nextgen $1 -> $2 seed=$3" '$Revision: 1.5 $' `date`

gawk -f select.awk $POPDIR/pop.$1 $POPDIR/pop.$1_*.fit5 \
     >! $POPDIR/pop.$1_select

set mutants=(`expr $POPSIZE / 2`);
set crosses=(`expr $POPSIZE - $mutants`);

gawk -f mutation.awk -v "seed=1$3"		\
     -v "pPopSize=$mutants" -v "nCross=$mutants"\
     lines_used.dat $POPDIR/pop.$1_select	\
     >! $POPDIR/pop.$2

date
