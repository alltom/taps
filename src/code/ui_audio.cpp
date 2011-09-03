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
#include <stdio.h>
#include <memory.h>
#include "ui_audio.h"
#include "taps_treesynth.h"
#include "util_thread.h"
using namespace std;


// static variables
AudioCentral * AudioCentral::our_instance = NULL;


//-----------------------------------------------------------------------------
// name: cb()
// desc: callback
//-----------------------------------------------------------------------------
static int cb( char * buffer, int buffer_size, void * user_data )
{
    AudioCentral * ac = (AudioCentral *)user_data;
    SAMPLE * buf = ac->m_buffer;
    SAMPLE * buf2 = ac->m_buffer2;
    t_TAPUINT num_frames = ac->m_num_frames;
    t_TAPUINT num_samples = num_frames * ac->m_channels_out;
    t_TAPBOOL yes = FALSE;
    assert( num_frames == buffer_size );

	// read from buffer
	if( ac->m_mic_buffer && ac->m_mic_buffer->get_recording() ) {
		ac->m_mic_buffer->add_samples( (SAMPLE *)buffer, num_frames );
	}
	
    // zero out the buffer 
    memset( buffer, 0, sizeof(SAMPLE) * num_samples );

    // lock
    ac->m_mutex.acquire();
    // get samples
    if( ac->m_channels_out == 2 )
		yes = ac->m_bus->stick( (SAMPLE *)buffer, num_frames );
    else
		yes = ac->m_bus->multitick( buf2, (SAMPLE *)buffer, num_frames );
	// release
    ac->m_mutex.release();
    
    return 0;
}

// blocking replacement for callback function
#ifdef __PLATFORM_WIN32__
unsigned __stdcall ac_go( void * data )
#else
void * ac_go( void * data )
#endif
{
	BB_log( BB_LOG_INFO, "ac_go()" );
	AudioCentral * ac = (AudioCentral *)data; 
	
	if( ac == NULL )
		return FALSE;
	t_TAPBOOL yes = TRUE;
	
	while( yes ) 
	{
		// get buffer
		char * buffer = ac->m_audio->getStreamBuffer();
		BB_log( BB_LOG_FINE, "got buffer" );
		if( buffer == NULL ) {
			BB_log( BB_LOG_WARNING, "ac_go() - null poiner" );
			return FALSE;
		}
		
		// read from buffer
		if( ac->m_mic_buffer && ac->m_mic_buffer->get_recording() ) {
			ac->m_mic_buffer->add_samples( (SAMPLE *)buffer, ac->m_num_frames );
			BB_log(BB_LOG_FINE, "got mic samples");
		}
	
    	// set to zero
		t_TAPUINT num_samples = ac->m_num_frames * ac->m_channels_out;
		memset( buffer, 0, sizeof(SAMPLE) * num_samples );
		yes = FALSE;
		BB_log( BB_LOG_FINE, "set to zero" );
		
		// lock
		ac->m_mutex.acquire();
		BB_log( BB_LOG_FINE, "acquired mutex" );
		
		// get samples
		if( ac->m_channels_out == 2 )
			yes = ac->m_bus->stick( (SAMPLE *)buffer, num_samples );
		else
			yes = ac->m_bus->multitick( ac->m_buffer2, (SAMPLE *)buffer, num_samples ); 
		BB_log( BB_LOG_FINE, "wrote samples" );
			
		// release
		ac->m_mutex.release();
		BB_log( BB_LOG_FINE, "released mutex" );
		
		// tick rtaudio
		ac->m_audio->tickStream();
		BB_log( BB_LOG_FINE, "ticked" );
	}
	
	return 0;
}


//-----------------------------------------------------------------------------
// name: mono2stereo()
// desc: on same buffer
//-----------------------------------------------------------------------------
void AudioSrc::mono2stereo( SAMPLE * self, t_TAPUINT num_frames )
{
    // start from end of mono
    SAMPLE * src = self + num_frames - 1;
    SAMPLE * dest = self + (num_frames*2) - 2;

    // copy
    while( src >= self )
    {
        *dest = *(dest+1) = *src;
        src--; dest -= 2;
    }
}


//-----------------------------------------------------------------------------
// name: stereo2mono()
// desc: on same buffer
//-----------------------------------------------------------------------------
void AudioSrc::stereo2mono( SAMPLE * self, t_TAPUINT num_frames )
{
    // start from end of mono
    SAMPLE * src = self;
    SAMPLE * end = self + (num_frames*2);
    SAMPLE * dest = self;

    // copy
    while( src < end )
    {
        *dest = (*src + *(src+1))/2.0f;
        src += 2; dest++;
    }

    // zero
    while( dest < end )
        *dest++ = 0.0f;
}

//-----------------------------------------------------------------------------
// name: mono2multi()
// desc: on same buffer
//-----------------------------------------------------------------------------
void AudioSrc::mono2multi( SAMPLE * myself, t_TAPUINT frames, t_TAPUINT channels )
{
    // start from end of mono
    SAMPLE * src = myself + frames - 1;
    SAMPLE * dest = myself + (frames*channels) - channels;

    // copy
    while( src >= myself )
    {
		for( int i = 0; i < channels; i++ )
			*(dest + i) = *src;
        src--; dest -= channels;
    }
}


//-----------------------------------------------------------------------------
// name: multi2mono()
// desc: on same buffer
//-----------------------------------------------------------------------------
void AudioSrc::multi2mono( SAMPLE * myself, t_TAPUINT frames, t_TAPUINT channels )
{
    // start from end of mono
    SAMPLE * src = myself;
    SAMPLE * end = myself + (frames*channels);
    SAMPLE * dest = myself;

    // copy
    while( src < end )
    {
		*dest = *src;
		for( int i = 1; i < channels; i++ )
			*dest = *dest + *(src + i);
		*dest = *dest / (float)channels;
        src += channels; dest++;
    }

    // zero
    while( dest < end )
        *dest++ = 0.0f;
}


//-----------------------------------------------------------------------------
// name: mono2stereo()
// desc: ...
//-----------------------------------------------------------------------------
void AudioSrc::mono2stereo( const SAMPLE * mono, t_TAPUINT num_frames, SAMPLE * stereo )
{
    SAMPLE * p = stereo, * end = stereo + (num_frames*2);
    // loop
    while( p < end )
    {
        // copy left and right from mono
        *p = *(p+1) = *mono;
        // advance to next frame
        mono++; p += 2;
    }
}


//-----------------------------------------------------------------------------
// name: stereo2mono()
// desc: ...
//-----------------------------------------------------------------------------
void AudioSrc::stereo2mono( const SAMPLE * stereo, t_TAPUINT num_frames, SAMPLE * mono )
{
    const SAMPLE * s = stereo, * end = stereo + (num_frames*2);
    SAMPLE * p = mono;
    // loop
    while( s < end )
    {
        // average and copy
        *p = (*s + *(s+1)) / 2.0f;
        // advance to next frame
        p++; s += 2;
    }
}


//-----------------------------------------------------------------------------
// name: mono2multi()
// desc: ...
//-----------------------------------------------------------------------------
void AudioSrc::mono2multi( const SAMPLE * mono, t_TAPUINT frames, t_TAPUINT channels, SAMPLE * multi )
{
    SAMPLE * p = multi, * end = multi + (frames*channels);
    // loop
    while( p < end )
    {
        // copy from mono
		for( int i = 0; i < channels; i++ ) 
			*(p + i) = *mono;
        // advance to next frame
        mono++; p += channels;
    }
}




//-----------------------------------------------------------------------------
// name: multi2mono()
// desc: ...
//-----------------------------------------------------------------------------
void AudioSrc::multi2mono( const SAMPLE * multi, t_TAPUINT frames, t_TAPUINT channels, SAMPLE * mono )
{
    const SAMPLE * s = multi, * end = multi + (frames*channels);
    SAMPLE * p = mono;
    // loop
    while( s < end )
    {
        // average and copy
		SAMPLE m = 0.0f;
		for( int i = 0; i < channels; i++ )
			m += *(s + i); 
		*p = m / (float)channels;
        // advance to next frame
        p++; s += channels;
    }
}


//-----------------------------------------------------------------------------
// name: mtick
// desc: mono tick
//-----------------------------------------------------------------------------
t_TAPBOOL AudioSrc::mtick( SAMPLE * buffer, t_TAPUINT num_frames )
{
    SAMPLE * sbuffer = new SAMPLE[num_frames * 2];
    t_TAPBOOL ret = this->stick( sbuffer, num_frames );
    stereo2mono( sbuffer, num_frames, buffer );
    SAFE_DELETE_ARRAY( sbuffer ); 
	return ret;
}




//-----------------------------------------------------------------------------
// name: pan_buf()
// desc: you know, the mythical creature
//-----------------------------------------------------------------------------
void AudioSrc::pan_buf( SAMPLE * stereo, t_TAPUINT num_frames, t_TAPSINGLE where )
{
    t_TAPSINGLE left_gain = where <= .5f ? 1.0f : (1.0f - where) * 2;
    t_TAPSINGLE right_gain = where >= .5f ? 1.0f : where * 2;
    SAMPLE * s = stereo, * end = stereo + (num_frames*2);
    // left
    if( left_gain != 1.0f )
    {
        while( s < end )
        {
            *s *= left_gain;
            s += 2;
        }
    }

    // right
    if( right_gain != 1.0f )
    {
        // reset pointer
        s = stereo + 1;
        while( s < end )
        {
            *s *= right_gain;
            s += 2;
        }
    }
}



//-----------------------------------------------------------------------------
// name: gain_buf()
// desc: ...
//-----------------------------------------------------------------------------
void AudioSrc::gain_buf( SAMPLE * buffer, t_TAPUINT num_samples, SAMPLE gain )
{
    if( gain != 1.0f )
        for( t_TAPUINT i = 0; i < num_samples; i++ )
            buffer[i] *= gain;
}


