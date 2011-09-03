//-----------------------------------------------------------------------------
// name: ui_treesynth.h
// desc: birdbrain ui
//
// authors: Ananya Misra (amisra@cs.princeton.edu)
//          Ge Wang (gewang@cs.princeton.edu)
//          Perry R. Cook (prc@cs.princeton.edu)
//          Philip Davidson (philipd@cs.princeton.edu)
// date: Autumn 2004
//-----------------------------------------------------------------------------
#ifndef __UI_TREESYNTH_H__
#define __UI_TREESYNTH_H__

#include "birdbrain.h"
#include "audicle_def.h"
#include "audicle_face.h"
#include "audicle_gfx.h"
#include "ui_audio.h"

// libsndfile
#ifndef __USE_SNDFILE_PRECONF__
#include <sndfile.h>
#else
#include "util_sndfile.h"
#endif


// forward reference
struct UI_Element;




//-----------------------------------------------------------------------------
// name: class UITreesynth
// desc: ...
//-----------------------------------------------------------------------------
class UITreesynth : public AudicleFace
{
public:
    UITreesynth( );
    virtual ~UITreesynth( );

public:
    virtual t_CKBOOL init();
    virtual t_CKBOOL destroy();

public:
    virtual void render_pre( );
    virtual void render_post( );
    virtual t_CKUINT render( void * data );
    virtual void render_view( );
    virtual t_CKUINT on_activate( );
    virtual t_CKUINT on_deactivate( t_CKDUR duration = 0.0 );
    virtual t_CKUINT on_event( const AudicleEvent & event );

public:
    void set_view( int w, int h );
    void spectrogram( t_CKSINGLE x = 0.0f, t_CKSINGLE y = 0.0f );
    void spectroinit();

protected:
    t_CKBOOL m_init;
    AudioCentral * m_audio;
};




#endif
