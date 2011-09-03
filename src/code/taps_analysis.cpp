/*----------------------------------------------------------------------------
    TAPESTREA: Techniques And Paradigms for Expressive Synthesis, 
               Transformation, and Rendering of Environmental Audio
      Engine and User Interface

    Copyright (c) 2006 Ananya Misra, Perry R. Cook, and Ge Wang.
      http://taps.cs.princeton.edu/
      http://soundlab.cs.princeton.edu/

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
    U.S.A.
-----------------------------------------------------------------------------*/

//-----------------------------------------------------------------------------
// file: taps_analysis.cpp
// desc: taps sinusoidal analysis
//
// author: Ananya Misra (amisra@cs.princeton.edu)
//         Ge Wang (gewang@cs.princeton.edu)
//         Perry R. Cook (prc@cs.princeton.edu)
// date: Autumn 2004
//-----------------------------------------------------------------------------
#include "taps_analysis.h"
#include "audicle_def.h"

using namespace std;




//-----------------------------------------------------------------------------
// name: Analysis()
// desc: constructor
//-----------------------------------------------------------------------------
Analysis::Analysis( )
{
    tracking = TRUE;
    max_tracks = BirdBrain::max_tracks();
    threshold = NULL;
    //threshold.intercept = 0.5;
    //threshold.slope = 0.0f;
    //threshold = 0.5;
    wnd_size = 0;
    minpoints = 8;
    maxgap = 0;
    //sig_max = 0.0f;
    frame_size = 0;
    thresh_cache = NULL;
    // grouping parameters
    harm_error = 0.3f;
    freq_mod_error = 0.3f;
    amp_mod_error = 0.3f;
    onset_error = 0.01f;
    offset_error = 0.03f;
    min_overlap_frac = 0.88f;
    min_event_length = 0.1f;
    processing_mode = NORMAL;
}




//-----------------------------------------------------------------------------
// name: ~Analysis()
// desc: destructor
//-----------------------------------------------------------------------------
Analysis::~Analysis()
{
    if( thresh_cache )
    {
        delete [] thresh_cache;
        thresh_cache = NULL;
    }
}




//-----------------------------------------------------------------------------
// name: cache_thresh()
// desc: ...
//-----------------------------------------------------------------------------
void Analysis::cache_thresh( t_TAPUINT len )
{
    if( thresh_cache )
    {
        delete [] thresh_cache;
        thresh_cache = NULL;
    }


    if( threshold )
    {
        frame_size = len;
        thresh_cache = new double[len];
        for( t_TAPUINT i = 0; i < len; i++ )
            thresh_cache[i] = threshold->y( i );
    }
}




//-----------------------------------------------------------------------------
// name: ~AnaSndObj()
// desc: destructor
//-----------------------------------------------------------------------------
AnaSndObj::~AnaSndObj()
{
    // not yet
}




#define MAX_FRAME_LEN 8192
//-----------------------------------------------------------------------------
// name: init()
// desc: ...
//-----------------------------------------------------------------------------
void AnaSndObj::init()
{
    // begin prehack
    int minpoints = 1;
    int maxgap = 3;
    f.len = MAX_FRAME_LEN;
    m_vecsize = max_tracks * 3;
    m_next_tid = 1;

    // begin hack
    m_minpoints = (minpoints > 1 ? minpoints : 1) - 1;
    m_num_tracks = 0;
    m_prev = 0; m_cur =1; m_accum = 0;
    m_maxgap = maxgap;
    
    m_bndx = new float*[2];
    m_pkmags = new float*[2];
    m_adthresh = new float*[2];
    m_tstart = new unsigned int*[2];
    m_lastpk = new unsigned int*[2];
    m_trkid = new unsigned int*[2];
    int i;
    for(i=0; i<2; i++){
        m_bndx[i] = (float *)calloc( max_tracks, sizeof(float) );
        m_pkmags[i] = (float *)calloc( max_tracks, sizeof(float) );
        m_adthresh[i] = (float *)calloc( max_tracks, sizeof(float) );
        m_tstart[i] = (unsigned int *)calloc( max_tracks, sizeof(unsigned int) );
        m_lastpk[i] = (unsigned int *)calloc( max_tracks, sizeof(unsigned int) );
        m_trkid[i] = (unsigned int *)calloc( max_tracks, sizeof(unsigned int) );        
    }
    
    m_bins = (float *)calloc( max_tracks, sizeof(float) );
    m_trndx = (int *)calloc( max_tracks, sizeof(int) ); 
    m_contflag = (bool *)calloc( max_tracks, sizeof(bool) );
    m_tracks = (Track **)calloc( max_tracks, sizeof(Track *) );

    m_binmax = (float *)calloc( f.len, sizeof(float) );
    m_magmax = (float *)calloc( f.len, sizeof(float) );
    m_diffs = (float *)calloc( f.len, sizeof(float) );
    
    m_maxix = (int *)calloc( f.len, sizeof(int) );
    m_timecount = 0;
    
    for(i = 0; i < max_tracks; i++)
        m_pkmags[m_prev][i] = m_bndx[m_prev][i] = m_adthresh[m_prev][i] = 0.f;
}

//-----------------------------------------------------------------------------
// name: get_tracks()
// desc: peak matching, except that here analyze() does the matching beforehand
//       so really just copy itracks to otracks
//-----------------------------------------------------------------------------
void AnaSndObj::get_tracks( vector<Track *> & itracks, vector<Track *> & otracks )
{
    for( int i = 0; i < itracks.size(); i++ ) {
        otracks.push_back( itracks[i] );
    }
}


