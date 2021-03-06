/****************************************************************************************[Solver.C]
MiniSat -- Copyright (c) 2003-2006, Niklas Een, Niklas Sorensson

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
associated documentation files (the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge, publish, distribute,
sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or
substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT
OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
**************************************************************************************************/

#include "Solver.h"
#include "Sort.h"
#include <cmath>


//=================================================================================================
// Constructor/Destructor:

int Solver::choosepolarity()
{
    if (1) 
    {
        return polarity_false;
    }
    return polarity_user;
}

char Solver::choosesign(char sign)
{
    if (1) 
    {
        return sign;
    }
    return false;
}

Solver::Solver() :

    // Parameters: (formerly in 'SearchParams')
    var_decay(1 / 0.95), clause_decay(1 / 0.999), random_var_freq(0.02)
  , restart_first(100), restart_inc(1.5), learntsize_factor((double)1/(double)3), learntsize_inc(1.1)

    // More parameters:
    //
  , expensive_ccmin  (true)
  , polarity_mode    (choosepolarity())
  //, polarity_mode    (polarity_false)
  , verbosity        (0)

    // Statistics: (formerly in 'SolverStats')
    //
  , starts(0), decisions(0), rnd_decisions(0), propagations(0), conflicts(0)
  , clauses_literals(0), learnts_literals(0), max_literals(0), tot_literals(0)

  , ok               (true)
  , cla_inc          (1)
  , var_inc          (1)
  , qhead            (0)
  , simpDB_assigns   (-1)
  , simpDB_props     (0)
  , order_heap       (VarOrderLt(activity))
  , random_seed      (91648253)
  , progress_estimate(0)
  , remove_satisfied (true)
{}


Solver::~Solver()
{
    for (int i = 0; i < learnts.size(); i++) 
    {
		free(learnts[i]);
    }
    for (int i = 0; i < clauses.size(); i++) 
	{
		free(clauses[i]);
	}
}


//=================================================================================================
// Minor methods:


// Creates a new SAT variable in the solver. If 'decision_var' is cleared, variable will not be
// used as a decision variable (NOTE! This has effects on the meaning of a SATISFIABLE result).
//

Var Solver::newVar(bool sign, bool dvar)
{
    int v = nVars();
    watches   .push();          // (list for positive literal)
    watches   .push();          // (list for negative literal)
    reason    .push(NULL);
    assigns   .push(toInt(l_Undef));
    level     .push(-1);
    activity  .push(0);
    seen      .push(0);

    polarity    .push(choosesign((char)sign));
    //polarity    .push((char)sign);
    decision_var.push((char)dvar);

    insertVarOrder(v);
    return v;
}


bool Solver::addClause(vec<Lit>& ps)
{
    assert(decisionLevel() == 0);

    if (!ok)
	{
        return false;
	}
    else
	{
        // Check if clause is satisfied and remove false/duplicate literals:
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
	}

    if (ps.size() == 0)
	{
        return ok = false;
	}
    else if (ps.size() == 1)
	{
        assert(value(ps[0]) == l_Undef);
        uncheckedEnqueue(ps[0]);
        return ok = (propagate() == NULL);
    }
	else
	{
        Clause* c = Clause::Clause_new(ps, false);
        clauses.push(c);
        attachClause(*c);
    }

    return true;
}


void Solver::attachClause(Clause& c) 
{
    assert(c.size() > 1);
    watches[toInt(~c[0])].push(&c);
    watches[toInt(~c[1])].push(&c);
    if (c.learnt()) 
	{
		learnts_literals += c.size();
	}
    else
	{
            clauses_literals += c.size(); 
	}
}


void Solver::detachClause(Clause& c) 
{
    assert(c.size() > 1);
    assert(find(watches[toInt(~c[0])], &c));
    assert(find(watches[toInt(~c[1])], &c));
    remove(watches[toInt(~c[0])], &c);
    remove(watches[toInt(~c[1])], &c);
    if (c.learnt()) 
	{
		learnts_literals -= c.size();
	}
    else
	{
            clauses_literals -= c.size(); 
	}
}


