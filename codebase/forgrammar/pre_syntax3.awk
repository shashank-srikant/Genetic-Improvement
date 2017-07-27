
#!/bin/tcsh
#WBL 8 March 2012 based on pre_syntax2.awk r1.51 aand pre_syntax.awk r1.18 $Revision: 1.31 $

#usage: gawk -f pre_syntax3.awk sources/*.h sources/*_E.cpp bowtie2.bnf

BEGIN{
  if(last_rule== "") last_rule ="<bt2_search_3239>";
  if(log_file=="") log_file = "/dev/null"; #"gismo_H.cpp";
  v = "$Revision: 1.31 $";
  printf("#pre_syntax3.awk %s %s %s\n#",
  #	 substr(v,2,length(v)-2),log_file,strftime());
  	 substr(v,2,length(v)-2),log_file,"Thu Mar 08 15:57:17 GMT 2012");

  #if(first_rule=="") first_rule="<bowtie_main_43>";
  printf("/*pre_syntax2.awk %s %s*/\n",
         substr(v,2,length(v)-2),strftime()) > log_file;
  printf("#include <iostream>\n")            > log_file;
  printf("using namespace std;\n")           > log_file;
  printf("int* Log_count;\n")                > log_file;
  printf("extern void print_log() {\n")      > log_file;
}

(FNR==1) {
  last2 = substr(FILENAME,length(FILENAME)-1);
  last4 = substr(FILENAME,length(FILENAME)-3);
  last5 = substr(FILENAME,length(FILENAME)-4);
  if(last4 == "_H.h" || last5 == "_GP.h") nextfile;
  bnf     = last4 == ".bnf";
  source_ = last2 == ".h" || last4 == ".cpp";
  printf("%s ",FILENAME);
  if(bnf) printf("\n");
}
function source() {return source_;}

(index($0,"#")==1 && index($0,"Revision:")) {print;}

(source() && ($1=="struct" || $1=="class")) {
  u = sprintf("%s %s",$1,FILENAME);
  if(substr($2,length($2))==";") type[substr($2,1,length($2)-1)] = u;
  else                           type[$2]                        = u;
}
(source() && $1=="typedef") {
  if(substr($NF,length($NF))!=";") {
    print "ERROR unexpected typedef syntax line"FNR,FILENAME,$0 >"/dev/stderr";
    error = 1; exit error;
  }
  type[substr($NF,1,length($NF)-1)] = sprintf("%s %s",$1,FILENAME);
}

($2=="::=" && NF>=3) {
# if(substr($0,length($0)-3)=="(\\n\"")  {open = 1; }
  if((index($0,"struct ") ||index($0,"extern \\\"C\\\"") ||index($0,"enum "))&&
     substr($0,length($0)-3)=="{\\n\"") {
    open = 1; #assume struct/extern not nested
    last = FNR; #avoid counting this line twice
    #print "open";
  }
}

($2=="::=" && substr($0,length($0)-3)=="(\\n\"")  { open_bracket = 1; }
($2=="::=" && substr($0,length($0)-3)==",\\n\"" && open_bracket==0)  { 
  if(balanced(substr($0,s+1,length($0)-4-s),"(",")")>0) open_bracket = 1;
}