#include <iostream>
using namespace std;
//-----------------------------------------------------------------------------
// name: analyze()
// desc: performs sinusoidal analysis, with peaking tracking and residual
//       input 'frame' contains 1 frequency domain frame
//       output 'otracks' contains vector of recent sinsuidal tracks
//       output 'oresidue' contains frequency domain frame of residue
//-----------------------------------------------------------------------------
void AnaSndObj::analyze( const Frame & frame, vector<Track *> & otracks, Frame & oresidue, Track * ppin )
{
    // input is in "real-spec" format packing 0 and Nyquist
    // together in pos 0 and 1

    double thresh = 0;
    //double thresh = threshold.intercept;
    //double thresh = threshold;

    // copy to local frame
    this->f = frame;
    // assert got polar
    assert( f.pol );
    // assert frame not too big
    assert( f.len <= MAX_FRAME_LEN );
    // copy to residue
    oresidue = frame;

    // sinusoidal analysis 
    // generates bin indexes and magnitudes
    // m_bins and m_mags, respectively              
    
    // m_mag[i] -> f.pol[i].mag
    // m_numbins -> f.len;
    // m_thresh -> this->threshold;
    // m_maxtracks -> this->max_tracks

    memset( m_bins, 0, max_tracks * sizeof(float) );
    memset( m_trndx, 0, max_tracks * sizeof(int) );
    memset( m_contflag, 0, max_tracks * sizeof(bool) );
    memset( m_binmax, 0, f.len * sizeof(float) );
    memset( m_magmax, 0, f.len * sizeof(float) );
    memset( m_diffs, 0, f.len * sizeof(float) );
    memset( m_maxix, 0, f.len * sizeof(int) );

    float startupThresh, logthresh;
    int bestix, count=0, i =0, n = 0, j = 0;
    float max = 0.f,dbstep;
    double y1, y2, a, b, ftmp;

    for(i=0; i<f.len;i++)
    {
        if(max < f.pol[i].mag)
        {
            max = f.pol[i].mag;
        }
    }

    startupThresh = thresh*max;
    logthresh = log(startupThresh/5.f);

    // Quadratic Interpolation 
    // obtains bin indexes and magnitudes
    // m_binmax & m_magmax respectively

    bool test1 = true, test2 = false;

    // take the logarithm of the magnitudes
    for(i=0; i<f.len;i++)
        f.pol[i].mag = log(f.pol[i].mag);

    // find potential peaks
    for(i=0;i < f.len-1; i++)
    {
        if(i) test1 = (f.pol[i].mag > f.pol[i-1].mag);
        else test1 = false;
    
        test2 = (f.pol[i].mag >= f.pol[i+1].mag); // check!

        if((f.pol[i].mag > logthresh) && (test1 && test2))
        {
            m_maxix[n] = i;
            n++;
        }
    }

    // loop over potential peaks and quadratic interpolate
    for(i =0; i < n; i++)
    {
        int rmax;
        rmax = m_maxix[i];

        y1 = f.pol[rmax].mag - (ftmp = (rmax ? f.pol[rmax-1].mag : f.pol[rmax].mag)) + 0.000001;
        y2 = (rmax < f.len-1 ? f.pol[rmax+1].mag : f.pol[rmax].mag) - ftmp + 0.000001;

        a = (y2 - 2*y1)/2.f;
        b = 1.f - y1/a;

        // should i below be rmax?
        m_binmax[i] = (float) (rmax - 1. + b/2.);
        m_magmax[i] = (float) exp(ftmp - a*b*b/4.);
    }

    // Peak-picking strawberry-picking

    // reset allowcont flags 
    for(i=0; i<max_tracks;i++)
    {
        m_contflag[i] = false;
    }

    // loop to the end of tracks (indicate by the 0'd bins)
    // find continuation tracks

    for(j=0; m_bndx[m_prev][j] != 0.f && j < max_tracks; j++)
    {
        int foundcont = 0;

        if(n > 0) // number of peaks
        { // check for peaks; n will be > 0
            float F = m_bndx[m_prev][j]; // We think F is the index of the jth peak?

            bestix = 0;  // best index
            for(i=0; i < f.len; i++)
            {
                m_diffs[i] = m_binmax[i] - F; // differences
                m_diffs[i] = ::fabs(m_diffs[i]); // L1 norm
                if(m_diffs[i] < m_diffs[bestix]) bestix = i;
            }

            // if difference smaller than 1 bin
            float tempf = F -  m_binmax[bestix];
            tempf = ::fabs(tempf);
            if(tempf < 1.)
            {
                // if amp jump is too great (check)
                if(m_adthresh[m_prev][j] < 
                    (dbstep = 20*log10(m_magmax[bestix]/m_pkmags[m_prev][j])))
                {
                    // mark for discontinuation;  
                    m_contflag[j] = false;
                    if( m_tracks[j] != NULL && m_tracks[j]->state != Track::INACTIVE )
                    {
                        // there is a track here
                        m_tracks[j]->state = Track::INACTIVE;
                        // since this will be removed soon from m_tracks because
                        // m_contflag will be false, we send this back one last
                        // time here (instead of at the end of this function)
                        otracks.push_back( m_tracks[j] );
                        // printf( "DONE 1: %i\n", m_tracks[j]->id );
                        // m_tracks[j] = NULL;
                    }
                }
                else
                {
                    m_bndx[m_prev][j] = m_binmax[bestix];
                    m_pkmags[m_prev][j] = m_magmax[bestix];
                    // track index keeps track history
                    // so we know which ones continue
                    m_contflag[j] = true;
                    if( m_tracks[j] == NULL || m_tracks[j]->state == Track::INACTIVE )
                    {
                        // start a new track
                        m_tracks[j] = new Track;
                        m_tracks[j]->state = Track::ACTIVE;
                        m_tracks[j]->id = m_next_tid++;
                        m_tracks[j]->start = f.time;            
                        // printf( "NEW 1: %i!\n", m_tracks[j]->id );
                    }

                    m_binmax[bestix] = m_magmax[bestix] = 0.f;
                    m_lastpk[m_prev][j] = m_timecount;
                    foundcont = 1;
                    count++;
                            
                    // update the adaptive mag threshold 
                    float tmp1 = dbstep*1.5f;
                    float tmp2 = m_adthresh[m_prev][j] -
                                (m_adthresh[m_prev][j] - 1.5f)*0.048770575f;
                    m_adthresh[m_prev][j] = (tmp1 > tmp2 ? tmp1 : tmp2);
                            
                }  // else (amp jump is not too great)      
            } // if difference          
        } // n > 0

        // if we did not find a continuation
        // we'll check if the magnitudes around (i mean closest to) it are below
        // a certain threshold (i mean .2 * the previous jth peak mag) Mags[] holds '
        // the logs of the magnitudes
        // Check also if the last peak in this track is more than m_maxgap old
        // (i mean m_lastpk contains the last time there was a peak in this track)
        if(!foundcont)
        {
            if(exp(f.pol[int(m_bndx[m_prev][j]+0.5)].mag) < 0.2*m_pkmags[m_prev][j]
                || (m_timecount - m_lastpk[m_prev][j] > m_maxgap))
            {
                m_contflag[j] = false;
                if( m_tracks[j] != NULL && m_tracks[j]->state != Track::INACTIVE )
                {
                    // there is a track here
                    m_tracks[j]->state = Track::INACTIVE;
                    // since this will be removed soon from m_tracks because
                    // m_contflag will be false, we send this back one last
                    // time here (instead of at the end of this function)
                    otracks.push_back( m_tracks[j] );
                    // printf( "DONE 2: %i\n", m_tracks[j]->id );
                    // m_tracks[j] = NULL;
                }
            }
            else
            {
                m_contflag[j] = true;
                if( m_tracks[j] == NULL || m_tracks[j]->state == Track::INACTIVE )
                {
                    // start a new track
                    m_tracks[j] = new Track;
                    m_tracks[j]->state = Track::ACTIVE;
                    m_tracks[j]->id = m_next_tid++;
                    m_tracks[j]->start = f.time;            
                    // printf( "NEW 2: %i!\n", m_tracks[j]->id );
                }
                count++;
            }
        }
    } // for loop 
        
    if(count <= max_tracks)
    {
        // if we have not exceeded available tracks.    
        // compress the arrays
        for(i=0, n=0; i < max_tracks; i++)
        {
            if(m_contflag[i])
            {
                m_bndx[m_cur][n] = m_bndx[m_prev][i];
                m_pkmags[m_cur][n] = m_pkmags[m_prev][i];
                m_adthresh[m_cur][n] = m_adthresh[m_prev][i];
                m_tstart[m_cur][n] = m_tstart[m_prev][i];
                m_trkid[m_cur][n] = m_trkid[m_prev][i];
                m_lastpk[m_cur][n] = m_lastpk[m_prev][i];
                m_tracks[n] = m_tracks[i];
                if( n != i )
                    m_tracks[i] = NULL;
                n++;
            }   // ID == -1 means zero'd track
            else
            {
                m_trndx[i] = -1;  // this doesn't seem to be used anywhere, anytime, by anyone
                if( m_tracks[i] != NULL && m_tracks[i]->state != Track::INACTIVE )
                {
                    // there is a track here
                    m_tracks[i]->state = Track::INACTIVE;
                    // since this will be removed soon from m_tracks because
                    // m_contflag will be false, we send this back one last
                    // time here (instead of at the end of this function)
                    otracks.push_back( m_tracks[i] );
                    // printf( "DONE 3: %i\n", m_tracks[i]->id );
                    // m_tracks[i] = NULL;
                }
            }
        }
            
        // now current arrays are the compressed previous arrays
            
        // create peaks for all new tracks
            
        for(j=0; j< f.len && count < max_tracks; j++)
        {
            if(m_magmax[j] > startupThresh)
            {
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
                if( m_tracks[count] == NULL || m_tracks[count]->state == Track::INACTIVE )
                {
                    // start
                    m_tracks[count] = new Track;
                    m_tracks[count]->state = Track::ACTIVE;
                    m_tracks[count]->id = m_next_tid++;
                    m_tracks[count]->start = f.time;            
                    // printf( "NEW 3: %i\n", m_tracks[count]->id );
                }
                count++;
            }
        }
        for(i = count; i < max_tracks; i++)
        {
            // zero the right-hand size of the current arrays
            // if(i >= count)
                m_pkmags[m_cur][i] = m_bndx[m_cur][i] = m_adthresh[m_cur][i] = 0.f;
        }
            
    } // if count != maxtracks
        
    // count is the number of continuing tracks + new tracks
    // now we check for tracks that have been there for more
    // than minpoints hop periods and output them
        
    m_num_tracks = 0;
    for(i=0; i < count; i++)
    {
        if(m_tstart[m_cur][i] <= m_timecount-m_minpoints)
        {
            m_bins[i] = m_bndx[m_cur][i];
            f.pol[i].mag = m_pkmags[m_cur][i];
            m_trndx[i] = m_trkid[m_cur][i];
            m_num_tracks++;
        }
        else if( m_tracks[i] != NULL && m_tracks[i]->state != Track::INACTIVE )
        {
            // there is a track here
            m_tracks[i]->state = Track::INACTIVE;
            // since this will be removed soon from m_tracks because
            // m_contflag will be false, we send this back one last
            // time here (instead of at the end of this function)
            otracks.push_back( m_tracks[i] );
            // printf( "DONE 4: %i\n", m_tracks[i]->id );
            // m_tracks[i] = NULL;
        }
        else
            printf("!@#$%^&*()\n");
    } 
    
    if( count )
    {
        printf( "-------------> tracks: %i/%i  now: %i <-------------------\n", m_num_tracks, count, f.time );
    }

    // end Peak-picking

    // current arrays become previous
    int tmp = m_prev;
    m_prev = m_cur;
    m_cur = tmp;
    m_timecount++;
    
    // m_output holds [amp, freq, pha]
    // m_vecsize is max_tracks*3 
    // estimated to be a little above count*3

    int m_vecpos;
    for(m_vecpos=0; m_vecpos < m_vecsize; m_vecpos+=3)
    {
        int pos = m_vecpos/3;
        float frac,a,b;
        if(pos < m_num_tracks)
        {
            // compute stuff (un)dutifully as (not) done (not) above
            freqpolar stuff;
            stuff.time = f.time;
            if(f.freqs) {
                frac =(m_bins[pos] - (int)m_bins[pos]);
                a = f.freqs[(int) m_bins[pos]];
                b = (m_bins[pos] < f.len-1 ? (f.freqs[(int) m_bins[pos]+1] - a) : 0);
                stuff.freq = a + frac * b;
            }
            else
                stuff.freq = m_bins[pos]; //?

            a = f.pol[(int)m_bins[pos]].phase;
            b = (m_bins[pos] < f.len-1 ? (f.pol[(int) m_bins[pos]+1].phase - a) : 0);
            stuff.p.phase = a + frac * b;

            stuff.p.mag = f.pol[pos].mag;

            // enter this computation into the track
            Track *track = m_tracks[pos];
            int maxloc, temp3, before_peak, after_peak, temp4;
            float themax, temp, temp2;
            if( track && track->state != Track::INACTIVE )
            {
                track->history.push_back(stuff);
                printf("Analysis: %i   %f  %f  %f\n", track->id, stuff.freq, stuff.p.mag, stuff.p.phase);
                
                // enter pointer to track into the output vector
                otracks.push_back( track );
        
                maxloc = (int)m_bins[pos];
                themax = oresidue.pol[(int)m_bins[pos]].mag;

                // find endpoints or something (figure it out yourself (each time))
                for( temp3 = maxloc-2, temp2 = themax;
                     temp3 > 0 && oresidue.pol[temp3 - 1].mag <= temp2;
                     temp2 = oresidue.pol[temp3--].mag );
                
                before_peak = temp3 >= 0 ? temp3 : 0;
                
                for( temp4 = maxloc+2, temp2 = themax;
                     temp4 < oresidue.len-1 && oresidue.pol[temp4 + 1].mag <= temp2;
                     temp2 = oresidue.pol[temp4++].mag );

                after_peak = temp3 < oresidue.len ? temp3 : oresidue.len - 1;

                // line interpolation
                temp = (float)(oresidue.pol[after_peak].mag - oresidue.pol[before_peak].mag) 
                    / (after_peak - before_peak + 1);
                
                for (int q = before_peak; q <= after_peak; q++) {
                    oresidue.pol[q].mag = 0.0f; //(float) oresidue.pol[before_peak].mag + temp * (q - before_peak);
                    oresidue.pol[q].phase = 2 * PIE * rand()/(float)RAND_MAX;
                }

                // residue?
                // oresidue.pol[(int)m_bins[pos]].mag = 0.f;
                // oresidue.pol[maxloc].phase = 2 * PIE * rand()/(float)RAND_MAX;
            }
            else if( track ) 
            {
                maxloc = (int)m_bins[pos];
                themax = oresidue.pol[(int)m_bins[pos]].mag;

                // find endpoints or something (figure it out yourself (each time))
                for( temp3 = maxloc, temp2 = themax;
                     temp3 > 0 && oresidue.pol[temp3 - 1].mag <= temp2;
                     temp2 = oresidue.pol[temp3--].mag );
                
                before_peak = temp3;
                
                for( temp3 = maxloc, temp2 = themax;
                     temp3 < oresidue.len-1 && oresidue.pol[temp3 + 1].mag <= temp2;
                     temp2 = oresidue.pol[temp3++].mag );

                after_peak = temp3;

                // line interpolation
                temp = (float)(oresidue.pol[after_peak].mag - oresidue.pol[before_peak].mag) 
                    / (after_peak - before_peak + 1);
                
                for (int q = before_peak; q <= after_peak; q++) {
                    oresidue.pol[q].mag = (float) oresidue.pol[before_peak].mag + temp * (q - before_peak);
                    oresidue.pol[q].phase = 2 * PIE * rand()/(float)RAND_MAX;
                }

                // residue?
                // oresidue.pol[(int)m_bins[pos]].mag = 0.f;
                // oresidue.pol[maxloc].phase = 2 * PIE * rand()/(float)RAND_MAX;
            }

                
        } // if( pos < num_tracks )
        else
        { // empty tracks
//            m_output[m_vecpos] = 
//            m_output[m_vecpos+1] = 
//            m_output[m_vecpos+2] = 0.f;
        }
    } // some for loop for pos
    if( m_num_tracks > 1 && false )
    {
        fprintf(stderr, "RESIDUE %i\n", frame.time);
        int o;
        for(o = 0; o < oresidue.len; o++)
        {
            fprintf(stderr, "%f ", oresidue.pol[o].mag);
        }
        fprintf(stderr, "\n\nORIGINAL\n");
        for(o = 0; o < frame.len; o++)
        {
            fprintf(stderr, "%f ", frame.pol[o].mag);
        }
        fprintf(stderr, "\n\n\n");
    }
}


