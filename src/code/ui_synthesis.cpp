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
// name: ui_synthesis.cpp
// desc: birdbrain ui
//
// authors: Ananya Misra (amisra@cs.princeton.edu)
//          Ge Wang (gewang@cs.princeton.edu)
//          Perry R. Cook (prc@cs.princeton.edu)
//          Philip Davidson (philipd@cs.princeton.edu)
// EST. January 14, 2005, 8:37 p.m. Friday
//-----------------------------------------------------------------------------
#include "ui_audio.h"
#include "ui_synthesis.h"
#ifdef __TAPS_SCRIPTING_ENABLE__
#include "ui_scripting.h"
#endif
#include "ui_element.h"
#include "ui_library.h"
#include "ui_filesave.h"
#include "taps_birdbrain.h"
#include "taps_sceptre.h"
#include "audicle.h"
#include "audicle_utils.h"
#include "audicle_gfx.h"
#include "audicle_geometry.h"

void clear_timeline_dummies( Timeline * tim ); // before deleting the timeline
void create_timeline_dummies( Timeline * tim ); // for when a timeline is copied
void clear_bag_dummies( BagTemplate * bag ); // before deleting a bag
void create_bag_dummies( BagTemplate * bag ); // for when a bag is copied
void clear_template_dummies( Template * temp );  // before deleting a timeline or bag
void create_template_dummies( Template * temp );  // for when a timeline or bag is copied
int g_tl_count = 1;
UISynthesis * g_synth_face = NULL;

// enumeration for ui elements
enum UI_ELMENTS
{
    BT_PLACE = 0,
    BT_LISTEN,
    BT_STOP,
    BT_REVERT,
    BT_COPY,
    BT_NEW_TIMELINE,
    BT_LEFT,  // butter
    BT_NOW,   // butter
    BT_RIGHT, // margarine
    // treesynth buttons
    BT_SAVE_FILE,
    BT_LOAD_ANALYSIS,
    BT_LOAD_FILE,
	// change residue method
	BT_CHANGE_RES_METHOD,
    // face switchers
    BT_ANALYSIS_FACE,
    BT_SYNTHESIS_FACE,
    BT_GROUP_FACE,
	BT_CONTROL_FACE,
    BT_SEARCH_FACE,
	// control
	BT_QUIT,
	BT_FULLSCREEN,
    
    FL_MS,
    FL_SECOND,
    FL_MINUTE,
    FL_HOUR,
    FL_DAY,
    FL_WEEK,
    FL_LOOP_ME,
    FL_BAGEL_CONTROLS,
    FL_BUS_WRITE_STEREO,
	FL_BUS_WRITE_MULTI,
	FL_BUS_WRITE_BOTH,
    // treesynth flips
    FL_ANCFIRST,
    FL_RANDFLIP,
    FL_TS_WRITE_FILE,
	// olar flips
	FL_SCALEAMP,
	// general sliders
    SL_PERIODICITY,
    SL_DENSITY,
    SL_GAIN,
    SL_FREQ_WARP,
    SL_TIME_STRETCH,
    // treesynth sliders
    SL_PERCENTAGE,
    SL_K,
    SL_TOTAL_LEVELS,
    SL_STARTLEVEL,
    SL_STOPLEVEL,
    // olar sliders
	SL_RAND_OLAR,
	SL_MINDIST,
	SL_SEGSIZE,
    // horizontals
	SL_PAN,
    SL_RAND_LOOP,
	// timeline sliders
    SL_TIMELINE_SHIFT,
    SL_TIMELINE_ZOOM,
    SL_TIMELINE_DURATION,

    // the timeline
    TL_TIMELINE,
    
    // keep this as last
    NUM_UI_ELEMENTS
};


// names of ui elements
static char * ui_str[] = { 
    "place",
    "listen",
    "stop",
    "revert",
    "copy",
    "new",
    "left",
    "now",
    "right",
    "save",
    "load from analysis",
    "load",
	"change method",
    "analysis",
    "synthesis",
    "group",
    "control",
    "search",
	"exit",
	"fullscreen",
    "ms",
    "second",
    "minute",
    "hour",
    "day",
    "week",
    "loop me",
    "see LOCAL controls",
    "write2",
	"write8",
	"both",
    "order",   // misleading because order is not flipped when this is true
    "++random",
    "write to file (ts)",
	"random amplitude",
    "periodicity",
    "density",
    "gain",
    "freq-warp",
    "time-stretch",
    "randomness",
    "similarity",
    "total-levels",
    "start-level",
    "stop-level",
    "randomness", // olar
	"min distance", // olar
	"segment size", // olar
    "pan",
    "randomness",
	"shift",
    "zoom",
    "duration",
    "timeline - but noone should be using this"
};


/*
//-----------------------------------------------------------------------------
// name: offset()
// desc: find global offset for element
//-----------------------------------------------------------------------------
float InnerFace::offset( const UI_Element * el, int which )
{
    // x
    if( which == 0 )
        return x_pos + ( el->loc ? (*el->loc)[0] : 0.0f );
    // y
    if( which == 1 )
        return y_pos + ( el->loc ? (*el->loc)[1] : 0.0f );
    // z
    assert( FALSE );

    return 0.0f;
}
*/



//-----------------------------------------------------------------------------
// name: UISynthesis()
// desc: ...
//-----------------------------------------------------------------------------
UISynthesis::UISynthesis( ) : AudicleFace( )
{ 
    if( !this->init( ) )
    {
        BB_log( BB_LOG_SEVERE, "[audicle]: cannot start face...\n" );
        return;
    }
}


//-----------------------------------------------------------------------------
// name: ~UISynthesis()
// desc: ...
//-----------------------------------------------------------------------------
UISynthesis::~UISynthesis( ) 
{ 
	int i;
    // delete ui_elements
    for( i = 0; i < NUM_UI_ELEMENTS; i++ )
    {
        SAFE_DELETE( ui_elements[i] );
    }
    SAFE_DELETE_ARRAY( ui_elements );
    // delete bagels
	for( i = 0; i < bagels.size(); i++ )
	{
		SAFE_DELETE( bagels[i] );
	}
	bagels.clear();
    // delete library?
    // anything else?
}


//-----------------------------------------------------------------------------
// name: init()
// desc: ...
//-----------------------------------------------------------------------------
t_TAPBOOL UISynthesis::init( )
{
    if( !AudicleFace::init() )
        return FALSE;

    int i;

    // log
    BB_log( BB_LOG_SYSTEM, "initializing synthesis user interface..." );
    // push log
    BB_pushlog();

    // ui elements
    ui_elements = new UI_Element *[NUM_UI_ELEMENTS];
    memset( ui_elements, 0, sizeof(UI_Element *) * NUM_UI_ELEMENTS );

    // allocate
    ui_elements[BT_PLACE] = new UI_Element;
    ui_elements[BT_LISTEN] = new UI_Element;
    ui_elements[BT_REVERT] = new UI_Element;
    ui_elements[BT_COPY] = new UI_Element;
    ui_elements[BT_STOP] = new UI_Element;
    ui_elements[BT_NEW_TIMELINE] = new UI_Element;
    ui_elements[BT_LEFT] = new UI_Element;
    ui_elements[BT_NOW] = new UI_Element;
    ui_elements[BT_RIGHT] = new UI_Element;
    ui_elements[BT_SAVE_FILE] = new UI_Element;
    ui_elements[BT_LOAD_ANALYSIS] = new UI_Element;
    ui_elements[BT_LOAD_FILE] = new UI_Element;
	ui_elements[BT_CHANGE_RES_METHOD] = new UI_Element;
    ui_elements[BT_ANALYSIS_FACE] = new UI_Element;
    ui_elements[BT_SYNTHESIS_FACE] = new UI_Element;
    ui_elements[BT_CONTROL_FACE] = new UI_Element;
    ui_elements[BT_SEARCH_FACE] = new UI_Element;
    ui_elements[BT_GROUP_FACE] = new UI_Element;
	ui_elements[BT_QUIT] = new UI_Element;
	ui_elements[BT_FULLSCREEN] = new UI_Element;
    ui_elements[FL_MS] = new UI_Element;
    ui_elements[FL_SECOND] = new UI_Element;
    ui_elements[FL_MINUTE] = new UI_Element;
    ui_elements[FL_HOUR] = new UI_Element;
    ui_elements[FL_DAY] = new UI_Element;
    ui_elements[FL_WEEK] = new UI_Element;
    ui_elements[FL_LOOP_ME] = new UI_Element;
    ui_elements[FL_BAGEL_CONTROLS] = new UI_Element;
    ui_elements[FL_ANCFIRST] = new UI_Element;
    ui_elements[FL_RANDFLIP] = new UI_Element;
    ui_elements[FL_TS_WRITE_FILE] = new UI_Element;
	ui_elements[FL_SCALEAMP] = new UI_Element;
    ui_elements[FL_BUS_WRITE_STEREO] = new UI_Element;
    ui_elements[FL_BUS_WRITE_MULTI] = new UI_Element;
	ui_elements[FL_BUS_WRITE_BOTH] = new UI_Element;
	ui_elements[SL_PERIODICITY] = new UI_Element;
    ui_elements[SL_DENSITY] = new UI_Exp( 1000000 );
    ui_elements[SL_GAIN] = new UI_Exp( 100 );
    ui_elements[SL_FREQ_WARP] = new UI_Exp( 10000 );
    ui_elements[SL_TIME_STRETCH] = new UI_Exp( 10000 );
    ui_elements[SL_TIMELINE_SHIFT] = new UI_Element;
    ui_elements[SL_TIMELINE_ZOOM] = new UI_Element; 
    ui_elements[SL_TIMELINE_DURATION] = new UI_Exp( 10000 );
    ui_elements[SL_PAN] = new UI_Element;
    ui_elements[SL_RAND_LOOP] = new UI_Element;
    ui_elements[SL_PERCENTAGE] = new UI_Element;
    ui_elements[SL_K] = new UI_Element;
    ui_elements[SL_TOTAL_LEVELS] = new UI_Element;
    ui_elements[SL_STARTLEVEL] = new UI_Element;
    ui_elements[SL_STOPLEVEL] = new UI_Element;
	ui_elements[SL_RAND_OLAR] = new UI_Element;
	ui_elements[SL_MINDIST] = new UI_Element;
	ui_elements[SL_SEGSIZE] = new UI_Element;
    ui_elements[TL_TIMELINE] = new UI_Element;

    // get id for each element
    for( i = 0; i < NUM_UI_ELEMENTS; i++ )
    {
        ui_elements[i]->id = IDManager::instance()->getPickID();
        ui_elements[i]->name = ui_str[i];
    }

    // font size
    ui_elements[SL_PERIODICITY]->font_size = 1.3f;
    ui_elements[SL_DENSITY]->font_size = 1.3f;
    ui_elements[SL_PAN]->font_size = 1.3f;
    ui_elements[SL_RAND_LOOP]->font_size = 1.3f;
    ui_elements[SL_GAIN]->font_size = 1.3f;
    ui_elements[SL_FREQ_WARP]->font_size = 1.3f;
    ui_elements[SL_TIME_STRETCH]->font_size = 1.3f;
    ui_elements[SL_PERCENTAGE]->font_size = 1.3f;
    ui_elements[SL_K]->font_size = 1.3f;
    ui_elements[SL_TOTAL_LEVELS]->font_size = 1.3f;
    ui_elements[SL_STARTLEVEL]->font_size = 1.3f;
    ui_elements[SL_STOPLEVEL]->font_size = 1.3f;
	ui_elements[FL_ANCFIRST]->font_size = ui_elements[FL_RANDFLIP]->font_size = .9f;
	ui_elements[SL_RAND_OLAR]->font_size = 1.3f;
	ui_elements[SL_MINDIST]->font_size = 1.3f;
	ui_elements[SL_SEGSIZE]->font_size = 1.3f;
	ui_elements[FL_SCALEAMP]->font_size = .9f;
	ui_elements[BT_CHANGE_RES_METHOD]->font_size = .7f;
	ui_elements[BT_CHANGE_RES_METHOD]->size_up = .04f;
    ui_elements[BT_LEFT]->font_size = .5f;
    ui_elements[BT_NOW]->font_size = .5f;
    ui_elements[BT_RIGHT]->font_size = .5f;
    // bounds
    ui_elements[SL_GAIN]->set_bounds( .1f, 10 );
    ui_elements[SL_FREQ_WARP]->set_bounds( .01f, 100 );
    ui_elements[SL_TIME_STRETCH]->set_bounds( .01f, 100 );
    ui_elements[SL_DENSITY]->set_bounds( .001f, 1000 );
    ui_elements[SL_TIMELINE_SHIFT]->set_bounds( 0, 1 );
    ui_elements[SL_TIMELINE_ZOOM]->set_bounds( 1, 10 );
    ui_elements[SL_TIMELINE_DURATION]->set_bounds( .01f, 100 );
    ui_elements[SL_RAND_LOOP]->set_bounds( 1.0f, 3.0f, false );
    // slide
    ui_elements[SL_PAN]->set_slide( .5f );
    ui_elements[SL_RAND_LOOP]->set_slide( 2.0f );
    ui_elements[SL_GAIN]->set_slide( 1.0f );
    ui_elements[SL_FREQ_WARP]->set_slide( 1.0f );
    ui_elements[SL_TIME_STRETCH]->set_slide( 1.0f );
    ui_elements[SL_DENSITY]->set_slide( 1.0f );
    ui_elements[SL_PERIODICITY]->set_slide( .5f );
    ui_elements[SL_TIMELINE_SHIFT]->set_slide( 0 );
    ui_elements[SL_TIMELINE_ZOOM]->set_slide( 1 );
    ui_elements[SL_TIMELINE_DURATION]->set_slide( 0.5f );
    ui_elements[BT_LEFT]->set_slide( 0.0f );
    ui_elements[BT_NOW]->set_slide( .5f );
    ui_elements[BT_RIGHT]->set_slide( 1.0f );
    ui_elements[BT_LEFT]->the_width = 1.7f;
    ui_elements[BT_LEFT]->size_up = .03f;
    ui_elements[BT_NOW]->size_up = .03f;
    ui_elements[BT_RIGHT]->size_up = .03f;
    ui_elements[FL_MINUTE]->set_slide( 1.0f );

    ui_elements[SL_PERCENTAGE]->set_bounds( .001f, 1.0f );
    ui_elements[SL_TOTAL_LEVELS]->set_bounds( 3.0f, (float)lg(CUTOFF), true );
    ui_elements[SL_STARTLEVEL]->set_bounds( 1.0f, (float)lg(CUTOFF) - 3, true );
    ui_elements[SL_STOPLEVEL]->set_bounds( 1.0f, (float)lg(CUTOFF) - 2, true );
    ui_elements[SL_PERCENTAGE]->set_slide( .25f );
    ui_elements[SL_K]->set_slide( .3f );
    ui_elements[SL_TOTAL_LEVELS]->set_slide( 13.0f );
    ui_elements[SL_STARTLEVEL]->set_slide( 1.0f );
    ui_elements[SL_STOPLEVEL]->set_slide( 9.0f );
	
	ui_elements[SL_RAND_OLAR]->set_bounds( 0.0f, 1.0f ); ui_elements[SL_RAND_OLAR]->set_slide(.1f);
	ui_elements[SL_MINDIST]->set_bounds( 0.0f, 1.5f ); ui_elements[SL_MINDIST]->set_slide(.0f);
	ui_elements[SL_SEGSIZE]->set_bounds( 0.0f, 3.0f ); ui_elements[SL_SEGSIZE]->set_slide(2.0f);

    // initialize
    down = NULL;
    selected = NULL;

    // log
    BB_log( BB_LOG_SYSTEM, "creating and selecting default timeline..." );
    // push log
    BB_pushlog();

    // make new timeline and select it
    timeline = new Timeline( get_duration() );
    timeline->name = "FirstTimeline";
    selected = Library::instance()->add( timeline );
    timeline = (Timeline *)selected->core;
    ui_elements[BT_NOW]->set_slide( timeline->duration * 0.5f / BirdBrain::srate() );
    set_timeline_bounds();
    
    // pop log
    BB_poplog();

    load_sliders = false;

    m_bg[0] = 1.0; m_bg[1] = 1.0; m_bg[2] = 1.0; m_bg[3] = 1.0;

    // -... -... --- -..-
    m_nbboxs = 4;
    m_cur_bbox = -1;
    m_bboxs = new BBox[m_nbboxs];
    // sliders
    m_bboxs[0].add2d( Point2D( -0.05f, -1.2f ) ); 
    m_bboxs[0].add2d( Point2D( 10.0f, -0.185f ) );
    // library
    m_bboxs[1].add2d( Point2D( -10.0f, -1.2f ) );
    m_bboxs[1].add2d( Point2D( -0.05f, -0.185f ) );
    // timeline
    m_bboxs[3].add2d( Point2D( -10.0f, 0.05f ) ); 
    m_bboxs[3].add2d( Point2D( 10.0f, 1.2f ) );
    // button
    m_bboxs[2].add2d( Point2D( -10.0f, -0.185f ) );
    m_bboxs[2].add2d( Point2D( 0.825f, 0.05f ) );
    // highlight ON/OFF
    m_highlight = false;

    // everything (default)
    m_vrs.push_back( new ViewRegionManual( 0, 0, -1.0f, 1.0f, FALSE, FALSE ) );
    // library
    m_vrs.push_back( new ViewRegionManual( -1.2, -0.05, -.925, -0.185, TRUE, TRUE ) );
    // slider
    m_vrs.push_back( new ViewRegionManual( -0.05, 1.2, -.925, 0.05, TRUE, TRUE ) );
    // timeline
    m_vrs.push_back( new ViewRegionManual( -1.25, 1.25, -.185, 1.1, TRUE, TRUE ) );
    // button
    m_vrs.push_back( new ViewRegionManual( -1.3, .825, -0.185, 0.05, TRUE, TRUE ) );

    // log
    BB_log( BB_LOG_INFO, "num ui elements: %d", NUM_UI_ELEMENTS );

#ifdef __TAPS_SCRIPTING_ENABLE__
    // scripting engine

    // log
    BB_log( BB_LOG_INFO, "initializing scripting engine..." );
    BB_pushlog();

    // start it
    ScriptCentral::startup();
    // start on last bus
    t_TAPUINT which_bus = AudioCentral::instance()->num_bus() - 2;
    // turn on frankenstein
    ScriptCentral::engine()->start_vm( AudioCentral::instance()->bus( which_bus ) );

    // pop log
    BB_poplog();
#endif // __TAPS_SCRIPTING_ENABLE__

    // pop log
    BB_poplog();

    // set
    if( !g_synth_face )
        g_synth_face = this;

    return TRUE;
}




