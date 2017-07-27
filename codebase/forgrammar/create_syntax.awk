
#!/bin/tcsh
#WBL 5 Dec 2008 $Revision: 1.51 $

#WBL 10 Jan 2012 r1.48 For use with many files
#WBL 30 Dec 2009 Revert to r1.13, disable compare,cmpfile, 2nd file
#WBL 11 Jan 2009 Make binary heirarchy of rules
#WBL 9 Jan 2009 Take advantage of long productions
#WBL 6 Jan 2009 for tcas, extract required C by hand, eg tcas_syntax.c

BEGIN{
  v = "$Revision: 1.51 $";
  printf("#create_syntax.awk %s %s\n",substr(v,2,length(v)-2),strftime());
}

(FNR==1){
#  variable = 0;
  end_file();
  n = split(FILENAME,t,"/");
  s = index(t[n],"_E.");
  filename = substr(t[n],1,s-1);
  print "#"FILENAME,filename;
  lastline = 0;
  header   = 0;
  for(i in exists) delete exists[i];
}
(index($0,"//")==1 && index($0,"Revision:")) {
  print"#"substr($0,3);
  header++;
  next;
}

#(index($1,"temp")==1) {variable=1;} #hack for scan_naive_kernel.cu
#(index($0,"}")==1)    {variable=0;}
#(NF && variable) {#hack for scan_naive_kernel.cu
#  varline[FNR] = 1;
#}
(NF) {
  lastline = FNR-header;
  exists[lastline] = 1;
  printf("%s\t::=\t",rulename(lastline));
  terminal = 0;
  for(i=1;i<=NF;i++) {
      if(terminal) printf(" ");
      else printf("\""); 
      terminal = 1;
      printf("%s",gensub(/\"/,"\\\\\\\"","g",$i));
  }
  if(terminal) printf("\\n\"");
  if(NF==0) printf("\"%s\\n\"",$0);
  printf("\n");
}

END{
  end_file();
  printf("<start>\t::=%s\n",filelist);
}

function rulename(line) { 
  return sprintf("<%s_%d>",filename,line);
}

#function rulename2(bot,top) { 
#  return (bot!=top)? sprintf("<%s_%d-%d>",filename,bot,top) : rulename(bot);
#}
function all_rules(name,bot,top, i) { #in this file
  first = 1;
  for(i=bot;i<=top;i++) {
    if(i in exists) {
      if(first) { first = 0; all[name] = 1; printf("%s\t::=\t",name); }
      printf("%s ",rulename(i));
    }
  }
  if(!first) printf("\n");
  return (!first);
}
function binary(name,bot,top, i,bot2,top2,mid) {
  if(bot<top) {
    printf("%s\t::=\t", (name!="")? name : rulename2(bot,top));
    for(i=bot;i<=top && (!(i in varline));i++) {printf("%s ",rulename(i));bot=i+1}
    bot2=bot;
    for(i=top;i> bot && (!(i in varline));i--) {;}
    top2=i;
    if(bot<top2) {
      mid = int((top2+bot)/2);
      printf("%s %s ",rulename2(bot,mid),rulename2(mid+1,top2));
      bot2=top2+1;
    }
    for(i=bot2;i<=top;i++) {
      printf("%s ",rulename(i));
    }
    printf("\n");
    if(bot<top2) {
      binary("",bot,mid);
      binary("",mid+1,top2);
    }
  }
}

function end_file() {
  if(filename!="") {
    if(all_rules(sprintf("<%s>",filename),1,lastline)) {
      filelist = sprintf("%s <%s>",filelist,filename);
    }
  }
}
