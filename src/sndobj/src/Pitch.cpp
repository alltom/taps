// Copyright (c)Victor Lazzarini, 1997-2004
// See License.txt for a disclaimer of all warranties
// and licensing information

//***********************************************************//
//  Pitch.cpp: Implementation of the Pitch Object           //
//                                                           //
//                                                           //
//                                                           //
//***********************************************************//

#include "Pitch.h"
#include <stdio.h>

//////////CONSTRUCTION /////////////////////

Pitch::Pitch(){
m_pointer1 = (float) m_rpointer;
m_pointer3 = (float) m_rpointer+(m_size/2);
m_pitch = 1;
m_incr = 0; 

AddMsg("multiplier", 21);
AddMsg("semitones", 22);

}

Pitch::Pitch(float delaytime, SndObj* InObj, float pitch, int vecsize, float sr)
   : DelayLine(delaytime, InObj, vecsize, sr)
{

m_pointer1 = (float) m_rpointer;
m_pointer3 = (float) m_rpointer+(m_size/2);
m_pitch = pitch;
m_incr = 0;

AddMsg("multiplier", 31);
AddMsg("semitones", 32);

}

Pitch::Pitch(float delaytime, SndObj* InObj, int semitones, int vecsize, float sr)
   : DelayLine(delaytime, InObj, vecsize, sr)
{

m_pointer1 = (float) m_rpointer;
m_pointer3 = (float) m_rpointer+(m_size/2);
m_pitch = (float) pow(2., semitones/12.);
m_incr = 0;

AddMsg("multiplier", 31);
AddMsg("semitones", 32);

}

Pitch::~Pitch()
{
}


////////////////////OPERATIONS////////////////////
int
Pitch::Set(char* mess, float value){

	switch (FindMsg(mess)){

	case 31:
    SetPitch(value);
	return 1;

	case 32:
	SetPitch((int) value);
    return 1;

	default:
    return DelayLine::Set(mess,value);
     
	}


}



short
Pitch::DoProcess(){
     
	if(!m_error){	
if(m_input){   
    float s1, s3;
	float absdiff;
	int halfsize;

 for(m_vecpos=0; m_vecpos<m_vecsize;m_vecpos++){	 
   if(m_enable){
	halfsize = m_size/2;
	absdiff = (float) (int) m_pointer1 - m_wpointer;
	absdiff = absdiff > 0 ? absdiff : -absdiff;
	
	if(absdiff > halfsize){
		if(m_pointer1 > m_wpointer) absdiff = m_wpointer+m_size - m_pointer1;
		else absdiff = m_pointer1+m_size - m_wpointer;
	}
	absdiff = absdiff/halfsize;

    s1 = GetSample(m_pointer1); 
	s3 = GetSample(m_pointer3);

	PutSample(m_input->Output(m_vecpos));
    m_output[m_vecpos] =  absdiff*s1+ (1.f - absdiff)*s3;
	
	m_incr += m_pitch;

	while(m_incr >= m_size) m_incr -= m_size;
	m_pointer1 = m_incr;
    m_pointer3 = m_pointer1+m_size/2;
    while(m_pointer3 >= m_size) m_pointer3 -= m_size;
   }
  else m_output[m_vecpos] = 0.f;
 }
 return 1;
}
 else{        
 m_error = 11;
        return 0;
 }
	}
 else return 0;
}
