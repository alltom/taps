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
// name: ui_synthesis.h
// desc: birdbrain ui
//
// authors: Ananya Misra (amisra@cs.princeton.edu)
//          Ge Wang (gewang@cs.princeton.edu)
//          Perry R. Cook (prc@cs.princeton.edu)
//          Philip Davidson (philipd@cs.princeton.edu)
// date: Autumn 2004
//-----------------------------------------------------------------------------
#ifndef __UI_SYNTHESIS_H__
#define __UI_SYNTHESIS_H__

#include "ui_audio.h"
#include "taps_birdbrain.h"
#include "audicle_def.h"
#include "audicle_face.h"
#include "audicle_gfx.h"
#include "util_readwrite.h"

// forward reference
struct UI_Element;
struct UI_Template;
struct Template;
struct Timeline;


/*class InnerFace
{
    float x_pos;
    float y_pos;
    float width;
    float height;
    std::vector<UI_Element *> ui_elements;

    virtual ~InnerFace() { }
    virtual void draw() = 0;
    virtual float offset( const UI_Element * el, int which );
};


class UI_Library : public InnerFace
{
    UI_Library( float x, float y, float w, float h );
    virtual ~UI_Library();

    void add( Template * temp );
    virtual void draw();
    virtual void handle_down( t_TAPUINT id );
    virtual Template * handle_up( t_TAPUINT id );
};*/



//-----------------------------------------------------------------------------
// name: class UISynthesis
// desc: ...
//-----------------------------------------------------------------------------
class UISynthesis : public AudicleFace
{
public:
    UISynthesis( );
    virtual ~UISynthesis( );

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
    //UI_Element ** ui_elements;

    UI_Template * selected;
    Timeline * timeline;
    std::string filename;

    void render_deterministic_pane();
    void render_transient_pane();
    void render_residue_pane();
    void render_loop_pane();
    void render_timeline_pane();
    void render_bag_pane();
    void render_the_usual();
    void toggle_rand_loop();
    t_TAPTIME get_duration();
    void set_duration( t_TAPTIME samples );
    t_TAPTIME get_tick_unit(); 
    void set_timeline_bounds();
    
    void play_template( UI_Template * playme ); 
    void make_bagels( UI_Template * bag ); 

	void check_bagel_event( InputEvent * ie, t_TAPBOOL & somewhere, t_TAPBOOL & hit );

#ifdef __TAPS_SCRIPTING_ENABLE__
	void render_script_pane();
	void check_chui_event( InputEvent * ie, t_TAPBOOL & somewhere, t_TAPBOOL & hit );
#endif

    Point3D curr_pt;
    Point3D prev_pt; 
    Point3D orig_pt;
    Color4D linecol;
    Color4D highlight;

    UI_Template * down;

    // bounding boxes
    BBox * m_bboxs;
    int m_nbboxs;
    int m_cur_bbox;
    bool m_highlight;

    // extra ui elements for box / bag template 
    // (hacked since the number we may need changes while running)
    std::vector<UI_Element *> bagels; 

    // last hit position (for knowing what to delete from if there are both timelines and bags)
    Point2D last_hit_pos; 
};


// pointer
extern UISynthesis * g_synth_face;
// load file
Template * synth_load_file( const std::string & path );
// load file into library by name
t_TAPBOOL synth_load_template( const std::string & name, t_TAPBOOL show = TRUE );
// copy template in library
t_TAPBOOL synth_copy_template( UI_Template * ui_temp, t_TAPINT copies );


#endif
