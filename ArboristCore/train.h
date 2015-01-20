// This file is part of ArboristCore.

/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef ARBORIST_TRAIN_H
#define ARBORIST_TRAIN_H

// Interface class for front end.
// Holds simulation-specific parameters of the data.
//
class Train {
  static const int accumExp = 5;
  static int reLevel; // Diagnostic:  # reallocations.
public:
  static int nTree;
  static int nSamp;
  static int levelMax;
  static double probCutoff;
  static bool doQuantiles;
  static int qCells;
  static double *qVec;
  static double minRatio; // Spread between parent and child information content.
  static int blockSize;
  static int accumRealloc;
  static int probResize;
  static double *sCDF;
  static int *cdfOff;
  static void IntBlock(int xBlock[], int _nrow, int _ncol);
  static void ResponseReg(double y[]);
  static int ResponseCtg(const int y[], double yPerturb[]);
  static int Training(int minH, int *facWidth, int *totBagCount, int *totQLeafWidth, int totLevels);
  static void Factory(int _nTree, bool _quantiles, double _minRatio, int _blockSize);
  static void DeFactory();
  static int ReFactory();
  static inline double MinInfo(double info) {
    return minRatio * info;
  }
  static void SampleWeights(double sWeight[]);
  static void WriteForest(int *rPreds, double *rSplits, double * rScores, int *rBump, int* rOrigins, int *rFacOff, int * rFacSplits);
  static void WriteQuantile(double rQYRanked[], int rQRankOrigin[], int rQRank[], int rQRankCount[], int rQLeafPos[], int rQLeafExtent[]);
  static void Quantiles(double *_qVec, const int _qCells);
};
#endif