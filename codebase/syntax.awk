
#!/bin/tcsh

BEGIN {
  INC = nolog? "" : "{Log_count64++;}";
  DEC = nolog? "" : "{Log_count64--;}";
  special[""] = special["for1"] = special["for2"] = special["for3"] = 1;
  special["IF"] = special["WHILE"] = special["ELSE"] = 1;
  args = ARGV[1];
  for(i=2;i<ARGC;i++) args = sprintf("%s %s",args,ARGV[i]);
  print_header("/dev/stdout");
}

function print_header(out,  v,ocomment) {
  v = "$Revision: 1.73 $";
  ocomment = (dif)? "%" : (out=="/dev/stdout")? "#" : "/*";
  printf("%s",ocomment)                                         > out;
  printf(" syntax.awk %s ",substr(v,2,length(v)-2))             > out;
  printf("%d %s %s %s %s*/\n",
	 individual,changes,nolog,allfiles,strftime())          > out;
  printf("%s",ocomment)                                         > out;
  printf("%s*/\n",args)                                         > out;
  if(dif) return;
  if(incfile=="") return;
  printf("#include \"%s\"\n",incfile)                           > out;
  if(!nolog) {
  print "extern int64_t Log_count64;"                           > out;
  print "extern void print_log();"                              > out;
  }
}

(FNR==1) {
  file++;
  if(file==1) firstfilename = FILENAME;
  if(file==2) for(i in rule) old_rule[i] = rule[i];
  if(file>2 && FILENAME!=firstfilename) {
    print "ERROR different syntax file",firstfilename,FILENAME > "/dev/stderr";
    error = 9; exit error;
  }
}

(index($0,"::=")==0 && index($0,"#")!=1 && FNR==individual) {
  for(i in output) delete output[i];
  for(i=1;i<=NF;i++) {
    s0=index($i,">+<");
    if(s0) {
    f = substr($i,1,s0);
    b = substr($i,s0+2);
    } else {
    s=index($i,"><");
    if(s==0) {f=$i;b="";}
    else {
    f = substr($i,1,s);
    b = substr($i,s+1);
    }}
    typef = type(f);
    #check types match for sanity sake
    if(b && (typef != type(b))) {
      print "ERROR mixed types",$i,"line"FNR,FILENAME > "/dev/stderr"; 
      error = 7; exit error;
    }
    if(s0 && (typef != "")) {
      print "ERROR cannot insert type",typef,$i,"line"FNR,FILENAME > "/dev/stderr"; 
      error = 3; exit error;
    }
    c=CPP(f);
    output[c] = 1;
    if(dif && (f in modified_rule)) {
      print "ERROR latex multiple inserts not implemented",f,modified_rule[f];
      error = 10; exit error;
    }
    modified_rule[f] = (b=="")? "" : s0;
    new_rule[f]      = rule[b];
    if(b=="") {
      rule[f] = void(typef);
    } else if((f in rule) && (b in rule)) {
      rule[f] = (s0)? sprintf("%s%s%s",rule[b],INC,rule[f]) :rule[b];
    } else {  
      print "ERROR missing rule["f"] or rule["b"]",$i,"line"FNR,FILENAME > "/dev/stderr"; 
      error = 8; exit error;
    }
    change[c] = sprintf("%s %s%s%s",change[c],f,(s0)? "+":"", b);
  }
}

function type(text, t) {
  split(substr(text,2),t,"_"); 
  if(!(t[1] in special)) {print "ERROR bad type("text")">"/dev/stderr";
    error = 5; exit error;}
  return t[1];
}
function old_void(type, t) {
  if(type == "" || type == "for1" || type == "for3" ) return "";
  if(type == "for2") return "0";
  if(type == "IF" || type=="WHILE") return "(0)";
  if(type == "ELSE") return ";";
  return "@opps@ void error";
}
function void(type, t) {
  if(type == "") return DEC;
  if(type == "for1" || type == "for3" ) return "";
  if(type == "for2") return "0";
  if(type == "IF" || type=="WHILE") return "(0)";
  if(type == "ELSE") return DEC;
  return "@opps@ void error";
}
function CPP(rule,  n,t) {
  n = split(rule,t,"_");
  return substr(rule,2+length(t[1]),length(rule)-2-length(t[1])-length(t[n]));
}
function cpp_line(rule,  n,t,u,i) {
  n = split(rule,t,"_");
  u = t[2];
  for(i=3;i<n;i++) u = sprintf("%s_%s",u,t[i]);
  u = sprintf("%s.cpp & %3d &",u,substr(t[n],1,length(t[n])-1));
  if(tex) gsub(/_/,"\\_",u);
  return u;
}
function code_tex(typef,text) {
  if(typef) return sprintf("%s & %s",           typef,code(text));
  else      return sprintf("\\multicolumn{2}{c|}{%s}",code(text));
}
function code(text) {
  if(!tex) return text;
  return sprintf("\\tt %s",gensub(/_/,"\\\\_","g",text));
}

(file==1 && $2=="::=" && index($0,"#")!=1 && NF>1 && ($1 in rule)) {
  print "ERROR repeated rule definition line",FNR,$1,rule[$1] > "/dev/stderr";
  error = 1; exit error;
}

END{
  if(error) exit error;
  if(output_lines==0) {
    print "ERROR syntax.awk",individual,args,"no output" > "/dev/stderr";
    exit 6;
  }
  if((!dif) && outfiles) printf("echo \"restored %s\"\n",outfiles);
}

