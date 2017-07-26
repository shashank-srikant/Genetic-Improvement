
#!/bin/tcsh

BEGIN{
  if(seed=="") seed = 3114133;
  if(count=="") count = 10;
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

(NF){
  b = $NF;
  Bin[b] = sprintf("%s %s,%d",Bin[b],$1,$NF);
  if(b>max) max=b;
}

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
  }
  print "exit 0" > bat2
  close(bat2);
}

function log10(x) {
  return log(x)/log(10)
}
      
function bin(x){
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
  printf("../gip_0.bat ../testcases/%-17s %4d $1 >> $2\n",t[1],t[2]) > bat2;
}

#http://lawker.googlecode.com/svn/fridge/doc/faq.txt
function exists(file) {
  return (system("test -r " file) == 0);
}
