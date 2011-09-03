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
// name: ui_group.h
// desc: track group editing face
//
// authors: Ananya Misra (amisra@cs.princeton.edu)
//          Ge Wang (gewang@cs.princeton.edu)
//          Perry R. Cook (prc@cs.princeton.edu)
//          Tom Lieber (lieber@princeton.edu)
// date: Fall 2007
//-----------------------------------------------------------------------------
#ifndef __UI_GROUP_H__
#define __UI_GROUP_H__

#include "ui_audio.h"
#include "ui_library.h"
#include "taps_driver.h"
#include "ui_element.h"
#include "taps_birdbrain.h"
#include "audicle_def.h"
#include "audicle_face.h"
#include "audicle_gfx.h"
#include <set>

#define GROUP_BUS 3

// forward reference
struct UI_Element;
class PointEditor;
class TrackBoxRenderer;

//-----------------------------------------------------------------------------
// name: class UIGroup
// desc: "group" face for grouping tracks from multiple templates into new templates
//-----------------------------------------------------------------------------
class UIGroup : public AudicleFace
{
public:
	UIGroup( );
	virtual ~UIGroup( );

public:
	virtual t_TAPBOOL init();
	virtual t_TAPBOOL create_widgets();
	virtual t_TAPBOOL setup_widgets();
	virtual t_TAPBOOL destroy();

	virtual void render_pre( );
	virtual void render_post( );
	virtual t_TAPUINT render( void * data );
	virtual void render_view( );
	t_TAPUINT render_det( void * data );

	virtual t_TAPUINT on_activate( );
	virtual t_TAPUINT on_deactivate( t_TAPDUR duration = 0.0 );
	virtual t_TAPUINT on_event( const AudicleEvent & event );

protected:
	void draw_thing_box(void);
	void draw_mode_button(UI_Element & e, float x, float y, float z, bool active);
	void draw_mode_buttons(void);
	void draw_track_box( UI_Element & e, float x, float y, float z, float width, float height );
	void draw_track_box_track( float x, float y, float z, float width, float height );

	t_TAPBOOL m_init;
	AudioCentral * m_audio;
	AudioBus * m_bus;
	Driver * driver;
	TrackBoxRenderer * tb_renderer;

	void sync_track_selections(void);
	void fetch_from_library(void);

	void select_harmonics(bool select, t_TAPTIME time, float freq);
	void quantize(void);
	void adjust_freq_warp(void);
	void adjust_time_stretch(void);
	void adjust_vibrato(void);
	void cut_one( int template_index, int track_index, const InputEvent * ie );
	void cut_all( const InputEvent * ie );
	void adjust_gain( const InputEvent * ie );
	void adjust_freq_move( Point2D prev, Point2D curr);
	void adjust_time_move( Point2D prev, Point2D curr);

	bool handle_mouse_down( const InputEvent * ie ); // returns true if event is consumed
	void handle_mouse_up( const InputEvent * ie );
	void handle_mouse_motion( const InputEvent * ie );
	void handle_keyboard_event( const InputEvent * ie );

	bool handle_uitemp_event( const InputEvent * ie ); // returns true if event is consumed
	bool handle_uitrack_event( const InputEvent * ie ); // returns true if event is consumed
	void handle_slider_event( int ui_index );

	void play(void);
	void save(void);
	void select_all(void);
	void invert_selection(void);
	void switch_to_view(long index);

	void change_mode( int which );
	int find_clicked_history_index( const UI_Track * track, int template_index, const InputEvent * ie );
	t_TAPTIME track_box_time(float mousex);
	t_TAPSINGLE track_box_freq(float mousey);
	t_TAPSINGLE time_to_track_box(t_TAPTIME time);
	t_TAPSINGLE freq_to_track_box(t_TAPSINGLE freq);

	void update_backups(std::set<UI_Track*> * selection = NULL);
	void revert_to_backup(void);
	void reset_effects(void); // reset effect sliders when a new backup is made
	std::deque<std::set<UI_Track *>* > * backup_selections; // selections at each backup point
	bool make_new_backup; // whether to make a new backup; yes if selection has been modified since last backup
	int max_backups; // maximum number of backups

	// Timeline for playback
	Timeline * timeline;
	UI_Template * ui_play_template;
	Deterministic * play_template;

	// Lists of templates/tracks to play
	std::vector<UI_Template*> * ui_temps;
	std::vector<std::vector<UI_Track*>* > * ui_tracks;
	std::vector<bool> * templ_selected; // whether a template is selected
	std::vector<std::vector<bool>* > * track_selected; // whether a track is selected; each list corresponds to a ui_temp
	std::vector<t_TAPTIME> * templ_offsets; // offsets to add to all the time points of the template's tracks

	int file_save_count;

	enum {V_SELECT, V_WARP, V_VIBRATO, V_QUANTIZE, V_GAIN, V_CUT, V_MOVE, V_POINT, V_HARM};
	int thing_mode; // TODO: maybe we should separate effect modes from click/cursor modes?
	bool time_changed; // to tell det->get_start()/get_end() to recompute the start/end
	t_TAPSINGLE max_fudge;

	// :P
	Point2D * last_point;
	PointEditor * pe;
	friend class PointEditor;

	const float trackbox_left, trackbox_top, trackbox_width, trackbox_height;
	const float thingbox_left, thingbox_top, thingbox_width, thingbox_height;
};

// global
extern UIGroup * g_group_face;


//-----------------------------------------------------------------------------
// name: class TrackBoxRenderer
// desc: renders the spectrogram-like display with clickable tracks and selection butters
// IN PROGRESS
//-----------------------------------------------------------------------------
class TrackBoxRenderer
{
public:
	TrackBoxRenderer( UI_Element *ele, float x, float y, float z, float width, float height )
		: ele(ele), x(x), y(y), z(z), width(width), height(height) {}
	void render(std::vector<struct GTrackInfo*> *tracks,
		t_TAPSINGLE start, t_TAPSINGLE end, bool show_all);
	void render_cut_marker(bool cut_one, t_TAPSINGLE px, t_TAPSINGLE py);
	void render_point_edit(PointEditor * pe, Track * pet, t_TAPSINGLE offset,
		t_TAPSINGLE start, t_TAPSINGLE end);
	void render_gain_coloring(t_TAPSINGLE px, t_TAPSINGLE py, t_TAPSINGLE w, t_TAPSINGLE h, t_TAPSINGLE amt);

private:
	float x, y, z, width, height;
	UI_Element *ele;
};


//-----------------------------------------------------------------------------
// name: class PointEditor
// desc: TODO
//-----------------------------------------------------------------------------
class PointEditor {
public:
	PointEditor(float trackbox_left, float trackbox_top, float trackbox_width, float trackbox_height);
	~PointEditor();
	void init( int template_index, int track_index, const InputEvent * ie );
	void edit_point( const InputEvent * ie );
	void stop();

	int templind;
	int trackind;
	int histoind;
	UI_Track * mytrack;
	t_TAPBOOL stopped;
	t_TAPINT prelee;
	t_TAPINT postlee;
	t_TAPINT prefreeze;
	t_TAPINT postfreeze;

protected:
	float trackbox_left, trackbox_top, trackbox_width, trackbox_height;
};

#endif
