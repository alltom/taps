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
// name: ui_group.cpp
// desc: track group editing face
//
// authors: Ananya Misra (amisra@cs.princeton.edu)
//          Ge Wang (gewang@cs.princeton.edu)
//          Perry R. Cook (prc@cs.princeton.edu)
//          Tom Lieber (lieber@princeton.edu)
// date: Fall 2007
//-----------------------------------------------------------------------------
#include <string>

#include "ui_group.h"
#include "ui_element.h"
#include "audicle_utils.h"
#include "ui_library.h"
#include "audicle_gfx.h"
#include "audicle_geometry.h"
#include "audicle.h"
#include "taps_birdbrain.h"
#include "ui_filesave.h"

using namespace std;


// which bus should be used for playback on this face
#define GROUP_BUS 3

#define INITIAL_TIMELINE_LENGTH (441000 )

// handle of the (singleton) instance of this face
UIGroup * g_group_face;

struct GTrackInfo {
	UI_Track *track;
	bool selected;
	t_TAPTIME offset;
};

// names for all the widgets on this face
enum UI_ELMENTS
{
	// face switchers
	BT_ANALYSIS_FACE, BT_SYNTHESIS_FACE, BT_GROUP_FACE,
	BT_CONTROL_FACE,   BT_SEARCH_FACE,
	// control
	BT_QUIT, BT_FULLSCREEN, 

	// timeline control
	BT_LEFT, BT_NOW, BT_RIGHT,

	BT_TRACKBOX,

	BT_PLAY,
	BT_STOP,

	BT_SAVE,
	BT_INVERT, BT_ALL,
	BT_UNDO,

	BT_VIBRUTTON,
	BT_WARP,
	BT_QUANTIZE,
	BT_GAIN,
	BT_CUT,
	BT_POINT,
	BT_HARM,
	BT_MOVE,

	BT_SELECT_HARMONICS,
	FL_SHOW_ALL_TRACKS, // show all tracks of selected templates, even white ones

	SL_TIME_STRETCH,
	SL_FREQ_WARP,
	SL_FUDGE,
	SL_GAIN_AMOUNT,
	SL_GAIN_TIME,
	SL_GAIN_FREQ,
	SL_PERIODIC_VIBE,
	SL_RANDOM_VIBE,
	SL_FREQ_VIBE,
	BT_QUANTIZE_ACTION,
	FL_MOVE_FREQ,
	FL_MOVE_TIME,
	FL_CUT_ONE,
	FL_CUT_ALL,

	// the floaty, sphere thing (not really a button)
	BT_THING,

	// keep this as last
	NUM_UI_ELEMENTS
};

// names of ui elements
static char * ui_str[] = {
	"analysis", "synthesis", "group",
	"control", "search",
	"exit", "fullscreen",

	"left", "now", "right",

	"tracks",

	"play",
	"stop",

	"save",
	"invert", "all",
	"revert",

	"vibrato",
	"warp",
	"quantize",
	"gain",
	"cut",
	"edit-point",
	"harmonics++",
	"move",

	"harmonics",
	"show-all",

	"time-stretch",
	"freq-warp",
	"fudge amount",
	"gain amount",
	"time range (s)",
	"frequency range (Hz)",
	"periodic-vibrato",
	"random-vibrato",
	"vibrato-frequency",
	"quantize pitch",
	"frequency",
	"time",
	"single-track",
	"line",

	"thing",
	"trackbox"
};


//-----------------------------------------------------------------------------
// LOCAL FUNCTIONS
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// verify that a track is not goofy (TODO: better description)
static bool check_track( Track * test, int template_index, int track_index ) {
	// check the track
	if( test->history.size() > 0 ) {
		if( test->start != test->history[0].time
			|| test->end != test->history.back().time
			|| test->start == test->end && test->history.size() > 1 ) {
			BB_log( BB_LOG_WARNING,
				"track %d:%d : start = %f, history[0] = %f, history.size() = %d, end = %f, history.back() = %f",
				template_index, track_index, test->start, test->history[0].time, test->history.size(),
				test->end, test->history.back().time );
			return FALSE;
		}
	}
	else {
		if( test->start != test->end ) {
			BB_log( BB_LOG_WARNING,
				"track %d-%d : start = %f, history.size() = %d, end = %f",
				template_index, track_index, test->start, test->history.size(), test->end );
			return FALSE;
		}
	}
	return TRUE;
}



//-----------------------------------------------------------------------------
// UIGroup FUNCTIONS
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
UIGroup::UIGroup() : AudicleFace(),
	trackbox_left(-1.2), trackbox_top(0.8), trackbox_width(1.9), trackbox_height(1.15),
	thingbox_left(0.7), thingbox_top(-0.4), thingbox_width(0.6), thingbox_height(0.6)
{
	g_group_face = this;

	if( !this->init( ) )
	{
		fprintf( stderr, "[audicle]: cannot start face...\n" );
		return;
	}
}

//-----------------------------------------------------------------------------
UIGroup::~UIGroup( )
{
	// TODO: why not move these to UIGroup::destroy?
	int i;
	for( i = 0; i < NUM_UI_ELEMENTS; i++ )
		SAFE_DELETE( ui_elements[i] );

	SAFE_DELETE_ARRAY( ui_elements );

	if( ui_temps )
	{
		for( i = 0; i < ui_temps->size(); i++ )
			SAFE_DELETE( (*ui_temps)[i] );

		SAFE_DELETE( ui_temps );
		SAFE_DELETE( templ_selected );
	}
}

//-----------------------------------------------------------------------------
t_TAPBOOL UIGroup::init( )
{
	if( !AudicleFace::init() )
		return FALSE;

	if( !create_widgets() )
		return FALSE;

	if( !setup_widgets() )
		return FALSE;

	// get audio
	m_audio = AudioCentral::instance();

	// whistle for a bus
	m_bus = m_audio->bus(GROUP_BUS);

	// Make a timeline
	timeline = new Timeline(INITIAL_TIMELINE_LENGTH);

	// Make the dummy template for placing on the timeline
	play_template = new Deterministic();
	ui_play_template = new UI_Template();
	ui_play_template->core = play_template;
	timeline->place(ui_play_template, 0, 0);

	// template, track, and track selection lists
	ui_temps = new std::vector<UI_Template*>();
	ui_tracks = new std::vector<std::vector<UI_Track*>* >();
	templ_selected = new std::vector<bool>();
	track_selected = new std::vector<std::vector<bool>*>();
	templ_offsets = new std::vector<t_TAPTIME>();

	// undo stack
	backup_selections = new std::deque<std::set<UI_Track *>* >();

	// set background color
	if(BirdBrain::white_bg()) 
		m_bg[0] = m_bg[1] = m_bg[2] = m_bg[3] = 1.0f;
	else {
		m_bg[0] = .3; m_bg[1] = .5; m_bg[2] = .6; m_bg[3] = 1.0;
	}
	
	// index appended to filenames
	file_save_count = 0;

	// view region: everything
	m_vrs.push_back(new ViewRegionManual(0, 0, -1.0f, 1.0f, FALSE, FALSE));
	// view region: trackbox
	m_vrs.push_back(new ViewRegionManual(
		trackbox_left - 0.08,
		trackbox_left + trackbox_width + 0.08,
		trackbox_top - trackbox_height - 0.13,
		trackbox_top + 0.07,
		TRUE, TRUE));
	// view region: trackbox lower 1/3
	m_vrs.push_back(new ViewRegionManual(
		trackbox_left - 0.08,
		trackbox_left + trackbox_width + 0.08,
		trackbox_top - trackbox_height - 0.13,
		trackbox_top - (2.0 * trackbox_height / 3.0) + 0.07,
		TRUE, TRUE));
	// view region: trackbox lower 2/3
	m_vrs.push_back(new ViewRegionManual(
		trackbox_left - 0.08,
		trackbox_left + trackbox_width + 0.08,
		trackbox_top - trackbox_height - 0.13,
		trackbox_top - (1.0 * trackbox_height / 3.0) + 0.07,
		TRUE, TRUE));
	// view region: library
	// left, right, down, up
	m_vrs.push_back(new ViewRegionManual(
		trackbox_left - 0.08,
		trackbox_left + trackbox_width + 0.08,
		-1.25,
		trackbox_top - trackbox_height + 0.13,
		TRUE, TRUE));

	// set up renderer for the track box
	tb_renderer = new TrackBoxRenderer(
		ui_elements[BT_TRACKBOX],
		trackbox_left, trackbox_top, 0,
		trackbox_width, trackbox_height);
	ui_elements[BT_LEFT]->the_width = trackbox_width;
	ui_elements[BT_LEFT]->the_height = trackbox_height;

	// click mode, cursor mode, pointer mode
	thing_mode = V_SELECT;

	// whether a track's timing has changed (TODO: huh?)
	time_changed = false;

	// maximum number of backups (a backup is made each time a different effect is chosen,
	// or when the track selections change AND some modification of tracks occurs?)
	max_backups = 10;

	// whether to make a new backup before applying effects
	// true if set of selected tracks has been modified since last backup
	// or if one has just reverted (and thus popped the most recent backup version)
	make_new_backup = false;

	// last point for cut line, time/frequency shift (move), and gain coloring
	last_point = NULL;

	// point editor
	pe = new PointEditor(trackbox_left, trackbox_top, trackbox_width, trackbox_height);

	// harmonic fudge amount
	max_fudge = ui_elements[SL_FUDGE]->fvalue();

	return TRUE;
}

//-----------------------------------------------------------------------------
t_TAPBOOL UIGroup::create_widgets( )
{
	int i;

	ui_elements = new UI_Element *[NUM_UI_ELEMENTS];

	// face-switching buttons
	for( i = BT_ANALYSIS_FACE; i <= BT_SEARCH_FACE; i++ )
		ui_elements[i] = new UI_Element;
	// ctrl
	for( i = BT_QUIT; i <= BT_FULLSCREEN; i++ )
		ui_elements[i] = new UI_Element;

	// others
	for( i = BT_LEFT; i <= FL_SHOW_ALL_TRACKS; i++ )
		ui_elements[i] = new UI_Element;

	// specific effect controls
	ui_elements[SL_TIME_STRETCH] = new UI_Exp(10000);
	ui_elements[SL_FREQ_WARP] = new UI_Exp(10000);
	ui_elements[SL_FUDGE] = new UI_Element;
	ui_elements[SL_GAIN_AMOUNT] = new UI_Element;
	ui_elements[SL_GAIN_TIME] = new UI_Element;
	ui_elements[SL_GAIN_FREQ] = new UI_Element;
	ui_elements[SL_PERIODIC_VIBE] = new UI_Exp(100);
	ui_elements[SL_RANDOM_VIBE] = new UI_Exp(100);
	ui_elements[SL_FREQ_VIBE] = new UI_Exp(10);
	ui_elements[BT_QUANTIZE_ACTION] = new UI_Element;
	ui_elements[FL_MOVE_FREQ] = new UI_Element;
	ui_elements[FL_MOVE_TIME] = new UI_Element;
	ui_elements[FL_CUT_ONE] = new UI_Element;
	ui_elements[FL_CUT_ALL] = new UI_Element;

	// the thing
	ui_elements[BT_THING] = new UI_Element;

	// get id for each element,
	// assign their names
	for( i = 0; i < NUM_UI_ELEMENTS; i++ )
	{
		ui_elements[i]->id = IDManager::instance()->getPickID();
		ui_elements[i]->name = ui_str[i];
	}

	return TRUE;
}

