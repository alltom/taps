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
// file: taps_regioncomparer.cpp
// desc: taps region comparer
//
// author: Matt Hoffman (mdhoffma@cs.princeton.edu)
//         Ananya Misra (amisra@cs.princeton.edu)
//         Ge Wang (gewang@cs.princeton.edu)
//         Perry R. Cook (prc@cs.princeton.edu)
// date: Spring 2006
//-----------------------------------------------------------------------------
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#ifndef __USE_SNDFILE_PRECONF__
#include <sndfile.h>
#else
#include "util_sndfile.h"
#endif

#include "taps_regioncomparer.h"
#include "taps_sceptre.h"

const double hopsize = 0.5;

int frameCentroids(Frame &inFrame, int winsize, std::vector<float> &centroids)
{
  Frame window;
  window.alloc_waveform(winsize);

  int hoplen = (int)(((double)winsize) * hopsize);
  // the number of windows including possibly fractional last window
  int numwins = inFrame.wsize / hoplen + (inFrame.wsize % hoplen != 0);

  int i;
  for(i = 0; i < numwins; i++){
    int j, winindex = i * hoplen;
    if(i < numwins - 1)
      for(j = 0; j < winsize; j++)
      {
        if( j + winindex < inFrame.wsize ) // bug since winsize is bigger than hoplen, last-but-one win may go out of bounds
          window.waveform[j] = inFrame.waveform[j + winindex];
        else
          window.waveform[j] = 0;
      }
    else{
      for(j = winindex; j < inFrame.wsize; j++)
        window.waveform[j - winindex] = inFrame.waveform[j];
      for(j = inFrame.wsize - winindex; j < winsize; j++)
        window.waveform[j] = 0;
    }

    rfft(window.waveform, window.len, FFT_FORWARD);
    window.cmp2pol();

    float sumpow = 0.000001f;
    centroids.push_back(0);
    for(j = 0; j < window.len; j++){
      centroids[i] += window.pol[j].mag * (float)(j + 1);
      sumpow += window.pol[j].mag;
    }
    centroids[i] /= sumpow * (float)window.len;
    if(centroids[i] != centroids[i])
      centroids[i] = 0;
  }

  return centroids.size();
}

int power(Frame &inFrame, int winsize, std::vector<float> &powers)
{
  int i, j;

  int hoplen = (int)(((double)winsize) * hopsize);
  // the number of windows including possibly fractional last window
  int numwins = inFrame.wsize / hoplen + (inFrame.wsize % hoplen != 0);
  
  for(i = 0; i < numwins; i++){
    powers.push_back(0);
    int startpt = i * hoplen;
    int stoppt = (i + 1) * hoplen;
    if(stoppt > inFrame.wsize)
      stoppt = inFrame.wsize;

    for(j = startpt; j < stoppt; j++)
      powers[i] += inFrame.waveform[j] * inFrame.waveform[j];
    powers[i] /= ((double)winsize);
    powers[i] = sqrt(powers[i]);
  }

  return powers.size();
}

