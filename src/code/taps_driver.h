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
// file: taps_driver.h
// desc: taps analysis/synthesis driver
//
// author: Ananya Misra (amisra@cs.princeton.edu)
//         Ge Wang (gewang@cs.princeton.edu)
//         Perry R. Cook (prc@cs.princeton.edu)
// date: Autumn 2004
//-----------------------------------------------------------------------------
#ifndef __DRIVER_H__
#define __DRIVER_H__

// libsndfile
/*#ifndef __USE_SNDFILE_PRECONF__
#include <sndfile.h>
#else
#include "util_sndfile.h"
#endif*/

#include "ui_audio.h"
#include "taps_analysis.h"
#include "taps_synthesis.h"
#include "taps_sceptre.h"
#include "audicle_def.h" // for <fstream> (need it in header because of ifstream class variable)


class Driver
{
public:
    Driver();
    ~Driver();

public:
    // set window size and fft size
    void set( t_TAPUINT wnd, t_TAPUINT fft, t_TAPUINT wnd_type = 1 );
    // set the analysis object
    void set( Analysis * ana );
    // get the analysis object
    Analysis * ana() const;
    // open file for reading
    bool open( const std::string & filename );
	// open mic for reading
	bool open_mic(); 

public:
    // eat (do work)
    bool dine();
    // move in time
    bool drive( int hop );
    // drink (the vovot)
    bool drink();
    // honk
    bool honk( std::vector<Track *> & event, Frame & out );
    // stop driving (and shutdown)
    void brake();
    // preprocess
    void preprocess(); 

public:
    // get now
    t_TAPUINT now() const;
    // is the input done
    bool done( int endpt ) const;
    // get sf_info size
    int sndfile_size( ) const;
    // second pass result
    std::vector<Track *> & the_event();
    // after grouping
    std::vector<Track *> & cur_event();
    std::vector<Track *> & next_event();
    std::vector<Track *> & prev_event();
    // get fft frame associated with sample number
    const Frame * get_frame( t_TAPUINT time_sample );
    // get first pass residue
    const Frame * get_fake_residue( t_TAPUINT time_sample );
    // get window
    const Frame * get_window();
    // write out the actual residue
    void write_res( t_TAPUINT count = 0, bool clip_only = false );
    void write_res( std::string filename, bool clip_only = false );
	// read preprocessed files
    bool read_preprocessed( std::string ppfilename ); 
    // get file name from pp file name
    std::string get_filename( std::string filename ); 

    void test();

    int last_hop;
    std::string res_file_name;
    std::string out_file_name;
    std::string in_file_name;
    std::string path; 
    // time / frequency analysis boundaries:
    int m_start;
    int m_stop;
    float m_freq_min;
    float m_freq_max;
    bool m_group; // whether to get events

public: 
    // alternative to run
    bool cruise( int start, int stop, float freq_min, float freq_max, xtoy * det_thresh, 
             int min_points, int max_gap, float error_f, float noise_r, int num_tracks, 
             float group_harm, float group_freq, float group_amp, float group_overlap, float group_on, 
             float group_off, float group_minlen, bool group);

protected:
	void write_res( bool clip_only );

protected:
    t_TAPUINT wnd_size;
    t_TAPUINT fft_size;
    t_TAPUINT time_now;

    // the analysis object
    Analysis * analysis;
    // the synthesis object
    Synthesis * synthesis;
    // the frame
    Frame f;
    Frame f_syn;
    // the window
    Frame window;
    // the inverse window
    Frame window_inv;
    // the hann window
    Frame window_han;
    // the big window
    Frame window_big;
    // the residue
    Frame residue;  
    // the overlap/add buffer for sinuisoidal resynthesis
    Frame ola_syn;
    // the overlap/add buffer for residue from analysis
    Frame ola_res;
    // vector of tracks
    std::vector<Track *> tracks;
    // vector of actual tracks (fix this later...)
    std::vector<Track *> realtracks;
    // vector of FFT frames
    std::vector<Frame *> fft_frames;
    // vector of first pass residue
    std::vector<Frame *> fakeresidue;
    // vector of inpute peaks (for preprocessed input)
    std::vector<Track *> peak_frames;
    // vector of vector of tracks
    // std::vector<std::vector<Track *> > vovot;
    // vector of sinusoidal events
    std::vector<SinEvent> events;
    // size and start location, for reading peaks file, set in read_preprocessed()
    int pp_framesize; 
    int pp_framestart;
    // ditto for fft frames
    int fft_framesize; 
    int fft_framestart;
    // index of current frame being read from pp and fft frame file (for preprocessed input)
    int frame_number;
    // times of frames, for preprocessing...
    t_TAPTIME * m_times; 
    // number of frames...
    int m_frames;
    // file pointers for preprocessed input
    FILE * ppfin; 
    FILE * fftfin; 

    // DC blocker
    Filter dcbloke;
    Filter dcunbloke; 

protected: // helper functions + file
    bool read_window( Frame & frame );
    float signal_scale;

	AudioSrcBuffer * m_input; 
	
	//SNDFILE * sf;
	//SF_INFO sf_info; 
	
    SNDFILE * sf_out;
    SF_INFO sf_info_out;

    SNDFILE * sf_res;
    SF_INFO sf_info_res;

    SAMPLE * sf_buffer;
    t_TAPUINT sf_buffer_size;

    t_TAPUINT event_count;
};


Driver * run( char * filename, int start, int stop, float freq_min, float freq_max, xtoy * det_thresh, 
             int min_points, int max_gap, float error_f, float noise_r, int num_tracks, int fft_size, int wnd_size,
             float group_harm, float group_freq, float group_amp, float group_overlap, float group_on, 
             float group_off, float group_minlen, bool group);

#endif
