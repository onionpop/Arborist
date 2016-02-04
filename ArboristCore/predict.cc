// This file is part of ArboristCore.

/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/**
   @file predict.cc

   @brief Methods for validation and prediction.

   @author Mark Seligman
 */

#include "predblock.h"
#include "forest.h"
#include "predict.h"
#include "quant.h"
#include "bv.h"

//#include <iostream>
using namespace std;


/**
   @brief Static entry for regression case.
 */
void Predict::Regression(double *_blockNumT, int *_blockFacT, unsigned int _nRow, unsigned int _nPredNum, unsigned int _nPredFac, std::vector<unsigned int> &_pred, std::vector<double> &_split, std::vector<unsigned int> &_bump, std::vector<unsigned int> &_origin, const std::vector<unsigned int> &_facOff, const std::vector<unsigned int> &_facSplit, double _yPred[], const std::vector<unsigned int> &_bag) {
  int nTree = _origin.size();
  PBPredict::Immutables(_blockNumT, _blockFacT, _nPredNum, _nPredFac, _nRow);
  PredictReg *predictReg = new PredictReg(_nRow, nTree);
  Forest *forest =  new Forest(_pred, _split, _bump, _origin, _facOff, _facSplit);
  BitMatrix *bag = new BitMatrix(_nRow, nTree, _bag);
  forest->PredictAcross(predictReg->predictLeaves, bag);
  predictReg->Score(_yPred, forest);

  delete bag;
  delete predictReg;
  delete forest;
  PBPredict::DeImmutables();
}


/**
   @brief Static entry for regression case.
 */
void Predict::Quantiles(double *_blockNumT, int *_blockFacT, unsigned int _nRow, unsigned int _nPredNum, unsigned int _nPredFac, std::vector<unsigned int> &_pred, std::vector<double> &_split, std::vector<unsigned int> &_bump, std::vector<unsigned int> &_origin, const std::vector<unsigned int> &_facOff, const std::vector<unsigned int> &_facSplit, unsigned int _rank[], unsigned int _sCount[], double _yRanked[], double _yPred[], double _quantVec[], int _qCount, unsigned int _qBin, double _qPred[], const std::vector<unsigned int> &_bag) {
  int nTree = _origin.size();
  PBPredict::Immutables(_blockNumT, _blockFacT, _nPredNum, _nPredFac, _nRow);
  PredictReg *predictReg = new PredictReg(_nRow, nTree);
  Forest *forest =  new Forest(_pred, _split, _bump, _origin, _facOff, _facSplit);
  BitMatrix *bag = new BitMatrix(_nRow, nTree, _bag);
  forest->PredictAcross(predictReg->predictLeaves, bag);
  predictReg->Score(_yPred, forest);
  Quant::Predict(_nRow, forest, _yRanked, _rank, _sCount, _quantVec, _qCount, _qBin, predictReg->predictLeaves, _qPred);

  delete bag;
  delete predictReg;
  delete forest;
  
  PBPredict::DeImmutables();
}


/**
   @brief Entry for separate classification prediction.
 */
void Predict::Classification(double *_blockNumT, int *_blockFacT, unsigned int _nRow, unsigned int _nPredNum, unsigned int _nPredFac, std::vector<unsigned int> &_pred, std::vector<double> &_split, std::vector<unsigned int> &_bump, std::vector<unsigned int> &_origin, const std::vector<unsigned int> &_facOff, const std::vector<unsigned int> &_facSplit, unsigned int _ctgWidth, double *_leafWeight, int *_yPred, int *_census, int *_yTest, int *_conf, double *_error, double *_prob, const std::vector<unsigned int> &_bag) {
  int nTree = _origin.size();
  PBPredict::Immutables(_blockNumT, _blockFacT, _nPredNum, _nPredFac, _nRow);
  PredictCtg *predictCtg = new PredictCtg(_nRow, nTree, _ctgWidth, _leafWeight);
  Forest *forest = new Forest(_pred, _split, _bump, _origin, _facOff, _facSplit);
  BitMatrix *bag = new BitMatrix(_nRow, nTree, _bag);
  predictCtg->PredictAcross(forest, bag, _census, _yPred, _yTest, _conf, _error, _prob);
  delete predictCtg;
  delete forest;
  delete bag;
  PBPredict::DeImmutables();
}


PredictCtg::PredictCtg(unsigned int _nRow, int _nTree, unsigned int _ctgWidth, double *_leafWeight) : Predict(_nRow, _nTree), ctgWidth(_ctgWidth), leafWeight(_leafWeight) {
}


void PredictCtg::PredictAcross(Forest *forest, BitMatrix *bag, int *census, int *yPred, int *yTest, int *conf, double *error, double *prob) {
  forest->PredictAcross(predictLeaves, bag);
  double *votes = Score(forest);
  Vote(votes, census, yPred);
  delete [] votes;

  if (yTest != 0) {
    Validate(yTest, yPred, conf, error);
  }
  if (prob != 0)
    Prob(prob, forest);
}


PredictReg::PredictReg(unsigned int _nRow, int _nTree)  : Predict(_nRow, _nTree) {
}


