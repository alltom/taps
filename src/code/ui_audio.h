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
// name: ui_audio.cpp
// desc: birdbrain ui audio
//
// authors: Ananya Misra (amisra@cs.princeton.edu)
//          Ge Wang (gewang@cs.princeton.edu)
//          Perry R. Cook (prc@cs.princeton.edu)
// date: Winter 2004
//-----------------------------------------------------------------------------
#ifndef __UI_AUDIO_H__
#define __UI_AUDIO_H__

#ifndef __PLATFORM_WIN32__
#include "taps_birdbrain.h"
#endif

// libsndfile
#ifndef __USE_SNDFILE_PRECONF__
#include <sndfile.h>
#else
#include "util_sndfile.h"
#endif

#ifdef __PLATFORM_WIN32__
#include "taps_birdbrain.h"
#endif

#include "audicle_def.h"
#include "rtaudio.h"
#include "util_thread.h"
#include "ui_audiofx.h"
#include "taps_olar.h"

// libsamplerate
#ifdef SECRET_RABBIT_CODE
#include <samplerate.h>
#endif

// std
#include <map>
#include <vector>
#include <string>

// forward reference
class TreesynthIO;

//-----------------------------------------------------------------------------
// name: AudioSrc
// desc: ...
//-----------------------------------------------------------------------------
struct AudioSrc
{
    AudioSrc() { m_done = TRUE; m_stop_asap = FALSE; on = NULL; m_delete = TRUE; 
                 m_gain = 1.0f; m_pan = .5f; m_time_elapsed = 0; }
    virtual ~AudioSrc() { m_done = TRUE; if( on ) *on = FALSE; }
    virtual t_TAPBOOL stick( SAMPLE * buffer, t_TAPUINT num_frames ) = 0;
	virtual t_TAPBOOL mtick( SAMPLE * buffer, t_TAPUINT num_frames );
    virtual t_TAPBOOL reset() { m_done = FALSE; m_stop_asap = FALSE; return TRUE; }
    virtual t_TAPBOOL rewind() { m_time_elapsed = 0; return TRUE; }
    virtual t_TAPBOOL done() { return m_done; }
    virtual t_TAPBOOL playing() { return !m_done; }
    virtual const std::string & name() { return m_name; }
    virtual t_TAPBOOL forward( t_TAPUINT num_frames );

    virtual void set_gain( t_TAPSINGLE gain )
    { if( gain<0 ) gain = 0; if( gain>128 ) gain = 128; m_gain = gain; }
    virtual t_TAPSINGLE get_gain() const { return m_gain; }
    virtual void set_pan( t_TAPSINGLE pan )
    { if( pan<0 ) pan = 0; if( pan>1 ) pan = 1; m_pan = pan; }
    virtual t_TAPSINGLE get_pan() const { return m_pan; }
    
	virtual t_TAPTIME get_time_elapsed() { return m_time_elapsed; }

    t_TAPBOOL * on; 
    t_TAPBOOL m_done; // whether it's done (safe to delete) (not in use) (...)
    t_TAPBOOL m_stop_asap; // (ignore: whether it's not done) (the real comment: whether to stop asap ) 
    t_TAPBOOL m_delete;
    t_TAPSINGLE m_gain;
    t_TAPSINGLE m_pan;
	std::string m_name;
	t_TAPTIME m_time_elapsed; // time elapsed since it started playing

    static void mono2stereo( SAMPLE * myself, t_TAPUINT frames );
    static void stereo2mono( SAMPLE * myself, t_TAPUINT frames );
	static void mono2multi( SAMPLE * myself, t_TAPUINT frames, t_TAPUINT channels );
    static void multi2mono( SAMPLE * myself, t_TAPUINT frames, t_TAPUINT channels );
	static void mono2stereo( const SAMPLE * mono, t_TAPUINT frames, SAMPLE * stereo );
    static void stereo2mono( const SAMPLE * stereo, t_TAPUINT frames, SAMPLE * mono );
	static void mono2multi( const SAMPLE * mono, t_TAPUINT frames, t_TAPUINT channels, SAMPLE * multi );
	static void multi2mono( const SAMPLE * multi, t_TAPUINT frames, t_TAPUINT channels, SAMPLE * mono ); 
    static void pan_buf( SAMPLE * stereo, t_TAPUINT frames, t_TAPSINGLE where );
    static void gain_buf( SAMPLE * buffer, t_TAPUINT num_samples, SAMPLE gain );
};




