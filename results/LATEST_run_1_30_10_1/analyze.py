import os
import subprocess

output = []
for i in range(1,31):
	if (i<10):
		s = 'gawk -f ../../select_find.awk pop.00'+str(i)+' pop.00'+str(i)+'_0.fit5 pop.00'+str(i)+'_*.fit5'
	else:
		s = 'gawk -f ../../select_find.awk pop.0'+str(i)+' pop.0'+str(i)+'_0.fit5 pop.0'+str(i)+'_*.fit5'
	
	proc = subprocess.Popen(s, stdout=subprocess.PIPE, shell=True)
	for line in proc.stdout.readlines():
	    output.append(str(i)+'$'+ line)
	
import csv
with open('output.csv', 'wb') as myfile:
    wr = csv.writer(myfile, quoting=csv.QUOTE_ALL)
    wr.writerow(output)



