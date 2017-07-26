
#!/bin/tcsh

gawk -f create_syntax.awk \
	Solver_E.cc \
    > Solver.bnf

