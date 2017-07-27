//clean_cpp.awk Revision: 1.2  "../core/Solver.C" Thu Oct 24 17:54:32 BST 2013


























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
    for (int i = 0; i < learnts.size(); i++)
    {
  free(learnts[i]);
    }
    for (int i = 0; i < clauses.size(); i++)
 {
  free(clauses[i]);
 }
}











Var Solver::newVar(bool sign, bool dvar)
{
    int v = nVars();
    watches .push();
    watches .push();
    reason .push(__null);
    assigns .push(toInt(l_Undef));
    level .push(-1);
    activity .push(0);
    seen .push(0);

    polarity .push(choosesign((char)sign));

    decision_var.push((char)dvar);

    insertVarOrder(v);
    return v;
}


bool Solver::addClause(vec<Lit>& ps)
{
    ((decisionLevel() == 0) ? static_cast<void> (0) : __assert_fail ("decisionLevel() == 0", "../core/Solver.C", 120, __PRETTY_FUNCTION__));

    if (!ok)
 {
        return false;
 }
    else
 {

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
        ((value(ps[0]) == l_Undef) ? static_cast<void> (0) : __assert_fail ("value(ps[0]) == l_Undef", "../core/Solver.C", 151, __PRETTY_FUNCTION__));
        uncheckedEnqueue(ps[0]);
        return ok = (propagate() == __null);
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
    ((c.size() > 1) ? static_cast<void> (0) : __assert_fail ("c.size() > 1", "../core/Solver.C", 168, __PRETTY_FUNCTION__));
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
    ((c.size() > 1) ? static_cast<void> (0) : __assert_fail ("c.size() > 1", "../core/Solver.C", 184, __PRETTY_FUNCTION__));
    ((find(watches[toInt(~c[0])], &c)) ? static_cast<void> (0) : __assert_fail ("find(watches[toInt(~c[0])], &c)", "../core/Solver.C", 185, __PRETTY_FUNCTION__));
    ((find(watches[toInt(~c[1])], &c)) ? static_cast<void> (0) : __assert_fail ("find(watches[toInt(~c[1])], &c)", "../core/Solver.C", 186, __PRETTY_FUNCTION__));
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




void Solver::cancelUntil(int level)
{
    if (decisionLevel() > level)
 {
        for (int c = trail.size()-1; c >= trail_lim[level]; c--)
  {
            Var x = var(trail[c]);
            assigns[x] = toInt(l_Undef);
            insertVarOrder(x);
  }
        qhead = trail_lim[level];
        trail.shrink(trail.size() - trail_lim[level]);
        trail_lim.shrink(trail_lim.size() - level);
    }
}






Lit Solver::pickBranchLit(int polarity_mode, double random_var_freq)
{
    Var next = (-1);


    if (drand(random_seed) < random_var_freq && !order_heap.empty())
 {
        next = order_heap[irand(random_seed,order_heap.size())];
        if (toLbool(assigns[next]) == l_Undef && decision_var[next])
  {
            rnd_decisions++;
  }
 }


    while (next == (-1) || toLbool(assigns[next]) != l_Undef || !decision_var[next])
 {
        if (order_heap.empty())
  {
            next = (-1);
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
    case polarity_true: sign = false; break;
    case polarity_false: sign = true; break;
    case polarity_user: sign = polarity[next]; break;
    case polarity_rnd: sign = irand(random_seed, 2); break;
    default: ((false) ? static_cast<void> (0) : __assert_fail ("false", "../core/Solver.C", 278, __PRETTY_FUNCTION__));
 }

    return next == (-1) ? lit_Undef : Lit(next, sign);
}



















void Solver::analyze(Clause* confl, vec<Lit>& out_learnt, int& out_btlevel)
{
    int pathC = 0;
    Lit p = lit_Undef;



    out_learnt.push();
    int index = trail.size() - 1;
    out_btlevel = 0;

    do
 {
        ((confl != __null) ? static_cast<void> (0) : __assert_fail ("confl != __null", "../core/Solver.C", 315, __PRETTY_FUNCTION__));
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


        while (!seen[var(trail[index--])]);
        p = trail[index+1];
        confl = reason[var(p)];
        seen[var(p)] = 0;
        pathC--;

    }
 while (pathC > 0);
    out_learnt[0] = ~p;



    int i, j;
    if (expensive_ccmin)
 {
        uint64_t abstract_level = 0;
        for (i = 1; i < out_learnt.size(); i++)
  {
            abstract_level |= abstractLevel(var(out_learnt[i]));
  }

        out_learnt.copyTo(analyze_toclear);

 if (1)
 {
        for (i = j = 1; i < out_learnt.size(); i++)
  {
            if (reason[var(out_learnt[i])] == __null || !litRedundant(out_learnt[i], abstract_level))
   {
                out_learnt[j++] = out_learnt[i];
   }
  }
 }
 else
 {
          i = out_learnt.size();
          int found_some = find_removable(out_learnt, i, abstract_level);
          if (found_some)
   {
              j = prune_removable(out_learnt);
   }
          else
   {
              j = i;
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
             if (reason[var(out_learnt[i])] == __null)
   {
                     out_learnt[j++] = out_learnt[i];
           continue;
   }
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
      out_learnt.shrink(out_learnt.size() - j);
 }
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
            if (level[var(out_learnt[i])] > level[var(out_learnt[max_i])])
   {
                max_i = i;
   }
  }
        Lit p = out_learnt[max_i];
        out_learnt[max_i] = out_learnt[1];
        out_learnt[1] = p;
        out_btlevel = level[var(p)];
    }


    for (int j = 0; j < analyze_toclear.size(); j++)
 {
  seen[var(analyze_toclear[j])] = 0;
 }
}




bool Solver::litRedundant(Lit p, uint64_t abstract_levels)
{
    analyze_stack.clear(); analyze_stack.push(p);
    int top = analyze_toclear.size();
    while (analyze_stack.size() > 0)
 {
        ((reason[var(analyze_stack.last())] != __null) ? static_cast<void> (0) : __assert_fail ("reason[var(analyze_stack.last())] != __null", "../core/Solver.C", 468, __PRETTY_FUNCTION__));
        Clause& c = *reason[var(analyze_stack.last())]; analyze_stack.pop();

        for (int i = 1; i < c.size(); i++)
  {
            Lit p = c[i];
            if (!seen[var(p)] && level[var(p)] > 0)
   {
                if (reason[var(p)] != __null && (abstractLevel(var(p)) & abstract_levels) != 0)
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
            if (reason[x] == __null)
   {
                ((level[x] > 0) ? static_cast<void> (0) : __assert_fail ("level[x] > 0", "../core/Solver.C", 527, __PRETTY_FUNCTION__));
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
    ((value(p) == l_Undef) ? static_cast<void> (0) : __assert_fail ("value(p) == l_Undef", "../core/Solver.C", 551, __PRETTY_FUNCTION__));
    assigns [var(p)] = toInt(lbool(!sign(p)));
    level [var(p)] = decisionLevel();
    reason [var(p)] = from;
    if (0)
    {
        polarity[var(p)] = sign(p);
    }
    trail.push(p);
}













Clause* Solver::propagate()
{
    Clause* confl = __null;
    int num_props = 0;

    while (qhead < trail.size())
 {
        Lit p = trail[qhead++];
        vec<Clause*>& ws = watches[toInt(p)];
        Clause **i, **j, **end;
        num_props++;

        for (i = j = (Clause**)ws, end = i + ws.size(); i != end;)
  {
            Clause& c = **i++;


            Lit false_lit = ~p;
            if (c[0] == false_lit)
   {
                c[0] = c[1], c[1] = false_lit;
   }

            ((c[1] == false_lit) ? static_cast<void> (0) : __assert_fail ("c[1] == false_lit", "../core/Solver.C", 597, __PRETTY_FUNCTION__));


            Lit first = c[0];
            if (value(first) == l_True)
   {
                *j++ = &c;
            }
   else
   {

                for (int k = 2; k < c.size(); k++)
    {
                    if (value(c[k]) != l_False)
     {
                        c[1] = c[k]; c[k] = false_lit;
                        watches[toInt(~c[1])].push(&c);
                        goto FoundWatch;
     }
    }


                *j++ = &c;
                if (value(first) == l_False)
    {
                    confl = &c;
                    qhead = trail.size();

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









struct reduceDB_lt { bool operator () (Clause* x, Clause* y) { return x->size() > 2 && (y->size() == 2 || x->activity() < y->activity()); } };
void Solver::reduceDB()
{
    int i, j;
    double extra_lim = cla_inc / learnts.size();

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
        nof_learnts *= learntsize_inc;
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










bool Solver::simplify()
{
    ((decisionLevel() == 0) ? static_cast<void> (0) : __assert_fail ("decisionLevel() == 0", "../core/Solver.C", 718, __PRETTY_FUNCTION__));

    if (!ok || propagate() != __null)
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


    order_heap.filter(VarFilter(*this));

    simpDB_assigns = nAssigns();
    simpDB_props = clauses_literals + learnts_literals;

    return true;
}
















lbool Solver::search(int nof_conflicts, int nof_learnts)
{
    ((ok) ? static_cast<void> (0) : __assert_fail ("ok", "../core/Solver.C", 763, __PRETTY_FUNCTION__));
    int backtrack_level;
    int conflictC = 0;
    vec<Lit> learnt_clause;

    starts++;

    bool first = true;

    for (;;)
 {
        Clause* confl = propagate();
        if (confl != __null)
  {

            conflicts++; conflictC++;
            if (decisionLevel() == 0) return l_False;

            first = false;

            learnt_clause.clear();
            analyze(confl, learnt_clause, backtrack_level);
            cancelUntil(backtrack_level);
            ((value(learnt_clause[0]) == l_Undef) ? static_cast<void> (0) : __assert_fail ("value(learnt_clause[0]) == l_Undef", "../core/Solver.C", 786, __PRETTY_FUNCTION__));

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


      if (0)
   {
       if (conflictC >= restartMore)
    {
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
     {
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

     progress_estimate = progressEstimate();
     cancelUntil(0);
     return l_Undef;
    }
      }


            if (decisionLevel() == 0 && !simplify())
   {
                return l_False;
   }

            if (nof_learnts >= 0 && learnts.size()-nAssigns() >= nof_learnts)
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
                next = pickBranchLit(polarity_mode, random_var_freq);

                if (next == lit_Undef)
    {

                    return l_True;
    }
            }


            ((value(next) == l_Undef) ? static_cast<void> (0) : __assert_fail ("value(next) == l_Undef", "../core/Solver.C", 902, __PRETTY_FUNCTION__));
            newDecisionLevel();
            uncheckedEnqueue(next);
        }
    }
}


double Solver::progressEstimate() const
{
    double progress = 0;
    double F = 1.0 / nVars();

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


    double nof_conflicts = restart_first;
    double nof_learnts = nClauses() * learntsize_factor;
    if (0)
    {
  double cvr= (double)nClauses() / (double)nVars();
        nof_learnts= 300000 / cvr;
    }
 restartLess= 5;
 restartMore= 42;
 restartTolerance= nVars() / 10000 +10;
 backtrackLevels= new int[restartMore];

    lbool status = l_Undef;









    while (status == l_Undef)
 {


        status = search((int)nof_conflicts, (int)nof_learnts);
        nof_conflicts *= restart_inc;
        nof_learnts *= learntsize_inc;
    }





    if (status == l_True)
 {

        model.growTo(nVars());
        for (int i = 0; i < nVars(); i++)
  {
   model[i] = value(i);
  }

        verifyModel();

    }
 else
 {
        ((status == l_False) ? static_cast<void> (0) : __assert_fail ("status == l_False", "../core/Solver.C", 988, __PRETTY_FUNCTION__));
        if (conflict.size() == 0)
  {
            ok = false;
  }
    }

    cancelUntil(0);
    return status == l_True;
}





void Solver::verifyModel()
{
    bool failed = false;
    for (int i = 0; i < clauses.size(); i++)
 {
        ((clauses[i]->mark() == 0) ? static_cast<void> (0) : __assert_fail ("clauses[i]->mark() == 0", "../core/Solver.C", 1008, __PRETTY_FUNCTION__));
        Clause& c = *clauses[i];
        for (int j = 0; j < c.size(); j++)
  {
            if (modelValue(c[j]) == l_True)
   {
                goto next;
   }
  }




        failed = true;
    next:;
    }

    ((!failed) ? static_cast<void> (0) : __assert_fail ("!failed", "../core/Solver.C", 1025, __PRETTY_FUNCTION__));


}


void Solver::checkLiteralCount()
{

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

        (((int)clauses_literals == cnt) ? static_cast<void> (0) : __assert_fail ("(int)clauses_literals == cnt", "../core/Solver.C", 1046, __PRETTY_FUNCTION__));
    }
}



     int Solver::prune_removable(vec<Lit>& out_learnt)
{
         int i, j, sz = out_learnt.size();
         j = 1;
         for (i = 1; i < sz; i++)
  {
             if ((seen[var(out_learnt[i])] & (1|2)) == (1|2))
  {
                 (((seen[var(out_learnt[i])] & (4|8)) == 0) ? static_cast<void> (0) : __assert_fail ("(seen[var(out_learnt[i])] & (4|8)) == 0", "../core/Solver.C", 1060, __PRETTY_FUNCTION__));
                 out_learnt[j++] = out_learnt[i];
             }
         }
         return j;
     }

     int Solver::find_removable(vec<Lit>& out_learnt, uint32_t sz0, uint32_t abstract_level)
{
         int found_some;
         found_some = 0;
         int sz = out_learnt.size();
         int i;






         for (i = 1; i < sz; i++)
  {
             Lit curLit = out_learnt[i];
             if (level[var(curLit)] <= 0)
  {
                 continue;
  }

             if ((seen[var(curLit)] & (2|4|8)) == 0)
  {
                 found_some |= dfs_removable(curLit, abstract_level);
              }
         }
         return found_some;
     }

     int Solver::quick_keeper(Lit p, uint64_t abstract_level, int maykeep)
{


         if (reason[var(p)] == __null)
  {
             return (maykeep ? 2 : 8);
         }
  else if ((abstractLevel(var(p)) & abstract_level) == 0)
  {
             ((maykeep == 0) ? static_cast<void> (0) : __assert_fail ("maykeep == 0", "../core/Solver.C", 1105, __PRETTY_FUNCTION__));
             return 8;
         }
  else
  {
             return 0;
         }
     }

     int Solver::dfs_removable(Lit p, uint32_t abstract_level)
{
         int pseen = seen[var(p)];
         (((pseen & (2|4|8)) == 0) ? static_cast<void> (0) : __assert_fail ("(pseen & (2|4|8)) == 0", "../core/Solver.C", 1117, __PRETTY_FUNCTION__));
         int maykeep = pseen & (1);
         int pstatus;
         pstatus = quick_keeper(p, abstract_level, maykeep);
         if (pstatus)
  {
             seen[var(p)] |= (char) pstatus;
             if (pseen == 0)
  {
   analyze_toclear.push(p);
  }
             return 0;
         }

         int found_some;
         found_some = 0;
         pstatus = 4;
         Clause& rp = *reason[var(p)];
         int sz = rp.size();
         int i;

         for (i = 1; i < sz; i++)
  {
             Lit q = rp[i];
             if (level[var(q)] <= 0)
  {
                 continue;
  }

             if ((seen[var(q)] & (2|4|8)) == 0)
  {
                 found_some |= dfs_removable(q, abstract_level);
              }
             int qseen = seen[var(q)];
             if (qseen & (8))
  {
                 pstatus = (maykeep ? 2 : 8);
                 break;
               }
              (((qseen & (2|4))) ? static_cast<void> (0) : __assert_fail ("(qseen & (2|4))", "../core/Solver.C", 1156, __PRETTY_FUNCTION__));
         }
         seen[var(p)] |= (char) pstatus;
         if (pseen == 0)
  {
  analyze_toclear.push(p);
  }
         found_some |= maykeep;
         return found_some;
     }

     void Solver::mark_needed_removable(Lit p)
{
         Clause& rp = *reason[var(p)];
         for (int i = 1; i < rp.size(); i++)
  {
             Lit q = rp[i];
             if (level[var(q)] <= 0)
  {
                 continue;
  }

             int qseen = seen[var(q)];
             if ((qseen & (1)) == 0 && reason[var(q) ] != __null)
  {
                     seen[var(q)] |= 1;
                     if (qseen == 0)
   {
    analyze_toclear.push(q);
   }
              }
         }
         return;
     }
