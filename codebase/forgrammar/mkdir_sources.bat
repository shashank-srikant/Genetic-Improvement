
#!/bin/tcsh
#WBL 25 April 2012 $Revision: 1.1 $

mkdir sources_$1
cd sources_$1

#use ln -s for file which are only ever read

rm ../sources/simp/*_E*
rm ../sources/core/*_E*

mkdir mtl
mkdir simp
mkdir core

cp -p ../sources/mtl/*.h mtl/
cp -p ../sources/simp/*.h simp/
cp -p ../sources/core/*.h core/

cp -p ../sources/simp/*.C simp/
cp -p ../sources/core/*.C core/

cp -p ../sources/mtl/template.mk mtl/
cp -p ../sources/simp/Makefile simp/
cp -p ../sources/core/Makefile core/

rm core/Solver.C
