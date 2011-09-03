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
// file: taps_birdbrain.h
// desc: taps birdbrain utilities
//
// author: Ananya Misra (amisra@cs.princeton.edu)
//         Ge Wang (gewang@cs.princeton.edu)
//         Perry R. Cook (prc@cs.princeton.edu)
// date: Autumn 2004
//-----------------------------------------------------------------------------
#ifndef __BIRDBRAIN_H__
#define __BIRDBRAIN_H__

#include "taps_def.h"

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <assert.h>
#include <vector>
#include <string>


class BirdBrain
{
public:
    static t_TAPUINT srate() { return our_srate; }
    static t_TAPUINT fft_size() { return our_fft_size; }
    static t_TAPUINT wnd_size() { return our_wnd_size; }
    static t_TAPUINT syn_wnd_size() { return our_syn_wnd_size; }
    static t_TAPUINT hop_size() { return our_hop_size; }
    static t_TAPUINT max_tracks() { return our_max_tracks; }
    static t_TAPFLOAT over_tracking() { return our_over_tracking; }
	static t_TAPBOOL rtaudio_blocking() { return our_rtaudio_blocking; }
	static t_TAPUINT rtaudio_buffer_size() { return our_rtaudio_buffer_size; }
	static t_TAPSINGLE record_buffer_size() { return our_record_buffer_size; }
    static int last_hop() { return our_last_hop; }
    static float freq_min() { return our_freq_min; }
    static float freq_max() { return our_freq_max; }
    static t_TAPUINT frame_rate() { return our_frame_rate; }
    static t_TAPBOOL use_gui() { return our_use_gui; }
	static t_TAPBOOL white_bg() { return our_white_bg; }
    static std::string get_start_dir() { return our_start_dir; }
    static void goto_start_dir();
    static void goto_dir( const std::string & path );

    static void ola( SAMPLE *buffer, SAMPLE *to_add, int shift, int size );
	static void ola( SAMPLE *buffer, SAMPLE *to_add, int shift, int buffer_size, int to_add_size);
    static void filter( const SAMPLE *x, const SAMPLE *winxp, SAMPLE *y, int len, float *B, int lenb, float *A, int lena );
    static void scale( SAMPLE *buffer, t_TAPUINT len, float factor );
    static void scale_fft( SAMPLE *buffer, t_TAPUINT len, t_TAPUINT fft_size = fft_size(), t_TAPUINT wnd_size = wnd_size() );
    static void scale_ifft( SAMPLE *buffer, t_TAPUINT len, t_TAPUINT fft_size = fft_size(), t_TAPUINT wnd_size = wnd_size() );

    static const char * getbase( const char * str );
    static std::string getpath( const char * str );
    static std::string getname( const char * str );
    static std::string get_part_of_name( const char * str );
    static std::string toString( long num );

public:
    // audio
    static t_TAPUINT our_srate;
    static t_TAPUINT our_fft_size;
    static t_TAPUINT our_wnd_size;
    static t_TAPUINT our_syn_wnd_size;
    static t_TAPUINT our_hop_size;
    static t_TAPUINT our_max_tracks;
    static t_TAPFLOAT our_over_tracking;
	static t_TAPBOOL our_rtaudio_blocking;
	static t_TAPUINT our_rtaudio_buffer_size;
    static t_TAPSINGLE our_record_buffer_size;
	static int our_last_hop;
    static float our_freq_min;
    static float our_freq_max;
    // graphics
    static t_TAPUINT our_frame_rate;
    static t_TAPBOOL our_use_gui;
	static t_TAPBOOL our_white_bg;
    // start directory
    static std::string our_start_dir;
    static std::string our_last_open_dir;
    static std::string our_last_save_dir;

};


// complex type
struct complex { float re ; float im ; };
// polar type
struct polar { float mag ; float phase ; };

// complex absolute value
#define cmp_abs(x) ( sqrt( (x).re * (x).re + (x).im * (x).im ) )
// magnitude
#define cmp_mag(x) cmp_abs(x)
// phase
#define cmp_phase(x) ( atan2((double)(x).im, (double)(x).re) )

// function
struct xtoy{ virtual double y( double x ) = 0; };

// line (for threshold, for example)
struct line : public xtoy
{ double slope; double intercept;
  virtual double y( double x ) { return x * slope + intercept; }
};