void Solver::removeClause(Clause& c) 
{
    detachClause(c);
    free(&c); 
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


// Revert to the state at given level (keeping all assignment at 'level' but not beyond).
//
void Solver::cancelUntil(int level) 
{
    if (decisionLevel() > level)
	{
        for (int c = trail.size()-1; c >= trail_lim[level]; c--)
		{
            Var     x  = var(trail[c]);
            assigns[x] = toInt(l_Undef);
            insertVarOrder(x); 
		}
        qhead = trail_lim[level];
        trail.shrink(trail.size() - trail_lim[level]);
        trail_lim.shrink(trail_lim.size() - level);
    } 
}


//=================================================================================================
// Major methods:


Lit Solver::pickBranchLit(int polarity_mode, double random_var_freq)
{
    Var next = var_Undef;

    // Random decision:
    if (drand(random_seed) < random_var_freq && !order_heap.empty())
	{
        next = order_heap[irand(random_seed,order_heap.size())];
        if (toLbool(assigns[next]) == l_Undef && decision_var[next])
		{
            rnd_decisions++; 
		}
	}

    // Activity based decision:
    while (next == var_Undef || toLbool(assigns[next]) != l_Undef || !decision_var[next])
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

    bool sign = false;
    switch (polarity_mode)
	{
    case polarity_true:  sign = false; break;
    case polarity_false: sign = true;  break;
    case polarity_user:  sign = polarity[next]; break;
    case polarity_rnd:   sign = irand(random_seed, 2); break;
    default: assert(false); 
	}

    return next == var_Undef ? lit_Undef : Lit(next, sign);
}


/*_________________________________________________________________________________________________
|
|  analyze : (confl : Clause*) (out_learnt : vec<Lit>&) (out_btlevel : int&)  ->  [void]
|  
|  Description:
|    Analyze conflict and produce a reason clause.
|  
|    Pre-conditions:
|      * 'out_learnt' is assumed to be cleared.
|      * Current decision level must be greater than root level.
|  
|    Post-conditions:
|      * 'out_learnt[0]' is the asserting literal at level 'out_btlevel'.
|  
|  Effect:
|    Will undo part of the trail, upto but not beyond the assumption of the current decision level.
|________________________________________________________________________________________________@*/
void Solver::analyze(Clause* confl, vec<Lit>& out_learnt, int& out_btlevel)
{
    int pathC = 0;
    Lit p     = lit_Undef;

    // Generate conflict clause:
    //
    out_learnt.push();      // (leave room for the asserting literal)
    int index   = trail.size() - 1;
    out_btlevel = 0;

    do
	{
        assert(confl != NULL);          // (otherwise should be UIP)
        Clause& c = *confl;

        if (c.learnt())
		{
            claBumpActivity(c);
		}

        for (int j = (p == lit_Undef) ? 0 : 1; j < c.size(); j++)
		{
            Lit q = c[j];

            if (!seen[var(q)] && level[var(q)] > 0)
			{
                varBumpActivity(var(q));
                seen[var(q)] = 1;
                if (level[var(q)] >= decisionLevel())
				{
                    pathC++;
				}
                else
				{
                    out_learnt.push(q);
                    if (level[var(q)] > out_btlevel)
					{
                        out_btlevel = level[var(q)];
					}
                }
            }
        }

        // Select next clause to look at:
        while (!seen[var(trail[index--])]);
        p     = trail[index+1];
        confl = reason[var(p)];
        seen[var(p)] = 0;
        pathC--;

    }
	while (pathC > 0);
    out_learnt[0] = ~p;

    // Simplify conflict clause:
    //
    int i, j;
    if (expensive_ccmin)
	{
        uint64_t abstract_level = 0;
        for (i = 1; i < out_learnt.size(); i++)
		{
            abstract_level |= abstractLevel(var(out_learnt[i])); // (maintain an abstraction of levels involved in conflict)
		}

        out_learnt.copyTo(analyze_toclear);

	if (1)
	{
        for (i = j = 1; i < out_learnt.size(); i++)
		{
            if (reason[var(out_learnt[i])] == NULL || !litRedundant(out_learnt[i], abstract_level))
			{
                out_learnt[j++] = out_learnt[i];
			}
		}
	}
	else
	{
/**/      i = out_learnt.size();
/**/      int found_some = find_removable(out_learnt, i, abstract_level);
/**/      if (found_some)
	  {
/**/          j = prune_removable(out_learnt);
	  }
/**/      else
	  {
/**/          j = i;
	  }
	}

    }
	else
	{
        out_learnt.copyTo(analyze_toclear);
        for (i = j = 1; i < out_learnt.size(); i++)
		{
	    if (0)
		{
/**/        	if (reason[var(out_learnt[i])] == NULL) 
			{            // BUG FIX
/**/                	out_learnt[j++] = out_learnt[i]; 
/**/		    	continue; 
			} // BUG FIX
		}
            Clause& c = *reason[var(out_learnt[i])];
            for (int k = 1; k < c.size(); k++)
			{
                if (!seen[var(c[k])] && level[var(c[k])] > 0)
				{
                    out_learnt[j++] = out_learnt[i];
                    break; 
				}
			}
        }
    }
    max_literals += out_learnt.size();
    if (1)
	{
	out_learnt.shrink(i - j);
	}
	else
	{
/**/  out_learnt.shrink(out_learnt.size() - j);
	}
    tot_literals += out_learnt.size();

    // Find correct backtrack level:
    //
    if (out_learnt.size() == 1)
	{
        out_btlevel = 0;
	}
    else
	{
        int max_i = 1;
        for (int i = 2; i < out_learnt.size(); i++)
		{
            if (level[var(out_learnt[i])] > level[var(out_learnt[max_i])])
			{
                max_i = i;
			}
		}
        Lit p             = out_learnt[max_i];
        out_learnt[max_i] = out_learnt[1];
        out_learnt[1]     = p;
        out_btlevel       = level[var(p)];
    }


    for (int j = 0; j < analyze_toclear.size(); j++) 
	{
		seen[var(analyze_toclear[j])] = 0;    // ('seen[]' is now cleared)
	}
}


// Check if 'p' can be removed. 'abstract_levels' is used to abort early if the algorithm is
// visiting literals at levels that cannot be removed later.
bool Solver::litRedundant(Lit p, uint64_t abstract_levels)
{
    analyze_stack.clear(); analyze_stack.push(p);
    int top = analyze_toclear.size();
    while (analyze_stack.size() > 0)
	{
        assert(reason[var(analyze_stack.last())] != NULL);
        Clause& c = *reason[var(analyze_stack.last())]; analyze_stack.pop();

        for (int i = 1; i < c.size(); i++)
		{
            Lit p  = c[i];
            if (!seen[var(p)] && level[var(p)] > 0)
			{
                if (reason[var(p)] != NULL && (abstractLevel(var(p)) & abstract_levels) != 0)
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


/*_________________________________________________________________________________________________
|
|  analyzeFinal : (p : Lit)  ->  [void]
|  
|  Description:
|    Specialized analysis procedure to express the final conflict in terms of assumptions.
|    Calculates the (possibly empty) set of assumptions that led to the assignment of 'p', and
|    stores the result in 'out_conflict'.
|________________________________________________________________________________________________@*/
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
            if (reason[x] == NULL)
			{
                assert(level[x] > 0);
                out_conflict.push(~trail[i]);
            }
			else
			{
                Clause& c = *reason[x];
                for (int j = 1; j < c.size(); j++)
				{
                    if (level[var(c[j])] > 0)
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


void Solver::uncheckedEnqueue(Lit p, Clause* from)
{
    assert(value(p) == l_Undef);
    assigns [var(p)] = toInt(lbool(!sign(p)));  // <<== abstract but not uttermost effecient
    level   [var(p)] = decisionLevel();
    reason  [var(p)] = from;
    if (0) 
    {
        polarity[var(p)] = sign(p);
    }
    trail.push(p);
}


/*_________________________________________________________________________________________________
|
|  propagate : [void]  ->  [Clause*]
|  
|  Description:
|    Propagates all enqueued facts. If a conflict arises, the conflicting clause is returned,
|    otherwise NULL.
|  
|    Post-conditions:
|      * the propagation queue is empty, even if there was a conflict.
|________________________________________________________________________________________________@*/
Clause* Solver::propagate()
{
    Clause* confl     = NULL;
    int     num_props = 0;

    while (qhead < trail.size())
	{
        Lit            p   = trail[qhead++];     // 'p' is enqueued fact to propagate.
        vec<Clause*>&  ws  = watches[toInt(p)];
        Clause         **i, **j, **end;
        num_props++;

        for (i = j = (Clause**)ws, end = i + ws.size();  i != end;)
		{
            Clause& c = **i++;

            // Make sure the false literal is data[1]:
            Lit false_lit = ~p;
            if (c[0] == false_lit)
			{
                c[0] = c[1], c[1] = false_lit;
			}

            assert(c[1] == false_lit);

            // If 0th watch is true, then clause is already satisfied.
            Lit first = c[0];
            if (value(first) == l_True)
			{
                *j++ = &c;
            }
			else
			{
                // Look for new watch:
                for (int k = 2; k < c.size(); k++)
				{
                    if (value(c[k]) != l_False)
					{
                        c[1] = c[k]; c[k] = false_lit;
                        watches[toInt(~c[1])].push(&c);
                        goto FoundWatch; 
					}
				}

                // Did not find watch -- clause is unit under assignment:
                *j++ = &c;
                if (value(first) == l_False)
				{
                    confl = &c;
                    qhead = trail.size();
                    // Copy the remaining watches:
                    while (i < end)
					{
                        *j++ = *i++;
					}
                }
				else
				{
                    uncheckedEnqueue(first, &c);
				}
            }
        FoundWatch:;
        }
        ws.shrink(i - j);
    }
    propagations += num_props;
    simpDB_props -= num_props;

    return confl;
}

/*_________________________________________________________________________________________________
|
|  reduceDB : ()  ->  [void]
|  
|  Description:
|    Remove half of the learnt clauses, minus the clauses locked by the current assignment. Locked
|    clauses are clauses that are reason to some assignment. Binary clauses are never removed.
|________________________________________________________________________________________________@*/
struct reduceDB_lt { bool operator () (Clause* x, Clause* y) { return x->size() > 2 && (y->size() == 2 || x->activity() < y->activity()); } };
void Solver::reduceDB()
{
    int     i, j;
    double  extra_lim = cla_inc / learnts.size();    // Remove any clause below this activity

    sort(learnts, reduceDB_lt());
    for (i = j = 0; i < learnts.size() / 2; i++)
	{
        if (learnts[i]->size() > 2 && !locked(*learnts[i]))
		{
            removeClause(*learnts[i]);
		}
        else
		{
            learnts[j++] = learnts[i];
		}
    }
    for (; i < learnts.size(); i++)
	{
        if (learnts[i]->size() > 2 && !locked(*learnts[i]) && learnts[i]->activity() < extra_lim)
		{
            removeClause(*learnts[i]);
		}
        else
		{
            learnts[j++] = learnts[i];
		}
    }
    learnts.shrink(i - j);
    if (0) 
    {
        nof_learnts   *= learntsize_inc;
    }
}


void Solver::removeSatisfied(vec<Clause*>& cs)
{
    int i,j;
    for (i = j = 0; i < cs.size(); i++)
	{
        if (satisfied(*cs[i]))
		{
            removeClause(*cs[i]);
		}
        else
		{
            cs[j++] = cs[i];
		}
    }
    cs.shrink(i - j);
}


/*_________________________________________________________________________________________________
|
|  simplify : [void]  ->  [bool]
|  
|  Description:
|    Simplify the clause database according to the current top-level assigment. Currently, the only
|    thing done here is the removal of satisfied clauses, but more things can be put here.
|________________________________________________________________________________________________@*/
bool Solver::simplify()
{
    assert(decisionLevel() == 0);

    if (!ok || propagate() != NULL)
	{
        return ok = false;
	}

    if (nAssigns() == simpDB_assigns || (simpDB_props > 0))
	{
        return true;
	}

    // Remove satisfied clauses:
    removeSatisfied(learnts);
    if (remove_satisfied)        // Can be turned off.
	{
        removeSatisfied(clauses);
	}

    // Remove fixed variables from the variable heap:
    order_heap.filter(VarFilter(*this));

    simpDB_assigns = nAssigns();
    simpDB_props   = clauses_literals + learnts_literals;   // (shouldn't depend on stats really, but it will do for now)

    return true;
}


/*_________________________________________________________________________________________________
|
|  search : (nof_conflicts : int) (nof_learnts : int) (params : const SearchParams&)  ->  [lbool]
|  
|  Description:
|    Search for a model the specified number of conflicts, keeping the number of learnt clauses
|    below the provided limit. NOTE! Use negative value for 'nof_conflicts' or 'nof_learnts' to
|    indicate infinity.
|  
|  Output:
|    'l_True' if a partial assigment that is consistent with respect to the clauseset is found. If
|    all variables are decision variables, this means that the clause set is satisfiable. 'l_False'
|    if the clause set is unsatisfiable. 'l_Undef' if the bound on number of conflicts is reached.
|________________________________________________________________________________________________@*/
lbool Solver::search(int nof_conflicts, int nof_learnts)
{
    assert(ok);
    int         backtrack_level;
    int         conflictC = 0;
    vec<Lit>    learnt_clause;

    starts++;

    bool first = true;

    for (;;)
	{
        Clause* confl = propagate();
        if (confl != NULL)
		{
            // CONFLICT
            conflicts++; conflictC++;
            if (decisionLevel() == 0) return l_False;

            first = false;

            learnt_clause.clear();
            analyze(confl, learnt_clause, backtrack_level);
            cancelUntil(backtrack_level);
            assert(value(learnt_clause[0]) == l_Undef);

	    if (0) 
	    {
		backtrackLevels[conflicts % restartMore]= backtrack_level;
	    }

            if (learnt_clause.size() == 1)
			{
                uncheckedEnqueue(learnt_clause[0]);
            }
			else
			{
                Clause* c = Clause::Clause_new(learnt_clause, true);
                learnts.push(c);
                attachClause(*c);
                claBumpActivity(*c);
                uncheckedEnqueue(learnt_clause[0], c);
            }

            varDecayActivity();
            claDecayActivity();

        }
		else
		{
            // NO CONFLICT

	    	if (0) 
			{
		    	if (conflictC >= restartMore) 	
				{ // search and count local minimum
					int LM= backtrackLevels[0];
					int nofLM= 1;
					
					for(int i=1; i< restartMore; i++) 
					{
						if(backtrackLevels[i]< LM) 
						{
							LM= backtrackLevels[i];
							nofLM= 1;
						} 
						else if(backtrackLevels[i]== LM) 
						{
							nofLM++;
						}
					}

					if(LM > restartTolerance && nofLM>= restartLess) 
					{ // restart
						progress_estimate= progressEstimate();
			    		cancelUntil(0);
						return l_Undef; 
					}
				}
	    	}

	    	if (1) 
		{
			if (nof_conflicts >= 0 && conflictC >= nof_conflicts)
				{
				// Reached bound on number of conflicts:
					progress_estimate = progressEstimate();
					cancelUntil(0);
					return l_Undef; 
				}
	    	}

            // Simplify the set of problem clauses:
            if (decisionLevel() == 0 && !simplify())
			{
                return l_False;
			}

            if (nof_learnts >= 0 && learnts.size()-nAssigns() >= nof_learnts)
			{
                // Reduce the set of learnt clauses:
                reduceDB();
			}

            Lit next = lit_Undef;
            while (decisionLevel() < assumptions.size())
			{
                // Perform user provided assumption:
                Lit p = assumptions[decisionLevel()];
                if (value(p) == l_True)
				{
                    // Dummy decision level:
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
                // New variable decision:
                decisions++;
                next = pickBranchLit(polarity_mode, random_var_freq);

                if (next == lit_Undef)
				{
                    // Model found:
                    return l_True;
				}
            }

            // Increase decision level and enqueue 'next'
            assert(value(next) == l_Undef);
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


bool Solver::solve(const vec<Lit>& assumps)
{
    model.clear();
    conflict.clear();

    if (!ok) 
	{
		return false;
	}

    assumps.copyTo(assumptions);


    double  nof_conflicts = restart_first;
    double  nof_learnts   = nClauses() * learntsize_factor;
    if (0) 
    {
		double cvr= (double)nClauses() / (double)nVars();
        nof_learnts= 300000 / cvr;
    }
	restartLess= 5;
	restartMore= 42;
	restartTolerance= nVars() / 10000 +10;
	backtrackLevels= new int[restartMore];

    lbool   status        = l_Undef;

    //if (verbosity >= 1){
    //    reportf("============================[ Search Statistics ]==============================\n");
    //    reportf("| Conflicts |          ORIGINAL         |          LEARNT          | Progress |\n");
    //    reportf("|           |    Vars  Clauses Literals |    Limit  Clauses Lit/Cl |          |\n");
    //    reportf("===============================================================================\n");
    //}

    // Search:
    while (status == l_Undef)
	{
        //if (verbosity >= 1)
        //    reportf("| %9d | %7d %8d %8d | %8d %8d %6.0f | %6.3f %% |\n", (int)conflicts, order_heap.size(), nClauses(), (int)clauses_literals, (int)nof_learnts, nLearnts(), (double)learnts_literals/nLearnts(), progress_estimate*100), fflush(stdout);
        status = search((int)nof_conflicts, (int)nof_learnts);
        nof_conflicts *= restart_inc;
        nof_learnts   *= learntsize_inc;
    }

    //if (verbosity >= 1)
    //    reportf("===============================================================================\n");


    if (status == l_True)
	{
        // Extend & copy model:
        model.growTo(nVars());
        for (int i = 0; i < nVars(); i++) 
		{
			model[i] = value(i);
		}
#ifndef NDEBUG
        verifyModel();
#endif
    }
	else
	{
        assert(status == l_False);
        if (conflict.size() == 0)
		{
            ok = false;
		}
    }

    cancelUntil(0);
    return status == l_True;
}

//=================================================================================================
// Debug methods:


void Solver::verifyModel()
{
    bool failed = false;
    for (int i = 0; i < clauses.size(); i++)
	{
        assert(clauses[i]->mark() == 0);
        Clause& c = *clauses[i];
        for (int j = 0; j < c.size(); j++)
		{
            if (modelValue(c[j]) == l_True)
			{
                goto next;
			}
		}

        //reportf("unsatisfied clause: ");
        //printClause(*clauses[i]);
        //reportf("\n");
        failed = true;
    next:;
    }

    assert(!failed);

    //reportf("Verified %d original clauses.\n", clauses.size());
}


void Solver::checkLiteralCount()
{
    // Check that sizes are calculated correctly:
    int cnt = 0;
    for (int i = 0; i < clauses.size(); i++)
	{
        if (clauses[i]->mark() == 0)
		{
            cnt += clauses[i]->size();
		}
	}

    if ((int)clauses_literals != cnt)
	{
        //fprintf(stderr, "literal count: %d, real value = %d\n", (int)clauses_literals, cnt);
        assert((int)clauses_literals == cnt);
    }
}



/**/ int  Solver::prune_removable(vec<Lit>& out_learnt) 
{
/**/     int i, j, sz = out_learnt.size();
/**/     j = 1;
/**/     for (i = 1; i < sz; i++) 
	 {
/**/         if ((seen[var(out_learnt[i])] & (1|2)) == (1|2)) 
		{
/**/             assert((seen[var(out_learnt[i])] & (4|8)) == 0);
/**/             out_learnt[j++] = out_learnt[i];
/**/         }
/**/     }
/**/     return j; 
/**/ }

/**/ int  Solver::find_removable(vec<Lit>& out_learnt, uint32_t sz0, uint32_t abstract_level) 
{
/**/     int found_some;
/**/     found_some = 0;
/**/     int sz = out_learnt.size();
/**/     int i;
#ifdef SHOW_CONF
/**/ // reportf("out_learnt.size = %d; i = %d; ab_lev = %d\n" , out_learnt.size(), i, abstract_level);
/**/ // for(int ii = 0; ii < out_learnt.size(); ii++) reportf(" %d", toInt(out_learnt[ii]));
/**/ // reportf("\n");
#endif
/**/ 
/**/     for (i = 1; i < sz; i++) 
	 {
/**/         Lit curLit = out_learnt[i];
/**/         if (level[var(curLit)] <= 0)
		{
/**/             continue;
		}
/**/ 
/**/         if ((seen[var(curLit)] & (2|4|8)) == 0) 
		{
/**/             found_some |= dfs_removable(curLit, abstract_level);
/**/         	}
/**/     }
/**/     return found_some;
/**/ }

/**/ int  Solver::quick_keeper(Lit p, uint64_t abstract_level, int maykeep) 
{
/**/     // See if I can kill myself right away.
/**/     // maykeep == 1 if I am in the original conflict clause.
/**/     if (reason[var(p)] == NULL) 
	 {
/**/         return (maykeep ? 2 : 8);
/**/     } 
	 else if ((abstractLevel(var(p)) & abstract_level) == 0) 
	 {
/**/         assert(maykeep == 0);
/**/         return 8;
/**/     } 
	 else 
	 {
/**/         return 0;
/**/     }
/**/ }

/**/ int  Solver::dfs_removable(Lit p, uint32_t abstract_level) 
{
/**/     int pseen = seen[var(p)];
/**/     assert((pseen & (2|4|8)) == 0);
/**/     int maykeep = pseen & (1);
/**/     int pstatus;
/**/     pstatus = quick_keeper(p, abstract_level, maykeep);
/**/     if (pstatus) 
	 {
/**/         seen[var(p)] |= (char) pstatus;
/**/         if (pseen == 0) 
		{
			analyze_toclear.push(p);
		}
/**/         return 0;
/**/     }
/**/ 
/**/     int found_some;
/**/     found_some = 0;
/**/     pstatus = 4;
/**/     Clause& rp = *reason[var(p)];
/**/     int sz = rp.size();
/**/     int i;
/**/     // rp[0] is p.  The rest of rp are predecessors of p.
/**/     for (i = 1; i < sz; i++) 
	 {
/**/         Lit q = rp[i];
/**/         if (level[var(q)] <= 0)
		{
/**/             continue;
		}
/**/ 
/**/         if ((seen[var(q)] & (2|4|8)) == 0) 
		{
/**/             found_some |= dfs_removable(q, abstract_level);
/**/         	}
/**/         int qseen = seen[var(q)];
/**/         if (qseen & (8)) 
		{
/**/             pstatus = (maykeep ? 2 : 8);
/**/             break;
/**/          	}
/**/          assert((qseen & (2|4)));
/**/     }
/**/     seen[var(p)] |= (char) pstatus;
/**/     if (pseen == 0) 
	 {
		analyze_toclear.push(p);
	 }
/**/     found_some |= maykeep;
/**/     return found_some;
/**/ }

/**/ void  Solver::mark_needed_removable(Lit p) 
{
/**/     Clause& rp = *reason[var(p)];
/**/     for (int i = 1; i < rp.size(); i++)
	 {
/**/         Lit q  = rp[i];
/**/         if (level[var(q)] <= 0)
		{
/**/             continue;
		}
/**/ 
/**/         int qseen = seen[var(q)];
/**/         if ((qseen & (1)) == 0 && reason[var(q) ] != NULL) 
		{
/**/                 seen[var(q)] |= 1;
/**/                 if (qseen == 0) 
			{
				analyze_toclear.push(q);
			}
/**/         	}
/**/     }
/**/     return;
/**/ }
