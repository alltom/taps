// Copyright (c)Victor Lazzarini, 1997-2004
// See License.txt for a disclaimer of all warranties
// and licensing information

////////////////////////////////////////////////
// FIR.cpp: implementation of the FIR class
//          Finite Impulse Response filter
//          by direct convolution
////////////////////////////////////////////////
#include "Fir.h"
#include "UsrDefTable.h"

FIR::FIR(){

m_table = 0;
m_newtable = false;
AddMsg("impulse", 31);
AddMsg("impulse size", 32);
AddMsg("table", 33);
}


FIR::FIR(Table* coeftable, SndObj* input, 
		 int vecsize, float sr)
		 : DelayLine(coeftable->GetLen()/sr, 
		               input, vecsize, sr){
m_table = coeftable;
m_newtable = false;
AddMsg("impulse", 31);
AddMsg("impulse size", 32);
AddMsg("table", 33);
}


FIR::FIR(float* impulse, int impulsesize, SndObj* input, 
		 int vecsize, float sr)
		 : DelayLine(impulsesize/sr, 
		               input, vecsize, sr){

m_table = new UsrDefTable(impulsesize, impulse);
m_newtable = true;
AddMsg("impulse", 31);
AddMsg("impulse size", 32);
AddMsg("table", 33);
}


FIR::~FIR(){

if(m_newtable) delete m_table;

}

void
FIR::SetImpulse(float* impulse, int impulsesize){

if(m_newtable) delete m_table;
m_table = new UsrDefTable(impulsesize, impulse);
SetDelayTime(impulsesize*m_sr);
m_newtable = true;

}

short
FIR::DoProcess(){
if(!m_error){     
 if(m_input){
	 float out;
	 int i;
  for(m_vecpos=0; m_vecpos < m_vecsize;m_vecpos++){
  if(m_enable){     
	  PutSample(m_input->Output(m_vecpos));
	  // wpointer is already incremented, so that the
	  // current sample is at wpointer - 1
	  // and oldest sample at wpointer - m_size
	  for(i=1,out=0.f; i <= m_size; i++){
       m_rpointer = m_wpointer - i;
       out += GetSample(m_rpointer)*m_table->Lookup(i);
	  }
     m_output[m_vecpos] = out;
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

int FIR::Set(char *mess, float value)
{

	switch(FindMsg(mess)){

    case 21:
	SetDelayTime(value);
	return 1;

	case 32:
	m_size = (int) value;
    return 1;
	
	default:
    return DelayLine::Set(mess, value);

    }

}

int FIR::Connect(char *mess, void *input)
{

   	switch(FindMsg(mess)){

    case 33:
	SetTable((Table *)input);
	return 1;

	case 31:
	SetImpulse((float *) input, m_size);

	default:
    return SndObj::Connect(mess, input);


    }


}
