// Copyright (c)Victor Lazzarini, 1997-2004
// See License.txt for a disclaimer of all warranties
// and licensing information

#include "AdSyn.h"

AdSyn::AdSyn(){
	AddMsg("pitch", 31);
}

AdSyn::AdSyn(SinAnal* input, int maxtracks, Table* table,
	float pitch, float scale, int vecsize, float sr)	  
	:SinSyn(input, maxtracks, table, scale, vecsize, sr){

m_pitch = pitch;
AddMsg("pitch", 31);
}
AdSyn::~AdSyn(){
}

int
AdSyn::Set(char* mess, float value){

	switch(FindMsg(mess)){

	case 31:
	SetPitch(value);
	return 1;
	
	default:
	return SinSyn::Set(mess, value);

	}
}


short
AdSyn::DoProcess() {
	
	if(m_input){
		
		float ampnext,amp,freq,freqnext,phase;
		int i3, i, j, ID, track;
		int notcontin = 0;
		bool contin = false;
		int oldtracks = m_tracks;
        float* tab = m_ptable->GetTable(); 
		if((m_tracks = ((SinAnal *)m_input)->GetTracks()) >
			m_maxtracks) m_tracks = m_maxtracks;
		
		memset(m_output, 0, sizeof(float)*m_vecsize);
		
		// for each track
		i = j = 0;
		while(i < m_tracks*3){
			
			i3 = i/3;
			ampnext =  m_input->Output(i)*m_scale;
			freqnext = m_input->Output(i+1)*m_pitch; 
			ID = ((SinAnal *)m_input)->GetTrackID(i3);

			j = i3+notcontin;
			
			if(i3 < oldtracks-notcontin){
				if(m_trackID[j]==ID){	
					// if this is a continuing track  	
					track = j;
					contin = true;	
					freq = m_freqs[track];
					phase = m_phases[track];
					amp = m_amps[track];
					
				}
				else {
					// if this is  a dead track
					contin = false;
					track = j;
					freqnext = freq = m_freqs[track];
					phase = m_phases[track];
					amp = m_amps[track]; 
					ampnext = 0.f;
				}
			}
			
			else{ 
				// new tracks
				contin = true;
				track = -1;
				freq = freqnext;
				phase = -freq*m_factor;
				amp = 0.f;
			}
		
			// interpolation & track synthesis loop	
			for(m_vecpos=0; m_vecpos < m_vecsize; m_vecpos++){
		
				if(m_enable) {    
					float interp,a,f;
					interp = m_vecpos/(float)m_vecsize;
					// linear interp
					a = amp + (ampnext - amp)*interp;
					// freqs/phase cubic interp
                    f = freq + (freqnext - freq)*interp;
					// table lookup oscillator
					float frac, val;
					phase += (f*m_size/m_sr);
					while(phase < 0) phase += m_size;
					while(phase > m_size) phase -= m_size;
					frac = (val = tab[(int)phase]) - tab[(int)phase+1];
					m_output[m_vecpos] += a*(val + (val - tab[(int)phase+1])*-frac);
					
				}
				else m_output[m_vecpos] = 0.f;
			
			}

		// keep amp, freq, and phase values for next time
		if(contin){
			m_amps[i3] = ampnext;
			m_freqs[i3] = freqnext;
			m_phases[i3] = phase;
			m_trackID[i3] = ID;    
			i += 3;
		} else notcontin++;
		} 
		return 1;
 }
 else {
	 m_error  = 1;
	 return 0;
 }
 
}
