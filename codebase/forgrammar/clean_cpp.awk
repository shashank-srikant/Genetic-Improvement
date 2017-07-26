#!/bin/tcsh

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