($2=="::=" && NF>=3) {
# Log=sprintf("/*Log(\\\"%s\\\");*/",substr($1,2,length($1)-2));
# Log=sprintf("Log(\\\"%s\\\");",substr($1,2,length($1)-2));
  new_overflow = 0;
  if($1==last_rule) IF="print_log();"
  else {
    n = split($1,t,"_");
    f = substr(t[1],2);
    for(i=2;i<n;i++) f = sprintf("%s_%s",f,t[i]);
    Log=sprintf("{Log_count64++;/*%d*/}",cpp(f,substr(t[n],1,length(t[n])-1)));
    IF=sprintf("if(ON(%d))",cpp(f,substr(t[n],1,length(t[n])-1)));
  }
  #if($1==first_rule) Log=sprintf("init_log();%s",Log);
  s=index($0,"\"");
  rhs=substr($0,s);
  if($1==last_rule) printf("%sprint_log(); %s\n",
			   substr($0,1,s),substr($0,s+1));
  else if(index($0,"#")==1)                              {print; }
# else if(index($3,"<")==1)                              {print; }
  #ignore lines which are just a single ;
  else if($3=="\";\\n\"")                                {print; }
  else if($3=="\");\\n\"")                               {print; }
  #ignore lines composed of single string followed by ;
  else if(substr($3,1,3)          == "\"\\\""      && 
          substr($3,length($3)-5) == "\\\";\\n\"")       {print; }
  else if(index($3,"\"#include")==1) {;} #printf("%s//%s\n",
					 #    substr($0,1,s),substr($0,s+1)); }
  else if(overflow                          && 
	  substr(rhs,1,2)=="\"("            &&
	  substr($0,length($0)-3)==")\\n\"" && 
	  Balanced(rhs)                        ) {
    new_overflow = 1;
    print;
  }
  else if(open==1 || open_bracket || open_conditional || overflow) {print; }
  else if(index($3,"\"")==1 && declaration(substr($3,2))){
    s2=index(rhs,"=");
    if(s2 && balanced(substr(rhs,s2+1),"{","}")>0) open_bracket = 1;
    print;
  }
  else if($3=="\"case"){
    s2=s+index(rhs,":");
    s3=index(substr($0,s2),"break;");
    if(s3) {
      u0= substr($0,s2,s3-1);
      u = substr($0,s2+s3-1);
      printf("%s %s%s%s\n",substr($0,1,s2-1),Log,rule_name2("",u0),u);
      print_rule("",u0,"case_break");
    } else {
      u = substr($0,s2,length($0)-s2-2);
      printf("%s %s%s\\n\"\n",substr($0,1,s2-1),Log,rule_name2("",u));
      print_rule("",u,"case");
    }
  }
  else if(index2(rhs,"for(",S)==2) {
    s2 = S[2]; u=nestround(substr(rhs,s2));
    n = split(substr(u,2,length(u)-2),t,";"); 
    #print n,u,"`"t[1]"'","`"t[2]"'","`"t[3]"'","line"FNR,$0;
    if(n==3 &&substitutable(t[1])&&substitutable(t[2])&&substitutable(t[3])) {
      u2 = substr(rhs,s2+length(u));
      printf("%sfor(\" %s \";\" %s \";\" %s \") %s\n",
             substr($0,1,s),
	     rule_name("for1"),rule_name("for2"),rule_name("for3"),u2);
      print_rule("for1",t[1],""); 
      print_rule("for2",t[2],""); 
      print_rule("for3",t[3],"");
    } else print;
  }
  else if(index2(rhs,"if(",S)==2) {
    s2 = S[2]; u=nestround(substr(rhs,s2));
    u2 = substr(rhs,s2+length(u));
    printf("%s%s if%s %s\n",substr($0,1,s),Log,rule_name2("IF",u),u2);
    print_rule("IF",u,"\"if");
  }
  else if(s2=index($0,"return")){
    printf("%s%s %s\n",substr($0,1,s2-1),Log,substr($0,s2));
  }
  else if(s2=index($0,"break")){
    printf("%s%s %s\n",substr($0,1,s2-1),Log,substr($0,s2));
  }
  else if(s2=index($0,"continue;")){
    printf("%s%s %s\n",substr($0,1,s2-1),Log,substr($0,s2));
  }
  else if((s2=index(rhs,"<<")) && 
	  (index(substr(rhs,s2+2),"\\\"") || index(substr(rhs,s2+2),"endl"))){
    u = substr(rhs,2,s2-2);
    n=split(u,t);
    #printf("rhs `%s' u `%s' s2=%d n=%d\n",rhs,u,s2,n);
    if(n==0) print;
    else printf("%s%s %s\n",substr($0,1,s),Log,substr($0,s+1));
    #actually does right thing for lines 2697 and 2703 of bowtie2.bnf
    #if(n>1) {print "Warning","`"u"'",n,"`"t[1]"'","`"t[2]"'","unexpected << syntax","line"FNR,FILENAME,$0 > "/dev/stderr";
    #  error = 1; exit error;
    #}
  }
  else if(substr($0,length($0)-5)=="; }\\n\"") {
    s2=index($0,"{")
    u = substr($0,s2+1,length($0)-s2-5);
    printf("%s%s %s }\\n\"\n",substr($0,1,s2),Log,rule_name2("",u));
    print_rule("",u,"; }");
  }
  else if(rhs=="\"}\\n\"" || rhs=="\"};\\n\""){
    printf("%s\n",$0);
  }
  else if(index2(rhs,"while(",S)) {
    s0=s+S[1]-2; s2=S[2];
    u=nestround(substr(rhs,s2)); u2 = substr(rhs,s2+length(u));
    printf("%swhile%s %s\n",substr($0,1,s0),rule_name2("WHILE",u),u2);
    print_rule("WHILE",u,"WHILE");
  }
  else if(s2=index2(rhs,"if(",S)) {# && index(rhs,"{")==0) {
    u=nestround(substr(rhs,S[2]));
    s3=S[2]+length(u);
    printf("%sif%s %s\n",
	   substr($0,1,s+s2-2),rule_name2("IF",u),substr(rhs,s3));
    print_rule("IF",u,"if2");
  } else if((s2=index(rhs,"else ")) && substr($0,length($0)-3)==";\\n\"" ){
    u = substr(rhs,s2+5,length(rhs)-s2-7);
    printf("%s{%s %s};\\n\"\n",
	   substr($0,1,s+s2+3),rule_name2("ELSE",u),Log);
    print_rule("ELSE",u,"ELSE");
  }
  else if(substr($0,length($0)-3)==";\\n\"" ){
    u = substr($0,s+1,length($0)-s-3)
    printf("%s%s%s\\n\"\n",substr($0,1,s),rule_name2("",u),Log);
    print_rule("",u,"other");
  }
  else if($3=="\"{\\n\""){
    printf("%s\n",$0);
  }
  else if(substr($0,length($0)-3)=="=\\n\"") {
    printf("%s%s %s\n",substr($0,1,s),Log,substr($0,s+1));
  } else                                                 {print; }
  #set ready for next line
  overflow = (new_overflow                      ||
	      substr($0,length($0)-3)=="=\\n\"" ||
	      substr($0,length($0)-3)=="|\\n\"" || #includes ||
	      substr($0,length($0)-3)=="&\\n\"" || #includes &&
	      substr($0,length($0)-4)==" +\\n\"" ); 

  if((!open_bracket) &&     #no need to track ?: inside func argument call
     (!open_conditional)) { #assume conditional exprs are never nested
    #may be only need to check !open_bracket, but seems ok with both checks
    s2 = index(rhs,"? ");
    if(s2) {
      if(substr($0,length($0)-3)==":\\n\"") open_conditional = 1;
      else {
	u = substr(rhs,s2+2);
	s2 = index(u," : ");
	u = substr(rhs,s2+3);
	if(s2 && (index(u,";")==0)) open_conditional = 1;
	#print "open_conditional",open_conditional,s2,"`"u"'";
  }}}
}

