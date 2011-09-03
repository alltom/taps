//-----------------------------------------------------------------------------
// name: ui_audio.cpp
// desc: birdbrain ui audio
//
// authors: Ananya Misra (amisra@cs.princeton.edu)
//          Ge Wang (gewang@cs.princeton.edu)
//          Perry R. Cook (prc@cs.princeton.edu)
//          Philip Davidson (philipd@cs.princeton.edu)
// date: Winter 2004
//-----------------------------------------------------------------------------
#include "ui_audio.h"
#include "Eliot.h"
#include <stdio.h>
#include <memory.h>
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
    t_CKUINT num_frames = ac->m_num_frames;
    t_CKUINT num_samples = num_frames * ac->m_channels;
    t_CKBOOL yes = FALSE;

    // zero out the buffer 
    assert( num_frames == buffer_size );
    memset( buffer, 0, sizeof(SAMPLE) * num_samples );

    // lock
    ac->m_mutex.acquire();
    // get samples
    yes = ac->m_bus->stick( (SAMPLE *)buffer, num_frames );
    // release
    ac->m_mutex.release();
    
    return 0;
}




//-----------------------------------------------------------------------------
// name: mono2stereo()
// desc: on same buffer
//-----------------------------------------------------------------------------
void AudioSrc::mono2stereo( SAMPLE * self, t_CKUINT num_frames )
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
void AudioSrc::stereo2mono( SAMPLE * self, t_CKUINT num_frames )
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
// name: mono2stereo()
// desc: ...
//-----------------------------------------------------------------------------
void AudioSrc::mono2stereo( const SAMPLE * mono, t_CKUINT num_frames, SAMPLE * stereo )
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
void AudioSrc::stereo2mono( const SAMPLE * stereo, t_CKUINT num_frames, SAMPLE * mono )
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
// name: pan_buf()
// desc: you know, the mythical creature
//-----------------------------------------------------------------------------
void AudioSrc::pan_buf( SAMPLE * stereo, t_CKUINT num_frames, t_CKSINGLE where )
{
    t_CKSINGLE left_gain = where <= .5f ? 1.0f : (1.0f - where) * 2;
    t_CKSINGLE right_gain = where >= .5f ? 1.0f : where * 2;
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
void AudioSrc::gain_buf( SAMPLE * buffer, t_CKUINT num_samples, SAMPLE gain )
{
    if( gain != 1.0f )
        for( t_CKUINT i = 0; i < num_samples; i++ )
            buffer[i] *= gain;
}




AudioCentral * AudioCentral::instance()
{
    if( !our_instance )
        our_instance = new AudioCentral;

    return our_instance;
}




//-----------------------------------------------------------------------------
// name: init()
// desc: ...
//-----------------------------------------------------------------------------
t_CKBOOL AudioCentral::init( t_CKUINT srate, t_CKUINT num_frames,
                             t_CKUINT sig_frames, t_CKUINT bus )
{
    // if already started...
    if( m_audio != NULL )
    {
        fprintf( stderr, "[ui_audio]: already started...\n" );
        return FALSE;
    }

    // frame
    if( num_frames < sig_frames )
    {
        fprintf( stderr, "[ui_audio]: num_frames < sig_framds - bad!\n" );
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
    t_CKINT num_buffers = 4;

    // log
    BB_log( BB_LOG_SYSTEM, "sample rate: %d", srate );
    BB_log( BB_LOG_SYSTEM, "buffer size: %d", num_frames );
    BB_log( BB_LOG_SYSTEM, "signal size: %d", sig_frames );
    BB_log( BB_LOG_SYSTEM, "num buffers: %d", num_buffers );
    BB_log( BB_LOG_SYSTEM, "channels: %d", m_channels );

    // open the device
    try
    {
        int size = m_num_frames;
        // open the audio device for capture and playback
        m_audio = new RtAudio( 0, m_channels, 0, 0, RTAUDIO_FLOAT32,
            m_srate, &size, num_buffers );
        m_num_frames = size;
    }
    catch( RtError & e )
    {
        // exception
        fprintf( stderr, "[ui_audio]: %s\n", e.getMessage().c_str() );
        fprintf( stderr, "[ui_audio]: cannot open audio device for playback...\n" );
        // zero
        m_audio = NULL;

        return FALSE;
    }

    // allocate buffers
    m_buffer = new SAMPLE[m_num_frames * m_channels];
    m_buffer2 = new SAMPLE[m_num_frames * m_channels];

    // log
    BB_log( BB_LOG_SYSTEM, "allocating parallel bus..." );
    // draw parallel
    m_bus = new AudioBusParallel( m_num_frames, m_sig_frames );
    m_bus->m_delete = FALSE;
    
    // log
    BB_log( BB_LOG_SYSTEM, "allocating %d audio buses...", bus );
    // push log
    BB_pushlog();
    for( t_CKUINT i = 0; i < bus; i++ )
    {
        // log
        BB_log( BB_LOG_INFO, "initializing audio bus %d...", i );
        AudioBus * bus = new AudioBus( m_num_frames );
        bus->m_delete = FALSE;
        m_bus->m_src.push_back( bus );
    }
    // pop log
    BB_poplog();

    // set the audio callback
    m_audio->setStreamCallback( cb, this );

    // log
    BB_log( BB_LOG_SYSTEM, "starting real-time audio..." );
    // start the audio
    m_audio->startStream( );

    // pop log
    BB_poplog();

    return TRUE;
}




//-----------------------------------------------------------------------------
// name: free()
// desc: ...
//-----------------------------------------------------------------------------
t_CKBOOL AudioCentral::free()
{
    // if not started
    if( m_audio == NULL )
        return FALSE;

    // clear the queue
    this->stop_all();
    // clear bus
    delete m_bus;

    // stop the audio
    m_audio->stopStream();
    
    // free the audio device
    delete m_audio;

    // free buffers
    delete [] m_buffer;
    delete [] m_buffer2;

    // zero
    m_audio = NULL;
    m_buffer = NULL;
    m_buffer2 = NULL;
    m_bus = NULL;

    return TRUE;
}




//-----------------------------------------------------------------------------
// name: bus()
// desc: ...
//-----------------------------------------------------------------------------
AudioBus * AudioCentral::bus( t_CKUINT index )
{
    if( index < 0 || index >= m_bus->m_src.size() )
        return NULL;

    return (AudioBus *)m_bus->m_src[index];
}




//-----------------------------------------------------------------------------
// name: stop()
// desc: ...
//-----------------------------------------------------------------------------
t_CKBOOL AudioCentral::stop_all()
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
t_CKBOOL AudioSrcTest::stick( SAMPLE * buffer, t_CKUINT num_frames )
{
//    assert( m_done == FALSE );

    // are we done?
    if( m_stop_asap )
        return FALSE;

    // 1 second
    if( m_t > 44100.0 )
    {
        m_done = TRUE;
        return FALSE;
    }

    // samples
    for( t_CKUINT i = 0; i < num_frames; i++ )
    {
        buffer[i*2] = .25 * sin( m_t * m_freq * 2.0 * 3.1415926 / 44100.0 );
        buffer[i*2+1] = buffer[i*2];
        m_t += 1.0;
    }

    // gain
    gain_buf( buffer, num_frames * 2, m_gain );
    // pan
    pan_buf( buffer, num_frames, m_pan );

    return TRUE;
}




//-----------------------------------------------------------------------------
// name: AudioSrcFile()
// desc: ...
//-----------------------------------------------------------------------------
AudioSrcFile::AudioSrcFile()
    : AudioSrc()
{
    m_buffer = NULL;
    m_buffer_size = 0;
    sf = NULL;
    slide = NULL;
    slide_locally = NULL;
    signal_scale = 1.0f;
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
// name: open()
// desc: ...
//-----------------------------------------------------------------------------
t_CKBOOL AudioSrcFile::open( const char * filename, t_CKUINT start, t_CKUINT len, t_CKBOOL scale )
{
    // sanity
    if( sf )
    {
       fprintf( stderr, "[audiosrcfile]: file '%s' already open...\n", m_filename.c_str() );
       return FALSE;
    }

    // open the file
    sf = sf_open( filename, SFM_READ, &sf_info );
    if( !sf )
    {
        fprintf( stderr, "[audiosrcfile]: cannot open file '%s'...\n", filename );
        return FALSE;
    }

    // support mono and stereo only
    if( sf_info.channels > 2 )
    {
        fprintf( stderr, "[audiosrcfile]: not mono/stereo file!\n" );
        this->close();
        return FALSE;
    }

    // support 44100 srate only
    if( sf_info.samplerate != 44100 )
    {
        fprintf( stderr, "[audiosrcfile]: not 44100 hz file!\n" );
        this->close();
        return FALSE;
    }
    
    // check start and len
    if( start >= sf_info.frames )
    {
        fprintf( stderr, "[audiosrcfile]: '%s' has only %li frames...\n",
            filename, sf_info.frames );
        fprintf( stderr, "[audiosrcfile]: ...cannot seek to '%li'...\n",
            start );
        this->close();
        return FALSE;
    }

    if( len && (start + len) > sf_info.frames ) // used to be >= but... see m_end below
    {
        fprintf( stderr, "[audiosrcfile]: '%s' has only %li frames...\n",
            filename, sf_info.frames );
        fprintf( stderr, "[audiosrcfile]: ...cannot seek upto '%li'...\n",
            start + len );
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
    if( start ) sf_seek( sf, start, SEEK_SET );
    
    return TRUE;
}




//-----------------------------------------------------------------------------
// name: close()
// desc: ...
//-----------------------------------------------------------------------------
t_CKBOOL AudioSrcFile::close()
{
    if( !sf ) return FALSE;

    // deallocate buffer
    if( m_buffer ) delete [] m_buffer;
    // close
    sf_close( sf );

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
t_CKBOOL AudioSrcFile::stick( SAMPLE * buffer, t_CKUINT num_frames )
{
//    assert( m_done == FALSE );

    // are we done?
    if( m_stop_asap )
        return FALSE;

    // do mono first
    t_CKBOOL it = mtick( buffer, num_frames );
    // convert to "stereo"
    mono2stereo( buffer, num_frames, m_buffer );
    // pan
    pan_buf( buffer, num_frames, m_pan );
    // copy back
    memcpy( buffer, m_buffer, sizeof(SAMPLE) * num_frames * 2 );

    return it;
}




//-----------------------------------------------------------------------------
// name: mtick()
// desc: mono tick
//-----------------------------------------------------------------------------
t_CKBOOL AudioSrcFile::mtick( SAMPLE * buffer, t_CKUINT num_frames )
{
//    assert( m_done == FALSE );

    // are we done?
    if( m_stop_asap )
        return FALSE;

    // sanity
    if( !sf )
    {
        memset( buffer, 0, sizeof(SAMPLE) * num_frames * 2 );
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

    // read
    sf_count_t thismuch = num_frames <= diff ? num_frames : diff;
    sf_count_t read = sf_readf_float( sf, m_buffer, thismuch );
    assert( thismuch == read );

    // update
    if( slide != NULL )
    {
        *slide = (sf_seek( sf, 0, SEEK_CUR ) - m_start) / (t_CKSINGLE)(m_end - m_start);
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
    // zero 
    for( i = thismuch; i < num_frames; i++ )
        buffer[i] = 0.0f;

    // gain
    gain_buf( buffer, num_frames, m_gain );

    return TRUE;
}




//-----------------------------------------------------------------------------
// name: stick()
// desc: ...
//-----------------------------------------------------------------------------
t_CKBOOL AudioSrcFrame::stick( SAMPLE * buffer, t_CKUINT num_frames )
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
    t_CKUINT thismuch = (frame->wsize - pos) > num_frames ? num_frames : (frame->wsize - pos);
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
        for( t_CKUINT i = thismuch; i < num_frames; i++ )
        {
            buffer[2*i] = 0.0f;
            buffer[2*i+1] = 0.0f;
        }

    // advance pos
    pos += thismuch;

    // update
    if( slide != NULL )
    {
        *slide = pos / (t_CKSINGLE)frame->wsize;
        *slide_locally = true;
    }

    // gain
    gain_buf( buffer, num_frames * 2, m_gain );
    // pan
    pan_buf( buffer, num_frames, m_pan );

    return TRUE;
}



//-----------------------------------------------------------------------------
// name: mtick()
// desc: ...
//-----------------------------------------------------------------------------
t_CKBOOL AudioSrcFrame::mtick( SAMPLE * buffer, t_CKUINT num_samples )
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
    t_CKUINT thismuch = (frame->wsize - pos) > num_samples ? num_samples : (frame->wsize - pos);
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
        *slide = pos / (t_CKSINGLE)frame->wsize;
        *slide_locally = true;
    }

    // gain
    gain_buf( buffer, num_samples, m_gain );

    return TRUE;
}


//-----------------------------------------------------------------------------
// name: seek()
// desc: audiosrcframe seek
//-----------------------------------------------------------------------------
t_CKBOOL AudioSrcFrame::seek( t_CKUINT where, t_CKUINT how )
{
	// checks
	if( how == SEEK_CUR )
		where += pos; 
	else if( how != SEEK_SET )
		return FALSE;
	if( where < 0 || where >= frame->wsize )
		return FALSE;
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
t_CKBOOL AudioSrcEliot::stick( SAMPLE * buffer, t_CKUINT num_frames )
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


    return TRUE;
}




//-----------------------------------------------------------------------------
// name: set()
// desc: ...
//-----------------------------------------------------------------------------
void AudioFxReverb::set( t_CKUINT which, t_CKFLOAT T60 )
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
void AudioFxReverb::mix( t_CKFLOAT value )
{
    m_mix = value;
    if( m_left ) m_left->setEffectMix( value );
    if( m_right ) m_right->setEffectMix( value );
}




//-----------------------------------------------------------------------------
// name: stick()
// desc: ...
//-----------------------------------------------------------------------------
t_CKBOOL AudioFxReverb::stick( SAMPLE * buffer, t_CKUINT num_frames )
{
    if( m_left && m_mix > 0.001 )
    {
        // do left
        for( t_CKUINT i = 0; i < num_frames; i++ )
        {
            buffer[i*2] = m_left->tick( buffer[i*2] );
        }
    }

    if( m_right && m_mix > 0.001 )
    {
        // do right
        for( t_CKUINT i = 0; i < num_frames; i++ )
        {
            buffer[i*2+1] = m_right->tick( buffer[i*2+1] );
        }
    }

    return TRUE;
}




//-----------------------------------------------------------------------------
// name: init()
// desc: ...
//-----------------------------------------------------------------------------
t_CKUINT AudioBus::init( t_CKUINT num_frames )
{
    m_num_frames = num_frames;
    m_buffer = new SAMPLE[num_frames*2];
    m_buffer2 = new SAMPLE[num_frames*2];
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
}




//-----------------------------------------------------------------------------
// name: stick()
// desc: ...
//-----------------------------------------------------------------------------
t_CKBOOL AudioBus::stick( SAMPLE * buffer, t_CKUINT num_frames )
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
    t_CKUINT size = num_frames ? num_frames : m_num_frames;
    t_CKBOOL yes = FALSE;

    // zero out the buffer
    memset( buf, 0, sizeof(SAMPLE) * size * 2 );
    // clear
    m_src2.clear();

    assert( num_frames == 0 || size <= m_num_frames );

    // lock
    m_mutex.acquire();
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
            for( t_CKUINT j = 0; j < size*2; j++ )
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
    // release
    m_mutex.release();

    // no return false because this is a bus
    return TRUE;
}




//-----------------------------------------------------------------------------
// name: play()
// desc: ...
//-----------------------------------------------------------------------------
void AudioBus::play( AudioSrc * src, t_CKBOOL solo, t_CKBOOL rew )
{
    // solo
    if( solo ) this->stop();

    // synchronize
    m_mutex.acquire();
    src->reset();
    if( rew )
        // rewind it
        src->rewind();
    //else
        // reset it
        //src->reset(); // redundant
    // add to vector
    t_CKBOOL got = FALSE;
    for( t_CKINT i = 0; i < m_src.size(); i++ )
        if( m_src[i] == src ) { got = TRUE; break; }
    if( !got )
        m_src.push_back( src );
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
    // clear src2
    m_src2.clear();
    // copy
    for( t_CKUINT i = 0; i < m_src.size(); i++ )
    {
        // match
        if( m_src[i] != src )
            m_src2.push_back( m_src[i] );
    }
    // copy
    m_src = m_src2;
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
AudioBusParallel::AudioBusParallel( t_CKUINT num_frames, t_CKUINT sig_frames )
    : AudioBus( num_frames )
{
    sf = NULL;
    m_sig_frames = sig_frames;
    // METHOD A: start thread
    // METHOD A: m_thread.start( the_parallel, this );
}




bool AudioBusParallel::open( const string & filename )
{
    if( sf )
        close();

    m_sf_mutex.acquire();
    memset( &sf_info, 0, sizeof(sf_info) );
    sf_info.samplerate = BirdBrain::srate();
    sf_info.channels = 2;
    sf_info.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;

    sf = sf_open( filename.c_str(), SFM_WRITE, &sf_info );
    m_sf_mutex.release();

    return sf != NULL;
}




void AudioBusParallel::close()
{
    m_sf_mutex.acquire();
    if( sf )
        sf_close( sf );
    sf = NULL;
    m_sf_mutex.release();
}




//-----------------------------------------------------------------------------
// name: stick()
// desc: ...
//-----------------------------------------------------------------------------
t_CKBOOL AudioBusParallel::stick( SAMPLE * buffer, t_CKUINT num_frames )
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
    t_CKUINT togo = m_num_frames;
    t_CKUINT num_frames = m_sig_frames;

    // loop
    while( togo > 0 )
    {
        // check
        if( num_frames > togo ) num_frames = togo;

        // tick sig
        AudioBus::stick( m_buffer + (m_num_frames-togo)*2, num_frames );
        // m_buffer[(m_num_frames-togo)*2] += .5f;

        // keep track of frames to go
        togo -= num_frames;
    }

    // for performance reasons (we are not stupid)
    if( sf )
    {
        m_sf_mutex.acquire();
        // in METHOD B, this may cause more audio clicks (maybe)
        if( sf ) sf_writef_float( sf, m_buffer, m_num_frames );
        m_sf_mutex.release();
    }
}