//-----------------------------------------------------------------------------
// name: forward()
// desc: forward to the state num_frames after current state 
//		 (forward by num_frames for simple audio sources)
//-----------------------------------------------------------------------------
t_TAPBOOL AudioSrc::forward( t_TAPUINT num_frames ) {
	// TODO: re-entry of this function crashes!
	// don't proceed if it is playing 
	// (also didn't proceed if m_stop_asap, but that prevented a timeline from replaying once stopped)
	if( playing() )
		return FALSE;
	// reset flags to show it's being used
	reset();
	// forward
	int bufsize = AudioCentral::instance()->m_num_frames;
	SAMPLE * discardbuffer = new SAMPLE[2 * bufsize];
	int remaining = num_frames;
	while( remaining > 0 ) {
		int tosynth = (remaining < bufsize ? remaining : bufsize); 
		if( this->stick( discardbuffer, tosynth ) )
			remaining -= tosynth; 
		else {
			BB_log(BB_LOG_INFO, "AudioSrc::forward() : reached end of audio src"); 
			break;
		}
	}
	// set m_done flag because it's no longer currently playing (hijacking control from AudioBus)
	m_done = TRUE; 
	// clean up (not good to do this each time, but AudioSrc doesn't own big buffers now)
	SAFE_DELETE_ARRAY(discardbuffer);
	// return 
	return TRUE;
}



// AUDIO CENTRAL
AudioCentral * AudioCentral::instance()
{
    if( !our_instance )
        our_instance = new AudioCentral;

    return our_instance;
}


XThread thread;

//-----------------------------------------------------------------------------
// name: init()
// desc: ...
//-----------------------------------------------------------------------------
t_TAPBOOL AudioCentral::init( t_TAPUINT srate, t_TAPUINT num_frames,
                             t_TAPUINT sig_frames, t_TAPUINT bus, t_TAPUINT channels_out, 
							 t_TAPUINT channels_in, t_TAPSINGLE record_secs )
{
    // if already started...
    if( m_audio != NULL )
    {
        BB_log( BB_LOG_SYSTEM, "[ui_audio]: already started..." );
        return FALSE;
    }

    // frame
    if( num_frames < sig_frames )
    {
        BB_log( BB_LOG_SYSTEM, "[ui_audio]: num_frames < sig_frames - bad!" );
        return FALSE;
    }

	// chief minister
	if( m_cachemanager != NULL )
	{
		BB_log( BB_LOG_SYSTEM, "[ui_audio]: cache manager already exists!" ); 
		return FALSE;
	}
	
	// mic buffer
	if( m_mic_buffer != NULL )
	{
		BB_log( BB_LOG_SYSTEM, "[ui_audio]: mic buffer already exists!" );
		return FALSE;
	}

    // log
    BB_log( BB_LOG_SYSTEM, "initializing audio engine..." );
    // push log
    BB_pushlog();

    // set srate
    m_srate = srate;
    // set num_frames
    m_num_frames = num_frames;
    // set sig_frames
    m_sig_frames = sig_frames;
    // number of buffers
    t_TAPINT num_buffers = 4;
	// number of channels for real-time audio output
	m_channels_out = (channels_out >= 0 && channels_out <= bus) ? channels_out : 2;
	// number of channels for real-time audio input
	m_channels_in = channels_in;  
	// record_secs used separately for mic input

    // log
    BB_log( BB_LOG_SYSTEM, "sample rate: %d", srate );
    BB_log( BB_LOG_SYSTEM, "buffer size: %d", num_frames );
    BB_log( BB_LOG_SYSTEM, "signal size: %d", sig_frames );
    BB_log( BB_LOG_SYSTEM, "num buffers: %d", num_buffers );
    BB_log( BB_LOG_SYSTEM, "channels: %d (output), %d (input)", m_channels_out, m_channels_in );
	BB_log( BB_LOG_SYSTEM, "max length recorded: %f seconds", record_secs );

    // open the device
    try
    {
        int size = m_num_frames;
        // open the audio device for capture and playback
        m_audio = new RtAudio( 0, m_channels_out, 0, m_channels_in, RTAUDIO_FLOAT32,
            m_srate, &size, num_buffers );
        m_num_frames = size;
    }
    catch( RtError & e )
    {
        // exception
        BB_log( BB_LOG_SYSTEM, "[ui_audio]: %s", e.getMessage().c_str() );
        BB_log( BB_LOG_SYSTEM, "[ui_audio]: cannot open audio device for record/playback" );
        // try again
		try 
		{
			int size = m_num_frames; 
			// open the audio device for playback
			m_audio = new RtAudio( 0, m_channels_out, 0, 0, RTAUDIO_FLOAT32, 
				m_srate, &size, num_buffers );
			m_num_frames = size;
		}
		catch( RtError & e2 )
		{
			BB_log( BB_LOG_SYSTEM, "[ui_audio]: %s", e2.getMessage().c_str() );
			BB_log( BB_LOG_SYSTEM, "[ui_audio]: cannot open audio device for playback" );
			// zero
			m_audio = NULL;
			return FALSE;
		}
		BB_log( BB_LOG_SYSTEM, "allowing audio playback" );
    }
	BB_log( BB_LOG_SYSTEM, "allowing audio record/playback" );

    // allocate buffers
    m_buffer = new SAMPLE[m_num_frames * m_channels_out];
    m_buffer2 = new SAMPLE[m_num_frames * m_channels_out];

    // log
    BB_log( BB_LOG_SYSTEM, "allocating parallel bus..." );
    // draw parallel
    m_bus = new AudioBusParallel( m_num_frames, m_sig_frames );
    m_bus->m_delete = FALSE;
    
    // log
    BB_log( BB_LOG_SYSTEM, "allocating %d audio buses...", bus );
    // push log
    BB_pushlog();
//	fprintf( stdout, "buses:\n" );
	for( t_TAPUINT i = 0; i < bus; i++ )
    {
        // log
        BB_log( BB_LOG_INFO, "initializing audio bus %d...", i );
        AudioBus * audbus = new AudioBus( m_num_frames );
        audbus->m_delete = FALSE;
        m_bus->m_src.push_back( audbus );
//      fprintf( stdout, "%i - %x\n", i, m_bus->m_src[i] );
    }
    // pop log
    BB_poplog();

	// cache manager
	m_cachemanager = new CacheManager; 
	BB_log( BB_LOG_SYSTEM, "initializing cache manager..." );
	
	// mic buffer
	m_mic_buffer = new MicBuffer;
	m_mic_buffer->open( record_secs * BirdBrain::srate(), m_channels_in );
	BB_log( BB_LOG_SYSTEM, "initializing mic... %srecording",
		m_mic_buffer->get_recording() ? "" : "NOT " );

    // set the audio callback
	if( BirdBrain::rtaudio_blocking() == FALSE )
		m_audio->setStreamCallback( cb, this );
	
	// log
    BB_log( BB_LOG_SYSTEM, "starting real-time audio..." );
    // start the audio
    m_audio->startStream( );

	// blocking
	if( BirdBrain::rtaudio_blocking() == TRUE )
		thread.start( ac_go, this );
    
    // pop log
    BB_poplog();
	
    return TRUE;
}


//-----------------------------------------------------------------------------
// name: free()
// desc: ...
//-----------------------------------------------------------------------------
t_TAPBOOL AudioCentral::free()
{
    // if not started
    if( m_audio == NULL )
        return FALSE;

    // clear the queue
    this->stop_all();
    // clear bus
    SAFE_DELETE( m_bus );

    // stop the audio
    m_audio->stopStream();
    //m_audio->abortStream();
	
    // free the audio device
    SAFE_DELETE( m_audio );

    // free buffers
	SAFE_DELETE_ARRAY( m_buffer );
	SAFE_DELETE_ARRAY( m_buffer2 );

	// delete cache manager
	SAFE_DELETE( m_cachemanager );

    return TRUE;
}




//-----------------------------------------------------------------------------
// name: bus()
// desc: ...
//-----------------------------------------------------------------------------
AudioBus * AudioCentral::bus( t_TAPUINT index )
{
    if( index < 0 || index >= m_bus->m_src.size() )
        return NULL;

    return (AudioBus *)m_bus->m_src[index];
}




//-----------------------------------------------------------------------------
// name: stop()
// desc: ...
//-----------------------------------------------------------------------------
t_TAPBOOL AudioCentral::stop_all()
{
    // audio
    if( m_audio == NULL ) return FALSE;

    // synchronize
    m_mutex.acquire();
    // stop
    for( long i = 0; i < m_bus->m_src.size(); i++ )
        ((AudioBus *)m_bus->m_src[i])->stop();
    // release
    m_mutex.release();

    return TRUE;
}




//-----------------------------------------------------------------------------
// name: stick()
// desc: ...
//-----------------------------------------------------------------------------
t_TAPBOOL AudioSrcTest::stick( SAMPLE * buffer, t_TAPUINT num_frames )
{
//    assert( m_done == FALSE );

    // are we done?
    if( m_stop_asap )
        return FALSE;

    // 1 second
    if( m_t > BirdBrain::srate() )
    {
        m_done = TRUE;
        return FALSE;
    }

    // samples
    for( t_TAPUINT i = 0; i < num_frames; i++ )
    {
        buffer[i*2] = .25 * sin( m_t * m_freq * 2.0 * PIE / BirdBrain::srate() );
        buffer[i*2+1] = buffer[i*2];
        m_t += 1.0;
    }

    // gain
    gain_buf( buffer, num_frames * 2, m_gain );
    // pan
    pan_buf( buffer, num_frames, m_pan );

	// time
	m_time_elapsed += num_frames; 
	
    return TRUE;
}




//-----------------------------------------------------------------------------
// name: AudioSrcFile()
// desc: ...
//-----------------------------------------------------------------------------
AudioSrcFile::AudioSrcFile()
    : AudioSrcBuffer()
{
    m_buffer = NULL;
    m_buffer_size = 0;
    sf = NULL;
    slide = NULL;
    slide_locally = NULL;
    signal_scale = 1.0f;
#ifdef SECRET_RABBIT_CODE
	sr = NULL;
	sr_gain = 1.0f;
#endif
}




//-----------------------------------------------------------------------------
// name: ~AudioSrcFile()
// desc: ...
//-----------------------------------------------------------------------------
AudioSrcFile::~AudioSrcFile()
{
    if( sf )
    {
        this->close();
    }

    if( on ) *on = false;
}




///-----------------------------------------------------------------------------
// name: rewind()
// desc: ...
//-----------------------------------------------------------------------------
t_TAPBOOL AudioSrcFile::rewind()
{
	AudioSrc::rewind();
	sf_seek( sf, 0, SEEK_SET );
#ifdef SECRET_RABBIT_CODE
	if( sr )
		src_reset( sr );
#endif
	return TRUE;
}




