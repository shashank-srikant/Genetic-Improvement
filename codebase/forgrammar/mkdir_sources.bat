
#!/bin/tcsh
#WBL 25 April 2012 $Revision: 1.1 $

mkdir sources_$1
cd sources_$1

#use ln -s for file which are only ever read

rm ../sources/simp/*_E*
rm ../sources/core/*_E*
rm ../sources/utils/*_E*

mkdir mtl
mkdir utils
mkdir simp
mkdir core

cp -p ../sources/mtl/*.h mtl/
cp -p ../sources/utils/*.h utils/
cp -p ../sources/simp/*.h simp/
cp -p ../sources/core/*.h core/

cp -p ../sources/simp/*.cc simp/
cp -p ../sources/core/*.cc core/
cp -p ../sources/utils/*.cc utils/

cp -p ../sources/mtl/template.mk mtl/
cp -p ../sources/mtl/config.mk mtl/
cp -p ../sources/simp/Makefile simp/
cp -p ../sources/core/Makefile core/
cp -p ../sources/utils/Makefile utils/

rm core/Solver.cc