($2=="::=" && open && FNR!=last) {
  for(i=1;i<=length($0);i++) {
    c=substr($0,i,1);
    if(c=="{") open++;
    if(c=="}") open--;
  }
  #if(open<=0) print "close",open;
}

($2=="::=" && substr($0,length($0)-3)==")\\n\"")  { open_bracket = 0; }
($2=="::=" && substr($0,length($0)-4)==");\\n\"") { open_bracket = 0; }
($2=="::=" && substr($0,length($0)-9)==") const\\n\"") { open_bracket = 0; }
($2=="::=" && substr($0,length($0)-4)=="};\\n\"") { open_bracket = 0; }

($2=="::=" && substr($0,length($0)-3)=="?\\n\"") { open_conditional = 1; }
($2=="::=" && substr($0,length($0)-3)==";\\n\"") { open_conditional = 0; }

function index2(text,text2,S, L,s) {#for use with 'if (' etc
  L = length(text2);
  s = index(text,text2);
  if(s) {S[1]=s; S[2]=s+L-1; return s;}
  s = index(text,sprintf("%s %s",substr(text2,1,L-1),substr(text2,L,1)));
  if(s) {S[1]=s; S[2]=s+L;   return s;}
  return 0;
}

#http://www.catonmat.net/blog/awk-one-liners-explained-part-two/
#Delete both leading and trailing whitespaces from each line (trim).
function trim(text) {
  gsub(/^[ \t]+|[ \t]+$/, "", text); #overwrites text but text is input only
  return text;
}

function substitutable(text) {
  if(index(text,"return")) return 0;
  return Balanced(text);
}
function rule_name(type) {
  return sprintf("<%s_%s",type,substr($1,2));
}
function rule_name2(type,text) {
  return substitutable(text)? sprintf("\" %s \"",rule_name(type)) : text;
}
function print_rule(type,text,comment) {
  if(substitutable(text)) {#test must be identical to that in rule_name2
    if(comment)printf("#%s\n",comment);
    printf("%s\t::=\t\"%s\"\n",rule_name(type),trim(text))
  }
}