///-----------------------------------------------------------------------------
// name: open()
// desc: ...
//-----------------------------------------------------------------------------
t_TAPBOOL AudioSrcFile::open( const char * filename, t_TAPUINT start, t_TAPUINT len, 
							 t_TAPBOOL scale, int id, t_TAPBOOL usecache )
{
    // sanity
    if( sf )
    {
       BB_log( BB_LOG_WARNING, "[audiosrcfile]: file '%s' already open...", m_filename.c_str() );
       return FALSE;
    }

    // open the file
//    sf = sf_open( filename, SFM_READ, &sf_info );
	if( usecache && AudioCentral::instance()->m_cachemanager->isopen( filename ) > id )
	{
		// if open, just add to the count
		sf = AudioCentral::instance()->m_cachemanager->opensf( filename, SFM_READ, &sf_info, id );
	}
	else
	{
		sf = sf_open( filename, SFM_READ, &sf_info );
		if( !sf )
		{
			BB_log( BB_LOG_WARNING, "[audiosrcfile]: cannot open file '%s'...", filename );
			m_lasterror = "cannot open file!";
			return FALSE;
		}

		// support mono and stereo only
		if( sf_info.channels > 2 )
		{
		    BB_log( BB_LOG_SEVERE, "[audiosrcfile]: not mono/stereo file!" );
		    m_lasterror = "not a mono/stereo file";
			this->close();
			return FALSE;
		}

		if( sf_info.samplerate != BirdBrain::srate() )
		{
#ifdef SECRET_RABBIT_CODE
			sr_gain = 1.0f;
			int converted;
			do {
				converted = samplerateconvert( filename );
			}
			while( converted < 0 );
#else
			BB_log( BB_LOG_SEVERE, "[audiosrcfile]: not %i hz file!", BirdBrain::srate() );
			m_lasterror = "TAPESTREA expects matching sample rate (";
			char buffy[128];
			sprintf( buffy, "%d", BirdBrain::srate() );
			m_lasterror += buffy;
			m_lasterror += "), for now...";
			this->close();
			return FALSE;
#endif
		}
		
		// add to cachemanager
		if( usecache )
			AudioCentral::instance()->m_cachemanager->addsf( filename, SFM_READ, sf_info, sf, id ); // mode could be wrong but needs work anyway
	}

    // check start and len
    if( start >= sf_info.frames )
    {
        BB_log( BB_LOG_SEVERE, "[audiosrcfile]: '%s' has only %li frames...",
            filename, sf_info.frames );
        BB_log( BB_LOG_SEVERE, "[audiosrcfile]: ...cannot seek to '%li'...",
            start );
        m_lasterror = "cannot seek to requested start position";
        this->close();
        return FALSE;
    }

    if( len && (start + len) > sf_info.frames ) // used to be >= but... see m_end below
    {
        BB_log( BB_LOG_SEVERE, "[audiosrcfile]: '%s' has only %li frames...",
            filename, sf_info.frames );
        BB_log( BB_LOG_SEVERE, "[audiosrcfile]: ...cannot seek upto '%li'...",
            start + len );
        m_lasterror = "cannot seek to requested start position";
        this->close();
        return FALSE;
    }

    // copy
    m_filename = filename;
    m_start = start;
    m_end = len ? start + len : sf_info.frames;

    if( scale )
    {
        // get scaling
        double   max_val ;
        sf_command (sf, SFC_CALC_NORM_SIGNAL_MAX, &max_val, sizeof (max_val)) ;
        signal_scale = (float)( .95 / max_val );
    }

    // seek
    if( start >= 0 ) sf_seek( sf, start, SEEK_SET );
    
    return TRUE;
}



#define SR_BUFFER_LEN 4096
#ifdef SECRET_RABBIT_CODE
//-----------------------------------------------------------------------------
// name: samplerateconvert()
// desc: duh
//-----------------------------------------------------------------------------
int AudioSrcFile::samplerateconvert( const char * filename )
{
	SNDFILE * outfile; 
	SF_INFO outfileinfo = sf_info;
	outfileinfo.samplerate = BirdBrain::srate();
	outfileinfo.sections = 0;
	outfileinfo.frames = 0;
	outfileinfo.format = SF_FORMAT_WAV | SF_FORMAT_FLOAT; 
	int error;

	BB_log( BB_LOG_INFO, "Converting sample rate of '%s' from %i Hz to %i Hz", 
		filename, (int)sf_info.samplerate, (int)outfileinfo.samplerate );
	// open converter
	if( sr )
	{
		BB_log( BB_LOG_WARNING, "[audiosrcfile]: surprise, sr already existed, quitting" );
		return 0;
	}
	sr = src_new( SRC_SINC_FASTEST, outfileinfo.channels, &error );
	if( !sr )
	{
		BB_log( BB_LOG_WARNING, "[audiosrcfile]: %s", src_strerror( error ) );
		return 0;
	}

	// open outfile
	/*char temp[8];
	std::string outfilename = BirdBrain::getname( filename ) + "_temp_" + itoa((int)BirdBrain::srate(),temp,10) + ".wav"; 
	if ( (outfile = sf_open( outfilename.c_str() , SFM_WRITE, &outfileinfo )) == NULL)
	{	
		BB_log( BB_LOG_SEVERE, "[audiosrcfile]: Not able to open output file '%s'; not converting", outfilename.c_str() );
		return 0;
	}*/
	FILE * temp;
	if( (temp = tmpfile()) == NULL )
	{
		BB_log( BB_LOG_SEVERE, "[audiosrcfile]: Could not open temp file; not converting" );
		src_delete( sr ); sr = NULL;
		return 0;
	}
	int temporary;
#ifdef __PLATFORM_WIN32__
	temporary = _fileno( temp );
#else
	temporary = fileno( temp );
#endif
	outfile = sf_open_fd( temporary, SFM_RDWR, &outfileinfo, TRUE );
	if( outfile == NULL )
	{
		BB_log( BB_LOG_SEVERE, "[audiosrcfile]: Error opening sound file from descriptor %i (%s); not converting", 
			temporary, sf_error_number( sf_error( sf ) ) );
		fclose( temp );
		src_delete( sr ); sr = NULL;
		return 0;
	}
	else
	{
#ifdef __PLATFORM_WIN32__
		BB_log( BB_LOG_INFO, "[audiosrcfile]: Opened temporary file %i", _fileno( temp ) );
#else
		BB_log( BB_LOG_INFO, "[audiosrcfile]: Opened temporary file %i", fileno( temp ) );
#endif
	}

	// convert (taken from libsamplerate example sndfile-resample.c)
	SAMPLE input [SR_BUFFER_LEN] ;
	SAMPLE output [SR_BUFFER_LEN] ;
	SRC_DATA src_data ;
	sf_count_t output_count = 0 ;

	sf_seek (sf, 0, SEEK_SET);
	sf_seek (outfile, 0, SEEK_SET);
	src_data.end_of_input = 0; // Set this later

	// Start with zero to force load in while loop. 
	src_data.input_frames = 0;
	src_data.data_in = input;
	src_data.src_ratio = (double)outfileinfo.samplerate / sf_info.samplerate; 
	src_data.data_out = output ;
	src_data.output_frames = SR_BUFFER_LEN / sf_info.channels ;

	while( true )
	{
		// If the input buffer is empty, refill it. 
		if (src_data.input_frames == 0)
		{	src_data.input_frames = sf_readf_float (sf, input, SR_BUFFER_LEN / sf_info.channels);
			src_data.data_in = input;

			// The last read will not be a full buffer, so end_of_input.
			if (src_data.input_frames < SR_BUFFER_LEN / sf_info.channels)
				src_data.end_of_input = SF_TRUE;
		}

		if ( (error = src_process( sr, &src_data )) )
		{	
			BB_log( BB_LOG_SEVERE, "[audiosrcfile]: %s - not converting", src_strerror( error ) );
			sf_close( outfile );
			return 0;
		}

		// Terminate if done.
		if (src_data.end_of_input && src_data.output_frames_gen == 0)
			break;
		
		// apply gain if needed
		float max = 1;
		for( int k = 0; k < src_data.output_frames_gen * sf_info.channels; k++ )
		{
			output[k] *= sr_gain;
			if( ::fabs( output[k] ) > max )
				max = ::fabs( output[k] );
		}
		if( max > 1 )
		{
			sr_gain *= 1 / max;
			BB_log( BB_LOG_WARNING, "[audiosrcfile]: output clipped (max = %f, gain = %f), redoing conversion", max, sr_gain );
			sf_close( outfile );
			src_delete( sr ); sr = NULL;
			return -1;
		}

		// Write output
		sf_writef_float( outfile, output, src_data.output_frames_gen );
		output_count += src_data.output_frames_gen;

		// Update read info
		src_data.data_in += src_data.input_frames_used * sf_info.channels;
		src_data.input_frames -= src_data.input_frames_used;
	}

//	sr = src_delete (sr);

	// close things
	//sf_close( outfile );
	sf_close( sf );
	src_delete( sr ); sr = NULL;

	sf_command( outfile, SFC_UPDATE_HEADER_NOW, NULL, 0 );
	sf_info = outfileinfo;
	sf_info.frames = sf_seek( outfile, 0, SEEK_CUR );
	BB_log( BB_LOG_CRAZY, "sf frames : %i", (int)sf_info.frames );
	BB_log( BB_LOG_CRAZY, "outfile seekable %i sf_info seekable %i", (int)outfileinfo.seekable, (int)sf_info.seekable );
	sf = outfile;

	// change sf to be the file we just wrote
	//sf_info = outfileinfo;
	/*if ( !(sf = sf_open( outfilename.c_str(), SFM_READ, &sf_info )) )
	{	
		BB_log( BB_LOG_SEVERE, "[audiosrcfile]: Not able to open temporary input file '%s'", outfilename.c_str() );
		return 0;
	} */
	/*if( (sf = sf_open_fd( _fileno( temp ), SFM_READ, &sf_info, TRUE )) == NULL )
	{
		BB_log( BB_LOG_SEVERE, "[audiosrcfile]: Not able to open temporary input file %i: %s", 
			_fileno( temp ), sf_error_number( sf_error( sf ) ) );
		fclose( temp );
		return 0;
	}*/
	
	return (int)output_count;
}
#endif