//-----------------------------------------------------------------------------
// name: destroy()
// desc: ...
//-----------------------------------------------------------------------------
t_TAPBOOL UISynthesis::destroy( )
{
    this->on_deactivate( 0.0 );
    m_id = Audicle::NO_FACE;
    m_state = INACTIVE;

#ifdef __TAPS_SCRIPTING_ENABLE__
    // log
    BB_log( BB_LOG_INFO, "deallocating scripting engine..." );

    // stop it
    ScriptCentral::shutdown();
#endif // __TAPS_SCRIPTING_ENABLE__

    return TRUE;
}




//-----------------------------------------------------------------------------
// name: render_deterministic_pane()
// desc: we think the name of the function pretty much says it all
//-----------------------------------------------------------------------------
void UISynthesis::render_deterministic_pane()
{
    // load slider values
    if( load_sliders && selected )
    {
        ui_elements[SL_GAIN]->set_slide( selected->core->gain );
        ui_elements[SL_FREQ_WARP]->set_slide( selected->core->freq_warp );
        ui_elements[SL_TIME_STRETCH]->set_slide( selected->core->time_stretch );
        ui_elements[SL_PAN]->set_slide( selected->core->pan );
        load_sliders = false;
    }

    // sliders
    //draw_slider_mini( *ui_elements[SL_PERIODICITY], .3f, -.75f, 0.0f );    
    //draw_slider_mini( *ui_elements[SL_DENSITY], .5f, -.75f, 0.0f );    
    draw_slider_mini( *ui_elements[SL_FREQ_WARP], .7f, -.75f, 0.0f );    
    draw_slider_mini( *ui_elements[SL_TIME_STRETCH], .9f, -.75f, 0.0f );    
    draw_slider_mini( *ui_elements[SL_GAIN], 1.1f, -.75f, 0.0f );    
    draw_slider_h_mini( *ui_elements[SL_PAN], .7f, -.25f, 0.0f );

    // flipper
    draw_flipper( *ui_elements[FL_LOOP_ME], .3f, -.28f, 0.0f );
}


//-----------------------------------------------------------------------------
// name: render_transient_pane()
// desc: we think the name of the function pretty much says it all
//-----------------------------------------------------------------------------
void UISynthesis::render_transient_pane()
{
    // abuse
    render_deterministic_pane();
}


//-----------------------------------------------------------------------------
// name: render_residue_pane()
// desc: we think the name of the function pretty much says it all
//-----------------------------------------------------------------------------
void UISynthesis::render_residue_pane()
{
	Residue * res = (Residue *)selected->core;
	
    // load slider values
    if( load_sliders && selected )
    {
		if(res->m_method == Residue::TS) {
			Treesynth * ts = res->ts;
			ui_elements[SL_PERCENTAGE]->set_slide( ts->percentage );
			ui_elements[SL_K]->set_slide( ts->kfactor );
			ui_elements[SL_STARTLEVEL]->set_slide( ts->startlevel );
			ui_elements[SL_STOPLEVEL]->set_slide( ts->stoplevel );
			ui_elements[SL_TOTAL_LEVELS]->set_slide( ts->tree->getLevels() );
			ui_elements[SL_GAIN]->set_slide( selected->core->gain );
			ui_elements[SL_PAN]->set_slide( selected->core->pan );
			ui_elements[FL_ANCFIRST]->set_slide( ts->ancfirst ? 1.0 : 0.0 );
			ui_elements[FL_RANDFLIP]->set_slide( ts->randflip ? 1.0 : 0.0 );
		}
		else if(res->m_method == Residue::OLAR) {
			ui_elements[SL_RAND_OLAR]->set_slide(res->olar->get_randomness());
			ui_elements[SL_MINDIST]->set_slide(res->olar->get_mindist_secs());
			ui_elements[SL_SEGSIZE]->set_slide(res->olar->get_segsize_secs());
			ui_elements[FL_SCALEAMP]->set_slide(res->olar->get_scaleamp() ? 1.0 : 0.0);
		}
        load_sliders = false;
    }

	// method
	glPushMatrix();
	glTranslatef(-0.04f, -.26f, 0.0f);
	glDisable(GL_LIGHTING);
	glColor3f(0.8f, 0.0f, 0.0f);
	scaleFont(0.03);
	drawString(res->m_method == Residue::TS ? "Using wavelet tree" : "Using overlap-add");
	glEnable(GL_LIGHTING);
	glPopMatrix();
	draw_button(*ui_elements[BT_CHANGE_RES_METHOD], .47f, -.24f, .0f, .6f, .6f, .6f, IMG_NEXT); 

    // sliders
	if(res->m_method == Residue::TS) {
		draw_slider_mini( *ui_elements[SL_PERCENTAGE], .18f, -.75f, 0.0f );    
		draw_slider_mini( *ui_elements[SL_K], .36f, -.75f, 0.0f );    
		draw_slider_mini( *ui_elements[SL_STARTLEVEL], .54f, -.75f, 0.0f );    
		draw_slider_mini( *ui_elements[SL_STOPLEVEL], .72f, -.75f, 0.0f );    
		draw_slider_mini( *ui_elements[SL_TOTAL_LEVELS], .9f, -.75f, 0.0f );    
		draw_flipper( *ui_elements[FL_ANCFIRST], .04f, -.45f, 0.0f ); // .38, -.28
		draw_flipper( *ui_elements[FL_RANDFLIP], .04f, -.65f, 0.0f ); // .55, -.28
	}
	else if(res->m_method == Residue::OLAR) {
		draw_slider_mini( *ui_elements[SL_RAND_OLAR], .35f, -.75f, 0.0f );
		draw_slider_mini( *ui_elements[SL_MINDIST], .6f, -.75f, 0.0f );
		draw_slider_mini( *ui_elements[SL_SEGSIZE], .85f, -.75f, 0.0f );
		draw_flipper( *ui_elements[FL_SCALEAMP], .12f, -.55f, 0.0f ); // .4f, -.28
	}
	draw_slider_mini( *ui_elements[SL_GAIN], 1.1f, -.75f, 0.0f );    
	draw_slider_h_mini( *ui_elements[SL_PAN], .7f, -.25f, 0.0f );
}