function declaration(text, ans) {
  if(index(text,"(")) return 0; //assume it will be executable;
  return  \
  (istype(text) ||
   text=="using" ||
   text=="static" ||
   text=="template<typename" ||
   text=="T" ||
   text=="struct" ||
   text=="const" ||
   text=="typedef" ||
   template(text));
#  print "declaration("text")",ans;
#  return ans;
}
function stdtype(text) {
  return  \
  (text=="int" ||
   text=="bool" ||
   text=="int16_t" ||
   text=="int32_t" ||
   text=="uint32_t" ||
   text=="int64_t" ||
   text=="uint64_t" ||
   text=="size_t" ||
   text=="string" ||
   text=="long" ||
   text=="char" ||
   text=="uint8_t" ||
   text=="void" ||
   text=="float" ||
   text=="double" ||
   text=="time_t" ||
   text=="pair" ||
   text=="__m128i" ||
   text=="ofstream" ||
   text=="ifstream" ||
   text=="stringstream" ||
   text=="ostringstream" ||
   text=="istringstream" ||
   text=="ssize_t" ||
   text=="FILE" );
}
function istype(text, a,c) {
  a = anytype(text);
  if(a==0) {
    c = substr(text,length(text));
    if(c=="*" || c=="&") a = anytype(substr(text,1,length(text)-1));
  }
  #printf("istype(%s) '%s' a=%s\n",text,c,a);
  return a;
}
function anytype(text) {
  return stdtype(text) || (text in type);
}
function template(text, i,c,u,s,s2) {
  #printf("template(%s) ",text);
  if(!isletter(substr(text,1,1))) return 0;
  s=index(text,"<");
  if(s==0) return 0;
  u = substr(text,s);
  s =index(u,">");
  s2=index(u,",");
  #printf("%d %d `%s'",s,s2,u);
  if(s==0 && s2==0) return 0;

  #printf("%s",substr(text,1,1));
  for(i=2;i<=length(text);i++) {
    c = substr(text,i,1);
    #printf("%s",c);
    if(c=="<") {
      #printf("\n");
      if(istype(substr(text,1,i-1))) return 1;
      u = substr(text,i+1,length(text)-i);
      s=index(u,",");
      if(s) u = substr(u,1,s-1);
      s=index(u,">");
      if(s) u = substr(u,1,s-1);
      #print "["u"]",s;
      if(istype(u)) return 1;
      return template(u);
    }
    if(c==":") { #check for ::
      i++;
      c = substr(text,i,1);
      #printf("%s",c);
      if(c!=":") return 0;
      else       continue;
    }
    if(!isidchar(c)) return 0;
  }
  return 0;
}

function isidchar(c) {
  if(isletter(c)) return 1;
  return (c=="_" || (c>="0" && c<="9"));
}
function isletter(C, c) {
  c =  tolower(C)
  return (c>="a" && c<="z");
}

function print_line() {
    printf("for(int i=%5d;i<=%5d;i++) ",start,start+max) > log_file;
    printf("if(Log_count[i]) ")                          > log_file;
    printf("printf(\"%-21s %%5d %%7d\\n\",i-%-5d,Log_count[i]);\n",
	   old,start)                                    > log_file;
}
function cpp(file,line) {
  if("0"+line!=line) return "?error?";
  if(file!=old && old!="") {
    print_line();
    start += max+1;
  }
  max = line;
  old = file;
  #printf("cpp(%s,%s) %s %s+%s=%s\n",file,line, $0,start,line,start + line);
  return start + line;
}

function Balanced(text, n,i,c) {
  O["["] = O["("] = O["{"] = 1;
  C["]"] = C[")"] = C["}"] = 1;
  n=0;
  for(i=1;i<=length(text);i++) {
    c=substr(text,i,1);
    if(     c in O) n++;
    else if(c in C) n--;
  }
  return n==0; #<<<<!!!!
}
function balanced(text,open,end, n,i,c) {
  #printf("balanced(%s,%s,%s) ",text,open,end);
  n=0;
  for(i=1;i<=length(text);i++) {
    c=substr(text,i,1);
    if(     c==open) n++;
    else if(c==end ) n--;
  }
  #printf("%d\n",n);
  return n;
}
function nestround(text, n,i,c) {
  n=0;
  for(i=1;i<=length(text);i++) {
    c=substr(text,i,1);
    if(     c=="(") n++;
    else if(c==")") n--;
    if(n==0) return substr(text,1,i);
  }
  #print "Warning nestround `"text"' line"FNR,FILENAME,$0 > "/dev/stderr";
  #error = 2; exit error;
  return text;
}

END{
  #for(i in type) print i,type[i]; exit;
  if(error) exit error;
  print_line();
  printf("}\n")                                       > log_file;
  printf("int Log_initialised = 0;\n")                > log_file;
  printf("extern void Log(const int line) {\n")       > log_file;
  printf("  if(Log_initialised==0) {\n")              > log_file;
  printf("    Log_count = new int[%s];\n",           start+max+1) > log_file;
  printf("    memset(Log_count,0,%s*sizeof(int));\n",start+max+1) > log_file;
  printf("    Log_initialised = 1;\n")                > log_file;
  printf("  }\n")                                     > log_file;
  printf("  Log_count[line]++;\n")                    > log_file;
  printf("}\n")                                       > log_file;
  close(log_file);
}