//-----------------------------------------------------------------------------
// name: close()
// desc: ...
//-----------------------------------------------------------------------------
t_TAPBOOL AudioSrcFile::close()
{
    if( !sf ) return FALSE;

    // deallocate buffer
    if( m_buffer ) delete [] m_buffer;
    // close
//    sf_close( sf );
	AudioCentral::instance()->m_cachemanager->closesf( sf );
	// delete s.r.c

#ifdef SECRET_RABBIT_CODE
	if( sr ) 
	{
		src_delete( sr );
		sr = NULL;
	}
#endif

    m_buffer = NULL;
    sf = NULL;
    m_start = 0;
    m_end = 0;
    m_filename = "";
    m_buffer_size = 0;

    return TRUE;
}




//-----------------------------------------------------------------------------
// name: stick()
// desc: ...
//-----------------------------------------------------------------------------
t_TAPBOOL AudioSrcFile::stick( SAMPLE * buffer, t_TAPUINT num_frames )
{
//    assert( m_done == FALSE );

    // are we done?
    if( m_stop_asap )
        return FALSE;

    // do mono first
    t_TAPBOOL it = mtick( buffer, num_frames );
    // convert to "stereo"
    mono2stereo( buffer, num_frames, m_buffer );
    // pan
    pan_buf( m_buffer, num_frames, m_pan );
    // copy back
    memcpy( buffer, m_buffer, sizeof(SAMPLE) * num_frames * 2 );

    return it;
}


//-----------------------------------------------------------------------------
// name: mtick()
// desc: mono tick
//-----------------------------------------------------------------------------
t_TAPBOOL AudioSrcFile::mtick( SAMPLE * buffer, t_TAPUINT num_frames )
{
//    assert( m_done == FALSE );

    // are we done?
    if( m_stop_asap )
        return FALSE;

	// zero
	memset( buffer, 0, num_frames * sizeof(SAMPLE) );

    // sanity
    if( !sf )
    {
        return FALSE;
    }

    sf_count_t i;
    if( m_buffer_size < ( num_frames * 2 ) )
    {
        // delete
        if( m_buffer ) delete [] m_buffer;
        // allocate
        m_buffer = new SAMPLE[num_frames * 2];
        if( !m_buffer )
        {
            fprintf( stderr, "[audiosrcfile]: cannot allocate buffer of size '%i'...\n",
                num_frames * 2 );
            return FALSE;
        }
        m_buffer_size = num_frames * 2;
    }

    // read the next chunk
    sf_count_t diff = m_end - sf_seek( sf, 0, SEEK_CUR );
    // check to make sure there is enough data from this point
    if( diff <= 0 )
    {
        if( slide_locally ) *slide_locally = false;
        return FALSE;
    }

	// hack
	sf_count_t pos = sf_seek( sf, 0, SEEK_CUR );
    // read
    sf_count_t thismuch = num_frames <= diff ? num_frames : diff;
    sf_count_t read = sf_readf_float( sf, m_buffer, thismuch );
    //assert( thismuch == read );
	if( thismuch != read )
	{
		BB_log( BB_LOG_FINE, "(audiosrcfile): couldn't read enough; %s", sf_error_number(sf_error(sf)) );
	}
	BB_log( BB_LOG_FINEST, "To read: %i; Read: %i", (int)thismuch, (int)read );
	// hack continued, possibly the correct action
	if( sf_seek( sf, 0, SEEK_CUR ) == pos )
	{
		BB_log( BB_LOG_FINEST, "sf_read no update cur pointer; me do manually :|" );
		sf_seek( sf, read, SEEK_CUR );
	}

    // update
    if( slide != NULL )
    {
        *slide = (sf_seek( sf, 0, SEEK_CUR ) - m_start) / (t_TAPSINGLE)(m_end - m_start);
		BB_log( BB_LOG_FINE, "slide = (%i - %i) / (%i - %i) = %f",
			(int)(sf_seek( sf, 0, SEEK_CUR )), (int)m_start, (int)m_end, (int)m_start, *slide );
        *slide_locally = true;
    }

    // if more than one channel
    if( sf_info.channels > 1 )
    {
        // convert multi channel to mono
        stereo2mono( m_buffer, num_frames, buffer );
    }
    else
    {
        // copy
        memcpy( buffer, m_buffer, sizeof(SAMPLE) * thismuch );
    }

    // scale
    for( i = 0; i < thismuch; i++ )
        buffer[i] *= signal_scale;

    // gain
    gain_buf( buffer, num_frames, m_gain );

	// time
	m_time_elapsed += num_frames;

    return TRUE;
}


//-----------------------------------------------------------------------------
// name: save()
// desc: save to another file
//		 samples [fromIndex, toIndex)
//-----------------------------------------------------------------------------
t_TAPBOOL AudioSrcFile::save(std::string & filename, t_TAPUINT fromIndex, t_TAPUINT toIndex, 
	t_TAPBOOL scale) 
{
	if( sf == NULL ) {
		BB_log( BB_LOG_INFO, "AudioSrcFile: no file exists; can't save" );
		return FALSE; 
	}
	if( fromIndex >= this->frames() || toIndex > this->frames() ) {
		BB_log( BB_LOG_INFO, "AudioSrcFile: index exceeds file size; can't save" );
		return FALSE;
	}
	// make buffer and read
	int nframes = toIndex - fromIndex;
	int bufsize = nframes < 81920 ? nframes : 81920; 
	SAMPLE * buffer = new SAMPLE[bufsize * sf_info.channels];
	if( !buffer ) {
		BB_log( BB_LOG_WARNING, "AudioSrcFile: could not allocate buffer; can't save" );
		return FALSE;
	}
	// set up file
	SF_INFO write_info = sf_info;
	SNDFILE * write_sf = AudioCentral::instance()->m_cachemanager->opensf( filename.c_str(), SFM_WRITE, &write_info );
	if( !write_sf ) {
		BB_log( BB_LOG_INFO, "AudioSrcFile: couldn't open file to save to" );
		return FALSE;
	}
	// read, scale and write
	while( fromIndex < toIndex ) {
		t_TAPUINT thismuch = toIndex - fromIndex < bufsize ? toIndex - fromIndex : bufsize;
		sf_seek( sf, fromIndex, SEEK_SET );
		sf_count_t read = sf_readf_float( sf, buffer, thismuch );
		fromIndex += read;
		if( scale ) {
			for( int b = 0; b < read; b++ )
				buffer[b] *= signal_scale; 
		}
		sf_writef_float( write_sf, buffer, read );
	}	
	// wrap up
	sf_close( write_sf );
	SAFE_DELETE_ARRAY( buffer );
	return TRUE;
}


//-----------------------------------------------------------------------------
// MICBUFFER
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// name: MicBuffer()
// desc: ...
//-----------------------------------------------------------------------------
MicBuffer::MicBuffer()
{
	m_buffer = NULL;
	m_buffer_frames = m_max_buffer_frames = m_mic_channels = m_frames_recorded = 0;
	m_write_index = 0;
	m_is_recording = FALSE;
}


//-----------------------------------------------------------------------------
// name: ~MicBuffer()
// desc: ...
//-----------------------------------------------------------------------------
MicBuffer::~MicBuffer()
{
	// stop recording and free space
	this->set_recording( FALSE );
	this->close();
}

//-----------------------------------------------------------------------------
// name: open()
// desc: allocate buffer space and clear buffer
//-----------------------------------------------------------------------------
t_TAPBOOL MicBuffer::open( t_TAPUINT num_frames, t_TAPUINT channels )
{
	// if recording is on, turn it off
	if( m_is_recording )
		this->set_recording( FALSE );
	// re-allocate buffer if needed
	if( num_frames * channels > m_max_buffer_frames * m_mic_channels ) {
		if( m_buffer ) {
			SAFE_DELETE_ARRAY( m_buffer );
		}
		m_buffer = new SAMPLE[num_frames * channels];
		m_max_buffer_frames = num_frames;
	}
	// re-set variables and clear buffer
	m_mic_channels = channels;
	m_buffer_frames = num_frames;
	memset( m_buffer, 0, m_buffer_frames * m_mic_channels * sizeof(SAMPLE) );
	m_frames_recorded = 0;
	m_write_index = 0;
	return TRUE;
}


//-----------------------------------------------------------------------------
// name: close()
// desc: free buffer space
//-----------------------------------------------------------------------------
t_TAPBOOL MicBuffer::close() {
	// if recording is on, turn it off
	if( m_is_recording )
		this->set_recording( FALSE );
	// delete buffer
	SAFE_DELETE_ARRAY( m_buffer ); 
	// re-set variables
	m_buffer_frames = 0;
	m_max_buffer_frames = 0;
	m_frames_recorded = 0;
	m_write_index = 0;
	return TRUE;
}

//-----------------------------------------------------------------------------
// name: set_recording()
// desc: start / stop recording
//-----------------------------------------------------------------------------
t_TAPBOOL MicBuffer::set_recording( t_TAPBOOL record ) {
	return m_is_recording = record;
}

//-----------------------------------------------------------------------------
// name: get_recording()
// desc: is it recording?
//-----------------------------------------------------------------------------
t_TAPBOOL MicBuffer::get_recording() {
	return m_is_recording;
}	


//-----------------------------------------------------------------------------
// name: add_samples()
// desc: add recorded samples to buffer
//-----------------------------------------------------------------------------
t_TAPBOOL MicBuffer::add_samples( SAMPLE * buffer, t_TAPUINT num_frames ) {
	if( !m_is_recording )
		return FALSE;
	// for now, keeping it simple
	if( m_frames_recorded < m_buffer_frames ) {
		SAMPLE * dst = m_buffer + m_frames_recorded * m_mic_channels;
		SAMPLE * src = buffer;
		t_TAPUINT to_add = num_frames <= (m_buffer_frames - m_frames_recorded) ? 
			num_frames : m_buffer_frames - m_frames_recorded; 
		memcpy( dst, src, to_add * m_mic_channels * sizeof(SAMPLE) ); 
		m_frames_recorded += to_add; 
	}
	return (m_frames_recorded <= m_buffer_frames); 
}