int spectralFlux(Frame &inFrame, int winsize, std::vector<float> &fluxes)
{
  int i, j;
  int hoplen = (int)(((double)winsize) * hopsize);
  // the number of windows including possibly fractional last window
  int numwins = inFrame.wsize / hoplen + (inFrame.wsize % hoplen != 0);
  
  Frame windows[2];
  float magsums[2];
  magsums[1] = 0.00001f;
  windows[0].alloc_waveform(winsize);
  windows[1].alloc_waveform(winsize);
  for(i = 0; i < winsize; i++)
    windows[1].waveform[i] = 0;
  windows[1].alloc_pol();

  for(i = 0; i < numwins; i++){
    fluxes.push_back(0);

    Frame &window = windows[i % 2];
    Frame &lastwindow = windows[(i + 1) % 2];
    float *magsum = &magsums[i % 2];
    float *lastmagsum = &magsums[(i + 1) % 2];
    *magsum = 0.00001f;
    int winindex = i * hoplen;
    if(i < numwins - 1)
      for(j = 0; j < winsize; j++)
      {
        if( j + winindex < inFrame.wsize )
          window.waveform[j] = inFrame.waveform[j + winindex];
        else
          window.waveform[j] = 0;
      }
    else{
      for(j = winindex; j < inFrame.wsize; j++)
        window.waveform[j - winindex] = inFrame.waveform[j];
      for(j = inFrame.wsize - winindex; j < winsize; j++)
        window.waveform[j] = 0;
    }
    
    rfft(window.waveform, window.len, FFT_FORWARD);
    window.cmp2pol();

    for(j = 0; j < window.len; j++)
      *magsum += window.pol[j].mag;
    for(j = 0; j < window.len; j++){
      float diff = window.pol[j].mag / *magsum - 
    lastwindow.pol[j].mag / *lastmagsum;
      fluxes[i] += diff * diff;
    }
    fluxes[i] = sqrt(fluxes[i]);
    if(fluxes[i] != fluxes[i])
      fluxes[i] = 0;
  }

  return fluxes.size();
}

float autocorr(long size, float *data, float *result)
{
    long i,j,k;
    float temp,norm;

    for (i=0;i<size/2;i++)      {
        result[i] = 0.0;
        for (j=0;j<size-i-1;j++)    {
            result[i] += data[i+j] * data[j];
        }
    }
    temp = result[0];
    j = (long) (size*0.02);
    while (result[j]<temp && j < size/2) {
        temp = result[j];
        j += 1;
    }
    temp = 0.0;
    for (i=j;i<size*0.5;i++) {
        if (result[i]>temp) {
        j = i;
        temp = result[i];
    }
    }
    norm = 1.0 / size;
    k = size/2;
    for (i=0;i<size/2;i++)
        result[i] *=  (k - i) * norm;
    if (result[j] == 0) 
      j = 0;
    else if ((result[j] / result[0]) < 0.4)
      j = 0;
    else if (j > size/4)
      j = 0;
    return (float) j;
}

int framePitches(Frame &inFrame, int winsize, std::vector<float> &pitches)
{
  int hoplen = (int)(((double)winsize) * hopsize);
  // the number of windows including possibly fractional last window
  int numwins = inFrame.wsize / hoplen + (inFrame.wsize % hoplen != 0);

  float * result = new float[winsize];
  for(int i = 0; i < numwins; i++) {
    int start = (i != numwins - 1) ? i * hoplen : inFrame.wsize - hoplen;
    float *samples = &inFrame.waveform[start];
    int size = (start + winsize <= inFrame.wsize) ? winsize : inFrame.wsize - start;

    long period = (long)autocorr(size, samples, result);
    float pitch = 0;
    float fperiod = (float)(2 * period);
    if(period > 0)
      pitch = BirdBrain::srate() / fperiod;
    else
      pitch = -1;

    pitches.push_back(pitch);
  }

  delete [] result;
  return pitches.size();
}

float varianceSum(std::vector<float> &means, 
          std::vector< std::vector<float>* > &features)
{
  int i, j;
  float result = 0;
  for(i = 0; i < features.size(); i++){
    std::vector<float> &pointFeatures = *features[i];
    for(j = 0; j < pointFeatures.size(); j++){
      float diff = (pointFeatures[j] - means[j]) / (2 * means[j]);
      result += diff * diff;
    }
  }
  result /= (float)(features.size() * means.size());
  return sqrt(result);
}

RegionComparer::RegionComparer(int _regionSize, int _winsize, Frame &inFrame)
{
  int i;
  winsize = _winsize;

  regFeats.push_back(&regCentMeans);
  regFeats.push_back(&regLowPowers);
  regFeats.push_back(&regFluxMeans);
  regFeats.push_back(&regPitchies);
  regFeats.push_back(&regVariances);
  for(i = 0; i < regFeats.size(); i++)
    weights.push_back(1);

  numwins = frameCentroids(inFrame, winsize, centroids);
  power(inFrame, winsize, powers);
  spectralFlux(inFrame, winsize, fluxes);
  framePitches(inFrame, winsize, pitches);

  setRegionSize(_regionSize);
}