void AnaPeaksFFT::init()
{
    //peaks = (int *)calloc( max_tracks, sizeof(int) );
    //magns = (float *)calloc( max_tracks, sizeof(float) );
    m_next_tid = 1;
    threshold = NULL;
    //threshold.intercept = .3;
    //threshold.slope = 0;
    //threshold = .3;
}


void AnaPeaksFFT::clear()
{
    // deleting tracks dangerous because these pointers are used elsewhere
//  for( int it = 0; it < tracks.size(); it++ ){ SAFE_DELETE( tracks[it] ); }
    tracks.clear(); 
}


// destructor (this)
AnaPeaksFFT::~AnaPeaksFFT()
{
    //if( peaks ) free( peaks );
    //if( magns ) free( magns );
}

/*
This function abuses the concept of tracks...
In this a track is actually a frame and each point in history in the track is actually a different peak. (Sorry.)
In the second pass, get_tracks uses these fake tracks to do peak matching to get real tracks across frames.
(Or that's what it's supposed to do.)
*/
void AnaPeaksFFT::analyze( const Frame & frame, vector<Track *> & otracks, Frame & oresidue, Track * ppin )
{
    assert( frame.len != 0 );
    // copy frame
    this->f = frame;
    // null out low frequency terms
    int length = frame.len;
    float max, absolute_max, temp2, average;
    int maxloc, peak, i, temp3, before_peak, after_peak, winWidth = (length / wnd_size);
    for (i=0;i<winWidth; i++)     
        f.pol[i].mag = 0;
    // appropriate processing mode
    if( processing_mode == PREPROCESS_OUT )
    {
        analyze_preprocess_out( otracks, oresidue ); 
        return; 
    }
    if( processing_mode == PREPROCESS_IN )
    {
        analyze_preprocess_in( otracks, oresidue, ppin ); 
        // copy to residue and add nulled out stuff back in
        oresidue = f;
        for ( i=0; i<winWidth; i++)     
            oresidue.pol[i].mag = frame.pol[i].mag;
        return;
    }
    // processing_mode == NORMAL
    Track * track;
    freqpolar stuff;

    track = new Track;
    track->start = frame.time;  

    // find average of magnitude
    average = 0.f;
    for( i = 0; i < f.len; i++ )
    {
        // find average
        average += f.pol[i].mag;
        // zero out all bins less than threshold (line)
        if( threshold && f.pol[i].mag < thresh_cache[i] )
            f.pol[i].mag = 0.0f;
    }
    average /= f.len;

    peak = 0;
    while (peak < max_tracks * BirdBrain::over_tracking() )        
    {
        max = 0.0;
        maxloc = -1;
        // this loop gets the current maximum value in the frame
        for (i=0;i<length;i++)
        {
            if( (BirdBrain::freq_min() >= 0 && f.freqs[i] < BirdBrain::freq_min())
              ||(BirdBrain::freq_max() >= 0 && f.freqs[i] > BirdBrain::freq_max()) ) // outside rectangle
                continue;

            if (::fabs(f.pol[i].mag) > max)   
            {
                max = (float)::fabs(f.pol[i].mag);  // why fabs? isn't it already +ve due to sqrt?
                maxloc = i;
            }
            else if( f.pol[i].mag < 0 )
                BB_log( BB_LOG_CRAZY, "i: f.pol[i].mag: %f", i, f.pol[i].mag );
        }
        
        if( peak == 0 ) absolute_max = max; // absolute_max is the king of maxes

        // observe threshold
        if( max / average < noise_ratio ) break;
        //if( max < threshold.intercept + threshold.slope * maxloc ) break;
        //if( max < threshold ) break;
        //if( max < absolute_max * (threshold.intercept + threshold.slope * maxloc) ) break;
        //if( max * 4.0 < absolute_max * threshold / (sqrt((double)peak+.5) ) ) break;

        // no more peaks found
        if( maxloc == -1 ) break;

        // peak found
        stuff.freq = frame.freqs[maxloc];
        stuff.p.mag = max;
        stuff.p.phase = 0;
        stuff.time = frame.time;
        stuff.bin = maxloc;
        stuff.isMatched = false;
        track->history.push_back( stuff );
        if( stuff.freq <= 0 )
            printf( "whoa\n" );
        
        // find endpoints or something (figure it out yourself (each time))
        for( temp3 = maxloc - winWidth/2, temp2 = max;
             temp3 > 0 && f.pol[temp3 - 1].mag <= temp2;
             temp2 = f.pol[temp3--].mag );
        
        before_peak = temp3 >= 0 ? temp3 : 0;
        
        for( temp3 = maxloc + winWidth/2, temp2 = max;
             temp3 < length-1 && f.pol[temp3 + 1].mag <= temp2;
             temp2 = f.pol[temp3++].mag );

        after_peak = temp3 < length ? temp3 : length - 1;
        
        // notch
        for (i = before_peak; i <= after_peak; i++) {
                f.pol[i].mag = 0;
        }

        peak++;
    } // all peaks have been found now

    otracks.push_back( track );

    // copy to residue and add nulled out stuff back in
    oresidue = f;
    for ( i=0; i<winWidth; i++)     
    {
        oresidue.pol[i].mag = frame.pol[i].mag;
    }
} // end of analyze



