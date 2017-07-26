#include <math.h>
#include "mtl/Sort.h"
#include "core/Solver.h"
using namespace Minisat;
static const char* _cat = "CORE";
static DoubleOption  opt_var_decay         (_cat, "var-decay",   "The variable activity decay factor",            0.95,     DoubleRange(0, false, 1, false));
static DoubleOption  opt_clause_decay      (_cat, "cla-decay",   "The clause activity decay factor",              0.999,    DoubleRange(0, false, 1, false));
static DoubleOption  opt_random_var_freq   (_cat, "rnd-freq",    "The frequency with which the decision heuristic tries to choose a random variable", 0, DoubleRange(0, true, 1, true));
static DoubleOption  opt_random_seed       (_cat, "rnd-seed",    "Used by the random variable selection",         91648253, DoubleRange(0, false, HUGE_VAL, false));
static IntOption     opt_ccmin_mode        (_cat, "ccmin-mode",  "Controls conflict clause minimization (0=none, 1=basic, 2=deep)", 2, IntRange(0, 2));
static IntOption     opt_phase_saving      (_cat, "phase-saving", "Controls the level of phase saving (0=none, 1=limited, 2=full)", 2, IntRange(0, 2));
static BoolOption    opt_rnd_init_act      (_cat, "rnd-init",    "Randomize the initial activity", false);
static BoolOption    opt_luby_restart      (_cat, "luby",        "Use the Luby restart sequence", true);
static IntOption     opt_restart_first     (_cat, "rfirst",      "The base restart interval", 100, IntRange(1, INT32_MAX));
static DoubleOption  opt_restart_inc       (_cat, "rinc",        "Restart interval increase factor", 2, DoubleRange(1, false, HUGE_VAL, false));
static DoubleOption  opt_garbage_frac      (_cat, "gc-frac",     "The fraction of wasted memory allowed before a garbage collection is triggered",  0.20, DoubleRange(0, false, HUGE_VAL, false));
Solver::Solver() :
    verbosity        (0)
  , var_decay        (opt_var_decay)
  , clause_decay     (opt_clause_decay)
  , random_var_freq  (opt_random_var_freq)
  , random_seed      (opt_random_seed)
  , luby_restart     (opt_luby_restart)
  , ccmin_mode       (opt_ccmin_mode)
  , phase_saving     (opt_phase_saving)
  , rnd_pol          (false)
  , rnd_init_act     (opt_rnd_init_act)
  , garbage_frac     (opt_garbage_frac)
  , restart_first    (opt_restart_first)
  , restart_inc      (opt_restart_inc)
  , learntsize_factor((double)1/(double)3), learntsize_inc(1.1)
  , learntsize_adjust_start_confl (100)
  , learntsize_adjust_inc         (1.5)
  , solves(0), starts(0), decisions(0), rnd_decisions(0), propagations(0), conflicts(0)
  , dec_vars(0), clauses_literals(0), learnts_literals(0), max_literals(0), tot_literals(0)
  , ok                 (true)
  , cla_inc            (1)
  , var_inc            (1)
  , watches            (WatcherDeleted(ca))
  , qhead              (0)
  , simpDB_assigns     (-1)
  , simpDB_props       (0)
  , order_heap         (VarOrderLt(activity))
  , progress_estimate  (0)
  , remove_satisfied   (true)
  , conflict_budget    (-1)
  , propagation_budget (-1)
  , asynch_interrupt   (false)
{
}
Solver::~Solver()
{
}
Var Solver::newVar(bool sign, bool dvar)
{
    int v = nVars();
    watches  .init(mkLit(v, false));
    watches  .init(mkLit(v, true ));
    assigns  .push(l_Undef);
    vardata  .push(mkVarData(CRef_Undef, 0));
    activity .push(rnd_init_act ? drand(random_seed) * 0.00001 : 0);
    seen     .push(0);
    polarity .push(sign);
    decision .push();
    trail    .capacity(v+1);
    setDecisionVar(v, dvar);
    return v;
}
bool Solver::addClause_(vec<Lit>& ps)
{
    assert(decisionLevel() == 0);
    if (!ok) 
    {
	return false;
    }
    sort(ps);
    Lit p; int i, j;
    for (i = j = 0, p = lit_Undef; i < ps.size(); i++)
    {
        if (value(ps[i]) == l_True || ps[i] == ~p)
	{
            return true;
	}
        else if (value(ps[i]) != l_False && ps[i] != p)
	{
            ps[j++] = p = ps[i];
	}
    }
    ps.shrink(i - j);
    if (ps.size() == 0)
{
        return ok = false;
}
    else if (ps.size() == 1)
{
        uncheckedEnqueue(ps[0]);
        return ok = (propagate() == CRef_Undef);
}
else
{
        CRef cr = ca.alloc(ps, false);
        clauses.push(cr);
        attachClause(cr);
}
    return true;
}
void Solver::attachClause(CRef cr) 
{
    const Clause& c = ca[cr];
    assert(c.size() > 1);
    watches[~c[0]].push(Watcher(cr, c[1]));
    watches[~c[1]].push(Watcher(cr, c[0]));
    if (c.learnt()) learnts_literals += c.size();
    else            clauses_literals += c.size(); 
}
void Solver::detachClause(CRef cr, bool strict) 
{
    const Clause& c = ca[cr];
    assert(c.size() > 1);
    if (strict)
{
        remove(watches[~c[0]], Watcher(cr, c[1]));
        remove(watches[~c[1]], Watcher(cr, c[0]));
}
else
{
        watches.smudge(~c[0]);
        watches.smudge(~c[1]);
}
    if (c.learnt()) 
{
	learnts_literals -= c.size();
}
    else
{            
	clauses_literals -= c.size(); 
}
}
void Solver::removeClause(CRef cr) 
{
    Clause& c = ca[cr];
    detachClause(cr);
    if (locked(c)) 
{
	vardata[var(c[0])].reason = CRef_Undef;
}
    c.mark(1); 
    ca.free(cr);
}
bool Solver::satisfied(const Clause& c) const 
{
    for (int i = 0; i < c.size(); i++)
    {
        if (value(c[i]) == l_True)
	{
            return true;
	}
    }
    return false; 
}
void Solver::cancelUntil(int level) 
{
    if (decisionLevel() > level)
{
        for (int c = trail.size()-1; c >= trail_lim[level]; c--)
{
            Var      x  = var(trail[c]);
            assigns [x] = l_Undef;
            if (phase_saving > 1 || (phase_saving == 1) && c > trail_lim.last())
	    {
                polarity[x] = sign(trail[c]);
	    }
            insertVarOrder(x); 
}
        qhead = trail_lim[level];
        trail.shrink(trail.size() - trail_lim[level]);
        trail_lim.shrink(trail_lim.size() - level);
}
}
Lit Solver::pickBranchLit()
{
    Var next = var_Undef;
    if (drand(random_seed) < random_var_freq && !order_heap.empty())
{
        next = order_heap[irand(random_seed,order_heap.size())];
        if (value(next) == l_Undef && decision[next])
	{
            rnd_decisions++; 
	}
}
    while (next == var_Undef || value(next) != l_Undef || !decision[next])
    {
        if (order_heap.empty())
	{
            next = var_Undef;
            break;
	}
	else
	{
            next = order_heap.removeMin();
	}
    }
    return next == var_Undef ? lit_Undef : mkLit(next, rnd_pol ? drand(random_seed) < 0.5 : polarity[next]);
}
void Solver::analyze(CRef confl, vec<Lit>& out_learnt, int& out_btlevel)
{
    int pathC = 0;
    Lit p     = lit_Undef;
    out_learnt.push();      
    int index   = trail.size() - 1;
    do
{
        assert(confl != CRef_Undef); 
        Clause& c = ca[confl];
        if (c.learnt())
	{
            claBumpActivity(c);
	}
        for (int j = (p == lit_Undef) ? 0 : 1; j < c.size(); j++)
{
            Lit q = c[j];
            if (!seen[var(q)] && level(var(q)) > 0)
{
                varBumpActivity(var(q));
                seen[var(q)] = 1;
                if (level(var(q)) >= decisionLevel())
		{
                    pathC++;
		}
                else
		{
                    out_learnt.push(q);
		}
}
}
        while (!seen[var(trail[index--])]);
        p     = trail[index+1];
        confl = reason(var(p));
        seen[var(p)] = 0;
        pathC--;
}
while (pathC > 0);
    out_learnt[0] = ~p;
    int i, j;
    out_learnt.copyTo(analyze_toclear);
    if (ccmin_mode == 2)
{
        uint32_t abstract_level = 0;
        for (i = 1; i < out_learnt.size(); i++)
	{
            abstract_level |= abstractLevel(var(out_learnt[i])); 
	}
        for (i = j = 1; i < out_learnt.size(); i++)
	{
            if (reason(var(out_learnt[i])) == CRef_Undef || !litRedundant(out_learnt[i], abstract_level))
	    {
                out_learnt[j++] = out_learnt[i];
	    }
	}
}
else if (ccmin_mode == 1)
{
        for (i = j = 1; i < out_learnt.size(); i++)
	{
            Var x = var(out_learnt[i]);
            if (reason(x) == CRef_Undef)
	    {
                out_learnt[j++] = out_learnt[i];
	    }
            else
	    {
                Clause& c = ca[reason(var(out_learnt[i]))];
                for (int k = 1; k < c.size(); k++)
		{
                    if (!seen[var(c[k])] && level(var(c[k])) > 0)
		    {
                        out_learnt[j++] = out_learnt[i];
                        break; 
		    }
		}
	    }
	}
}
else
{
        i = j = out_learnt.size();
}
    max_literals += out_learnt.size();
    out_learnt.shrink(i - j);
    tot_literals += out_learnt.size();
    if (out_learnt.size() == 1)
	{
        out_btlevel = 0;
	}
    else
{
        int max_i = 1;
        for (int i = 2; i < out_learnt.size(); i++)
	{
            if (level(var(out_learnt[i])) > level(var(out_learnt[max_i])))
	    {
                max_i = i;
	    }
	}
        Lit p             = out_learnt[max_i];
        out_learnt[max_i] = out_learnt[1];
        out_learnt[1]     = p;
        out_btlevel       = level(var(p));
}
    for (int j = 0; j < analyze_toclear.size(); j++) 
	{
	seen[var(analyze_toclear[j])] = 0;    
	}
}
bool Solver::litRedundant(Lit p, uint32_t abstract_levels)
{
    analyze_stack.clear(); analyze_stack.push(p);
    int top = analyze_toclear.size();
    while (analyze_stack.size() > 0)
{
        assert(reason(var(analyze_stack.last())) != CRef_Undef);
        Clause& c = ca[reason(var(analyze_stack.last()))]; analyze_stack.pop();
        for (int i = 1; i < c.size(); i++)
{
            Lit p  = c[i];
            if (!seen[var(p)] && level(var(p)) > 0)
{
                if (reason(var(p)) != CRef_Undef && (abstractLevel(var(p)) & abstract_levels) != 0)
{
                    seen[var(p)] = 1;
                    analyze_stack.push(p);
                    analyze_toclear.push(p);
}
else
{
                    for (int j = top; j < analyze_toclear.size(); j++)
		    {
                        seen[var(analyze_toclear[j])] = 0;
		    }
                    analyze_toclear.shrink(analyze_toclear.size() - top);
                    return false;
}
}
}
}
    return true;
}
void Solver::analyzeFinal(Lit p, vec<Lit>& out_conflict)
{
    out_conflict.clear();
    out_conflict.push(p);
    if (decisionLevel() == 0)
    {
        return;
    }
    seen[var(p)] = 1;
    for (int i = trail.size()-1; i >= trail_lim[0]; i--)
{
        Var x = var(trail[i]);
        if (seen[x])
{
            if (reason(x) == CRef_Undef)
{
                assert(level(x) > 0);
                out_conflict.push(~trail[i]);
}
else
{
                Clause& c = ca[reason(x)];
                for (int j = 1; j < c.size(); j++)
		{
                    if (level(var(c[j])) > 0)
			{
                        seen[var(c[j])] = 1;
			}
		}
}
            seen[x] = 0;
}
}
    seen[var(p)] = 0;
}
void Solver::uncheckedEnqueue(Lit p, CRef from)
{
    assert(value(p) == l_Undef);
    assigns[var(p)] = lbool(!sign(p));
    vardata[var(p)] = mkVarData(from, decisionLevel());
    trail.push_(p);
}
CRef Solver::propagate()
{
    CRef    confl     = CRef_Undef;
    int     num_props = 0;
    watches.cleanAll();
    while (qhead < trail.size())
{
        Lit            p   = trail[qhead++];     
        vec<Watcher>&  ws  = watches[p];
        Watcher        *i, *j, *end;
        num_props++;
        for (i = j = (Watcher*)ws, end = i + ws.size();  i != end;)
{
            Lit blocker = i->blocker;
            if (value(blocker) == l_True)
{
                *j++ = *i++; continue; 
}
            CRef     cr        = i->cref;
            Clause&  c         = ca[cr];
            Lit      false_lit = ~p;
            if (c[0] == false_lit)
	    {
                c[0] = c[1], c[1] = false_lit;
	    }
            assert(c[1] == false_lit);
            i++;
            Lit     first = c[0];
            Watcher w     = Watcher(cr, first);
            if (first != blocker && value(first) == l_True)
{
                *j++ = w; continue; 
}
            for (int k = 2; k < c.size(); k++)
                if (value(c[k]) != l_False)
{
                    c[1] = c[k]; c[k] = false_lit;
                    watches[~c[1]].push(w);
                    goto NextClause; 
}
            *j++ = w;
            if (value(first) == l_False)
{
                confl = cr;
                qhead = trail.size();
                while (i < end)
                    *j++ = *i++;
}
else
{
                uncheckedEnqueue(first, cr);
}
        NextClause:;
}
        ws.shrink(i - j);
}
    propagations += num_props;
    simpDB_props -= num_props;
    return confl;
}
struct reduceDB_lt 
{
    ClauseAllocator& ca;
    reduceDB_lt(ClauseAllocator& ca_) : ca(ca_) 
{
}
    bool operator () (CRef x, CRef y) 
{
        return ca[x].size() > 2 && (ca[y].size() == 2 || ca[x].activity() < ca[y].activity()); 
}
}
;
void Solver::reduceDB()
{
    int     i, j;
    double  extra_lim = cla_inc / learnts.size();    
    sort(learnts, reduceDB_lt(ca));
    for (i = j = 0; i < learnts.size(); i++)
{
        Clause& c = ca[learnts[i]];
        if (c.size() > 2 && !locked(c) && (i < learnts.size() / 2 || c.activity() < extra_lim))
	{
            removeClause(learnts[i]);
	}
        else
	{
            learnts[j++] = learnts[i];
	}
}
    learnts.shrink(i - j);
    checkGarbage();
}
void Solver::removeSatisfied(vec<CRef>& cs)
{
    int i, j;
    for (i = j = 0; i < cs.size(); i++)
{
        Clause& c = ca[cs[i]];
        if (satisfied(c))
	{
            removeClause(cs[i]);
	}
        else
	{
            cs[j++] = cs[i];
	}
}
    cs.shrink(i - j);
}
void Solver::rebuildOrderHeap()
{
    vec<Var> vs;
    for (Var v = 0; v < nVars(); v++)
    {
        if (decision[v] && value(v) == l_Undef)
	{
            vs.push(v);
	}
    }
    order_heap.build(vs);
}
bool Solver::simplify()
{
    assert(decisionLevel() == 0);
    if (!ok || propagate() != CRef_Undef)
    {
        return ok = false;
    }
    if (nAssigns() == simpDB_assigns || (simpDB_props > 0))
    {
        return true;
    }
    removeSatisfied(learnts);
    if (remove_satisfied)        
    {
        removeSatisfied(clauses);
    }
    checkGarbage();
    rebuildOrderHeap();
    simpDB_assigns = nAssigns();
    simpDB_props   = clauses_literals + learnts_literals;   
    return true;
}
lbool Solver::search(int nof_conflicts)
{
    assert(ok);
    int         backtrack_level;
    int         conflictC = 0;
    vec<Lit>    learnt_clause;
    starts++;
    for (;;)
{
        CRef confl = propagate();
        if (confl != CRef_Undef)
{
            conflicts++; conflictC++;
            if (decisionLevel() == 0)
	    {
		return l_False;
	    }
            learnt_clause.clear();
            analyze(confl, learnt_clause, backtrack_level);
            cancelUntil(backtrack_level);
            if (learnt_clause.size() == 1)
{
                uncheckedEnqueue(learnt_clause[0]);
}
else
{
                CRef cr = ca.alloc(learnt_clause, true);
                learnts.push(cr);
                attachClause(cr);
                claBumpActivity(ca[cr]);
                uncheckedEnqueue(learnt_clause[0], cr);
}
            varDecayActivity();
            claDecayActivity();
            if (--learntsize_adjust_cnt == 0)
{
                learntsize_adjust_confl *= learntsize_adjust_inc;
                learntsize_adjust_cnt    = (int)learntsize_adjust_confl;
                max_learnts             *= learntsize_inc;
                if (verbosity >= 1)
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
            if (nof_conflicts >= 0 && conflictC >= nof_conflicts || !withinBudget())
{
                progress_estimate = progressEstimate();
                cancelUntil(0);
                return l_Undef; 
}
            if (decisionLevel() == 0 && !simplify())
		{
                return l_False;
		}
            if (learnts.size()-nAssigns() >= max_learnts)
		{
                reduceDB();
		}
            Lit next = lit_Undef;
            while (decisionLevel() < assumptions.size())
{
                Lit p = assumptions[decisionLevel()];
                if (value(p) == l_True)
{
                    newDecisionLevel();
}
else if (value(p) == l_False)
{
                    analyzeFinal(~p, conflict);
                    return l_False;
}
else
{
                    next = p;
                    break;
}
}
            if (next == lit_Undef)
{
                decisions++;
                next = pickBranchLit();
                if (next == lit_Undef)
		{
                    return l_True;
		}
}
            newDecisionLevel();
            uncheckedEnqueue(next);
}
}
}
double Solver::progressEstimate() const
{
    double  progress = 0;
    double  F = 1.0 / nVars();
    for (int i = 0; i <= decisionLevel(); i++)
{
        int beg = i == 0 ? 0 : trail_lim[i - 1];
        int end = i == decisionLevel() ? trail.size() : trail_lim[i];
        progress += pow(F, i) * (end - beg);
}
    return progress / nVars();
}
static double luby(double y, int x)
{
    int size, seq;
    for (size = 1, seq = 0; size < x+1; seq++, size = 2*size+1);
    while (size-1 != x)
{
        size = (size-1)>>1;
        seq--;
        x = x % size;
}
    return pow(y, seq);
}
lbool Solver::solve_()
{
    model.clear();
    conflict.clear();
    if (!ok) return l_False;
    solves++;
    max_learnts               = nClauses() * learntsize_factor;
    learntsize_adjust_confl   = learntsize_adjust_start_confl;
    learntsize_adjust_cnt     = (int)learntsize_adjust_confl;
    lbool   status            = l_Undef;
    if (verbosity >= 1)
{
        printf("============================[ Search Statistics ]==============================\n");
        printf("| Conflicts |          ORIGINAL         |          LEARNT          | Progress |\n");
        printf("|           |    Vars  Clauses Literals |    Limit  Clauses Lit/Cl |          |\n");
        printf("===============================================================================\n");
}
    int curr_restarts = 0;
    while (status == l_Undef)
{
        double rest_base = luby_restart ? luby(restart_inc, curr_restarts) : pow(restart_inc, curr_restarts);
        status = search(rest_base * restart_first);
        if (!withinBudget()) break;
        curr_restarts++;
}
    if (verbosity >= 1)
	{
        printf("===============================================================================\n");
	}
    if (status == l_True)
{
        model.growTo(nVars());
        for (int i = 0; i < nVars(); i++) model[i] = value(i);
}
else if (status == l_False && conflict.size() == 0)
	{
        ok = false;
	}
    cancelUntil(0);
    return status;
}
static Var mapVar(Var x, vec<Var>& map, Var& max)
{
    if (map.size() <= x || map[x] == -1)
{
        map.growTo(x+1, -1);
        map[x] = max++;
}
    return map[x];
}
void Solver::toDimacs(FILE* f, Clause& c, vec<Var>& map, Var& max)
{
    if (satisfied(c))
	{
	return;
	}
    for (int i = 0; i < c.size(); i++)
    {
        if (value(c[i]) != l_False)
	{
            fprintf(f, "%s%d ", sign(c[i]) ? "-" : "", mapVar(var(c[i]), map, max)+1);
	}
    }
    fprintf(f, "0\n");
}
void Solver::toDimacs(const char *file, const vec<Lit>& assumps)
{
    FILE* f = fopen(file, "wr");
    if (f == NULL)
	{
        fprintf(stderr, "could not open file %s\n", file), exit(1);
	}
    toDimacs(f, assumps);
    fclose(f);
}
void Solver::toDimacs(FILE* f, const vec<Lit>& assumps)
{
    if (!ok)
{
        fprintf(f, "p cnf 1 2\n1 0\n-1 0\n");
        return; 
}
    vec<Var> map; Var max = 0;
    int cnt = 0;
    for (int i = 0; i < clauses.size(); i++)
    {
        if (!satisfied(ca[clauses[i]]))
	{
            cnt++;
	}
    }
    for (int i = 0; i < clauses.size(); i++)
    {
        if (!satisfied(ca[clauses[i]]))
{
            Clause& c = ca[clauses[i]];
            for (int j = 0; j < c.size(); j++)
                if (value(c[j]) != l_False)
		{
                    mapVar(var(c[j]), map, max);
		}
}
    }
    cnt += assumptions.size();
    fprintf(f, "p cnf %d %d\n", max, cnt);
    for (int i = 0; i < assumptions.size(); i++)
{
        assert(value(assumptions[i]) != l_False);
        fprintf(f, "%s%d 0\n", sign(assumptions[i]) ? "-" : "", mapVar(var(assumptions[i]), map, max)+1);
}
    for (int i = 0; i < clauses.size(); i++)
	{
        toDimacs(f, ca[clauses[i]], map, max);
	}
    if (verbosity > 0)
	{
        printf("Wrote %d clauses with %d variables.\n", cnt, max);
	}
}
void Solver::relocAll(ClauseAllocator& to)
{
    watches.cleanAll();
    for (int v = 0; v < nVars(); v++)
    {
        for (int s = 0; s < 2; s++)
{
            Lit p = mkLit(v, s);
            vec<Watcher>& ws = watches[p];
            for (int j = 0; j < ws.size(); j++)
	    {
                ca.reloc(ws[j].cref, to);
	    }
}
    }
    for (int i = 0; i < trail.size(); i++)
{
        Var v = var(trail[i]);
        if (reason(v) != CRef_Undef && (ca[reason(v)].reloced() || locked(ca[reason(v)])))
	{
            ca.reloc(vardata[v].reason, to);
	}
}
    for (int i = 0; i < learnts.size(); i++)
	{
        ca.reloc(learnts[i], to);
	}
    for (int i = 0; i < clauses.size(); i++)
	{
        ca.reloc(clauses[i], to);
	}
}
void Solver::garbageCollect()
{
    ClauseAllocator to(ca.size() - ca.wasted()); 
    relocAll(to);
    if (verbosity >= 2)
	{
        printf("|  Garbage collection:   %12d bytes => %12d bytes             |\n", 
               ca.size()*ClauseAllocator::Unit_Size, to.size()*ClauseAllocator::Unit_Size);
	}
    to.moveTo(ca);
}
