#include "FeatureLibrary.h"

#include <stdio.h>
#include <math.h>

#include <algorithm>

FeatureLibrary::FeatureLibrary(char *libName)
{
  int i;
  //char cbuff[4096];
  //sprintf(cbuff, "%s/library.fli", dirName);
  //FILE *libfile = fopen(cbuff, "r");
  FILE *libfile = fopen(libName, "r");

  fscanf(libfile, "%d\n", &numFeats);
  
  for(i = 0; i < numFeats; i++){
    char *featName = new char[128];
    fscanf(libfile, "%s", featName);
    featureNames.push_back(featName);
  }

  while(!feof(libfile)){
    char *filename = new char[256];
    if( fscanf(libfile, "%s", filename) != EOF )
    {
        fileNames.push_back(filename);
        bool hasTapsFile;
        fscanf(libfile, "%d", &hasTapsFile);
        tapsFile.push_back(hasTapsFile);
        float *fvec = new float[numFeats];
        for(i = 0; i < numFeats; i++)
          fscanf(libfile, "%f", &fvec[i]);
        featureVals.push_back(fvec);
    }
  }

  weights = new float[numFeats];
  for(i = 0; i < numFeats; i++)
    weights[i] = 1;

  fclose(libfile);
}

FeatureLibrary::~FeatureLibrary()
{
  int i;
  for(i = 0; i < featureNames.size(); i++)
    delete[] featureNames[i];
  for(i = 0; i < featureVals.size(); i++){
    delete[] featureVals[i];
    delete[] fileNames[i];
  }
  delete[] weights;
}

void FeatureLibrary::addFile(const char *filename, const float *features, 
			     bool isTemplate)
{
  float *addVals = new float[numFeats];
  char *addFName = new char[strlen(filename) + 1];
  int i;
  for(i = 0; i < numFeats; i++)
    addVals[i] = features[i];
  strcpy(addFName, filename);

  featureVals.push_back(addVals);
  fileNames.push_back(addFName);
  tapsFile.push_back(isTemplate);
}

void FeatureLibrary::setWeights(float *newWeights)
{
  for(int i = 0; i < numFeats; i++)
    weights[i] = newWeights[i];
}

float FeatureLibrary::distance(const float *v1, const float *v2)
{
  float dist = 0;
  for(int i = 0; i < numFeats; i++){
    float diff = weights[i] * (v1[i] - v2[i]);
    dist += diff * diff;
  }
  return sqrt(dist);
}

struct DistIndex
{
  float dist;
  int index;
};

bool operator<(const DistIndex &di1, const DistIndex &di2)
{
  return di1.dist < di2.dist;
}

void FeatureLibrary::rankClosestFiles(const float *targetVals, int *indices, 
				      float *distances)
{
  int i;

  DistIndex * sortedDistIndices = new DistIndex[featureVals.size()];
  for(i = 0; i < featureVals.size(); i++){
    DistIndex di;
    di.dist = distance(targetVals, featureVals[i]);
    di.index = i;
    sortedDistIndices[i] = (di);
  }
  std::sort(sortedDistIndices, 
	    &sortedDistIndices[featureVals.size()]);

  for(i = 0; i < featureVals.size(); i++)
    indices[i] = sortedDistIndices[i].index;
  if(distances)
    for(i = 0; i < featureVals.size(); i++)
      distances[i] = sortedDistIndices[i].dist;

  delete [] sortedDistIndices;
}

void FeatureLibrary::writeFLIFile(const char *filename)
{
  int i, j;
  FILE *libfile = fopen(filename, "w");

  fprintf(libfile, "%d\n", numFeats);
  for(i = 0; i < numFeats; i++){
    if(i > 0)
      fprintf(libfile, " ");
    fprintf(libfile, "%s", featureNames[i]);
  }
  fprintf(libfile, "\n");

  for(i = 0; i < fileNames.size(); i++){
    fprintf(libfile, "%s %d", fileNames[i], (int)tapsFile[i]);
    for(j = 0; j < numFeats; j++)
      fprintf(libfile, " %f", featureVals[i][j]);
    fprintf(libfile, "\n");
  }

  fclose(libfile);
}
