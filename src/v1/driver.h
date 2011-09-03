#ifndef __DRIVER_H__
#define __DRIVER_H__

#include "analysis.h"
#include "synthesis.h"
#include "sceptre.h"
#include "audicle_def.h" // for <fstream> (need it in header because of ifstream class variable)

// libsndfile
#ifndef __USE_SNDFILE_PRECONF__
#include <sndfile.h>
#else
#include "util_sndfile.h"
#endif


class Driver
{
public:
    Driver();
    ~Driver();

public:
    // set window size and fft size
    void set( uint wnd, uint fft, uint wnd_type = 1 );
	// set the analysis object
	void set( Analysis * ana );
	// get the analysis object
	Analysis * ana() const;
	// open file for reading
	bool open( const std::string & filename );

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
    uint now() const;
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
    const Frame * get_frame( uint time_sample );
    // get first pass residue
    const Frame * get_fake_residue( uint time_sample );
    // get window
    const Frame * get_window();
	// write out the actual residue
	void write_res( uint count = 0, bool clip_only = false );
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
    uint wnd_size;
    uint fft_size;
    uint time_now;

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
	TIME * m_times; 
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

	SNDFILE * sf;
	SF_INFO sf_info;

	SNDFILE * sf_out;
	SF_INFO sf_info_out;

	SNDFILE * sf_res;
	SF_INFO sf_info_res;

    SAMPLE * sf_buffer;
    uint sf_buffer_size;

	uint event_count;
};


Driver * run( char * filename, int start, int stop, float freq_min, float freq_max, xtoy * det_thresh, 
			 int min_points, int max_gap, float error_f, float noise_r, int num_tracks, int fft_size, int wnd_size,
			 float group_harm, float group_freq, float group_amp, float group_overlap, float group_on, 
			 float group_off, float group_minlen, bool group);

#endif
