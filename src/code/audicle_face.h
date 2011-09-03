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
// name: audicle_face.h
// desc: interface for audicle face
//
// authors: Ge Wang (gewang@cs.princeton.edu)
//          Perry R. Cook (prc@cs.princeton.edu)
//          Philip Davidson (philipd@cs.princeton.edu)
//          Ananya Misra (amisra@cs.princeton.edu)
// date: 2/16/2004
//-----------------------------------------------------------------------------
#ifndef __AUDICLE_FACE_H__
#define __AUDICLE_FACE_H__

#include "audicle_def.h"
#include "audicle_event.h"
#include "audicle_gfx.h"
// using namespace std;

// forward references
class Audicle;
struct UI_Element;

//-----------------------------------------------------------------------------
// name: class AudicleFace
// desc: interface
//-----------------------------------------------------------------------------
class AudicleFace
{
public:
    AudicleFace( );
    virtual ~AudicleFace( );
    
    virtual t_TAPBOOL init();
    virtual t_TAPBOOL destroy();

public: // id and state
    t_TAPUINT id( ) const { return m_id; }
    t_TAPUINT state( ) const { return m_state; }
    const std::string & name( ) const { return m_name; }
    void set_id( t_TAPUINT id ) { m_id = id; }
    void set_name( const std::string & name ) { m_name = name; }

public: // state
    enum { INACTIVE, ACTIVATING, ACTIVE, DEACTIVATING };

public: // rendering functions
    virtual void render_pre( );
    virtual t_TAPUINT render( void * data );
    virtual void render_post( );
    virtual void render_view( ) {}

public: // face switching & control
    virtual void draw_face_buttons( int button_from, int button_to );
    virtual void handle_face_button( int button_from, int button_to, int button_hit );
	virtual void draw_ctrl_buttons( int button_quit, int button_fullscreen );
	virtual void handle_ctrl_button( int button_quit, int button_fullscreen, int button_hit );

public: // coordinate
    Point2D ndc_to_world( const Point2D & pt );
    virtual double ndc_to_world_x( double x );
    virtual double ndc_to_world_y( double y );

public: // audicle signals
    virtual t_TAPUINT on_activate( );
    virtual t_TAPUINT on_deactivate( t_TAPDUR duration = 0.0 );
    virtual t_TAPUINT on_event( const AudicleEvent & event );

public:
    Color4D m_bg;
    Color4D m_bg2;
    t_TAPFLOAT m_bg_alpha;
    t_TAPFLOAT m_bg_speed;
    t_TAPUINT sleep_time;

protected:
    virtual void xfade_bg( );

protected:
    t_TAPUINT m_id;
    t_TAPUINT m_state;
    std::string m_name;

protected:
    // light 0 position
    t_TAPSINGLE m_light0_pos[4];

    // light 1 parameters
    t_TAPSINGLE m_light1_ambient[4];
    t_TAPSINGLE m_light1_diffuse[4];
    t_TAPSINGLE m_light1_specular[4];
    t_TAPSINGLE m_light1_pos[4];

    // modelview stuff
    t_TAPSINGLE m_angle_y;
    t_TAPSINGLE m_eye_y;

protected:
    // everything (default)
    static ViewRegionManual default_vr;

    // view regions
    ViewRegionInterp m_vr;
    std::vector<ViewRegion *> m_vrs;
    // hack
    bool m_first;

    // ui elemnts
    UI_Element ** ui_elements;
    
    // face button initialized?
    bool m_face_button_init;
	// ctrl buttons initialized?
	bool m_ctrl_button_init;
};


class Shred_Stat;


//-----------------------------------------------------------------------------
// name: struct Shred_Data
// desc: ...
//-----------------------------------------------------------------------------
struct Shred_Data
{

    Shred_Stat * stat;
    t_TAPBOOL in_shredder_map; //XXX HAAAAACK!  
    Color4D color;
    Point3D pos;
    Point3D pos2;
    Point3D vel;
    Point3D acc;
    t_TAPFLOAT radius;
    t_TAPFLOAT radius2;
    t_TAPUINT name;
    t_TAPFLOAT x;
    
    t_TAPBOOL mouse_down;
    t_TAPBOOL mouse_clicked;
    
    t_TAPUINT ref_count;

    Shred_Data()
    {
        stat = NULL;
        in_shredder_map = false;
        radius = .2;
        name = 0;
        ref_count = 1;
        x = 0;
    }
};



/*
//-----------------------------------------------------------------------------
// name: struct Shred_Time
// desc: ...
//-----------------------------------------------------------------------------
struct Shred_Time
{
    vector < ShredActivation > history; // hey memory... fuck you!
    bool active;
    bool switching;
    t_TAPUINT group_cycles;
    t_TAPTIME switch_time;
    t_TAPDUR  switch_span;
};
*/



#endif
