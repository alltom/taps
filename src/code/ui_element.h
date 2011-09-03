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
// name: ui_element.h
// desc: birdbrain ui element
//
// authors: Ananya Misra (amisra@cs.princeton.edu)
//          Ge Wang (gewang@cs.princeton.edu)
//          Perry R. Cook (prc@cs.princeton.edu)
//          Philip Davidson (philipd@cs.princeton.edu)
// date: Winter 2004
//-----------------------------------------------------------------------------
#ifndef __UI_ELEMENTS_H__
#define __UI_ELEMENTS_H__


#include "audicle_geometry.h"
#include "audicle.h"
#include <string>
#include <deque>

#ifdef __TAPS_SCRIPTING_ENABLE__
#include "chuck_oo.h"
#endif


//-----------------------------------------------------------------------------
// name: struct UI_Element
// desc: ...
//-----------------------------------------------------------------------------
struct UI_Element
{
    t_TAPUINT id; // pick id
    t_TAPUINT down; // is it down?
    t_TAPSINGLE size_up; // size up
    t_TAPSINGLE size_down; // size down
    t_TAPSINGLE font_size; // font scale
    std::string name; // name
    Point3D * loc; // location ("absolute") 
	Point3D * rel_loc; // location within a view region
	t_TAPBOOL must_adjust_loc; // for chui, compute loc according to vr coordinates

    // slider level
    t_TAPSINGLE slide;
    t_TAPSINGLE slide_local;
    t_TAPSINGLE slide_last;
    t_TAPSINGLE slide_0;
    t_TAPSINGLE slide_1;
    bool slide_int;
    bool slide_locally;

    // starting point from where it is drawn? (redundant with loc, which is never set)
    // returned by draw_slider_*
    // x (for horizontal sliders) or y (for vertical sliders) offset
    t_TAPSINGLE offset;

    // on
    t_TAPBOOL on;
    t_TAPDOUBLE on_where;
    // local
    t_TAPSINGLE the_height;
    t_TAPSINGLE the_width;
    // ...
    t_TAPUINT id2;
    t_TAPSINGLE slide2;
    t_TAPSINGLE slide2_last;

	// ui element types
	enum {UI_SLIDER, UI_BUTTON, UI_FLIPPER, UI_BUTTER, UI_OTHER};
	// ui element (slider) lengths
	enum {UI_NORMAL, UI_MINI, UI_MICRO};
	// ui element (slider) orientation
	enum {UI_HORIZONTAL, UI_VERTICAL};
	// storing the related info (though usually unused)
	int element_type, element_length, element_orientation;

    // constructor
    UI_Element();
	// destructor
	~UI_Element() {};
    // value
    virtual t_TAPSINGLE fvalue() const;
	virtual t_TAPINT ivalue() const;
    // string value
    const std::string value() const;
	// set bounds
    virtual void set_bounds( t_TAPSINGLE s0, t_TAPSINGLE s1, bool integer = false );
    // set slide
    virtual void set_slide( t_TAPSINGLE val );
    virtual void set_slide2( t_TAPSINGLE val );
	// draw
	t_TAPSINGLE draw() const; // do not use under normal circumstances
	// adjust location, mapping to ViewRegion coordinates
	void adjust_loc( ViewRegion * vr );
#ifdef __TAPS_SCRIPTING_ENABLE__	
	// Chuck event
	Chuck_Event * event;
	// set event
	void set_event( Chuck_Event * e );
#endif
};

struct UI_Exp : public UI_Element
{
    double k, c, base;
    UI_Exp( double base_ );
    virtual t_TAPSINGLE fvalue() const;
    // set bounds
    virtual void set_bounds( t_TAPSINGLE s0, t_TAPSINGLE s1, bool integer = false );
    // set slide
    virtual void set_slide( t_TAPSINGLE val );
    virtual void set_slide2( t_TAPSINGLE val );
};

struct Template;

struct UI_Template : public UI_Element
{
    Template * backup;  // to restore to earlier version, synchronized with core only when something is selected
                        // timelines and bags have no backup due to complications in dealing with the uitemps they have
    Template * core; // the one used and modified all the time

