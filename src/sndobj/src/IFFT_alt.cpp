////////////////////////////////////////////////////////
// IFFT_alternative.cpp : a self-contained alternative
//  to the IFFT class, not using FFTW, using its own
//  FFT functions. Slower, but not depending on other code.
//  Victor Lazzarini, 2003
/////////////////////////////////////////////////////////
#include "IFFT.h"


IFFT::IFFT(){
rfftw_destroy_plan(m_plan);
m_plan = rfftw_create_plan(m_fftsize, FFTW_COMPLEX_TO_REAL, FFTW_ESTIMATE);
m_vecsize = m_hopsize;
}

IFFT::IFFT(Table* window, SndObj* input, int fftsize, int hopsize, int vecsize, 
		   float sr):
       FFT(window, input, 1.f, fftsize, hopsize, vecsize, sr)
{
rfftw_destroy_plan(m_plan);
m_plan = rfftw_create_plan(m_fftsize, FFTW_COMPLEX_TO_REAL, FFTW_ESTIMATE);
}

IFFT::~IFFT(){
}


short
IFFT::DoProcess(){


if(!m_error){
 if(m_input){
	 if(m_enable){
		 int i; float out = 0.;  	  	  
	 // Put the input fftframe into
	 // the current (free) signal frame
	 // and transform it
	 ifft(m_sigframe[m_cur]);
	 // set the current signal frame to the next
	 // one in the circular list
     m_counter[m_cur] = 0;
     m_cur--; if(m_cur<0) m_cur = m_frames-1;	 
		    
	 for(m_vecpos = 0; m_vecpos < m_vecsize; m_vecpos++){ 
         // overlap-add the time-domain signal frames
		 for(i=0; i < m_frames; i++){
		 out += m_sigframe[i][m_counter[i]]*m_table->Lookup(m_counter[i]);
		 m_counter[i]++;
		 }
		 // output it.
	 m_output[m_vecpos] = (float) out;
	 out = 0.;	   
	 }

     return 1;
	 } else { // if disabled
       for(m_vecpos = 0; m_vecpos < m_vecsize; m_vecpos++)
		             m_output[m_vecpos] = 0.f;
	    return 1;
	 }
 } else {
	m_error = 3;
	return 0;
 }
}
else 
return 0;
}

void 
IFFT::ifft(float* signal) {

// Inverse FFT function
for(int i; i < m_fftsize; i++)
   signal[i] = m_input->Output(i);
rfft(signal, m_fftsize/2, 0);

}


