#/usr/bin/bash

# Builds a CRAN-style temporary directory tree suitable for package
#construction, then archives, then deletes the source tree.

mkdir Rborist
cd Rborist; mkdir src; mkdir R; mkdir man; mkdir inst; cd ..
cp ../LICENSE Rborist/
cp ../FrontEnd/DESCRIPTION Rborist/
cp ../FrontEnd/NAMESPACE Rborist/
cp ../FrontEnd/*.Rd Rborist/man/
cp ../FrontEnd/NEWS Rborist/inst/
cp ../FrontEnd/*R Rborist/R/
cp ../Shared/Makevars Rborist/src/
cp ../Shared/?[^cpp]*.cc Rborist/src/
cp ../Shared/rcppArmadillo*.cc Rborist/src
cat ../Shared/rcppMonolithHeader ../Shared/rcpp[^Armadillo]*.cc > Rborist/src/rcppMonolith.cc
cp ../Shared/*.h Rborist/src/
cp ../../ArboristCore/*.cc Rborist/src/
cp ../../ArboristCore/*.h Rborist/src/
cp -r ../tests Rborist
R CMD build Rborist
rm -rf Rborist

