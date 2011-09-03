// Copyright (c)Victor Lazzarini, 1997-2004
// See License.txt for a disclaimer of all warranties
// and licensing information

//************************************************************//
//  IADSR.cpp: implementation of the IADSR class              //
//          ADSR with init and end values                     //
//                                                            //
//                                                            //
//************************************************************//

#include "IADSR.h"

//////////////// CONSTRUCTION / DESTRUCTION ////////////////////

IADSR::IADSR(){

  m_init = m_end = 0.f;
  AddMsg("init", 31);
  AddMsg("end", 32);

           }

IADSR::IADSR(float init, float att, float maxamp, float dec, 
            float sus, float rel, float end,
			float dur, SndObj* InObj, int vecsize, float sr) : 
        ADSR(att, maxamp, dec, sus, rel,dur,InObj, vecsize, sr)
{
  m_init = init;
  m_end = end;
  AddMsg("init", 31);
  AddMsg("end", 32);
}

IADSR::~IADSR(){
     
}

////////////////// OPERATIONS ///////////////////////////////
int
IADSR::Set(char* mess, float value){

switch (FindMsg(mess)){

	case 31:
    SetInit(value);
	return 1;

	case 32:
	SetEnd(value);
    return 1;

	default:
    return ADSR::Set(mess,value);
}

}


short
IADSR::DoProcess()
{
if(!m_error){  
float a;
 for(m_vecpos=0; m_vecpos < m_vecsize; m_vecpos++){
  if(m_enable){   
  if(m_count == m_dur)m_count=0;
   if(m_count < m_att)
    a = ((m_maxamp - m_init)/m_att)*m_count + m_init;	
     if(m_count >= m_att && m_count < (m_att+m_dec)) 
      a = ((m_sus - m_maxamp)/m_dec)*(m_count - m_att) + m_maxamp;
       if(m_count >= (m_att+m_dec) && m_count <= (m_dur - m_rel))
	     a = m_sus;
	   if (m_count > (m_dur - m_rel)) {
		  if( !m_sustain){
	      a = ((m_end - m_sus)/m_rel)*(m_count - (m_dur - m_rel)) + m_sus;
		  m_count++;
		   }
		  else a = m_sus;
		}
       else  m_count++;

    if(m_input) 
        m_output[m_vecpos] = m_input->Output(m_vecpos)*a;
	else m_output[m_vecpos] = a;
  }	 
 else m_output[m_vecpos] = 0.f ;
 
 }
 return 1;
	} else return 0;
	
}
 

