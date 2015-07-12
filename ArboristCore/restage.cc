// This file is part of ArboristCore.

/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/**
   @file restage.cc

   @brief Methods maintaining the per-predictor ordering of sample information.

   @author Mark Seligman
 */

#include "samplepred.h"
#include "restage.h"
#include "index.h"
#include "splitpred.h"
#include "predictor.h"

//include <iostream>
using namespace std;

int RestageMap::nPred = 0;
int RestageMap::nSamp = 0;

void RestageMap::Immutables(int _nPred, int _nSamp) {
  nPred = _nPred;
  nSamp = _nSamp;
}

void RestageMap::DeImmutables() {
  nPred = 0;
}


/**
   @brief Class constructor.

   @param bagCount enables sizing of predicate bit vectors.

   @param splitCount specifies the number of splits to map.
 */
RestageMap::RestageMap(SplitPred *_splitPred, unsigned int _bagCount, int _splitPrev, int _splitNext) : splitPrev(_splitPrev), splitNext(_splitNext), splitPred(_splitPred) {
  mapNode = new MapNode[splitPrev];
  unsigned int slotBits = 8 * sizeof(unsigned int);
  bitSlots = (_bagCount + slotBits - 1) / slotBits;
  sIdxLH = new unsigned int[bitSlots];
  sIdxRH = new unsigned int[bitSlots];
}


/**
   @brief Class finalizer.

   @return void.
 */
RestageMap::~RestageMap() {
  delete [] mapNode;
  delete [] sIdxLH;
  delete [] sIdxRH;
}


/**
   @brief Consumes all fields in current NodeCache item relevant to restaging.

   @param _splitIdx is the split index.

   @param _lNext is the index node offset of the LHS in the next level.

   @param _rNext is the index node offset of the RHS in the next level.

   @param _lhIdxCount is the count of indices associated with the split's LHS.

   @param _rhIdxCount is the count of indices associated with the split's RHS.

   @return void.
*/
void RestageMap::ConsumeSplit(int _splitIdx, int _lNext, int _rNext, int _lhIdxCount, int _rhIdxCount, int _startIdx, int _endIdx) {
  mapNode[_splitIdx].Init(_splitIdx, _lNext, _rNext, _lhIdxCount, _rhIdxCount, _startIdx, _endIdx);
}

/**
  @brief Finishes setting of map fields.

  @param index caches state information for the predicate bits.

  @param _splitPrev is the number of splitable nodes in the previous level.

  @return void.
*/
void RestageMap::Conclude(const Index *index) {
  endPrev = mapNode[splitPrev-1].EndIdx(); // Terminus of PREVIOUS level.

  int rhIdxTot, lhIdxTot;
  index->PredicateBits(sIdxLH, sIdxRH, lhIdxTot, rhIdxTot);
  rhIdxNext = lhIdxTot;
  endThis = rhIdxTot + lhIdxTot - 1;
}


void RestageMap::RestageLevel(SamplePred *samplePred, int level) {
  int predIdx;
  SPNode *source, *targ;
  unsigned int *sIdxSource, *sIdxTarg;

#pragma omp parallel default(shared) private(predIdx, source, sIdxSource, targ, sIdxTarg)
  {
#pragma omp for schedule(dynamic, 1)
    for (predIdx = 0; predIdx < nPred; predIdx++) {
      samplePred->Buffers(predIdx, level, source, sIdxSource, targ, sIdxTarg);
      RestagePred(source, sIdxSource, targ, sIdxTarg, predIdx);
    }
  }
}


/**
   @brief Walks the live split indices for a predictor and either restages or propagates runs.

   @param predIdx is the predictor being restaged.

   @return void.
 */
void RestageMap::RestagePred(const SPNode source[], const unsigned int sIdxSource[], SPNode targ[], unsigned int sIdxTarg[], int predIdx) const {
  int lhIdx = 0;
  int rhIdx = rhIdxNext;
  for (int splitIdx = 0; splitIdx < splitPrev; splitIdx++) {
    // Runs need not be restaged, but lh, rh index positions should be
    // updated for uniformity across predictors.  Hence the data in
    // unrestaged SamplePreds is dirty.
    //
    MapNode *mn = &mapNode[splitIdx];
    if (!splitPred->Singleton(splitIdx, predIdx)) {
      mn->Restage(source, sIdxSource, targ, sIdxTarg, sIdxLH, sIdxRH, lhIdx, rhIdx);
      mn->Singletons(splitPred, targ, predIdx, lhIdx, rhIdx);
    }
    mn->UpdateIndices(lhIdx, rhIdx);
  }
}


