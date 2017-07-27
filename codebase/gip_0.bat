#!/bin/tcsh

#WBL 27 Feb 2012 run gismo bowtie2 Based on bowtie2_gismo_1.bat r1.1
#inner script to ease outer script gismo_H_1.bat collection of stderr

#WBL 30 Apr 2012 process bowtie2-align stdout with fitness.awk immediately
#WBL  7 Apr 2012 Reuse gismo_H_0.bat r1.1 as gip_0.bat

#on summit

echo -n 'gip_0.bat $Revision: 1.8 $' $HOST $1 $2 $3 ""
date

setenv MALLOC_CHECK_ 2
limit cputime 65

sh -c "simp/./minisat $1 2>&1"


setenv save $status
if($save == 10 || $save == 20) then 
  echo "it's ok: exit $save $1"
else
  echo "-5592179909 minisat status $save on $1" 
  exit $save;
endif