//-----------------------------------------------------------------------------
t_TAPBOOL UIGroup::setup_widgets( )
{
	int i;
	Flt * pt;

	ui_elements[FL_SHOW_ALL_TRACKS]->slide = 0.75f; // show all tracks by default

	for( i = BT_VIBRUTTON; i <= BT_MOVE; i++ ) {
		ui_elements[i]->size_up = 0.04f;
		ui_elements[i]->font_size = 0.85f;
	}

	ui_elements[SL_TIME_STRETCH]->set_bounds(.01f, 100.0f);
	ui_elements[SL_TIME_STRETCH]->set_slide(1);
	ui_elements[SL_FREQ_WARP]->set_bounds(.01f, 100.0f);
	ui_elements[SL_FREQ_WARP]->set_slide(1);
	ui_elements[SL_PERIODIC_VIBE]->set_bounds(0.0f, 10.0f);
	ui_elements[SL_PERIODIC_VIBE]->set_slide(0);
	ui_elements[SL_RANDOM_VIBE]->set_bounds(0.0f, 10.0f);
	ui_elements[SL_PERIODIC_VIBE]->set_slide(0);
	ui_elements[SL_FREQ_VIBE]->set_bounds(0.0f, 30.0f);
	ui_elements[SL_FREQ_VIBE]->set_slide(0.0f);
	ui_elements[FL_MOVE_FREQ]->slide = 1.0f;
	ui_elements[FL_CUT_ONE]->slide = 1.0f;
	ui_elements[SL_FUDGE]->set_bounds(0.0f, 1.0f);
	ui_elements[SL_FUDGE]->set_slide(0.2f);
	ui_elements[SL_GAIN_AMOUNT]->set_bounds(0.5f, 1.5f);
	ui_elements[SL_GAIN_AMOUNT]->set_slide(1.3f);
	ui_elements[SL_GAIN_TIME]->set_bounds(0.0f, 1.0f);
	ui_elements[SL_GAIN_TIME]->set_slide(0.1f);
	ui_elements[SL_GAIN_FREQ]->set_bounds(0, BirdBrain::srate() / 16, true); 
	ui_elements[SL_GAIN_FREQ]->set_slide(100);

	// warp controls
	for( i = SL_TIME_STRETCH; i <= SL_FREQ_VIBE; i++) {
		if(!ui_elements[i]->loc)
			ui_elements[i]->loc = new Point3D();
		ui_elements[i]->element_type = UI_Element::UI_SLIDER;
		ui_elements[i]->element_length = UI_Element::UI_NORMAL;
		ui_elements[i]->element_orientation = UI_Element::UI_HORIZONTAL;
	}
	pt = ui_elements[SL_FREQ_WARP]->loc->pdata();
	pt[0] = thingbox_left + 0.05;
	pt[1] = thingbox_top - 0.15;
	pt[2] = 0.0;
	pt = ui_elements[SL_TIME_STRETCH]->loc->pdata();
	pt[0] = thingbox_left + 0.05;
	pt[1] = thingbox_top - 0.3;
	pt[2] = 0.0;

	pt = ui_elements[SL_FUDGE]->loc->pdata();
	pt[0] = thingbox_left + 0.05;
	pt[1] = thingbox_top - 0.15;
	pt[2] = 0.0;

	pt = ui_elements[SL_GAIN_AMOUNT]->loc->pdata();
	pt[0] = thingbox_left + 0.05;
	pt[1] = thingbox_top - 0.1;
	pt[2] = 0.0;
	pt = ui_elements[SL_GAIN_TIME]->loc->pdata();
	pt[0] = thingbox_left + 0.05;
	pt[1] = thingbox_top - 0.25; 
	pt[2] = 0.0;
	pt = ui_elements[SL_GAIN_FREQ]->loc->pdata();
	pt[0] = thingbox_left + 0.05;
	pt[1] = thingbox_top - 0.4;
	pt[2] = 0.0;

	pt = ui_elements[SL_PERIODIC_VIBE]->loc->pdata();
	pt[0] = thingbox_left + 0.05;
	pt[1] = thingbox_top - 0.1;
	pt[2] = 0.0;
	pt = ui_elements[SL_RANDOM_VIBE]->loc->pdata();
	pt[0] = thingbox_left + 0.05;
	pt[1] = thingbox_top - 0.25;
	pt[2] = 0.0;
	pt = ui_elements[SL_FREQ_VIBE]->loc->pdata();
	pt[0] = thingbox_left + 0.05;
	pt[1] = thingbox_top - 0.4;
	pt[2] = 0.0;

	// playback sliders
	for( i = BT_LEFT; i <= BT_RIGHT; i++ )
	{
		ui_elements[i]->slide_int = false;
		ui_elements[i]->size_up = .03f;
		ui_elements[i]->font_size = .7f;
	}

	return TRUE;
}

//-----------------------------------------------------------------------------
t_TAPBOOL UIGroup::destroy( )
{
	this->on_deactivate( 0.0 );
	m_id = Audicle::NO_FACE; // creepy
	m_state = INACTIVE;

	for( int i = 0; i < NUM_UI_ELEMENTS; i++ )
		SAFE_DELETE( ui_elements[i] );

	if( ui_temps )
	{
		for( int j = 0; j < ui_temps->size(); j++ )
			SAFE_DELETE( (*ui_temps)[j] );
		SAFE_DELETE( ui_temps );
		SAFE_DELETE( templ_selected );
	}

	if(ui_tracks)
	{
		for( int k = 0; k < ui_tracks->size(); k++ ) {
			for( int m = 0; m < (*ui_tracks)[k]->size(); m++ ) {
				// SAFE_DELETE( (*(*ui_tracks)[k])[m]->backup );
				while( !(*(*ui_tracks)[k])[m]->backups->empty() ) {
					Track * t = (*(*ui_tracks)[k])[m]->backups->front();
					SAFE_DELETE(t);
					(*(*ui_tracks)[k])[m]->backups->pop_front();
				}
				SAFE_DELETE( (*(*ui_tracks)[k])[m]->backups );
			}
		}
		SAFE_DELETE( ui_tracks );
		SAFE_DELETE( track_selected );
		SAFE_DELETE( templ_offsets );
	}

	while(!backup_selections->empty()) {
		std::set<UI_Track *> * s = backup_selections->front();
		SAFE_DELETE(s);
		backup_selections->pop_front();
	}
	SAFE_DELETE(backup_selections);

	if( pe )
		SAFE_DELETE( pe );

	SAFE_DELETE( tb_renderer );

	return TRUE;
}

//-----------------------------------------------------------------------------
t_TAPUINT UIGroup::render( void * data )
{
	// face + ctrl buttons
	draw_face_buttons( BT_ANALYSIS_FACE, BT_SEARCH_FACE );
	draw_ctrl_buttons( BT_QUIT, BT_FULLSCREEN );

	// playback controls
	draw_button( *ui_elements[BT_PLAY], 0.9, 0.8, 0.0, 0.5, 1.0, 0.5, IMG_PLAY );
	draw_button( *ui_elements[BT_STOP], 1.1, 0.8, 0.0, 0.5, 1.0, 0.5, IMG_STOP );

	draw_button( *ui_elements[BT_SAVE], 0.9, 0.6, 0.0, 1.0, 1.0, 0.5, IMG_SAVE );
	draw_button( *ui_elements[BT_UNDO], 1.1, 0.6, 0.0, 1.0, 1.0, 0.5, IMG_REV );

	draw_button( *ui_elements[BT_ALL], 0.9, 0.4, 0.0, 0.5, 1.0, 1.0, IMG_ALL );
	draw_button( *ui_elements[BT_INVERT], 1.1, 0.4, 0.0, 0.5, 1.0, 1.0, IMG_NEWT );

	draw_flipper( *ui_elements[BT_SELECT_HARMONICS], 0.9, 0.16, 0.0 );
	draw_flipper( *ui_elements[FL_SHOW_ALL_TRACKS], 1.1, 0.16, 0.0 );

	draw_mode_buttons();

	// Draw the track box
	draw_track_box( *ui_elements[BT_TRACKBOX],
		trackbox_left, trackbox_top, 0.0, trackbox_width, trackbox_height );

	// library
	draw_group_library( *templ_selected, *ui_temps,
		-1.2, -0.9, 0.8, -0.65 ); // function in ui_element.h/cpp

	// the thing
	draw_thing_box();

	return 0;
}

//-----------------------------------------------------------------------------
void UIGroup::draw_mode_button(UI_Element & e, float x, float y, float z, bool active) {
	if(active)
		draw_button( e, x, y, z, 0.6, 0.3, 0.3, IMG_CENTER);
	else
		draw_button( e, x, y, z, 1.0, 0.5, 0.5, IMG_NONE);
}

//-----------------------------------------------------------------------------
// draw the effect buttons colored according to what the current effect/mode is
void UIGroup::draw_mode_buttons() {
	draw_mode_button( *ui_elements[BT_VIBRUTTON], 0.85, 0.02, 0.0, thing_mode == V_VIBRATO);
	draw_mode_button( *ui_elements[BT_WARP], 1.0, 0.02, 0.0, thing_mode == V_WARP);
	draw_mode_button( *ui_elements[BT_QUANTIZE], 1.15, 0.02, 0.0, thing_mode == V_QUANTIZE);
	draw_mode_button( *ui_elements[BT_GAIN], 0.85, -0.14, 0.0, thing_mode == V_GAIN);
	draw_mode_button( *ui_elements[BT_CUT], 1.0, -0.14, 0.0, thing_mode == V_CUT);
	draw_mode_button( *ui_elements[BT_MOVE], 1.15, -0.14, 0.0, thing_mode == V_MOVE);
	draw_mode_button( *ui_elements[BT_POINT], 0.85, -0.3, 0.0, thing_mode == V_POINT);
	draw_mode_button( *ui_elements[BT_HARM], 1.15, -0.3, 0.0, thing_mode == V_HARM);
}