void RegionComparer::setRegionSize(int _regionSize)
{
  int i, j;
  regionSize = _regionSize;

  numRegions = numwins - regionSize + 1;
  if(numRegions <= 0)
    return;

  float normFactor = 1 / (float)regionSize;
  float meanpower = 0;
  float meanpitch = 0;
  for(i = 0; i < regionSize; i++){
    meanpower += powers[i];
    meanpitch += pitches[i];
  }
  meanpower *= normFactor;
  meanpitch *= normFactor;

  regCentMeans.clear();
  regLowPowers.clear();
  regFluxMeans.clear();
  regPitchies.clear();
  regVariances.clear();
  regCentMeans.push_back(0);
  regLowPowers.push_back(0);
  regFluxMeans.push_back(0);
  regPitchies.push_back(0);
  for(i = 0; i < regionSize; i++){
    regCentMeans[0] += centroids[i];
    regLowPowers[0] += powers[i] > meanpower;
    regFluxMeans[0] += fluxes[i];
    regPitchies[0] += pitches[i] > 0;
  }
  regCentMeans[0] *= normFactor;
  regLowPowers[0] *= normFactor;
  regFluxMeans[0] *= normFactor;
  regPitchies[0] *= normFactor;

  for(i = 1; i < numRegions; i++){
    regCentMeans.push_back(regCentMeans[i - 1]);
    regLowPowers.push_back(0);
    regFluxMeans.push_back(regFluxMeans[i - 1]);
    regPitchies.push_back(regPitchies[i - 1]);

    meanpower += (powers[i + regionSize - 1] - powers[i - 1]) * normFactor;
    meanpitch += (pitches[i + regionSize - 1] - pitches[i - 1]) * normFactor;
    for(j = 0; j < regionSize; j++)
      regLowPowers[i] += powers[i + j] > meanpower;
    regLowPowers[i] *= normFactor;
    regCentMeans[i] += 
      (centroids[i + regionSize - 1] - centroids[i - 1]) * normFactor;
    regFluxMeans[i] +=
      (fluxes[i + regionSize - 1] - fluxes[i - 1]) * normFactor;
    regPitchies[i] += 
      ((pitches[i + regionSize - 1] > 0) - (pitches[i - 1] > 0)) * normFactor;

    std::vector< std::vector<float>* > pointFeatures;
    for(j = 0; j < regionSize; j++){
      std::vector<float> *pointFeat = new std::vector<float>;
      pointFeat->push_back(centroids[i + j]);
      pointFeat->push_back(powers[i + j]);
      pointFeat->push_back(fluxes[i + j]);
      pointFeat->push_back(pitches[i + j]);
      pointFeatures.push_back(pointFeat);
    }
    std::vector<float> means;
    means.push_back(regCentMeans[i]);
    means.push_back(meanpower);
    means.push_back(regFluxMeans[i]);
    means.push_back(meanpitch);

    regVariances.push_back(varianceSum(means, pointFeatures));

    for(j = 0; j < regionSize; j++)
      delete pointFeatures[j];
  }
}

void RegionComparer::setWeights(double *newWeights)
{
  for(int i = 0; i < weights.size(); i++)
    weights[i] = newWeights[i];
}

double RegionComparer::distance(int region1, int region2)
{
  double dist = 0;
  for(int i = 0; i < weights.size(); i++){
    double diff = weights[i] * 
      ((*regFeats[i])[region1] - (*regFeats[i])[region2]);
    dist += diff * diff;
  }
  return sqrt(dist);
}

double RegionComparer::distance(std::vector<float> &regFeats1, 
                std::vector<float> &regFeats2)
{
  double dist = 0;
  for(int i = 0; i < regFeats1.size(); i++){
    double diff = weights[i] * (regFeats1[i] - regFeats2[i]);
    dist += diff * diff;
  }
  return sqrt(dist);
}