//-----------------------------------------------------------------------------
// name: analyze_preprocess_out()
// desc: preprocess given sound
//       find all peaks
//       assume this->f is the frame to be analyzed
//-----------------------------------------------------------------------------
void AnaPeaksFFT::analyze_preprocess_out( vector<Track *> & otracks, Frame & oresidue)
{
    int i,c,d,e; 
    freqpolar stuff; 
    
    Track * track = new Track;
    track->start = f.time; 

    // find all peaks
    for( i = 1; i < f.len - 1; i++ )
    {
        if( f.pol[i].mag - f.pol[i-1].mag > 0 && f.pol[i+1].mag - f.pol[i].mag <= 0 )
        {
            // peak found
            stuff.freq = f.freqs[i];
            stuff.p.mag = f.pol[i].mag;
            stuff.p.phase = 0;
            stuff.time = f.time; // ah. time absolute or from start of analysis? (absolute!!!)
            stuff.bin = i;
            stuff.isMatched = false;
            track->history.push_back( stuff );
        }
    }
    
    // sort by descending magnitudes? 
    for( c = 1; c < track->history.size(); c++ )
    {
        for( d = c-1; d >= 0; d-- )
        {
            e = d + 1;
            if( track->history[e].p.mag > track->history[d].p.mag )
            {
                stuff = track->history[d];
                track->history[d] = track->history[e];
                track->history[e] = stuff;
            }
            else break;
        }
    }

    otracks.push_back( track ); 
}


