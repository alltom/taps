// Copyright (c)Victor Lazzarini, 1997-2004
// See License.txt for a disclaimer of all warranties
// and licensing information

#include "SinAnal.h"

SinAnal::SinAnal(){
	
	m_thresh = 0.f;
	m_maxtracks = 0;
	m_tracks = 0;
	m_prev = 0; m_cur =1;
	m_numbins = m_accum = 0;
	
	m_bndx = m_pkmags = m_adthresh = 0;
	m_phases = m_freqs = m_mags = m_bins = 0; 
	m_trndx = 0;
	m_binmax = m_magmax = m_diffs = 0;
	m_maxix = 0;
	m_contflag = 0;
	m_minpoints = 0;
	m_maxgap = 3;
	AddMsg("max tracks", 21);
	AddMsg("threshold", 22);
}

SinAnal::SinAnal(SndObj* input, float threshold, int maxtracks, 
				 int minpoints, int maxgap, float sr)
				 :SndObj(input,maxtracks*3,sr){
	
	m_minpoints = (minpoints > 1 ? minpoints : 1) - 1;
	m_thresh = threshold;
	m_maxtracks = maxtracks;
	m_tracks = 0;
	m_prev = 0; m_cur =1; m_accum = 0;
	m_maxgap = maxgap;
	m_numbins = ((FFT *)m_input)->GetFFTSize()/2 + 1;
	
	m_bndx = new float*[2];
	m_pkmags = new float*[2];
	m_adthresh = new float*[2];
	m_tstart = new unsigned int*[2];
	m_lastpk = new unsigned int*[2];
	m_trkid = new unsigned int*[2];
	int i;
	for(i=0; i<2; i++){
		m_bndx[i] = new float[m_maxtracks];
		m_pkmags[i] = new float[m_maxtracks];
		m_adthresh[i] = new float[m_maxtracks];
		m_tstart[i] = new unsigned int[m_maxtracks];
		m_lastpk[i] = new unsigned int[m_maxtracks];
		m_trkid[i] = new unsigned int[m_maxtracks];
		
	}
	
	m_bins = new float[m_maxtracks];
	m_trndx = new int[m_maxtracks];	
	m_contflag = new bool[m_maxtracks];

	m_phases = new float[m_numbins];
	m_freqs = new float[m_numbins];
	m_mags = new float[m_numbins];

	m_binmax = new float[m_numbins];
	m_magmax = new float[m_numbins];
	m_diffs = new float[m_numbins];
	
	m_maxix = new int[m_numbins];
	m_timecount = 0;
	
	m_phases[0] = 0.f;
	m_freqs[0] = 0.f;
	m_phases[m_numbins-1] = 0.f;
	m_freqs[m_numbins-1] = m_sr/2;
		
	AddMsg("max tracks", 21);
	AddMsg("threshold", 22);
	
	for(i = 0; i < m_maxtracks; i++)
		m_pkmags[m_prev][i] = m_bndx[m_prev][i] = m_adthresh[m_prev][i] = 0.f;
	
}

SinAnal::~SinAnal(){
	
	delete[] m_phases;
	delete[] m_freqs;
	delete[] m_mags;
	delete[] m_binmax;
	delete[] m_magmax;
	delete[] m_diffs;
	delete[] m_maxix;
	delete[] m_bndx;  
	delete[] m_pkmags;  
	delete[] m_adthresh; 
	delete[] m_tstart; 
	delete[] m_lastpk; 
	delete[] m_trkid;  
	delete[] m_trndx;
	delete[] m_contflag;
	delete[] m_bins;
	
	
}

