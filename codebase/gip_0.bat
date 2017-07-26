#!/bin/tcsh

echo -n 'gip_0.bat $Revision: 1.8 $' $HOST $1 $2 $3 ""
date

setenv MALLOC_CHECK_ 2
limit cputime 130

sh -c "simp/./minisat $1 2>&1"


setenv save $status
if($save == 10 || $save == 20) then 
  echo "it's ok: exit $save $1"
else
  echo "-5592179909 minisat status $save on $1" 
  exit $save;
endif