//-----------------------------------------------------------------------------
// name: analyze_preprocess_in()
// desc: analyze preprocessed peak information
//       apply magnitude thresholding
//       assume this->f is the frame to be analyzed
//-----------------------------------------------------------------------------
void AnaPeaksFFT::analyze_preprocess_in( vector<Track *> & otracks, Frame & oresidue, Track * ppin )
{
    //int length = frame.len;
    //float max, absolute_max, temp2, average;
    //int maxloc, peak, i, temp3, before_peak, after_peak, winWidth = (length / wnd_size);
    
    if( !ppin || ppin->history.empty() )
        return;

    int length = f.len;
    int peak, maxloc, i, j, temp3, before_peak, after_peak, winWidth = length / wnd_size;
    float average, max, absolute_max, temp2;
    Track * track;
    freqpolar stuff;

    track = new Track;
    track->start = ppin->history[0].time;   

    // find average of magnitude
    average = 0.f;
    for( i = 0; i < f.len; i++ )
    {
        // find average
        average += f.pol[i].mag;
        // zero out all bins less than threshold (line) [unnecessary; just to be consistent]
        if( threshold && f.pol[i].mag < thresh_cache[i] )
            f.pol[i].mag = 0.0f;
    }
    average /= f.len;

    absolute_max = ppin->history[0].p.mag;

    peak = 0;
    for( i = 0; i < ppin->history.size() && peak < max_tracks * BirdBrain::over_tracking(); i++ )        
    {
        // observe thresholds
        max = ppin->history[i].p.mag;
        maxloc = ppin->history[i].bin;
        if( max / average < noise_ratio ) break;
        if( threshold && max < thresh_cache[maxloc] ) continue;
        if( (BirdBrain::freq_min() >= 0 && ppin->history[i].freq < BirdBrain::freq_min())
          ||(BirdBrain::freq_max() >= 0 && ppin->history[i].freq > BirdBrain::freq_max()) ) // outside rectangle
            continue;

        stuff.freq = ppin->history[i].freq;
        stuff.p.mag = max;
        stuff.p.phase = ppin->history[i].p.phase;
        stuff.time = ppin->history[i].time;
        stuff.bin = maxloc;
        stuff.isMatched = false;
        track->history.push_back( stuff );
    
        //peaks[peak] = maxloc;
        //magns[peak] = max;
        
        // find endpoints or something (figure it out yourself (each time))
        for( temp3 = maxloc - winWidth/2, temp2 = max;
             temp3 > 0 && f.pol[temp3 - 1].mag <= temp2;
             temp2 = f.pol[temp3--].mag );
        
        before_peak = temp3 >= 0 ? temp3 : 0;
        
        for( temp3 = maxloc + winWidth/2, temp2 = max;
             temp3 < length-1 && f.pol[temp3 + 1].mag <= temp2;
             temp2 = f.pol[temp3++].mag );

        after_peak = temp3 < length ? temp3 : length - 1;
        
        // notch
        for (j = before_peak; j <= after_peak; j++) {
                f.pol[j].mag = 0;
        }

        peak++;
    } // all peaks have been found now

    otracks.push_back( track );
}