// freq + polar
struct freqpolar
{
    float freq;
    polar p;
    // questionable
    t_TAPTIME time; // danger comment: timeout
    // even more questionable
    t_TAPUINT bin;
    // no further comment
    bool isMatched;
};

// Frame in frequency domain
struct Frame
{
    union
    {
        // time-domain
        float * waveform;
        // freq-domain real + imag pairs
        complex * cmp;
    };
    // mag + phase pairs
    polar * pol;
    // corresponding frequencies to bins
    float * freqs;
    // number of elements in each of cmp, pol, freqs
    t_TAPUINT len;
    // capacity of waveform
    t_TAPUINT wlen;
    // how much data is in waveform
    t_TAPUINT wsize;
    // time associated with this frame
    t_TAPTIME time;  // danger comment: timeout

    // constructor
    Frame() { this->init(); }
    Frame( const Frame & rhs ) { this->init(); *this = rhs; }
    ~Frame() { this->clear(); }
    
    // alloc
    void alloc_waveform( int length )
    {
         // even numbers
         assert( length % 2 == 0 );

         if( waveform ) delete [] waveform;
         waveform = new float[length];
         memset( waveform, 0, sizeof(float) * length );
         wlen = length;
         len = length / 2;

         if( pol ) alloc_pol();
         if( freqs ) alloc_freqs();
    }

    // alloc polar
    void alloc_pol()
    {
         assert( waveform != NULL );

         if( pol ) delete [] pol;
         pol = new polar[len];
         memset( pol, 0, sizeof(polar) * len );
    }

    // alloc freqs
    void alloc_freqs()
    {
        assert( waveform != NULL );

        if( freqs ) delete [] freqs;
        freqs = new float[len];
        memset( freqs, 0, sizeof(float) * len );
    }

    // convert to polar
    void cmp2pol()
    {
        if( !pol ) alloc_pol();
        for( t_TAPUINT i = 0; i < len; i++ )
        {
            pol[i].mag = cmp_mag(cmp[i]);
            pol[i].phase = cmp_phase(cmp[i]);
        }
    }

    // convert to complex
    void pol2cmp()
    {
        assert( pol != NULL );  
        if( !cmp ) alloc_waveform( 2 * len );
        for( t_TAPUINT i = 0; i < len; i++ )
        {
            cmp[i].re = pol[i].mag * cos(pol[i].phase);
            cmp[i].im = pol[i].mag * sin(pol[i].phase);
        }
    }

    // map bins to frequencies
    void bins2freqs( double srate )
    {
        // use nyquist
        srate /= 2.0;
        if( !freqs ) alloc_freqs();
        for( t_TAPUINT i = 0; i < len; i++ )
        {
            freqs[i] = (double)i/len * srate;
        }
    }

    // clear
    void clear()
    {
        if( cmp ) delete [] cmp;
        if( pol ) delete [] pol;
        if( freqs ) delete [] freqs;
        cmp = NULL; pol = NULL; freqs = NULL;
        wlen = len = 0;
    }
    
    // zero
    void zero()
    {
        if( cmp ) memset( cmp, 0, len * sizeof(complex) );
        if( pol ) memset( pol, 0, len * sizeof(polar) );
        if( freqs ) memset( freqs, 0, len * sizeof(float) );
    }

    // init
    void init()
    {
        cmp = NULL;
        pol = NULL;
        freqs = NULL;
        wlen = wsize = len = 0;
        time = 0;
    }
    
    // deep copy
    Frame & operator =( const Frame & rhs )
    {
        this->clear();
        wlen = rhs.wlen;
        wsize = rhs.wsize;
        len = rhs.len;
        time = rhs.time;
        
        if( rhs.cmp )
        {
            cmp = new complex[len];
            memcpy( cmp, rhs.cmp, len * sizeof(complex) );
        }
        if( rhs.pol )
        {
            pol = new polar[len];
            memcpy( pol, rhs.pol, len * sizeof(polar) );
        }
        if( rhs.freqs )
        {
            freqs = new float[len];
            memcpy( freqs, rhs.freqs, len * sizeof(float) );
        }

        return *this;
    }

