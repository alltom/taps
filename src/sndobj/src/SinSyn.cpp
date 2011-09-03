// Copyright (c)Victor Lazzarini, 1997-2004
// See License.txt for a disclaimer of all warranties
// and licensing information

#include "SinSyn.h"

SinSyn::SinSyn(){

m_factor = m_vecsize/m_sr;
m_facsqr = m_factor*m_factor;
m_ptable = 0;
m_size = 0;
m_LoTWOPI = 0.f;
m_maxtracks = 0;
m_freqs = 0;
m_amps = 0;
m_phases = 0;
m_trackID = 0;
m_scale =  0.f;

AddMsg("max tracks", 21);
AddMsg("scale", 23);
AddMsg("table", 24);
}

SinSyn::SinSyn(SinAnal* input, int maxtracks, Table* table, 
			   float scale, int vecsize, float sr)	  
	:SndObj(input, vecsize, sr){

m_ptable = table;
m_size = m_ptable->GetLen();
m_LoTWOPI = m_size/TWOPI;

m_factor = m_vecsize/m_sr;
m_facsqr = m_factor*m_factor;
m_maxtracks = maxtracks;

m_scale = scale;
m_input = input;
m_freqs = new float[m_maxtracks];
m_amps = new float[m_maxtracks];
m_phases = new float[m_maxtracks];
m_trackID = new int[m_maxtracks];

m_incr = 0.f;
AddMsg("max tracks", 21);
AddMsg("scale", 23);
AddMsg("table", 24);
memset(m_trackID, 0, sizeof(int));

}

SinSyn::~SinSyn(){

delete[] m_freqs;  
delete[] m_amps;  
delete[] m_phases;  
delete[] m_trackID;

}

void
SinSyn::SetTable(Table *table)
{
m_ptable = table;
m_size = m_ptable->GetLen();
m_LoTWOPI = m_size/TWOPI;
}

int
SinSyn::Connect(char* mess, void* input){

	switch (FindMsg(mess)){

	case 24:
	SetTable((Table *) input);
    return 1;

	default:
    return SndObj::Connect(mess,input);
     
	}

}


int
SinSyn::Set(char* mess, float value){

	switch(FindMsg(mess)){

	case 21:
	SetMaxTracks((int)value);
	return 1;

	case 23:
	SetScale(value);
	return 1;
	
	default:
	return SndObj::Set(mess, value);

	}
}


void
SinSyn::SetMaxTracks(int maxtracks){

if(m_maxtracks){

delete[] m_freqs;  
delete[] m_amps;  
delete[] m_phases;  
delete[] m_trackID;

}

m_maxtracks = maxtracks;
m_freqs = new float[m_maxtracks];
m_amps = new float[m_maxtracks];
m_phases = new float[m_maxtracks];
m_trackID = new int[m_maxtracks];

}

short
SinSyn::DoProcess() {
	
	if(m_input){
		
		float ampnext,amp,freq, freqnext, phase,phasenext;
		float a2, a3, phasediff, ncycs;
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
			freqnext = m_input->Output(i+1)*TWOPI; 
			phasenext = m_input->Output(i+2);
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
					phasenext = phase + freq*m_factor;
					amp = m_amps[track]; 
					ampnext = 0.f;
				}
			}
			
			else{ 
				// new tracks
				contin = true;
				track = -1;
				freq = freqnext;
				phase = phasenext - freq*m_factor;
				amp = 0.f;
			}
			
			// frame-to-frame delta phi 
			phasediff = phasenext - phase;
			// average number of cycles in one hop period
			ncycs = ((freq+freqnext)*m_factor/2. - phasediff)/TWOPI;
			// round it up/down and add the phase diff
			phasediff += (TWOPI * ((int) (ncycs + 0.5)));

			// interpolation coefs
			a2 = (3./m_facsqr) * (phasediff - (m_factor/3.)*(2*freq+freqnext));
			a3 = (1./(3*m_facsqr))  * (freqnext - freq - 2*a2*m_factor);
			
			// interpolation & track synthesis loop	
			for(m_vecpos=0; m_vecpos < m_vecsize; m_vecpos++){
		
				if(m_enable) {    
					float interp, ph, a, ti;
					interp = m_vecpos/(float)m_vecsize;
					// linear interp
					a = amp + (ampnext - amp)*interp;
					// freqs/phase cubic interp
                   
					ti = interp * m_factor;
					ph = phase + freq*ti + a2*ti*ti +
						a3*ti*ti*ti;
						
					// table lookup oscillator
					// m_LoTWOPI is L/TWOPI
					float frac, val;
					ph *= m_LoTWOPI;
					while(ph < 0) ph += m_size;
					while(ph > m_size) ph -= m_size;
					frac = (val = tab[(int)ph]) - tab[(int)ph+1];
					m_output[m_vecpos] += a*(val + (val - tab[(int)ph+1])*-frac); 
				}
				else m_output[m_vecpos] = 0.f;
			
			}

		// keep amp, freq, and phase values for next time
		if(contin){
			m_amps[i3] = ampnext;
			m_freqs[i3] = freqnext;
			m_phases[i3] = phasenext;
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

