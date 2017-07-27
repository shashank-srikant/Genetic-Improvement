
#!/bin/tcsh
#WBL 24 April 2012 Based on create_lines_used.awk r1.2 $Revision: 1.26 $

#WBL also use for mutation
#usage: 
#gawk -f mutation.awk -v "pPopSize=40" -v "seed=999" lines_used.dat
#gawk -f mutation.awk -v "seed=555" -v "pPopSize=20" lines_used.dat /tmp/RE_5/pop.000_select

function print_head(out,  v,i) {
  v = "$Revision: 1.26 $";
  printf("#mutation.awk %s %s %s %s %s ",
	 substr(v,2,length(v)-2),pPopSize,nCross,seed,strftime()) > out;
  for(i=1;i<ARGC-1;i++) printf("%s ",ARGV[i])  > out;
  printf("%s ",ARGV[ARGC-1])                   > out;
}

BEGIN {
  if(pPopSize=="") pPopSize =      1;
  if(seed=="")     seed     = 180407;
  srand(seed);
  print_head("/dev/stderr");
}

(FNR==1) {file++;}

(file==1 && index($0,"#")!=1) {
  ++nrule;
  rule[nrule]   = $1;
  weight[$1]    = $2;
  prod[$1] = $3;
  if($3) {
    cpp = CPP($1);
    split(substr($1,2,length($1)-2),t,"_");
    if(!((cpp,t[1],$3) in prod3)) {
      prod3[cpp,t[1],$3] = 1;
      rule3[cpp,t[1],++nrule3[cpp,t[1]]] = $1;
    }
  }
}

(file>1 && index($0,"#")!=1) {
  pop[++old_size] = $0;
  unique[$0]=1;
}

END{
  nmut = min(old_size,pPopSize);
  nxo  = min(old_size,nCross);
  for(i=1;i<=nmut;i++) {
    set_weights(i);
    do {
      new = mutant(i);
    } while(new in unique);
    unique[new]=1; print new;
  }
  for(i=0;i<nxo;i++) {
    for(j=0;j<5;j++) {
      m = i%old_size;
      d = (m + 1 + rnd(old_size-1))%old_size;
      new = cross(m+1,d+1);
      if(!(new in unique)){nCrossOut++; unique[new]=1; print new; break;}
    }
  }
  for(i=1;i<=nxo-nCrossOut;i++) {
    set_weights(i);
    do {
      new = mutant(i);
    } while(new in unique);
    unique[new]=1; print new;
  }
  #Initial pop, or if old_size was too small. Create missing guys at random
  nInit = (pPopSize+nCross) - (nmut+nxo);
  set_weights(0);
  for(i=1;i<=nInit;i++) {
    do {
      new = mutant(0);
    } while(new in unique);
    unique[new]=1; print new;
  }
  printf("%d %d %d\n",nmut+max(0,nxo-nCrossOut),nCrossOut,nInit)>"/dev/stderr";
}

function min(a,b) { return (a<=b)? a : b;}
function max(a,b) { return (a>=b)? a : b;}

function mutant(I,  m,t,type,c,n,r,u,insert,old) {
  do { #do ensures we have at least one suitable alternative to m
    m = rule[wrnd()];
    c = CPP(m);
    split(substr(m,2,length(m)-2),t,"_");
    type = t[1];
    r = (prod[m]==0)? 2+rnd(2) : 1+rnd(3);
    if(r==1) u=""; #delete
    else {
      if(type=="" && r==2) insert="+"; #else replace
      n = nrule3[c,type];
    }
  } while(r!=1 && n<2);
  if(r!=1) do {
    r = 1+rnd(n);
    u = rule3[c,type,r];
    #printf("%s `%s' %s %d %s\n",c,type,n,r,u)
  } while(prod[u]==prod[m]);
  #printf("%s `%s' %s %d ",c,type,n,r)
  old = (file>1 && I)? sprintf("%s ",pop[1+((I-1)%old_size)])  : "";
  return norepeats(sprintf("%s%s%s%s",old,m,insert,u));
}