//-----------------------------------------------------------------------------
// name: AudioSrcTest
// desc: ...
//-----------------------------------------------------------------------------
struct AudioSrcTest : public AudioSrc
{
    AudioSrcTest( t_TAPFLOAT freq ) : AudioSrc() { m_freq = freq; m_t = 0.0; }
    virtual t_TAPBOOL stick( SAMPLE * buffer, t_TAPUINT num_frames );

protected:
    t_TAPFLOAT m_freq;
    t_TAPFLOAT m_t;
};


//-----------------------------------------------------------------------------
// name: AudioSrcBuffer
// desc: audio sources that basically read from a buffer 
//		 (rather than generating samples)
//-----------------------------------------------------------------------------
struct AudioSrcBuffer : public AudioSrc
{
	AudioSrcBuffer() : AudioSrc() { }
	~AudioSrcBuffer() { }
	virtual t_TAPBOOL seek( t_TAPUINT where, t_TAPUINT how ) { return FALSE; }
	virtual t_TAPUINT frames() { return 0; }
	virtual t_TAPUINT srate() { return BirdBrain::srate(); }
	enum buffertype {BUFF_NONE, BUFF_FILE, BUFF_MIC, BUFF_FRAME}; 
	virtual buffertype type() { return BUFF_NONE; } 
	virtual t_TAPUINT current() { return 0; }
};


//-----------------------------------------------------------------------------
// name: AudioSrcFile
// desc: ...
//-----------------------------------------------------------------------------
struct AudioSrcFile : public AudioSrcBuffer
{
    AudioSrcFile( );
    virtual ~AudioSrcFile();

    virtual t_TAPBOOL mtick( SAMPLE * buffer, t_TAPUINT num_samples );
    virtual t_TAPBOOL stick( SAMPLE * buffer, t_TAPUINT num_frames );
    virtual t_TAPBOOL rewind();
    t_TAPBOOL open( const char * filename, t_TAPUINT start = 0, t_TAPUINT len = 0, 
					t_TAPBOOL scale = FALSE, int id = 0, t_TAPBOOL usecache = TRUE );
    t_TAPBOOL close();
    virtual t_TAPBOOL seek( t_TAPUINT where, t_TAPUINT how )
		{ return sf_seek( sf, where, how ) < sf_info.frames; }
	virtual t_TAPUINT frames() { return sf_info.frames; }
	virtual t_TAPUINT srate() { return sf_info.samplerate; }
	virtual buffertype type() { return BUFF_FILE; }
	virtual t_TAPUINT current() { return sf_seek( sf, 0, SEEK_CUR ); }
    const std::string & filename() { return m_filename; }
    const std::string & last_error() { return m_lasterror; }
    const SF_INFO & info() { return sf_info; }
    t_TAPUINT start() { return m_start; }
    t_TAPUINT end() { return m_end; }
    t_TAPUINT now() { return sf_seek( sf, 0, SEEK_CUR ); }
	// save to another file [fromIndex, toIndex) 
	t_TAPBOOL save(std::string & filename, t_TAPUINT fromIndex, t_TAPUINT toIndex, 
		t_TAPBOOL scale = TRUE);

#ifdef SECRET_RABBIT_CODE
	int samplerateconvert( const char * filename );
#endif

    t_TAPSINGLE * slide;
    bool * slide_locally;

protected:
    SAMPLE * m_buffer;
    t_TAPUINT m_buffer_size;
    SNDFILE * sf;
    SF_INFO sf_info;
#ifdef SECRET_RABBIT_CODE
	SRC_STATE * sr;
	float sr_gain;
#endif
    t_TAPUINT m_start;
    t_TAPUINT m_end;
    std::string m_filename;
    t_TAPSINGLE signal_scale;
    std::string m_lasterror;
};


