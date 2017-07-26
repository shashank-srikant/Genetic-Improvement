#!/bin/tcsh

setenv POPSIZE  $2
setenv MAXGEN   $3
setenv CPU      $4  #1--8

echo -n "RE_gp.bat seed $1, pop $POPSIZE gens $MAXGEN cpu $CPU starting "; date

setenv start `pwd`
cd $start

setenv TMPDIR   /tmp/RE_$CPU
mkdir -p $TMPDIR
setenv POPDIR $start/sources_$CPU

gawk -f mutation.awk					\
     -v "pPopSize=$POPSIZE"  -v seed=$1"33936"		\
     lines_used.dat >! $POPDIR/pop.000
if($status) exit $status;

set g=0
while ( $g <= $MAXGEN )
  set gen=(`printf "%03d" $g`)
  ./RE_gp1.bat $gen 2$g$1
  if($status) exit $status;
  set g=(`expr $g + 1`)
  if ( $g <= $MAXGEN ) then
    ./RE_next.bat $gen `printf "%03d" $g` 1$g$1
    if($status) exit $status;
  endif
end

echo "RE_gp.bat done" $gen `date`