void RegionComparer::getFeatures(Frame &inFrame, std::vector<float> &features,
                 int _winsize)
{
  int i;

  std::vector<float> localCentroids;
  std::vector<float> localPowers;
  std::vector<float> localFluxes;
  std::vector<float> localPitches;
  int numlocalwins = frameCentroids(inFrame, _winsize, localCentroids);
  power(inFrame, _winsize, localPowers);
  spectralFlux(inFrame, _winsize, localFluxes);
  framePitches(inFrame, _winsize, localPitches);
  
  float normFactor = 1 / (float)numlocalwins;
  float meanpower = 0;
  for(i = 0; i < numlocalwins; i++)
    meanpower += localPowers[i];
  meanpower *= normFactor;

  float meanpitch = 0;
  features.clear();
  for(i = 0; i < 4; i++)
    features.push_back(0);
  for(i = 0; i < numlocalwins; i++){
    features[0] += localCentroids[i];
    features[1] += localPowers[i] > meanpower;
    features[2] += localFluxes[i];
    features[3] += localPitches[i] > 0;
    meanpitch += localPitches[i];
  }
  meanpitch /= (float)numlocalwins;
  features[0] *= normFactor;
  features[1] *= normFactor;
  features[2] *= normFactor;
  features[3] *= normFactor;

  std::vector<float> means;
  std::vector< std::vector<float>* > pointFeatures;
  means.push_back(features[0]);
  means.push_back(meanpower);
  means.push_back(features[2]);
  means.push_back(meanpitch);
  for(i = 0; i < numlocalwins; i++){
    std::vector<float> *pointFeat = new std::vector<float>;
    pointFeat->push_back(localCentroids[i]);
    pointFeat->push_back(localPowers[i]);
    pointFeat->push_back(localFluxes[i]);
    pointFeat->push_back(localPitches[i]);
    pointFeatures.push_back(pointFeat);
  }
  
//   features.push_back(0);
//   for(i = 0; i < numlocalwins; i++){
//     float diff = (localCentroids[i] - features[0]) / (2 * features[0]);
//     features[4] += diff * diff;
//     diff = (localPowers[i] - meanpower) / (2 * meanpower);
//     features[4] += diff * diff;
//     diff = (localFluxes[i] - features[2]) / (2 * features[2]);
//     features[4] += diff * diff;
//     if(localPitches[i] > 0){
//       diff = (localPitches[i] - meanpitch) / (2 * meanpitch);
//       features[4] += diff * diff;
//     }
//   }
//   features[4] /= (float)(numlocalwins * 4);
//   features[4] = sqrt(features[4]);
  features.push_back(varianceSum(means, pointFeatures));

  for(i = 0; i < numlocalwins; i++)
    delete pointFeatures[i];
}

void cullToMinima(std::vector<int> &indices, std::vector<float> &distances)
{
  if(distances.size() < 2)
    return;

  std::vector<int> newindices;
  std::vector<float> newdistances;

  indices.push_back(999999);
  distances.push_back(999999.0);
  if((distances[0] < distances[1]) || (indices[1] - indices[0] > 1)){
    newindices.push_back(indices[0]);
    newdistances.push_back(distances[0]);
  }
  for(int i = 1; i < indices.size() - 1; i++){
    bool lessthanleft = (indices[i] - indices[i - 1] > 1) ||
      (distances[i] < distances[i - 1]);
    bool lessthanright = (indices[i + 1] - indices[i] > 1) ||
      (distances[i] < distances[i + 1]);
    if(lessthanleft && lessthanright){
      newindices.push_back(indices[i]);
      newdistances.push_back(distances[i]);
    }
  }

  indices = newindices;
  distances = newdistances;
}