//-----------------------------------------------------------------------------
// name: MicBuffer
// desc: buffer to store mic data
//-----------------------------------------------------------------------------
struct MicBuffer
{
	//create
	MicBuffer();
	// delete
	~MicBuffer();
	// allocate space
	t_TAPBOOL open( t_TAPUINT num_frames, t_TAPUINT channels );
	// free space
	t_TAPBOOL close();
	// start / stop recording
	t_TAPBOOL set_recording( t_TAPBOOL record );
	// is it recording
	t_TAPBOOL get_recording();
	// add recorded samples to internal buffer
	t_TAPBOOL add_samples( SAMPLE * buffer, t_TAPUINT num_frames ); 
	// read samples from internal buffer [fromIndex, toIndex) (assume big enough buffer)
	t_TAPBOOL get_samples( SAMPLE * buffer, t_TAPUINT fromIndex, t_TAPUINT toIndex );
	// recorded frames
	t_TAPUINT get_frames(); 
	// channels
	t_TAPUINT get_channels(); 
	// maximum recorded value
	SAMPLE maxval();
	// rewind: reset write index and frames recorded
	t_TAPBOOL rewind(); 
	
protected:
	SAMPLE * m_buffer;
    t_TAPUINT m_buffer_frames;
	t_TAPUINT m_max_buffer_frames;
	t_TAPUINT m_mic_channels; 
	t_TAPUINT m_frames_recorded;
	t_TAPBOOL m_is_recording;
	t_TAPUINT m_write_index; 
};


//-----------------------------------------------------------------------------
// name: AudioSrcMic
// desc: read samples from mic buffer
//-----------------------------------------------------------------------------
struct AudioSrcMic : public AudioSrcBuffer
{
	AudioSrcMic();
	~AudioSrcMic();
	// open
	t_TAPBOOL open( t_TAPUINT start = 0, t_TAPUINT len = 0, t_TAPBOOL scale = FALSE );
	// return samples from MicBuffer
    virtual t_TAPBOOL mtick( SAMPLE * buffer, t_TAPUINT num_samples );
    virtual t_TAPBOOL stick( SAMPLE * buffer, t_TAPUINT num_frames );
    // reset read index
	virtual t_TAPBOOL rewind();
	// seek (read index)
    virtual t_TAPBOOL seek( t_TAPUINT where, t_TAPUINT how );
	// return number of frames recorded
	virtual t_TAPUINT frames(); 
	// type of buffer
	virtual buffertype type() { return BUFF_MIC; }
	// get current position
	virtual t_TAPUINT current(); 
	// save recorded samples [fromIndex, toIndex)
	t_TAPBOOL save(std::string & filename, t_TAPUINT fromIndex, t_TAPUINT toIndex, 
		t_TAPBOOL scale = TRUE);
	
	const std::string & last_error() { return m_lasterror; }    
	t_TAPUINT start() { return m_start; }
    t_TAPUINT end() { return m_end; }
    t_TAPUINT now() { return m_read_index; }

    t_TAPSINGLE * slide;
    bool * slide_locally;

protected:
    t_TAPUINT m_read_index;  
    SNDFILE * sf;
    SF_INFO sf_info;
    t_TAPUINT m_start;
    t_TAPUINT m_end;
    t_TAPSINGLE signal_scale;
    std::string m_lasterror;
};


//-----------------------------------------------------------------------------
// name: AudioSrcFrame
// desc: ...
//-----------------------------------------------------------------------------
struct AudioSrcFrame : public AudioSrcBuffer
{
    AudioSrcFrame( Frame * f ) : AudioSrcBuffer() 
		{ frame = f; pos = 0; slide = NULL; slide_locally = NULL; }
    virtual t_TAPBOOL mtick( SAMPLE * buffer, t_TAPUINT num_samples ); 
    virtual t_TAPBOOL stick( SAMPLE * buffer, t_TAPUINT num_frames );
    virtual t_TAPBOOL rewind(); 
    virtual t_TAPBOOL seek( t_TAPUINT where, t_TAPUINT how );
    virtual t_TAPUINT frames() { return (t_TAPUINT)num_samples(); } // no, but it doesn't know # channels
	virtual buffertype type() { return BUFF_FRAME; }
	virtual t_TAPUINT current() { return pos; } // get current position
	int num_samples();
	
