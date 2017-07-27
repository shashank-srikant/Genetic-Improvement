#!/bin/bash
c=0
for f in $(ls testcases_test/*) ; do 
	  echo $f >> runtimes
	  ./checkruns_0.sh $f >> out
	  c=$(($c+1))
	  sat=$(gawk '($1=="SATISFIABLE") {print $1;}' out)
	  unsat=$(gawk '($1=="UNSATISFIABLE") {print $1;}' out)
	  echo "finished minisat run $c"
	  if [ $sat ]
	  then
		  echo "$sat" >> runtimes
	  elif [ $unsat ]
	  then
		  echo "$unsat">> runtimes
	  else
		echo "INDETERMINATE">>runtimes
		break
	  fi
	  n=$(gawk '($1=="Log_count64:") {print $2;}' out)
	  t=$(gawk '($1=="CPU") {print $4;}' out)
	  echo "Log_count64: $n , CPU time: $t" >> runtimes
done
