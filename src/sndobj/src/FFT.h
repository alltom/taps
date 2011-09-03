// Copyright (c)Victor Lazzarini, 1997-2004
// See License.txt for a disclaimer of all warranties
// and licensing information

//////////////////////////////////////////////////////
// FFT.h: interface of the FFT class: 
//        short-time fast fourier transform
//        using the FFTW library (v. 2.1.3) 
//        Victor Lazzarini, 2003 
/////////////////////////////////////////////////////////

#ifndef _FFT_H
#define _FFT_H
#include "SndObj.h"
#include "Table.h"
#include <rfftw.h>

class FFT : public SndObj {

protected:

// m_vecsize is FFT size
// m_hopsize should always be set to the time-domain
// vector size
int m_fftsize;
int m_hopsize;  // hopsize
int m_halfsize; // 1/2 fftsize
int *m_counter; // counter 
rfftw_plan m_plan; // FFTW initialisation
float m_fund;

float m_scale; // scaling factor
float m_norm;  // norm factor
int m_frames;  // frame overlaps
float** m_sigframe; // signal frames
float* m_ffttmp; // tmp vector for fft transform
int m_cur;    // index into current frame 

Table*  m_table; // window

private:
// fft wrapper method
void inline fft(float* signal);

// reset memory and initialisation
void ReInit();

public:

	FFT();
	FFT(Table* window, SndObj* input, float scale=1.f, 
		  int fftsize=DEF_FFTSIZE, int hopsize=DEF_VECSIZE,
		  float m_sr=DEF_SR);

	~FFT();
  
	int GetFFTSize() { return m_fftsize; }
	int GetHopSize() { return m_hopsize; }
	void SetWindow(Table* window){ m_table = window;}
	int Connect(char* mess, void* input);
	int Set(char* mess, float value);
	void SetScale(float scale){ m_scale = scale; m_norm = m_fftsize/m_scale;}
	virtual void SetFFTSize(int fftsize);
	virtual void SetHopSize(int hopsize);

	short DoProcess();
  
};

#endif





