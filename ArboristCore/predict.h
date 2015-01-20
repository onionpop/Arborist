// This file is part of ArboristCore.

/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef ARBORIST_PREDICT_H
#define ARBORIST_PREDICT_H

// Quantile signature.
//
class QuantSig {
 public:
  static void DeFactory();
  static void Factory(double *_qVec, int _qCells, double *_qPred);
  static int qCells;
  static double *qVec;
  static double *qPred;
};

// Interface class for front end.
//
class Predict {
  static void Finish(double predGini[]);
public:
  static void ForestReload(int _nTree, int _forestSize, int _preds[], double _splits[], double _scores[], int _bump[], int _origins[], int _facOff[], int _facSplits[]);
  static void ForestReloadQuant(double qYRanked[], int qYLen, int qRankOrigin[], int qRank[], int qRankCount[], int qLeafPos[], int qLeafExtent[]);
  static void PredictOOBQuant(double *err, double quantVec[], int qCells, double qPred[], double predGini[]);
  static void PredictOOBReg(double *err, double predGini[]);
  static void PredictOOBCtg(int conf[], double *error, double predGini[]);
  static void PredictQuant(double quantVec[], const int qcells, double qPred[], double y[]);
  static void PredictReg(double y[]);
  static void PredictCtg(int y[], int ctgWidth);
};
#endif