Predict::Predict(unsigned int _nRow, int _nTree) : nRow(_nRow), nTree(_nTree) {
  predictLeaves = new int[_nRow * _nTree];
}


Predict::~Predict() {
  delete [] predictLeaves;
}


/**
   @brief Fills in confusion matrix and error vector.

   @param yCtg contains the training response.

   @param yPred is the predicted response.

   @param confusion is the confusion matrix.

   @param error outputs the classification errors.

   @return void.
*/
void PredictCtg::Validate(const int yCtg[], const int yPred[], int confusion[], double error[]) {
  for (unsigned int row = 0; row < nRow; row++) {
    confusion[ctgWidth * yCtg[row] + yPred[row]]++;
  }

  // Fills in classification error vector from off-diagonal confusion elements..
  //
  for (unsigned int rsp = 0; rsp < ctgWidth; rsp++) {
    int numWrong = 0;
    int numRight = 0;
    for (unsigned int predicted = 0; predicted < ctgWidth; predicted++) {
      if (predicted != rsp) {  // Mispredictions are off-diagonal.
        numWrong += confusion[ctgWidth * rsp + predicted];
      }
      else {
	numRight = confusion[ctgWidth * rsp + predicted];
      }
    }
    error[rsp] = double(numWrong) / double(numWrong + numRight);
  }
}

 
/**
   @brief Voting for non-bagged prediction.  Rounds jittered scores to category.

   @param predictLeaves are the predicted terminal indices.

   @param yCtg outputs predicted response.

   @return void, with output reference vector.
*/
void PredictCtg::Vote(double *votes, int census[], int yPred[]) {
  unsigned int row;

#pragma omp parallel default(shared) private(row)
  {
#pragma omp for schedule(dynamic, 1)
  for (row = 0; row < nRow; row++) {
    int argMax = -1;
    double scoreMax = 0.0;
    double *score = votes + row * ctgWidth;
    for (unsigned int ctg = 0; ctg < ctgWidth; ctg++) {
      double ctgScore = score[ctg]; // Jittered vote count.
      if (ctgScore > scoreMax) {
	scoreMax = ctgScore;
	argMax = ctg;
      }
      census[row * ctgWidth + ctg] = ctgScore; // De-jittered.
    }
    yPred[row] = argMax;
  }
  }
}


/**
   @brief Computes score from leaf predictions.

   @param predictLeaves are the predicted leaf indices.

   @return internal vote table, with output reference vector.
 */
double *PredictCtg::Score(const Forest *forest) {
  unsigned int row;
  double *votes = new double[nRow * ctgWidth];
  for (row = 0; row < nRow * ctgWidth; row++)
    votes[row] = 0.0;

  // TODO:  Recast loop by blocks, to avoid
  // false sharing.
#pragma omp parallel default(shared) private(row)
  {
#pragma omp for schedule(dynamic, 1)
  for (row = 0; row < nRow; row++) {
    int *leaves = predictLeaves + row * nTree;
    double *prediction = votes + row * ctgWidth;
    for (int tc = 0; tc < nTree; tc++) {
      int leafIdx = leaves[tc];
      if (leafIdx >= 0) {
	double val = forest->LeafVal(tc, leafIdx);
	unsigned int ctg = val; // Truncates jittered score for indexing.
	prediction[ctg] += 1 + val - ctg;
      }
    }
  }
  }

  return votes;
}


void PredictCtg::Prob(double *prob, const Forest *forest) {
  for (unsigned int row = 0; row < nRow; row++) {
    int *leafRow = predictLeaves + row * nTree;
    double *probRow = prob + row * ctgWidth;
    double rowSum = 0.0;
    for (int tc = 0; tc < nTree; tc++) {
      int leafIdx = leafRow[tc];
      if (leafIdx >= 0) {
        double *idxWeight = leafWeight + ctgWidth * forest->LeafPos(tc, leafIdx);
	for (unsigned int ctg = 0; ctg < ctgWidth; ctg++) {
	  probRow[ctg] += idxWeight[ctg];
	  rowSum += idxWeight[ctg];
	}
      }
    }
    double recipSum = 1.0 / rowSum;
    for (unsigned int ctg = 0; ctg < ctgWidth; ctg++)
      probRow[ctg] *= recipSum;
  }
}


/**
  @brief Sets regression scores from leaf predictions.

  @param predictLeaves holds the leaf predictions.

  @param yPred outputs the score predictions.

  @return void, with output refererence vector.
 */
void PredictReg::Score(double yPred[], const Forest *forest) {
  unsigned int row;

#pragma omp parallel default(shared) private(row)
  {
#pragma omp for schedule(dynamic, 1)
  for (row = 0; row < nRow; row++) {
    double score = 0.0;
    int treesSeen = 0;
    int *leaves = predictLeaves + row * nTree;
    for (int tc = 0; tc < nTree; tc++) {
      int leafIdx = leaves[tc];
      if (leafIdx >= 0) {
        treesSeen++;
        score += forest->LeafVal(tc, leafIdx);
      }
    }
    yPred[row] = score / treesSeen; // Assumes >= 1 tree seen.
  }
  }
}
