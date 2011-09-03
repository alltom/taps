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
// file: taps_regioncomparer.h
// desc: taps region comparer
//
// author: Matt Hoffman (mdhoffma@cs.princeton.edu)
//         Ananya Misra (amisra@cs.princeton.edu)
//         Ge Wang (gewang@cs.princeton.edu)
//         Perry R. Cook (prc@cs.princeton.edu)
// date: Spring 2006
//-----------------------------------------------------------------------------
#ifndef REGION_COMPARER__H
#define REGION_COMPARER__H

#include "taps_birdbrain.h"

#include <vector>

int frameCentroids(Frame &inFrame, int winsize, std::vector<float> &centroids);
int power(Frame &inFrame, int winsize, std::vector<float> &powers);
int spectralFlux(Frame &inFrame, int winsize, std::vector<float> &fluxes);

class RegionComparer
{
 protected:
  int regionSize;
  int winsize;
  int numRegions;
  int numwins;

  std::vector<float> weights;

  std::vector<float> centroids;
  std::vector<float> powers;
  std::vector<float> fluxes;
  std::vector<float> pitches;

  std::vector< std::vector<float>* > regFeats;
  std::vector<float> regCentMeans;
  std::vector<float> regLowPowers;
  std::vector<float> regFluxMeans;
  std::vector<float> regPitchies;
  std::vector<float> regVariances;

  double distance(int region1, int region2);
  double distance(std::vector<float> &regFeats1, 
          std::vector<float> &regFeats2);

 public:
  RegionComparer(int _regionSize, int _winsize, Frame &inFrame);
  void setRegionSize(int _regionSize);
  void setWeights(double *newWeights);

  // pass in a Frame, get out a vector of features describing it
  static void getFeatures(Frame &inFrame, std::vector<float> &features,
              int _winsize = 512);
  // pass in a Frame, get out the sample numbers where matching regions from 
  // the large Frame start
  void findMatchingRegions(Frame &inFrame, std::vector<int> &indices,
               float thresh, bool minimaOnly = 0);
  // given the index of a region, find the indices of all the other regions
  // that match it within threshold.
  void findMatchingRegions(int regionNum, std::vector<int> &indices,
               float thresh, bool minimaOnly = 0);
};

#endif