function eatspace(tail,  i,c) {
    for(i=1;i<=length(tail);i++){
	c = substr(tail,i,1);
	if(c!=" " && c!="\t") break;
    }
    return substr(tail,i);
}
function minpos(x,y) {
    return (x>0 && (y==0||x<y))? x : (y>0 && (x==0||y<x))? y : 0;
}
function nextquote(str,  text,a) {
  text = str;
  for(;i=index(text,"\"");) {
    if(substr(text,i-1,1)!="\\") return i+length(str)-length(text);
    text = substr(text,i+1);
  }
  return 0;
}
function nextpunctuation(str,  s,a,v) {
    s = nextquote(str);
    a = index(str,"<");
    v = index(str,"|");
    return minpos(s,minpos(a,v));
}
function closestr(  str,s) { #global tail
    str = tail;
    s = nextquote(substr(tail,2)); #exclude first char in tail
    if(s==0) {printf("ERROR in Line `%s', unterminated string `%s'\n",Line,str)> "/dev/stderr";
      error = 2; exit error;}
    tail = substr(str,s+2);
    return substr(str,1,s+1);
}
function closechr(c,  str,s) { #global tail
    str = tail;
    s = index(substr(tail,2),c); #exclude first char in tail
    if(s==0) {
	tail = "";
	return str;
    }
    else {
	s++;                     #take note that excluded first char in tail
	tail = substr(str,s+1);
	return substr(str,1,s);
    }
}
function parseLine(comp,  Line,i,n,s,c) {
    s = index($0,"::=");
    Line = substr($0,s+3);
    for(i in comp) delete comp[i];
    n=0; tail = eatspace(Line);
    for(s=1;s = nextpunctuation(tail);){
	c = substr(tail,s,1);
	if(c=="\"")     comp[++n] = closestr();
	else if(c=="<") comp[++n] = closechr(">");
	else if(c=="|"){comp[++n] = c; tail = substr(tail,s+1);}
	else {comp[++n] = tail; tail = "";}
	tail = eatspace(tail);

	if(c=="\"") {
          comp[n] = (tail=="" && substr(comp[n],length(comp[n])-2)=="\\n\"") ?
                       sprintf("%s\n",substr(comp[n],2,length(comp[n])-4)) :
                       substr(comp[n],2,length(comp[n])-2);
          gsub(/\\"/,"\"",comp[n]);
	}
    }
    return n;
}

function processLine( n,comp,i) {
  n = parseLine(comp);
  for(i=1;i<=n;i++) rule[lhs] = sprintf("%s%s",rule[lhs],comp[i]);
}

(file==1 && index($0,"#")!=1 && $2 == "::=" && 
 split(substr($1,2),t,"_") && (t[1] in special)) {
    lhs = $1;
    processLine();
}
(file>1 && index($0,"#")!=1 && $2 == "::=" && select($1)) {
  n = parseLine(comp);
  if(old != outfile) {
    if(old) close (old);
    if(dif) {
      if(old && tex) print "[2ex]";
      print_header("/dev/stdout");
    } else {
    print outfile > "/dev/stderr";
    print_header(outfile);
    s=index(outfile,".cpp");
    obj = substr(outfile,1,s-1);
    printf("rm -f %s\n",  outfile);
    printf("rm -f %s.o\n",obj);
    printf("cp -p ../sources/%s .\n",  outfile);
    printf("cp -p ../sources/%s.o .\n",obj);
    }
    outfiles = outfiles? sprintf("%s, %s",outfiles,outfile) : outfile;
  }
  for(i=1;i<=n;i++) {
    if(dif) {
      f = comp[i];
      if(f in modified_rule) {
	typef = type(f);
        l     = cpp_line(f);
	if(modified_rule[f]=="") {
	  if(typef) print "disabled&",l,code_tex(typef,old_rule[f]);
	  else      print "deleted &",l,code_tex("",   old_rule[f]);
	} else if(modified_rule[f]==0)
	  print "replaced&",l,code_tex(typef,old_rule[f]),"&",code(new_rule[f]);
        else print "inserted&",code_text("",new_rule[f]),"&before",l,code(old_rule[f]);
        if(tex) print "\\\\";
      }
    } else
    printf("%s",(comp[i] in rule)? rule[comp[i]] : (nolog? striplog(comp[i]) : comp[i])) > outfile;
  }
  output_lines++;
  old = outfile;
}

function striplog(text,  n,t,u,v) {
  s=index(text,"{Log_count64++;");
  s2 = index(substr(text,s),"}");
  if(s==0) {
    s = index(text,"print_log(); ");
    if(s==0) return text;
    s2 = s+12;
  }
  return sprintf("%s%s",substr(text,1,s-1),substr(text,s+s2));
}

function select(text,  n,t,u,v) {
  n = split(substr($1,2),t,"_");
  if(t[1] in special) return 0;
  u = substr(text,2,length(text)-length(t[n])-2);
  if(u=="") return 0;
  if(allfiles==0 && (!(u in output))) return 0;
  outfile  = sprintf("core/%s.cc",u);
  incfile  = sprintf("%s_H.h",u);
  changes  = change[u];
  v = substr(t[n],1,length(t[n])-1);
  return (n>1) && (0+v >= 1) && (gensub(/[0-9]/,"","g",t[n])==">");
}