//-----------------------------------------------------------------------------
// name: render_loop_pane()
// desc: we think the name of the function pretty much says it all
//-----------------------------------------------------------------------------
void UISynthesis::render_loop_pane()
{
    // load slider values
    if( load_sliders && selected )
    {
        ui_elements[SL_PERIODICITY]->set_slide( selected->core->periodicity );
        ui_elements[SL_DENSITY]->set_slide( selected->core->density );
        ui_elements[SL_GAIN]->set_slide( selected->core->gain );
        ui_elements[SL_FREQ_WARP]->set_slide( selected->core->freq_warp );
        ui_elements[SL_TIME_STRETCH]->set_slide( selected->core->time_stretch );
        ui_elements[SL_PAN]->set_slide( selected->core->pan );
        ui_elements[SL_RAND_LOOP]->set_slide( ((LoopTemplate *)selected->core)->random );
        load_sliders = false;
    }

    // sliders
    draw_slider_mini( *ui_elements[SL_PERIODICITY], .3f, -.75f, 0.0f );    
    draw_slider_mini( *ui_elements[SL_DENSITY], .5f, -.75f, 0.0f );    
    draw_slider_mini( *ui_elements[SL_FREQ_WARP], .7f, -.75f, 0.0f );    
    draw_slider_mini( *ui_elements[SL_TIME_STRETCH], .9f, -.75f, 0.0f );
    draw_slider_mini( *ui_elements[SL_GAIN], 1.1f, -.75f, 0.0f );    
    draw_slider_h_mini( *ui_elements[SL_PAN], .7f, -.25f, 0.0f );
    draw_slider_h_mini( *ui_elements[SL_RAND_LOOP], .1f, -.25f, 0.0f );
}


//-----------------------------------------------------------------------------
// name: render_bag_pane()
// desc: we think the name of the function pretty much says it all
//-----------------------------------------------------------------------------
void UISynthesis::render_bag_pane()
{
    if( !selected || selected->core->type != TT_BAG )
        return; 

    BagTemplate * bag; 
    int i, j; 
    
    // have enough bag element controls?
    make_bagels( selected ); 

    // load slider values
    if( load_sliders )
    {
        ui_elements[SL_PERIODICITY]->set_slide( selected->core->periodicity );
        ui_elements[SL_DENSITY]->set_slide( selected->core->density );
        ui_elements[SL_GAIN]->set_slide( selected->core->gain );
        ui_elements[SL_PAN]->set_slide( selected->core->pan );
        
        bag = (BagTemplate *)(selected->core);
        for( i = 0; i < bag->marbles.size(); i++ )
            for( j = 0; j < BagTemplate::nctrls; j++ )
                switch( j % BagTemplate::nctrls )
                {
                case 0: 
                    bagels[i*BagTemplate::nctrls + j]->set_slide( bag->marbles[i].playonce );
                    break;
                case 1:
                    bagels[i*BagTemplate::nctrls + j]->set_slide( bag->marbles[i].likelihood );
                    break;
                case 2:
                    bagels[i*BagTemplate::nctrls + j]->set_slide( bag->marbles[i].random ); 
                    break;
                default:
                    break;
                }

        load_sliders = false;
    }

    // draw elements
    bag = (BagTemplate *)(selected->core);
    UI_Template * ui_temp = NULL;
    float y_pos = -0.30f;
    float y_inc = -0.08f;
    if( y_pos + y_inc * (bag->marbles.size() - 1) < -0.8 ) // goes too far down? squeeze
        y_inc = (-0.8 - y_pos) / (bag->marbles.size() - 1); 
    for( i = 0; i < bag->marbles.size(); i++ )
    {
        ui_temp = bag->marbles[i].ui_temp;
        y_pos += y_inc;
        // draw it
        draw_template( 0.0f, y_pos, ui_temp, true );
        // draw its controls, if applicable
        if( ui_elements[FL_BAGEL_CONTROLS]->slide > 0.5 )
        {
            if( bag->marbles[i].changeable )
            {
                draw_flipper_micro( *bagels[i*BagTemplate::nctrls], 0.35f, y_pos + y_inc / 4, 0.0f );
                draw_slider_h_micro( *bagels[i*BagTemplate::nctrls + 1], 0.5f, y_pos, 0.0f ); 
                draw_slider_h_micro( *bagels[i*BagTemplate::nctrls + 2], 0.9f, y_pos, 0.0f );
            }
            else
            {               
                glPushMatrix();
                glTranslatef( 0.28f, y_pos + y_inc / 4, 0.0f );
                glDisable(GL_LIGHTING);
                glColor3f( 0.8f, 0.0f, 0.0f );
                scaleFont( .02 );
                drawString( "PLAY ONCE AND KEEP GOING . . . " );
                glEnable(GL_LIGHTING);
                glPopMatrix();
            }
        }
    }
   
    // sliders
    if( ui_elements[FL_BAGEL_CONTROLS]->slide <= 0.5f )
    {
        // bag controls
        draw_slider_mini( *ui_elements[SL_GAIN], 1.1f, -.75f, 0.0f );    
        draw_slider_h_mini( *ui_elements[SL_PAN], .7f, -.25f, 0.0f );
        draw_slider_mini( *ui_elements[SL_DENSITY], .9f, -.75f, 0.0f );
        draw_slider_mini( *ui_elements[SL_PERIODICITY], .7f, -.75f, 0.0f );  
    }
    
    // flipper
    draw_flipper( *ui_elements[FL_BAGEL_CONTROLS], 0.15f, -.28f, 0.0f );
}


//-----------------------------------------------------------------------------
// name: render_timeline_pane()
// desc: we think the name of the function pretty much says it all
//-----------------------------------------------------------------------------
void UISynthesis::render_timeline_pane()
{
    // load slider values
    if( load_sliders && selected )
    {
        ui_elements[SL_GAIN]->set_slide( selected->core->gain ); 
        ui_elements[SL_PAN]->set_slide( selected->core->pan ); 
        load_sliders = false;
    }

    // sliders
    draw_slider_mini( *ui_elements[SL_GAIN], 1.1f, -.75f, 0.0f );    
    draw_slider_h_mini( *ui_elements[SL_PAN], .7f, -.25f, 0.0f );
}


#ifdef __TAPS_SCRIPTING_ENABLE__
//-----------------------------------------------------------------------------
// name: render_script_pane()
// desc: the script pane offers... uh nothing
//-----------------------------------------------------------------------------
void UISynthesis::render_script_pane()
{
    if( !selected || selected->core->type != TT_SCRIPT )
		return;

	Scriptor * script = (Scriptor *)selected->core;
	// for each element
	for( int i = 0; i < script->num_elements(); i++ )
	{
		UI_Element * ele = script->element_at(i);
		if( ele->id == 0 )
			ele->id = IDManager::instance()->getPickID();
		// draw if it has a position
		if( ele->rel_loc )
		{
			// adjust location
			if( ele->must_adjust_loc )
				ele->adjust_loc( m_vrs[2] );
			// draw
			if( ele->element_type == UI_Element::UI_SLIDER || ele->element_type == UI_Element::UI_FLIPPER )
				ele->offset = ele->draw();
			else if( ele->element_type == UI_Element::UI_BUTTON && ele->loc )
				draw_button( *ele, 0, 0, 0, 0.5, 0.5, 0.5 );
		}
		else
			BB_log( BB_LOG_INFO, "Cannot draw TapsUI element with no position" );
	}
}
#endif


//-----------------------------------------------------------------------------
// name: render_the_usual()
// desc: we think the name of the function pretty much says it all
//-----------------------------------------------------------------------------
void UISynthesis::render_the_usual()
{
    draw_lr_butter( *ui_elements[BT_LEFT], ui_elements[BT_NOW], *ui_elements[BT_RIGHT],
        -1.1f, 0.2f, 0.0f );

    // draw sliders
    draw_slider( *ui_elements[SL_TIMELINE_DURATION], .85f, .25f, 0.0f );
    draw_slider_h_micro( *ui_elements[SL_TIMELINE_SHIFT], -1.1f, 0.07f, 0.0f );
    draw_slider_h_micro( *ui_elements[SL_TIMELINE_ZOOM], -0.7f, 0.07f, 0.0f );

    // draw flippers
    draw_flipper2( *ui_elements[FL_WEEK], 1.0f, .7f, 0.0f );
    draw_flipper2( *ui_elements[FL_DAY], 1.0f, .6f, 0.0f );
    draw_flipper2( *ui_elements[FL_HOUR], 1.0f, .5f, 0.0f );
    draw_flipper2( *ui_elements[FL_MINUTE], 1.0f, .4f, 0.0f );
    draw_flipper2( *ui_elements[FL_SECOND], 1.0f, .3f, 0.0f );
    draw_flipper2( *ui_elements[FL_MS], 1.0f, .2f, 0.0f );
    draw_flipper( *ui_elements[FL_BUS_WRITE_STEREO], -1.1f, -.10f, 0.0f );
	draw_flipper( *ui_elements[FL_BUS_WRITE_MULTI], -0.95f, -.10f, 0.0f );
	draw_flipper( *ui_elements[FL_BUS_WRITE_BOTH], -0.8f, -.10f, 0.0f );

    // draw buttons
    draw_button( *ui_elements[BT_NEW_TIMELINE], .7f, .5f, 0.0f, .5f, .5f, 1.0f, IMG_NEWT );
    // buttons
    draw_button( *ui_elements[BT_LOAD_FILE], 0.0f, -0.05f, 0.0f, .5f, .5f, 1.0f, IMG_LOAD );
    draw_button( *ui_elements[BT_LISTEN], .15f, -0.05f, 0.0f, .5f, 1.0f, .5f, IMG_PLAY );
    draw_button( *ui_elements[BT_STOP], .3f, -0.05f, 0.0f, 1.0f, .5f, .5f, IMG_STOP );
    draw_button( *ui_elements[BT_REVERT], .45f, -0.05f, 0.0f, 1.0f, 0.75f, .25f, IMG_REV );
    draw_button( *ui_elements[BT_COPY], .6f, -0.05f, 0.0f, 1.0f, .5f, 1.0f, IMG_COPY );
    draw_button( *ui_elements[BT_SAVE_FILE], .75f, -0.05f, 0.0f, 1.0f, 1.0f, .5f, IMG_SAVE );

    float left = -1.1f;
    float top = g_butter_height + .2f;
    float bottom = 0.0f + .2f;
    float right = .6f;

    // draw line (for timeline)
        // "invisible" quad
    glPushName( ui_elements[TL_TIMELINE]->id );
    glColor4f( 1.0f, 1.0f, 1.0f, 0.0f );
    glBegin( GL_QUADS );
    glVertex3f( left, bottom + .025f, -.1f); // + 0.025f to avoid hit detection conflict with butter
    glVertex3f( right, bottom + .025f, -.1f ); // + 0.025f to avoid hit detection conflict with butter
    glVertex3f( right, top, -.1f );
    glVertex3f( left, top, -.1f );
    glEnd();

        // line
    glLineWidth( 3 );
    glColor3f( 0.0f, 0.0f, 0.0f );
    glBegin( GL_LINES );
    //glVertex2f( -1.1f, .5f );
    //glVertex2f( .6f, .5f );
    glVertex2f( left, (top + bottom) / 2 );
    glVertex2f( right, (top + bottom) / 2 );
    glEnd();
    glLineWidth( 1 );

        // ticks
    float slide_L = ui_elements[BT_LEFT]->slide_0; // left butter bound
    float slide_R = ui_elements[BT_RIGHT]->slide_1; // right butter bound
    t_TAPTIME unit = get_tick_unit();    
    if( unit > 0 ) 
    {
        float tick_height = (top - bottom) / 40; // g_butter_height = top - bottom
        float center = (top + bottom) / 2;
        t_TAPTIME duration = (timeline) ? timeline->duration : get_duration();
        glColor3f( 0.5f, 0.0f, 0.0f );
        for( int u = 1; u < duration / unit; u++ )
        {
            float u_sec = u*unit/BirdBrain::srate();
            if( u_sec < slide_L )
                continue;
            if( u_sec > slide_R )
                break;

            float tick_point = left + (u_sec - slide_L) * (right - left)/(slide_R - slide_L);
            //float tick_point = left + (u*unit) * (right - left) / duration;
            glBegin( GL_LINES );
            glVertex2f( tick_point, center + tick_height );
            glVertex2f( tick_point, center - tick_height );         
            glEnd();
        }
    }
    glPopName();

    char duration[128];
    bool there_is_more = false;
    bool there_is_less = false;

    // draw templates on timeline
    if( timeline != NULL )
    {
        UI_Template * ui_temp = NULL;
        float where;
        for( int i = 0; i < timeline->instances.size(); i++ )
        {
            ui_temp = timeline->instances[i].ui_temp;
            where = timeline->instances[i].start_time;
            // scale the where
            //where /= (float)timeline->duration;
            // shift and scale the where
            where = (where/BirdBrain::srate() - slide_L) / (slide_R - slide_L);
            if( where <= 1.0f && where >= 0.0f )
            {
                // world coordinate
                where = (where * ui_elements[BT_LEFT]->the_width) - 1.1f;
                // draw it
                draw_template( where, .5f, ui_temp, false, timeline->instances[i].y_offset );
            }
            else
            {
                there_is_more = there_is_more || (where > 1.0f);
                there_is_less = there_is_less || (where < 0.0f);
            }
        }
        
        // get duration
        sprintf( duration, "duration: %f seconds", 1.0 * timeline->duration / BirdBrain::srate() );
    }
    else
    {
        sprintf( duration, "duration: undefined (select or create a timeline!)" );
    }

    // there is more after?
    if( there_is_more )
    {
        glColor3f( 1.0f, 0.0f, 0.0f );
        glBegin( GL_TRIANGLES );
        glVertex2f( right, (top + bottom) / 2 + .025f );
        glVertex2f( right, (top + bottom) / 2 - .025f );
        glVertex2f( right + .025f, (top + bottom) / 2 );
        glEnd();        
    }
    // there is more before?
    if( there_is_less )
    {
        glColor3f( 1.0f, 0.0f, 0.0f );
        glBegin( GL_TRIANGLES );
        glVertex2f( left, (top + bottom) / 2 + .025f );
        glVertex2f( left - .025f, (top + bottom) / 2 );
        glVertex2f( left, (top + bottom) / 2 - .025f );
        glEnd();        
    }

    // timeline scrolling?
    if( timeline && timeline->playing() )
    {
        float btnow = ui_elements[BT_NOW]->fvalue();
        if( (btnow < slide_L || btnow > slide_R) && slide_R - slide_L < timeline->duration/BirdBrain::srate() )
        {
            float shift = ui_elements[BT_NOW]->fvalue() / 
                (timeline->duration/BirdBrain::srate() - (slide_R - slide_L));
            if( shift < 0 ) shift = 0;
            if( shift > 1 ) shift = 1;
            ui_elements[SL_TIMELINE_SHIFT]->set_slide( shift );
            set_timeline_bounds();
        }
    }


    // draw duration
    glPushMatrix();
        glTranslatef( -1.2f, g_butter_height + .3f, 0.0f );
        glColor3f( 0.0f, 0.0f, 0.0f );
        scaleFont( .030 );
        drawString( duration );
    glPopMatrix();
}


