#!/bin/tcsh
#WBL 1 May 2012 created from RE_gp1.bat r1.43 $Revision: 1.5 $

#WBL  3 Jun 2012 pop in sources_$CPU

if( -e sources_$CPU/STOP ) then
  echo "sources_$CPU/STOP" `date`
  exit 1;
endif
if( -e STOP ) then
  echo "STOP" `date`
  exit 1;
endif

#inputs gen next_gen seed

echo "nextgen $1 -> $2 seed=$3" '$Revision: 1.5 $' `date`

#rm -f /tmp/pop.$1.best /tmp/pop.$1.mut /tmp/pop.$1.xo /tmp/pop.$2.fit
#rm -f $OLD_POPULATION $TRAIN 

#removed stuff dealing with NSGA-II interface

gawk -f select.awk $POPDIR/pop.$1 $POPDIR/pop.$1_*.fit5 \
     >! $POPDIR/pop.$1_select
#cp $POPDIR/pop.$1 $POPDIR/pop.$1_select

set mutants=(`expr $POPSIZE / 2`);
set crosses=(`expr $POPSIZE - $mutants`);

gawk -f mutation.awk -v "seed=1$3"		\
     -v "pPopSize=$mutants" -v "nCross=$mutants"\
     lines_used.dat $POPDIR/pop.$1_select	\
     >! $POPDIR/pop.$2

date
