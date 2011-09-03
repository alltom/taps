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

#include "ui_audio.h"
#include "taps_birdbrain.h"
#include "audicle_def.h"
#include "audicle_face.h"
#include "audicle_gfx.h"


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
    virtual t_TAPBOOL init();
    virtual t_TAPBOOL destroy();

public:
    virtual void render_pre( );
    virtual void render_post( );
    virtual t_TAPUINT render( void * data );
    virtual void render_view( );
    virtual t_TAPUINT on_activate( );
    virtual t_TAPUINT on_deactivate( t_TAPDUR duration = 0.0 );
    virtual t_TAPUINT on_event( const AudicleEvent & event );
    bool load_sliders;

protected:
    t_TAPBOOL m_init;
    AudioCentral * m_audio;
};


// global
extern UIControl * g_control_face;



#endif