//-----------------------------------------------------------------------------
// name: get_samples()
// desc: read samples from internal buffer [fromIndex, toIndex)
//		 assume big enough buffer
//-----------------------------------------------------------------------------
t_TAPBOOL MicBuffer::get_samples( SAMPLE * buffer, t_TAPUINT fromIndex, t_TAPUINT toIndex ) {
	if( fromIndex >= m_frames_recorded || toIndex > m_frames_recorded ) {
		BB_log( BB_LOG_INFO, "MicBuffer : not enough samples to get" );
		return FALSE;
	}
	SAMPLE * dst = buffer;
	SAMPLE * src = m_buffer + fromIndex * m_mic_channels; 
	t_TAPUINT num_samples = (toIndex - fromIndex) * m_mic_channels; 
	memcpy( dst, src, num_samples * sizeof(SAMPLE) );
	return TRUE;
}

//-----------------------------------------------------------------------------
// name: get_frames()
// desc: number of frames recorded
//-----------------------------------------------------------------------------
t_TAPUINT MicBuffer::get_frames() {
	return m_frames_recorded;
}

//-----------------------------------------------------------------------------
// name: get_channels()
// desc: number of frames recorded
//-----------------------------------------------------------------------------
t_TAPUINT MicBuffer::get_channels() {
	return m_mic_channels;
}

//-----------------------------------------------------------------------------
// name: maxval()
// desc: maximum recorded value
//-----------------------------------------------------------------------------
SAMPLE MicBuffer::maxval() {
	SAMPLE max = 0; 
	for( t_TAPUINT i = 0; i < m_frames_recorded; i++ ) {
		if( m_buffer[i] > max )
			max = m_buffer[i];
		else if( -m_buffer[i] > max )
			max = -m_buffer[i]; 
	}
	return max;
}

//-----------------------------------------------------------------------------
// name: rewind()
// desc: reset write index
//-----------------------------------------------------------------------------
t_TAPBOOL MicBuffer::rewind() {
	if( m_is_recording )
		BB_log( BB_LOG_WARNING, "[Mic] Rewinding while recording! Danger!" );
	m_write_index = 0;
	m_frames_recorded = 0;
	return TRUE;
}


//-----------------------------------------------------------------------------
// AUDIOSRCMIC
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// name: AudioSrcMic()
// desc: ...
//-----------------------------------------------------------------------------
AudioSrcMic::AudioSrcMic() 
	: AudioSrcBuffer()
{
	sf = NULL;
	m_start = 0;
	m_end = 0;
	signal_scale = 1.0f;
	m_lasterror = "";
	m_read_index = 0;
	slide = NULL;
	slide_locally = NULL;
}


//-----------------------------------------------------------------------------
// name: ~AudioSrcMic()
// desc: ...
//-----------------------------------------------------------------------------
AudioSrcMic::~AudioSrcMic()
{
    if( sf )
    {
        sf_close( sf );
    }
	// AudioSrc variable
    if( on ) *on = false;
}


//-----------------------------------------------------------------------------
// name: rewind()
// desc: reset read index
//-----------------------------------------------------------------------------
t_TAPBOOL AudioSrcMic::rewind()
{
	if( AudioSrc::rewind() ) {
		m_read_index = 0;
		return TRUE;
	}
	return FALSE;
}


//-----------------------------------------------------------------------------
// name: open()
// desc: open
//-----------------------------------------------------------------------------
t_TAPBOOL AudioSrcMic::open( t_TAPUINT start, t_TAPUINT len, t_TAPBOOL scale )
{
	if( AudioCentral::instance()->m_mic_buffer == NULL ) {
		BB_log( BB_LOG_WARNING, "AudioSrcMic : no mic buffer exists; can't open" );
		return FALSE; 
	}
	m_start = start; 
	m_end = (m_start + len < this->frames() && len > 0) ? m_start + len : this->frames(); 
	m_read_index = m_start;
	if( scale ) {
		SAMPLE max = AudioCentral::instance()->m_mic_buffer->maxval();
		signal_scale = 0.95f / max; 
	}
	slide = NULL;
	slide_locally = NULL;
	BB_log( BB_LOG_INFO, "AudioSrcMic : opened; start = %d, end = %d", m_start, m_end );
	return TRUE; 
}


//-----------------------------------------------------------------------------
// name: seek()
// desc: reset read index
//-----------------------------------------------------------------------------
t_TAPBOOL AudioSrcMic::seek( t_TAPUINT where, t_TAPUINT how ) {
	if( AudioCentral::instance()->m_mic_buffer == NULL ) {
		BB_log( BB_LOG_WARNING, "AudioSrcMic : no mic buffer exists; can't seek" );
		return FALSE; 
	}
	// checks
    if( how == SEEK_CUR )
        where += m_read_index; 
    else if( how != SEEK_SET )
        return FALSE;
    if( where < 0 )
        return FALSE;
	if(where >= this->frames()) {
		m_read_index = this->frames();
		return FALSE;
	}
    // gist
    m_read_index = where;
    return TRUE;
}


//-----------------------------------------------------------------------------
// name: current()
// desc: get current read index
//-----------------------------------------------------------------------------
t_TAPUINT AudioSrcMic::current() {
	return m_read_index; 
}

//-----------------------------------------------------------------------------
// name: frames()
// desc: number of frames recorded
//-----------------------------------------------------------------------------
t_TAPUINT AudioSrcMic::frames() {
	if( AudioCentral::instance()->m_mic_buffer == NULL ) {
		BB_log( BB_LOG_WARNING, "AudioSrcMic : no mic buffer exists; can't count" );
		return 0; 
	}
	return AudioCentral::instance()->m_mic_buffer->get_frames();
}


//-----------------------------------------------------------------------------
// name: save()
// desc: save recorded samples to file [fromIndex, toIndex)
//-----------------------------------------------------------------------------
t_TAPBOOL AudioSrcMic::save(std::string & filename, t_TAPUINT fromIndex, t_TAPUINT toIndex, 
	t_TAPBOOL scale) 
{
	MicBuffer * mb = AudioCentral::instance()->m_mic_buffer;
	if( mb == NULL ) {
		BB_log( BB_LOG_INFO, "AudioSrcMic: no mic buffer exists; can't save" );
		return FALSE; 
	}
	if( mb->get_recording() ) {
		BB_log( BB_LOG_INFO, "AudioSrcMic: mic is in use; cannot record" );
		return FALSE;
	}
	if( fromIndex >= mb->get_frames() || toIndex > mb->get_frames() ) {
		BB_log( BB_LOG_INFO, "AudioSrcMic: index exceeds buffer size; cannot record" );
		return FALSE;
	}
	// make buffer and read
	int num_samples = (toIndex - fromIndex) * mb->get_channels();
	SAMPLE * buffer = new SAMPLE[num_samples];
	if( !mb->get_samples( buffer, fromIndex, toIndex ) ) {
		BB_log( BB_LOG_INFO, "AudioSrcMic: could not get mic samples" );
		SAFE_DELETE_ARRAY( buffer );
		return FALSE;
	}
	// scale
	if( scale ) {
		t_TAPSINGLE scale_factor = mb->maxval();
		if( scale_factor != 0 )
			scale_factor = 0.95f / scale_factor;
		else
			scale_factor = 1.0f;
		for( int s = 0; s < num_samples; s++ )
			buffer[s] *= scale_factor;
	}
	// set up file
	memset( &sf_info, 0, sizeof(sf_info) );
	sf_info.samplerate = this->srate();
	sf_info.channels = mb->get_channels();
	sf_info.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
	if( sf )
		AudioCentral::instance()->m_cachemanager->closesf( sf );
	sf = AudioCentral::instance()->m_cachemanager->opensf( filename.c_str(), SFM_WRITE, &sf_info );
	t_TAPBOOL good = FALSE;
	if( sf ) {
		BB_log( BB_LOG_INFO, "AudioSrcMic: opened write file, %i channels", sf_info.channels );
		BB_pushlog();
		if( sf_writef_float( sf, buffer, toIndex - fromIndex ) ) {
			BB_log( BB_LOG_INFO, "AudioSrcMic: wrote to file" );
			good = TRUE;
		}
		else
			BB_log( BB_LOG_INFO, "AudioSrcMic: could not write to file: %s", 
				sf_error_number( sf_error( sf ) ) );
		BB_poplog();
		sf_close( sf );
		sf = NULL;
	}		
	else
		BB_log( BB_LOG_INFO, "AudioSrcMic: failed file open, %i channels: %s", sf_info.channels,
			sf_error_number( sf_error( sf ) ) );
	// delete buffer
	SAFE_DELETE_ARRAY( buffer );
	return good;
}

//-----------------------------------------------------------------------------
// name: mtick()
// desc: ...
//-----------------------------------------------------------------------------
t_TAPBOOL AudioSrcMic::mtick( SAMPLE * buffer, t_TAPUINT num_samples )
{
	// are we done?
    if( m_stop_asap )
        return FALSE;
	
	// zero
	memset( buffer, 0, num_samples * sizeof(SAMPLE) );

	// sanity
	if( AudioCentral::instance()->m_mic_buffer == NULL ) {
		BB_log( BB_LOG_WARNING, "AudioSrcMic : no mic buffer exists; can't mtick" );
		return FALSE; 
	}
    
	// how much is available
    t_TAPUINT diff = m_end - m_read_index;
    if( diff <= 0 )
    {
        if( slide_locally ) *slide_locally = false;
        return FALSE;
    }
    
	// if more than one channel
	t_TAPUINT channels = AudioCentral::instance()->m_mic_buffer->get_channels();
	t_TAPUINT thismuch = diff < num_samples ? diff : num_samples;
	if( channels > 1 )
    {
		// copy into internal buffer
		int size = num_samples * channels;
		SAMPLE * temp_buffer = new SAMPLE[size];
		AudioCentral::instance()->m_mic_buffer->get_samples( temp_buffer, m_read_index, 
			m_read_index + thismuch );
		// convert multi channel to mono
        multi2mono( temp_buffer, num_samples, channels, buffer );
		// delete temp_buffer
		SAFE_DELETE_ARRAY( temp_buffer );
    }
    else
    {
        // get directly
		AudioCentral::instance()->m_mic_buffer->get_samples( buffer, m_read_index, 
			m_read_index + thismuch );
    }

    // scale
    for( t_TAPUINT i = 0; i < thismuch; i++ )
        buffer[i] *= signal_scale;

    // gain
    gain_buf( buffer, num_samples, m_gain );

	// update slider
    if( slide != NULL )
    {
        *slide = (m_read_index - m_start) / (t_TAPSINGLE)(m_end - m_start);
		BB_log( BB_LOG_FINE, "slide = (%i - %i) / (%i - %i) = %f",
			(int)m_read_index, (int)m_start, (int)m_end, (int)m_start, *slide );
        *slide_locally = true;
    }

	// read index!
	m_read_index += thismuch;

	// time
	m_time_elapsed += num_samples;

    return TRUE;
}


