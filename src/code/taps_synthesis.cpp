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
// file: taps_synthesis.cpp
// desc: taps sinusoidal synthesis
//
// author: Ananya Misra (amisra@cs.princeton.edu)
//         Ge Wang (gewang@cs.princeton.edu)
//         Perry R. Cook (prc@cs.princeton.edu)
//         SndObj
// date: Autumn 2004
//-----------------------------------------------------------------------------
#include "taps_synthesis.h"
#include <iostream>
using namespace std;


//-----------------------------------------------------------------------------
// name: Synthesis()
// desc: constructor
//-----------------------------------------------------------------------------
Synthesis::Synthesis()
{
    // yes
    max_tracks = BirdBrain::max_tracks();
    time_stretch = 1.0f; 
    freq_warp = 1.0f; 
}


//-----------------------------------------------------------------------------
// name: ~Synthesis()
// desc: destructor
//-----------------------------------------------------------------------------
Synthesis::~Synthesis()
{
}




//-----------------------------------------------------------------------------
// name: init()
// desc: ...
//-----------------------------------------------------------------------------
void SynSndObj::init()
{
    m_freqs = new float[max_tracks];
    m_amps = new float[max_tracks];
    m_phases = new float[max_tracks];
}




//-----------------------------------------------------------------------------
// name: synthesize()
// desc: ...
//-----------------------------------------------------------------------------
void SynSndObj::synthesize( Frame & frame )
{   
    //if(!tracks.empty())
    {
        float ampnext,amp,freq, freqnext, phase,phasenext;
        float a2, a3, phasediff, ncycs;
        int i3, j, ID, track;
        int notcontin = 0;
        bool contin = false;
        int oldtracks = m_tracks;
        //float* tab = m_ptable->GetTable(); 
        m_tracks = tracks.size();
        if( m_tracks > max_tracks ) m_tracks = max_tracks;
        
        //memset(m_output, 0, sizeof(float)*m_vecsize);
        
        float m_factor = frame.wlen / (float)BirdBrain::srate();
        float m_facsqr = m_factor * m_factor;

        // pass the number of samples generated back
        frame.wsize = frame.wlen;

        // for each track
        i3 = j = 0;
        while(i3 < m_tracks)
        {
            ampnext = 1.0 * tracks[i3]->history.back().p.mag;// m_input->Output(i)*m_scale;//?
            freqnext = tracks[i3]->history.back().freq * 2 * PIE; //m_input->Output(i+1)*TWOPI; 
            phasenext = tracks[i3]->history.back().p.phase; //m_input->Output(i+2);
            ID =  tracks[i3]->id; //((SinAnal *)m_input)->GetTrackID(i3);

            j = i3+notcontin;
            
            if(i3 < oldtracks-notcontin)
            {
                //if(m_trackID[j]==ID){ 
                if(tracks[j]->state == Track::ACTIVE)
                {
                    // if this is a continuing track    
                    track = j;
                    contin = true;  
                    freq = m_freqs[track];
                    phase = m_phases[track];
                    amp = m_amps[track];
                    
                }
                else
                {
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
            else
            {
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
            ncycs = ((freq+freqnext)*m_factor/2. - phasediff) * .5/PIE;
            // round it up/down and add the phase diff
            phasediff += (2.0 * PIE * ((int) (ncycs + 0.5)));

            // interpolation coefs
            a2 = (3./m_facsqr) * (phasediff - (m_factor/3.)*(2*freq+freqnext));
            a3 = (1./(3*m_facsqr))  * (freqnext - freq - 2*a2*m_factor);
            
            // interpolation & track synthesis loop 
            int m_vecpos;
            float tab[10000];
            for(m_vecpos=0; m_vecpos < 10000; m_vecpos++)
                tab[m_vecpos] = cos( m_vecpos / 10000.0 * 2.0 * PIE );
            double m_LoTWOPI = 5000/PIE;
            for(m_vecpos=0; m_vecpos < frame.wlen; m_vecpos++)
            {
                if(true)
                {//if(m_enable) {    
                    float interp, ph, a, ti;
                    interp = m_vecpos/(float)frame.wlen;
                    // linear interp
                    a = amp + (ampnext - amp)*interp;
                    // freqs/phase cubic interp
                   
                    ti = interp * m_factor;
                    ph = phase + freq*ti + a2*ti*ti + a3*ti*ti*ti;

                    // table lookup oscillator
                    // m_LoTWOPI is L/TWOPI
                    float frac, val;
                    ph *= m_LoTWOPI;
                    while(ph < 0) ph += 10000;
                    while(ph > 10000) ph -= 10000;
                    frac = (val = tab[(int)ph]) - tab[(int)ph+1];
                    frame.waveform[m_vecpos] += a*(val + (val - tab[(int)ph+1])*-frac); 
                }
                else
                    frame.waveform[m_vecpos] = 0.f;
            }

            // keep amp, freq, and phase values for next time
            if(contin)
            {
                m_amps[i3] = ampnext;
                m_freqs[i3] = freqnext;
                m_phases[i3] = phasenext;
//          m_trackID[i3] = ID;    
                i3++;
            } 
            else notcontin++;
        } 
        return;
    }
    //else
    //d assert( false );
}


//-----------------------------------------------------------------------------
// name: init()
// desc: ...
//-----------------------------------------------------------------------------
void Syn::init()
{
}

//-----------------------------------------------------------------------------
// name: setTracks()
// desc: crash
//-----------------------------------------------------------------------------
void Syn::setTracks( std::vector<Track *> me )
{
    tracks = me; 
    if( me.empty() )
        BB_log( BB_LOG_INFO, "Syn:setTracks : input tracks are empty\n" );
    if( tracks.empty() )
        BB_log( BB_LOG_INFO, "Syn:setTracks : internal tracks are empty\n" );
}

//-----------------------------------------------------------------------------
// name: synthesize()
// desc: takes vector of tracks to be synthesized together
//       starts synthesizing when earliest track begins (without silence before)
//       returns one big frame bigger than hop(_)size
//-----------------------------------------------------------------------------
void Syn::synthesize( Frame & frame )
{
    Frame f;
    t_TAPUINT t;


    // make sure there are tracks
    if( tracks.empty() ) {
        BB_log( BB_LOG_INFO, "synthesize: no tracks" );
        return;
    }

    // find start time by taking min of start times for all tracks
    t_TAPTIME start_time = tracks[0]->history[0].time;       // danger comment: timeout
    t_TAPTIME end_time = tracks[0]->history.back().time; // danger comment: timeout
    for( t = 1; t < tracks.size(); t++ ) {
        if( tracks[t]->history[0].time < start_time )
            start_time = tracks[t]->history[0].time;
        if( tracks[t]->history.back().time > end_time )
            end_time = tracks[t]->history.back().time;
    }

    // round to t_TAPUINT because that worked in old version
    start_time = (t_TAPUINT)(start_time * time_stretch);
    end_time = (t_TAPUINT)(end_time * time_stretch);

    int howlong = (int)end_time - (int)start_time + BirdBrain::syn_wnd_size(); // danger comment: timeout
    // allocate waveform for synthesis frame
    frame.wsize = howlong;
    if( howlong % 2 ) howlong++;
    frame.alloc_waveform( howlong ); // needs to be even

    // remember the start time of the frame
    frame.time = start_time;

    // synthesize each track and add to frame
    for( t = 0; t < tracks.size(); t++ ) {
        // printf("-");
        
        freqpolar halt = tracks[t]->history.back();
        halt.p.mag = 0.f;
        // halt.time += tracks[t]->history.size() < 2 ? 256: halt.time - tracks[t]->history[tracks[t]->history.size()-2].time;
        halt.time += BirdBrain::hop_size() / time_stretch; //BirdBrain::hop_size(); // 256
        tracks[t]->history.push_back( halt );
   
        for( int offset = 1; offset < tracks[t]->history.size(); offset++ ) {
            f.zero();
            // synthesize history[curr]-history[prev]
            synhelp( f, tracks[t], offset );
            // add f back to frame
            //frame.add( f, 0, (t_TAPUINT)(time_stretch * tracks[t]->history[offset-1].time) - start_time );
            frame.add( f, 0, (int)(time_stretch * tracks[t]->history[offset-1].time - start_time) );
        }

        // remove the extra thing you pushed back, silly
        tracks[t]->history.pop_back();
    } 
    //printf("\n");
}

// modeling by example
// start_time = 2
// track t starts at time 4
// we are at offset 2 of track t
// 4 + 2 - 2 = 4


//-----------------------------------------------------------------------------
// name: synhelp()
// desc: synthesize between two consecutive points in history of 1 track
//-----------------------------------------------------------------------------
void Syn::synhelp( Frame &frame, Track *t, int offset )
{
    freqpolar curr, prev;
    int diff_t;
    double diff_f, diff_m, delta_f, delta_m, delta_p, freq, mag;
    bool single_point_track = false;

    assert( offset < t->history.size() && offset > 0 );
    
    curr = t->history[offset];
    prev = t->history[offset-1];

    // this is how many samples we synthesize
    diff_t = (t_TAPUINT)(time_stretch*curr.time) - (t_TAPUINT)(time_stretch*prev.time);
    // this is the frequency difference between last 2 frames
    diff_f = freq_warp * (curr.freq - prev.freq);
    // this is the magnitude differnce between last 2 frames
    diff_m = curr.p.mag - prev.p.mag;
    // this is how much to change the frequency per sample
    delta_f = diff_f / (double)diff_t;
    // this is how much to change the magnitude per sample
    delta_m = diff_m / (double)diff_t;
    // delta phase
    delta_p = 2.0 * PIE / (double)BirdBrain::srate();
    // the frequency at given sample
    freq = freq_warp * prev.freq;
    // the magnitude at given sample
    mag = prev.p.mag;
    // pass the number of samples generated back
    frame.wsize = diff_t;

    if( diff_t > frame.wlen ) // are you sure we aren't off by one?
    {
        frame.alloc_waveform( diff_t % 2 ? diff_t + 1 : diff_t );
    }

    // the very beginning of the track
    if( offset == 1 )
    {
        // start from silence
        mag = 0.0;
        delta_m = curr.p.mag / (double)diff_t;
        // start from 0
        t->phase = 0.0;

        // both start and end of the track
        if( t->history.size() == 2 )
        {
            delta_m = 2 * prev.p.mag / (double)diff_t;
            delta_f *= 2;
            single_point_track = true;
        }
    }
    // the end of the track
    else if( offset == t->history.size() - 1 )
    {
        // fade to silence
        mag = prev.p.mag;
        delta_m = -prev.p.mag / (double)diff_t;
        freq = freq_warp * prev.freq;
        delta_f = -diff_f / (double)diff_t; // this used to be delta_f = -diff_f / (double)diff_t; but I didn't know why
                                           // so I changed it. :)  now we changed it back to -  still don't know why
										   // anyway, diff_f should be 0 b/c this is freqpolar halt from synthesize()
    }

    for( int j = 0; j < diff_t; j++ )
    {
        frame.waveform[j] += 1.25 * PIE * mag * sin( t->phase );
        freq += delta_f;
        mag += delta_m;
        t->phase += freq * delta_p; // deep

        // tracks with just one frame
        if( j == diff_t / 2 && single_point_track )
        {
            delta_f = -delta_f;
            delta_m = -delta_m;
        }
    }
}


// DELETE ASAP

//-----------------------------------------------------------------------------
// name: init()
// desc: 
//-----------------------------------------------------------------------------
void SynFast::init()
{
    cur_time = 0;
	death_bed = 0;
}


//-----------------------------------------------------------------------------
// name: reset()
// desc: stuff that really needs to be done on a rewind
//-----------------------------------------------------------------------------
void SynFast::reset()
{
    for( int t = 0; t < tracks_info.size(); t++ )
    {
        TrackInfo * tif = &(tracks_info[t]); 
        tif->freq = tif->mag = tif->ratio_t = tif->diff_t = tif->histind = 0;
    }
    cur_time = 0;
	death_bed = 0; 
}


//-----------------------------------------------------------------------------
// name: setTracks()
// desc: crash
//-----------------------------------------------------------------------------
void SynFast::setTracks( std::vector<Track *> me )
{
    tracks = me; 
    tracks_info.reserve( me.size() ); 
    tracks_info.clear();
    for( int t = 0; t < me.size(); t++ )
    {
        TrackInfo tif; 
        tif.start_time = me[t]->start; 
        tif.end_time = me[t]->end; 
        tif.histind = 0; 
        tif.freq = 0; 
        tif.mag = 0; 
        tif.diff_t = 0; 
        tif.ratio_t = 0; 
        tif.single_point_track = false;
        tracks_info.push_back( tif ); 
    }
    cur_time = 0; 
	death_bed = 0;
}


//-----------------------------------------------------------------------------
// name: synthesize()
// desc: synthesize frame's wsize worth of sound (ignore tracks)
//-----------------------------------------------------------------------------
void SynFast::synthesize( Frame &frame )
{
    t_TAPUINT tocompute = frame.wsize; 
    if( tracks.empty() )
    {
        BB_log( BB_LOG_INFO, "SynFast::sythesize: no tracks" ); 
        return;
    }

    Track * tr; 
    TrackInfo trin; 
    frame.zero(); 

    // find start time and end times by taking min/max of start/end times for all tracks
	death_bed = BirdBrain::hop_size();
    t_TAPTIME start_time = tracks_info[0].start_time;
    t_TAPTIME end_time = tracks_info[0].end_time;
    for( int x = 1; x < tracks_info.size(); x++ ) {
        if( tracks_info[x].start_time < start_time )
            start_time = tracks_info[x].start_time;
        if( tracks_info[x].end_time > end_time )
            end_time = tracks_info[x].end_time;
    }
    if( cur_time < start_time )
        cur_time = start_time; 
    if( cur_time >= end_time + death_bed )
    {
        frame.wsize = 0;
        return;
    }

    // do synthesis, track by track
    for( int i = 0; i < tracks.size(); i++ )
    {
        tr = tracks[i]; 
        trin = tracks_info[i];
        // how to make it start at the right time?  (using cur_time)
        if( time_stretch * trin.start_time <= time_stretch * cur_time + frame.wsize 
            && time_stretch * (trin.end_time + death_bed) > time_stretch * cur_time )
        {
            if( synframe.wlen < frame.wsize )
                synframe.alloc_waveform( frame.wsize + frame.wsize % 2 );
            int offset = (int)(time_stretch * (trin.start_time - cur_time)) > 0 ? 
                         (int)(time_stretch * (trin.start_time - cur_time)) : 0; 
            synframe.wsize = frame.wsize - offset;
			synhelp( synframe, i );
			frame.add( synframe, 0, offset );
        }   
    }
    cur_time += frame.wsize / time_stretch;
}


//-----------------------------------------------------------------------------
// name: synhelp()
// desc: synthesize frame.wsize amount of given track 
//-----------------------------------------------------------------------------
void SynFast::synhelp( Frame &frame, int trackind )
{
    if( trackind < 0 || trackind >= tracks.size() )
    {
        BB_log( BB_LOG_INFO, "SynFast::synhelp: invalid track index" );
        return;
    }
    
    t_TAPUINT tocompute = frame.wsize;
    int j = 0; 
    
    freqpolar prev, next;
    TrackInfo * ti = &tracks_info[trackind];
    Track * t = tracks[trackind];
    if( t == NULL || t->history.empty() )
    {
        BB_log( BB_LOG_INFO,"SynFast::synhelp: nothing to synthesize (weird); returning defeated" );
        return;
    }

    double diff_f, diff_m, delta_f, delta_m, delta_p, diff_t;

    frame.zero();

    while( j < tocompute )
    {
        prev = t->history[ti->histind];
        if( ti->histind < t->history.size() - 1 )
            next = t->history[ti->histind+1]; 
        else {
            next = prev;
            next.p.mag = 0.f;
            next.time += death_bed; // hop_size, 128
		}

        // this is how many samples are left till the next point in history, without time stretching
        ti->diff_t = (next.time - prev.time) - (ti->ratio_t * (next.time - prev.time)); 
        // this is how many samples are left till the next point in history, with time stretching
        diff_t = time_stretch * ti->diff_t; 
		//diff_t = (double)((t_TAPUINT)diff_t); // is this better or worse? (seems worse for chirploops, better for firework whistle loops)
		BB_log( BB_LOG_FINE, "j: %i, tocompute: %i, diff_t: %f, ti->diff_t: %f, ti->ratio_t: %f", j, tocompute, diff_t, ti->diff_t, ti->ratio_t ); 
        // this is the frequency difference between last 2 frames, with frequency warping
        diff_f = freq_warp * next.freq - ti->freq;
        // this is the magnitude differnce between last 2 frames
        diff_m = next.p.mag - ti->mag;
        // this is how much to change the frequency per sample
        delta_f = diff_f / diff_t;
        // this is how much to change the magnitude per sample
        delta_m = diff_m / diff_t;
        // delta phase
        delta_p = 2.0 * PIE / (double)BirdBrain::srate();

        // the very beginning of the track
        if( ti->histind == 0 && ti->ratio_t == 0 )
        {
            // start from silence-
            ti->mag = 0.0;
            delta_m = next.p.mag / (double)diff_t;
            // start from 0
            t->phase = 0.0;
            // reset frequency
            ti->freq = freq_warp * prev.freq; // prev being t->history[0]
            diff_f = freq_warp * next.freq - ti->freq;
            delta_f = diff_f / (double)diff_t; 

            // both start and end of the track
            if( t->history.size() == 1)
            {
                delta_m = 2 * prev.p.mag / (double)diff_t;
                delta_f *= 2;
                ti->single_point_track = true;
            }
        }
        // just the end of the track
        else if( ti->histind == t->history.size() - 1 /*&& ti->ratio_t == 0*/ )
        {
            // fade to silence
            delta_m = -ti->mag / (double)diff_t;
            delta_f = -diff_f / (double)diff_t;
        }
        while( j < tocompute && diff_t > 0 )
        {
            //fprintf( stderr, " %f", ti->mag );
            frame.waveform[j] += 1.25 * PIE * ti->mag * sin( t->phase );
            ti->freq += delta_f;
            ti->mag += delta_m;
            t->phase += ti->freq * delta_p; // deep

            // tracks with just one frame
            if( ::fabs(ti->ratio_t - 0.5f) < 0.0001 && ti->single_point_track && delta_m >= 0 )
            {
                delta_f = -delta_f;
                delta_m = -delta_m;
            }

            diff_t--;
            ti->diff_t = diff_t / time_stretch;
            ti->ratio_t = ((next.time - prev.time) - ti->diff_t) / (next.time - prev.time); // check rounding stuff
            j++;
        }
		//BB_log(BB_LOG_FINE, "track: %d history: %d tocompute: %d j: %d diff_t: %f", trackind, ti->histind, tocompute, j, diff_t);
        if( diff_t <= 0 ) {
            if( ti->histind < t->history.size() - 1 ) {
                ti->histind++;
                ti->ratio_t = 0; 
                //fprintf (stderr, "\n" );
            }
            else {
                // done with track; reset things
                ti->freq = ti->mag = ti->ratio_t = ti->diff_t = ti->histind = 0;
                break;
            }
        }
    } // end of computations
	
	// update track's own history index counter for group face
	t->current_syn_index = ti->histind; 
}

