// Copyright (c)Victor Lazzarini, 1997-2004
// See License.txt for a disclaimer of all warranties
// and licensing information

#include "SpecSplit.h"

SpecSplit::SpecSplit(){
  // outputs fftsize/2+1 pairs of values
  m_channel = new SndObj[2];
  SetVectorSize(DEF_FFTSIZE+2);
  m_halfsize = m_vecsize/2;
  m_channel[0].SetVectorSize(m_halfsize);
  m_channel[1].SetVectorSize(m_halfsize);
  magnitude = &m_channel[0];
  phase = &m_channel[1];

}

SpecSplit::SpecSplit(SndObj *input, int vecsize, float sr)
                 :SpecPolar(input,vecsize, sr){
  
  m_channel = new SndObj[2];
  m_halfsize = m_vecsize/2;
  m_channel[0].SetVectorSize(m_halfsize);
  m_channel[1].SetVectorSize(m_halfsize);
  magnitude = &m_channel[0];
  phase = &m_channel[1];

}

SpecSplit::~SpecSplit(){
delete[] m_channel;
}


short
SpecSplit::DoProcess(){

if(!m_error){
 if(m_input){ 
	if(m_enable) {  
	 float a, b; float* start = m_output;
	 int p;
	 // mag[0] -> pos 0
     m_output[0] = m_input->Output(0);
	 // mag[nyquist] -> pos fftsize/2
	 m_output[m_halfsize-1] = m_input->Output(1);
	 // pha[0] -> pos fftsize/2  + 1
     m_output[m_halfsize] = 0.f;
	 // pha[nyquist] pos fftsize + 1
	 m_output[m_vecsize-1] = 0.f;

	 // ignore the 0,nyquist pair
	 // and go all the way to FFTSIZE/2 
     // halfsize is (FFTSIZE+2)/2

	 for(m_vecpos = 1; m_vecpos < m_halfsize-1; m_vecpos++) {
		
		// every complex pair
		   
        // convert 
		   p = m_vecpos*2;
		   a = m_input->Output(p); 	               
		   b  = m_input->Output(p+1);
           convert(&a, &b);
           m_output[m_vecpos] = a; 
           m_output[m_vecpos+m_halfsize] = b;

	 }
 // split the mags and phases
 m_channel[0].DoProcess();
 m_output += m_halfsize;
 m_channel[1].DoProcess();
 m_output = start;	
}
	else 
	for(m_vecpos = 0; m_vecpos < m_halfsize; m_vecpos++)
		m_output[m_vecpos] = m_output[m_vecpos+m_halfsize] = 0.f;
 
		 return 1;

 } else {
	m_error = 3;
	return 0;
 }
}
else return 0;

}
