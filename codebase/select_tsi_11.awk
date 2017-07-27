
#!/bin/tcsh
#WBL 3 Feb 2012 analyse cuda/sdk/projects/1000_bowtie/count_tsi_11.bat
# $Revision: 1.9 $

#WBL  7 Apr 2012 For GiP training cases
#WBL 24 Feb 2012 For gismo_H, add MAX so can use bin2 for fewer examples 

#usage. E.g:
# gawk -f select_tsi_11.awk select_tsi_11.out select_tsi_11.out
# gawk -f select_tsi_11.awk -v seed=24144606 -v count=1 -v MAX=2000 select_tsi_11.out select_tsi_11.out > select_tsi_11.out_H5

# gawk -f select_tsi_11.awk -v seed=24144606 -v count=1 -v MAX=999 count_tsi_11.out_nomatch > /tmp/gip_training

BEGIN{
  if(seed=="") seed = 3114133;
  if(count=="") count = 10;
#  if(sample_dir=="") sample_dir = "/scratch0/NOT_BACKED_UP/crest/ucacbbl";
  if(sample_dir=="") sample_dir = "/testcases";
  if(bat2=="")       bat2       = "/tmp/gip_5.bat";
  srand(seed);
  print_header("/dev/stdout");
}
function print_header(out, v,i) {
  if(out!="/dev/stdout") printf("creating %s\n",out) > "/dev/stderr";
  v = "$Revision: 1.9 $";
  printf("#select_tsi_11.awk %s %s %s %s %s ",
	 substr(v,2,length(v)-2),seed,count,MAX,strftime()) > out;
  for(i=1;i<ARGC-1;i++) printf("%s ",ARGV[i])               > out;
  printf("%s\n",ARGV[ARGC-1])                               > out;
}

#(index($1,"SRR")==1 && $NF<=MAX) {
(NF){
  b = $NF;
  Bin[b] = sprintf("%s %s,%d",Bin[b],$1,$NF);
  if(b>max) max=b;
}

#(file==2 && index($0,"Revision:")) { print; }
#(file==2 && (FNR in out))          { print; }

END {
  first=1;
  for(i=0;i<=max;i++) {
    r = 0;
    n = split(Bin[i],t);
    if(n<=count) for(j=1;j<=n;j++) Print(t[j]);
    else for(j=1;j<=count;j++){
      do {
	Q = t[1+int(n*rand())];
	f=0;r++;
	for(k=1; f==0 && k<j; k++) if(Q in out) f=1;
      } while(f);
      Print(Q);
    }
    #print i,n,r > "/dev/stderr";
  }
  print "exit 0" > bat2
  close(bat2);
}

function log10(x) {
  return log(x)/log(10)
}
      
function bin(x){
  #return int(10*log10(x));
  return (x<2)? x : 2+int(log10(x));
}

function Print(x, t,tt,tsi,tmpfile,status) {
  out[x]=1;
  split(x,t,",")
  printf("%-17s %4d\n",t[1],t[2]);
  
  if(first) {
    print_header(bat2); 
    printf("cat < /dev/null >! $2\n")                     > bat2;
    first=0;
  }
  #based on tsi_11_1.bat r1.6
  #split(t[1],tt,"."); tsi=tt[1];
  #tmpfile = sprintf("/tmp/%s.in",t[1]);
  #if(!exists(tmpfile)) {
  #  s = sprintf("gawk '($1==\"@%s\"){on=FNR+3}(on && FNR<=on){print}' ",t[1]);
  #  s = sprintf("%s%s/%s.sample1000 > %s\n",s,sample_dir,tsi,tmpfile);
    #printf("doing %s\n",s);
  #  status = system(s);
  #  if(status){ printf("%s failed with status %s\n",s,status); exit status;}
  #}
  printf("../gip_0.bat ../testcases/%-17s %4d $1 >> $2\n",t[1],t[2]) > bat2;
}

#http://lawker.googlecode.com/svn/fridge/doc/faq.txt
function exists(file) {
  return (system("test -r " file) == 0);
}