/**
   @brief Advises SplitPred of any singletons arising as a result of this
   restaging.

   @param splitPred is the current SplitPred object.

   @param targ is the restaged data.

   @param predIdx is the predictor index.

   @param lhIdx is the starting index of the left successor.

   @param rhIdx is the starting index of the right successor.

   @return void.
 */
void MapNode::Singletons(SplitPred *splitPred, const SPNode targ[], int predIdx, int lhIdx, int rhIdx) {
  if (lNext >= 0 && targ->IsRun(lhIdx, lhIdx + lhIdxCount - 1)) {
      splitPred->LengthNext(lNext, predIdx) = 1;
  }
  if (rNext >= 0 && targ->IsRun(rhIdx, rhIdx + rhIdxCount - 1)) {
      splitPred->LengthNext(rNext, predIdx) = 1;
  }
}


/**

  @return void.
 */
void MapNode::UpdateIndices(int &lhIdx, int &rhIdx) {
  lhIdx += (lNext >= 0 ? lhIdxCount : 0);
  rhIdx += (rNext >= 0 ? rhIdxCount : 0);
}


/**
   @brief Sends contents of previous level's SamplePreds to this level's descendents, via a stable partition.

   @param source contains the previous level's SamplePreds.

   @param targ outputs this level's SamplePreds.

   @param lhIdx is the index node offset for the LHS.

   @param rhIdx is the index node offset for the RHS.

   @return void, with output parameter vector.
*/
void MapNode::Restage(const SPNode source[], const unsigned int sIdxSource[], SPNode targ[], unsigned int sIdxTarg[], const unsigned int sIdxLH[], const unsigned int sIdxRH[], int lhIdx, int rhIdx) {

  if (lNext >= 0 && rNext >= 0) // Both subnodes nonterminal.
    RestageLR(source, sIdxSource, targ, sIdxTarg, startIdx, endIdx, sIdxLH, lhIdx, rhIdx);
  else if (lNext >= 0) // Only LH subnode nonterminal.
    RestageSingle(source, sIdxSource, targ, sIdxTarg, startIdx, endIdx, sIdxLH, lhIdx);
  else if (rNext >= 0) // Only RH subnode nonterminal.
    RestageSingle(source, sIdxSource, targ, sIdxTarg, startIdx, endIdx, sIdxRH, rhIdx);

  // Otherwise, either node is itself terminal or both subnodes are.
  
  // Post-conditions:  lhIdx = lhIdx in + lhIdxCount && rhIdx = rhIdx in + rhIdxCount
}

/**
   @brief Sends SamplePred contents to both LH and RH targets.

   @param source contains the previous level's SamplePred values.

   @param targ outputs the current level's SamplePred values.

   @param startIdx is the first index in the node being restaged.

   @param endIdx is the last index in the node being restaged.

   @param lhIdx is the index node offset of the LHS.

   @param rhIdx is the index node offset of the RHS.

   @return void.
 */
// Target nodes should all equal either lh or rh.
//
void MapNode::RestageLR(const SPNode source[], const unsigned int sIdxSource[], SPNode targ[], unsigned int sIdxTarg[], int startIdx, int endIdx, const unsigned int bvL[], int lhIdx, int rhIdx) {
  for (int i = startIdx; i <= endIdx; i++) {
    unsigned int sIdx = sIdxSource[i];
    int destIdx = IsSet(bvL, sIdx) ? lhIdx++ : rhIdx++;
    sIdxTarg[destIdx] = sIdx;
    targ[destIdx] = source[i];
  }
}

/**
   @brief Sends SamplePred contents to one of either LH or RH targets.

   @param source contains the previous level's SamplePred values.

   @param targ outputs the current level's SamplePred values.

   @param startIdx is the first index in the node being restaged.

   @param endIdx is the last index in the node being restaged.

   @param bv is the bit vector testing the given handedness.

   @param idx is the offset of the descendent index node.

   @return void.
 */
// Target nodes should all be either leaf or set in bv[].
void MapNode::RestageSingle(const SPNode source[], const unsigned int sIdxSource[], SPNode targ[], unsigned int sIdxTarg[], int startIdx, int endIdx, const unsigned int bv[], int idx) {
  for (int i = startIdx; i <= endIdx; i++) {
    unsigned int sIdx = sIdxSource[i];
    if (IsSet(bv, sIdx)) {
      int destIdx = idx++;
      sIdxTarg[destIdx] = sIdx;
      targ[destIdx] = source[i];
    }
  }
}