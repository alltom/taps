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




//-----------------------------------------------------------------------------
// name: struct UI_Element
// desc: ...
//-----------------------------------------------------------------------------
struct UI_Element
{
    // pick id
    t_CKUINT id;
    // is it down?
    t_CKUINT down;
    // size up
    t_CKSINGLE size_up;
    // size down
    t_CKSINGLE size_down;
    // font scale
    t_CKSINGLE font_size;
    // name
    std::string name;
    // location
    Point2D * loc;

    // slider level
    t_CKSINGLE slide;
    t_CKSINGLE slide_local;
    t_CKSINGLE slide_last;
    t_CKSINGLE slide_0;
    t_CKSINGLE slide_1;
    bool slide_int;
    bool slide_locally;

    // starting point from where it is drawn? 
	// (not used in any consistent way now, but returned by draw_slider_*)
	// x or y offset
	t_CKSINGLE offset;

	// on
    t_CKBOOL on;
    t_CKDOUBLE on_where;

    // local
    t_CKSINGLE the_height;
    t_CKSINGLE the_width;

    // ...
    t_CKUINT id2;
    t_CKSINGLE slide2;
    t_CKSINGLE slide2_last;

    // constructor
    UI_Element()
    {
        id = 0;
        down = FALSE;
        size_up = -1.0f;
        size_down = -1.0f;
        font_size = 1.0f;
        loc = NULL;

        slide = 0.0f;
        set_bounds( 0.0f, 1.0f );
        slide_int = false;
        slide_locally = false;

        on = FALSE;
        on_where = 0.0;

        the_height = -1.0f;
        the_width = -1.0f;

        id2 = 0;
        slide2 = 0.0f;
        slide2_last = 0.0f;

		offset = 0.0f;
    }

    // value
    virtual t_CKSINGLE fvalue() const
    {
        return slide * (slide_1 - slide_0) + slide_0;
    }

    // value
    virtual t_CKSINGLE fvalue2() const
    {
        return slide2 * (slide_1 - slide_0) + slide_0;
    }

    // set bounds
    virtual void set_bounds( t_CKSINGLE s0, t_CKSINGLE s1, bool integer = false )
    {
        slide_0 = s0;
        slide_1 = s1;
        slide_int = integer;
    }

    // set slide
    virtual void set_slide( t_CKSINGLE val )
    {
        slide = (val - slide_0) / (slide_1 - slide_0);
    }

    // set slide
    virtual void set_slide2( t_CKSINGLE val )
    {
        slide2 = (val - slide_0) / (slide_1 - slide_0);
    }

    // value
    const std::string value() const
    {
        char buffer[256];
        std::string val;

        t_CKSINGLE v = fvalue();
        if( slide_int )
            sprintf( buffer, "%i", (int)v );
        else
            sprintf( buffer, "%.3f", v );
        val = buffer;

        return val;
    }

    const std::string value2() const
    {
        char buffer[256];
        std::string val;

        t_CKSINGLE v = fvalue();
        t_CKSINGLE v2 = fvalue2();
        if( v == v2 ) return value();
        if( slide_int )
            sprintf( buffer, "%i - %i", (int)v, (int)v2 );
        else
            sprintf( buffer, "%.2f - %.2f", v, v2 );
        val = buffer;

        return val;
    }
};

struct UI_Exp : public UI_Element
{
    double k, c, base;

    UI_Exp( double base_ ) : UI_Element() { base = base_; set_bounds( 0.0f, 1.0f ); }

    virtual t_CKSINGLE fvalue() const
    {
        return k * ::pow(base, slide) + c;
    }

    virtual t_CKSINGLE fvalue2() const
    {
        return k * ::pow(base, slide) + c;
    }

    // set bounds
    virtual void set_bounds( t_CKSINGLE s0, t_CKSINGLE s1, bool integer = false )
    {
        slide_0 = s0;
        slide_1 = s1;
        k = (slide_1 - slide_0) / (base-1);
        c = slide_0 - k;
        slide_int = integer;
    }

    // set slide
    virtual void set_slide( t_CKSINGLE val )
    {
        slide = ::log10((val-c)/k) / ::log10(base);
    }

    virtual void set_slide2( t_CKSINGLE val )
    {
        slide2 = ::log10((val-c)/k) / ::log10(base);
    }
};

struct Template;

struct UI_Template : public UI_Element
{
    Template * backup;  // to restore to earlier version, synchronized with core only when something is selected
                        // timelines and bags have no backup due to complications in dealing with the uitemps they have
    Template * core; // the one used and modified all the time

    // danger
    std::vector<UI_Template *> dummies;
    UI_Template * orig; // pointer to the "original" library ui template for dummies on a timeline or bag
    
    UI_Template();

    // danger
    void makedummy( UI_Template * dummy ); 
    void removedummy( UI_Template * dummy );
    ~UI_Template();
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
    NUM_IMGS
};

//-----------------------------------------------------------------------------
// name: struct Spectre (moved from ui_analysis.cpp)
// desc: rgb
//-----------------------------------------------------------------------------
struct Spectre
{
    t_CKSINGLE r;
    t_CKSINGLE g;
    t_CKSINGLE b;
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
t_CKSINGLE draw_slider( const UI_Element & e, float x, float y, float z );
t_CKSINGLE draw_slider_h( const UI_Element & e, float x, float y, float z );
void draw_knob( const UI_Element & e, float x, float y, float z );
t_CKSINGLE draw_slider_mini( const UI_Element & e, float x, float y, float z );
t_CKSINGLE draw_slider_h_mini( const UI_Element & e, float x, float y, float z );
t_CKSINGLE draw_slider_h_micro( const UI_Element & e, float x, float y, float z ); 
t_CKSINGLE draw_slider_range( const UI_Element & e, float x, float y, float z );
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
void draw_template( float x, float y, UI_Template * ui_temp, bool draw_name = true, float y_offset = 0.0f, Spectre * color = NULL );
void draw_arrow( Point3D cur_pt, Point3D prev_pt, 
                 Point3D orig_pt, Color4D linecol,
                 Color4D highlight );
void msg_box( const char * title, const char * msg );

// fix slider -- set slide_last etc appropriately during motion
void fix_slider_motion( UI_Element & e, ViewRegion & vr, const Point2D & cur_pos, 
					   float base_pt, float height, bool vertical );

#endif