//-----------------------------------------------------------------------------
// name: get_tracks()
// desc: peak matching
//       each 'track' in itracks can contain all the peaks in a frame.
//       otracks returns matched peaks from frame to frame (meaning actual tracks)
//-----------------------------------------------------------------------------
void AnaPeaksFFT::get_tracks( vector<Track *> & itracks, vector<Track *> & otracks )
{
    // go backwards from last frame to first?
    // do our old matching tricks
    // but keep stuff around even if they discontinue for some time

    // no tracks
    if( itracks.size() == 0 )
        return;

    BB_log( BB_LOG_INFO, "(analysis) Matching peaks..." ); 

    int h, i, j, k, closest_index, c, d, e, size;
    float ratio_freq, ratio, error_m = 0.5f; // error_f = 0.9f, 
    Track *track;
    Track *cur_frame = itracks.back();
    freqpolar stuff;
    vector<Track *> temp5;

    if(!tracks.empty())
      BB_log(BB_LOG_WARNING, "AnaPeaksFFT::get_tracks(): internal tracks not empty");
    tracks.clear();
    for( i = 0; i < cur_frame->history.size(); i++ ) {
        // add current track to 'tracks'
        track = new Track;
        track->history.push_back( cur_frame->history[i] );
        track->state = Track::AMBIGUOUS;
        track->start = cur_frame->start;
        track->id = m_next_tid++;
        tracks.push_back( track );
    }

    // for each frame (going backwards) 
    for( h = itracks.size() - 2; h >= 0; h-- ) {
        cur_frame = itracks[h];

        // for each existing track from previous (but actually subsequent) frames
        for( i = 0; i < tracks.size(); i++ ) {
            // make ambiguous, if continuation found, will set back to active
            if( tracks[i]->state == Track::INACTIVE )
                continue;
            tracks[i]->state = Track::AMBIGUOUS;
            closest_index = -1;
            ratio_freq = 0.f;

            // go through all peaks in the frame: 
            for( j = 0; j < cur_frame->history.size(); j++ ) {
                if( !cur_frame->history[j].isMatched ) {
                    // find the peak closest in frequency to the current track (tracks[i])
                    if( tracks[i]->history.back().freq <= 0.f ) {
                        //cout << tracks[i]->history.back().freq << endl;
                        continue;
                    }
                    //assert( tracks[i]->history.back().freq > 0.f );
                    ratio = cur_frame->history[j].freq / tracks[i]->history.back().freq;
    
                    if( ratio > 1.0 ) ratio = 1.0 / ratio;
                    if( ratio > ratio_freq ) {
                        closest_index = j;
                        ratio_freq = ratio;
                    }
                }               
            } // end of for each peak

            Track *t = tracks[i];       
            // see if frequency jump from this track to closest frequency is small enough
            if( closest_index >= 0 && ratio_freq > error_f ) {
                // enter history for any possible gap
                if( tracks[i]->historian > 0 ) {
                    for( c = tracks[i]->historian; c >= 1; c-- ) {
                        stuff.freq = tracks[i]->history.back().freq;
                        stuff.p.mag = 0.f;
                        stuff.p.phase = 0.f;
                        stuff.time = itracks[h+c]->start;
                        stuff.bin = tracks[i]->history.back().bin;
                        stuff.isMatched = true;
                        tracks[i]->history.push_back( stuff );
                    }
                }

                // track continuation
                tracks[i]->state = Track::ACTIVE;
                tracks[i]->historian = 0;
                // add this peak to history
                stuff = cur_frame->history[closest_index];
                stuff.time = cur_frame->start;
                stuff.isMatched = true;
                tracks[i]->history.push_back( stuff );
                assert( tracks[i]->start == tracks[i]->history[0].time );
                // mark this peak as used
                cur_frame->history[closest_index].isMatched = true;
            }
            else { // this track is dormant
                tracks[i]->historian++;
                if( tracks[i]->historian > maxgap ) {
                    tracks[i]->state = Track::INACTIVE;
                    tracks[i]->historian = 0;
                    tracks[i]->end = tracks[i]->history.back().time; //cur_frame->start;
                }
                // maybe we can push in a mag = 0, freq = 0 thing into history here
            }
        } // end of for each existing track
        
        // sort peaks by ascending magnitudes
        for( c = 1; c < cur_frame->history.size(); c++ )
        {
            for( d = c-1; d >= 0; d-- )
            {
                e = d + 1;
                if( cur_frame->history[e].p.mag < cur_frame->history[d].p.mag )
                {
                    stuff = cur_frame->history[d];
                    cur_frame->history[d] = cur_frame->history[e];
                    cur_frame->history[e] = stuff;
                }
                else break;
            }
        }

        // sort tracks by ascending magnitudes
        for( c = 1; c < tracks.size(); c++ )
        {
            for( d = c-1; d >= 0; d-- )
            {
                e = d + 1;
                if( tracks[e]->history.back().p.mag < tracks[d]->history.back().p.mag )
                {
                    track = tracks[d];
                    tracks[d] = tracks[e];
                    tracks[e] = track;
                }
                else break;
            }
        }

        // at this point:
        // peaks are sorted small -> big (magnitude)
        // tracks are sorted small -> big (magnitude)

        // look for potential new tracks in the remaining peaks
        // go backwards (big to small)
        for( i = cur_frame->history.size() - 1; i >= 0; i-- )
        {
            // already used
            if( cur_frame->history[i].isMatched ) continue;

            // too small
            //if( cur_frame->history[i].freq / average < noise_ratio ) break;

            size = tracks.size();
            if( size < max_tracks )
            {
                // add the peak as a new track
                track = new Track;
                track->state = Track::ACTIVE;
                track->id = m_next_tid++;
                track->start = cur_frame->start;
                track->historian = 0;
                stuff = cur_frame->history[i];
                stuff.time = cur_frame->start;
                stuff.isMatched = cur_frame->history[i].isMatched = true;
                track->history.push_back( stuff );
                assert( track->start == track->history[0].time );
                tracks.push_back( track );
            }
            else break;
        }
        
        // now there are no empty slots so some tracks need to be evicted
        for( j = i; j >= 0; j-- )
        {
            // already used
            if( cur_frame->history[j].isMatched ) continue;

            // too small
            //if( cur_frame->history[j].freq / average < noise_ratio ) continue;

            for( k = 0; k < tracks.size(); k++ )
            {
                if( tracks[k]->state == Track::INACTIVE )
                {
                    // evict existing track, and replace with new track
                    if( tracks[k]->history.size() >= minpoints ) {
                        tracks[k]->invert();
                        assert( tracks[k]->end != 0 );
                        otracks.push_back( tracks[k] );
                    }
					else { // discard
						SAFE_DELETE( tracks[k] );
					}

                    // make new track
                    tracks[k] = new Track;
                    tracks[k]->state = Track::ACTIVE;
                    tracks[k]->id = m_next_tid++;
                    tracks[k]->start = cur_frame->start;
                    stuff = cur_frame->history[j];
                    stuff.time = cur_frame->start;
                    stuff.isMatched = cur_frame->history[j].isMatched = true;
                    tracks[k]->history.push_back( stuff );
                    assert( tracks[k]->start == tracks[k]->history[0].time );
                    break;
                }
            }
        }

        // sort tracks by ascending magnitudes
        for( c = 1; c < tracks.size(); c++ )
        {
            for( d = c-1; d >= 0; d-- )
            {
                e = d + 1;
                if( tracks[e]->id < tracks[d]->id )
                {
                    track = tracks[d];
                    tracks[d] = tracks[e];
                    tracks[e] = track;
                }
                else break;
            }
        }

        // set tracks to only active or ambiguous tracks    
        temp5.clear();
        for( int s = 0; s < tracks.size(); s++ ) {
            // if at the last frame, we should also add all active and ambiguous tracks
            // to the output track vector so they don't get ignored, so set those to inactive too
            if( h == 0 )
                tracks[s]->state = Track::INACTIVE;
            // push inactive tracks to output vector, keep others in analysis
            if( tracks[s]->state != Track::INACTIVE )
                temp5.push_back( tracks[s] );
            else {
                tracks[s]->end = tracks[s]->history.back().time;
                // assert( tracks[s]->start != 0 );
                if( tracks[s]->history.size() >= minpoints ) {
                    tracks[s]->invert();
                    otracks.push_back( tracks[s] );
                }
                else {
                   // track is too short: discard
                   SAFE_DELETE(tracks[s]);
                }
            }
        }
        tracks = temp5; 

    } // end of for each frame

    // printf("\n");

    //verify( otracks );

    // clean up
    temp5.clear(); // everything in temp5 should be in tracks due to last line in loop
    for( i = 0; i < tracks.size(); i++ ) {
        SAFE_DELETE( tracks[i] );
    }
    tracks.clear();
}


