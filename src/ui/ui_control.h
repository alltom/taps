//-----------------------------------------------------------------------------
// name: ui_treesynth.h
// desc: control panel
//
// authors: Ananya Misra (amisra@cs.princeton.edu)
//          Ge Wang (gewang@cs.princeton.edu)
//          Perry R. Cook (prc@cs.princeton.edu)
// date: Spring 2006
//-----------------------------------------------------------------------------
#ifndef __UI_CONTROL_H__
#define __UI_CONTROL_H__

#include "birdbrain.h"
#include "audicle_def.h"
#include "audicle_face.h"
#include "audicle_gfx.h"
#include "ui_audio.h"


// forward reference
struct UI_Element;




//-----------------------------------------------------------------------------
// name: class UIControl
// desc: ...
//-----------------------------------------------------------------------------
class UIControl : public AudicleFace
{
public:
    UIControl( );
    virtual ~UIControl( );

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
    bool load_sliders;

protected:
    t_CKBOOL m_init;
    AudioCentral * m_audio;
};


// global
extern UIControl * g_control_face;



#endif
