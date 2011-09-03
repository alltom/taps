// Copyright (c)Victor Lazzarini, 1997-2004
// See License.txt for a disclaimer of all warranties
// and licensing information

//************************************************************//
//  Gain.cpp: implementation of the Gain class                //
//           (Sound Object gain manipulator)                  //
//                                                            //
//                                                            //
//************************************************************//


#include "Gain.h"
#include <math.h>
//////////////// CONSTRUCTION / DESTRUCTION ////////////////////

Gain::Gain(){
m_gain = 0.f;
AddMsg("gain", 21);
AddMsg("multiplier", 22);
           }

Gain::Gain(float gain, SndObj* InObj, int vecsize, float sr):
		   SndObj(InObj, vecsize, sr) {
  m_gain = dBToAmp(gain);
  AddMsg("gain", 21);
AddMsg("multiplier", 22);
                           }

Gain::~Gain(){
}

////////////////// OPERATIONS ///////////////////////////////


float 
Gain::dBToAmp(float amp)
               {
	amp = (float) pow(10.0, (double) (amp/20));
    return amp;
               }


int
Gain::Set(char* mess, float value){

	switch(FindMsg(mess)){

	case 21:
	SetGain(value);
	return 1;

	case 22:
	SetGainM(value);
	return 1;

	default:
    return SndObj::Set(mess, value);


	}

}

void
Gain::SetGain(float gain)
                 {
        m_gain = dBToAmp(gain);
                  }

short
Gain::DoProcess(){
if(!m_error){
 if(m_input){
  for(m_vecpos = 0; m_vecpos < m_vecsize; m_vecpos++){
  if(m_enable) m_output[m_vecpos] = m_input->Output(m_vecpos)*m_gain;
  else m_output[m_vecpos] = 0.f;
	       }
 return 1;
 } else {
		m_error=3;
		return 0;
	}
}
else return 0;
}
   
