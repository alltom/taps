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
// name: ui_analysis.h
// desc: birdbrain ui
//
// authors: Ananya Misra (amisra@cs.princeton.edu)
//          Ge Wang (gewang@cs.princeton.edu)
//          Perry R. Cook (prc@cs.princeton.edu)
//          Philip Davidson (philipd@cs.princeton.edu)
// date: Autumn 2004
//-----------------------------------------------------------------------------
#ifndef __UI_ANALYSIS_H__
#define __UI_ANALYSIS_H__

#include "ui_audio.h"
#include "taps_regioncomparer.h"
#include "taps_driver.h"
#include "ui_group.h"
#include "taps_transient.h"
#include "audicle_def.h"
#include "audicle_face.h"
#include "audicle_gfx.h"
#include "util_readwrite.h"

// forward reference
struct UI_Element;
extern UIGroup * g_group_face;

//-----------------------------------------------------------------------------
// name: struct SpectroGram
// desc: ...
//-----------------------------------------------------------------------------
struct SpectroGram
{
    Point2D loc;
    Point2D start;
    Point2D curr;
};
#define SG_HEIGHT 1024 //256
#define SG_WIDTH 512 //256


//-----------------------------------------------------------------------------
// name: struct RectCoords
// desc: rectangle coordinates
//-----------------------------------------------------------------------------
struct RectCoords
{
	t_TAPSINGLE left;
	t_TAPSINGLE right; 
	t_TAPSINGLE down; 
	t_TAPSINGLE up;
};


//-----------------------------------------------------------------------------
// name: class UIAnalysis
// desc: ...
//-----------------------------------------------------------------------------
class UIAnalysis : public AudicleFace
{
public:
    UIAnalysis( );
    virtual ~UIAnalysis( );

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
    void spectrogram( t_TAPSINGLE x = 0.0f, t_TAPSINGLE y = 0.0f );
    void spectroinit();
    t_TAPUINT render_sine_pane( void * data );
    t_TAPUINT render_group_pane( void * data );
    t_TAPUINT render_det_mode( void * data );
    t_TAPUINT render_tran_mode( void * data );
    t_TAPUINT render_tran_pane( void * data );
    t_TAPUINT render_raw_pane( void * data );
    t_TAPUINT render_raw_mode( void * data );
    t_TAPUINT render_features_pane( void * data );
    t_TAPUINT render_features_mode( void * data );
    t_TAPUINT render_lib_pane( void * data );
	t_TAPUINT render_settings_pane( void * data );
    void play_transient();
    bool update_transients();
    void read_transient( Frame * fr, t_TAPBOOL usecache = TRUE );
    bool save_raw_spec( Frame * fr, int bounds[] );
    void play_raw_spec( Frame * rawframe );
    bool load_input( AudioSrcBuffer * f );
    bool create_regioncomparer( int regionsize ); 
    void read_segment( Frame * fr );
    void play_segment();
	void zoom_in( t_TAPSINGLE left, t_TAPSINGLE right, t_TAPSINGLE down, t_TAPSINGLE up ); 
	void zoom_out(); 
	void zoom(); // apply zoom in/out coordinates
	void change_sizes(); // change fft and window sizes

public: 
    int m_analysis_mode;
    int m_which_pane;
	
protected:
    Driver * driver;
    Frame * frame;
    Frame * biggerframe;
    TransientExtractor * ee; 
    RegionComparer * rc;

protected: // raw frame stuff
    Frame * rf_rawframe;       // frame for selected specgram region raw extraction
    int rf_specbounds[4];      // left, right, low, high for specgram when rf was extracted
    t_TAPUINT rf_offset;            // where the actual sound begins in rf_rawframe->waveform, given 0-padding at both ends
    SAMPLE * rf_buffer;        // spare buffer, like for frame-by-frame ffts
    AudioSrcFrame * rf_display;// audiosrcframe for display purposes (mticks into rf_buffer)           

protected:
    Frame * rc_extractframe;    // frame to extract samples into for feature-based RegionComparer
    Frame * rc_wholeframe;      // frame to tick entire file into, for feature-based RegionComparer

protected:
	Frame * tranframe;			// frame to store current transient samples
	SAMPLE * tranres;			// waveform of residue after transients are removed
	int tranressize;			// # of samples in above waveform
	AudioSrcBuffer * tranbufex;	// extra audio src for read_transient() so that it doesn't interfere with play_transient()
	int tranfileid;				// id of ye tranfilex in cache

protected:
    t_TAPBOOL m_init;
    AudioCentral * m_audio;
    SAMPLE * m_waveform;
    SAMPLE * m_buffer;
    SAMPLE * m_window;
    t_TAPUINT m_waveform_size;
    AudioSrcFile * m_fin;

    AudioSrcBuffer * m_orig; // 
    AudioSrcFile * m_res;  //
    std::string m_sndfilename;
    std::string m_loadfilename;
    std::string m_loadfileprev; 
    // elements
    //UI_Element * ui_elements;
    SpectroGram m_specgram;
    float m_spectre[SG_WIDTH][SG_HEIGHT];
    line * m_threshold; // which is actually a line, but not a straight line sometimes
	RectCoords m_zoom; 

	// mic input 
	AudioSrcMic * m_mic; 
	
	// input source
	AudioSrcBuffer ** m_input; 
	
    // global variables
    int m_fft_size;
    int m_wnd_size;
    int m_num_channels;
    int m_curr_tran;
	int m_last_tran;
    int rc_curr_segment;
    std::vector<int> rc_segments;
    int rc_regionsize;
	int m_max_fft_size;
	int m_min_fft_size;
	int m_max_wnd_size;
	int m_min_wnd_size;
	int m_max_waveform_size;

    // gain
    GLfloat m_gain;
    GLfloat m_time_scale;
    GLfloat m_freq_scale;
    GLint m_time_view;
    GLint m_freq_view;
    GLboolean m_spec;

    // bounding boxes
    BBox * m_bboxs;
    int m_nbboxs;
    int m_cur_bbox; 
    bool m_highlight; 
    bool m_use_tex;
	
	// saving analysis info
#ifdef __TAPS_XML_ENABLE__
	void save_xml_anal_info();
	xmlDocPtr xml_anal_info;
#endif

protected:
    Point3D fp;
    t_TAPFLOAT m_asp;
};




#endif