//-----------------------------------------------------------------------------
// name: render()
// desc: ...
//-----------------------------------------------------------------------------
t_TAPUINT UISynthesis::render( void * data )
{
    if( selected != NULL && selected->core != NULL )
        ui_elements[BT_LISTEN]->on = selected->core->playing();
    else
        ui_elements[BT_LISTEN]->on = false;

    // figure out which pane to draw
    if( selected == NULL )
    {
        // draw windows xp logo here
        //render_deterministic_pane();
    }
    else if( selected->core->type == TT_DETERMINISTIC )
    {
        render_deterministic_pane();
    }
    else if( selected->core->type == TT_TRANSIENT )
    {
        render_transient_pane();
    }
    else if( selected->core->type == TT_RESIDUE )
    {
        render_residue_pane();
    }
    else if( selected->core->type == TT_LOOP )
    {
        render_loop_pane();
    }
    else if( selected->core->type == TT_TIMELINE )
    {
        render_timeline_pane();
    }
    else if( selected->core->type == TT_FILE )
    {
        render_transient_pane();
    }
    else if( selected->core->type == TT_BAG )
    {
        render_bag_pane();
    }
    else if( selected->core->type == TT_SCRIPT )
    {
#ifdef __TAPS_SCRIPTING_ENABLE__
        render_script_pane();
#endif
    }
    else if( selected->core->type = TT_RAW )
    {
        render_transient_pane();
    }
    else
    {
        msg_box( "about to crash", "what is this type?" );
        assert( FALSE );
    }

    // draw the usual
    render_the_usual();

    char name[128];
    char type[128];

    if( selected == NULL )
    {
        strcpy( name, "[nothing selected]" );
        strcpy( type, "(no type)" );
    }
    else
    {
        strcpy( name, selected->core->name.c_str() );
        sprintf( type, "(%s)", selected->core->type_str() );
        glLineWidth( 1.0 );
        draw_template( 1.0f, 0.00f, selected, false );
    }

    // name
    glDisable( GL_LIGHTING );
    glLineWidth( 2.0f );
    glPushMatrix();
        glTranslatef( 1.0f, -0.08f, 0.0f );
        glColor3f( 0.0f, 0.0f, 0.0f );
        scaleFont( .035 );
        drawString_centered( name );
    glPopMatrix();
    glPushMatrix();
        glTranslatef( 1.0f, -.16f, 0.0f );
        glColor3f( 0, 0, 1.0 );
        scaleFont( .030 );
        drawString_centered( type );
    glPopMatrix();
    glLineWidth( 1.0f );
    
    // render
    g_r += 1.0;

    // highlighting
    if( m_highlight && m_cur_bbox >= 0 && m_cur_bbox < m_nbboxs )
    {
        Point3D p1 = m_bboxs[m_cur_bbox].pmin();
        Point3D p2 = m_bboxs[m_cur_bbox].pmax();
        glPushMatrix();
            glTranslatef( 0.0f, 0.0f, -0.5f );
            glColor3f( 0.925f, 0.925f, 0.925f );
            glBegin( GL_QUADS );
            glVertex2f( p1[0], p1[1] );
            glVertex2f( p2[0], p1[1] );
            glVertex2f( p2[0], p2[1] );
            glVertex2f( p1[0], p2[1] );
            glEnd();
        glPopMatrix();
    }
    glEnable( GL_LIGHTING );

    // draw library
    draw_library( selected, timeline, -1.05f, -0.8f, -0.25f, 0.2f );

    // faces / control
    draw_face_buttons( BT_ANALYSIS_FACE, BT_SEARCH_FACE );
	draw_ctrl_buttons( BT_QUIT, BT_FULLSCREEN );
	
    // more danger
    // the arrow is drawn only during render pass, and not select pass
    bool render_pass = ( AudicleWindow::main()->m_render_mode == GL_RENDER );
    // draw arrow
    if( down && render_pass )
        draw_arrow( curr_pt, prev_pt, orig_pt, linecol, highlight );

    return 0;
}




//-----------------------------------------------------------------------------
// name: render_pre()
// desc: ...
//-----------------------------------------------------------------------------
void UISynthesis::render_pre()
{
    AudicleFace::render_pre();

    glPushAttrib( GL_LIGHTING_BIT | GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ); 

    // enable depth
    glEnable( GL_DEPTH_TEST );
    // enable lighting
    glEnable( GL_LIGHTING );

    // enable light 0
    glEnable( GL_LIGHT0 );

    // material have diffuse and ambient lighting 
    glColorMaterial( GL_FRONT, GL_AMBIENT_AND_DIFFUSE );
    // enable color
    glEnable( GL_COLOR_MATERIAL );
    
    // setup and enable light 1
    glLightfv( GL_LIGHT1, GL_AMBIENT, m_light1_ambient );
    glLightfv( GL_LIGHT1, GL_DIFFUSE, m_light1_diffuse );
    glLightfv( GL_LIGHT1, GL_SPECULAR, m_light1_specular );
    glEnable( GL_LIGHT1 );
}




//-----------------------------------------------------------------------------
// name: render_post()
// desc: ...
//-----------------------------------------------------------------------------
void UISynthesis::render_post()
{
    glPopAttrib();

    AudicleFace::render_post();
}




//-----------------------------------------------------------------------------
// name: render_view()
// desc: ...
//-----------------------------------------------------------------------------
void UISynthesis::render_view( )
{
    if( m_first )
    {
        // set again
        m_vr.init( *m_vrs[0] );
        m_first = false;
    }

    // set the matrix mode to project
    glMatrixMode( GL_PROJECTION );
    // load the identity matrix
    // this is handled by AudicleWindow, in order to set up pick matrices...
    // you can assume that LoadIdentity has been called already
    // glLoadIdentity( ); 
    // create the viewing frustum
    // gluPerspective( 45.0, (GLfloat) AudicleWindow::main()->m_w / 
    //     (GLfloat) AudicleWindow::main()->m_h, 1.0, 300.0 );
    
    glOrtho( m_vr.left(), m_vr.right(), m_vr.down(), m_vr.up(), -10, 10 );
    m_vr.next();

    // set the matrix mode to modelview
    glMatrixMode( GL_MODELVIEW );
    // load the identity matrix
    glLoadIdentity( );
    // position the view point
    gluLookAt( 0.0f, 0.0f, 3.5f, 
               0.0f, 0.0f, 0.0f, 
               0.0f, 1.0f, 0.0f );

    // set the position of the lights
    glLightfv( GL_LIGHT0, GL_POSITION, m_light0_pos );
    glLightfv( GL_LIGHT1, GL_POSITION, m_light1_pos );
}




//-----------------------------------------------------------------------------
// name: on_activate()
// desc: ...
//-----------------------------------------------------------------------------
t_TAPUINT UISynthesis::on_activate()
{
    g_text_color[0] = 0.0f;
    g_text_color[1] = 0.0f;
    g_text_color[2] = 0.0f;
    return AudicleFace::on_activate();
}




//-----------------------------------------------------------------------------
// name: on_deactivate()
// desc: ...
//-----------------------------------------------------------------------------
t_TAPUINT UISynthesis::on_deactivate( t_TAPDUR dur )
{
    return AudicleFace::on_deactivate( dur );
}


void UISynthesis::toggle_rand_loop( )
{
    t_TAPBOOL r = ui_elements[SL_RAND_LOOP]->slide > .5f;

    if( r )
    {
        // gain
        ui_elements[SL_GAIN]->set_slide( .5f );
        ui_elements[SL_GAIN]->set_slide2( 2.0f );
        // periodicity
        ui_elements[SL_PERIODICITY]->set_slide( .25f );
        ui_elements[SL_PERIODICITY]->set_slide2( .75f );
        // density
        ui_elements[SL_DENSITY]->set_slide( .5f );
        ui_elements[SL_DENSITY]->set_slide2( 2.0f );
        // freq
        ui_elements[SL_FREQ_WARP]->set_slide( .5f );
        ui_elements[SL_FREQ_WARP]->set_slide2( 2.0f );
        // time
        ui_elements[SL_TIME_STRETCH]->set_slide( .5f );
        ui_elements[SL_TIME_STRETCH]->set_slide2( 2.0f );
    }
}


static const double dur_values[] = 
{
    1.0 / 1000,  // ms
    1.0, // second
    60.0, // minute
    3600.0, // hour
    3600.0 * 24.0, // day
    3600.0 * 24.0 * 7, // week
};

t_TAPTIME UISynthesis::get_duration()
{
    int i = -1, j;
    for( j = FL_MS; j <= FL_WEEK; j++ )
    {
        if( ui_elements[j]->slide > .5f )
        {
            i = j;
            break;
        }
    }

    assert( i != -1 );
    // j = i-FL_MS;
    t_TAPTIME unit = BirdBrain::srate() * dur_values[i-FL_MS];
    t_TAPTIME dur = ui_elements[SL_TIMELINE_DURATION]->fvalue() * unit;

    return dur;
}

void UISynthesis::set_duration( t_TAPTIME samples )
{
    double seconds = samples / BirdBrain::srate(); 
    double dur = -1, d; 
    int i, j;
    for( j = FL_MS; j <= FL_WEEK; j++ )
    {
        d = seconds / dur_values[j-FL_MS];
        if( dur < 0 || fabs(1/d - 1) < fabs(1/dur - 1) )
        {
            dur = d; 
            i = j;
        }
        ui_elements[j]->set_slide( 0.0f );
    }
    
    ui_elements[SL_TIMELINE_DURATION]->set_slide( dur );
    ui_elements[i]->set_slide( 1.0f );
}


// return tickmark unit based on the duration slider and flippers
t_TAPTIME UISynthesis::get_tick_unit()
{
    // find out which flipper's selected
    int i = -1, j;
    for( j = FL_MS; j <= FL_WEEK; j++ )
    {
        if( ui_elements[j]->slide > .5f )
        {
            i = j;
            break;
        }
    }

    assert( i != -1 );
    
    // read slider value
    t_TAPTIME val = ui_elements[SL_TIMELINE_DURATION]->fvalue();

    // pick unit depending on val (if val < 1, pick previous unit, if val > 60, pick next unit)
    if( val <= 1 ) {
        if( i > FL_MS ) 
            i--;
    }
    else if( val > 60 ) {
        if( i < FL_WEEK - FL_MS )
            i++;
    }

    // determine how many sample that unit is
    t_TAPTIME samples = BirdBrain::srate() * dur_values[i-FL_MS]; 

    return samples;
}


