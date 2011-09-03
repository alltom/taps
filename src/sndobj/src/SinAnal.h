// Copyright (c)Victor Lazzarini, 1997-2004
// See License.txt for a disclaimer of all warranties
// and licensing information

#ifndef _SINANAL_H
#define _SINANAL_H

#include "SndObj.h"
#include "PVA.h"

class SinAnal : public SndObj {

protected:

float** m_bndx;  // bin indexes
float** m_pkmags;  // peak mags
float** m_adthresh;  // thresholds
unsigned int** m_tstart;  // start times 
unsigned int** m_lastpk; // end times
unsigned int** m_trkid; // track ids

float* m_phases; // phases
float* m_freqs;  // frequencies
float* m_mags;   // magnitudes
float* m_bins;   // track bin indexes
int* m_trndx;    // track IDs

float* m_binmax;  // peak bin indexes
float* m_magmax;  // peak mags
float* m_diffs;    // differences

int* m_maxix;     // max peak locations
bool* m_contflag; // continuation flags

int m_numbins;    // number of bins
int m_maxtracks;   // max number of tracks
float m_thresh;    // threshold
int m_tracks;      // tracks in a frame
int m_prev;         
int m_cur;
int m_accum;       // ID counter
unsigned int m_timecount;
int m_minpoints;     // minimun number of points in track
int m_maxgap;     // max gap (in points) between consecutive points   

private:

void inline sinanalysis();

public:

SinAnal();
SinAnal(SndObj* input, float threshold, int maxtracks, int minpoints=1,
		int maxgap=3, float sr=DEF_SR);
~SinAnal();

int GetTrackID(int track){ return m_trndx[track]; }
int GetTracks(){ return m_tracks;}

int Set(char* mess, float value);
int Connect(char* mess, void* input);

void SetThreshold(float threshold){ m_thresh = threshold; }
void SetIFGram(SndObj* input);
void SetMaxTracks(int maxtracks);


short DoProcess();

};


#endif
