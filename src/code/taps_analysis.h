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
// file: taps_analysis.h
// desc: taps sinusoidal analysis
//
// author: Ananya Misra (amisra@cs.princeton.edu)
//         Ge Wang (gewang@cs.princeton.edu)
//         Perry R. Cook (prc@cs.princeton.edu)
// date: Autumn 2004
//-----------------------------------------------------------------------------
#ifndef __ANALYSIS_H__
#define __ANALYSIS_H__

#include "taps_birdbrain.h"

// processing modes
enum {
    PREPROCESS_OUT, // preprocess file
    PREPROCESS_IN,  // handle preprocessed information
    NORMAL          // normal
};

class Analysis
{
public:
    // do it.  input == FFT frame, output == tracks and residue
    virtual void analyze( 
        const Frame & frame,
        std::vector<Track *> & otracks,
        Frame & oresidue,
        Track * ppin = NULL
    ) = 0;

    virtual void get_tracks(
        std::vector<Track *> & itracks, 
        std::vector<Track *> & otracks
    ) = 0;

    virtual void get_events(
        std::vector<Track *> & itracks,
        std::vector<SinEvent> & oevents 
    ) = 0;

    virtual void init() { }
    virtual void clear() { }
    virtual void get_res( std::vector <Track *> &itracks, Frame &resframe ) { }
    virtual void cache_thresh( t_TAPUINT len );

public:
    // parameters
    bool tracking;
    int max_tracks;
    xtoy * threshold;
    double * thresh_cache;
    t_TAPUINT frame_size;
    int processing_mode; 
    //double threshold;
    int minpoints;
    int maxgap;
    float error_f;  // ratio (of peak frequencies in adjacent frames) above which implies track continuation (.8 was default)
    float noise_ratio;  // ratio (of max/average per frame) below which implies frame is noise (3.1 was default)
    //SAMPLE sig_max;
    // grouping parameters:
    float harm_error;
    float freq_mod_error;
    float amp_mod_error;
    float onset_error; // seconds
    float offset_error; // seconds
    float min_overlap_frac;
    float min_event_length; // seconds

public:
    // constructor and destructor
    Analysis();
    virtual ~Analysis();

public:
    t_TAPUINT wnd_size;
    
protected:
    Frame f;
};

class AnaSndObj : public Analysis
{
public:
    // do it.  input == FFT frame, output == tracks and residue
    virtual void analyze(
        const Frame & frame,
        std::vector<Track *> & otracks,
        Frame & oresidue, 
        Track * ppin = NULL
    );

    virtual void get_tracks(
        std::vector<Track *> & itracks, 
        std::vector<Track *> & otracks
    );

    virtual void get_events(
        std::vector<Track *> & itracks,
        std::vector<SinEvent> & oevents ) {}


    virtual void init();

public:
    virtual ~AnaSndObj();

    
protected:
    int* m_maxix;     // max peak locations
    float* m_binmax;  // peak bin indexes
    float* m_magmax;  // peak mags

    // no we don't want
    bool* m_contflag; // continuation flags
    Track ** m_tracks; // tracks
    float** m_bndx;  // bin indexes

    float** m_pkmags;  // peak mags
    float** m_adthresh;  // thresholds
    unsigned int** m_tstart;  // start times 
    unsigned int** m_lastpk; // end times
    unsigned int** m_trkid; // track ids

    float* m_phases; // phases
    float* m_freqs;  // frequencies
    float* m_mags;   // magnitudes
    float* m_bins;   // track bin indexes
    int* m_trndx;    // track IDs

    float* m_diffs;    // differences

    int m_num_tracks;      // tracks in a frame
    int m_prev;         
    int m_cur;
    int m_accum;       // ID counter
    unsigned int m_timecount;
    int m_minpoints;     // minimun number of points in track
    int m_maxgap;     // max gap (in points) between consecutive points

    int m_vecsize;

    // the next track id
    unsigned long m_next_tid;
};

class AnaPeaksFFT : public Analysis
{
public:
    // do it.  input == FFT frame, output == tracks and residue
    virtual void analyze( 
        const Frame & frame,
        std::vector<Track *> & otracks,
        Frame & oresidue,
        Track * ppin = NULL
    );

    virtual void get_tracks(
        std::vector<Track *> & itracks, 
        std::vector<Track *> & otracks
    );

    virtual void get_events(
        std::vector<Track *> & itracks,
        std::vector<SinEvent> & oevents );

    virtual void init();
    virtual void clear(); 
    virtual void get_res( std::vector <Track *> &itracks, Frame &resframe );

    // non virtual
    void verify( std::vector <Track *> &itracks );
    
    void analyze_preprocess_in( 
        std::vector<Track *> & otracks,
        Frame & oresidue, 
        Track * ppin
    );
    
    void analyze_preprocess_out(
        std::vector<Track *> & otracks, 
        Frame & oresidue 
    ); 

public:
    //AnaPeaksFFT( bool _tracking, int _mt, double _thresh );
    virtual ~AnaPeaksFFT();

protected:
    //int *peaks;
    //float *magns;
    std::vector<Track *> tracks;
    // the next track id
    unsigned long m_next_tid;
};




#endif
