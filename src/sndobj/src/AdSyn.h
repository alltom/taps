// Copyright (c)Victor Lazzarini, 1997-2004
// See License.txt for a disclaimer of all warranties
// and licensing information

#ifndef _ADSYN_H
#define _ADSYN_H

#include "SinSyn.h"

class AdSyn : public SinSyn {

 protected:

 float m_pitch;

 public:

 AdSyn();
 AdSyn(SinAnal* input, int maxtracks, Table* table,
	    float pitch = 1.f, float scale=1.f, 
		int vecsize=DEF_VECSIZE, float sr=DEF_SR);
 ~AdSyn();
  void SetPitch(float pitch){ m_pitch = pitch; }
  int Set(char* mess, float value);
  short DoProcess();


};

#endif