//-----------------------------------------------------------------------------
void UIGroup::draw_track_box( UI_Element & e, float x, float y, float z, float width, float height )
{
	// PLAY button should pulsate when playing
	ui_elements[BT_PLAY]->on = timeline->playing();

	// Position butters 'n' stuff
	draw_lr_butter(*ui_elements[BT_LEFT], ui_elements[BT_NOW],
		*ui_elements[BT_RIGHT], x, y - height, 0.051f);

	// Make a combined list of tracks from all selected templates
	std::vector<struct GTrackInfo*> *trackinfo = new std::vector<struct GTrackInfo*>;
	for( int i = 0; i < ui_tracks->size(); i++ )
	{
		if( !(*templ_selected)[i] )
			continue;

		std::vector<UI_Track*> * trs = (*ui_tracks)[i];
		std::vector<bool> * sel = (*track_selected)[i];

		for(int k = 0; k < trs->size(); k++)
		{
			struct GTrackInfo *info = new struct GTrackInfo;
			info->track = (*trs)[k];
			info->selected = (*sel)[k];
			info->offset = (*templ_offsets)[i];
			trackinfo->push_back( info );
		}
	}
	time_changed = false;

	// common variables
	t_TAPSINGLE start = ui_elements[BT_LEFT]->slide_0 * BirdBrain::srate();
	t_TAPSINGLE end = ui_elements[BT_RIGHT]->slide_1 * BirdBrain::srate();

	// render the track box
	tb_renderer->render(trackinfo, start, end, 
		ui_elements[FL_SHOW_ALL_TRACKS]->slide > 0.5);

	// draw the point being edited, if any
	if( thing_mode == V_POINT && !pe->stopped ) {
		Track * pet = ((*(*ui_tracks)[pe->templind])[pe->trackind])->core;
		t_TAPTIME offset = (*templ_offsets)[pe->templind];
		tb_renderer->render_point_edit(pe, pet, offset, start, end);
	}

	// draw the cut marker
	if( thing_mode == V_CUT && last_point != NULL ) {
		BBox bb(Point2D(x, y), Point2D(x + width, y - height));
		t_TAPSINGLE lpx = vr_to_world_x(m_vr, (*last_point)[0]);
		t_TAPSINGLE lpy = vr_to_world_y(m_vr, (*last_point)[1]);
		if( bb.in( Point2D(lpx, lpy) ) ) 
			tb_renderer->render_cut_marker(ui_elements[FL_CUT_ONE]->slide > 0.5, lpx, lpy);
	}

	// draw the gain coloring (redundant code from above, but easier to understand)
	if( thing_mode == V_GAIN && last_point != NULL ) {
		BBox bb(Point2D(x, y), Point2D(x + width, y - height));
		t_TAPSINGLE lpx = vr_to_world_x(m_vr, (*last_point)[0]);
		t_TAPSINGLE lpy = vr_to_world_y(m_vr, (*last_point)[1]);
		if( bb.in( Point2D(lpx, lpy) ) ) {
			t_TAPSINGLE time_range = ui_elements[SL_GAIN_TIME]->fvalue() * BirdBrain::srate();
			t_TAPSINGLE freq_range = ui_elements[SL_GAIN_FREQ]->fvalue();
			tb_renderer->render_gain_coloring(lpx, lpy, 
				time_range / (end - start) * trackbox_width, 
				freq_range / (BirdBrain::srate() / 2.0) * trackbox_height, 
				ui_elements[SL_GAIN_AMOUNT]->slide); // to match the color of the slider
		}
	}

	delete trackinfo;
}

//-----------------------------------------------------------------------------
void UIGroup::draw_thing_box(void)
{
	static float thing_spin = 0.0; // rotation of the spinny thing

	switch(thing_mode) {
	case V_SELECT:
		thing_spin += 2.0;
		draw_thing(thingbox_left + thingbox_width/2, thingbox_top - thingbox_height/2, thing_spin, ui_elements[BT_THING]);
		break;
	case V_WARP:
	{
		for(int j = SL_TIME_STRETCH; j <= SL_FREQ_WARP; j++)
			ui_elements[j]->draw();
		break;
	}
	case V_VIBRATO:
	{
		for(int j = SL_PERIODIC_VIBE; j <= SL_FREQ_VIBE; j++)
			ui_elements[j]->draw();
		break;
	}
	case V_QUANTIZE:
		draw_button( *ui_elements[BT_QUANTIZE_ACTION], thingbox_left + thingbox_width/2, thingbox_top - thingbox_height/2, 0.0, 0.5, 1.0, 0.5, IMG_PLAY );
		break;
	case V_GAIN:
	{
		for(int j = SL_GAIN_AMOUNT; j <= SL_GAIN_FREQ; j++)
			ui_elements[j]->draw();
		break;
	}
	case V_CUT:
		draw_flipper( *ui_elements[FL_CUT_ONE], thingbox_left + thingbox_width / 3, thingbox_top - thingbox_height / 2, 0.0 );
		draw_flipper( *ui_elements[FL_CUT_ALL], thingbox_left + 2 * thingbox_width / 3, thingbox_top - thingbox_height / 2, 0.0 );
		break;
	case V_MOVE:
		draw_flipper( *ui_elements[FL_MOVE_FREQ], thingbox_left + thingbox_width / 3, thingbox_top - thingbox_height / 2, 0.0 );
		draw_flipper( *ui_elements[FL_MOVE_TIME], thingbox_left + 2 * thingbox_width / 3, thingbox_top - thingbox_height / 2, 0.0 );
		break;
	case V_HARM:
		ui_elements[SL_FUDGE]->draw();
		break;
	case V_POINT:
		break;
	}
}

//-----------------------------------------------------------------------------
void UIGroup::render_pre()
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
void UIGroup::render_post()
{
	glPopAttrib();

	AudicleFace::render_post();
}

//-----------------------------------------------------------------------------
void UIGroup::render_view( )
{
	if( m_first )
	{
		// set again
		m_vr.init( *m_vrs[0] );
		m_first = false;
	}

	glMatrixMode( GL_PROJECTION );

	glOrtho( m_vr.left(), m_vr.right(), m_vr.down(), m_vr.up(), -10, 10 );
	m_vr.next();

	glMatrixMode( GL_MODELVIEW );

	glLoadIdentity();
	gluLookAt( 0.0f, 0.0f, 3.5f,
			   0.0f, 0.0f, 0.0f,
			   0.0f, 1.0f, 0.0f );

	// set the position of the lights
	glLightfv( GL_LIGHT0, GL_POSITION, m_light0_pos );
	glLightfv( GL_LIGHT1, GL_POSITION, m_light1_pos );
}

//-----------------------------------------------------------------------------
t_TAPUINT UIGroup::on_activate()
{
	g_text_color[0] = 0.0f;
	g_text_color[1] = 0.0f;
	g_text_color[2] = 0.0f;

	fetch_from_library();

	return AudicleFace::on_activate();
}

//-----------------------------------------------------------------------------
void UIGroup::fetch_from_library()
{
	// Check if there are any new templates in the library
	// TODO: Check if templates were removed? Don't think you can remove templates yet.
	Library * lib = Library::instance();
	for(int i = 0; i < lib->templates.size(); i++)
	{
		if(lib->templates[i]->core->type != TT_DETERMINISTIC)
			continue;

		bool match = FALSE;

		for(int k = 0; k < ui_temps->size(); k++)
		{
			if(lib->templates[i]->core == (*ui_temps)[k]->core)
			{
				match = TRUE;
				break;
			}
		}

		if(match)
			continue;

		UI_Template * dummy = new UI_Template;
		dummy->id = IDManager::instance()->getPickID();
		dummy->core = lib->templates[i]->core;
		dummy->backup = lib->templates[i]->backup;

		ui_temps->push_back(dummy);

		templ_selected->push_back(FALSE); // deselected by default

		// Create lists to hold each track for this template, and whether each track is selected
		// and its offset
		ui_tracks->push_back(new std::vector<UI_Track*>);
		track_selected->push_back(new std::vector<bool>);

		// Create an initial selection set with every track selected
		std::set<UI_Track *> * mysel = new std::set<UI_Track *>();

		// Add the tracks and their selection statuses to the lists
		Deterministic * det = (Deterministic *) dummy->core;
		t_TAPTIME detstart = det->get_start(time_changed); // make sure time_changed is true
		time_changed = false;
		templ_offsets->push_back( detstart );
		for(int j = 0; j < det->tracks.size(); j++)
		{
			// Encapsulate the track in a UI_Track and add it to the list for this template
			UI_Track * ui_track = new UI_Track;
			ui_track->core = det->tracks[j];
//			ui_track->backup = new Track(*(ui_track->core)); // where to delete this? destructor?
			ui_track->backups = new std::deque<Track *>();
			ui_track->backups->push_front(new Track(*(ui_track->core)));
			ui_track->id = IDManager::instance()->getPickID();
			(*ui_tracks)[ui_tracks->size()-1]->push_back(ui_track);

			// Mark it as selected
			(*track_selected)[track_selected->size()-1]->push_back(TRUE);

			// Insert it into the selection set
			mysel->insert( ui_track );
		}

		// Insert the selection set into backup_selections
		backup_selections->push_front( mysel );
	}

	sync_track_selections();
}

//-----------------------------------------------------------------------------
t_TAPUINT UIGroup::on_deactivate( t_TAPDUR dur )
{
	return AudicleFace::on_deactivate( dur );
}

//-----------------------------------------------------------------------------
t_TAPUINT UIGroup::on_event( const AudicleEvent & event )
{
	if( event.type == ae_event_INPUT )
	{
		InputEvent * ie = (InputEvent *)event.data;
		if( ie->type == ae_input_MOUSE )
		{
			ie->popStack();

			bool hit = false;

			if( ie->state == ae_input_DOWN )
				hit = handle_mouse_down(ie);
			else if( ie->state == ae_input_UP )
				handle_mouse_up(ie);

			if(!hit)
				hit = handle_uitemp_event( ie );

			if(!hit)
				hit = handle_uitrack_event( ie );
		}
		else if( ie->type == ae_input_MOTION )
		{
			handle_mouse_motion(ie);
		}
		else if( ie->type == ae_input_KEY ) {
			handle_keyboard_event(ie);
		}
	}

	return AudicleFace::on_event( event );
}

//-----------------------------------------------------------------------------
bool UIGroup::handle_mouse_down( const InputEvent * ie )
{
	bool hit = false;

	for( int i = 0; i < NUM_UI_ELEMENTS; i++ )
	{
		if( ie->checkID( ui_elements[i]->id ) )
		{
			hit = TRUE;
			ui_elements[i]->down = TRUE;

			// slider / butter update
			if( i >= BT_LEFT && i <= BT_RIGHT ) // use -xpos of butters instead of 1.1 below
				ui_elements[i]->slide_last = ( vr_to_world_x(m_vr,ie->pos[0]) - trackbox_left )
					/ ui_elements[BT_LEFT]->the_width;
			if( i >= SL_TIME_STRETCH && i <= SL_FREQ_VIBE )
				ui_elements[i]->slide_last = ( vr_to_world_x(m_vr,ie->pos[0]) - (*ui_elements[i]->loc)[0] )
					/ g_slider_height;

			// save the point
			/*if( thing_mode == V_MOVE || thing_mode == V_CUT || thing_mode == V_GAIN ) {
				if( last_point == NULL )
					last_point = new Point2D;
				(*last_point)[0] = ie->pos[0];
				(*last_point)[1] = ie->pos[1];
			}*/
		}
	}

	return hit;
}

