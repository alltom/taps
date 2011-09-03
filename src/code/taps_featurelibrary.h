/*----------------------------------------------------------------------------
    TAPESTREA: Techniques And Paradigms for Expressive Synthesis, 
               Transformation, and Rendering of Environmental Audio
      Engine and User Interface

    Copyright (c) 2006 Ananya Misra, Perry R. Cook, and Ge Wang.
      http://taps.cs.princeton.edu/
      http://soundlab.cs.princeton.edu/

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
    U.S.A.
-----------------------------------------------------------------------------*/

//-----------------------------------------------------------------------------
// file: taps_featurelibrary.h
// desc: taps feature library
//
// author: Matt Hoffman (mdhoffma@cs.princeton.edu)
//         Ananya Misra (amisra@cs.princeton.edu)
//         Ge Wang (gewang@cs.princeton.edu)
//         Perry R. Cook (prc@cs.princeton.edu)
// date: Spring 2006
//-----------------------------------------------------------------------------
#ifndef FEATURE_LIB__H
#define FEATURE_LIB__H

#include <vector>

class FeatureLibrary
{
 protected:
  int numFeats;
  std::vector<char*> featureNames;
  std::vector<float*> featureVals;

  float *weights;

  std::vector<char*> fileNames;
  std::vector<bool> tapsFile;

  float distance(const float *v1, const float *v2);

 public:
  FeatureLibrary(char *libName);
  ~FeatureLibrary();

  void addFile(const char *filename, const float *features,
           bool isTemplate = 0);
  void setWeights(float *newWeights);
  
  int size() { return featureVals.size(); }
  int getNumFeats() { return numFeats; }

  const char* getFeatureName(int featnum) { return featureNames[featnum]; }
  const float* getFeatureVec(int filenum) { return featureVals[filenum]; }
  const char* getFileName(int filenum) { return fileNames[filenum]; }
  const bool isTemplate(int filenum) { return tapsFile[filenum]; }

  void rankClosestFiles(const float *targetVals, int *indices, 
            float *distances = 0);

  void writeFLIFile(const char *filename);
};

#endif
