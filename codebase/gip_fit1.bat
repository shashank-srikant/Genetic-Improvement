#!/bin/tcsh

if( -e STOP ) then
  echo "gip_fit1.bat STOP" `date`
  exit 1;
endif
cd sources_$CPU/
if( -e STOP ) then
  echo "gip_fit1.bat sources_$CPU/STOP" `date`
  exit 1;
endif

setenv DF `df -k $TMPDIR/ | gawk '(index($1,"/dev")){print $4}'`
if( $DF  < 50000 ) then
  echo "Only $DF free blocks on $TMPDIR STOP"
  df -k $TMPDIR/ > STOP;
  exit 1;
endif

echo '\ngip_fit1.bat $Revision: 1.24 $' $HOST $1 $2 cpu=$CPU `date`

limit cputime 240 # sum five gip_0.bat limits plus margin

gawk -f ../syntax.awk -v "individual=$2" \
     ../Solver.bnf_GP $POPULATION ../Solver.bnf_GP >! $TMPDIR/tidy.$1_$2
if($status) exit $status;

echo -n "Compilation time "

cd simp
time make -s -f Makefile
setenv save $status
cd ..

if($save) then 
  exit 0; #compilation failures are expected
endif
echo "make status" $save

echo -n "$GIP_5_BAT $POPDIR/pop.$1_$2.fit5 "
$GIP_5_BAT "" $POPDIR/pop.$1_$2.fit5 >& /dev/null
setenv save $status
if($save) then 
  echo "$GIP_5_BAT status $save" 
  exit 0 
endif