//-----------------------------------------------------------------------------
// name: stick()
// desc: ...
//-----------------------------------------------------------------------------
t_TAPBOOL AudioSrcMic::stick( SAMPLE * buffer, t_TAPUINT num_frames ) 
{
	// are we done?
	if( m_stop_asap )
		return FALSE;

	// zero
	memset( buffer, 0, num_frames * 2 * sizeof(SAMPLE) );
	
	// sanity 
	if( AudioCentral::instance()->m_mic_buffer == NULL ) {
		BB_log( BB_LOG_WARNING, "AudioSrcMic : no mic buffer exists; can't stick" );
		return FALSE; 
	}
	
	// channel business	
	t_TAPUINT channels = AudioCentral::instance()->m_mic_buffer->get_channels();
	if( channels != 2 ) {
		// do mono first
		t_TAPBOOL it = mtick( buffer, num_frames );
		// convert to "stereo"
		mono2stereo( buffer, num_frames );
		// pan
		pan_buf( buffer, num_frames, m_pan );
		// leave
		return it;
	}
	// actually got stereo input
	else {
		// how much is available
		t_TAPUINT diff = m_end - m_read_index;
		if( diff <= 0 )
		{
			if( slide_locally ) *slide_locally = false;
			return FALSE;
		}
		// get samples directly
		t_TAPUINT thismuch = (diff < num_frames) ? diff : num_frames;
		AudioCentral::instance()->m_mic_buffer->get_samples( buffer, m_read_index, 
			m_read_index + thismuch );
		// scale
		for( t_TAPUINT i = 0; i < 2 * thismuch; i++ )
			buffer[i] *= signal_scale;
		// gain
		gain_buf( buffer, num_frames * 2, m_gain );
		// pan
		pan_buf( buffer, num_frames, m_pan ); 
		// update slider
		if( slide != NULL )
		{
			*slide = (m_read_index - m_start) / (t_TAPSINGLE)(m_end - m_start);
			BB_log( BB_LOG_FINE, "slide = (%i - %i) / (%i - %i) = %f",
				(int)m_read_index, (int)m_start, (int)m_end, (int)m_start, *slide );
			*slide_locally = true;
		}
		// read index!
		m_read_index += thismuch;
		// time
		m_time_elapsed += num_frames;
		
		return TRUE;
	}
}

//-----------------------------------------------------------------------------
// name: rewind()
// desc: ...
//-----------------------------------------------------------------------------
t_TAPBOOL AudioSrcFrame::rewind()
{
	if( AudioSrc::rewind() ) {
		pos = 0; 
		return TRUE;	
	}
	return FALSE;
}

//-----------------------------------------------------------------------------
// name: stick()
// desc: ...
//-----------------------------------------------------------------------------
t_TAPBOOL AudioSrcFrame::stick( SAMPLE * buffer, t_TAPUINT num_frames )
{
//    assert( m_done == FALSE );

    // are we done?
    if( m_stop_asap )
        return FALSE;

    // more data?
    if( pos >= frame->wsize )
    {
        if( slide_locally ) *slide_locally = false;
        return FALSE;
    }

    // copy
    t_TAPUINT thismuch = (frame->wsize - pos) > num_frames ? num_frames : (frame->wsize - pos);
    SAMPLE * src = frame->waveform + pos;
    SAMPLE * end = src + thismuch;
    SAMPLE * dest = buffer;
    while( src < end )
    {
        *dest = *(dest+1) = *src;
        src++;
        dest += 2;
    }

    // zero 
    if( thismuch < num_frames )
        for( t_TAPUINT i = thismuch; i < num_frames; i++ )
        {
            buffer[2*i] = 0.0f;
            buffer[2*i+1] = 0.0f;
        }

    // advance pos
    pos += thismuch;

    // update
    if( slide != NULL )
    {
        *slide = pos / (t_TAPSINGLE)frame->wsize;
        *slide_locally = true;
    }

    // gain
    gain_buf( buffer, num_frames * 2, m_gain );
    // pan
    pan_buf( buffer, num_frames, m_pan );

	// time
	m_time_elapsed += num_frames;

    return TRUE;
}



//-----------------------------------------------------------------------------
// name: mtick()
// desc: ...
//-----------------------------------------------------------------------------
t_TAPBOOL AudioSrcFrame::mtick( SAMPLE * buffer, t_TAPUINT num_samples )
{
//    assert( m_done == FALSE );

    // are we done?
    if( m_stop_asap )
        return FALSE;

    // more data?
    if( pos >= frame->wsize )
    {
        if( slide_locally ) *slide_locally = false;
        return FALSE;
    }

    // copy
    t_TAPUINT thismuch = (frame->wsize - pos) > num_samples ? num_samples : (frame->wsize - pos);
    SAMPLE * src = frame->waveform + pos;
    SAMPLE * dest = buffer;
    
    memcpy( dest, src, thismuch * sizeof(SAMPLE) ); 

    // zero 
    if( thismuch < num_samples )
        memset( buffer + thismuch, 0, (num_samples - thismuch) * sizeof(SAMPLE) ); 

    // advance pos
    pos += thismuch;

    // update
    if( slide != NULL )
    {
        *slide = pos / (t_TAPSINGLE)frame->wsize;
        *slide_locally = true;
    }

    // gain
    gain_buf( buffer, num_samples, m_gain );

	// time
	m_time_elapsed += num_samples;

    return TRUE;
}


//-----------------------------------------------------------------------------
// name: seek()
// desc: audiosrcframe seek
//-----------------------------------------------------------------------------
t_TAPBOOL AudioSrcFrame::seek( t_TAPUINT where, t_TAPUINT how )
{
    // checks
    if( how == SEEK_CUR )
        where += pos; 
    else if( how != SEEK_SET )
        return FALSE;
    if( where < 0 )
        return FALSE;
	if(where >= frame->wsize ) {
		pos = frame->wsize;
		return FALSE;
	}
    if( this->playing() )
        return FALSE;
	
    // gist
    pos = where;
    return TRUE;
}


// ...
int AudioSrcFrame::num_samples()
{
    if( frame )
        return frame->wsize;
    else
        return 0;
}



//-----------------------------------------------------------------------------
// name: stick()
// desc: ...
//-----------------------------------------------------------------------------
t_TAPBOOL AudioSrcEliot::stick( SAMPLE * buffer, t_TAPUINT num_frames )
{
//    assert( m_done == FALSE );

    // are we done?
    if( m_stop_asap )
        return FALSE;

    memset( buffer, 0, sizeof(SAMPLE) * num_frames );
    ts_io->m_audio_cb( (char *)buffer, num_frames, NULL );
    // convert to "stereo" in-place
    mono2stereo( buffer, num_frames );
    // gain
    gain_buf( buffer, num_frames * 2, m_gain );
    // pan
    pan_buf( buffer, num_frames, m_pan );

	// time
	m_time_elapsed += num_frames;

    return TRUE;
}


//-----------------------------------------------------------------------------
// name: stick()
// desc: ...
//-----------------------------------------------------------------------------
t_TAPBOOL AudioSrcOlar::stick( SAMPLE * buffer, t_TAPUINT num_frames )
{
//    assert( m_done == FALSE );

    // are we done?
    if( m_stop_asap )
        return FALSE;

    memset( buffer, 0, sizeof(SAMPLE) * num_frames );
    m_olar->m_audio_cb( (char *)buffer, num_frames, NULL );
    // convert to "stereo" in-place
    mono2stereo( buffer, num_frames );
    // gain
    gain_buf( buffer, num_frames * 2, m_gain );
    // pan
    pan_buf( buffer, num_frames, m_pan );

	// time
	m_time_elapsed += num_frames;

    return TRUE;
}



//-----------------------------------------------------------------------------
// name: set()
// desc: ...
//-----------------------------------------------------------------------------
void AudioFxReverb::set( t_TAPUINT which, t_TAPFLOAT T60 )
{
    // do nothing
    if( which == m_state ) return;

    if( m_left ) SAFE_DELETE( m_left );
    if( m_right ) SAFE_DELETE( m_right );

    // no reverb
    if( which == NONE )
    { } // do nothing
    else if( which == PRC )
    {
        m_left = new FxPRCRev( T60 );
        m_right = new FxPRCRev( T60 );
    }
    else if( which == JC )
    {
        m_left = new FxJCRev( T60 );
        m_right = new FxJCRev( T60 );
    }
    else if( which == N )
    {
        m_left = new FxNRev( T60 );
        m_right = new FxNRev( T60 );
    }
    else
        // set to none
        set( NONE, 0 );

    // set mix
    mix( m_mix );
}




//-----------------------------------------------------------------------------
// name: mix()
// desc: ...
//-----------------------------------------------------------------------------
void AudioFxReverb::mix( t_TAPFLOAT value )
{
    m_mix = value;
    if( m_left ) m_left->setEffectMix( value );
    if( m_right ) m_right->setEffectMix( value );
}




//-----------------------------------------------------------------------------
// name: stick()
// desc: ...
//-----------------------------------------------------------------------------
t_TAPBOOL AudioFxReverb::stick( SAMPLE * buffer, t_TAPUINT num_frames )
{
    if( m_left && m_mix > 0.001 )
    {
        // do left
        for( t_TAPUINT i = 0; i < num_frames; i++ )
        {
            buffer[i*2] = m_left->tick( buffer[i*2] );
        }
    }

    if( m_right && m_mix > 0.001 )
    {
        // do right
        for( t_TAPUINT i = 0; i < num_frames; i++ )
        {
            buffer[i*2+1] = m_right->tick( buffer[i*2+1] );
        }
    }

	// time
	m_time_elapsed += num_frames;

    return TRUE;
}




