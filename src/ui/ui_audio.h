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
#ifndef __UI_AUDIO_H__
#define __UI_AUDIO_H__

#include "birdbrain.h"
#include "audicle_def.h"
#include "rtaudio.h"
#include "util_thread.h"
#include "ui_audiofx.h"

// libsndfile
#ifndef __USE_SNDFILE_PRECONF__
#include <sndfile.h>
#else
#include "util_sndfile.h"
#endif


// forward reference
class TreesynthIO;


//-----------------------------------------------------------------------------
// name: AudioSrc
// desc: ...
//-----------------------------------------------------------------------------
struct AudioSrc
{
    AudioSrc() { m_done = TRUE; m_stop_asap = FALSE; on = NULL; m_delete = TRUE; 
                 m_gain = 1.0f; m_pan = .5f; }
    virtual ~AudioSrc() { m_done = TRUE; if( on ) *on = FALSE; }
    virtual t_CKBOOL stick( SAMPLE * buffer, t_CKUINT num_frames ) = 0;
    virtual void reset() { m_done = FALSE; m_stop_asap = FALSE; }
    virtual void rewind() { }
    virtual t_CKBOOL done() { return m_done; }
    virtual t_CKBOOL playing() { return !m_done; }
    virtual const std::string & name() { return m_name; }
    std::string m_name;

    virtual void set_gain( t_CKSINGLE gain )
    { if( gain<0 ) gain = 0; if( gain>128 ) gain = 128; m_gain = gain; }
    virtual t_CKSINGLE get_gain() const { return m_gain; }
    virtual void set_pan( t_CKSINGLE pan )
    { if( pan<0 ) pan = 0; if( pan>1 ) pan = 1; m_pan = pan; }
    virtual t_CKSINGLE get_pan() const { return m_pan; }
    
    t_CKBOOL * on; 
    t_CKBOOL m_done; // whether it's done (safe to delete) (not in use) (...)
    t_CKBOOL m_stop_asap; // (ignore: whether it's not done) (the real comment: whether to stop asap ) 
    t_CKBOOL m_delete;
    SAMPLE m_gain;
    SAMPLE m_pan;

    static void mono2stereo( SAMPLE * self, t_CKUINT frames );
    static void stereo2mono( SAMPLE * self, t_CKUINT frames );
    static void mono2stereo( const SAMPLE * mono, t_CKUINT frames, SAMPLE * stereo );
    static void stereo2mono( const SAMPLE * stereo, t_CKUINT frames, SAMPLE * mono );
    static void pan_buf( SAMPLE * stereo, t_CKUINT frames, t_CKSINGLE where );
    static void gain_buf( SAMPLE * buffer, t_CKUINT num_samples, SAMPLE gain );
};




//-----------------------------------------------------------------------------
// name: AudioSrcTest
// desc: ...
//-----------------------------------------------------------------------------
struct AudioSrcTest : public AudioSrc
{
    AudioSrcTest( t_CKFLOAT freq ) : AudioSrc() { m_freq = freq; m_t = 0.0; }
    virtual t_CKBOOL stick( SAMPLE * buffer, t_CKUINT num_frames );

protected:
    t_CKFLOAT m_freq;
    t_CKFLOAT m_t;
};




//-----------------------------------------------------------------------------
// name: AudioSrcFile
// desc: ...
//-----------------------------------------------------------------------------
struct AudioSrcFile : public AudioSrc
{
    AudioSrcFile( );
    virtual ~AudioSrcFile();

    virtual t_CKBOOL mtick( SAMPLE * buffer, t_CKUINT num_samples );
    virtual t_CKBOOL stick( SAMPLE * buffer, t_CKUINT num_frames );
    virtual void rewind() { AudioSrc::rewind(); sf_seek( sf, 0, SEEK_SET ); }
    t_CKBOOL open( const char * filename, t_CKUINT start = 0, t_CKUINT len = 0, t_CKBOOL scale = FALSE );
    t_CKBOOL close();
    t_CKBOOL seek( t_CKUINT where, t_CKUINT how )
    { return sf_seek( sf, where, how ) < sf_info.frames; }
    const std::string & filename() { return m_filename; }
    const SF_INFO & info() { return sf_info; }
    t_CKUINT start() { return m_start; }
    t_CKUINT end() { return m_end; }
    t_CKUINT now() { return sf_seek( sf, 0, SEEK_CUR ); }

    t_CKSINGLE * slide;
    bool * slide_locally;

protected:
    SAMPLE * m_buffer;
    t_CKUINT m_buffer_size;
    SNDFILE * sf;
    SF_INFO sf_info;
    t_CKUINT m_start;
    t_CKUINT m_end;
    std::string m_filename;
    t_CKSINGLE signal_scale;
};




