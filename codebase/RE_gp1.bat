#!/bin/tcsh
#WBL  14 April 2008 $Revision: 1.51 $

#WBL  3 Jun 2012 Keep pop in $POPDIR not $TMPDIR
#WBL  1 May 2012 Move breeding new population to RE_next.bat
#WBL 26 Apr 2012 Rewrite RE_GP1.bat r1.34 for GISMO bowtie2

if( -e sources_$CPU/STOP ) then
  echo "sources_$CPU/STOP" `date`
  exit 1;
endif
if( -e STOP ) then
  echo "STOP" `date`
  exit 1;
endif

#inputs gen seed

echo "\nGen $1 seed=$2" '$Revision: 1.51 $' `date`

setenv POPULATION $POPDIR/pop.$1
setenv TRAIN      $TMPDIR/train.$1
setenv GIP_5_BAT  $TMPDIR/gip_5_$1.bat

gawk -f select_tsi_11.awk -v seed=$2 -v count=1 -v MAX=200 \
     -v "bat2=$GIP_5_BAT"					  \
     count_tsi_11.out >! $TRAIN
if($status) exit $status;
chmod +x $GIP_5_BAT

#rm -f $POPDIR/pop.$1_*.fit  |& grep -v 'No match.' #rm needed cf pop.$1_sort.fit
rm -f $POPDIR/pop.$1_*.fit5 |& grep -v 'No match.' #rm needed cf select.awk

cd sources/
#ps
echo -n "Instrumented bowtie2 $GIP_5_BAT $POPDIR/pop.$1_0.fit5 "
#ignoring stderr output from bowtie2
time $GIP_5_BAT "" $POPDIR/pop.$1_0.fit5 >& /dev/null
#instrumented bowtie2 should not fail but gip_5*.bat surpresses errors
cd ..
#gawk -f fitness5.awk $POPDIR/pop.$1_0.fit5
if($status) then
  echo "Problem $status with instrumented bowtie2 $GIP_5_BAT $POPDIR/pop.$1_0.fit5"
endif

#gip_fit500.bat $1
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

#sort -n -r $POPDIR/pop.$1_*.fit >! $POPDIR/pop.$1_sort.fit
#sort -n -r $POPDIR/pop.$1_*.fit |		\
#     gawk '(FNR<=ENVIRON["POPSIZE"]/2){print}'	\
#     >! $POPDIR/pop.$1_sort.fit
#echo "Best generation $1", `head -1 $POPDIR/pop.$1_sort.fit` `date`
