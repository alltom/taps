// Copyright (c)Victor Lazzarini, 1997-2004
// See License.txt for a disclaimer of all warranties
// and licensing information

//***********************************************************//
//  Loop.cpp: Implementation of the SndLoop Object           //
//                (loop)                                     //
//                                                           //
//                                                           //
//***********************************************************//

#include "Loop.h"

//////////CONSTRUCTION /////////////////////

SndLoop::SndLoop(){

m_xfade = 0;
m_count = 0;
m_point = (float) m_rpointer;
m_pitch = 1; 

AddMsg("pitch", 31);
AddMsg("crossfade", 32);
AddMsg("resample", 33);


}

SndLoop::SndLoop(float xfadetime, float looptime, SndObj* InObj, float pitch,
				 int vecsize, float sr)
   : DelayLine(looptime, InObj, vecsize, sr)
{
Enable();
m_xfade = (xfadetime*m_sr);
m_sample = 1;
m_count = 0;
m_pitch = pitch;
m_point = (float) m_rpointer;

AddMsg("pitch", 31);
AddMsg("crossfade", 32);
AddMsg("resample", 33);

}

SndLoop::~SndLoop()
{
}

////////////////////OPERATIONS////////////////////
int
SndLoop::Set(char* mess, float value){

	switch (FindMsg(mess)){

	case 31:
    SetPitch(value);
	return 1;

	case 32:
	SetXFade(value);
        
        case 33:
        ReSample();
    return 1;

	default:
    return DelayLine::Set(mess,value);
     
	}


}

short
SndLoop::DoProcess(){
  

if(!m_error){   
if(m_input){       
 for(m_vecpos=0; m_vecpos<m_vecsize;m_vecpos++){  
  if(m_enable){
   if(m_sample){
	if(m_count < m_size){
	  if(m_count < m_xfade)
		PutSample((m_output[m_vecpos] = m_input->Output(m_vecpos)*(m_count/m_xfade)));
	  else 
	    PutSample((m_output[m_vecpos] = m_input->Output(m_vecpos)));	
	}
	else 
	  PutSample((m_output[m_vecpos] = GetSample() +
	   m_input->Output(m_vecpos)*((m_xfade - (m_count - m_size))/m_xfade)));
		
	if(m_count < m_size + m_xfade) m_count++;
    else  m_sample = 0;
   }
   else {
	 m_output[m_vecpos] = GetSample();
	 m_point += m_pitch;
	 m_rpointer = (long)m_point;
	 while(m_point > m_size) m_point -= m_size;
   }    
  }
 else m_output[m_vecpos] = 0.f;
 }
 return 1;
}
else {        
	m_error = 11;
    return 0;
    }
}
else return 0;
}