//-----------------------------------------------------------------------------
// name: on_event()
// desc: ...
//-----------------------------------------------------------------------------
t_TAPUINT UISynthesis::on_event( const AudicleEvent & event )
{
    static t_TAPUINT m_mouse_down = FALSE;
    static t_TAPUINT which = 0;
    static Point2D last;
    t_TAPBOOL hit = FALSE;
    t_TAPBOOL somewhere = FALSE; // danger
    Point2D diff;
    int i, j, k;

    if( event.type == ae_event_INPUT )
    {
        InputEvent * ie = (InputEvent *)event.data;
        if( ie->type == ae_input_MOUSE )
        {
            ie->popStack();

            for( i = 0; i < NUM_UI_ELEMENTS; i++ )
            {
                if( ie->checkID( ui_elements[i]->id ) )
                {
                    somewhere = TRUE; // danger

                    if( ie->state == ae_input_DOWN )
                    {
                        hit = TRUE;
                        last_hit_pos = ie->pos; 
                        ui_elements[i]->down = TRUE;

                        // sliders
                        if( i >= SL_PERIODICITY && i <= SL_SEGSIZE )
                            ui_elements[i]->slide_last = ( vr_to_world_y(m_vr,ie->pos[1]) + .75 ) / g_slider_height_mini;
                        if( i >= BT_LEFT && i <= BT_RIGHT )
                            ui_elements[i]->slide_last = ( vr_to_world_x(m_vr,ie->pos[0]) + 1.1 ) / ui_elements[BT_LEFT]->the_width;
                        if( i == SL_PAN )
                            ui_elements[i]->slide_last = ( vr_to_world_x(m_vr,ie->pos[0]) -.7 ) / g_slider_height_mini;
                        if( i == SL_RAND_LOOP )
                            ui_elements[i]->slide_last = ( vr_to_world_x(m_vr,ie->pos[0]) -.1 ) / g_slider_height_mini;
                        if( i == SL_TIMELINE_SHIFT )
                            ui_elements[i]->slide_last = ( vr_to_world_x(m_vr,ie->pos[0]) + 1.1 ) / g_slider_height_micro;
                        if( i == SL_TIMELINE_ZOOM )
                            ui_elements[i]->slide_last = ( vr_to_world_x(m_vr,ie->pos[0]) + .7 ) / g_slider_height_micro;
                        if( i == SL_TIMELINE_DURATION )
                            ui_elements[i]->slide_last = ( vr_to_world_y(m_vr,ie->pos[1]) -.25 ) / g_slider_height;
                    }

                    if( ie->state == ae_input_UP && ui_elements[i]->down == TRUE )
                    {
                        if( i == FL_LOOP_ME )
                        {
                            ui_elements[i]->slide = 1.0f - ui_elements[i]->slide;
                            if( selected != NULL )
                                selected->core->set_param( Template::LOOP_ME, ui_elements[i]->fvalue() );
                        }
                        else if( i == FL_BAGEL_CONTROLS )
                        {
                            ui_elements[i]->slide = 1.0f - ui_elements[i]->slide;
                        }
                        else if( i >= FL_MS && i <= FL_WEEK )
                        {
                            for( j = FL_MS; j <= FL_WEEK; j++ )
                            {
                                ui_elements[j]->slide = 0.0f;
                            }
                            ui_elements[i]->slide = 1.0f;
                            t_TAPTIME dur = get_duration();
                            if( timeline )
                            {
                                timeline->duration = dur;
                                if( timeline->starttime > timeline->duration )
                                    timeline->starttime = 0; 
                                if( timeline->stoptime > timeline->duration )
                                    timeline->stoptime = timeline->duration;
                            }
                            ui_elements[SL_TIMELINE_SHIFT]->set_slide( 0 );
                            ui_elements[SL_TIMELINE_ZOOM]->set_slide( 1 );
                            set_timeline_bounds();
                        }
                        else if( i == FL_ANCFIRST )
                        {
                            ui_elements[i]->slide = 1.0f - ui_elements[i]->slide;
                            if( selected != NULL )
                                selected->core->set_param( Template::ANCFIRST, ui_elements[FL_ANCFIRST]->fvalue() );
                        }
                        else if( i == FL_RANDFLIP )
                        {
                            ui_elements[i]->slide = 1.0f - ui_elements[i]->slide;
                            if( selected != NULL )  
                                selected->core->set_param( Template::RANDFLIP, ui_elements[FL_RANDFLIP]->fvalue() );
                        }
						else if( i == FL_SCALEAMP )
						{
							ui_elements[i]->slide = 1.0f - ui_elements[i]->slide;
							if( selected != NULL )
								selected->core->set_param( Template::SCALE_AMP, ui_elements[FL_SCALEAMP]->fvalue() );
						}
                        /*else if( i == SL_RAND_LOOP )
                        {
                            ui_elements[i]->slide = 1.0f - ui_elements[i]->slide;
                            if( selected != NULL ) {
                                selected->core->set_param( Template::RANGE_MODE, ui_elements[i]->fvalue() );
                            }
                            // do stuff
                            //toggle_rand_loop();
                        }*/
                        else if( i >= FL_BUS_WRITE_STEREO && i <= FL_BUS_WRITE_BOTH )
                        {
                            ui_elements[i]->slide = 1.0f - ui_elements[i]->slide;
							// start recording
							if( ui_elements[i]->slide > .5f ) {
								// stop other flippers
								for( j = FL_BUS_WRITE_STEREO; j <= FL_BUS_WRITE_BOTH; j++ )
								{
									if( j != i && ui_elements[j]->slide > .5 )
									{
										ui_elements[j]->slide = 1.0f - ui_elements[j]->slide; 
										AudioCentral::instance()->record_stop( j - FL_BUS_WRITE_STEREO );
									}
								}

								// generate file name
                                char buffer[1024];
                                char prefix[] = "tapestrea-";
    
                                time_t t; time(&t);
                                strcpy( buffer, prefix );
                                strcat( buffer, "(" );
                                strncat( buffer, ctime(&t), 24 );
                                buffer[strlen(prefix)+14] = 'h';
                                buffer[strlen(prefix)+17] = 'm';
								strcat( buffer, ")_" );
                                
								// start record
								if( i == FL_BUS_WRITE_STEREO )
								{
									strcat( buffer, "2.wav" );
									// change to start dir
									BirdBrain::goto_start_dir();
									// go!
									AudioCentral::instance()->record_start( AudioCentral::STEREO, std::string(buffer) );
								}
								else if( i == FL_BUS_WRITE_MULTI )
								{
									strcat( buffer, "M.wav" );
									// change to start dir
									BirdBrain::goto_start_dir();
									// go!
									AudioCentral::instance()->record_start( AudioCentral::MULTICHANNEL, std::string(buffer) );
								}
								else if( i == FL_BUS_WRITE_BOTH )
								{
									filename = std::string(buffer) + "2.wav"; 
									std::string filename2 = std::string(buffer) + "M.wav"; 
									// change to start dir
									BirdBrain::goto_start_dir();
									// go!
									AudioCentral::instance()->record_start( AudioCentral::BOTH, filename, filename2 );
								}
                            } 
							// stop recording
							else 
							{
								if( i == FL_BUS_WRITE_STEREO )
									AudioCentral::instance()->record_stop( AudioCentral::STEREO );
								else if( i == FL_BUS_WRITE_MULTI )
									AudioCentral::instance()->record_stop( AudioCentral::MULTICHANNEL );
								else if( i == FL_BUS_WRITE_BOTH )
									AudioCentral::instance()->record_stop( AudioCentral::BOTH );
                            }
                        }
						else if( i == BT_CHANGE_RES_METHOD )
						{
							if( selected != NULL && selected->core != NULL ) 
							{
								Residue * res = (Residue *)selected->core;
								int m = res->m_method;
								m = (m + 1) % Residue::NUM_METHODS;
								res->change_method(m);
								load_sliders = true;
							}
						}
                        else if( i == BT_COPY )
                        {
                            if( selected != NULL )
                            {
                                assert( selected->core != NULL );
                                Template * temp = NULL;

                                // if this is the creation of a loop
                                if( ui_elements[FL_LOOP_ME]->slide > .5f )
                                {
                                    LoopTemplate * loop = new LoopTemplate( *selected->core );
                                    loop->copy_params( *selected->core );
                                    temp = loop;
                                    // reset flipper to avoid looping unloopable things later
                                    ui_elements[FL_LOOP_ME]->slide = 0.0f; 
                                    load_sliders = true;
                                }
                                else
                                {
                                    temp = selected->core->copy(); // but with new template id
									temp->name += "Copy"; 
                                }

                                // timeline/bag special treatment
                                if( temp->type == TT_TIMELINE || temp->type == TT_BAG )
                                    create_template_dummies( temp ); 

                                Library::instance()->add( temp );
                            }
                            else
                            {
                                msg_box( "pastatree!!!", "select something first!" );
                            }
                        }
                        else if( i == BT_REVERT )
                        {
                            if( selected != NULL )
                            {
                                assert( selected->core != NULL );
                                assert( selected->backup != NULL );
                                // copy parameters back to backup
                                selected->core->copy_params( *selected->backup );
                                load_sliders = true;
                            }
                            else
                            {
                                msg_box( "pastatree!!!", "select something first!" );
                            }
                        }
                        else if( i == BT_LISTEN )
                        {
                            if( selected != NULL )
                            {
                                play_template( selected ); 
                            }
                            else
                            {
                                msg_box( "pastatree!!!", "select something first!" );
                            }
                        }
                        else if( i == BT_STOP )
                        {
                            if( selected != NULL )
                                selected->core->stop();
                        }
                        else if( i == BT_NEW_TIMELINE )
                        {
                            // old timeline: null now butter
							if( timeline ) 
								timeline->now_butter = NULL;
							// make new timeline
							timeline = new Timeline( get_duration() );
                            // set the name
                            timeline->name = "Timeline"+BirdBrain::toString( ++g_tl_count );
                            // add to library
                            // make selected be that timeline, because otherwise
                            // if we select one timeline and create another one, "selected" is the old one
                            // but "timeline" is the new one
                            selected = Library::instance()->add( timeline );
                            timeline = (Timeline *)selected->core;
                            timeline->now_butter = ui_elements[BT_NOW];

                            // reset shift and zoom and now
                            ui_elements[SL_TIMELINE_SHIFT]->set_slide( 0 );
                            ui_elements[SL_TIMELINE_ZOOM]->set_slide( 1 );
                            ui_elements[BT_NOW]->set_slide( timeline->duration * 0.5f / BirdBrain::srate() );
                            set_timeline_bounds();
                            load_sliders = true;
                        }
                        else if( i == BT_SAVE_FILE )
                        {
                            // verify that something is selected
                            if( selected != NULL )
                            {
                                DirScanner dScan;
#ifdef __TAPS_XML_ENABLE__
                                dScan.setFileTypes( "TAPESTREA Template Files (*.tap;*.xml)\0*.tap;*.xml\0Tapestrea Template XML Files (*.xml)\0*.xml\0Tapestrea Template TAP files (*.tap)\0*.tap\0All Files (*.*)\0*.*\0" );
#else	
								dScan.setFileTypes( "Tapestrea Template TAP files (*.tap)\0*.tap\0All Files (*.*)\0*.*\0" );
#endif
                                fileData * fD = dScan.saveFileDialog();
                                if ( fD )
                                {
                                    if( fD->fileName.rfind( ".tap" ) == fD->fileName.length()-4 
										|| fD->fileName.rfind( ".xml" ) == fD->fileName.length()-4 )
                                    {
                                        save_to_file(selected->core, fD->fileName, NULL);
                                    }
									else
                                    {
                                        msg_box( "nice try", "only .tap or .xml files are allowed here" );
                                    }
                                }
                            }
                            else
                            {
                                // bad
                                msg_box( "pastatree!!!", "select something first!" );
                            }
                        }                   
                        else if( i == BT_LOAD_FILE )
                        {
                            DirScanner dScan;
#ifdef __TAPS_XML_ENABLE__
							dScan.setFileTypes( "All Tapestrea Files (*.tap;*.ck;*.xml;*.qz;*.wav)\0*.tap;*.ck;*.xml;*.qz;*.wav\0Tapestrea Template Files (*.tap;*.xml)\0*.tap;*.xml\0Tapestrea Template XML Files (*.xml)\0*.xml\0Tapestrea Template TAP files (*.tap)\0*.tap\0Tapestrea ChucK Files (*.ck)\0*.ck\0Tapestrea Quantization Files (*.qz)\0*.qz\0Wave Files (*.wav)\0*.wav\0All Files (*.*)\0*.*\0" );
#else
							dScan.setFileTypes( "All Tapestrea Files (*.tap;*.ck;*.qz;*.wav)\0*.tap;*.ck;*.qz;*.wav\0Tapestrea Template TAP files (*.tap)\0*.tap\0Tapestrea ChucK Files (*.ck)\0*.ck\0Tapestrea Quantization Files (*.qz)\0*.qz\0Wave Files (*.wav)\0*.wav\0All Files (*.*)\0*.*\0" );
#endif
							fileData * fD = dScan.openFileDialog();

                            // loop over file list
                            while( fD )
                            {
                                BB_log( BB_LOG_INFO, "opening file : %s", fD->fileName.c_str() );
                                const char * c = fD->fileName.c_str();
                                //if ( fD->next ) 
                                //    fprintf( stderr, "opening first of multiple files...\n");
                                
                                if( fD->fileName.rfind( ".tap" ) == fD->fileName.length()-4 
									|| fD->fileName.rfind( ".xml" ) == fD->fileName.length()-4 )
								{
                                    BB_log( BB_LOG_INFO, "Reading '%s' as template file",
                                            BirdBrain::getbase(fD->fileName.c_str()) );
                                    TemplateReader r;
                                    if( r.open( (char *)c ) )
                                    {
                                        Template * tmp = r.read_template();
                                        r.close();
                                        if( tmp != NULL && !Library::instance()->hasID( tmp ) )
                                        {
                                            Library::instance()->add( tmp );
                                        }
                                        else if( tmp != NULL )
                                        {
                                            msg_box( "make a copy", "you've already loaded this one" );
                                        }
										else {
											msg_box( "ding!", "could not read the template" );
										}
                                    }
                                    else
                                    {
                                        msg_box( "ding!", "cannot open file for reading!" );
                                    }
                                }
								else if( fD->fileName.rfind( ".ck" ) == fD->fileName.length()-3 )
                                {
                                #ifdef __TAPS_SCRIPTING_ENABLE__ 
                                    BB_log( BB_LOG_INFO, "Reading '%s' as .ck script file",
                                            BirdBrain::getbase(fD->fileName.c_str()) );
                                    Scriptor * script = ScriptCentral::engine()->compile( fD->fileName );
                                    if( script != NULL )
                                    {
                                        script->name = BirdBrain::getname(fD->fileName.c_str());
                                        // replace space with _
                                        int space = script->name.find( " ", 0 );
                                        while( space != std::string::npos )
                                        {
                                            script->name[space] = '_';
                                            space = script->name.find( " ", space );
                                        }
                                        Library::instance()->add( script );
                                    }
                                    else
                                    {
                                        msg_box( "ding!", "parse/type/bus/bike error in script" );
                                    }
                                #else
                                    msg_box( "ding!", "scripting not enabled during compilation" );
                                #endif
                                }
                                else if( fD->fileName.rfind( ".qz" ) == fD->fileName.length()-3 )
                                {
                                    BB_log( BB_LOG_INFO, "Reading '%s' as .qz quantization file",
                                            BirdBrain::getbase(fD->fileName.c_str()) );
                                    if( selected )
                                    {
                                        selected->core->read_table( (char *)c );
                                    }
                                    else
                                    {
                                        msg_box( "look...", "no point reading a quantization file without a selecting a template for it" );
                                    }
                                    // if you read more than one of these, the last one should win
                                }
                                else
                                {
                                    BB_log( BB_LOG_INFO, "Reading %s as sound file", fD->fileName.c_str() ); 
                                    File * file = new File( c );
                                    if( !file->goodtogo )
                                    {
                                        string msg = "cannot open file: ";
                                        AudioSrcFile * f = (AudioSrcFile *)(file->src); 
                                        msg += f->last_error();
                                        msg_box( c, msg.c_str() );     
                                    }
                                    else
                                    {
                                        file->name = BirdBrain::getname( c );
                                        // replace space with _
                                        int space = file->name.find( " ", 0 );
                                        while( space != std::string::npos )
                                        {
                                            file->name[space] = '_';
                                            space = file->name.find( " ", space );
                                        }
                                        Library::instance()->add( file );
                                    }
                                }

                                // next one
                                fD = fD->next;
                                // TODO: delete
                            }
                        }
                        // switch face
                        else if( i >= BT_ANALYSIS_FACE && i <= BT_SEARCH_FACE )
                        {
                            handle_face_button( BT_ANALYSIS_FACE, BT_SEARCH_FACE, i );
                        }
						else if( i >= BT_QUIT && i <= BT_FULLSCREEN )
						{
							handle_ctrl_button( BT_QUIT, BT_FULLSCREEN, i );
						}
                        else
                            load_sliders = true; // perhaps
                    } // if state is up and down is true
                    
                    if( ie->state == ae_input_UP )
                    {
                        // place events on timeline
                        if( i == TL_TIMELINE && timeline != NULL && down != NULL )
                        {
                            // figure out where
                            float where = vr_to_world_x(m_vr,ie->pos[0]);
                            // subtract the left
                            where -= -1.1f;
                            // scale it by the width
                            where /= ui_elements[BT_LEFT]->the_width;
                            // find point in seconds
                            where = ui_elements[BT_LEFT]->slide_0 
                                + where * (ui_elements[BT_LEFT]->slide_1 - ui_elements[BT_LEFT]->slide_0);
                            // must pass in as fraction of duration
                            where = where * BirdBrain::srate() / timeline->duration;

                            // if it's an original or being dragged from a bag or something
                            if( down->orig == down || !m_bboxs[3].in( vr_to_world(m_vr, last_hit_pos) ) )
                            {
                                // make dummy copy
                                UI_Template * dummy = Library::instance()->add( down->core );
                                down->makedummy( dummy );
                                
                                // place dummy template on timeline
                                timeline->place( dummy, where, vr_to_world_y(m_vr,ie->pos[1]) - .5f );
                            }
                            else // move existing dummy
                            {
                                timeline->remove( down );
                                timeline->place( down, where, vr_to_world_y(m_vr,ie->pos[1]) -.5f );
                            }

                            // safe: place a template on timeline
                            //timeline->place( down, where, vr_to_world_y(m_vr,ie->pos[1]) - .5f );
                        }
                    }
                } // if checkid

                // button up
                if( ie->state == ae_input_UP && ui_elements[i]->down )
                    ui_elements[i]->down = FALSE;
            }

            // check bagels
            check_bagel_event( ie, somewhere, hit );
            
#ifdef __TAPS_SCRIPTING_ENABLE__
			// check chui
			check_chui_event( ie, somewhere, hit );
#endif

            // check templates
            bool nulldown = false;
            for( j = 0; j < Library::instance()->size(); j++ )
            {
                // if a template is selected
                if( ie->checkID( Library::instance()->templates[j]->id ) )
                {
                    UI_Template * ui_temp_j = Library::instance()->templates[j]; 
                    somewhere = TRUE; // danger

                    if( ie->state == ae_input_DOWN )
                    {
                        hit = TRUE;
                        last_hit_pos = ie->pos;
                        down = ui_temp_j; 
                        orig_pt = curr_pt = vr_to_world(m_vr,ie->pos);
                        ui_temp_j->down = TRUE;
                    }

                    if( ie->state == ae_input_UP && ui_temp_j->down == TRUE )
                    {
                        // right button: just play
                        if( ie->button == ae_input_RIGHT_BUTTON )
                        {
							UI_Template * me = ui_temp_j->orig;                         
                            if( me->core->playing() )
                            {
                                me->core->stop();
                                BB_log( BB_LOG_INFO, "Stopping template" ); 
                            }
                            else
                            {
								play_template( me );
/*                                // start over, taking parameteres into account 
                                me->core->recompute();
                                if( me->core->type == TT_TIMELINE && me->core != timeline )
                                    ((Timeline *)(me->core))->now_butter = NULL;
                                BB_log( BB_LOG_INFO, "Playing template without selecting" ); 

                                AudioCentral::instance()->bus(2)->play( me->core );
*/                            }
						}
                        // other button: select
                        else 
                        {
                            // first update the backup of the currently selected uitemp (make same as core)
                            if( selected != NULL )
                            {
                                selected->backup->copy_params( *selected->core ); // if backup == core, this should make no difference
                            }
                            // NOW select the newly selected template // safe would be without orig
                            selected = ui_temp_j->orig;
                            if( selected->core->type == TT_TIMELINE ) {
                                if(timeline) 
									timeline->now_butter = NULL;
								timeline = (Timeline *)selected->core;
                                set_duration( timeline->duration ); 
                                set_timeline_bounds();
								timeline->now_butter = ui_elements[BT_NOW];
                            }
                            // non-timelines: nothing more to do
                            // prepare to load sliders
                            load_sliders = true;
                            // end
                        }
                    }
                }

                // button up
                if( ie->state == ae_input_UP && Library::instance()->templates[j]->down )
                {
                    Library::instance()->templates[j]->down = FALSE;
                    // safe: down = NULL;
                    // danger:
                    nulldown = true;
                }
            }

            // background
            if( hit == FALSE )
            {
                if( ie->state == ae_input_DOWN )
                {
                    which = !!(ie->mods & ae_input_CTRL);
                    m_mouse_down = TRUE;
                    last = ie->pos;
                }
                else 
                {
                    m_mouse_down = FALSE;
                }
            }

            // mouse up in background (hopefully)
            if( somewhere == FALSE && ie->state != ae_input_DOWN )
            {
                // remove from timeline
                if( m_bboxs[3].in( vr_to_world(m_vr, last_hit_pos) ) 
                    && timeline != NULL && down != NULL && down->orig != down )
                {
                    timeline->remove( down );
                    down->orig->removedummy( down );
                    if( !Library::instance()->remove( down ) )
                        BB_log( BB_LOG_INFO, "ui_synthesis: couldn't remove down" ); 
                }
                // dealing with a bag
                else if( selected != NULL && selected->core->type == TT_BAG && down != NULL )
                {
                    // remove from bag
                    if( m_bboxs[0].in( vr_to_world(m_vr, last_hit_pos) ) 
                        && !m_bboxs[0].in( vr_to_world(m_vr, ie->pos) ) && down->orig != down )
                    {
                        ((BagTemplate *)(selected->core))->remove( down );
                        down->orig->removedummy( down );
                        if( !Library::instance()->remove( down ) )
                            BB_log( BB_LOG_INFO, "ui_synthesis: couldn't remove down" ); 
                        load_sliders = true;
                    }
                    // place in bag 
                    else if( !m_bboxs[0].in( vr_to_world(m_vr, last_hit_pos) )
                             && m_bboxs[0].in( vr_to_world(m_vr, ie->pos) ) )
                    {
                        BB_log( BB_LOG_INFO, "Placing template in bag" ); 
                        BB_log( BB_LOG_FINEST, "down 0x%x template 0x%x", down, down->core );
                        // make dummy copy
                        UI_Template * dummy = Library::instance()->add( down->core );
                        down->makedummy( dummy );
                        // place dummy in bag
                        BagTemplate * bag = (BagTemplate *)(selected->core);
                        if( bag->insert( dummy ) ) { 
                            BB_log( BB_LOG_FINEST, "dummy 0x%x template 0x%x", dummy, dummy->core );
                            load_sliders = true;
                        }
                        else {
                            // failed, clean up
                            down->orig->removedummy( dummy ); 
                            if( !Library::instance()->remove( dummy ) )
                                BB_log( BB_LOG_INFO, "ui_synthesis: couldn't remove dummy" ); 
                        }
                        // controls added in render_bag_pane
                    } // END of insert template into bag
                } // END of dealing with bag                
            } // END of mouse up in background

            // danger
            if( nulldown ) down = NULL; 
        } // END OF ae_input_MOUSE

        else if( ie->type == ae_input_MOTION )
        {
            t_TAPBOOL nothing = TRUE;

            for( i = SL_PERIODICITY; i <= SL_SEGSIZE; i++ )
            {
                if( ui_elements[i]->down )
                {
                    fix_slider_motion( *ui_elements[i], m_vr, ie->pos, -.75f, g_slider_height_mini, true );
                    nothing = FALSE;
                }
            }

            for( i = BT_LEFT; i <= BT_RIGHT; i++ )
            {
                if( ui_elements[i]->down )
                {
                    ui_elements[i]->slide += ( vr_to_world_x(m_vr,ie->pos[0]) + 1.1) 
                                            / ui_elements[BT_LEFT]->the_width - ui_elements[i]->slide_last;
                    if( ui_elements[BT_RIGHT]->slide - ui_elements[BT_LEFT]->slide <= .004f )
                    {
                        if( i == BT_RIGHT )
                            ui_elements[i]->slide = ui_elements[i]->slide_last = ui_elements[BT_LEFT]->slide + .005f;
                        else if( i == BT_LEFT )
                            ui_elements[i]->slide = ui_elements[i]->slide_last = ui_elements[BT_RIGHT]->slide - .005f;
                    }
                    else
                    {
                        ui_elements[i]->slide_last = (vr_to_world_x(m_vr,ie->pos[0]) + 1.1) / ui_elements[BT_LEFT]->the_width;
                        if( ui_elements[i]->slide > 1.0 ) ui_elements[i]->slide = 1.0; 
                        if( ui_elements[i]->slide_last > 1.0 ) ui_elements[i]->slide_last = 1.0;
                        if( ui_elements[i]->slide < 0.0 ) ui_elements[i]->slide = 0.0; 
                        if( ui_elements[i]->slide_last < 0.0 ) ui_elements[i]->slide_last = 0;
                        ui_elements[i]->slide_locally = false;
                    }

                    // update timeline start and stop times
                    if( timeline )
                    {
                        timeline->starttime = ui_elements[BT_LEFT]->fvalue() * BirdBrain::srate();
                        timeline->stoptime = ui_elements[BT_RIGHT]->fvalue() * BirdBrain::srate();
                    }

                    nothing = FALSE;
                }
            }

            i = SL_PAN;
            if( ui_elements[i]->down )
            {
                fix_slider_motion( *ui_elements[i], m_vr, ie->pos, .7f, g_slider_height_mini, false );
                nothing = FALSE;
            }

            i = SL_RAND_LOOP;
            if( ui_elements[i]->down )
            {
                fix_slider_motion( *ui_elements[i], m_vr, ie->pos, .1f, g_slider_height_mini, false );              
                nothing = FALSE;
            }

            i = SL_TIMELINE_SHIFT;
            if( ui_elements[i]->down )
            {
                fix_slider_motion( *ui_elements[i], m_vr, ie->pos, -1.1f, g_slider_height_micro, false );
                set_timeline_bounds();
                nothing = FALSE;
            }

            i = SL_TIMELINE_ZOOM;
            if( ui_elements[i]->down )
            {
                fix_slider_motion( *ui_elements[i], m_vr, ie->pos, -.7f, g_slider_height_micro, false );
                set_timeline_bounds();
                nothing = FALSE;
            }

            i = SL_TIMELINE_DURATION;
            if( ui_elements[i]->down )
            {
                fix_slider_motion( *ui_elements[i], m_vr, ie->pos, .25f, g_slider_height, true );
                if( timeline )
                {
                    t_TAPTIME dur = get_duration();
                    timeline->duration = dur;
                    if( timeline->starttime > timeline->duration )
                        timeline->starttime = 0; 
                    if( timeline->stoptime > timeline->duration )
                        timeline->stoptime = timeline->duration; 
                }
                ui_elements[SL_TIMELINE_SHIFT]->set_slide( 0 );
                ui_elements[SL_TIMELINE_ZOOM]->set_slide( 1 );
                set_timeline_bounds();
                nothing = FALSE;
            }
            
            // bag element controls
            for( k = 0; k < bagels.size(); k++ )
            {
                if( bagels[k]->down )
                {
                    switch( k % BagTemplate::nctrls )
                    {
                    case 1:
                        fix_slider_motion( *bagels[k], m_vr, ie->pos, .5f, g_slider_height_micro, false );
                        break;
                    case 2:
                        fix_slider_motion( *bagels[k], m_vr, ie->pos, .9f, g_slider_height_micro, false );
                        break;
                    default:
                        break;
                    }
                    nothing = FALSE;
                }
            }

#ifdef __TAPS_SCRIPTING_ENABLE__
			// chui element controls
			if( selected && selected->core->type == TT_SCRIPT )
			{
				Scriptor * script = (Scriptor *)selected->core;
				for( k = 0; k < script->num_elements(); k++ )
				{
					UI_Element * chuiel = script->element_at( k );
					if( chuiel->down )
					{
						t_TAPSINGLE length; 
						if( chuiel->element_length == UI_Element::UI_NORMAL ) length = g_slider_height;
						else if( chuiel->element_length == UI_Element::UI_MINI ) length = g_slider_height_mini;
						else if( chuiel->element_length == UI_Element::UI_MICRO ) length = g_slider_height_micro;
						else length = -1;
						t_TAPBOOL vertical = chuiel->element_orientation == UI_Element::UI_VERTICAL;
						if( length > 0 )
							fix_slider_motion( *chuiel, m_vr, ie->pos, chuiel->offset, length, vertical );
						chuiel->event->queue_broadcast();
						nothing = FALSE;
					}
				}
			}
#endif

            // highlighting...
            if( nothing ) {
                m_cur_bbox = -1; 
                for( int b = 0; b < m_nbboxs; b++ )
                    if( m_bboxs[b].in( vr_to_world(m_vr, ie->pos ) ) ) {
                        m_cur_bbox = b;
                    }
            }


            // save sliders and flippers too!?
            if( selected && !load_sliders )
            {
                assert( selected->core );
                selected->core->set_param( Template::GAIN, ui_elements[SL_GAIN]->fvalue() );
                selected->core->set_param( Template::PAN, ui_elements[SL_PAN]->fvalue() );
                selected->core->set_param( Template::FREQ_WARP, ui_elements[SL_FREQ_WARP]->fvalue() );
                selected->core->set_param( Template::TIME_STRETCH, ui_elements[SL_TIME_STRETCH]->fvalue() );
                selected->core->set_param( Template::PERIODICITY, ui_elements[SL_PERIODICITY]->fvalue() );
                selected->core->set_param( Template::DENSITY, ui_elements[SL_DENSITY]->fvalue() );
                selected->core->set_param( Template::RANDOM, ui_elements[SL_RAND_LOOP]->fvalue() );
                //selected->core->set_param( Template::LOOP_ME, ui_elements[FL_LOOP_ME]->fvalue() ); 

                // Treesynth sliders
                selected->core->set_param( Template::PERCENTAGE, ui_elements[SL_PERCENTAGE]->fvalue() );
                selected->core->set_param( Template::K, ui_elements[SL_K]->fvalue() );
                selected->core->set_param( Template::STARTLEVEL, ui_elements[SL_STARTLEVEL]->fvalue() );
                selected->core->set_param( Template::STOPLEVEL, ui_elements[SL_STOPLEVEL]->fvalue() );
                selected->core->set_param( Template::TOTAL_LEVELS, ui_elements[SL_TOTAL_LEVELS]->fvalue() );
				//selected->core->set_param( Template::ANCFIRST, ui_elements[FL_ANCFIRST]->fvalue() );
                //selected->core->set_param( Template::RANDFLIP, ui_elements[FL_RANDFLIP]->fvalue() );
				// Olar sliders
				selected->core->set_param( Template::OLARANDOMNESS, ui_elements[SL_RAND_OLAR]->fvalue() );
				selected->core->set_param( Template::SEGSIZE, ui_elements[SL_SEGSIZE]->fvalue() );
				selected->core->set_param( Template::MINDIST, ui_elements[SL_MINDIST]->fvalue() );
				//selected->core->set_param( Template::SCALE_AMP, ui_elements[FL_SCALEAMP]->fvalue() );

                // bag element control sliders
                for( k = 0; k < bagels.size(); k++ )
                    if( k % BagTemplate::nctrls > 0 )
                        selected->core->set_param( Template::BAG_OFFSET + k, bagels[k]->fvalue() ); 

//              load_sliders = true; // perhaps
            }

            // ...
            if( down )
            {
                prev_pt = curr_pt;
                curr_pt = vr_to_world(m_vr,ie->pos);
            }
        } // end of ae_input_MOTION 

        else if( ie->type == ae_input_KEY )
        {
            switch( ie->key )
            {
                case 8: // backspace
                    if( selected != NULL )
                    {
                        if( selected->core->playing() ) {
                            BB_log( BB_LOG_INFO, "Stopping template in preparation for deletion" );
                            selected->core->stop(); 
                            while( selected->core->playing() )
                                usleep( 5000 );
                        }                               
                        BB_log( BB_LOG_INFO, "Deleting template %s (0x%x)...", selected->core->name.c_str(), selected );
                        // special case for timeline, as usual (also bag)
                        // a timeline can't be selected without 'timeline' var being reset
                        if( selected->core == timeline || selected->core->type == TT_BAG ) 
                        {
                            if( selected->dummies.empty() ) // it can be deleted
                            {
                                if( selected->core == timeline ) 
                                    timeline = NULL;
                                clear_template_dummies( selected->core ); // remove ui temps on timeline or bag
                                Library::instance()->remove( selected ); // remove ui template representing the timeline
                                selected = NULL;
                            }
                            else
                                msg_box( "cannot delete template", "this template is being used in another template" );
                        }
                        // other templates types
                        else
                        {
                            if( !Library::instance()->remove( selected ) ) // not good, for sure
                                msg_box( "cannot delete template", "this template is being used in another template" );
                            else
                            {
                                selected = NULL;
                            }
                        }
                    }
                    break;
                
                case ' ':
                    if( selected != NULL )
                        play_template( selected );
                    break;
                case 'h':
                case 'H':
                    m_highlight = !m_highlight;
                    BB_log( BB_LOG_INFO, "Highlighting %s", m_highlight ? "ON" : "OFF" );
                    break;
                case 'r': 
                case 'R':
                    g_show_slider_range = !g_show_slider_range; 
                    BB_log( BB_LOG_INFO, "%showing ranges of sliders", g_show_slider_range ? "S" : "Not s" ); // :)
                    break;
                case 'q':
                case 'z':
                    g_show_qz = !g_show_qz;
                    BB_log( BB_LOG_INFO, "%showing quantization markers", g_show_qz ? "S" : "Not s" ); 
                    break;
                case 'N':
                {
                    BagTemplate * b = new BagTemplate(); 
                    // set the name
                    b->name = "MixedBag";
                    // add to library and select it
                    selected = Library::instance()->add( b );
                    // load sliders?
                    load_sliders = true;
                    break;
                }
                case '0':
                case  27: // escape
                    m_vr.setDest( *m_vrs[0] );
                break;
                case '1':
                case '2':
                case '3':
                case '4':
                    {
                        long index = ie->key - '0';
                        m_vr.setDest( m_vr.m_dest == m_vrs[index] ? *m_vrs[0] : *m_vrs[index] );
                    }
                    break;
                default: 
                    break;
                }
        } // end of ae_input_KEY
    }

    return AudicleFace::on_event( event );
}