function set_weights(I, n,t,i,s,gene,new_w,delta) { 
  #printf("set_weights(%d) ",I);
  n=split(pop[I],t);
  for(i=1;i<=n;i++) {
    s = index(t[i],">"); gene = substr(t[i],1,s); 
    if(!(gene in weight)) { 
      print "ERROR bad rule",n,i,"`"gene"'" > "/dev/stderr"; exit 2;}
    if(!(gene in new_w)) delta += weight[gene]-1;
    new_w[gene] = 1;
    #printf("%s-%d ",gene,weight[gene]);
  }
  total = 0;
  for(i=1;i<=nrule;i++) { 
    g = rule[i];
    sumweight[i] = total += (g in new_w)? 1 : weight[g];
    #printf("rule[%d]=%s sumweight[%d]=%s\n",i,rule[i],i,sumweight[i]);
  }
  #printf("total %d %d (%d)\n",total,delta,total+delta);
}

function wrnd( w,ans) { 
  w = total*rand();
  ans = binary_chop(w,1,nrule);
  #print "wrnd",nrule,total,w,ans;
  return ans;
}

function binary_chop(w,bot,top,  mid) { 
  if(bot>=top) return top;
  #if(bot>=top) {printf("binary_chop(%s,%s,%s)\n",w,bot,top); return top;}
  mid = int((bot+top)/2);
  #printf("binary_chop(%s,%s,%s) %s %s\n",w,bot,top, mid,sumweight[mid]);
  if(w<=sumweight[mid]) return binary_chop(w,bot,mid);
  return binary_chop(w,mid+1,top);
}

function CPP(rule,  n,t) { #only used if in special
  n = split(rule,t,"_");
  #printf("%d `%s' `%s' %s\n",
  #	 n,t[2],t[n],
  #	 substr(rule,2+length(t[1]),length(rule)-2-length(t[1])-length(t[n])));
  return substr(rule,2+length(t[1]),length(rule)-2-length(t[1])-length(t[n]));
}

function cross(mum,dad) { 
  #claire removes 50% at random but perhaps we need to actively grow chromosome
  return norepeats(sprintf("%s %s",pop[mum],pop[dad]));
}

function norepeats(child,  n,t,i,F,FI,first,text) { 
  n = split(child,t);
  split_genes(n,t,F,FI);
  for(i=1;i<=n;i++) remove_duplicate(t,i,F,FI);
  first=1;
  #text = sprintf("C(%d,%d) ",mum,dad);
  for(i=1;i<=n;i++) if(t[i]) {
    if(first) first=0; else text = sprintf("%s ",text);
    text=sprintf("%s%s",text,t[i]);
  }
  if(first) { print "ERROR bad cross("mum","dad")" > "/dev/stderr"; exit 1;}
  return text;
}

function split_genes(n,genes,front,frontI,  i,c,f,s) {
  for(i in front)  delete front[i];
  for(i in frontI) delete frontI[i];
#ignore contents of rules at present
  for(i=1;i<=n;i++) {
    c = genes[i]; 
#To reduce bloat, also remove duplicate >+< instructions 
#- even though they do give different phenotypes
    s  = index(c,"><");
    if(s==0) f = c;
    else     f = substr(c,1,s);
    front[i]  = f;
    frontI[f] = i;
    #printf("%-3d %s `%s'\n",i,c,f)
  }
}

function remove_duplicate(genes,I,front,frontI, f) {
  if(!(I in front)) return;
  f = front[I];
  #printf("genes[%d]=`%s' front[%d]=`%s' frontI[%s]=`%s'\n",
  #	 I,genes[I],
  #	 I,(I in front)?   front[I] : "empty1",
  #	 f,(f in frontI)? frontI[f] : "empty2");
  if((f in frontI) && frontI[f]>I) genes[I] = "";
}

function rnd(d) { return int(d*rand()); }

