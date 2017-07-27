
#!/bin/tcsh
#WBL 10 Jan 2012 Strip included files from cpp output $Revision: 1.2 $ 

#usage ???
#cpp -Wall -dI
#-DCOMPILER_OPTIONS="\"-O3 -m64 -msse2 -funroll-loops \""  \
#-DBOWTIE2_VERSION="\"`cat VERSION`\"" -DBUILD_HOST="\"`hostname`\"" -DBUILD_TIME="\"`date`\"" -DCOMPILER_VERSION="\"`/usr/bin/g++ -E -dI -v 2>&1 | tail -1`\"" -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -D_GNU_SOURCE  -DPREFETCH_LOCALITY=2 -DBOWTIE_MM  -DBOWTIE2 -DNDEBUG
#bowtie_main.cpp bowtie_main.eI

#gawk -f clean_cpp.awk > bowtie2-align.eI

(FNR==1) {
  if(filename!="") filename = sprintf("\"%s\"",filename);
  else {
    n = split(FILENAME,t,"/");
    m = split(t[n],tt,".");
    if(tt[m]=="") {
      print "ERROR null filename",n,t[n],m,FILENAME > "/dev/stderr";
      exit 1;
    }
    filename = sprintf("\"%s.cpp\"",tt[n]);
  }
  v = "$Revision: 1.2 $";
  printf("//clean_cpp.awk %s %s %s\n",
	 substr(v,2,length(v)-2),filename,strftime());
  line = 0;
}

($1=="#") {
  output = ($3==filename);
  #print $3 > "/dev/stderr";
  if(output) {
    target = ($4=="2")? $2 - 1 : $2;
    if($2 != old && target < line) {
      print "ERROR line",line,"less than",target,"`"$0"'",FNR > "/dev/stderr";
      exit 1;
    }
    old = $2;
    if(line<target-1) do {print ""; ++line;} while (line<target-1);
  }
}

($1!="#" && output) {
  print
  ++line;
}