//-----------------------------------------------------------------------------
// name: init()
// desc: ...
//-----------------------------------------------------------------------------
t_TAPUINT AudioBus::init( t_TAPUINT num_frames )
{
    m_num_frames = num_frames;
	m_max_channels = 9; 
    m_buffer = new SAMPLE[num_frames*2];
    m_buffer2 = new SAMPLE[num_frames*2];
	m_bufferM = new SAMPLE[num_frames * m_max_channels]; 
	memset( m_buffer, 0, sizeof(SAMPLE) * num_frames * 2 );
    m_ready = FALSE;
    m_done = FALSE;
    m_gain = 1.0f;
    on = NULL;

    // allocate reverb
    m_reverb.set( AudioFxReverb::N, 4.0 );

    return TRUE;
}



//-----------------------------------------------------------------------------
// name: ~AudioBus()
// desc: ...
//-----------------------------------------------------------------------------
AudioBus::~AudioBus()
{
    SAFE_DELETE_ARRAY( m_buffer );
    SAFE_DELETE_ARRAY( m_buffer2 );
	SAFE_DELETE_ARRAY( m_bufferM );
}




//-----------------------------------------------------------------------------
// name: stick()
// desc: ...
//-----------------------------------------------------------------------------
t_TAPBOOL AudioBus::stick( SAMPLE * buffer, t_TAPUINT num_frames )
{
//    assert( m_done == FALSE );

    // buses always return true
    if( m_src.size() == 0 )
    {
        memset( buffer, 0, sizeof(SAMPLE) * num_frames * 2 );
        // need to do reverb
        m_reverb.stick( buffer, num_frames );
        return TRUE;
    }

    SAMPLE * buf = buffer ? buffer : m_buffer;
    SAMPLE * buf2 = m_buffer2;
    t_TAPUINT size = num_frames ? num_frames : m_num_frames;
    t_TAPBOOL yes = FALSE;

    // zero out the buffer
    memset( buf, 0, sizeof(SAMPLE) * size * 2 );
    
    assert( num_frames == 0 || size <= m_num_frames );

    // lock
    m_mutex.acquire();
    // clear
    m_src2.clear();
	// loop
    for( int i = 0; i < m_src.size(); i++ )
    {
        // get samples
        yes = m_src[i]->stick( buf2, size );
        if( !yes )
        {
            // log
            BB_log( BB_LOG_FINE, "audio source '%s' done...", m_src[i]->m_name.c_str() );

            if( m_src[i]->m_delete )
                delete m_src[i];
            else
                m_src[i]->m_done = TRUE;
        }
        else
        {
            // if there is more
            m_src2.push_back( m_src[i] );
            // add
            for( t_TAPUINT j = 0; j < size*2; j++ )
                buf[j] += buf2[j];
        }
    }
    // gain
    gain_buf( buf, size*2, m_gain );
    // pan
    pan_buf( buf, size, m_pan );
    // reverb
    m_reverb.stick( buf, size );
    // copy
    if( buffer && buffer != buf )
        memcpy( buffer, buf, sizeof(SAMPLE) * num_frames * 2 );
    // more src
    m_src = m_src2;
	// time
	m_time_elapsed += num_frames;
    // release
    m_mutex.release();

    // no return false because this is a bus
    return TRUE;
}


//-----------------------------------------------------------------------------
// name: multitick()
// desc: stick into stereobuffer, multiple single channels into multibuffer
//-----------------------------------------------------------------------------
t_TAPBOOL AudioBus::multitick( SAMPLE * stereobuffer, SAMPLE * multibuffer, t_TAPUINT num_frames )
{
//    assert( m_done == FALSE );

    // buses always return true
    if( m_src.size() == 0 )
    {
        memset( stereobuffer, 0, sizeof(SAMPLE) * num_frames * 2 );
        // memset( multibuffer, 0, sizeof(SAMPLE) * num_frames * m_src.size() );
		// need to do reverb
        m_reverb.stick( stereobuffer, num_frames );
        return TRUE;
    }

    SAMPLE * buf = stereobuffer ? stereobuffer : m_buffer;
    SAMPLE * buf2 = m_buffer2;
	SAMPLE * bufM = multibuffer; 
	int M = m_src.size(); 
    t_TAPUINT size = num_frames ? num_frames : m_num_frames;
    t_TAPBOOL yes = FALSE;

    // zero out the buffers
    memset( buf, 0, sizeof(SAMPLE) * size * 2 );
	if( bufM ) memset( bufM, 0, sizeof(SAMPLE) * size * M );


    assert( num_frames == 0 || size <= m_num_frames );

    // lock
    m_mutex.acquire(); 
	// clear
    m_src2.clear();
    // loop
    for( int i = 0; i < M; i++ )
    {
        // get samples
        yes = m_src[i]->stick( buf2, size );
        if( !yes )
        {
            // log
            BB_log( BB_LOG_FINE, "audio source '%s' done...", m_src[i]->m_name.c_str() );

            if( m_src[i]->m_delete )
                delete m_src[i];
            else
                m_src[i]->m_done = TRUE;
        }
        else
        {
            // if there is more
            m_src2.push_back( m_src[i] );
            // add for stereo buffer
            for( t_TAPUINT j = 0; j < size*2; j++ )
                buf[j] += buf2[j];
			// pre-process + interleave for multibuffer
			if( bufM )
			{
				gain_buf( buf2, size*2, m_gain ); 
				pan_buf( buf2, size, m_pan );
				//m_reverb.stick( buf2, size );
				int source = i;
				for( t_TAPUINT sample = 0; sample < size; sample++ )
					bufM[sample*M + source] = (buf2[2*sample] + buf2[2*sample+1])/2;
			}
        }
    }
	// post-process for stereo buffer
    // gain
    gain_buf( buf, size*2, m_gain );
    // pan
    pan_buf( buf, size, m_pan );
    // reverb
    m_reverb.stick( buf, size );
    // copy
    if( stereobuffer && stereobuffer != buf )
        memcpy( stereobuffer, buf, sizeof(SAMPLE) * num_frames * 2 );
    // more src
    m_src = m_src2;
	// time
	m_time_elapsed += num_frames;
    // release
    m_mutex.release();

    // no return false because this is a bus
    return TRUE;
}


//-----------------------------------------------------------------------------
// name: play()
// desc: ...
//-----------------------------------------------------------------------------
void AudioBus::play( AudioSrc * src, t_TAPBOOL solo, t_TAPBOOL rew )
{
    // solo
    if( solo ) this->stop();

    // synchronize
    m_mutex.acquire();
    src->reset();
    if( rew )
        src->rewind();
    // add to vector
    t_TAPBOOL got = FALSE;
    for( t_TAPINT i = 0; i < m_src.size(); i++ )
        if( m_src[i] == src ) { got = TRUE; break; }
    if( !got )
	{
        m_src.push_back( src );
//		fprintf( stdout, "Bus %x push back %x\n", this, src );
//		for( t_TAPINT i = 0; i < m_src.size(); i++ )
//			fprintf( stdout, "  %x\n", m_src[i] );
	}
    // release
    m_mutex.release();
}




//-----------------------------------------------------------------------------
// name: play()
// desc: ...
//-----------------------------------------------------------------------------
void AudioBus::remove( AudioSrc * src )
{
    // synchronize
    m_mutex.acquire();
//	vector<AudioSrc *> temp;
    // clear src2
    m_src2.clear();
    // copy
    for( t_TAPUINT i = 0; i < m_src.size(); i++ )
    {
        // match
        if( m_src[i] != src )
            m_src2.push_back( m_src[i] );
//        if( m_src[i] != src )
//            temp.push_back( m_src[i] );
//		if( m_src[i] == src )
//			m_src.erase( m_src.begin() + i-- ); 
    }
    // copy
	m_src = m_src2;
	m_src2.clear();
	//m_src = temp;
    // synchronize
    m_mutex.release();
}




//-----------------------------------------------------------------------------
// name: stop()
// desc: ...
//-----------------------------------------------------------------------------
void AudioBus::stop()
{
    // synchronize
    m_mutex.acquire();
    // delete
    for( long i = 0; i < m_src.size(); i++ )
    {
        // log
        BB_log( BB_LOG_FINE, "audio source '%s' done...", m_src[i]->m_name.c_str() );

        if( m_src[i]->m_delete )
            delete m_src[i];
        else
            m_src[i]->m_done = TRUE;
    }

    // clear
    m_src.clear();
    // more
    m_ready = FALSE;
    // release
    m_mutex.release();
}




//-----------------------------------------------------------------------------
// name: the_parallel()
// desc: (METHOD A)
//-----------------------------------------------------------------------------
#ifdef __PLATFORM_WIN32__
unsigned __stdcall the_parallel( void * data )
#else
void * the_parallel( void * data )
#endif
{
    AudioBusParallel * parallel = (AudioBusParallel *)data;

    // loop until done
    while( parallel->m_done == FALSE )
    {
        // wait for signal
        while( parallel->m_ready )
            usleep( 1000 );

        // accumulate
        parallel->accumulate();

        // wait
        parallel->m_ready = TRUE;
    }

    return 0;
}




//-----------------------------------------------------------------------------
// name: AudioBusParallel()
// desc: ...
//-----------------------------------------------------------------------------
AudioBusParallel::AudioBusParallel( t_TAPUINT num_frames, t_TAPUINT sig_frames )
    : AudioBus( num_frames )
{
    sf_stereo = NULL;
	sf_multi = NULL;
    m_sig_frames = sig_frames;
    // METHOD A: start thread
    // METHOD A: m_thread.start( the_parallel, this );
}


