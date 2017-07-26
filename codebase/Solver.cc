#include "Solver_H.h"
extern int64_t Log_count64;
extern void print_log();
using namespace Minisat;
static const char* _cat = "CORE";
static DoubleOption opt_var_decay (_cat, "var-decay", "The variable activity decay factor", 0.95, DoubleRange(0, false, 1, false));
static DoubleOption opt_clause_decay (_cat, "cla-decay", "The clause activity decay factor", 0.999, DoubleRange(0, false, 1, false));
static DoubleOption opt_random_var_freq (_cat, "rnd-freq", "The frequency with which the decision heuristic tries to choose a random variable", 0, DoubleRange(0, true, 1, true));
static DoubleOption opt_random_seed (_cat, "rnd-seed", "Used by the random variable selection", 91648253, DoubleRange(0, false,
(__builtin_huge_val())
, false));
static IntOption opt_ccmin_mode (_cat, "ccmin-mode", "Controls conflict clause minimization (0=none, 1=basic, 2=deep)", 2, IntRange(0, 2));
static IntOption opt_phase_saving (_cat, "phase-saving", "Controls the level of phase saving (0=none, 1=limited, 2=full)", 2, IntRange(0, 2));
static BoolOption opt_rnd_init_act (_cat, "rnd-init", "Randomize the initial activity", false);
static BoolOption opt_luby_restart (_cat, "luby", "Use the Luby restart sequence", true);
static IntOption opt_restart_first (_cat, "rfirst", "The base restart interval", 100, IntRange(1, (2147483647)));
static DoubleOption opt_restart_inc (_cat, "rinc", "Restart interval increase factor", 2, DoubleRange(1, false, (__builtin_huge_val()) , false));
static DoubleOption opt_garbage_frac (_cat, "gc-frac", "The fraction of wasted memory allowed before a garbage collection is triggered", 0.20, DoubleRange(0, false, (__builtin_huge_val()) , false));
Solver::Solver() :
verbosity (0)
, var_decay (opt_var_decay)
, clause_decay (opt_clause_decay)
, random_var_freq (opt_random_var_freq)
, random_seed (opt_random_seed)
, luby_restart (opt_luby_restart)
, ccmin_mode (opt_ccmin_mode)
, phase_saving (opt_phase_saving)
, rnd_pol (false)
, rnd_init_act (opt_rnd_init_act)
, garbage_frac (opt_garbage_frac)
, restart_first (opt_restart_first)
, restart_inc (opt_restart_inc)
, learntsize_factor((double)1/(double)3), learntsize_inc(1.1)
, learntsize_adjust_start_confl (100)
, learntsize_adjust_inc (1.5)
, solves(0), starts(0), decisions(0), rnd_decisions(0), propagations(0), conflicts(0)
, dec_vars(0), clauses_literals(0), learnts_literals(0), max_literals(0), tot_literals(0)
, ok (true)
, cla_inc (1)
, var_inc (1)
, watches (WatcherDeleted(ca))
, qhead (0)
, simpDB_assigns (-1)
, simpDB_props (0)
, order_heap (VarOrderLt(activity))
, progress_estimate (0)
, remove_satisfied (true)
, conflict_budget (-1)
, propagation_budget (-1)
, asynch_interrupt (false)
{
}
Solver::~Solver()
{
}
Var Solver::newVar(bool sign, bool dvar)
{
int v = nVars();
watches .init(mkLit(v, false));{Log_count64++;/*58*/}
watches .init(mkLit(v, true ));{Log_count64++;/*59*/}
assigns .push((lbool((uint8_t)2)));{Log_count64++;/*60*/}
vardata .push(mkVarData(CRef_Undef, 0));{Log_count64++;/*61*/}
activity .push(rnd_init_act ? drand(random_seed) * 0.00001 : 0);{Log_count64++;/*62*/}
seen .push(0);{Log_count64++;/*63*/}
polarity .push(sign);{Log_count64++;/*64*/}
decision .push();{Log_count64++;/*65*/}
trail .capacity(v+1);{Log_count64++;/*66*/}
setDecisionVar(v, dvar);{Log_count64++;/*67*/}
{Log_count64++;/*68*/} return v;
}
bool Solver::addClause_(vec<Lit>& ps)
{
(( decisionLevel() == 0) ? static_cast<void> (0) : __assert_fail ( "decisionLevel() == 0" , "/home/justyna/Documents/minisatgirelease/sources/core/Solver.cc", 73, __PRETTY_FUNCTION__)) ;{Log_count64++;/*73*/}
{Log_count64++;/*74*/} if(!ok) 
{
{Log_count64++;/*76*/} return false;
}
sort(ps);{Log_count64++;/*78*/}
Lit p; int i, j;
for(i = j = 0, p = lit_Undef;i < ps.size();i++) 
{
{Log_count64++;/*82*/} if(value(ps[i]) == (lbool((uint8_t)0)) || ps[i] == ~p) 
{
{Log_count64++;/*84*/} return true;
}
else if(value(ps[i]) != (lbool((uint8_t)1)) && ps[i] != p) 
{
ps[j++] = p = ps[i];{Log_count64++;/*88*/}
}
}
ps.shrink(i - j);{Log_count64++;/*91*/}
{Log_count64++;/*92*/} if(ps.size() == 0) 
{
{Log_count64++;/*94*/} return ok = false;
}
else if(ps.size() == 1) 
{
uncheckedEnqueue(ps[0]);{Log_count64++;/*98*/}
{Log_count64++;/*99*/} return ok = (propagate() == CRef_Undef);
}
else
{
CRef cr = ca.alloc(ps, false);
clauses.push(cr);{Log_count64++;/*104*/}
attachClause(cr);{Log_count64++;/*105*/}
}
{Log_count64++;/*107*/} return true;
}
void Solver::attachClause(CRef cr)
{
const Clause& c = ca[cr];
(( c.size() > 1) ? static_cast<void> (0) : __assert_fail ( "c.size() > 1" , "/home/justyna/Documents/minisatgirelease/sources/core/Solver.cc", 113, __PRETTY_FUNCTION__)) ;{Log_count64++;/*113*/}
watches[~c[0]].push(Watcher(cr, c[1]));{Log_count64++;/*114*/}
watches[~c[1]].push(Watcher(cr, c[0]));{Log_count64++;/*115*/}
{Log_count64++;/*116*/} if(c.learnt())  learnts_literals += c.size();
else {clauses_literals += c.size(); {Log_count64++;/*117*/}};
}
void Solver::detachClause(CRef cr, bool strict)
{
const Clause& c = ca[cr];
(( c.size() > 1) ? static_cast<void> (0) : __assert_fail ( "c.size() > 1" , "/home/justyna/Documents/minisatgirelease/sources/core/Solver.cc", 123, __PRETTY_FUNCTION__)) ;{Log_count64++;/*123*/}
{Log_count64++;/*124*/} if(strict) 
{
remove(watches[~c[0]], Watcher(cr, c[1]));{Log_count64++;/*126*/}
remove(watches[~c[1]], Watcher(cr, c[0]));{Log_count64++;/*127*/}
}
else
{
watches.smudge(~c[0]);{Log_count64++;/*131*/}
watches.smudge(~c[1]);{Log_count64++;/*132*/}
}
{Log_count64++;/*134*/} if(c.learnt()) 
{
learnts_literals -= c.size();{Log_count64++;/*136*/}
}
else
{
clauses_literals -= c.size();{Log_count64++;/*140*/}
}
}
void Solver::removeClause(CRef cr)
{
Clause& c = ca[cr];
detachClause(cr);{Log_count64++;/*146*/}
{Log_count64++;/*147*/} if(locked(c)) 
{
vardata[var(c[0])].reason = CRef_Undef;{Log_count64++;/*149*/}
}
c.mark(1);{Log_count64++;/*151*/}
ca.free(cr);{Log_count64++;/*152*/}
}
bool Solver::satisfied(const Clause& c) const
{
for(int i = 0;i < c.size();i++) 
{
{Log_count64++;/*158*/} if(value(c[i]) == (lbool((uint8_t)0))) 
{
{Log_count64++;/*160*/} return true;
}
}
{Log_count64++;/*163*/} return false;
}
void Solver::cancelUntil(int level)
{
{Log_count64++;/*167*/} if(decisionLevel() > level) 
{
for(int c = trail.size()-1;c >= trail_lim[level];c--) 
{
Var x = var(trail[c]);
assigns [x] = (lbool((uint8_t)2));{Log_count64++;/*172*/}
{Log_count64++;/*173*/} if(phase_saving > 1 || (phase_saving == 1) && c > trail_lim.last()) 
{
polarity[x] = sign(trail[c]);{Log_count64++;/*175*/}
}
insertVarOrder(x);{Log_count64++;/*177*/}
}
qhead = trail_lim[level];{Log_count64++;/*179*/}
trail.shrink(trail.size() - trail_lim[level]);{Log_count64++;/*180*/}
trail_lim.shrink(trail_lim.size() - level);{Log_count64++;/*181*/}
}
}
Lit Solver::pickBranchLit()
{
Var next = (-1);
{Log_count64++;/*187*/} if(drand(random_seed) < random_var_freq && !order_heap.empty()) 
{
next = order_heap[irand(random_seed,order_heap.size())];{Log_count64++;/*189*/}
{Log_count64++;/*190*/} if(value(next) == (lbool((uint8_t)2)) && decision[next]) 
{
rnd_decisions++;{Log_count64++;/*192*/}
}
}
while(next == (-1) || value(next) != (lbool((uint8_t)2)) || !decision[next]) 
{
{Log_count64++;/*197*/} if(order_heap.empty()) 
{
next = (-1);{Log_count64++;/*199*/}
{Log_count64++;/*200*/} break;
}
else
{
next = order_heap.removeMin();{Log_count64++;/*204*/}
}
}
{Log_count64++;/*207*/} return next == (-1) ? lit_Undef : mkLit(next, rnd_pol ? drand(random_seed) < 0.5 : polarity[next]);
}
void Solver::analyze(CRef confl, vec<Lit>& out_learnt, int& out_btlevel)
{
int pathC = 0;
Lit p = lit_Undef;
out_learnt.push();{Log_count64++;/*213*/}
int index = trail.size() - 1;
do
{
(( confl != CRef_Undef) ? static_cast<void> (0) : __assert_fail ( "confl != CRef_Undef" , "/home/justyna/Documents/minisatgirelease/sources/core/Solver.cc", 218, __PRETTY_FUNCTION__)) ;{Log_count64++;/*218*/}
Clause& c = ca[confl];
{Log_count64++;/*220*/} if(c.learnt()) 
{
claBumpActivity(c);{Log_count64++;/*222*/}
}
for(int j = (p == lit_Undef) ? 0 : 1;j < c.size();j++) 
{
Lit q = c[j];
{Log_count64++;/*227*/} if(!seen[var(q)] && level(var(q)) > 0) 
{
varBumpActivity(var(q));{Log_count64++;/*229*/}
seen[var(q)] = 1;{Log_count64++;/*230*/}
{Log_count64++;/*231*/} if(level(var(q)) >= decisionLevel()) 
{
pathC++;{Log_count64++;/*233*/}
}
else
{
out_learnt.push(q);{Log_count64++;/*237*/}
}
}
}
while(!seen[var(trail[index--])]) ;
p = trail[index+1];{Log_count64++;/*242*/}
confl = reason(var(p));{Log_count64++;/*243*/}
seen[var(p)] = 0;{Log_count64++;/*244*/}
pathC--;{Log_count64++;/*245*/}
}
while(pathC > 0) ;
out_learnt[0] = ~p;{Log_count64++;/*248*/}
int i, j;
out_learnt.copyTo(analyze_toclear);{Log_count64++;/*250*/}
{Log_count64++;/*251*/} if(ccmin_mode == 2) 
{
uint32_t abstract_level = 0;
for(i = 1;i < out_learnt.size();i++) 
{
abstract_level |= abstractLevel(var(out_learnt[i]));{Log_count64++;/*256*/}
}
for(i = j = 1;i < out_learnt.size();i++) 
{
{Log_count64++;/*260*/} if(reason(var(out_learnt[i])) == CRef_Undef || !litRedundant(out_learnt[i], abstract_level)) 
{
out_learnt[j++] = out_learnt[i];{Log_count64++;/*262*/}
}
}
}
else if(ccmin_mode == 1) 
{
for(i = j = 1;i < out_learnt.size();i++) 
{
Var x = var(out_learnt[i]);
{Log_count64++;/*271*/} if(reason(x) == CRef_Undef) 
{
out_learnt[j++] = out_learnt[i];{Log_count64++;/*273*/}
}
else
{
Clause& c = ca[reason(var(out_learnt[i]))];
for(int k = 1;k < c.size();k++) 
{
{Log_count64++;/*280*/} if(!seen[var(c[k])] && level(var(c[k])) > 0) 
{
out_learnt[j++] = out_learnt[i];{Log_count64++;/*282*/}
{Log_count64++;/*283*/} break;
}
}
}
}
}
else
{
i = j = out_learnt.size();{Log_count64++;/*291*/}
}
max_literals += out_learnt.size();{Log_count64++;/*293*/}
out_learnt.shrink(i - j);{Log_count64++;/*294*/}
tot_literals += out_learnt.size();{Log_count64++;/*295*/}
{Log_count64++;/*296*/} if(out_learnt.size() == 1) 
{
out_btlevel = 0;{Log_count64++;/*298*/}
}
else
{
int max_i = 1;
for(int i = 2;i < out_learnt.size();i++) 
{
{Log_count64++;/*305*/} if(level(var(out_learnt[i])) > level(var(out_learnt[max_i]))) 
{
max_i = i;{Log_count64++;/*307*/}
}
}
Lit p = out_learnt[max_i];
out_learnt[max_i] = out_learnt[1];{Log_count64++;/*311*/}
out_learnt[1] = p;{Log_count64++;/*312*/}
out_btlevel = level(var(p));{Log_count64++;/*313*/}
}
for(int j = 0;j < analyze_toclear.size();j++) 
{
seen[var(analyze_toclear[j])] = 0;{Log_count64++;/*317*/}
}
}
bool Solver::litRedundant(Lit p, uint32_t abstract_levels)
{
analyze_stack.clear(); analyze_stack.push(p);{Log_count64++;/*322*/}
int top = analyze_toclear.size();
while(analyze_stack.size() > 0) 
{
(( reason(var(analyze_stack.last())) != CRef_Undef) ? static_cast<void> (0) : __assert_fail ( "reason(var(analyze_stack.last())) != CRef_Undef" , "/home/justyna/Documents/minisatgirelease/sources/core/Solver.cc", 327, __PRETTY_FUNCTION__)) ;{Log_count64++;/*327*/}
Clause& c = ca[reason(var(analyze_stack.last()))]; analyze_stack.pop();
for(int i = 1;i < c.size();i++) 
{
Lit p = c[i];
{Log_count64++;/*332*/} if(!seen[var(p)] && level(var(p)) > 0) 
{
{Log_count64++;/*334*/} if(reason(var(p)) != CRef_Undef && (abstractLevel(var(p)) & abstract_levels) != 0) 
{
seen[var(p)] = 1;{Log_count64++;/*336*/}
analyze_stack.push(p);{Log_count64++;/*337*/}
analyze_toclear.push(p);{Log_count64++;/*338*/}
}
else
{
for(int j = top;j < analyze_toclear.size();j++) 
{
seen[var(analyze_toclear[j])] = 0;{Log_count64++;/*344*/}
}
analyze_toclear.shrink(analyze_toclear.size() - top);{Log_count64++;/*346*/}
{Log_count64++;/*347*/} return false;
}
}
}
}
{Log_count64++;/*352*/} return true;
}
void Solver::analyzeFinal(Lit p, vec<Lit>& out_conflict)
{
out_conflict.clear();{Log_count64++;/*356*/}
out_conflict.push(p);{Log_count64++;/*357*/}
{Log_count64++;/*358*/} if(decisionLevel() == 0) 
{
{Log_count64++;/*360*/} return;
}
seen[var(p)] = 1;{Log_count64++;/*362*/}
for(int i = trail.size()-1;i >= trail_lim[0];i--) 
{
Var x = var(trail[i]);
{Log_count64++;/*366*/} if(seen[x]) 
{
{Log_count64++;/*368*/} if(reason(x) == CRef_Undef) 
{
(( level(x) > 0) ? static_cast<void> (0) : __assert_fail ( "level(x) > 0" , "/home/justyna/Documents/minisatgirelease/sources/core/Solver.cc", 371, __PRETTY_FUNCTION__)) ;{Log_count64++;/*371*/}
out_conflict.push(~trail[i]);{Log_count64++;/*372*/}
}
else
{
Clause& c = ca[reason(x)];
for(int j = 1;j < c.size();j++) 
{
{Log_count64++;/*379*/} if(level(var(c[j])) > 0) 
{
seen[var(c[j])] = 1;{Log_count64++;/*381*/}
}
}
}
seen[x] = 0;{Log_count64++;/*385*/}
}
}
seen[var(p)] = 0;{Log_count64++;/*388*/}
}
void Solver::uncheckedEnqueue(Lit p, CRef from)
{
(( value(p) == (lbool((uint8_t)2))) ? static_cast<void> (0) : __assert_fail ( "value(p) == l_Undef" , "/home/justyna/Documents/minisatgirelease/sources/core/Solver.cc", 393, __PRETTY_FUNCTION__)) ;{Log_count64++;/*393*/}
assigns[var(p)] = lbool(!sign(p));{Log_count64++;/*394*/}
vardata[var(p)] = mkVarData(from, decisionLevel());{Log_count64++;/*395*/}
trail.push_(p);{Log_count64++;/*396*/}
}
CRef Solver::propagate()
{
CRef confl = CRef_Undef;
int num_props = 0;
watches.cleanAll();{Log_count64++;/*402*/}
while(qhead < trail.size()) 
{
Lit p = trail[qhead++];
vec<Watcher>& ws = watches[p];
Watcher *i, *j, *end;
num_props++;{Log_count64++;/*408*/}
for(i = j = (Watcher*)ws, end = i + ws.size();i != end;) 
{
Lit blocker = i->blocker;
{Log_count64++;/*412*/} if(value(blocker) == (lbool((uint8_t)0))) 
{
*j++ = *i++; {Log_count64++;/*414*/} continue;
}
CRef cr = i->cref;
Clause& c = ca[cr];
Lit false_lit = ~p;
{Log_count64++;/*419*/} if(c[0] == false_lit) 
{
c[0] = c[1], c[1] = false_lit;{Log_count64++;/*421*/}
}
(( c[1] == false_lit) ? static_cast<void> (0) : __assert_fail ( "c[1] == false_lit" , "/home/justyna/Documents/minisatgirelease/sources/core/Solver.cc", 424, __PRETTY_FUNCTION__)) ;{Log_count64++;/*424*/}
i++;{Log_count64++;/*425*/}
Lit first = c[0];
Watcher w = Watcher(cr, first);
{Log_count64++;/*428*/} if(first != blocker && value(first) == (lbool((uint8_t)0))) 
{
*j++ = w; {Log_count64++;/*430*/} continue;
}
for(int k = 2;k < c.size();k++) 
if(value(c[k]) != (lbool((uint8_t)1))) 
{{Log_count64++;/*433*/} 
c[1] = c[k]; c[k] = false_lit;{Log_count64++;/*435*/}
watches[~c[1]].push(w);{Log_count64++;/*436*/}
goto NextClause;{Log_count64++;/*437*/}
}
*j++ = w;{Log_count64++;/*439*/}
{Log_count64++;/*440*/} if(value(first) == (lbool((uint8_t)1))) 
{
confl = cr;{Log_count64++;/*442*/}
qhead = trail.size();{Log_count64++;/*443*/}
while(i < end) 
*j++ = *i++;{Log_count64++;/*445*/}
}
else
{
uncheckedEnqueue(first, cr);{Log_count64++;/*449*/}
}
NextClause:;{Log_count64++;/*451*/}
}
ws.shrink(i - j);{Log_count64++;/*453*/}
}
propagations += num_props;{Log_count64++;/*455*/}
simpDB_props -= num_props;{Log_count64++;/*456*/}
{Log_count64++;/*457*/} return confl;
}
struct reduceDB_lt
{
ClauseAllocator& ca;
reduceDB_lt(ClauseAllocator& ca_) : ca(ca_)
{
}
bool operator () (CRef x, CRef y)
{
{Log_count64++;/*467*/} return ca[x].size() > 2 && (ca[y].size() == 2 || ca[x].activity() < ca[y].activity());
}
}
;
void Solver::reduceDB()
{
int i, j;
double extra_lim = cla_inc / learnts.size();
sort(learnts, reduceDB_lt(ca));{Log_count64++;/*475*/}
for(i = j = 0;i < learnts.size();i++) 
{
Clause& c = ca[learnts[i]];
{Log_count64++;/*479*/} if(c.size() > 2 && !locked(c) && (i < learnts.size() / 2 || c.activity() < extra_lim)) 
{
removeClause(learnts[i]);{Log_count64++;/*481*/}
}
else
{
learnts[j++] = learnts[i];{Log_count64++;/*485*/}
}
}
learnts.shrink(i - j);{Log_count64++;/*488*/}
checkGarbage();{Log_count64++;/*489*/}
}
void Solver::removeSatisfied(vec<CRef>& cs)
{
int i, j;
for(i = j = 0;i < cs.size();i++) 
{
Clause& c = ca[cs[i]];
{Log_count64++;/*497*/} if(satisfied(c)) 
{
removeClause(cs[i]);{Log_count64++;/*499*/}
}
else
{
cs[j++] = cs[i];{Log_count64++;/*503*/}
}
}
cs.shrink(i - j);{Log_count64++;/*506*/}
}
void Solver::rebuildOrderHeap()
{
vec<Var> vs;
for(Var v = 0;v < nVars();v++) 
{
{Log_count64++;/*513*/} if(decision[v] && value(v) == (lbool((uint8_t)2))) 
{
vs.push(v);{Log_count64++;/*515*/}
}
}
order_heap.build(vs);{Log_count64++;/*518*/}
}
bool Solver::simplify()
{
(( decisionLevel() == 0) ? static_cast<void> (0) : __assert_fail ( "decisionLevel() == 0" , "/home/justyna/Documents/minisatgirelease/sources/core/Solver.cc", 523, __PRETTY_FUNCTION__)) ;{Log_count64++;/*523*/}
{Log_count64++;/*524*/} if(!ok || propagate() != CRef_Undef) 
{
{Log_count64++;/*526*/} return ok = false;
}
{Log_count64++;/*528*/} if(nAssigns() == simpDB_assigns || (simpDB_props > 0)) 
{
{Log_count64++;/*530*/} return true;
}
removeSatisfied(learnts);{Log_count64++;/*532*/}
{Log_count64++;/*533*/} if(remove_satisfied) 
{
removeSatisfied(clauses);{Log_count64++;/*535*/}
}
checkGarbage();{Log_count64++;/*537*/}
rebuildOrderHeap();{Log_count64++;/*538*/}
simpDB_assigns = nAssigns();{Log_count64++;/*539*/}
simpDB_props = clauses_literals + learnts_literals;{Log_count64++;/*540*/}
{Log_count64++;/*541*/} return true;
}
lbool Solver::search(int nof_conflicts)
{
(( ok) ? static_cast<void> (0) : __assert_fail ( "ok" , "/home/justyna/Documents/minisatgirelease/sources/core/Solver.cc", 546, __PRETTY_FUNCTION__)) ;{Log_count64++;/*546*/}
int backtrack_level;
int conflictC = 0;
vec<Lit> learnt_clause;
starts++;{Log_count64++;/*550*/}
for(;;) 
{
CRef confl = propagate();
{Log_count64++;/*554*/} if(confl != CRef_Undef) 
{
conflicts++; conflictC++;{Log_count64++;/*556*/}
{Log_count64++;/*557*/} if(decisionLevel() == 0) 
{
{Log_count64++;/*559*/} return (lbool((uint8_t)1));
}
learnt_clause.clear();{Log_count64++;/*561*/}
analyze(confl, learnt_clause, backtrack_level);{Log_count64++;/*562*/}
cancelUntil(backtrack_level);{Log_count64++;/*563*/}
{Log_count64++;/*564*/} if(learnt_clause.size() == 1) 
{
uncheckedEnqueue(learnt_clause[0]);{Log_count64++;/*566*/}
}
else
{
CRef cr = ca.alloc(learnt_clause, true);
learnts.push(cr);{Log_count64++;/*571*/}
attachClause(cr);{Log_count64++;/*572*/}
claBumpActivity(ca[cr]);{Log_count64++;/*573*/}
uncheckedEnqueue(learnt_clause[0], cr);{Log_count64++;/*574*/}
}
varDecayActivity();{Log_count64++;/*576*/}
claDecayActivity();{Log_count64++;/*577*/}
{Log_count64++;/*578*/} if(--learntsize_adjust_cnt == 0) 
{
learntsize_adjust_confl *= learntsize_adjust_inc;{Log_count64++;/*580*/}
learntsize_adjust_cnt = (int)learntsize_adjust_confl;{Log_count64++;/*581*/}
max_learnts *= learntsize_inc;{Log_count64++;/*582*/}
{Log_count64++;/*583*/} if(verbosity >= 1) 
{
printf("| %9d | %7d %8d %8d | %8d %8d %6.0f | %6.3f %% |\n",
(int)conflicts,
(int)dec_vars - (trail_lim.size() == 0 ? trail.size() : trail_lim[0]), nClauses(), (int)clauses_literals,
(int)max_learnts, nLearnts(), (double)learnts_literals/nLearnts(), progressEstimate()*100);
}
}
}
else
{
{Log_count64++;/*594*/} if(nof_conflicts >= 0 && conflictC >= nof_conflicts || !withinBudget()) 
{
progress_estimate = progressEstimate();{Log_count64++;/*596*/}
cancelUntil(0);{Log_count64++;/*597*/}
{Log_count64++;/*598*/} return (lbool((uint8_t)2));
}
{Log_count64++;/*600*/} if(decisionLevel() == 0 && !simplify()) 
{
{Log_count64++;/*602*/} return (lbool((uint8_t)1));
}
{Log_count64++;/*604*/} if(learnts.size()-nAssigns() >= max_learnts) 
{
reduceDB();{Log_count64++;/*606*/}
}
Lit next = lit_Undef;
while(decisionLevel() < assumptions.size()) 
{
Lit p = assumptions[decisionLevel()];
{Log_count64++;/*612*/} if(value(p) == (lbool((uint8_t)0))) 
{
newDecisionLevel();{Log_count64++;/*614*/}
}
else if(value(p) == (lbool((uint8_t)1))) 
{
analyzeFinal(~p, conflict);{Log_count64++;/*618*/}
{Log_count64++;/*619*/} return (lbool((uint8_t)1));
}
else
{
next = p;{Log_count64++;/*623*/}
{Log_count64++;/*624*/} break;
}
}
{Log_count64++;/*627*/} if(next == lit_Undef) 
{
decisions++;{Log_count64++;/*629*/}
next = pickBranchLit();{Log_count64++;/*630*/}
{Log_count64++;/*631*/} if(next == lit_Undef) 
{
{Log_count64++;/*633*/} return (lbool((uint8_t)0));
}
}
newDecisionLevel();{Log_count64++;/*636*/}
uncheckedEnqueue(next);{Log_count64++;/*637*/}
}
}
}
double Solver::progressEstimate() const
{
double progress = 0;
double F = 1.0 / nVars();
for(int i = 0;i <= decisionLevel();i++) 
{
int beg = i == 0 ? 0 : trail_lim[i - 1];
int end = i == decisionLevel() ? trail.size() : trail_lim[i];
progress += pow(F, i) * (end - beg);{Log_count64++;/*649*/}
}
{Log_count64++;/*651*/} return progress / nVars();
}
static double luby(double y, int x)
{
int size, seq;
for(size = 1, seq = 0;size < x+1;seq++, size = 2*size+1) ;
while(size-1 != x) 
{
size = (size-1)>>1;{Log_count64++;/*659*/}
seq--;{Log_count64++;/*660*/}
x = x % size;{Log_count64++;/*661*/}
}
{Log_count64++;/*663*/} return pow(y, seq);
}
lbool Solver::solve_()
{
model.clear();{Log_count64++;/*667*/}
conflict.clear();{Log_count64++;/*668*/}
{Log_count64++;/*669*/} if(!ok)  return (lbool((uint8_t)1));
solves++;{Log_count64++;/*670*/}
max_learnts = nClauses() * learntsize_factor;{Log_count64++;/*671*/}
learntsize_adjust_confl = learntsize_adjust_start_confl;{Log_count64++;/*672*/}
learntsize_adjust_cnt = (int)learntsize_adjust_confl;{Log_count64++;/*673*/}
lbool status = (lbool((uint8_t)2));
{Log_count64++;/*675*/} if(verbosity >= 1) 
{
printf("============================[ Search Statistics ]==============================\n");{Log_count64++;/*677*/}
printf("| Conflicts | ORIGINAL | LEARNT | Progress |\n");{Log_count64++;/*678*/}
printf("| | Vars Clauses Literals | Limit Clauses Lit/Cl | |\n");{Log_count64++;/*679*/}
printf("===============================================================================\n");{Log_count64++;/*680*/}
}
int curr_restarts = 0;
while(status == (lbool((uint8_t)2))) 
{
double rest_base = luby_restart ? luby(restart_inc, curr_restarts) : pow(restart_inc, curr_restarts);
status = search(rest_base * restart_first);{Log_count64++;/*686*/}
{Log_count64++;/*687*/} if(!withinBudget())  break;
curr_restarts++;{Log_count64++;/*688*/}
}
{Log_count64++;/*690*/} if(verbosity >= 1) 
{
printf("===============================================================================\n");{Log_count64++;/*692*/}
}
{Log_count64++;/*694*/} if(status == (lbool((uint8_t)0))) 
{
model.growTo(nVars());{Log_count64++;/*696*/}
for(int i = 0;i < nVars();i++)  model[i] = value(i);
}
else if(status == (lbool((uint8_t)1)) && conflict.size() == 0) 
{
ok = false;{Log_count64++;/*701*/}
}
cancelUntil(0);{Log_count64++;/*703*/}
{Log_count64++;/*704*/} return status;
}
static Var mapVar(Var x, vec<Var>& map, Var& max)
{
{Log_count64++;/*708*/} if(map.size() <= x || map[x] == -1) 
{
map.growTo(x+1, -1);{Log_count64++;/*710*/}
map[x] = max++;{Log_count64++;/*711*/}
}
{Log_count64++;/*713*/} return map[x];
}
void Solver::toDimacs(FILE* f, Clause& c, vec<Var>& map, Var& max)
{
{Log_count64++;/*717*/} if(satisfied(c)) 
{
{Log_count64++;/*719*/} return;
}
for(int i = 0;i < c.size();i++) 
{
{Log_count64++;/*723*/} if(value(c[i]) != (lbool((uint8_t)1))) 
{
fprintf(f, "%s%d ", sign(c[i]) ? "-" : "", mapVar(var(c[i]), map, max)+1);{Log_count64++;/*725*/}
}
}
fprintf(f, "0\n");{Log_count64++;/*728*/}
}
void Solver::toDimacs(const char *file, const vec<Lit>& assumps)
{
FILE* f = fopen(file, "wr");
{Log_count64++;/*733*/} if(f == __null) 
{
fprintf( stderr , "could not open file %s\n", file), exit(1);{Log_count64++;/*735*/}
}
toDimacs(f, assumps);{Log_count64++;/*737*/}
fclose(f);{Log_count64++;/*738*/}
}
void Solver::toDimacs(FILE* f, const vec<Lit>& assumps)
{
{Log_count64++;/*742*/} if(!ok) 
{
fprintf(f, "p cnf 1 2\n1 0\n-1 0\n");{Log_count64++;/*744*/}
{Log_count64++;/*745*/} return;
}
vec<Var> map; Var max = 0;
int cnt = 0;
for(int i = 0;i < clauses.size();i++) 
{
{Log_count64++;/*751*/} if(!satisfied(ca[clauses[i]])) 
{
cnt++;{Log_count64++;/*753*/}
}
}
for(int i = 0;i < clauses.size();i++) 
{
{Log_count64++;/*758*/} if(!satisfied(ca[clauses[i]])) 
{
Clause& c = ca[clauses[i]];
for(int j = 0;j < c.size();j++) 
 if(value(c[j]) != (lbool((uint8_t)1))) 
{{Log_count64++;/*762*/}
mapVar(var(c[j]), map, max);{Log_count64++;/*764*/}
}
}
}
cnt += assumptions.size();{Log_count64++;/*768*/}
fprintf(f, "p cnf %d %d\n", max, cnt);{Log_count64++;/*769*/}
for(int i = 0;i < assumptions.size();i++) 
{
(( value(assumptions[i]) != (lbool((uint8_t)1))) ? static_cast<void> (0) : __assert_fail ( "value(assumptions[i]) != l_False" , "/home/justyna/Documents/minisatgirelease/sources/core/Solver.cc", 773, __PRETTY_FUNCTION__)) ;{Log_count64++;/*773*/}
fprintf(f, "%s%d 0\n", sign(assumptions[i]) ? "-" : "", mapVar(var(assumptions[i]), map, max)+1);{Log_count64++;/*774*/}
}
for(int i = 0;i < clauses.size();i++) 
{
toDimacs(f, ca[clauses[i]], map, max);{Log_count64++;/*778*/}
}
{Log_count64++;/*780*/} if(verbosity > 0) 
{
printf("Wrote %d clauses with %d variables.\n", cnt, max);{Log_count64++;/*782*/}
}
}
void Solver::relocAll(ClauseAllocator& to)
{
watches.cleanAll();{Log_count64++;/*787*/}
for(int v = 0;v < nVars();v++) 
{
for(int s = 0;s < 2;s++) 
{
Lit p = mkLit(v, s);
vec<Watcher>& ws = watches[p];
for(int j = 0;j < ws.size();j++) 
{
ca.reloc(ws[j].cref, to);{Log_count64++;/*796*/}
}
}
}
for(int i = 0;i < trail.size();i++) 
{
Var v = var(trail[i]);
{Log_count64++;/*803*/} if(reason(v) != CRef_Undef && (ca[reason(v)].reloced() || locked(ca[reason(v)]))) 
{
ca.reloc(vardata[v].reason, to);{Log_count64++;/*805*/}
}
}
for(int i = 0;i < learnts.size();i++) 
{
ca.reloc(learnts[i], to);{Log_count64++;/*810*/}
}
for(int i = 0;i < clauses.size();i++) 
{
ca.reloc(clauses[i], to);{Log_count64++;/*814*/}
}
}
void Solver::garbageCollect()
{
ClauseAllocator to(ca.size() - ca.wasted());
relocAll(to);{Log_count64++;/*820*/}
{Log_count64++;/*821*/} if(verbosity >= 2) 
{
printf("| Garbage collection: %12d bytes => %12d bytes |\n",
ca.size()*ClauseAllocator::Unit_Size, to.size()*ClauseAllocator::Unit_Size);
}
to.moveTo(ca);{Log_count64++;/*826*/}
}