//-----------------------------------------------------------------------------
void UIGroup::handle_mouse_up( const InputEvent * ie )
{
	for( int i = 0; i < NUM_UI_ELEMENTS; i++ )
	{
		if( ie->checkID( ui_elements[i]->id ) )
		{
			if( ui_elements[i]->down == TRUE )
			{
				switch(i)
				{
				case BT_ANALYSIS_FACE:
				case BT_SYNTHESIS_FACE:
				case BT_CONTROL_FACE:
				case BT_SEARCH_FACE:
				case BT_GROUP_FACE:
					handle_face_button( BT_ANALYSIS_FACE, BT_SEARCH_FACE, i );
					break;
				
				case BT_QUIT:
				case BT_FULLSCREEN:
					handle_ctrl_button(BT_QUIT, BT_FULLSCREEN, i);
					break;

				case BT_PLAY:
					play();
					break;

				case BT_STOP:
					timeline->stop();
					break;

				case BT_SAVE:
					save();
					break;

				case BT_ALL:
					select_all();
					break;

				case BT_INVERT:
					invert_selection();
					break;

				case BT_VIBRUTTON:
					change_mode( V_VIBRATO );
					break;

				case BT_QUANTIZE:
					change_mode( V_QUANTIZE );
					break;

				case BT_QUANTIZE_ACTION:
					if( make_new_backup )
						update_backups();
					quantize();
					break;

				case BT_SELECT_HARMONICS:
					ui_elements[i]->slide = 1.0f - ui_elements[i]->slide;
					break;

				case FL_SHOW_ALL_TRACKS:
					ui_elements[i]->slide = 1.0f - ui_elements[i]->slide;
					sync_track_selections();
					break;

				case FL_CUT_ONE:
					ui_elements[i]->slide = 1.0f;
					ui_elements[FL_CUT_ALL]->slide = 0.0f;
					break;

				case FL_CUT_ALL:
					ui_elements[i]->slide = 1.0f;
					ui_elements[FL_CUT_ONE]->slide = 0.0f;
					break;

				case FL_MOVE_FREQ:
					ui_elements[i]->slide = 1.0f;
					ui_elements[FL_MOVE_TIME]->slide = 0.0f;
					break;

				case FL_MOVE_TIME:
					ui_elements[i]->slide = 1.0f;
					ui_elements[FL_MOVE_FREQ]->slide = 0.0f;
					break;

				case BT_WARP:
					change_mode( V_WARP );
					break;

				case BT_CUT:
					change_mode( V_CUT );
					break;

				case BT_GAIN:
					change_mode( V_GAIN );
					break;

				case BT_MOVE:
					change_mode( V_MOVE );
					break;

				case BT_POINT:
					change_mode( V_POINT );
					break;

				case BT_UNDO:
					revert_to_backup();
					time_changed = true; // quite possible
					sync_track_selections();
					break;

				case BT_HARM:
					change_mode( V_HARM );
					break;

				case BT_TRACKBOX:
					// cut tracks on a line?
					if( thing_mode == V_CUT && ui_elements[FL_CUT_ALL]->slide > 0.5 )
						cut_all( ie );
				}
			}
		}

		// button up
		if( ui_elements[i]->down ) {
			ui_elements[i]->down = FALSE;
		}
	}
}

//-----------------------------------------------------------------------------
void UIGroup::handle_mouse_motion( const InputEvent * ie )
{
	int i;

	// butter
	for( i = BT_LEFT; i <= BT_RIGHT; i++ )
	{
		if( ui_elements[i]->down )
		{
			fix_slider_motion(*ui_elements[i], m_vr, ie->pos, trackbox_left, ui_elements[BT_LEFT]->the_width, FALSE);
			if( ui_elements[BT_RIGHT]->slide - ui_elements[BT_LEFT]->slide <= .004f )
			{
				if( i == BT_RIGHT )
					ui_elements[i]->slide = ui_elements[i]->slide_last = ui_elements[BT_LEFT]->slide + .005f;
				else if( i == BT_LEFT )
					ui_elements[i]->slide = ui_elements[i]->slide_last = ui_elements[BT_RIGHT]->slide - .005f;
			}
		}
	}

	// slider
	for( i = SL_TIME_STRETCH; i <= SL_FREQ_VIBE; i++ )
	{
		if( ui_elements[i]->down ) {
			fix_slider_motion(*ui_elements[i], m_vr, ie->pos, (*ui_elements[i]->loc)[0], g_slider_height, FALSE);
			handle_slider_event(i);
		}
	}

	// move
	if( thing_mode == V_MOVE && ui_elements[BT_TRACKBOX]->down )
	{
		if( make_new_backup )
			update_backups();

		if( ui_elements[FL_MOVE_FREQ]->slide > 0.5 )
			adjust_freq_move( *last_point, ie->pos );
		else
			adjust_time_move( *last_point, ie->pos );
	}

	// gain painting
	if( thing_mode == V_GAIN && ui_elements[BT_TRACKBOX]->down )
	{
		if( make_new_backup )
			update_backups();
		adjust_gain(ie);
	}

	// cut / move / gain: save position
	if( thing_mode == V_CUT || thing_mode == V_MOVE || thing_mode == V_GAIN )
	{
		// save last mouse position
		if( last_point == NULL )
			last_point = new Point2D;
		(*last_point)[0] = ie->pos[0];
		(*last_point)[1] = ie->pos[1];
	}

	// edit point
	if( thing_mode == V_POINT )
	{
		if( make_new_backup )
			update_backups();
		pe->edit_point( ie );
	}
}

//-----------------------------------------------------------------------------
// mainly for zooming in and out, so far
void UIGroup::handle_keyboard_event(const InputEvent *ie)
{
	switch( ie->key )
	{
		case '0':
		case 27: // esc
			m_vr.setDest( *m_vrs[0] );
			break;

		case '1': // track box view region
		case '2': // lower 1/3 of track box view region
		case '3': // lower 2/3 of track box view region
			switch_to_view(ie->key - '0');
			break;

		case '4':
			switch_to_view(1);
			break;

		case '5': // library
			switch_to_view(4);
			break;

		case 's':
			timeline->stop();
			break;

		case ' ': // space bar: play
			timeline->starttime = (t_TAPUINT)(ui_elements[BT_LEFT]->fvalue() * BirdBrain::srate());
			timeline->stoptime = (t_TAPUINT)(ui_elements[BT_RIGHT]->fvalue() * BirdBrain::srate());
			timeline->play(m_bus);
			break;

		case 'a':
			select_all();
			break;

		case 'i':
			invert_selection();
			break;
	}
}

//-----------------------------------------------------------------------------
void UIGroup::switch_to_view(long index)
{
	// if already there, go to full view
	if(m_vr.m_dest == m_vrs[index])
		m_vr.setDest( *m_vrs[0] );
	else
		m_vr.setDest( *m_vrs[index] );
}

//-----------------------------------------------------------------------------
// call appropriate effect/adjustment function depending on which slider was changed
void UIGroup::handle_slider_event(int ui_index)
{
	switch(ui_index)
	{
	case SL_TIME_STRETCH:
		if(make_new_backup)
			update_backups();
		adjust_time_stretch();
		break;

	case SL_FREQ_WARP:
		if(make_new_backup)
			update_backups();
		adjust_freq_warp();
		break;

	case SL_PERIODIC_VIBE:
	case SL_RANDOM_VIBE:
	case SL_FREQ_VIBE:
		if(make_new_backup)
			update_backups();
		adjust_vibrato();
		break;
	}
}

//-----------------------------------------------------------------------------
void UIGroup::play(void)
{
	// temporarily testing tracks to make sure their time info didn't get messed up somewhere
	bool ok = true;
	for( int m = 0; m < play_template->tracks.size(); m++ )
		ok = check_track(play_template->tracks[m], -1, m) && ok;
	if( !ok )
		msg_box( "warning", "Track inconsistencies; see terminal" );

	timeline->starttime = (t_TAPUINT)(ui_elements[BT_LEFT]->fvalue() * BirdBrain::srate());
	timeline->stoptime = (t_TAPUINT)(ui_elements[BT_RIGHT]->fvalue() * BirdBrain::srate());
	timeline->play(m_bus);
}

//-----------------------------------------------------------------------------
void UIGroup::save(void)
{
	Deterministic * temp = new Deterministic( play_template->tracks );

	std::string fname;
	if( save_as_name( fname ) )
	{
		temp->name = BirdBrain::getname( fname.c_str() );

		// replace space with _
		int space = temp->name.find( " ", 0 );
		while( space != std::string::npos )
		{
			temp->name[space] = '_';
			space = temp->name.find( " ", space );
		}

		Library::instance()->add( temp );

		// write
		save_to_file( temp, fname );

		/** DESELECT EVERYTHING ELSE **/

		for(int i = 0; i < ui_temps->size(); i++)
			if( (*ui_temps)[i]->core != temp)
				(*templ_selected)[i] = FALSE;

		/** UPDATE LIBRARY **/

		fetch_from_library();
	}
}

//-----------------------------------------------------------------------------
void UIGroup::invert_selection(void)
{
	for( int p = 0; p < ui_tracks->size(); p++ )
	{
		std::vector<UI_Track*> * trs = (*ui_tracks)[p];

		if( !(*templ_selected)[p] )
			continue;

		for(int k = 0; k < trs->size(); k++)
			(*(*track_selected)[p])[k] = !(*(*track_selected)[p])[k];
	}

	sync_track_selections();
}

//-----------------------------------------------------------------------------
void UIGroup::select_all(void)
{
	for( int p = 0; p < ui_tracks->size(); p++ )
	{
		std::vector<UI_Track*> * trs = (*ui_tracks)[p];

		if( !(*templ_selected)[p] )
			continue;

		for(int k = 0; k < trs->size(); k++)
			(*(*track_selected)[p])[k] = true;
	}

	sync_track_selections();
}