bool AudioBusParallel::open( int mode, const string & filename, const string & filename2 )
{
	bool ret;

	if( mode == AudioCentral::STEREO || mode == AudioCentral::BOTH )
	{
		m_sf_stereo_mutex.acquire();
		memset( &sf_stereo_info, 0, sizeof(sf_stereo_info) );
		sf_stereo_info.samplerate = BirdBrain::srate();
		sf_stereo_info.channels = 2;
		sf_stereo_info.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
		if( sf_stereo )
			AudioCentral::instance()->m_cachemanager->closesf( sf_stereo );
		sf_stereo = AudioCentral::instance()->m_cachemanager->opensf( filename.c_str(), SFM_WRITE, &sf_stereo_info );
		if( sf_stereo )
			BB_log( BB_LOG_INFO, "AudioBusParallel: opened write file, %i channels", sf_stereo_info.channels );
		else
			BB_log( BB_LOG_INFO, "AudioBusParallel: failed open, %i channels", sf_stereo_info.channels );
		ret = (sf_stereo != NULL);
		m_sf_stereo_mutex.release();
	}
	if( (mode == AudioCentral::MULTICHANNEL || mode == AudioCentral::BOTH) && m_src.size() > 0 )
	{
		m_sf_multi_mutex.acquire();
		memset( &sf_multi_info, 0, sizeof(sf_multi_info) );
		sf_multi_info.samplerate = BirdBrain::srate();
		sf_multi_info.channels = m_src.size();
		sf_multi_info.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
		if( sf_multi )
			AudioCentral::instance()->m_cachemanager->closesf( sf_multi );
		sf_multi = AudioCentral::instance()->m_cachemanager->opensf( 
			( mode == AudioCentral::BOTH ) ? filename2.c_str() : filename.c_str(), 
			SFM_WRITE, &sf_multi_info );
		if( sf_multi )
			BB_log( BB_LOG_INFO, "AudioBusParallel: opened write file, %i channels", sf_multi_info.channels );
		else
			BB_log( BB_LOG_INFO, "AudioBusParallel: failed open, %i channels", sf_multi_info.channels );
		ret = ret && (sf_multi != NULL);
		m_sf_multi_mutex.release();
	}

//    sf = sf_open( filename.c_str(), SFM_WRITE, &sf_info );

    return ret;
}


// close all
void AudioBusParallel::close()
{
	m_sf_stereo_mutex.acquire(); 
	if( sf_stereo )
	{
		AudioCentral::instance()->m_cachemanager->closesf( sf_stereo );
		sf_stereo = NULL;
	}
	m_sf_stereo_mutex.release();
	m_sf_multi_mutex.acquire();
	if( sf_multi )
	{
		AudioCentral::instance()->m_cachemanager->closesf( sf_multi );
		sf_multi = NULL;
	}
	m_sf_multi_mutex.release(); 
//    if( sf )
////        sf_close( sf );
//		AudioCentral::instance()->m_cachemanager->closesf( sf );
//    sf = NULL;
}

void AudioBusParallel::close( int mode )
{
    m_sf_stereo_mutex.acquire();
	if( (mode == AudioCentral::STEREO || mode == AudioCentral::BOTH) && sf_stereo )
	{
		AudioCentral::instance()->m_cachemanager->closesf( sf_stereo );
		sf_stereo = NULL;
	}
	m_sf_stereo_mutex.release();
	m_sf_multi_mutex.acquire();
	if( (mode == AudioCentral::MULTICHANNEL || mode == AudioCentral::BOTH) && sf_multi )
	{
		AudioCentral::instance()->m_cachemanager->closesf( sf_multi );
		sf_multi = NULL;
	}
	m_sf_multi_mutex.release();
}




//-----------------------------------------------------------------------------
// name: stick()
// desc: ...
//-----------------------------------------------------------------------------
t_TAPBOOL AudioBusParallel::stick( SAMPLE * buffer, t_TAPUINT num_frames )
{
//    assert( m_done == FALSE );

    if( m_src.size() == 0 )
    {
        memset( buffer, 0, sizeof(SAMPLE) * num_frames * 2 );
        return TRUE;
    }

    // METHOD A: wait for data
    // while( !m_ready )
    //     usleep( 2000 );

    // METHOD B: callback (no blocking)
    accumulate();

    // copy
    memcpy( buffer, m_buffer, sizeof(SAMPLE) * num_frames * 2 );

	// time
	m_time_elapsed += num_frames;

    // METHOD A: signal (no harm to keep in METHOD B)
    m_ready = FALSE;

    return TRUE;
}


//-----------------------------------------------------------------------------
// name: multitick()
// desc: ...
//-----------------------------------------------------------------------------
t_TAPBOOL AudioBusParallel::multitick( SAMPLE * stereobuffer, SAMPLE * multibuffer, t_TAPUINT num_frames )
{
//    assert( m_done == FALSE );

    if( m_src.size() == 0 )
    {
        memset( stereobuffer, 0, sizeof(SAMPLE) * num_frames * 2 );
        return TRUE;
    }

    // METHOD A: wait for data
    // while( !m_ready )
    //     usleep( 2000 );

    // METHOD B: callback (no blocking)
    accumulate();

    // copy
    memcpy( stereobuffer, m_buffer, sizeof(SAMPLE) * num_frames * 2 );
	memcpy( multibuffer, m_bufferM, sizeof(SAMPLE) * num_frames * m_src.size() );

	// time
	m_time_elapsed += num_frames;

    // METHOD A: signal (no harm to keep in METHOD B)
    m_ready = FALSE;

    return TRUE;
}



//-----------------------------------------------------------------------------
// name: accumulate()
// desc: ...
//-----------------------------------------------------------------------------
void AudioBusParallel::accumulate( )
{
    t_TAPUINT togo = m_num_frames;
    t_TAPUINT num_frames = m_sig_frames;

    // loop
    while( togo > 0 )
    {
        // check
        if( num_frames > togo ) num_frames = togo;

        // tick sig
        //AudioBus::stick( m_buffer + (m_num_frames-togo)*2, num_frames );
    	int channels = m_src.size() < m_max_channels ? m_src.size() : m_max_channels; 
		AudioBus::multitick( m_buffer + (m_num_frames-togo)*2, 
							 m_bufferM + (m_num_frames-togo) * channels,  
							 num_frames );

        // keep track of frames to go
        togo -= num_frames;
    }

    // for performance reasons (we are not stupid)
    if( sf_stereo )
    {
        m_sf_stereo_mutex.acquire();
        // in METHOD B, this may cause more audio clicks (maybe)
        if( sf_stereo ) sf_writef_float( sf_stereo, m_buffer, m_num_frames );
        m_sf_stereo_mutex.release();
    }
	if( sf_multi )
    {
        m_sf_multi_mutex.acquire();
        // in METHOD B, this may cause more audio clicks (maybe)
        if( sf_multi ) sf_writef_float( sf_multi, m_bufferM, m_num_frames );
        m_sf_multi_mutex.release();
    }
}


// what's going on???
SNDFILE * CacheManager::opensf( std::string path, int mode, SF_INFO * info, int id, t_TAPBOOL usecache )
{
	// allowed to use cached version, and it's already open
	if( usecache && mode == SFM_READ && isopen( path ) )
	{
		BB_log( BB_LOG_INFO, "[CacheManager]: '%s' was already open (%d)", path.c_str(), id );
		std::vector<CacheInfo> *civ = &(names[path]);
		if( id < 0 )
			id = 0;
		if( id < civ->size() )
		{
			BB_log( BB_LOG_FINE, "[CacheManager]: size: %d", civ->size() );
			CacheInfo ci = civ->at( id );
			ci.count++;
			ci.close = false;
			*info = ci.lastinfo;
			// rewind
			sf_seek( ci.sf, 0, SEEK_SET );
			(*civ)[id] = ci;
			return ci.sf; 
		}
	}
	// need to open
	BB_log( BB_LOG_FINE, "[CacheManager]: %s", path.c_str() );
	SNDFILE * sf = sf_open( path.c_str(), mode, info );
	if( sf == NULL )
	{
		BB_log( BB_LOG_WARNING, "[CacheManager]: could not open '%s'; %s", path.c_str(), 
			sf_error_number( sf_error( sf ) ) );
	}
	else
	{
		if( mode != SFM_READ || !usecache )
			return sf;
		// otherwise
		CacheInfo ci; 
		ci.sf = sf;
		ci.count = 1;
		ci.close = false;
		ci.lastinfo = *info;
		names[path].push_back( ci );
		BB_log( BB_LOG_INFO, "[CacheManager]: opening '%s'", path.c_str() );
		// rewind
		sf_seek( sf, 0, SEEK_SET );
	}
	return sf;
}


// must not already be open with this id
bool CacheManager::addsf( std::string path, int mode, const SF_INFO &info, SNDFILE * sf, int id )
{
	if( isopen( path ) > id )
	{
		BB_log( BB_LOG_WARNING, "[CacheManager]: '%s' already open, not adding", path.c_str() );
		return false;
	}
	else if( mode != SFM_READ )
	{
		BB_log( BB_LOG_WARNING, "[CacheManager]: non-read-only file not included in cache" );
		return false;
	}
	else
	{
		CacheInfo ci; 
		ci.sf = sf;
		ci.close = false;
		ci.count = 1; 
		ci.lastinfo = info;
		std::vector<CacheInfo> * civ = &(names[path]);
		if( id < civ->size() )
			(*civ)[id] = ci;
		else
		{
			civ->push_back( ci );
		}
		BB_log( BB_LOG_INFO, "[CacheManager]: adding '%s'", path.c_str() );
	}
	return true;
}


void CacheManager::closesf( SNDFILE * sf )
{
	std::map<std::string, std::vector<CacheInfo> >::iterator iter;
	bool found = false;
	for( iter = names.begin(); iter != names.end(); )
	{
		std::vector<CacheInfo> * civ = &(iter->second);
		bool erased = false;
		for( int i = 0; i < civ->size(); i++ )
		{
			CacheInfo ci = (*civ)[i];
			if( ci.sf == sf )
			{
				found = true;
				std::string path = iter->first;
				if( --ci.count < 0 ) ci.count = 0;
				if( ci.close && ci.count == 0 )
				{
					BB_log( BB_LOG_INFO, "[CacheManager]: closing '%s'", path.c_str() );
					sf_close( sf ); 
					// erase
					civ->erase( civ->begin() + i-- );
					if( civ->empty() )
					{
						names.erase( iter++ );
						erased = true;
					}
				}
				else
				{
					// update
					(*civ)[i] = ci;
					BB_log( BB_LOG_FINE, "[CacheManager]: '%s' not closed just in case it will be used again", path.c_str() );
				}
				break;
			}
		}
		if( !erased ) ++iter;
	}
	// may not be cached, still needs to be closed
	if( !found )
		sf_close( sf );
}


int CacheManager::isopen( std::string path )
{
	std::map<std::string, std::vector<CacheInfo> >::iterator iter = names.find( path );
	if( iter != names.end() )
		return iter->second.size();
	else
		return 0;
}


// AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH!!!!!!!!!!!!!!!!!!!