// set bounds on timeline area, taking into account shift, zoom, duration
void UISynthesis::set_timeline_bounds()
{
    double left, right, view_range; 
    t_TAPTIME dur = get_duration();
    float now = ui_elements[BT_NOW]->fvalue();
    // actual duration
    left = 0; 
    right = dur / BirdBrain::srate();
    // zoom
    float zoom = ui_elements[SL_TIMELINE_ZOOM]->fvalue();
    view_range = (right - left) / zoom;
    // shift
    float shift = ui_elements[SL_TIMELINE_SHIFT]->fvalue();
    left = shift * (right - view_range);
    right = left + view_range;

    // set bounds
    for( int bt = BT_LEFT; bt <= BT_RIGHT; bt++ )
        ui_elements[bt]->set_bounds( left, right );
    // set start/stop (to avoid scrolling)
    if( timeline )
    {
        // set butter boundaries so they don't go off the timeline
        // left butter
//      if( left * BirdBrain::srate() > timeline->starttime )
//          ui_elements[BT_LEFT]->set_slide( left );
//      else if( right * BirdBrain::srate() < timeline->starttime )
//          ui_elements[BT_LEFT]->set_slide( right - .005f*(view_range) );
//      else
            ui_elements[BT_LEFT]->set_slide( timeline->starttime / BirdBrain::srate() );
        // right butter
//      if( left * BirdBrain::srate() > timeline->stoptime )
//          ui_elements[BT_RIGHT]->set_slide( left + .005f*(view_range) );
//      else if( right * BirdBrain::srate() < timeline->stoptime )
//          ui_elements[BT_RIGHT]->set_slide( right );
//      else
            ui_elements[BT_RIGHT]->set_slide( timeline->stoptime / BirdBrain::srate() );
        // now butter
        ui_elements[BT_NOW]->set_slide( now );
    }
}


