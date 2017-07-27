
#!/bin/tcsh
#WBL 25 April 2012  $Revision: 1.21 $

#WBL  4 Apr 2012 Add referring to real bowtie2 runs
#cf gip_fit1.bat r1.17, gen_0_fit2.awk
#WBL 26 Apr 2012 Rename select.awk and simplify

# gawk -f t.awk /tmp/pop.000 /tmp/pop.0_*_1.fit
# gawk -f gen_0_fit.awk /tmp/pop.000 /tmp/pop.000_sort1.fit
# gawk -f select.awk $TMPDIR/pop.$1 $TMPDIR/pop.$1_*.fit5

BEGIN{
  print_header("/dev/stderr");
}
function print_header(out,  v,i) {
  v = "$Revision: 1.21 $";
  #printf("#select.awk %s %s %s \n",
#	 substr(v,2,length(v)-2),ARGC,strftime())            > out;
}
(FNR==1) {
  file++;
  fnr=0;
  filename[file]=FILENAME;
  n = split(FILENAME,t,"/");
  n = split(t[n],tt,"_");
  n = split(tt[2],t3,".");
  filenumber[file]=t3[1];
  #filename[file] contains * from pop.$1_*.fit5
  gen = tt[1];
}

(file==1 ) {
  pop[FNR]=$0;
  max=FNR;
  #pop contains individuals and max is the number of individuals in pop.$1
}

(file==2){
  if(index(FILENAME,"_0.fit")==0) {
    print "ERROR select.awk missing reference fitness. Line"FNR,FILENAME;exit 1
  }
}

(file>=2 && $1=="Log_count64:" && $2){
  nok[file]++
  fnr++;
  if(file==2) { zero_64[fnr] = $2;} 
  count64[file] += $2;
  countl[file] = $2;
  if(count64[file] > max64) max64 = count64[file];
  check = $2 / zero_64[fnr];
}

(file>=2 && $1=="it's" && $4){
  if(file==2) { zero_ok[fnr] = $4;}
  if(file>2){
    if($4 == zero_ok[fnr]) {
      better[file] = better[file] + 240; 
      if(check < 1 ) better[file] = better[file] + 50; 
      if(check < 0.95 ) better[file]++; 
      if(check < 0.9 ) better[file]++; 
      if(check < 0.85 ) better[file]++; 
      if(check < 0.8 ) better[file]++; 
      if(check < 0.75 ) better[file]++; 
      if(check < 0.7 ) better[file]++; 
      if(check < 0.65 ) better[file]++; 
      if(check < 0.60 ) better[file]++; 
      if(check < 0.55 ) better[file]++; 
      if(check < 0.50 ) better[file]++; 
    }
  }
  check = 1;
  # works assuming 'it's ok' occurs only if 'Log_count64' exists
}


END {
  for(i=3;i<=file;i++) {
    if(i in better && better[i]<1200) delete better[i];
    if(i in better) {
      printf("gen: %s ind: %d fit: %d lines %d lastline %d \n",gen,filenumber[i],better[i],count64[i],countl[i]);
      nok_=nok[i];
      sortx[i] = sprintf("%01d|%020d,%s",nok_,max64-count64[i],i);
      if(nok_==5) nok5++;
    }
  }
  n = asort(sortx);
  for(j=n;j > 0 && j > (n-max/2); --j) { #best first
    split(sortx[j],t,","); f=t[2];i=filenumber[f];
    #if(j==n)printf("%d %d %d %s %d %s,",
#		   j,nok5,nok[f],count64[2]-count64[f],
 #                   better[f],filename[f]) > "/dev/stderr";
  #  print pop[i];
    last=j;
  }
  #printf(" %d %d %s %d %s\n",
   #      (n-last+1),nok[f],count64[2]-count64[f],
    #                better[f],filename[f]) > "/dev/stderr";
}

function min(a,b)    { return (a<=b)? a : b;   }

function type(text) {
  if(index(text,">+<")) return "insert ";
  if(index(text,"><"))  return "replace";
                        return "delete "
}

function p(x,y){
  return (y)? x/y : x;
}
