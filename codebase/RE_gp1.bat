#!/bin/tcsh

if( -e sources_$CPU/STOP ) then
  echo "sources_$CPU/STOP" `date`
  exit 1;
endif
if( -e STOP ) then
  echo "STOP" `date`
  exit 1;
endif

echo "\nGen $1 seed=$2" '$Revision: 1.51 $' `date`

setenv POPULATION $POPDIR/pop.$1
setenv TRAIN      $TMPDIR/train.$1
setenv GIP_5_BAT  $TMPDIR/gip_5_$1.bat

gawk -f select_tsi_11.awk -v seed=$2 -v count=1 -v MAX=200 \
     -v "bat2=$GIP_5_BAT"					  \
     count_tsi_11.out_nomatch >! $TRAIN
if($status) exit $status;
chmod +x $GIP_5_BAT

rm -f $POPDIR/pop.$1_*.fit5 |& grep -v 'No match.' 

cd sources/
echo -n "Instrumented minisat $GIP_5_BAT $POPDIR/pop.$1_0.fit5 "
time $GIP_5_BAT "" $POPDIR/pop.$1_0.fit5 >& /dev/null
cd ..
if($status) then
  echo "Problem $status with instrumented minisat $GIP_5_BAT $POPDIR/pop.$1_0.fit5"
endif

set id=1
while ( $id <= $POPSIZE )
  ./gip_fit1.bat $1 $id
  setenv save $status
  if($save) then
    if( -e sources_$CPU/STOP ) exit 1;
    if( -e STOP )              exit 1;
    echo "gip_fit1.bat $1 $id failed $save " `date`
    exit $save;
  endif
set id=(`expr $id + 1`)
end