//-----------------------------------------------------------------------------
void UIGroup::sync_track_selections()
{
	// Clear the old track placeholders
	for(int ii = 0; ii < play_template->tracks.size(); ii++)
		delete play_template->tracks[ii];
	play_template->tracks.clear();

	// Add all the selected tracks to the template to be played
	// And calculate the length of the longest track
	t_TAPTIME max_len = 0;
	t_TAPTIME max_template_len = 0;
	t_TAPTIME earliest_start = -1;
	for(int i = 0; i < ui_temps->size(); i++)
	{
		if(!(*templ_selected)[i]) continue;

		std::vector<UI_Track*> * tracks = (*ui_tracks)[i];
		std::vector<bool> * selected = (*track_selected)[i];
		t_TAPTIME offset = (*templ_offsets)[i];

		// find bounds and size of this "template"
		// (not using get_start() because on cuts, the new tracks are not in the original template)
		t_TAPTIME start = -1, end = 0;
		for( int t = 0; t < tracks->size(); t++ ) {
			Track * tr = (*tracks)[t]->core;
			if( start < 0 )
				start = tr->start - offset;
			else
				start = min(start, tr->start - offset);
			end = max(end, tr->end - offset);
		}
		max_template_len = max(max_template_len, end); // - start

        // copy selected tracks
		for(int k = 0; k < tracks->size(); k++)
		{
			if((*selected)[k])
			{
				// de-select 0-sized tracks
				if( (*tracks)[k]->core->history.empty() ) {
					Track * temp = (*tracks)[k]->core;
					(*selected)[k] = FALSE;
					continue;
				}

				// Add a COPY of the track so we can shift it to 0
				Track * dummy = new Track();
				*dummy = *((*tracks)[k]->core);

				// Shift for "template" to start at 0
				dummy->start = dummy->start - offset;
				dummy->end = dummy->end - offset;
				// Shift all the history points too
				freqpolar * point;
				for(int m = 0; m < dummy->history.size(); m++) {
					point = &(dummy->history[m]);
					point->time = point->time - offset;
				}
				max_len = max(max_len, dummy->end);// - d->get_start(time_changed));

				// Just dump it somewhere dawg (okay!)
				play_template->tracks.push_back(dummy);

				// get earliest start from all selected tracks so far
				if( earliest_start < 0 || earliest_start > dummy->start )
					earliest_start = dummy->start;
			}
		}
	}
	time_changed = false;
	play_template->syn.setTracks( play_template->tracks );
	timeline->stoptime = max_len + BirdBrain::syn_wnd_size(); // pad with enough time to render last frame
	timeline->starttime = earliest_start;
	bool all = ui_elements[FL_SHOW_ALL_TRACKS]->slide > 0.5;
	timeline->duration = max_template_len + BirdBrain::syn_wnd_size();

	// move the play_template according to when its earliest track starts
	// relative to the other unselected tracks
	timeline->remove( ui_play_template );
	timeline->place( ui_play_template, timeline->starttime / timeline->duration, 0 );

	// adjust butter bounds
	for( int j = BT_LEFT; j <= BT_RIGHT; j++ )
		if( all )
			ui_elements[j]->set_bounds(0, timeline->duration / BirdBrain::srate(), false );
		else
			ui_elements[j]->set_bounds(timeline->starttime / BirdBrain::srate(),
			                           timeline->stoptime / BirdBrain::srate(), false );
	ui_elements[BT_LEFT]->set_slide(timeline->starttime / BirdBrain::srate());
	ui_elements[BT_RIGHT]->set_slide(timeline->stoptime / BirdBrain::srate());
	timeline->now_butter = ui_elements[BT_NOW];
}

//-----------------------------------------------------------------------------
bool UIGroup::handle_uitemp_event( const InputEvent * ie )
{
	bool hit = FALSE;

	for( int i = 0; i < ui_temps->size(); i++ )
	{
		if( ie->checkID( ((*ui_temps)[i])->id ) )
		{
			if( ie->state == ae_input_DOWN )
			{
				hit = TRUE;
				((*ui_temps)[i])->down = TRUE;
			}
			if( ie->state == ae_input_UP && ((*ui_temps)[i])->down == TRUE )
			{
				// select or deselect
				bool selection = !((*templ_selected)[i]);
				(*templ_selected)[i] = selection;

				// Select/deselect the template's tracks
				std::vector<bool> * sel = (*track_selected)[i];
				for(int p = 0; p < sel->size(); p++)
					(*sel)[0] = selection;

				sync_track_selections();
			}
		}

		// button up
		if( ie->state == ae_input_UP && ((*ui_temps)[i])->down )
			((*ui_temps)[i])->down = FALSE;
	}

	return hit;
}

//-----------------------------------------------------------------------------
void UIGroup::select_harmonics(bool select, t_TAPTIME time, float freq)
{
	for( int i = 0; i < ui_tracks->size(); i++ )
	{
		Deterministic * det = (Deterministic *) (*ui_temps)[i]->core;
		std::vector<UI_Track*> * trs = (*ui_tracks)[i];
		std::vector<bool> * sel = (*track_selected)[i];

		for(int k = 0; k < trs->size(); k++)
		{
			UI_Track * track = (*trs)[k];
			
			if(track->core->start - (*templ_offsets)[i] > time)
				continue;

			freqpolar trinfo;
			for( int h = 0; h < track->core->history.size(); h++ )
			{
				trinfo = track->core->history[h];

				// Wait 'til we (I) get to the one right at where we clicked
				if(trinfo.time - (*templ_offsets)[i] < time)
					continue;

				// Check whether it's a multiple (within a fudge)
				float multiple = trinfo.freq / freq;
				max_fudge = ui_elements[SL_FUDGE]->fvalue();
				if(multiple < 2.0 - max_fudge)
					continue;
				float fudge = multiple - (int) multiple; // get the fractional part
				if(fudge > 0.5)
					fudge = 1.0 - fudge;
				if(fudge < max_fudge)
				{
					(*(*track_selected)[i])[k] = select;
				}

				break;
			}
		}
	}
}

//-----------------------------------------------------------------------------
bool UIGroup::handle_uitrack_event( const InputEvent * ie )
{
	bool hit = FALSE;

	for( int i = 0; i < ui_tracks->size(); i++ )
	{
		std::vector<UI_Track*> * tracks = (*ui_tracks)[i];
		std::vector<bool> * sel = (*track_selected)[i];

		for(int k = 0; k < tracks->size(); k++)
		{
			UI_Track * track = (*tracks)[k];

			if(ie->checkID(track->id))
			{
				// mouse down on this track
				if( ie->state == ae_input_DOWN )
				{
					hit = TRUE;
					track->down = TRUE;
					if( thing_mode == V_POINT )
						pe->init(i, k, ie );
				}

				// finished mouse click on this track
				if( ie->state == ae_input_UP && track->down == TRUE )
				{
					switch(thing_mode)
					{
					case V_SELECT:
					case V_VIBRATO:
					case V_WARP:
					case V_QUANTIZE:
					case V_MOVE:
					case V_HARM:
					{
						bool select_it = !(*sel)[k];

						// If they checked the harmonics checkbox, (de)select harmonics!
						if(ui_elements[BT_SELECT_HARMONICS]->slide > 0.5)
						{
							t_TAPTIME time = track_box_time(ie->pos[0]);
							int h = find_clicked_history_index(track, i, ie);
							if( h < track->core->history.size() ) {
								float clicked_freq = track->core->history[h].freq;
								select_harmonics(select_it, time, clicked_freq); // (de)select all harmonics, too
							}
						}

						// select or deselect the track
						(*sel)[k] = select_it;

						// mark that the selection set has been modified
						make_new_backup = true;

						// reset sliders to their default values to be valid with new backup
						reset_effects();

						// sync track selections...!
						sync_track_selections();
						break;
					}
					case V_CUT:
						if( ui_elements[FL_CUT_ONE]->slide > 0.5 )
							cut_one( i, k, ie );
						else
							cut_all( ie );
						break;

					case V_GAIN:
						fprintf( stderr, "Track hit in gain painting mode\n" );
					}
				}
			}

			// no longer pressed!
			if( ie->state == ae_input_UP && track->down )
				track->down = FALSE;
		}
	}

	// and finally, if mouse up, stop editing a point, no matter which element is touched
	if( ie->state == ae_input_UP && thing_mode == V_POINT )
		pe->stop();

	return hit;
}

//-----------------------------------------------------------------------------
void UIGroup::change_mode(int which)
{
	thing_mode = (thing_mode == which) ? V_SELECT : which;
	reset_effects();
	update_backups();
}

//-----------------------------------------------------------------------------
void UIGroup::quantize()
{
	for( int i = 0; i < ui_tracks->size(); i++ )
	{
		if( !(*templ_selected)[i] )
			continue;

		std::vector<UI_Track*> * trs = (*ui_tracks)[i];

		for(int k = 0; k < trs->size(); k++)
		{
			if((*(*track_selected)[i])[k] == true)
			{
				Track * core = (*trs)[k]->core;
				Track * backup = (*trs)[k]->backups->front();
				for(int j = 0; j < core->history.size(); j++)
				{
					// this converts the frequency to the midi key,
					//      rounds to the nearest midi key (cast to int),
					//      then converts back to a frequency
					int midi = 12 * log((float)(backup->history[j].freq / 440.0)) / log((float)2) + 69; // ftom
					core->history[j].freq = 440.0 * pow(2.0, (midi - 69.0) / 12.0); // mtof
					core->history[j].bin = (t_TAPUINT)(core->history[j].freq / BirdBrain::srate() * BirdBrain::fft_size());
				}
			}
		}
	}

	sync_track_selections();
}

//-----------------------------------------------------------------------------
void UIGroup::adjust_freq_warp(void)
{
	// get freq_warp amount
	t_TAPSINGLE freqwarp = ui_elements[SL_FREQ_WARP]->fvalue();

	// adjust selection
	for( int i = 0; i < ui_tracks->size(); i++ )
	{
		std::vector<UI_Track*> * trs = (*ui_tracks)[i];
		if( !(*templ_selected)[i] )
			continue;

		for(int k = 0; k < trs->size(); k++) {
			if((*(*track_selected)[i])[k] == true) {
				Track * core = (*trs)[k]->core;
				Track * backup = (*trs)[k]->backups->front();

				for(int j = 0; j < core->history.size(); j++)
				{
					core->history[j].freq = backup->history[j].freq * freqwarp;
					core->history[j].bin = (t_TAPUINT)(core->history[j].freq / BirdBrain::srate() * BirdBrain::fft_size());
				}
			}
		}
	}

	sync_track_selections();
}