    // add two frames
    void add( const Frame &to_add, int to_add_start_pos, int this_start_pos )
    {
		assert( this_start_pos + to_add.wsize <= this->wlen );

        for( int i = to_add_start_pos; i < to_add.wsize; i++ )
        {
            this->waveform[this_start_pos + i] += to_add.waveform[i];
        }
    }

    // shift waveform right or left by some number of samples
    void shift_waveform( int from, int to )
    {
        // check bounds
        assert( from >= 0 && from < wlen && to >= 0 && to < wlen );
        // at least one of from and to must be zero, otherwise it's kind of undefined
        assert( from == 0 || to == 0 );
        // copy / shift
        int i;
        if( from > to ) // shift left
        {
            for( i = 0; i < wlen - from; i++ )
                waveform[to + i] = waveform[from + i];
            memset( waveform + (to + wlen - from), 0, sizeof(float) * (from - to) );
        }
        else if( to > from ) // shift right
        {
            for( i = wlen - to - 1; i >= 0; i-- )
                waveform[to + i] = waveform[from + i];
            memset( waveform, 0, sizeof(float) * to );
        }
    }
};

// sinusoidal track
struct Track
{
    // id of the track
    int id;
    // state
    enum { ACTIVE = 1, INACTIVE = 2, AMBIGUOUS = 3 };
    int state;
    // trajectory of the track
    std::vector<freqpolar> history; // freqpolartime
    // start time (same type of thing as Frame::time)
    t_TAPTIME start; // danger comment: timeout
    // end time
    t_TAPTIME end; // danger comment: timeout
    // phase offset for synthesis
    double phase;
    // marker used by analysis
    t_TAPUINT historian;
    // keeping track of current history index for synthesis, for group face
	t_TAPUINT current_syn_index; 

    // constructor
    Track();    
	
	// destructor
    ~Track();

    // invert history
    void invert();
};


struct TimeGrid
{
    // time of a given frame
    t_TAPTIME frametime;
    // tracks that have a point in that frame
    std::vector<Track *> tracks;
    // index into history of corresponding track in tracks vector
    std::vector<int> trackinds; 
};


// a sinusoidal event
// what other info would be useful?
class SinEvent
{
public:
    SinEvent();
    SinEvent( const SinEvent &me ); // copy constructor
    ~SinEvent();
    
    SinEvent &operator=( const SinEvent &rhs ); // assignment
    void clear(); // yeah

    // add a track to the event
    bool add_track( Track * track );
    // compare harmonics
    bool cmp_harm( Track * track, float error, float overlap_amt );
    // compare common frequency & amplitude modulation
    bool cmp_mod( Track * track, float f_error, float a_error, float overlap_amt );
    // compare common onset & offset
    bool cmp_onoff( Track * track, float on_error, float off_error );

    std::vector< Track * > event_tracks;
    Track * average; 
    t_TAPTIME start; // earliest start time from all tracks
    t_TAPTIME end;   // latest end time from all tracks
};


// filter class
class Filter
{
public:
    Filter();
    ~Filter();

    void init( float *B_, int lenb_, float *A_, int lena_ );
    void clear();
    void apply_filter( Frame &x );
    void clear_sigs(); // clears the windows/signals but retains coefficients
    
    // filter coefficients or whatever they're called
    float *A;
    float *B;
    int lena, lenb;

    // signals
    SAMPLE *winxp, *ys;
    int len;
};


// levels
#define BB_LOG_CRAZY            10 // set this to log everything
#define BB_LOG_FINEST           9
#define BB_LOG_FINER            8
#define BB_LOG_FINE             7
#define BB_LOG_CONFIG           6
#define BB_LOG_INFO             5
#define BB_LOG_WARNING          4
#define BB_LOG_SEVERE           3
#define BB_LOG_SYSTEM           2
#define BB_LOG_SYSTEM_ERROR     1
#define BB_LOG_NONE             0  // set this to log nothing

// functions
void BB_log( int, const char *, ... );
void BB_setlog( int );
void BB_pushlog();
void BB_poplog();

// actual level
extern int g_loglevel_bb;
// macro to compare
#define BB_DO_LOG(x) ( x <= g_loglevel_bb )

// number of features in feature vector
// (still need to figure out where this actually belongs)
// (didn't seem to fit in birdbrain class, but needs to be available 
// in ui_search and ui_library and probably other places)
extern int g_num_features;


#endif
