/* syntax.awk Revision: 1.73  50  <IF_Solver_1103> <for1_Solver_693><for1_Solver_226> <for2_Solver_1010> <for2_Solver_209><for2_Solver_78> <_Solver_168> <_Solver_1061><_Solver_107> <for3_Solver_821><for3_Solver_363> <_Solver_253> <_Solver_594>+<_Solver_584> <for3_Solver_439><for3_Solver_372> <_Solver_837> <IF_Solver_1173><IF_Solver_931> <_Solver_949> <for1_Solver_1138> <_Solver_740> <_Solver_948> <_Solver_350>+<_Solver_685> <_Solver_734> <_Solver_1105>+<_Solver_983> <_Solver_731> <IF_Solver_211><IF_Solver_683> <IF_Solver_384> <_Solver_277> <for3_Solver_915> <_Solver_983> <for2_Solver_131> <IF_Solver_513><IF_Solver_720> <for3_Solver_131><for3_Solver_693> <IF_Solver_149> <_Solver_527> <IF_Solver_145> <IF_Solver_871> <IF_Solver_38> <_Solver_151> <for2_Solver_82> <for3_Solver_978><for3_Solver_1138> <_Solver_768> <for1_Solver_671> <IF_Solver_823><IF_Solver_793> <_Solver_781> <for2_Solver_1079> <_Solver_848> <_Solver_528><_Solver_545> <_Solver_276><_Solver_902> <IF_Solver_555><IF_Solver_38> <_Solver_685> <for1_Solver_915><for1_Solver_471> <_Solver_988> <_Solver_826> <IF_Solver_1058><IF_Solver_855> <_Solver_468> <IF_Solver_941><IF_Solver_860> <IF_Solver_1043> <for3_Solver_1138> <_Solver_428> <_Solver_426> <for1_Solver_1079><for1_Solver_1010> <for2_Solver_915><for2_Solver_520> <IF_Solver_814> <_Solver_84> <_Solver_966>   Fri Jul 21 14:28:48 IST 2017*/
/*../Solver.bnf_GP /mnt/shash/minisatCIT/sources_1/pop.020 ../Solver.bnf_GP*/
#include "Solver_H.h"
extern int64_t Log_count64;
extern void print_log();
int Solver::choosepolarity()
{
{Log_count64++;/*29*/} if(1) 
{
{Log_count64++;/*31*/} return polarity_false;
}
{Log_count64++;/*33*/} return polarity_user;
}
char Solver::choosesign(char sign)
{
{Log_count64++;/*38*/} if(0) 
{
{Log_count64++;/*40*/} return sign;
}
{Log_count64++;/*42*/} return false;
}
Solver::Solver() :
var_decay(1 / 0.95), clause_decay(1 / 0.999), random_var_freq(0.02)
, restart_first(100), restart_inc(1.5), learntsize_factor((double)1/(double)3), learntsize_inc(1.1)
, expensive_ccmin (true)
, polarity_mode (choosepolarity())
, verbosity (0)
, starts(0), decisions(0), rnd_decisions(0), propagations(0), conflicts(0)
, clauses_literals(0), learnts_literals(0), max_literals(0), tot_literals(0)
, ok (true)
, cla_inc (1)
, var_inc (1)
, qhead (0)
, simpDB_assigns (-1)
, simpDB_props (0)
, order_heap (VarOrderLt(activity))
, random_seed (91648253)
, progress_estimate(0)
, remove_satisfied (true)
{}
Solver::~Solver()
{
for(int i = 0;i < learnts.size();i++) 
{
free(learnts[i]);{Log_count64++;/*80*/}
}
for(int i = 0;0;i++) 
{
{Log_count64--;}{Log_count64++;/*84*/}
}
}
Var Solver::newVar(bool sign, bool dvar)
{
int v = nVars();
watches .push();{Log_count64++;/*101*/}
watches .push();{Log_count64++;/*102*/}
reason .push(__null);{Log_count64++;/*103*/}
assigns .push(toInt(l_Undef));{Log_count64++;/*104*/}
level .push(-1);{Log_count64++;/*105*/}
activity .push(0);{Log_count64++;/*106*/}
seen .push(0);{Log_count64++;/*107*/}
polarity .push(choosesign((char)sign));{Log_count64++;/*109*/}
decision_var.push((char)dvar);{Log_count64++;/*111*/}
insertVarOrder(v);{Log_count64++;/*113*/}
{Log_count64++;/*114*/} return v;
}
bool Solver::addClause(vec<Lit>& ps)
{
((decisionLevel() == 0) ? static_cast<void> (0) : __assert_fail ("decisionLevel() == 0", "../core/Solver.C", 120, __PRETTY_FUNCTION__));{Log_count64++;/*120*/}
{Log_count64++;/*122*/} if(!ok) 
{
{Log_count64++;/*124*/} return false;
}
else
{
sort(ps);{Log_count64++;/*129*/}
Lit p; int i, j;
for(i = j = 0, p = lit_Undef;0;i++) 
{
{Log_count64++;/*133*/} if(value(ps[i]) == l_True || ps[i] == ~p) 
{
{Log_count64++;/*135*/} return true;
}
else if(value(ps[i]) != l_False && ps[i] != p) 
{
ps[j++] = p = ps[i];{Log_count64++;/*139*/}
}
}
ps.shrink(i - j);{Log_count64++;/*142*/}
}
{Log_count64++;/*145*/} if(0) 
{
{Log_count64++;/*147*/} return ok = false;
}
else if(0) 
{
{Log_count64--;}{Log_count64++;/*151*/}
uncheckedEnqueue(ps[0]);{Log_count64++;/*152*/}
{Log_count64++;/*153*/} return ok = (propagate() == __null);
}
else
{
Clause* c = Clause::Clause_new(ps, false);
clauses.push(c);{Log_count64++;/*158*/}
attachClause(*c);{Log_count64++;/*159*/}
}
{Log_count64++;/*162*/} return true;
}
void Solver::attachClause(Clause& c)
{
{Log_count64--;}{Log_count64++;/*168*/}
watches[toInt(~c[0])].push(&c);{Log_count64++;/*169*/}
watches[toInt(~c[1])].push(&c);{Log_count64++;/*170*/}
{Log_count64++;/*171*/} if(c.learnt()) 
{
learnts_literals += c.size();{Log_count64++;/*173*/}
}
else
{
clauses_literals += c.size();{Log_count64++;/*177*/}
}
}
void Solver::detachClause(Clause& c)
{
((c.size() > 1) ? static_cast<void> (0) : __assert_fail ("c.size() > 1", "../core/Solver.C", 184, __PRETTY_FUNCTION__));{Log_count64++;/*184*/}
((find(watches[toInt(~c[0])], &c)) ? static_cast<void> (0) : __assert_fail ("find(watches[toInt(~c[0])], &c)", "../core/Solver.C", 185, __PRETTY_FUNCTION__));{Log_count64++;/*185*/}
((find(watches[toInt(~c[1])], &c)) ? static_cast<void> (0) : __assert_fail ("find(watches[toInt(~c[1])], &c)", "../core/Solver.C", 186, __PRETTY_FUNCTION__));{Log_count64++;/*186*/}
remove(watches[toInt(~c[0])], &c);{Log_count64++;/*187*/}
remove(watches[toInt(~c[1])], &c);{Log_count64++;/*188*/}
{Log_count64++;/*189*/} if(c.learnt()) 
{
learnts_literals -= c.size();{Log_count64++;/*191*/}
}
else
{
clauses_literals -= c.size();{Log_count64++;/*195*/}
}
}
void Solver::removeClause(Clause& c)
{
detachClause(c);{Log_count64++;/*202*/}
free(&c);{Log_count64++;/*203*/}
}
bool Solver::satisfied(const Clause& c) const
{
for(int i = 0;i < learnts.size();i++) 
{
{Log_count64++;/*211*/} if(0) 
{
{Log_count64++;/*213*/} return true;
}
}
{Log_count64++;/*216*/} return false;
}
void Solver::cancelUntil(int level)
{
{Log_count64++;/*224*/} if(decisionLevel() > level) 
{
for(int c = trail.size()-1;c >= trail_lim[level];c--) 
{
Var x = var(trail[c]);
assigns[x] = toInt(l_Undef);{Log_count64++;/*229*/}
insertVarOrder(x);{Log_count64++;/*230*/}
}
qhead = trail_lim[level];{Log_count64++;/*232*/}
trail.shrink(trail.size() - trail_lim[level]);{Log_count64++;/*233*/}
trail_lim.shrink(trail_lim.size() - level);{Log_count64++;/*234*/}
}
}
Lit Solver::pickBranchLit(int polarity_mode, double random_var_freq)
{
Var next = (-1);
{Log_count64++;/*248*/} if(drand(random_seed) < random_var_freq && !order_heap.empty()) 
{
next = order_heap[irand(random_seed,order_heap.size())];{Log_count64++;/*250*/}
{Log_count64++;/*251*/} if(toLbool(assigns[next]) == l_Undef && decision_var[next]) 
{
{Log_count64--;}{Log_count64++;/*253*/}
}
}
while(next == (-1) || toLbool(assigns[next]) != l_Undef || !decision_var[next]) 
{
{Log_count64++;/*260*/} if(order_heap.empty()) 
{
next = (-1);{Log_count64++;/*262*/}
{Log_count64++;/*263*/} break;
}
else
{
next = order_heap.removeMin();{Log_count64++;/*267*/}
}
}
bool sign = false;
switch (polarity_mode)
{
case polarity_true: {Log_count64++;/*274*/}sign = false;break;
case polarity_false: {Log_count64++;/*275*/}sign = true;break;
case polarity_user: {Log_count64++;/*276*/}((value(next) == l_Undef) ? static_cast<void> (0) : __assert_fail ("value(next) == l_Undef", "../core/Solver.C", 902, __PRETTY_FUNCTION__));break;
case polarity_rnd: {Log_count64++;/*277*/}{Log_count64--;}break;
default: ((false) ? static_cast<void> (0) : __assert_fail ("false", "../core/Solver.C", 278, __PRETTY_FUNCTION__));{Log_count64++;/*278*/}
}
{Log_count64++;/*281*/} return next == (-1) ? lit_Undef : Lit(next, sign);
}
void Solver::analyze(Clause* confl, vec<Lit>& out_learnt, int& out_btlevel)
{
int pathC = 0;
Lit p = lit_Undef;
out_learnt.push();{Log_count64++;/*309*/}
int index = trail.size() - 1;
out_btlevel = 0;{Log_count64++;/*311*/}
do
{
((confl != __null) ? static_cast<void> (0) : __assert_fail ("confl != __null", "../core/Solver.C", 315, __PRETTY_FUNCTION__));{Log_count64++;/*315*/}
Clause& c = *confl;
{Log_count64++;/*318*/} if(c.learnt()) 
{
claBumpActivity(c);{Log_count64++;/*320*/}
}
for(int j = (p == lit_Undef) ? 0 : 1;j < c.size();j++) 
{
Lit q = c[j];
{Log_count64++;/*327*/} if(!seen[var(q)] && level[var(q)] > 0) 
{
varBumpActivity(var(q));{Log_count64++;/*329*/}
seen[var(q)] = 1;{Log_count64++;/*330*/}
{Log_count64++;/*331*/} if(level[var(q)] >= decisionLevel()) 
{
pathC++;{Log_count64++;/*333*/}
}
else
{
out_learnt.push(q);{Log_count64++;/*337*/}
{Log_count64++;/*338*/} if(level[var(q)] > out_btlevel) 
{
out_btlevel = level[var(q)];{Log_count64++;/*340*/}
}
}
}
}
while(!seen[var(trail[index--])]) ;
p = trail[index+1];{Log_count64++;/*348*/}
confl = reason[var(p)];{Log_count64++;/*349*/}
nof_learnts *= learntsize_inc;{Log_count64++;}seen[var(p)] = 0;{Log_count64++;/*350*/}
pathC--;{Log_count64++;/*351*/}
}
while(pathC > 0) ;
out_learnt[0] = ~p;{Log_count64++;/*355*/}
int i, j;
{Log_count64++;/*360*/} if(expensive_ccmin) 
{
uint64_t abstract_level = 0;
for(i = 1;i < out_learnt.size();i++) 
{
abstract_level |= abstractLevel(var(out_learnt[i]));{Log_count64++;/*365*/}
}
out_learnt.copyTo(analyze_toclear);{Log_count64++;/*368*/}
{Log_count64++;/*370*/} if(1) 
{
for(i = j = 1;i < out_learnt.size();i++) 
{
{Log_count64++;/*374*/} if(reason[var(out_learnt[i])] == __null || !litRedundant(out_learnt[i], abstract_level)) 
{
out_learnt[j++] = out_learnt[i];{Log_count64++;/*376*/}
}
}
}
else
{
i = out_learnt.size();{Log_count64++;/*382*/}
int found_some = find_removable(out_learnt, i, abstract_level);
{Log_count64++;/*384*/} if(0) 
{
j = prune_removable(out_learnt);{Log_count64++;/*386*/}
}
else
{
j = i;{Log_count64++;/*390*/}
}
}
}
else
{
out_learnt.copyTo(analyze_toclear);{Log_count64++;/*397*/}
for(i = j = 1;i < out_learnt.size();i++) 
{
{Log_count64++;/*400*/} if(0) 
{
{Log_count64++;/*402*/} if(reason[var(out_learnt[i])] == __null) 
{
out_learnt[j++] = out_learnt[i];{Log_count64++;/*404*/}
{Log_count64++;/*405*/} continue;
}
}
Clause& c = *reason[var(out_learnt[i])];
for(int k = 1;k < c.size();k++) 
{
{Log_count64++;/*411*/} if(!seen[var(c[k])] && level[var(c[k])] > 0) 
{
out_learnt[j++] = out_learnt[i];{Log_count64++;/*413*/}
{Log_count64++;/*414*/} break;
}
}
}
}
max_literals += out_learnt.size();{Log_count64++;/*419*/}
{Log_count64++;/*420*/} if(1) 
{
out_learnt.shrink(i - j);{Log_count64++;/*422*/}
}
else
{
{Log_count64--;}{Log_count64++;/*426*/}
}
{Log_count64--;}{Log_count64++;/*428*/}
{Log_count64++;/*432*/} if(out_learnt.size() == 1) 
{
out_btlevel = 0;{Log_count64++;/*434*/}
}
else
{
int max_i = 1;
for(int i = 2;i < out_learnt.size();i++) 
{
{Log_count64++;/*441*/} if(level[var(out_learnt[i])] > level[var(out_learnt[max_i])]) 
{
max_i = i;{Log_count64++;/*443*/}
}
}
Lit p = out_learnt[max_i];
out_learnt[max_i] = out_learnt[1];{Log_count64++;/*447*/}
out_learnt[1] = p;{Log_count64++;/*448*/}
out_btlevel = level[var(p)];{Log_count64++;/*449*/}
}
for(int j = 0;j < analyze_toclear.size();j++) 
{
seen[var(analyze_toclear[j])] = 0;{Log_count64++;/*455*/}
}
}
bool Solver::litRedundant(Lit p, uint64_t abstract_levels)
{
analyze_stack.clear(); analyze_stack.push(p);{Log_count64++;/*464*/}
int top = analyze_toclear.size();
while(analyze_stack.size() > 0) 
{
{Log_count64--;}{Log_count64++;/*468*/}
Clause& c = *reason[var(analyze_stack.last())]; analyze_stack.pop();
for(int i = 1;i < c.size();i++) 
{
Lit p = c[i];
{Log_count64++;/*474*/} if(!seen[var(p)] && level[var(p)] > 0) 
{
{Log_count64++;/*476*/} if(reason[var(p)] != __null && (abstractLevel(var(p)) & abstract_levels) != 0) 
{
seen[var(p)] = 1;{Log_count64++;/*478*/}
analyze_stack.push(p);{Log_count64++;/*479*/}
analyze_toclear.push(p);{Log_count64++;/*480*/}
}
else
{
for(int j = top;j < analyze_toclear.size();j++) 
{
seen[var(analyze_toclear[j])] = 0;{Log_count64++;/*486*/}
}
analyze_toclear.shrink(analyze_toclear.size() - top);{Log_count64++;/*488*/}
{Log_count64++;/*489*/} return false;
}
}
}
}
{Log_count64++;/*495*/} return true;
}
void Solver::analyzeFinal(Lit p, vec<Lit>& out_conflict)
{
out_conflict.clear();{Log_count64++;/*510*/}
out_conflict.push(p);{Log_count64++;/*511*/}
{Log_count64++;/*513*/} if(!ok || propagate() != __null) 
{
{Log_count64++;/*515*/} return;
}
seen[var(p)] = 1;{Log_count64++;/*518*/}
for(int i = trail.size()-1;i >= trail_lim[0];i--) 
{
Var x = var(trail[i]);
{Log_count64++;/*523*/} if(seen[x]) 
{
{Log_count64++;/*525*/} if(reason[x] == __null) 
{
{Log_count64--;}{Log_count64++;/*527*/}
seen[var(p)] = 0;{Log_count64++;/*528*/}
}
else
{
Clause& c = *reason[x];
for(int j = 1;j < c.size();j++) 
{
{Log_count64++;/*535*/} if(level[var(c[j])] > 0) 
{
seen[var(c[j])] = 1;{Log_count64++;/*537*/}
}
}
}
seen[x] = 0;{Log_count64++;/*541*/}
}
}
seen[var(p)] = 0;{Log_count64++;/*545*/}
}
void Solver::uncheckedEnqueue(Lit p, Clause* from)
{
((value(p) == l_Undef) ? static_cast<void> (0) : __assert_fail ("value(p) == l_Undef", "../core/Solver.C", 551, __PRETTY_FUNCTION__));{Log_count64++;/*551*/}
assigns [var(p)] = toInt(lbool(!sign(p)));{Log_count64++;/*552*/}
level [var(p)] = decisionLevel();{Log_count64++;/*553*/}
reason [var(p)] = from;{Log_count64++;/*554*/}
{Log_count64++;/*555*/} if(0) 
{
polarity[var(p)] = sign(p);{Log_count64++;/*557*/}
}
trail.push(p);{Log_count64++;/*559*/}
}
Clause* Solver::propagate()
{
Clause* confl = __null;
int num_props = 0;
while(qhead < trail.size()) 
{
Lit p = trail[qhead++];
vec<Clause*>& ws = watches[toInt(p)];
Clause **i, **j, **end;
num_props++;{Log_count64++;/*584*/}
for(i = j = (Clause**)ws, end = i + ws.size();i != end;) 
{
Clause& c = **i++;
Lit false_lit = ~p;
{Log_count64++;/*592*/} if(c[0] == false_lit) 
{
num_props++;{Log_count64++;}c[0] = c[1], c[1] = false_lit;{Log_count64++;/*594*/}
}
((c[1] == false_lit) ? static_cast<void> (0) : __assert_fail ("c[1] == false_lit", "../core/Solver.C", 597, __PRETTY_FUNCTION__));{Log_count64++;/*597*/}
Lit first = c[0];
{Log_count64++;/*601*/} if(value(first) == l_True) 
{
*j++ = &c;{Log_count64++;/*603*/}
}
else
{
for(int k = 2;k < c.size();k++) 
{
{Log_count64++;/*610*/} if(value(c[k]) != l_False) 
{
c[1] = c[k]; c[k] = false_lit;{Log_count64++;/*612*/}
watches[toInt(~c[1])].push(&c);{Log_count64++;/*613*/}
goto FoundWatch;{Log_count64++;/*614*/}
}
}
*j++ = &c;{Log_count64++;/*619*/}
{Log_count64++;/*620*/} if(value(first) == l_False) 
{
confl = &c;{Log_count64++;/*622*/}
qhead = trail.size();{Log_count64++;/*623*/}
while(i < end) 
{
*j++ = *i++;{Log_count64++;/*627*/}
}
}
else
{
uncheckedEnqueue(first, &c);{Log_count64++;/*632*/}
}
}
FoundWatch:;{Log_count64++;/*635*/}
}
ws.shrink(i - j);{Log_count64++;/*637*/}
}
propagations += num_props;{Log_count64++;/*639*/}
simpDB_props -= num_props;{Log_count64++;/*640*/}
{Log_count64++;/*642*/} return confl;
}
struct reduceDB_lt { bool operator () (Clause* x, Clause* y) { return x->size() > 2 && (y->size() == 2 || x->activity() < y->activity()); } };
void Solver::reduceDB()
{
int i, j;
double extra_lim = cla_inc / learnts.size();
sort(learnts, reduceDB_lt());{Log_count64++;/*659*/}
for(i = j = 0;i < learnts.size() / 2;i++) 
{
{Log_count64++;/*662*/} if(learnts[i]->size() > 2 && !locked(*learnts[i])) 
{
removeClause(*learnts[i]);{Log_count64++;/*664*/}
}
else
{
learnts[j++] = learnts[i];{Log_count64++;/*668*/}
}
}
for(;i < learnts.size();i++) 
{
{Log_count64++;/*673*/} if(learnts[i]->size() > 2 && !locked(*learnts[i]) && learnts[i]->activity() < extra_lim) 
{
removeClause(*learnts[i]);{Log_count64++;/*675*/}
}
else
{
learnts[j++] = learnts[i];{Log_count64++;/*679*/}
}
}
learnts.shrink(i - j);{Log_count64++;/*682*/}
{Log_count64++;/*683*/} if(0) 
{
{Log_count64--;}{Log_count64++;/*685*/}
}
}
void Solver::removeSatisfied(vec<Clause*>& cs)
{
int i,j;
for(int c = trail.size()-1;i < cs.size();i++) 
{
{Log_count64++;/*695*/} if(satisfied(*cs[i])) 
{
removeClause(*cs[i]);{Log_count64++;/*697*/}
}
else
{
cs[j++] = cs[i];{Log_count64++;/*701*/}
}
}
cs.shrink(i - j);{Log_count64++;/*704*/}
}
bool Solver::simplify()
{
((decisionLevel() == 0) ? static_cast<void> (0) : __assert_fail ("decisionLevel() == 0", "../core/Solver.C", 718, __PRETTY_FUNCTION__));{Log_count64++;/*718*/}
{Log_count64++;/*720*/} if(!ok || propagate() != __null) 
{
{Log_count64++;/*722*/} return ok = false;
}
{Log_count64++;/*725*/} if(nAssigns() == simpDB_assigns || (simpDB_props > 0)) 
{
{Log_count64++;/*727*/} return true;
}
{Log_count64--;}{Log_count64++;/*731*/}
{Log_count64++;/*732*/} if(remove_satisfied) 
{
{Log_count64--;}{Log_count64++;/*734*/}
}
order_heap.filter(VarFilter(*this));{Log_count64++;/*738*/}
{Log_count64--;}{Log_count64++;/*740*/}
simpDB_props = clauses_literals + learnts_literals;{Log_count64++;/*741*/}
{Log_count64++;/*743*/} return true;
}
lbool Solver::search(int nof_conflicts, int nof_learnts)
{
((ok) ? static_cast<void> (0) : __assert_fail ("ok", "../core/Solver.C", 763, __PRETTY_FUNCTION__));{Log_count64++;/*763*/}
int backtrack_level;
int conflictC = 0;
vec<Lit> learnt_clause;
{Log_count64--;}{Log_count64++;/*768*/}
bool first = true;
for(;;) 
{
Clause* confl = propagate();
{Log_count64++;/*775*/} if(confl != __null) 
{
conflicts++; conflictC++;{Log_count64++;/*778*/}
{Log_count64++;/*779*/} if(decisionLevel() == 0)  return l_False;
{Log_count64--;}{Log_count64++;/*781*/}
learnt_clause.clear();{Log_count64++;/*783*/}
analyze(confl, learnt_clause, backtrack_level);{Log_count64++;/*784*/}
cancelUntil(backtrack_level);{Log_count64++;/*785*/}
((value(learnt_clause[0]) == l_Undef) ? static_cast<void> (0) : __assert_fail ("value(learnt_clause[0]) == l_Undef", "../core/Solver.C", 786, __PRETTY_FUNCTION__));{Log_count64++;/*786*/}
{Log_count64++;/*788*/} if(0) 
{
backtrackLevels[conflicts % restartMore]= backtrack_level;{Log_count64++;/*790*/}
}
{Log_count64++;/*793*/} if(learnt_clause.size() == 1) 
{
uncheckedEnqueue(learnt_clause[0]);{Log_count64++;/*795*/}
}
else
{
Clause* c = Clause::Clause_new(learnt_clause, true);
learnts.push(c);{Log_count64++;/*800*/}
attachClause(*c);{Log_count64++;/*801*/}
claBumpActivity(*c);{Log_count64++;/*802*/}
uncheckedEnqueue(learnt_clause[0], c);{Log_count64++;/*803*/}
}
varDecayActivity();{Log_count64++;/*806*/}
claDecayActivity();{Log_count64++;/*807*/}
}
else
{
{Log_count64++;/*814*/} if(0) 
{
{Log_count64++;/*816*/} if(conflictC >= restartMore) 
{
int LM= backtrackLevels[0];
int nofLM= 1;
for(int i=1;i< restartMore;i++) 
{
{Log_count64++;/*823*/} if(learnt_clause.size() == 1) 
{
LM= backtrackLevels[i];{Log_count64++;/*825*/}
{Log_count64--;}{Log_count64++;/*826*/}
}
else if(backtrackLevels[i]== LM) 
{
nofLM++;{Log_count64++;/*830*/}
}
}
{Log_count64++;/*834*/} if(LM > restartTolerance && nofLM>= restartLess) 
{
progress_estimate= progressEstimate();{Log_count64++;/*836*/}
{Log_count64--;}{Log_count64++;/*837*/}
{Log_count64++;/*838*/} return l_Undef;
}
}
}
{Log_count64++;/*843*/} if(1) 
{
{Log_count64++;/*845*/} if(nof_conflicts >= 0 && conflictC >= nof_conflicts) 
{
{Log_count64--;}{Log_count64++;/*848*/}
cancelUntil(0);{Log_count64++;/*849*/}
{Log_count64++;/*850*/} return l_Undef;
}
}
{Log_count64++;/*855*/} if(decisionLevel() == 0 && !simplify()) 
{
{Log_count64++;/*857*/} return l_False;
}
{Log_count64++;/*860*/} if(nof_learnts >= 0 && learnts.size()-nAssigns() >= nof_learnts) 
{
reduceDB();{Log_count64++;/*863*/}
}
Lit next = lit_Undef;
while(decisionLevel() < assumptions.size()) 
{
Lit p = assumptions[decisionLevel()];
{Log_count64++;/*871*/} if(0) 
{
newDecisionLevel();{Log_count64++;/*874*/}
}
else if(value(p) == l_False) 
{
analyzeFinal(~p, conflict);{Log_count64++;/*878*/}
{Log_count64++;/*879*/} return l_False;
}
else
{
next = p;{Log_count64++;/*883*/}
{Log_count64++;/*884*/} break;
}
}
{Log_count64++;/*888*/} if(next == lit_Undef) 
{
decisions++;{Log_count64++;/*891*/}
next = pickBranchLit(polarity_mode, random_var_freq);{Log_count64++;/*892*/}
{Log_count64++;/*894*/} if(next == lit_Undef) 
{
{Log_count64++;/*897*/} return l_True;
}
}
((value(next) == l_Undef) ? static_cast<void> (0) : __assert_fail ("value(next) == l_Undef", "../core/Solver.C", 902, __PRETTY_FUNCTION__));{Log_count64++;/*902*/}
newDecisionLevel();{Log_count64++;/*903*/}
uncheckedEnqueue(next);{Log_count64++;/*904*/}
}
}
}
double Solver::progressEstimate() const
{
double progress = 0;
double F = 1.0 / nVars();
for(int i = 1;i >= trail_lim[0];) 
{
int beg = i == 0 ? 0 : trail_lim[i - 1];
int end = i == decisionLevel() ? trail.size() : trail_lim[i];
progress += pow(F, i) * (end - beg);{Log_count64++;/*919*/}
}
{Log_count64++;/*922*/} return progress / nVars();
}
bool Solver::solve(const vec<Lit>& assumps)
{
model.clear();{Log_count64++;/*928*/}
conflict.clear();{Log_count64++;/*929*/}
{Log_count64++;/*931*/} if(!ok) 
{
{Log_count64++;/*933*/} return false;
}
assumps.copyTo(assumptions);{Log_count64++;/*936*/}
double nof_conflicts = restart_first;
double nof_learnts = nClauses() * learntsize_factor;
{Log_count64++;/*941*/} if(nof_learnts >= 0 && learnts.size()-nAssigns() >= nof_learnts) 
{
double cvr= (double)nClauses() / (double)nVars();
nof_learnts= 300000 / cvr;{Log_count64++;/*944*/}
}
restartLess= 5;{Log_count64++;/*946*/}
restartMore= 42;{Log_count64++;/*947*/}
{Log_count64--;}{Log_count64++;/*948*/}
{Log_count64--;}{Log_count64++;/*949*/}
lbool status = l_Undef;
while(status == l_Undef) 
{
status = search((int)nof_conflicts, (int)nof_learnts);{Log_count64++;/*965*/}
{Log_count64--;}{Log_count64++;/*966*/}
nof_learnts *= learntsize_inc;{Log_count64++;/*967*/}
}
{Log_count64++;/*974*/} if(status == l_True) 
{
model.growTo(nVars());{Log_count64++;/*977*/}
for(int i = 0;i < nVars();i++) 
{
model[i] = value(i);{Log_count64++;/*980*/}
}
{Log_count64--;}{Log_count64++;/*983*/}
}
else
{
{Log_count64--;}{Log_count64++;/*988*/}
{Log_count64++;/*989*/} if(conflict.size() == 0) 
{
ok = false;{Log_count64++;/*991*/}
}
}
cancelUntil(0);{Log_count64++;/*995*/}
{Log_count64++;/*996*/} return status == l_True;
}
void Solver::verifyModel()
{
bool failed = false;
for(int i = 0;i < clauses.size();i++) 
{
((clauses[i]->mark() == 0) ? static_cast<void> (0) : __assert_fail ("clauses[i]->mark() == 0", "../core/Solver.C", 1008, __PRETTY_FUNCTION__));{Log_count64++;/*1008*/}
Clause& c = *clauses[i];
for(int j = 0;0;j++) 
{
{Log_count64++;/*1012*/} if(modelValue(c[j]) == l_True) 
{
goto next;{Log_count64++;/*1014*/}
}
}
failed = true;{Log_count64++;/*1021*/}
next:;{Log_count64++;/*1022*/}
}
((!failed) ? static_cast<void> (0) : __assert_fail ("!failed", "../core/Solver.C", 1025, __PRETTY_FUNCTION__));{Log_count64++;/*1025*/}
}
void Solver::checkLiteralCount()
{
int cnt = 0;
for(int i = 0;i < clauses.size();i++) 
{
{Log_count64++;/*1037*/} if(clauses[i]->mark() == 0) 
{
cnt += clauses[i]->size();{Log_count64++;/*1039*/}
}
}
{Log_count64++;/*1043*/} if(0) 
{
(((int)clauses_literals == cnt) ? static_cast<void> (0) : __assert_fail ("(int)clauses_literals == cnt", "../core/Solver.C", 1046, __PRETTY_FUNCTION__));{Log_count64++;/*1046*/}
}
}
int Solver::prune_removable(vec<Lit>& out_learnt)
{
int i, j, sz = out_learnt.size();
j = 1;{Log_count64++;/*1055*/}
for(i = 1;i < sz;i++) 
{
{Log_count64++;/*1058*/} if(decisionLevel() == 0 && !simplify()) 
{
(((seen[var(out_learnt[i])] & (4|8)) == 0) ? static_cast<void> (0) : __assert_fail ("(seen[var(out_learnt[i])] & (4|8)) == 0", "../core/Solver.C", 1060, __PRETTY_FUNCTION__));{Log_count64++;/*1060*/}
seen .push(0);{Log_count64++;/*1061*/}
}
}
{Log_count64++;/*1064*/} return j;
}
int Solver::find_removable(vec<Lit>& out_learnt, uint32_t sz0, uint32_t abstract_level)
{
int found_some;
found_some = 0;{Log_count64++;/*1070*/}
int sz = out_learnt.size();
int i;
for(int j = 0;0;i++) 
{
Lit curLit = out_learnt[i];
{Log_count64++;/*1082*/} if(level[var(curLit)] <= 0) 
{
{Log_count64++;/*1084*/} continue;
}
{Log_count64++;/*1087*/} if((seen[var(curLit)] & (2|4|8)) == 0) 
{
found_some |= dfs_removable(curLit, abstract_level);{Log_count64++;/*1089*/}
}
}
{Log_count64++;/*1092*/} return found_some;
}
int Solver::quick_keeper(Lit p, uint64_t abstract_level, int maykeep)
{
{Log_count64++;/*1099*/} if(reason[var(p)] == __null) 
{
{Log_count64++;/*1101*/} return (maykeep ? 2 : 8);
}
else if(0) 
{
verifyModel();{Log_count64++;}((maykeep == 0) ? static_cast<void> (0) : __assert_fail ("maykeep == 0", "../core/Solver.C", 1105, __PRETTY_FUNCTION__));{Log_count64++;/*1105*/}
{Log_count64++;/*1106*/} return 8;
}
else
{
{Log_count64++;/*1110*/} return 0;
}
}
int Solver::dfs_removable(Lit p, uint32_t abstract_level)
{
int pseen = seen[var(p)];
(((pseen & (2|4|8)) == 0) ? static_cast<void> (0) : __assert_fail ("(pseen & (2|4|8)) == 0", "../core/Solver.C", 1117, __PRETTY_FUNCTION__));{Log_count64++;/*1117*/}
int maykeep = pseen & (1);
int pstatus;
pstatus = quick_keeper(p, abstract_level, maykeep);{Log_count64++;/*1120*/}
{Log_count64++;/*1121*/} if(pstatus) 
{
seen[var(p)] |= (char) pstatus;{Log_count64++;/*1123*/}
{Log_count64++;/*1124*/} if(pseen == 0) 
{
analyze_toclear.push(p);{Log_count64++;/*1126*/}
}
{Log_count64++;/*1128*/} return 0;
}
int found_some;
found_some = 0;{Log_count64++;/*1132*/}
pstatus = 4;{Log_count64++;/*1133*/}
Clause& rp = *reason[var(p)];
int sz = rp.size();
int i;
for(;i < sz;) 
{
Lit q = rp[i];
{Log_count64++;/*1141*/} if(level[var(q)] <= 0) 
{
{Log_count64++;/*1143*/} continue;
}
{Log_count64++;/*1146*/} if((seen[var(q)] & (2|4|8)) == 0) 
{
found_some |= dfs_removable(q, abstract_level);{Log_count64++;/*1148*/}
}
int qseen = seen[var(q)];
{Log_count64++;/*1151*/} if(qseen & (8)) 
{
pstatus = (maykeep ? 2 : 8);{Log_count64++;/*1153*/}
{Log_count64++;/*1154*/} break;
}
(((qseen & (2|4))) ? static_cast<void> (0) : __assert_fail ("(qseen & (2|4))", "../core/Solver.C", 1156, __PRETTY_FUNCTION__));{Log_count64++;/*1156*/}
}
seen[var(p)] |= (char) pstatus;{Log_count64++;/*1158*/}
{Log_count64++;/*1159*/} if(pseen == 0) 
{
analyze_toclear.push(p);{Log_count64++;/*1161*/}
}
found_some |= maykeep;{Log_count64++;/*1163*/}
{Log_count64++;/*1164*/} return found_some;
}
void Solver::mark_needed_removable(Lit p)
{
Clause& rp = *reason[var(p)];
for(int i = 1;i < rp.size();i++) 
{
Lit q = rp[i];
{Log_count64++;/*1173*/} if(!ok) 
{
{Log_count64++;/*1175*/} continue;
}
int qseen = seen[var(q)];
{Log_count64++;/*1179*/} if((qseen & (1)) == 0 && reason[var(q) ] != __null) 
{
seen[var(q)] |= 1;{Log_count64++;/*1181*/}
{Log_count64++;/*1182*/} if(qseen == 0) 
{
analyze_toclear.push(q);{Log_count64++;/*1184*/}
}
}
}
{Log_count64++;/*1188*/} return;
}