//-----------------------------------------------------------------------------
void UIGroup::adjust_time_stretch(void)
{
	// get time-stretch amount
	t_TAPSINGLE timestretch = ui_elements[SL_TIME_STRETCH]->fvalue();

	// first find earliest start-time of any track in entire selection
	t_TAPTIME first;
	bool first_invalid = true;
	for( int ii = 0; ii < ui_tracks->size(); ii++ )
	{
		if( !(*templ_selected)[ii] )
			continue;

		t_TAPTIME offset = (*templ_offsets)[ii];
		std::vector<UI_Track*> * trs = (*ui_tracks)[ii];

		for( int k = 0; k < trs->size(); k++ )
		{
			if( !(*(*track_selected)[ii])[k] )
				continue;

			if( first > ((*trs)[k]->backups->front())->start - offset || first_invalid )
			{
				first = ((*trs)[k]->backups->front())->start - offset;
				first_invalid = false;
			}
		}
	}

	// then adjust selection
	for( int i = 0; i < ui_tracks->size(); i++ )
	{
		std::vector<UI_Track*> * trs = (*ui_tracks)[i];
		if( !(*templ_selected)[i] )
			continue;

		t_TAPTIME offset = (*templ_offsets)[i];
		for(int k = 0; k < trs->size(); k++)
		{
			if((*(*track_selected)[i])[k] == true)
			{
				Track * core = (*trs)[k]->core;
				Track * backup = (*trs)[k]->backups->front();

				for(int j = 0; j < core->history.size(); j++)
					core->history[j].time = first + timestretch*(backup->history[j].time - offset - first) + offset;

				core->start = first + timestretch*(backup->start - offset - first) + offset;
				core->end = first + timestretch*(backup->end - offset - first) + offset;
			}
		}
	}

	time_changed = true;
	sync_track_selections();
}

//-----------------------------------------------------------------------------
void UIGroup::adjust_vibrato()
{
	t_TAPSINGLE periodic = ui_elements[SL_PERIODIC_VIBE]->fvalue();
	t_TAPSINGLE random = ui_elements[SL_RANDOM_VIBE]->fvalue();
	t_TAPSINGLE frequency = ui_elements[SL_FREQ_VIBE]->fvalue();

	for( int i = 0; i < ui_tracks->size(); i++ )
	{
		if( !(*templ_selected)[i] )
			continue;

		Deterministic * det = (Deterministic *)(*ui_temps)[i]->core;
		std::vector<UI_Track*> * trs = (*ui_tracks)[i];
		std::vector<bool> * sel = (*track_selected)[i];

		for( int k = 0; k < trs->size(); k++ )
		{
			if( !(*(*track_selected)[i])[k] )
				continue;

			UI_Track * track = (*trs)[k];
			freqpolar * trinfo, * trbkup;

			// modify track frequencies
			for( int h = 0; h < track->core->history.size(); h++ )
			{
				trinfo = &track->core->history[h];
				trbkup = &((track->backups->front())->history[h]);
				t_TAPSINGLE r = rand() / (RAND_MAX + 1.0) * 2 - 1; // -1 to 1

				trinfo->freq = trbkup->freq * (1 +
					periodic * sin(2 * PIE * frequency * trinfo->time / BirdBrain::srate()) +
					random * r);
				trinfo->bin = (t_TAPUINT)(trinfo->freq / BirdBrain::srate() * BirdBrain::fft_size());
			}
		}
	}

	sync_track_selections();
}

//-----------------------------------------------------------------------------
void UIGroup::adjust_gain( const InputEvent * ie )
{
	t_TAPTIME time = track_box_time(ie->pos[0]);
	t_TAPSINGLE freq = track_box_freq(ie->pos[1]);

	for( int template_index = 0; template_index < ui_tracks->size(); template_index++ )
	{
		if( !(*templ_selected)[template_index] )
			continue;

		std::vector<UI_Track* > * tracks = (*ui_tracks)[template_index];
		std::vector<bool> * sel = (*track_selected)[template_index];
		t_TAPTIME offset = (*templ_offsets)[template_index];

		for( int track_index = 0; track_index < tracks->size(); track_index++ )
		{
			if( !(*sel)[track_index] )
				continue;

			UI_Track * track = (*tracks)[track_index];

			freqpolar *trinfo;
			for( int h = 0; h < track->core->history.size(); h++ )
			{
				trinfo = &track->core->history[h];
				t_TAPTIME tdiff = (trinfo->time - offset) - time;
				t_TAPSINGLE fdiff = trinfo->freq - freq;
				if(tdiff < 0) tdiff = -tdiff;
				if(fdiff < 0) fdiff = -fdiff;
				if(tdiff < ui_elements[SL_GAIN_TIME]->fvalue() * BirdBrain::srate() 
				   && fdiff < ui_elements[SL_GAIN_FREQ]->fvalue())
					trinfo->p.mag *= ui_elements[SL_GAIN_AMOUNT]->fvalue();
			}
		}
	}

	sync_track_selections();
}

//-----------------------------------------------------------------------------
// adjust frequency shifting / transposing
void UIGroup::adjust_freq_move( Point2D prev, Point2D curr)
{
	// get coordinates
	t_TAPSINGLE prevy = vr_to_world_y( m_vr, prev[1] );
	t_TAPSINGLE curry = vr_to_world_y( m_vr, curr[1] ); // yum... curry
	t_TAPSINGLE dy = (curry - prevy) / trackbox_height;
	t_TAPSINGLE dy2freq = dy / trackbox_height * BirdBrain::srate() / 2;
	BB_log( BB_LOG_FINE, "vertical displacement: %f = %f Hz", dy, dy2freq );	
	// out of bounds checking
	t_TAPSINGLE top = BirdBrain::srate() / 2;
	t_TAPSINGLE bottom = 0;
	// apply to selected tracks
	for( int i = 0; i < ui_tracks->size(); i++ ) {
		if( !(*templ_selected)[i] )
			continue;
		std::vector<UI_Track*> * trs = (*ui_tracks)[i];
		for(int k = 0; k < trs->size(); k++) {
			if((*(*track_selected)[i])[k] == true) {
				Track * core = (*trs)[k]->core;
				for(int j = 0; j < core->history.size(); j++) {
					core->history[j].freq += dy2freq;
					core->history[j].bin = (t_TAPUINT)(core->history[j].freq / BirdBrain::srate() * BirdBrain::fft_size());
					// check bounds
					if(core->history[j].freq - top > 0) 
						top = core->history[j].freq;
					if(core->history[j].freq - bottom < 0)
						bottom = core->history[j].freq;
				}
			}
		}
	}
	// correct tracks if out of bounds
	t_TAPSINGLE correction = 0;
	if( top > BirdBrain::srate() / 2 )
		correction = top - BirdBrain::srate() / 2;
	else if( bottom < 0 )
		correction = bottom; 
	// TODO: verify that at most one of the above conditions can be true
	// Well, it is so with the precondition that all tracks are within bounds
	if( correction > 0.00001 || correction < -0.00001 ) {
		// Now for actual correction (groan, do it all again)
		for( int i = 0; i < ui_tracks->size(); i++ )
		{
			if( !(*templ_selected)[i] )
				continue;
			std::vector<UI_Track*> * trs = (*ui_tracks)[i];
			for(int k = 0; k < trs->size(); k++) {
				if((*(*track_selected)[i])[k] == true) {
					Track * core = (*trs)[k]->core;
					for(int j = 0; j < core->history.size(); j++) {
						core->history[j].freq -= correction;
						core->history[j].bin = (t_TAPUINT)
							(core->history[j].freq / BirdBrain::srate() * BirdBrain::fft_size());
					}
				}
			}
		}
	}
	// done!
	sync_track_selections();
}

//-----------------------------------------------------------------------------
// adjust time shifting / translating
void UIGroup::adjust_time_move( Point2D prev, Point2D curr)
{
	// get coordinates
	t_TAPSINGLE prevx = vr_to_world_x( m_vr, prev[0] );
	t_TAPSINGLE currx = vr_to_world_x( m_vr, curr[0] );
	t_TAPSINGLE dx = (currx - prevx) / trackbox_height;
	t_TAPSINGLE length = ui_elements[BT_RIGHT]->slide_1 - ui_elements[BT_LEFT]->slide_0;
	length *= BirdBrain::srate(); // to get length in samples
	t_TAPSINGLE dx2time = dx / trackbox_width * length;
	BB_log( BB_LOG_FINE, "horizontal displacement: %f = %f Hz", dx, dx2time );
	// out of bounds checking
	t_TAPSINGLE correction = 0;
	// apply to selected tracks
	for( int i = 0; i < ui_tracks->size(); i++ ) {
		if( !(*templ_selected)[i] )
			continue;
		// if template selected
		std::vector<UI_Track*> * trs = (*ui_tracks)[i];
		for(int k = 0; k < trs->size(); k++) {
			if((*(*track_selected)[i])[k] == true) {
				// update
				Track * core = (*trs)[k]->core;
				for(int j = 0; j < core->history.size(); j++)
					core->history[j].time += dx2time;
				core->start += dx2time;
				core->end += dx2time;
				// check bounds (only at start point, not end)
				if(core->start - (*templ_offsets)[i] < correction)
					correction = core->start - (*templ_offsets)[i];
			}
		}
	}
	time_changed = true;
	// correct if out of bounds
	if( correction < 0 ) {
		// groan, do it all again
		for( int i = 0; i < ui_tracks->size(); i++ ) {
			if( !(*templ_selected)[i] )
				continue;
			std::vector<UI_Track *> * trs = (*ui_tracks)[i];
			for(int k = 0; k < trs->size(); k++) {
				if((*(*track_selected)[i])[k]) {
					Track * core = (*trs)[k]->core;
					for(int j = 0; j < core->history.size(); j++) 
						core->history[j].time -= correction;
					core->start -= correction;
					core->end -= correction;
				}
			}
		}
	}
	// done!
	sync_track_selections();
}

//-----------------------------------------------------------------------------
// cut a track (would it be better to cut a track only if it has already been selected?)
void UIGroup::cut_one( int template_index, int track_index, const InputEvent * ie )
{
	BB_log(BB_LOG_INFO, "UIGroup::cutting a track");

	std::vector<UI_Track*> * tracks = (*ui_tracks)[template_index];
	std::vector<bool> * sel = (*track_selected)[template_index];
	UI_Track * track = (*tracks)[track_index];

	// find the clicked point
	int h = find_clicked_history_index(track, template_index, ie);
	// fprintf(stderr, "h: %i, size: %i, trinfo.time: %f\n", h, track->core->history.size(), trinfo.time);

	// now cut the track
	if( h < track->core->history.size() )
	{
		// make sure original track is selected
		(*sel)[track_index] = TRUE;

		// create a new track and make it usable
		UI_Track * new_track = new UI_Track;
		new_track->backups = new std::deque<Track *>();
		new_track->backups->push_front(new Track);
		new_track->core = new Track;
		new_track->id = IDManager::instance()->getPickID();
		tracks->push_back(new_track);
		sel->push_back(TRUE);

		// make a new backup of original + new track
		std::set<UI_Track *> * selection = new std::set<UI_Track *>();
		selection->insert(track);
		selection->insert(new_track);
		update_backups(selection); // selection is stored for backup purposes, don't delete

		// fill new track's history and set start and end (ignoring other member variables)
		Track * t = new_track->core;
		t->start = track->core->history[h].time;
		t->end = track->core->end;
		for( int g = h; g < track->core->history.size(); g++ )
			t->history.push_back(track->core->history[g]);

		// remove those points from original track's history and update its end time
		if(h >= 1)
			track->core->end = track->core->history[h-1].time;
		else
			track->core->end = track->core->start;

		track->core->history.erase(track->core->history.begin() + h, track->core->history.end());
		sync_track_selections();
	}
}

