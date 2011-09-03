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

#include "driver.h"
#include "audicle_def.h"
#include "audicle_face.h"
#include "audicle_gfx.h"
#include "ui_audio.h"
#include "transient.h"
#include "RegionComparer.h"

// libsndfile
#ifndef __USE_SNDFILE_PRECONF__
#include <sndfile.h>
#else
#include "util_sndfile.h"
#endif


// forward reference
struct UI_Element;


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
// name: class UIAnalysis
// desc: ...
//-----------------------------------------------------------------------------
class UIAnalysis : public AudicleFace
{
public:
    UIAnalysis( );
    virtual ~UIAnalysis( );

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

public:
    void spectrogram( t_CKSINGLE x = 0.0f, t_CKSINGLE y = 0.0f );
    void spectroinit();
    t_CKUINT render_sine_pane( void * data );
    t_CKUINT render_group_pane( void * data );
	t_CKUINT render_det_mode( void * data );
    t_CKUINT render_tran_mode( void * data );
	t_CKUINT render_tran_pane( void * data );
    t_CKUINT render_raw_pane( void * data );
	t_CKUINT render_raw_mode( void * data );
	t_CKUINT render_features_pane( void * data );
	t_CKUINT render_features_mode( void * data );
    t_CKUINT render_lib_pane( void * data );
    void play_transient();
    bool update_transients();
    void read_transient( Frame * fr );
	bool save_raw_spec( Frame * fr, int bounds[] );
	void play_raw_spec( Frame * rawframe );
    bool load_file( AudioSrcFile * f );
	bool create_regioncomparer( int regionsize ); 
	void read_segment( Frame * fr );
	void play_segment();

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
	Frame * rf_rawframe;	   // frame for selected specgram region raw extraction
	int rf_specbounds[4];	   // left, right, low, high for specgram when rf was extracted
	uint rf_offset;			   // where the actual sound begins in rf_rawframe->waveform, given 0-padding at both ends
	SAMPLE * rf_buffer;		   // spare buffer, like for frame-by-frame ffts
	AudioSrcFrame * rf_display;// audiosrcframe for display purposes (mticks into rf_buffer) 		   

protected:
	Frame * rc_extractframe;	// frame to extract samples into for feature-based RegionComparer
	Frame * rc_wholeframe;		// frame to tick entire file into, for feature-based RegionComparer

protected:
    t_CKBOOL m_init;
    AudioCentral * m_audio;
    SAMPLE * m_waveform;
    SAMPLE * m_buffer;
    SAMPLE * m_window;
    t_CKUINT m_waveform_size;
    AudioSrcFile * m_fin;

    AudioSrcFile * m_orig; // 
    AudioSrcFile * m_res;  //
    std::string m_sndfilename;
    std::string m_loadfilename;
    std::string m_loadfileprev; 
    // elements
    UI_Element * ui_elements;
    SpectroGram m_specgram;
    float m_spectre[SG_WIDTH][SG_HEIGHT];
    line * m_threshold; // which is actually a line, but not a straight line sometimes

    // global variables
    int m_fft_size;
    int m_wnd_size;
    int m_num_channels;
    int m_curr_tran;
	int rc_curr_segment;
	std::vector<int> rc_segments;
	int rc_regionsize;

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

protected:
    Point3D fp;
    t_CKFLOAT m_asp;
};




#endif