void AnaPeaksFFT::verify( vector <Track *> &itracks )
{
    for( int i = 0; i < itracks.size(); i++ ) {
        Track * t = itracks[i];
        cout << "--------------------------------------------------------------------------------------" << endl;
        cout << "ID: " << t->id << endl;
        cout << "start: " << t->start << endl;
        cout << "end: " << t->end << endl;
        cout << "historian: " << t->historian << endl;
        cout << "history: " << t->history.size() << endl;
        for( int j = 0; j < t->history.size(); j++ ) {
            cout << "    " << j << " "<< "freq(" << t->history[j].freq << ")  time(" << itracks[i]->history[j].time << ")" << endl;
        }
    }
}


void AnaPeaksFFT::get_res( vector <Track *> &itracks, Frame &resframe )
{
    int i, j, k, hat, before_peak, after_peak, maxloc, winWidth = 2 * (resframe.len / wnd_size);
    float temp, temp2, max;
    int temp3;
    // Track * bad;

    // I think tracks are sorted by start time, with earliest tracks last
    // but this order is not necessarily preserved for the last 'maxgap' tracks.
    
    for( i = itracks.size()-1; i >= 0; i-- ) {
        
        if( itracks[i]->start > resframe.time || itracks[i]->end < resframe.time )
            continue;
        
        j = -1;
        for( hat = 0; hat < itracks[i]->history.size(); hat++ ) {
            if( itracks[i]->history[hat].time == resframe.time ) {
                j = hat;
                break;
            }
        }       
        assert( j != -1 );
        // asserts are apparently skipped in the Release build, so...
        if( j == -1 )
        {
            BB_log( BB_LOG_SYSTEM_ERROR, "Assertion failed in AnaPeaksFFT::get_res; no matching frame found" ); 
            exit(1); 
        }

        max = itracks[i]->history[j].freq;
        maxloc = itracks[i]->history[j].bin;
        //int qerpoiuqwer = (int)(itracks[i]->history[j].freq * resframe.len * 2 / BirdBrain::srate() + .5);
        // check here again when you see this
//        if( maxloc > 0 && resframe.pol[maxloc - 1].mag > resframe.pol[maxloc].mag )
//            maxloc--;
//        else if( maxloc < resframe.len - 1 && resframe.pol[maxloc + 1].mag > resframe.pol[maxloc].mag )
//            maxloc++;

        if( ::fabs(resframe.freqs[maxloc] - itracks[i]->history[j].freq) > 1 )
            cout << "AAAAAAAAAAAA" << endl;

        // find endpoints or something (figure it out yourself (each time))
        for( temp3 = maxloc - winWidth/2, temp2 = max;
             temp3 > 0 && resframe.pol[temp3 - 1].mag <= temp2;
             temp2 = resframe.pol[temp3--].mag );
        
        before_peak = temp3 >= 0 ? temp3 : 0;
        
        for( temp3 = maxloc + winWidth/2, temp2 = max;
             temp3 < resframe.len - 1 && resframe.pol[temp3 + 1].mag <= temp2;
             temp2 = resframe.pol[temp3++].mag );

        after_peak = temp3 < resframe.len ? temp3 : resframe.len - 1;

        // line interpolation
        temp = (float) (resframe.pol[after_peak].mag - resframe.pol[before_peak].mag) / (after_peak - before_peak + 1);
        
        for (k = before_peak; k <= after_peak; k++) {
            resframe.pol[k].mag = (float) resframe.pol[before_peak].mag + temp * (k - before_peak);
            resframe.pol[k].phase = 2 * PIE * rand() / RAND_MAX;
        }
    }
}