void
SinAnal::SetMaxTracks(int maxtracks){
	
	m_maxtracks = maxtracks;
	
	if(m_numbins){
		
		delete[] m_bndx;  
		delete[] m_pkmags;  
		delete[] m_adthresh;  
		delete[] m_trndx;
		delete[] m_contflag;
		delete[] m_bins;
		
	}
	
	m_contflag = new bool[m_maxtracks];
	m_bins = new float[m_maxtracks];
	m_trndx = new int[m_maxtracks];
	
	m_bndx = new float*[2];
	m_pkmags = new float*[2];
	m_adthresh = new float*[2];
	m_tstart = new unsigned int*[2];
	m_lastpk = new unsigned int*[2];
	m_trkid = new unsigned int*[2];
	int i;
	for(i=0; i<m_minpoints; i++){
		m_bndx[i] = new float[m_maxtracks];
		m_pkmags[i] = new float[m_maxtracks];
		m_adthresh[i] = new float[m_maxtracks];
		m_tstart[i] = new unsigned int[m_maxtracks];
		m_lastpk[i] = new unsigned int[m_maxtracks];
		m_trkid[i] = new unsigned int[m_maxtracks];
		
	}
	for(i = 0; i < m_maxtracks; i++)
		m_pkmags[m_prev][i] = m_bndx[m_prev][i] = m_adthresh[m_prev][i] = 0.f;
	
	SetVectorSize(m_maxtracks*3);
}


void
SinAnal::SetIFGram(SndObj* input){
	
	if(m_input){
		
		delete[] m_phases;
		delete[] m_freqs;
		delete[] m_mags;
		delete[] m_binmax;
		delete[] m_magmax;
		delete[] m_diffs;
		delete[] m_maxix;
		
	}
	
	SetInput(input);
	m_numbins = ((FFT *)m_input)->GetFFTSize()/2 + 1;
	
	m_phases = new float[m_numbins];
	m_freqs = new float[m_numbins];
	m_mags = new float[m_numbins];
	m_binmax = new float[m_numbins];
	m_magmax = new float[m_numbins];
	m_diffs = new float[m_numbins];
	m_maxix = new int[m_numbins];
	
	m_phases[0] = 0.f;
	m_freqs[0] = 0.f;
	m_phases[m_numbins-1] = 0.f;
	m_freqs[m_numbins-1] = m_sr/2;
	
}



int
SinAnal::Set(char* mess, float value){
	
	switch(FindMsg(mess)){
		
	case 21:
		SetMaxTracks((int)value);
		return 1;
		
	case 22:
		SetThreshold(value);
		return 1;
		
	default:
		return	SndObj::Set(mess, value);
		
	}
}

int
SinAnal::Connect(char* mess, void *input){
	
	switch(FindMsg(mess)){
		
    case 3:
		SetIFGram((SndObj *)input);
		return 1;
		
	default:
		return SndObj::Connect(mess, input);
		
	}
	
}


