#!/bin/tcsh
#WBL  7 Apr 2012  $Revision: 1.24 $ based on mutate_1.bat r1.4

#WBL 3 Jun 2012 Use $POPDIR instead of $TMPDIR, disable diff

#inputs generation population_id cpu

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
#rm -f bowtie2-align
#diff ../Solver_GP.C ./core/Solver_GP.C # | egrep -v '^Only in ../sources/core:'

echo -n "Compilation time "

cd simp
time make -s -f Makefile
setenv save $status
# chmod +x $TMPDIR/tidy.$1_$2; $TMPDIR/tidy.$1_$2
cd ..

if($save) then 
#  echo "-2000 0 0 $POPDIR/pop.$1_$2.fit $save" >! $POPDIR/pop.$1_$2.fit
  exit 0; #compilation failures are expected
endif
echo "make status" $save

echo -n "$GIP_5_BAT $POPDIR/pop.$1_$2.fit5 "
#ignoring stderr output from bowtie2
$GIP_5_BAT "" $POPDIR/pop.$1_$2.fit5 >& /dev/null
setenv save $status
if($save) then 
  echo "$GIP_5_BAT status $save" 
  exit 0 #bowtie2 failures are expected
endif

#sum five fitness values calculated by gip_5.bat
#penalty of -360 for any missing

#gawk -f ../fitness5.awk $POPDIR/pop.$1_$2.fit5 >! $POPDIR/pop.$1_$2.fit

#cat $POPDIR/pop.$1_$2.fit