    t_TAPSINGLE * slide;
    bool * slide_locally;

protected:
    Frame * frame;
    t_TAPUINT pos;
};




//-----------------------------------------------------------------------------
// name: AudioSrcEliot
// desc: ...
//-----------------------------------------------------------------------------
struct AudioSrcEliot : public AudioSrc
{
    AudioSrcEliot( TreesynthIO * io ) : AudioSrc() { ts_io = io; }
    virtual t_TAPBOOL stick( SAMPLE * buffer, t_TAPUINT num_frames );
    void stop() { m_stop_asap = TRUE; }

protected:
    TreesynthIO * ts_io;
};



//-----------------------------------------------------------------------------
// name: AudioSrcOlar
// desc: like AudioSrcEliot but for OLAR
//-----------------------------------------------------------------------------
struct AudioSrcOlar : public AudioSrc
{
    AudioSrcOlar( OlaRandom * olar ) : AudioSrc() { m_olar = olar; }
    virtual t_TAPBOOL stick( SAMPLE * buffer, t_TAPUINT num_frames );
    void stop() { m_stop_asap = TRUE; }

protected:
    OlaRandom * m_olar;
};



//-----------------------------------------------------------------------------
// name: AudioFxReverb
// desc: ...
//-----------------------------------------------------------------------------
struct AudioFxReverb : public AudioSrc
{
public:
    AudioFxReverb() : AudioSrc()
    { m_mix = 0; m_state = NONE; m_left = m_right = NULL; }
    virtual ~AudioFxReverb()
    { SAFE_DELETE( m_left ); SAFE_DELETE( m_right ); m_state = NONE; }

    enum { NONE, PRC, JC, N };
    virtual void set( t_TAPUINT which, t_TAPFLOAT T60 );
    virtual void mix( t_TAPFLOAT value );
    t_TAPUINT getset() const { return m_state; }
    t_TAPFLOAT getmix() const { return m_mix; }

    virtual t_TAPBOOL stick( SAMPLE * buffer, t_TAPUINT num_frames );

protected:
    t_TAPFLOAT m_mix;
    t_TAPUINT m_state;
    FxReverb * m_left;
    FxReverb * m_right;
};




//-----------------------------------------------------------------------------
// name: AudioBus
// desc: ...
//-----------------------------------------------------------------------------
struct AudioBus : public AudioSrc
{
    AudioBus( t_TAPUINT num_frames ) : AudioSrc() { this->init( num_frames ); }
    virtual ~AudioBus();

    virtual t_TAPBOOL stick( SAMPLE * buffer, t_TAPUINT num_frames );
	virtual t_TAPBOOL multitick( SAMPLE * stereobuffer, SAMPLE * multibuffer, t_TAPUINT num_frames ); 

    virtual void play( AudioSrc * src, t_TAPBOOL solo = FALSE, t_TAPBOOL rew = TRUE );
    virtual void remove( AudioSrc * src );
    virtual void stop();

    virtual t_TAPUINT num_src() const { return m_src.size(); }

    AudioFxReverb * reverb() { return &m_reverb; }
    AudioFxReverb m_reverb;

    t_TAPBOOL m_ready;
    std::vector<AudioSrc *> m_src;
    std::vector<AudioSrc *> m_src2;
    SAMPLE * m_buffer;
    SAMPLE * m_buffer2;
	SAMPLE * m_bufferM; 
    t_TAPUINT m_num_frames;
    XMutex m_mutex;
	t_TAPUINT m_max_channels; 

private:
    t_TAPBOOL init( t_TAPUINT buffer_size );
};




