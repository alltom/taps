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