void UISynthesis::play_template( UI_Template * playme )
{
	assert( playme->core != NULL );
    if( playme->core->playing() )
    {
        playme->core->stop();
        while( playme->core->playing() )
            usleep( 5000 );
    }
    // start over, taking parameters into account
    playme->core->recompute();
    if( playme->core->type == TT_TIMELINE )
	{
		if( playme->core != timeline )
			((Timeline *)(playme->core))->now_butter = NULL;
		else
	        ((Timeline *)(playme->core))->now_butter = ui_elements[BT_NOW];
    }
	// determine bus number
	// (test)
	// playme->core->mybus = rand() % 8; 
	// (end test)
	int busno = playme->core->mybus;
	if( busno < 0 || busno > AudioCentral::instance()->num_bus() )
		busno = 2;
	// play
//	fprintf( stdout, "Playing %x (%s) on bus %i\n", playme->core, playme->core->name.c_str(), busno );
    //AudioCentral::instance()->bus(busno)->play( playme->core );
	playme->core->play( AudioCentral::instance()->bus(busno) );
}


// add more bag element controls if needed
void UISynthesis::make_bagels( UI_Template * ui_temp )
{
    if( ui_temp->core->type != TT_BAG )
        return;
    BagTemplate * bag = (BagTemplate *)(ui_temp->core); 

    while( bagels.size() < bag->marbles.size() * BagTemplate::nctrls )
    {
        for( int bct = 0; bct < BagTemplate::nctrls; bct++ ) {
            bagels.push_back( new UI_Element );                         
            UI_Element *ctrl = bagels.back();
            switch( bct % BagTemplate::nctrls )
            {
            case 0:
                ctrl->name = "PLAY ONCE";
                ctrl->id = IDManager::instance()->getPickID();
                break;
            case 1: 
                ctrl->name = "LIKELIHOOD";
                ctrl->id = IDManager::instance()->getPickID();
                ctrl->set_bounds( 0, 20, true );
                ctrl->set_slide( bag->marbles.back().likelihood ); 
                break;
            case 2:
                ctrl->name = "RANDOMNESS"; 
                ctrl->id = IDManager::instance()->getPickID();
                ctrl->set_bounds( 1.0, 3.0 ); 
                ctrl->set_slide( bag->marbles.back().random ); 
                break;
            default:
                break;
            }
        }
    }
}