//-----------------------------------------------------------------------------
// name: AudioSrcFrame
// desc: ...
//-----------------------------------------------------------------------------
struct AudioSrcFrame : public AudioSrc
{
    AudioSrcFrame( Frame * f ) : AudioSrc() { frame = f; pos = 0; slide = NULL; slide_locally = NULL; }
    virtual t_CKBOOL mtick( SAMPLE * buffer, t_CKUINT num_samples ); 
    virtual t_CKBOOL stick( SAMPLE * buffer, t_CKUINT num_frames );
    virtual void rewind() { AudioSrc::rewind(); pos = 0; }
	t_CKBOOL seek( t_CKUINT where, t_CKUINT how );
    int num_samples();

    t_CKSINGLE * slide;
    bool * slide_locally;

protected:
    Frame * frame;
    t_CKUINT pos;
};




//-----------------------------------------------------------------------------
// name: AudioSrcEliot
// desc: ...
//-----------------------------------------------------------------------------
struct AudioSrcEliot : public AudioSrc
{
    AudioSrcEliot( TreesynthIO * io ) : AudioSrc() { ts_io = io; }
    virtual t_CKBOOL stick( SAMPLE * buffer, t_CKUINT num_frames );
    void stop() { m_stop_asap = TRUE; }

protected:
    TreesynthIO * ts_io;
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
    virtual void set( t_CKUINT which, t_CKFLOAT T60 );
    virtual void mix( t_CKFLOAT value );
    t_CKUINT getset() const { return m_state; }
    t_CKFLOAT getmix() const { return m_mix; }

    virtual t_CKBOOL stick( SAMPLE * buffer, t_CKUINT num_frames );

protected:
    t_CKFLOAT m_mix;
    t_CKUINT m_state;
    FxReverb * m_left;
    FxReverb * m_right;
};




//-----------------------------------------------------------------------------
// name: AudioBus
// desc: ...
//-----------------------------------------------------------------------------
struct AudioBus : public AudioSrc
{
    AudioBus( t_CKUINT num_frames ) : AudioSrc() { this->init( num_frames ); }
    virtual ~AudioBus();

    virtual t_CKBOOL stick( SAMPLE * buffer, t_CKUINT num_frames );

    virtual void play( AudioSrc * src, t_CKBOOL solo = FALSE, t_CKBOOL rew = TRUE );
    virtual void remove( AudioSrc * src );
    virtual void stop();

    virtual t_CKUINT num_src() const { return m_src.size(); }

    AudioFxReverb * reverb() { return &m_reverb; }
    AudioFxReverb m_reverb;

    t_CKBOOL m_ready;
    std::vector<AudioSrc *> m_src;
    std::vector<AudioSrc *> m_src2;
    SAMPLE * m_buffer;
    SAMPLE * m_buffer2;
    t_CKUINT m_num_frames;
    XMutex m_mutex;

private:
    t_CKBOOL init( t_CKUINT buffer_size );
};




//-----------------------------------------------------------------------------
// name: AudioBusParallel
// desc: ...
//-----------------------------------------------------------------------------
struct AudioBusParallel : public AudioBus
{
    AudioBusParallel( t_CKUINT num_frames, t_CKUINT sig_frames );
    virtual ~AudioBusParallel() { close(); }
    virtual t_CKBOOL stick( SAMPLE * buffer, t_CKUINT num_frames );
    void accumulate();

    bool open( const std::string & filename );
    void close();
    SNDFILE * sf;
    SF_INFO sf_info;
    t_CKUINT m_sig_frames;

protected:
    XThread m_thread;
    XMutex m_sf_mutex;
};




//-----------------------------------------------------------------------------
// name: AudioCentral
// desc: ...
//-----------------------------------------------------------------------------
struct AudioCentral
{
    virtual ~AudioCentral() { if( m_audio ) this->free(); }
    static AudioCentral * instance();

    t_CKBOOL init( t_CKUINT srate, t_CKUINT num_frames,
                   t_CKUINT sig_frames, t_CKUINT bus = 8 );
    t_CKBOOL free();

    virtual t_CKBOOL stop_all();
    AudioBus * bus( t_CKUINT index );
    virtual t_CKUINT num_bus() const { return m_bus->m_src.size(); }

    virtual void set_gain( t_CKSINGLE gain ) { m_bus->set_gain( gain ); }
    virtual t_CKSINGLE get_gain() const { return m_bus->get_gain(); }

    t_CKBOOL record_start( const std::string & filename )
    { return m_bus->open( filename ); }
    t_CKBOOL record_stop()
    { m_bus->close(); return TRUE; }

public:
    SAMPLE * m_buffer;
    SAMPLE * m_buffer2;
    t_CKUINT m_num_frames;
    t_CKUINT m_sig_frames;
    t_CKUINT m_channels;
    t_CKUINT m_srate;
    RtAudio * m_audio;
    XMutex m_mutex;
    AudioBusParallel * m_bus;

protected:
    static AudioCentral * our_instance;
    AudioCentral() { m_buffer = NULL; m_buffer2 = NULL; m_num_frames = 0;
                     m_srate = 0; m_audio = NULL; m_bus = NULL; m_channels = 2; }
};




#endif
