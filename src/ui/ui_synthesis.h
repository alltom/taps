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
    virtual void handle_down( t_CKUINT id );
    virtual Template * handle_up( t_CKUINT id );
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
    UI_Element ** ui_elements;

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
    TIME get_duration();
    void set_duration( TIME samples );
    TIME get_tick_unit(); 
    
    void play_template( UI_Template * playme ); 
    void make_bagels( UI_Template * bag ); 

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
t_CKBOOL synth_load_template( const std::string & name );
// copy template in library
t_CKBOOL synth_copy_template( UI_Template * ui_temp, t_CKINT copies );


#endif