// check if a bagel element was clicked / moved and act accordingly
void UISynthesis::check_bagel_event( InputEvent * ie, t_TAPBOOL & somewhere, t_TAPBOOL & hit )
{
	for( int k = 0; k < bagels.size(); k++ )
    {
        // if a bag element control is selected
        if( ie->checkID( bagels[k]->id ) )
        {
            somewhere = TRUE; 
            if( ie->state == ae_input_DOWN )
            {
                hit = TRUE; 
                last_hit_pos = ie->pos; 
                bagels[k]->down = TRUE;

                // sliders
                if( k % BagTemplate::nctrls == 1 )
                    bagels[k]->slide_last = ( vr_to_world_x(m_vr,ie->pos[0]) - .5 ) / g_slider_height_micro;
                else if( k % BagTemplate::nctrls == 2 )
                    bagels[k]->slide_last = ( vr_to_world_x(m_vr,ie->pos[0]) - .9 ) / g_slider_height_micro;
            }
            if( ie->state == ae_input_UP && bagels[k]->down == TRUE )
            {
                // control flipper here
                if( k % BagTemplate::nctrls == 0 ) 
                {
                    bagels[k]->slide = 1.0f - bagels[k]->slide;
                    if( selected != NULL )
                        selected->core->set_param( Template::BAG_OFFSET + k, bagels[k]->fvalue() );
                }
            }
        }
        // button up
        if( ie->state == ae_input_UP && bagels[k]->down )
            bagels[k]->down = FALSE;
    }
}


#ifdef __TAPS_SCRIPTING_ENABLE__
// check if a chui element was clicked / moved and act accordingly
void UISynthesis::check_chui_event( InputEvent * ie, t_TAPBOOL & somewhere, t_TAPBOOL & hit )
{
	// return if impossible
	if( !ie || !selected || selected->core->type != TT_SCRIPT )
		return;
	Scriptor * script = (Scriptor *)selected->core;

	// go through all script elements
	for( int i = 0; i < script->num_elements(); i++ )
    {
		UI_Element * chuiel = script->element_at(i);
        // get information
		if( chuiel->must_adjust_loc ) chuiel->adjust_loc( m_vrs[2] );
		const Flt * p = chuiel->loc->data();
		t_TAPSINGLE length;
		if( chuiel->element_length == UI_Element::UI_NORMAL )
			length = g_slider_height;
		else if( chuiel->element_length == UI_Element::UI_MINI )
			length = g_slider_height_mini;
		else if( chuiel->element_length == UI_Element::UI_MICRO )
			length = g_slider_height_micro;
		else
			length = -1;

		// MOUSE
		if( ie->type == ae_input_MOUSE )
		{
			// if a chui control is selected
			if( ie->checkID( chuiel->id ) )
			{
				somewhere = TRUE; 
				if( ie->state == ae_input_DOWN )
				{
					hit = TRUE; 
					last_hit_pos = ie->pos; 
					chuiel->down = TRUE;

					// sliders?
					if( chuiel->element_type == UI_Element::UI_SLIDER && chuiel->rel_loc )
					{
						// adjust slide_last
						if( chuiel->element_orientation == UI_Element::UI_HORIZONTAL )
							chuiel->slide_last = ( vr_to_world_x(m_vr,ie->pos[0]) - p[0] ) / length;
						else if( chuiel->element_orientation == UI_Element::UI_VERTICAL )
							chuiel->slide_last = ( vr_to_world_y(m_vr,ie->pos[1]) - p[1] ) / length;
					}
				}
				if( ie->state == ae_input_UP && chuiel->down == TRUE )
				{
					// control button / flipper here
					if( chuiel->element_type == UI_Element::UI_BUTTON || chuiel->element_type == UI_Element::UI_FLIPPER ) 
					{
						chuiel->slide = 1.0f - chuiel->slide;
						chuiel->event->queue_broadcast();  
					}
				}
			}
			// button up
			if( ie->state == ae_input_UP && chuiel->down )
				chuiel->down = FALSE;
		}
		
		// MOTION
/*		else if( ie->type == ae_input_MOTION )
		{
			if( chuiel->element_type == UI_Element::UI_SLIDER ) 
			{
				fix_slider_motion( *chuiel, m_vr, ie->pos, chuiel->offset, length, 
							chuiel->element_orientation == UI_Element::UI_VERTICAL );
			}
		}
*/	}
}
#endif

// 'helper'; not sure where it belonged
void clear_timeline_dummies( Timeline * tim )
{
    for( int j = 0; j < tim->instances.size(); j++ )
    {
        UI_Template * dum = tim->instances[j].ui_temp;
        dum->orig->removedummy( dum );
        Library::instance()->remove( dum ); // returns bool
    }
}

// 'helper'; not sure where it belonged (another one)
void create_timeline_dummies( Timeline * tim )
{
    // dummify (make dummy copies of) instances
    for( int i = 0; i < tim->instances.size(); i++ )
    {
        // get existing instance (from tim)
        UI_Template * ui_temp = tim->instances[i].ui_temp;  
        // create another ui template with the same temp, by adding to library
        // call this new ui template dummy
        UI_Template * dummy = Library::instance()->add( ui_temp->core );
        // make dummy a dummy of ui_temp
        ui_temp->makedummy( dummy );
        // replace tim's this instance's ui_temp with dummy
        tim->instances[i].ui_temp = dummy;
    }
}

// 'helper'; not sure where it belonged (and another one)
void clear_bag_dummies( BagTemplate * bag )
{
    for( int j = 0; j < bag->marbles.size(); j++ )
    {
        UI_Template * dum = bag->marbles[j].ui_temp;
        dum->orig->removedummy( dum );
        Library::instance()->remove( dum ); // returns bool
    }
}

// 'helper'; not sure where it belonged (one more)
void create_bag_dummies( BagTemplate * bag )
{
    // dummify (make dummy copies of) marbles
    for( int i = 0; i < bag->marbles.size(); i++ )
    {
        // get existing instance (from tim)
        UI_Template * ui_temp = bag->marbles[i].ui_temp;  
        // create another ui template with the same temp, by adding to library
        // call this new ui template dummy
        UI_Template * dummy = Library::instance()->add( ui_temp->core );
        // make dummy a dummy of ui_temp
        ui_temp->makedummy( dummy );
        // replace bag's this marble's ui_temp with dummy
        bag->marbles[i].ui_temp = dummy;
    }
}


// slightly more generic 'helper'
void clear_template_dummies( Template * temp )
{
    switch( temp->type )
    {
    case TT_TIMELINE:
        clear_timeline_dummies( (Timeline *)temp );
        break;
    case TT_BAG:
        clear_bag_dummies( (BagTemplate *)temp );
        break;
    default:
        break;
    }
}

// slightly more generic 'helper'
void create_template_dummies( Template * temp )
{
    switch( temp->type )
    {
    case TT_TIMELINE:
        create_timeline_dummies( (Timeline *)temp );
        break;
    case TT_BAG:
        create_bag_dummies( (BagTemplate *)temp ); 
        break;
    default:
        break;
    }
}


// load_file
Template * synth_load_file( const std::string & path )
{
    // BB_log( BB_LOG_INFO, "%s", path.c_str() );
    const char * c = path.c_str();
        
    if( path.rfind( ".tap" ) == path.length()-4 || path.rfind( ".xml" ) == path.length()-4 )
    {
        BB_log( BB_LOG_INFO, "Reading '%s' as template file", path.c_str() );
        TemplateReader r;
        Template * tmp = NULL;
        if( r.open( (char *)c ) )
        {
            tmp = r.read_template();
            r.close();
        }

        return tmp;
    }
    else if( path.rfind( ".ck" ) == path.length()-3 )
    {
	#ifdef __TAPS_SCRIPTING_ENABLE__ 
        BB_log( BB_LOG_INFO, "Reading '%s' as .ck script file", path.c_str() );
        Scriptor * script = ScriptCentral::engine()->compile( path );
        if( script != NULL )
        {
            script->name = BirdBrain::getname(path.c_str());
            // replace space with _
            int space = script->name.find( " ", 0 );
            while( space != std::string::npos )
            {
                script->name[space] = '_';
                space = script->name.find( " ", space );
            }
        }
        return script;
	#else
        msg_box( "ding!", "scripting not enabled during compilation" );
        return NULL;
	#endif
    }
    else
    {
        BB_log( BB_LOG_INFO, "Reading %s as sound file", path.c_str() ); 
        File * file = new File( c );
        // check
        if( file->goodtogo )
        {
            file->name = BirdBrain::getname( c );
            // replace space with _
            int space = file->name.find( " ", 0 );
            while( space != std::string::npos )
            {
                file->name[space] = '_';
                space = file->name.find( " ", space );
            }
        }
        else
            SAFE_DELETE( file );
        // Library::instance()->add( file );
        return file;
    }
}

// load file into library by name
t_TAPBOOL synth_load_template( const std::string & name, t_TAPBOOL show )
{
    // check
    vector<UI_Template *> temps = Library::instance()->search_name_exact( name );
    // use first
    if( temps.size() > 0 )
    {
        BB_log( BB_LOG_INFO, "load template found '%s', moving on...", name.c_str() );
        return TRUE;
    }

    // load
	string names[] = {name + ".tap", name + ".xml"};
	t_TAPBOOL good = FALSE;
	for(int i = 0; i < 2; i++) { // 2 is length of names
		string filename = names[i];
        BB_log( BB_LOG_INFO, "Reading '%s' as a template file", filename.c_str() );
        TemplateReader r;
        if( r.open( (char *)filename.c_str() ) )
        {
            Template * tmp = r.read_template();
            r.close();
            if( tmp != NULL && !Library::instance()->hasID( tmp ) )
            {
                UI_Template * ui_temp = Library::instance()->add( tmp );
				ui_temp->show = show;
				good = TRUE;
            }
            else if( tmp != NULL )
            {
                BB_log( BB_LOG_INFO, "'%s' already in library", name.c_str() );
				good = TRUE;
            }
            else
            {
                BB_log( BB_LOG_INFO, "cannot read '%s'...", filename.c_str() );
                good = good || FALSE;
            }
        }
        else
        {
            BB_log( BB_LOG_INFO, "cannot open '%s' for reading!", filename.c_str() );
            good = good || FALSE;
        }
    }
	
    return good;
}

// copy template in library
t_TAPBOOL synth_copy_template( UI_Template * ui_temp, t_TAPINT copies )
{
    Template * temp = NULL;

    BB_log( BB_LOG_INFO, "making ui template copies: %s x %d...", ui_temp->name.c_str(), copies );

    for( t_TAPINT i = 0; i < copies; i++ )
    {
        // copy
        temp = ui_temp->core->copy(); // but with new template id

        // timeline/bag special treatment
        if( temp->type == TT_TIMELINE || temp->type == TT_BAG )
            create_template_dummies( temp ); 

        Library::instance()->add( temp );
    }

    return TRUE;
}