//-----------------------------------------------------------------------------
// cut all tracks intersecting a vertical line
void UIGroup::cut_all( const InputEvent * ie )
{
	BB_log(BB_LOG_INFO, "UIGroup::cutting multiple tracks");

	// collect set of selected and appopriate to cut tracks
	std::set<UI_Track *> * selection = new std::set<UI_Track *>();
	std::vector<UI_Track *> * old_tracks = new std::vector<UI_Track *>();
	std::vector<UI_Track *> * new_tracks = new std::vector<UI_Track *>();
	std::vector<int> * histpts = new std::vector<int>();

	// (find cut-time first)
	t_TAPTIME cuttime = track_box_time(ie->pos[0]);

	for( int template_index = 0; template_index < ui_tracks->size(); template_index++ )
	{
		if( !(*templ_selected)[template_index] )
			continue;

		std::vector<UI_Track*> * tracks = (*ui_tracks)[template_index];
		t_TAPTIME offset = (*templ_offsets)[template_index];
		std::vector<bool> * sel = (*track_selected)[template_index];

		for( int track_index = 0; track_index < tracks->size(); track_index++ )
		{
			if( !(*sel)[track_index] )
				continue;

			UI_Track * tr = (*tracks)[track_index];

			// if selected track begins before cut point and ends after cut point
			if( tr->core->start - offset < cuttime && tr->core->end - offset > cuttime )
			{
				int h = find_clicked_history_index(tr, template_index, ie);
				if( h >= tr->core->history.size() )
					continue;

				// make new track
				UI_Track * new_track = new UI_Track;
				new_track->backups = new std::deque<Track *>();
				new_track->backups->push_front(new Track);
				new_track->core = new Track;
				new_track->id = IDManager::instance()->getPickID();
				tracks->push_back(new_track);
				sel->push_back(TRUE);

				// add to selection  + other bookkeeping
				selection->insert( tr );
				selection->insert( new_track );
				old_tracks->push_back( tr );
				new_tracks->push_back( new_track );
				histpts->push_back( h );
			}
		}
	}

	// make new backups of selection
	update_backups(selection);

	// edit tracks
	for( int i = 0; i < old_tracks->size(); i++ )
	{
		UI_Track * track = (*old_tracks)[i];
		UI_Track * new_track = (*new_tracks)[i];
		int h = (*histpts)[i];
		Track * t = new_track->core;

		// add points to new track
		t->start = track->core->history[h].time;
		t->end = track->core->end;
		for( int g = h; g < track->core->history.size(); g++ )
			t->history.push_back(track->core->history[g]);

		// remove those points from original track's history and update its end time
		if(h >= 1)
			track->core->end = track->core->history[h-1].time;
		else
			track->core->end = track->core->start;
		track->core->history.erase(track->core->history.begin() + h, track->core->history.end());
	}

	sync_track_selections();

	// delete stuff (selection is stored for backup purposes; don't delete)
	SAFE_DELETE( old_tracks );
	SAFE_DELETE( new_tracks );
	SAFE_DELETE( histpts );
}

//-----------------------------------------------------------------------------
// find the point in a track's history (index to a freqpolar value) that was clicked
// (from select_harmonics code)
int UIGroup::find_clicked_history_index(const UI_Track* track, int template_index, const InputEvent* ie)
{
	t_TAPTIME time = track_box_time(ie->pos[0]);
	// fprintf(stderr, "through: %f time: %f\n", through, time);

	// First we gotta find the history point at which they clicked
	t_TAPTIME offset = (*templ_offsets)[template_index];
	freqpolar trinfo;
	int h = 0;
	for( h = 0; h < track->core->history.size(); h++ )
	{
		trinfo = track->core->history[h];
		if(trinfo.time - offset >= time)
		{
			// we just passed the point where the user clicked, so we're close enough
			if( h > 0 ) {
				t_TAPTIME off_this = (trinfo.time - offset) - time;
				t_TAPTIME off_prev = time - (track->core->history[h-1].time - offset);
				BB_log( BB_LOG_FINE, "history point: this %f prev %f h %d ", off_this, off_prev, h );
				if( off_prev < off_this )
					h = h-1;
			}
			break;
		}
	}

	return h;
}

//-----------------------------------------------------------------------------
t_TAPTIME UIGroup::track_box_time(float mousex)
{
	// percentage through the trackbox they clicked
	float through = (vr_to_world_x(m_vr, mousex) - trackbox_left) / trackbox_width;

	// corresponding time
	t_TAPSINGLE left = ui_elements[BT_LEFT]->slide_0 * BirdBrain::srate();
	t_TAPSINGLE length = ui_elements[BT_RIGHT]->slide_1 * BirdBrain::srate() - left;
	return through * length + left;
}

//-----------------------------------------------------------------------------
t_TAPSINGLE UIGroup::track_box_freq(float mousey)
{
	float through = (vr_to_world_y(m_vr, mousey) - (trackbox_top - trackbox_height)) / trackbox_height;
	return through * BirdBrain::srate() / 2;
}

//-----------------------------------------------------------------------------
t_TAPSINGLE UIGroup::time_to_track_box(t_TAPTIME time)
{
	t_TAPSINGLE
		left = ui_elements[BT_LEFT]->slide_0 * BirdBrain::srate(),
		length = ui_elements[BT_RIGHT]->slide_1 * BirdBrain::srate() - left,
		xx = (t_TAPSINGLE)(time - left) / length * trackbox_width + trackbox_left;
	return xx;
}

//-----------------------------------------------------------------------------
t_TAPSINGLE UIGroup::freq_to_track_box(t_TAPSINGLE freq)
{
	return freq / BirdBrain::srate() * 2 * trackbox_height + (trackbox_top - trackbox_height);
}

//-----------------------------------------------------------------------------
// update backup/book-keeping for undos
// a backup is made (each time a new effect is selected) OR
// (each time track selections change AND an effect adjustment takes place)
// if which_tracks is null, it backs up all user-selected tracks
// for cuts, it gets a smaller set (pair) of tracks to backup, instead of the entire selection
void UIGroup::update_backups(std::set<UI_Track*> * selection)
{
	if(ui_tracks->empty())
		return;

	int i, k;

	// create set of selected tracks if needed
	if(selection == NULL)
	{
		selection = new std::set<UI_Track *>;
		for( i = 0; i < ui_tracks->size(); i++ )
		{
			if( !(*templ_selected)[i] )
				continue;

			std::vector<UI_Track*> * trs = (*ui_tracks)[i];
			std::vector<bool> * sel = (*track_selected)[i];

			for( k = 0; k < trs->size(); k++)
			{
				if( (*sel)[k] )
				{
					selection->insert( (*trs)[k] );

					// add a backup for this track
					(*trs)[k]->backups->push_front(new Track(*(*trs)[k]->core));
				}
			}
		}
	}
	// otherwise add a backup for each track in the selection provided
	else
	{
		// go through the selection
		std::set<UI_Track *>::iterator it;
		for ( it=selection->begin() ; it != selection->end(); it++ ) {
			// add a backup for each track
			UI_Track * uit = *it;
			uit->backups->push_front(new Track(*(uit->core)));
		}
	}

	// get rid of oldest backup if there are too many
	if( backup_selections->size() == max_backups )
	{
		if( !backup_selections->empty() )
		{
			std::set<UI_Track *> * oldest = backup_selections->back();
			backup_selections->pop_back();

			// get rid of oldest backup on associated tracks
			std::set<UI_Track *>::iterator it;
			for ( it=oldest->begin() ; it != oldest->end(); it++ )
			{
				UI_Track * uit = *it;
				if( uit->backups->empty() )
				{
					BB_log( BB_LOG_WARNING, "ui_track %x: (while cleaning) backup unexpectedly empty", uit );
				}
				else if( uit->backups->size() > 1 ) // slightly unethical, but don't want empty backup
				{
					Track * t = uit->backups->back();
					uit->backups->pop_back();
					SAFE_DELETE(t);
				}
			}
			SAFE_DELETE(oldest);
		}
	}

	// add new set to backup_selections
	backup_selections->push_front(selection);

	// mark that this backup is for the latest backup point
	make_new_backup = false;
}

//-----------------------------------------------------------------------------
void UIGroup::revert_to_backup(void)
{
	// anything to revert to?
	if( backup_selections->empty() )
		return;

	// get latest backup
	std::set<UI_Track *> * latest = backup_selections->front();
	backup_selections->pop_front();

	std::set<UI_Track *>::iterator it;
	for ( it=latest->begin() ; it != latest->end(); it++ )
	{
		UI_Track * uit = *it;
		if( uit->backups->empty() )
		{
			BB_log( BB_LOG_WARNING, "ui_track %x: (while reverting) backup unexpectedly empty", uit );
		}
		else
		{
			// revert to backup version of each track here
			Track * t = uit->backups->front();
			*(uit->core) = *t; // copy track by value, since core pointer must not change
			// slightly unethical, but we don't want empty backups :(
			if( uit->backups->size() > 1 )
			{
				uit->backups->pop_front();
				SAFE_DELETE(t);
			}
		}
	}

	// make a new backup of this same version before changing anything else
	make_new_backup = true;

	// reset sliders and such to be valid for latest backup
	reset_effects();
}

//-----------------------------------------------------------------------------
// reset effect sliders / other related ui elements, if any
void UIGroup::reset_effects(void)
{
	ui_elements[SL_TIME_STRETCH]->set_slide(1.0f);
	ui_elements[SL_FREQ_WARP]->set_slide(1.0f);
	ui_elements[SL_PERIODIC_VIBE]->set_slide(0.0f);
	ui_elements[SL_RANDOM_VIBE]->set_slide(0.0f);
	ui_elements[SL_FREQ_VIBE]->set_slide(0.0f);
}