bool is_int( float question, float error )
{
    return fabs( question - (int)(question + 0.5f) ) < error; 
}


// get_events: 
// itracks = vector of sinusoidal tracks after peak matching
// oevents = tracks grouped into events (each SinEvent obj has a vector of tracks)
void AnaPeaksFFT::get_events( vector<Track *> & itracks, vector<SinEvent> & oevents )
{
    if( itracks.empty() )
        return;

    SinEvent * se;
    SinEvent * bigger;
    SinEvent * smaller;
    vector<SinEvent *> vse;
    // temp
    int s, t, m;

    // each new track is compared with existing groups to find if it fits in with any
    // otherwise it's added as a new group
    bool fit;
    Track * tr;
    int my_group, my_new_group, del; 

    // create groups
    for( t = itracks.size() - 1; t >= 0; t-- ) // whoa... why does this work so much better?
    {
        my_group = -1;
        my_new_group = -1;
        fit = false;
        tr = itracks[t];
        
        // check with all existing groups
        for( s = 0; s < vse.size(); s++ )
        {
            se = vse[s];
            if( se == NULL ) // merged with earlier group
                continue;

            if( se->cmp_harm( tr, harm_error, min_overlap_frac ) )
            {
                fit = true;
                my_new_group = s;
                BB_log( BB_LOG_FINE, "harmonic fit of track %i with group %i", t, s );
                BB_log( BB_LOG_FINE, my_group == -1 ? "" : " - already fit to group %i", my_group ); 
            }
            else if( se->cmp_mod( tr, freq_mod_error, amp_mod_error, min_overlap_frac ) )
            {
                fit = true;
                my_new_group = s;
                BB_log( BB_LOG_FINE, "modulation fit of track %i with group %i", t, s );
                BB_log( BB_LOG_FINE, my_group == -1 ? "" : " - already fit to group %i", my_group ); 
            }
            else if( se->cmp_onoff( tr, onset_error, offset_error ) )
            {
                fit = true;
                my_new_group = s;
                BB_log( BB_LOG_FINE, "onset/offset fit of track %i with group %i", t, s );
                BB_log( BB_LOG_FINE, my_group == -1 ? "" : " - already fit to group %i", my_group ); 
            }
            // add track to group while keeping groups disjoint
            if( my_new_group == s )
            {
                if( my_group != -1 )
                {
                    if( vse[my_group]->event_tracks.size() < se->event_tracks.size() )
                    {
                        bigger = se;
                        smaller = vse[my_group];
                        del = my_group;
                        my_group = s;
                    }
                    else
                    {
                        bigger = vse[my_group];
                        smaller = se;
                        del = s;
                    }
                    // now bigger = vse[my_group] and smaller = vse[del]
                    for( m = 0; m < smaller->event_tracks.size(); m++ )
                    {
                        bigger->add_track( smaller->event_tracks[m] );
                    }
                    bigger->add_track( tr );
                    SAFE_DELETE( vse[del] );
                }
                else
                {
                    se->add_track( tr );
                    my_group = s;
                }
            }
        } // end of checking this track ('me' or 'tr') with all existing groups

        if( !fit )
        {
            se = new SinEvent;
            se->add_track( tr );
            vse.push_back( se ); 
            BB_log( BB_LOG_FINE, "creating new group %i specially for track %i", s, t );
        }
    }

    // get oevents from vse
    oevents.clear();
    for( s = 0; s < vse.size(); s++ )
    {
        if( vse[s] != NULL && (vse[s]->end - vse[s]->start + 1.0) / BirdBrain::srate() >= min_event_length )
        {
            oevents.push_back( *(vse[s]) );
            SAFE_DELETE( vse[s] );
        }
    }

    BB_log( BB_LOG_INFO, "created %i groups", oevents.size() );
}