//-----------------------------------------------------------------------------
// name: AudioBusParallel
// desc: ...
//-----------------------------------------------------------------------------
struct AudioBusParallel : public AudioBus
{
    AudioBusParallel( t_TAPUINT num_frames, t_TAPUINT sig_frames );
    virtual ~AudioBusParallel() { close(); }
    virtual t_TAPBOOL stick( SAMPLE * buffer, t_TAPUINT num_frames );
	virtual t_TAPBOOL multitick( SAMPLE * stereobuffer, SAMPLE * multibuffer, t_TAPUINT num_frames ); 
    void accumulate();

    bool open( int mode, const std::string & filename, const std::string & filename2 = "" );
    void close( int mode );
	void close( ); // close all
    SNDFILE * sf_stereo;
    SF_INFO sf_stereo_info;
	SNDFILE * sf_multi; 
	SF_INFO sf_multi_info; 
    t_TAPUINT m_sig_frames;

protected:
    XThread m_thread;
    XMutex m_sf_stereo_mutex;
	XMutex m_sf_multi_mutex;
};



//-----------------------------------------------------------------------------
// name: CacheInfo
// desc: info on open sound files
//-----------------------------------------------------------------------------
struct CacheInfo
{
	SNDFILE * sf; // sound file
	int count; // reference count
	bool close; // whether it should be closed during closesf?
	SF_INFO lastinfo; // last sfinfo
};


//-----------------------------------------------------------------------------
// name: CacheManager
// desc: maintains cache of open sound files
//-----------------------------------------------------------------------------
struct CacheManager
{
	// open
	SNDFILE * opensf( std::string path, int mode, SF_INFO * info, 
		int id = 0, t_TAPBOOL usecache = TRUE ); 

	// add an already open file
	bool addsf( std::string path, int mode, const SF_INFO &info, 
		SNDFILE * sf, int id = 0 );

	// close
	void closesf( SNDFILE * sf );

	// check if already open, return cacheinfo vector size (= number of pointers)
	int isopen( std::string path );

protected:
	// some map of address to soundfile	
	std::map< std::string, std::vector<CacheInfo> > names;
};


//-----------------------------------------------------------------------------
// name: AudioCentral
// desc: ...
//-----------------------------------------------------------------------------
struct AudioCentral
{
    virtual ~AudioCentral() { if( m_audio ) this->free(); }
    static AudioCentral * instance();

    t_TAPBOOL init( t_TAPUINT srate, t_TAPUINT num_frames, t_TAPUINT sig_frames, t_TAPUINT bus = 8, 
					t_TAPUINT channels_out = 2, t_TAPUINT channels_in = 1, t_TAPSINGLE recorded_secs = 60 );
    t_TAPBOOL free();

    virtual t_TAPBOOL stop_all();
    AudioBus * bus( t_TAPUINT index );
    virtual t_TAPUINT num_bus() const { return m_bus->m_src.size(); }

    virtual void set_gain( t_TAPSINGLE gain ) { m_bus->set_gain( gain ); }
    virtual t_TAPSINGLE get_gain() const { return m_bus->get_gain(); }

    t_TAPBOOL record_start( int mode, const std::string & filename, const std::string & filename2 = "" )
    { return m_bus->open( mode, filename, filename2 ); }
    t_TAPBOOL record_stop( int mode )
    { m_bus->close( mode ); return TRUE; }

	// file write modes
	enum { STEREO=0, MULTICHANNEL, BOTH };

public:
    SAMPLE * m_buffer;
    SAMPLE * m_buffer2;
    t_TAPUINT m_num_frames;
    t_TAPUINT m_sig_frames;
    t_TAPUINT m_channels_out;
	t_TAPUINT m_channels_in; 
    t_TAPUINT m_srate;
    RtAudio * m_audio;
    XMutex m_mutex;
    AudioBusParallel * m_bus;
	CacheManager * m_cachemanager;
	MicBuffer * m_mic_buffer; 

protected:
    static AudioCentral * our_instance;
    AudioCentral() { m_buffer = NULL; m_buffer2 = NULL; m_num_frames = 0;
                     m_srate = 0; m_audio = NULL; m_bus = NULL; m_channels_out = 2;
					 m_channels_in = 0; m_cachemanager = NULL; m_mic_buffer = NULL; }
};




#endif
