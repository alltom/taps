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
// name: ui_search.h
// desc: birdbrain ui
//
// authors: Ananya Misra (amisra@cs.princeton.edu)
//          Ge Wang (gewang@cs.princeton.edu)
//          Matt Hoffman (mdhoffma@cs.princeton.edu)
//          Perry R. Cook (prc@cs.princeton.edu)
// date: Autumn 2004
//-----------------------------------------------------------------------------
#ifndef __UI_SEARCH_H__
#define __UI_SEARCH_H__

//#include "birdbrain.h"
//#include "audicle_def.h"
#include "ui_audio.h"
#include "ui_library.h"
#include "audicle_face.h"
#include "audicle_gfx.h"

// feature database
#include "taps_featurelibrary.h"

// forward reference
struct UI_Element;
struct UI_Template;
struct Template;
struct Timeline;



//-----------------------------------------------------------------------------
// name: class UISearch
// desc: ...
//-----------------------------------------------------------------------------
class UISearch : public AudicleFace
{
public:
    UISearch( );
    virtual ~UISearch( );

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

public:
    void render_library_pane();
    void render_matches_pane();
    void render_values_pane();
    void render_weights_pane();
    void play_template( UI_Template * playme );
    t_TAPBOOL check_library_event( InputEvent * ie, Library * lib, t_TAPBOOL &released );
    t_TAPBOOL handle_keyboard_event( char key );
    t_TAPBOOL handle_motion_event( InputEvent * ie );
    t_TAPBOOL load_features_from_db( Template * temp, int index ); 
    t_TAPBOOL extract_features( Template * temp );
    t_TAPBOOL load_match( int index );

protected:
    t_TAPBOOL m_init;
    AudioCentral * m_audio;
    //UI_Element ** ui_elements;

    UI_Template * selected;
    bool selected_from_matches; // selected from matches or library? (if no selected, indifferent)
    std::string filename;

    Point3D curr_pt;
    Point3D prev_pt; 
    Point3D orig_pt;
    Color4D linecol;
    Color4D highlight;

    UI_Template * down;

    // database
    FeatureLibrary * db;
    int * db_indices;
    float * db_distances;
    std::string db_path;
    std::string db_filename;

    // fog
    GLuint fog_mode[4]; // Storage For Three/Four Types Of Fog
    GLuint fog_filter;  // Which Fog To Use
    GLfloat fog_density;  // Fog Density

    // bounding boxes
    BBox * m_bboxs;
    int m_nbboxs;
    int m_cur_bbox;
    bool m_highlight;

    // view region indices/ids
    int m_vr_library_id;
    int m_vr_matches_id;
    int m_vr_values_id;
    int m_vr_weights_id;

    // last hit position (for knowing what to delete from if there are both timelines and bags)
    Point2D last_hit_pos; 

    // load sliders from selected?
    bool load_sliders;
    // view in list mode versus 3d
    bool m_list_view;
    // "center" on selected template
    bool m_center;
    // center coordinates. there must be a better way to do this.
    float x_center, y_center, z_center;
    bool xyz_center_init;
};


// pointer
//extern UISearch * g_search_face;

// helping along : map given value in [val_low, val_high] to [low, high], return offset from low
float offset_in_range( float value, float val_low, float val_high, float low, float high); 

#endif
