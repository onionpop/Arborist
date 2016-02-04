// Copyright (C)  2012-2016   Mark Seligman
//
// This file is part of ArboristBridgeR.
//
// ArboristBridgeR is free software: you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// ArboristBridgeR is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with ArboristBridgeR.  If not, see <http://www.gnu.org/licenses/>.

/**
   @file rcppForest.cc

   @brief C++ interface to R entry for Forest methods.

   @author Mark Seligman
 */

#include <R.h>
#include <Rcpp.h>

using namespace std;
using namespace Rcpp;

#include "rcppForest.h"

//#include <iostream>

RcppExport SEXP RcppForestWrap(const std::vector<unsigned int> &pred, const std::vector<double> &split, const std::vector<unsigned int> &bump, const std::vector<unsigned int> &origin, const std::vector<unsigned int> &facOrigin, const std::vector<unsigned int> &facSplit) {
  List forest = List::create(
     _["pred"] = pred,
     _["split"] = split,
     _["bump"] = bump,
     _["origin"] = origin,
     _["facOrig"] = facOrigin,
     _["facSplit"] = facSplit);
  forest.attr("class") = "Forest";

  return forest;
}

/**
   @brief Exposes front-end Forest fields for transmission to core.

   @return void.
 */
void RcppForestUnwrap(SEXP sForest, std::vector<unsigned int> &_pred, std::vector<double> &_split, std::vector<unsigned int> &_bump, std::vector<unsigned int> &_origin, std::vector<unsigned int> &_facOrig, std::vector<unsigned int> &_facSplit) {
  List forest(sForest);
  if (!forest.inherits("Forest"))
    stop("Expecting Forest");

  _pred = as<std::vector<unsigned int> >(forest["pred"]);
  _split = as<std::vector<double> >(forest["split"]);
  _bump = as<std::vector<unsigned int> >(forest["bump"]);
  _origin = as<std::vector<unsigned int> >(forest["origin"]);
  _facOrig = as<std::vector<unsigned int> >(forest["facOrig"]);
  _facSplit = as<std::vector<unsigned int> >(forest["facSplit"]);
}