void
SinAnal::sinanalysis(){
	
	float startupThresh, logthresh;
	int bestix, count=0, i =0, n = 0, j = 0;
	float max = 0.f,dbstep;
	double y1, y2, a, b, ftmp;
	
	for(i=0; i<m_numbins;i++)
		if(max < m_mags[i]) max = m_mags[i];
		
		
		startupThresh = m_thresh*max;
		logthresh = log(startupThresh/5.f);
		
		// Quadratic Interpolation 
		// obtains bin indexes and magnitudes
		// m_binmax & m_magmax respectively
		
		bool test1 = true, test2 = false;
		
		// take the logarithm of the magnitudes
		for(i=0; i<m_numbins;i++)
			m_mags[i] = log(m_mags[i]);
		
		for(i=0;i < m_numbins-1; i++) {
			
			if(i) test1 = (m_mags[i] > m_mags[i-1] ? true : false );
			else test1 = false;
			test2 = (m_mags[i] >= m_mags[i+1] ? true : false); // check!
			
			if((m_mags[i] > logthresh) && 
				(test1 && test2)){
				m_maxix[n] = i;
				n++;
			}
			
		}
		
		for(i =0; i < n; i++){
			int rmax;
			rmax = m_maxix[i]; // index at the ith peak
			
			y1 = m_mags[rmax] - (ftmp = (rmax ? m_mags[rmax-1] : m_mags[rmax])) + 0.000001;
			y2 = (rmax < m_numbins-1 ? m_mags[rmax+1] : m_mags[rmax]) - ftmp + 0.000001;
			
			a = (y2 - 2*y1)/2.f;
			b = 1.f - y1/a;
			
			m_binmax[i] = (float) (rmax - 1. + b/2.);  // why m_binmax[i]?  why i?  why not rmax?
			m_magmax[i] = (float) exp(ftmp - a*b*b/4.);
		}
		
		// end QuadInterp;
		
		// Peak-picking strawberry-picking
		
		// reset allowcont flags 
		for(i=0; i<m_maxtracks;i++){
			m_contflag[i] = false;
		}
		
		// loop to the end of tracks (indicate by the 0'd bins)
		// find continuation tracks
		
		for(j=0; m_bndx[m_prev][j] != 0.f && j < m_maxtracks; j++){
			
			int foundcont = 0;
			
			if(n > 0){ // check for peaks; n will be > 0
				
				float F = m_bndx[m_prev][j];
				
				for(i=0; i < m_numbins; i++){
					m_diffs[i] = m_binmax[i] - F; //differences
					m_diffs[i] = (m_diffs[i] < 0 ? -m_diffs[i] : m_diffs[i]);
				}
				
				
				bestix = 0;  // best index
				for(i=0; i < m_numbins; i++) 
					if(m_diffs[i] < m_diffs[bestix]) bestix = i;
					
					// if difference smaller than 1 bin
					float tempf = F -  m_binmax[bestix];
					tempf = (tempf < 0 ? -tempf : tempf);
					if(tempf < 1.){
						
						// if amp jump is too great (check)
						if(m_adthresh[m_prev][j] < 
							(dbstep = 20*log10(m_magmax[bestix]/m_pkmags[m_prev][j]))){
							// mark for discontinuation;  
							m_contflag[j] = false;							
						}
						else {
							m_bndx[m_prev][j] = m_binmax[bestix];
							m_pkmags[m_prev][j] = m_magmax[bestix];
							// track index keeps track history
							// so we know which ones continue
							m_contflag[j] = true; 
							m_binmax[bestix] = m_magmax[bestix] = 0.f;
							m_lastpk[m_prev][j] = m_timecount;
							foundcont = 1;
							count++;
							
							// update the adaptive mag threshold 
							float tmp1 = dbstep*1.5f;
							float tmp2 = m_adthresh[m_prev][j] -
								(m_adthresh[m_prev][j] - 1.5f)*0.048770575f;
							m_adthresh[m_prev][j] = (tmp1 > tmp2 ? tmp1 : tmp2);
							
						}  // else  		
					} // if difference          
					// if check
			}
			
			// if we did not find a continuation
			// we'll check if the magnitudes around it are below
			// a certain threshold. Mags[] holds the logs of the magnitudes
			// Check also if the last peak in this track is more than m_maxgap
			// old
			if(!foundcont){ 
				if((exp(m_mags[int(m_bndx[m_prev][j]+0.5)]) < 0.2*m_pkmags[m_prev][j])
					|| ((m_timecount - m_lastpk[m_prev][j]) > m_maxgap))
				{
					m_contflag[j] = false;

				} else {
                    m_contflag[j] = true;
                    count++;
				}

			}	
				
		} // for loop 
		
		if(count < m_maxtracks){
			
			// if we have not exceeded available tracks.	
			// compress the arrays
			for(i=0, n=0; i < m_maxtracks; i++){
				if(m_contflag[i]){
					m_bndx[m_cur][n] = m_bndx[m_prev][i];
					m_pkmags[m_cur][n] = m_pkmags[m_prev][i];
					m_adthresh[m_cur][n] = m_adthresh[m_prev][i];
					m_tstart[m_cur][n] = m_tstart[m_prev][i];
					m_trkid[m_cur][n] = m_trkid[m_prev][i];
					m_lastpk[m_cur][n] = m_lastpk[m_prev][i];
					n++;
				}	// ID == -1 means zero'd track
				else
					m_trndx[i] = -1; 
			}
			
			// now current arrays are the compressed previous
			// arrays
			
			// create peaks for all new tracks
			
			for(j=0; j< m_numbins && count < m_maxtracks; j++){
				
				if(m_magmax[j] > startupThresh){
					
					m_bndx[m_cur][count] = m_binmax[j];    
					m_pkmags[m_cur][count] = m_magmax[j];      
					m_adthresh[m_cur][count] = 400.f;    
					// track ID is a positive number in the
					// range of 0 - maxtracks*3 - 1
					// it is given when the track starts
					// used to identify and match tracks
					m_tstart[m_cur][count] = m_timecount;
					m_trkid[m_cur][count] = ((m_accum++)%m_vecsize);
					m_lastpk[m_cur][count] = m_timecount;
					count++;
					
				}
			}		
			for(i = count; i < m_maxtracks; i++){
				// zero the right-hand size of the current arrays
				if(i >= count)
					m_pkmags[m_cur][i] = m_bndx[m_cur][i] = m_adthresh[m_cur][i] = 0.f;
			}
			
		} // if count != maxtracks
		
		// count is the number of continuing tracks + new tracks
		// now we check for tracks that have been there for more
		// than minpoints hop periods and output them
		
		m_tracks = 0;
		for(i=0; i < count; i++){
			if(m_tstart[m_cur][i] <= m_timecount-m_minpoints){		
				m_bins[i] = m_bndx[m_cur][i];
				m_mags[i] = m_pkmags[m_cur][i];
				m_trndx[i] = m_trkid[m_cur][i];
				m_tracks++;
			}
			
		} 
		// end Peak-picking
		// current arrays become previous
		int tmp = m_prev;
		m_prev = m_cur;
		m_cur = tmp;
		m_timecount++;
		
}


