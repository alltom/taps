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
// file: taps_synthesis.h
// desc: taps sinusoidal synthesis
//
// author: Ananya Misra (amisra@cs.princeton.edu)
//         Ge Wang (gewang@cs.princeton.edu)
//         Perry R. Cook (prc@cs.princeton.edu)
// date: Autumn 2004
//-----------------------------------------------------------------------------
#ifndef __SYNTHESIS_H__
#define __SYNTHESIS_H__

#include "taps_birdbrain.h"


class Synthesis
{
public:
    // do it.  input == FFT frame, output == tracks and residue
    virtual void synthesize( Frame & frame ) = 0;
    virtual void init() { } 
    virtual void reset() { }
    virtual void setTracks( std::vector<Track *> me ) { tracks = me; }; 

public:
    // parameters
    int max_tracks; // number of tracks to synthesize

public:
    // constructor and destructor
    Synthesis();
    virtual ~Synthesis();
    
public: 
    double time_stretch;
    double freq_warp;

protected:
    Frame f;
    std::vector<Track *> tracks;
};


class Syn : public Synthesis
{
public:
    virtual void synthesize( Frame &frame );
    void synhelp( Frame &frame, Track *itrack, int offset );
    
    virtual void init();
    virtual void setTracks( std::vector<Track *> me );

public:
};


struct TrackInfo
{
    t_TAPTIME start_time;
    t_TAPTIME end_time;
    t_TAPUINT histind; // last history index  
    double freq; // last frequency value
    double mag; // last magnitude value
    t_TAPTIME diff_t; // diff_t at end of last step
    double ratio_t; // how far it is between history points
                   // time of history[histind+1] =  diff_t + history[histind] + ratio_t * (history[histind+1]-history[histind])
    bool single_point_track;
};


class SynFast : public Synthesis
{
public:
    virtual void synthesize( Frame &frame ); 
    void synhelp( Frame &frame, int trackind ); 
    virtual void setTracks( std::vector<Track *> me ); 
    virtual void init(); 
    virtual void reset(); 

public:
    std::vector<TrackInfo> tracks_info;
    t_TAPTIME cur_time; // without stretching
    Frame synframe; // for synhelp
	t_TAPUINT death_bed; // extra samples for end of track (like syn_wnd_size, but more exact)
};


class SynSndObj : public Synthesis
{
public:
    virtual void synthesize( Frame &frame );
    virtual void init();

protected:
    int m_tracks;
    float * m_freqs;
    float * m_amps;
    float * m_phases;
};



#endif