void RegionComparer::findMatchingRegions(Frame &inFrame, 
                     std::vector<int> &indices,
                     float thresh, bool minimaOnly)
{
  int i;
  double *distances = new double[numRegions];
  std::vector<float> targetFeats;
  getFeatures(inFrame, targetFeats);

  int hoplen = (int)(hopsize * (double)winsize);
  std::vector<float> compareFeats(targetFeats.size());
  std::vector<float> matchDistances;
  for(i = 0; i < numRegions; i++){
    for(int j = 0; j < targetFeats.size(); j++){
      compareFeats[j] = (*regFeats[j])[i];
    }
    distances[i] = distance(targetFeats, compareFeats);
    if(distances[i] < thresh){
      indices.push_back(i);
      matchDistances.push_back(distances[i]);
    }
  }
  
  if(minimaOnly)
    cullToMinima(indices, matchDistances);

  for(i = 0; i < indices.size(); i++)
    indices[i] *= hoplen;

  delete[] distances;
}

void RegionComparer::findMatchingRegions(int regionNum, 
                     std::vector<int> &indices, 
                     float thresh, bool minimaOnly)
{
  int i;
  double *distances = new double[numRegions];
  int hoplen = (int)(hopsize * (double)winsize);
  std::vector<float> matchDistances;
  for(i = 0; i < numRegions; i++){
    distances[i] = distance(regionNum, i);
    if(distances[i] < thresh){
      indices.push_back(i);
      matchDistances.push_back(distances[i]);
    }
  }
  
  if(minimaOnly)
    cullToMinima(indices, matchDistances);

  for(i = 0; i < numRegions; i++)
    indices[i] *= hoplen;
}

// int main(int argc, char **argv)
// {
//   int i;

//   SF_INFO sfinfo;
//   SNDFILE *sfile = sf_open(argv[1], SFM_READ, &sfinfo);

//   float buffer[sfinfo.frames];
//   int read = sf_read_float(sfile, buffer, sfinfo.frames);

//   Frame buffFrame;
//   buffFrame.waveform = buffer;
//   buffFrame.wsize = read;
//   buffFrame.wlen = read;

// //   std::vector<float> features;
// //   RegionComparer::getFeatures(buffFrame, features);
// //   for(i = 0; i < features.size(); i++)
// //     printf("%f ", features[i]);

//   RegionComparer comparer(100, 512, buffFrame);
//   int regwidth = 6332;
//   int regsize = regwidth / 256;
//   std::vector<int> closestRegions;
// //   comparer.findMatchingRegions(0, closestRegions, 0.05);
// //   for(i = 0; i < closestRegions.size(); i++)
// //     printf("%d ", closestRegions[i]);
// //   printf("\n");

//   Frame subframe;
//   subframe.waveform = &buffer[0]; //40 * 256 + 23];
//   //printf("should get %d\n", 40 * 256);
//   subframe.wsize = regwidth; //23 * 256;
//   subframe.wlen = regwidth; //23 * 256;
//   for(i = 0; i < 5000; i += 1000)
//     printf("%f ", subframe.waveform[i]);
//   printf("\n");
// //   closestRegions.clear();
//   comparer.setRegionSize(regsize);
//   comparer.findMatchingRegions(subframe, closestRegions, 0.1, 1);
//   for(i = 0; i < closestRegions.size(); i++)
//     printf("%d ", closestRegions[i]);
//   printf("\n");

//   std::vector<float> features;
//   RegionComparer::getFeatures(subframe, features);
//   for(i = 0; i < features.size(); i++)
//     printf("%f ", features[i]);
//   printf("\n");

// //   std::vector<float> centroids;
// //   std::vector<float> rmspower;
// //   std::vector<float> fluxes;
// //   int numCentroids = frameCentroids(buffFrame, 512, centroids);
// //   int numPowers = power(buffFrame, 512, rmspower);
// //   int numFluxes = spectralFlux(buffFrame, 512, fluxes);
// //   for(i = 0; i < numFluxes; i++)
// //     printf("%f ", fluxes[i]);
// //   printf("\n");

//   buffFrame.waveform = 0;
//   subframe.waveform = 0;
  
//   return 0;
// }
