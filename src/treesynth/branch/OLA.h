// Random OLA from ICMC 07 (Frojd & Horner)

#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <assert.h>

// libsndfile
#ifndef __USE_SNDFILE_PRECONF__
#include <sndfile.h>
#else
#include "util_sndfile.h"
#endif

#define OLAR_BIG_BUFFER_SIZE  100000
#define OLAR_BIG_BUFFER_COUNT 8
#define OLAR_BUFFER_SIZE	2048;

class OlaRandom
{
public:
	OlaRandom(); 
	~OlaRandom();

	void initialize( char filename[], int datasize, float randomness, int mindist, bool scale_amp );
	int next_segment(); 

	int m_audio_cb( char * buffer, int buffer_size, void * user_data );
	bool audio_initialize( int (*audio_cb) (char *, int, void *), int srate = 44100 );
	int ReadSoundFile( char filename[] );
	int WriteSoundFile( char filename[], float * data, int datasize );

	bool write_to_buffer;
	bool write_to_file;
	char ifilename[1024]; // input
	
	char ofilename[1024]; // output

protected:
	float * origbuffer; // input
	int origsize; // input
	SF_INFO readinfo, writeinfo; // input, output
	SNDFILE * sfread; // input
	SNDFILE * sfwrite; // output
	int m_write_index; // output
	int m_read_index; // output
	float m_big_buffer[OLAR_BIG_BUFFER_COUNT][OLAR_BIG_BUFFER_SIZE]; // output
	int m_buffer_samples[OLAR_BIG_BUFFER_COUNT]; // output: how many samples have been written into each of the big buffers
	float m_ola_buffer[OLAR_BIG_BUFFER_SIZE]; // output
	float * m_audio_begin; // output
	float * m_audio_end; // output
	int m_last_buffer_size;
	int m_next_buffer_size; 
	int m_last_start_index;
	int m_hopsize; // hop size
	bool m_top; // top or bottom row (just for randomizing...)
	int m_rt_buffer_size; 
	int m_data_count;
	int m_max_data_count;
	int m_ready, m_callbacking; // ?
	int m_srate;
	float m_window[OLAR_BIG_BUFFER_SIZE]; // windowing for overlap add (max 2 seconds @ 44100 srate)
	int m_winsize;
	float m_data_buffer[OLAR_BIG_BUFFER_SIZE];
	float m_swap_buffer[OLAR_BIG_BUFFER_SIZE];

	float randomness; // 0 to 1
	bool scale_amp; 
	int mindist; 
	int segsize; 
};
