#ifndef REGION_COMPARER__H
#define REGION_COMPARER__H

#include "birdbrain.h"

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