short
SinAnal::DoProcess(){
	
	if(!m_error){     
		if(m_input){
			int i2;

			// input is in "real-spec" format packing 0 and Nyquist
			// together in pos 0 and 1

			for(m_vecpos=1; m_vecpos < m_numbins-1; m_vecpos++){
				i2 = m_vecpos*2;
				m_phases[m_vecpos] = ((PVA *)m_input)->Outphases(m_vecpos);
				m_freqs[m_vecpos] = m_input->Output(i2+1);
				m_mags[m_vecpos] = m_input->Output(i2);
			} 
			m_mags[0] = m_input->Output(0);   
			m_mags[m_numbins-1] = m_input->Output(1);
			
			if(m_enable){
				
				// sinusoidal analysis 
				// generates bin indexes and magnitudes
				// m_bins and m_mags, respectively
				
				sinanalysis();
				
				// m_output holds [amp, freq, pha]
				// m_vecsize is m_maxtracks*3 
				// estimated to be a little above count*3
				
				static unsigned int dontdothis = 0;
                fprintf( stdout, "-----------------------|%i|-------------------------\n", dontdothis );
				dontdothis++;
				for(m_vecpos=0; m_vecpos < m_vecsize; m_vecpos+=3){
					int pos = m_vecpos/3;
					float frac,a,b;
					if(pos < m_tracks){
						// magnitudes
						m_output[m_vecpos] = m_mags[pos];
						// fractional part of bin indexes
						frac =(m_bins[pos] - (int)m_bins[pos]);
						// freq Interpolation
						// m_output[1,4,7, ..etc] holds track freq
						a = m_freqs[(int) m_bins[pos]];
						b = (m_bins[pos] < m_numbins-1 ? (m_freqs[(int) m_bins[pos]+1] - a) : 0);
						m_output[m_vecpos+1] = a + frac*b;
						// phase Interpolation
						// m_output[2,5,8 ...] holds track phase
						a = m_phases[(int)m_bins[pos]];
						b = (m_bins[pos] < m_numbins-1 ? (m_phases[(int) m_bins[pos]+1] - a) : 0);
						m_output[m_vecpos+2] = a + frac*b;
						fprintf( stdout, "hi: amp: %f  freq: %f  pha: %f\n", 
							     m_output[m_vecpos], m_output[m_vecpos+1], m_output[m_vecpos+2] );
					}
					else{ // empty tracks
						m_output[m_vecpos] = 
						m_output[m_vecpos+1] = 
						m_output[m_vecpos+2] = 0.f;
					}
				}
                fprintf( stdout, "--------------------------------------------------\n\n" );
			}
			else // if disabled
				for(m_vecpos=0; m_vecpos < m_vecsize;m_vecpos++)	  
					m_output[m_vecpos] = 0.f;
				return 1;
		} 
		else {
			m_error = 11;        
			return 0;
		}
	}
	else return 0;
	
	
}