//-----------------------------------------------------------------------------
// TrackBoxRenderer FUNCTIONS
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
void TrackBoxRenderer::render(std::vector<struct GTrackInfo*> *tracks,
	t_TAPSINGLE start, t_TAPSINGLE end, bool show_all)
{
	// Draw the background and border
	glPushName(ele->id);
	glPushMatrix();

		// blue background
		glTranslatef( 0.0f, 0.0f, .05f );
		if( BirdBrain::white_bg() ) glColor3f( .6f, .8f, 1.0f );
		else glColor3f( 0.4f, 0.6f, 0.7f );
		glRectf(x + 0.001, y - 0.001, x + width - 0.001, y - height + 0.001);

		// black outline
		glColor3f( 0.0f, 0.0f, 0.0f );
		glBegin( GL_LINE_LOOP );
			glVertex2f( x, y );
			glVertex2f( x + width, y );
			glVertex2f( x + width, y - height );
			glVertex2f( x, y - height );
		glEnd();

	glPopMatrix();
	glPopName();

	// draw the tracks
	t_TAPSINGLE
		xx, yy,
		left = start,
		length = end - left;
	freqpolar trinfo;
	for( int eint = 0; eint < tracks->size(); eint++ )
	{
		struct GTrackInfo *info = (*tracks)[eint];

		glPushName(info->track->id);
		glLineWidth( 3.0 );
		glBegin( GL_LINE_STRIP );
		for( int h = 0; h < info->track->core->history.size(); h++ )
		{
			trinfo = info->track->core->history[h];

			if(info->selected)
				glColor3f( 0.3f, 0.45f - trinfo.p.mag * 20, 0.6f );
			else if( show_all ) {
				if( BirdBrain::white_bg() ) glColor3f( 0.5f, 0.5f, 0.5f );
				else glColor3f( 1.0f, 1.0f, 1.0f );
			}
			else
				continue;

			xx = (t_TAPSINGLE)(trinfo.time - info->offset - left) / length * width + x;
			yy = (t_TAPSINGLE)trinfo.freq / BirdBrain::srate() * 2 * height + y - height;
			glVertex3f( xx, yy, 0.06f ); // it was xx + 0.01, but that made cuts less accurate
		}
		glEnd();
		glLineWidth(1.0);
		glPopName();
	}
	glColor3f( 1.0f, 1.0f, 1.0f );
}

//-----------------------------------------------------------------------------
void TrackBoxRenderer::render_cut_marker(bool cut_one, t_TAPSINGLE px, t_TAPSINGLE py)
{
	glColor3f(1.0f, 0.3f, 0.3f);

	if( cut_one )
	{
		glLineWidth(2.2);

		glBegin(GL_LINE_LOOP);
			glVertex3f(px - 0.02f, py - 0.02f, 0.07f);
			glVertex3f(px + 0.02f, py + 0.02f , 0.07f);
		glEnd();

		glBegin(GL_LINE_LOOP);
			glVertex3f(px - 0.02f, py + 0.02f, 0.07f);
			glVertex3f(px + 0.02f, py - 0.02f, 0.07f);
		glEnd();
	}
	else
	{
		glLineWidth(1.2);

		glBegin(GL_LINE_LOOP);
			glVertex3f(px, y, 0.07f);
			glVertex3f(px, y - height, 0.07f);
		glEnd();
	}

	glLineWidth(1.0);
	glColor3f( 1.0f, 1.0f, 1.0f );
}

//-----------------------------------------------------------------------------
void TrackBoxRenderer::render_gain_coloring(t_TAPSINGLE px, t_TAPSINGLE py, 
											t_TAPSINGLE w, t_TAPSINGLE h, t_TAPSINGLE amt)
{
	// get rectange endpoints (left, right, up, down)
	t_TAPSINGLE L = px - w > x ? px - w : x;
	t_TAPSINGLE R = px + w < x + width ? px + w : x + width;
	t_TAPSINGLE U = py + h < y ? py + h : y;
	t_TAPSINGLE D = py - h > y - height ? py - h : y - height;
	// draw
	glColor3f(amt *.6f + .4f, (1-amt)*.6f + .4f, .4f );
	glPushMatrix();
	glTranslatef(0.0f, 0.0f, 0.055f);
	glBegin(GL_QUADS);
	glVertex2f(L, D);
	glVertex2f(R, D);
	glVertex2f(R, U);
	glVertex2f(L, U);
	glEnd();
	glPopMatrix();
	glColor3f( 1.0f, 1.0f, 1.0f );
}

//-----------------------------------------------------------------------------
void TrackBoxRenderer::render_point_edit(PointEditor * pe, Track * pet, t_TAPSINGLE offset, 
										 t_TAPSINGLE start, t_TAPSINGLE end)
{
	t_TAPSINGLE
		xx, yy,
		left = start,
		length = end - left;
	freqpolar h;
	int ind, pgs;
	for(ind = pe->histoind-pe->prefreeze-pe->prelee; ind <= pe->histoind+pe->postfreeze+pe->postlee; ind++) {
		if( ind < 0 ) continue;
		if( ind >= pet->history.size() ) break;
		h = pet->history[ind];
		xx = (t_TAPSINGLE)(h.time - offset);
		xx = (xx - left) / length * width + x;
		yy = (t_TAPSINGLE)h.freq / BirdBrain::srate() * 2 * height + y - height;
		if( ind == pe->histoind ) {
			glColor3f( 1.0f, 0.8f, 0.5f );
			pgs = 8;
		}
		else {
			if( ind >= pe->histoind-pe->prefreeze && ind <= pe->histoind+pe->postfreeze )
				glColor3f( 1.0f, 0.5f, 0.3f );
			else
				glColor3f( 1.0f, 0.3f, 0.5f );
			pgs = 4;
		}
		glPushMatrix();
		glTranslatef( xx, yy, 0.07f );
		glutSolidSphere( ind == pe->histoind ? 0.01 : 0.006, pgs, pgs );
		glPopMatrix();
	}
	glColor3f( 1.0f, 1.0f, 1.0f );
}

//-----------------------------------------------------------------------------
// PointEditor FUNCTIONS
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// construct: create and keep it turned off
PointEditor::PointEditor(float trackbox_left, float trackbox_top, float trackbox_width, float trackbox_height) :
	trackbox_left(trackbox_left), trackbox_top(trackbox_top),
	trackbox_width(trackbox_width), trackbox_height(trackbox_height)
{
	mytrack = NULL;
	stopped = TRUE;
}

//-----------------------------------------------------------------------------
PointEditor::~PointEditor() {
	mytrack = NULL;
}

//-----------------------------------------------------------------------------
// init: initialize to a specific track and history point, and turn on
void PointEditor::init( int template_index, int track_index, const InputEvent * ie )
{
	templind = template_index;
	trackind = track_index;
	std::vector<UI_Track*> * tracks = (*(g_group_face->ui_tracks))[templind];
	std::vector<bool> * sel = (*(g_group_face->track_selected))[templind];
	mytrack = (*tracks)[trackind];
	histoind = g_group_face->find_clicked_history_index( mytrack, templind, ie );
	prelee = 5; postlee = 5; prefreeze = 0; postfreeze = 5;
	stopped = FALSE;
}

//-----------------------------------------------------------------------------
// edit_point: do the actual edit
void PointEditor::edit_point( const InputEvent * ie )
{
	if( stopped )
		return;

	freqpolar * hist = &(mytrack->core->history[histoind]);
	t_TAPSINGLE x = vr_to_world_x( g_group_face->m_vr, ie->pos[0] );
	t_TAPSINGLE y = vr_to_world_y( g_group_face->m_vr, ie->pos[1] );

	/// frequency
	t_TAPSINGLE base = trackbox_top - trackbox_height;
	hist->bin = (t_TAPUINT)((y - base) / trackbox_height * BirdBrain::fft_size() / 2);
	hist->freq = (t_TAPSINGLE)(hist->bin) / BirdBrain::fft_size() * BirdBrain::srate();
	// hist->freq = (y - base) / trackbox_height * BirdBrain::srate() / 2;
	// hist->bin = (t_TAPUINT)(hist->freq / BirdBrain::srate() * BirdBrain::fft_size());

	/// time (not applicable to start and end points of the track)
	int histsize = mytrack->core->history.size();
	if( histoind - prefreeze > 0 && histoind + postfreeze < histsize - 1 ) // to keep endpoints intact
	{
		int i, j;
		t_TAPTIME endpt;

		t_TAPSINGLE left = g_group_face->ui_elements[BT_LEFT]->slide_0 * BirdBrain::srate();
		t_TAPSINGLE length = g_group_face->ui_elements[BT_RIGHT]->slide_1 * BirdBrain::srate() - left;
		t_TAPSINGLE offset = (*(g_group_face->templ_offsets))[templind];
		t_TAPTIME wanted = (x - trackbox_left) / trackbox_width * length + left + offset;
		t_TAPTIME shift = wanted - hist->time;
		t_TAPTIME pre_pt = mytrack->core->history[histoind - prefreeze].time;
		t_TAPTIME post_pt = mytrack->core->history[histoind + postfreeze].time;

		if( histsize > 1 && pre_pt + shift <= mytrack->core->history[0].time )
			wanted = mytrack->core->history[0].time + 1 + hist->time - pre_pt;
		else if( histsize > 1 && post_pt + shift >= mytrack->core->history.back().time )
			wanted = mytrack->core->history.back().time - 1 + hist->time - post_pt;

		shift = wanted - hist->time; // re-calculated

		for( i = histoind - prefreeze; i <= histoind + postfreeze; i++ )
			mytrack->core->history[i].time += shift;
		hist->time = wanted;

		//// pre
		i = j = histoind - prefreeze; // is greater than 0
		endpt = mytrack->core->history[i].time;
		while( i >= 0 && endpt <= mytrack->core->history[i].time )
			i--;

		int pre = i > prelee ? i - prelee : 0;
		t_TAPSINGLE dt = 0;

		if( j - pre > 0 )
			dt = (mytrack->core->history[j].time - mytrack->core->history[pre].time) / (j - pre);

		for( i = pre + 1; i < j; i++ )
			mytrack->core->history[i].time = mytrack->core->history[i-1].time + dt;

		//// post
		i = j = histoind + postfreeze; // is less than histsize - 1
		endpt = mytrack->core->history[i].time;

		while( i < histsize && endpt >= mytrack->core->history[i].time )
			i++;

		int post = i + postlee < histsize ? i + postlee : histsize - 1;
		if( post - j > 0 )
			dt = (mytrack->core->history[post].time - mytrack->core->history[j].time) / (post - j);

		for( i = post - 1; i > j; i-- )
			mytrack->core->history[i].time = mytrack->core->history[i+1].time - dt;
	}

	// wrap up
	g_group_face->sync_track_selections();
}

//-----------------------------------------------------------------------------
// stop: turn it off again
void PointEditor::stop()
{
	stopped = TRUE;
	mytrack = NULL;
}


// NOTES
// timeline end is ok now but start's still shaky. if i select tracks only from the
// middle, the audio doesn't match the now butter. maybe: (a) find the earliest
// start time for selected tracks too, so that it can zoom in on them in non-show-all-tracks
// mode, and (b) in show-all-tracks mode, move the play_template to start later in the timeline
// so that audio and visuals line up. hmm. gack.
// end also not so good at times if time-stretch of only some tracks takes place after a cut
// (it incorrectly estimates the end point because the new cut tracks are in no template)
// GOOD NEWS: the above was "fixed" by using the lr_butters more, saving template offsets,
//			  and mostly getting rid of template->get_start() and timeline->duration references

// MOVE: if shifted beyond end points? put checks in place...?
// CUT: should be more consistent about how the original template is affected;
//		either it should remain unchanged (so the original tracks are not cut, but just deselected,
//		and two new tracks are created for each original one that the user wanted to cut),
//		or the new tracks after the cut should be added to the original template as well.
//