    // dummies
    std::vector<UI_Template *> dummies;
    UI_Template * orig; // pointer to the "original" library ui template for dummies on a timeline or bag
    
	// visibility
	t_TAPBOOL show;

    UI_Template();
    ~UI_Template();

    // dummies
    void makedummy( UI_Template * dummy ); 
    void removedummy( UI_Template * dummy );
};

struct Track;

struct UI_Track : public UI_Element
{
    Track * core;
//	Track * backup; 
	std::deque<Track *> * backups;
};


// button image types
enum {
    IMG_NONE,
    IMG_PLAY,
    IMG_STOP,
    IMG_LOAD,
    IMG_SAVE,
    IMG_NEXT,
    IMG_PREV,
    IMG_ALL,
    IMG_SEP,
    IMG_COPY,
    IMG_MOD,
    IMG_REV,
    IMG_NEWT,
    IMG_TOG,
    IMG_UPARROW,
    IMG_CENTER,
    IMG_FIND,
	IMG_PLUS,
	IMG_RECORD,
    NUM_IMGS
};

//-----------------------------------------------------------------------------
// name: struct Spectre (moved from ui_analysis.cpp)
// desc: rgb
//-----------------------------------------------------------------------------
struct Spectre
{
    t_TAPSINGLE r;
    t_TAPSINGLE g;
    t_TAPSINGLE b;
};

// very global variables
extern const float g_slider_height;
extern const float g_slider_height_mini;
extern const float g_slider_height_micro;
extern const float g_butter_width;
extern const float g_butter_height;
extern double g_r;
extern float g_text_color[3];
extern bool g_show_slider_range; 
extern bool g_show_qz;

// use these
void draw_button(
    UI_Element & e,
    float x, float y, float z,
    float r, float g, float b,
    int img_type = IMG_NONE
);
t_TAPSINGLE draw_slider( const UI_Element & e, float x, float y, float z );
t_TAPSINGLE draw_slider_h( const UI_Element & e, float x, float y, float z );
void draw_knob( const UI_Element & e, float x, float y, float z );
t_TAPSINGLE draw_slider_mini( const UI_Element & e, float x, float y, float z );
t_TAPSINGLE draw_slider_h_mini( const UI_Element & e, float x, float y, float z );
t_TAPSINGLE draw_slider_h_micro( const UI_Element & e, float x, float y, float z ); 
t_TAPSINGLE draw_slider_range( const UI_Element & e, float x, float y, float z );
void draw_lr_butter(
    const UI_Element & left,
    UI_Element * now,
    const UI_Element & right,
    float x, float y, float z
);
void draw_flipper( const UI_Element & e, float x, float y, float z );
void draw_flipper2( const UI_Element & e, float x, float y, float z );
void draw_flipper_micro( const UI_Element & e, float x, float y, float z );
void draw_label( const std::string & text, float x, float y, float z, float font_size = 1.0f, bool center = false, float * c = NULL );
void draw_library( UI_Template * selected, Template * timeline = NULL, float x_start = 0.0f, float y_start = 0.0f, 
                  float x_max = 0.8f, float y_max = 1.0f, Spectre * color = NULL ); 
void draw_group_library( std::vector<bool>& selected, std::vector<UI_Template *>& all, float x_start = 0.0f, 
						 float y_start = 0.0f, float x_max = 0.8f, float y_max = 1.0f, Spectre * color = NULL ); 
void draw_template( float x, float y, UI_Template * ui_temp, bool draw_name = true, float y_offset = 0.0f, Spectre * color = NULL );
void draw_arrow( Point3D cur_pt, Point3D prev_pt, 
                 Point3D orig_pt, Color4D linecol,
                 Color4D highlight );
void draw_thing(float x, float y, float spin, UI_Element * ui_temp );
void msg_box( const char * title, const char * msg );

// fix slider -- set slide_last etc appropriately during motion
void fix_slider_motion( UI_Element & e, ViewRegion & vr, const Point2D & cur_pos, 
                       t_TAPSINGLE base_pt, t_TAPSINGLE height, t_TAPBOOL vertical );

// snap slider value to nearest int, if within bounds
void snap_slider_value( UI_Element & e );

#endif

