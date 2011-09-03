//-----------------------------------------------------------------------------
// name: ui_analysis.cpp
// desc: birdbrain ui
//
// authors: Ananya Misra (amisra@cs.princeton.edu)
//          Ge Wang (gewang@cs.princeton.edu)
//          Perry R. Cook (prc@cs.princeton.edu)
//          Philip Davidson (philipd@cs.princeton.edu)
// date: Winter 2004
//-----------------------------------------------------------------------------
#include <string>
#include <iostream>
using namespace std;

#include "ui_analysis.h"
#include "ui_audio.h"
#include "ui_element.h"
#include "audicle_utils.h"
#include "audicle_gfx.h"
#include "audicle_geometry.h"
#include "audicle.h"
#include "birdbrain.h"
#include "sceptre.h"
#include "ui_library.h"

bool save_as_name( std::string & myname );
bool save_to_file( Template * temp, std::string fname );

// enumeration for ui elements
enum UI_ELMENTS
{
    BT_LOAD = 0,
    BT_ORIG,
    BT_CLASSIFIED,
    BT_REWIND,
    BT_FF,
    BT_PLAY,
    BT_STOP,
	BT_SPEC_SAVE,
    BT_ZOOM_IN,
    BT_ZOOM_OUT,
    BT_SEPARATE,
    BT_TRAN_METHOD,
    BT_GROUP, 
    BT_SELECT_LEFT,
    BT_SELECT_NOW,
    BT_SELECT_RIGHT,
    BT_FREQ_LEFT,
    BT_FREQ_RIGHT,
    BT_TOGGLE,
    BT_SYN_PLAY,
    BT_SYN_SAVE,
    BT_NEXT_EVENT,
    BT_PREV_EVENT,
    BT_ALL,
    BT_RES_PLAY,
    BT_RES_LOAD,
    BT_RES_SAVE,
    // toggle views
    BT_VIEW_SINE,
    BT_VIEW_GROUP,
    BT_VIEW_TRAN,
	BT_VIEW_RAW,
	BT_VIEW_FEATURES,
    BT_VIEW_LIB,
    // res
    BT_CLIP_ONLY,

    SL_DET_TRACKS,      
    SL_DET_MINPOINTS,   
    SL_DET_MAXGAP,      
    SL_GROUP_HARM,
    SL_GROUP_MOD,
    SL_GROUP_OVERLAP,
    SL_RAW_ROLLOFF,			// rolloff for raw specgram extraction
	SL_FEAT_THRESH,			// similarity threshold
	SL_FEAT_B,				// similarity feature b (unnamed to be more changeable) weight
	SL_FEAT_D,				// similarity feature d  weight
    SL_TRAN_LONG,           // energy ratio
    SL_TRAN_MAX_LENGTH,     // energy ratio
    SL_TRAN_ATTACK,         // envelope-based
    SL_TRAN_THRESH,         // both
    SL_TRAN_AGEAMT,         // envelope-based
    SL_DET_THRESH,
    SL_DET_ERROR_FREQ,
    SL_DET_NOISE_RATIO,
    SL_GROUP_ONSET,
    SL_GROUP_OFFSET,
    SL_GROUP_MINLEN,
	SL_FEAT_A,				// similarity feature a weight
	SL_FEAT_C,				// similarity feature c weight
	SL_FEAT_E,				// similarity feature e weight
    SL_TRAN_SHORT,          // energy ratio
    SL_TRAN_DECAY,          // envelope-based
    SL_TRAN_GAP,            // both
	SL_BRIGHTNESS,
    SL_CONTRAST,

    KN_THRESH_TILT,

    UI_SPECTROGRAM,

    // keep this as last
    NUM_UI_ELEMENTS
};


// names of ui elements
static char * ui_str[] = { 
    "load",
    "back",
    "[classified]",
    "<",
    ">",
    "play",
    "stop",
	"save",
    "zoom in",
    "zoom out",
    "separate",
    "use energy ratio",
    "group",
    "left",
    "now",
    "right",
    "low",
    "high",
    "view ->",
    "play",
    "save",
    "next",
    "previous",
    "ALL",
    "play",
    "load me",
    "save",
    "sines",
    "groups",
    "transients",
	"raw",
	"find", 
    "library",
    "clip only",
    "# sine tracks",
    "min. track length",
    "allowable silence",
    "harmonic grouping error",
    "common modulation error",
    "min. track overlap",
	"rolloff",
	"similarity threshold", 
	"low power weighting",
	"pitchiness weighting",
    "long frame size",          // energy ratio
    "max transient length",     // energy ratio
    "attack",                   // envelope-based 
    "threshold",                // both
    "anti-aging factor",        // envelope-based
    "mag. threshold",
    "freq. sensitivity",
    "peak-to-noise ratio",
    "onset error (seconds)",
    "offset error (seconds)", 
    "min. event length (seconds)",
	"centroid weighting", 
	"flux weighting", 
	"variance weighting",
    "short frame size",     // energy ratio
    "decay",                // envelope-based
    "min gap (samples)",    // both
    "brightness",
    "contrast",
    "tilt",
    "spectrogram"
};


// not line (for threshold, for example)
struct notline : public line
{
    double ridiculous_factor;
    notline( double rf ) { ridiculous_factor = rf; }

    virtual double y( double x )
    { 
        double they = line::y( x );
        return (they < 0.0f ? -1 : 1) * ::pow( they / (ridiculous_factor), 2.0 ) / 25.0;
    }
};

// enumeration of rendering views (for sliders)
enum {SINE_PANE, TRAN_PANE, GROUP_PANE, RAW_PANE, FEAT_PANE, LIB_PANE};
// enumeration of analysis modes
enum {ANA_DET, ANA_TRAN, ANA_RAW, ANA_FEAT};

GLfloat g_view_left = 0.15f;

#define THRESH_POW 4.0
void set_rect( SpectroGram & s, UI_Element * ui_elements );
void get_rect( SpectroGram & s, UI_Element * ui_elements );
t_CKUINT g_out_count = 0;
t_CKUINT g_res_count = 0;
bool g_tran_suppressed = false;
std::string g_tran_outfile = ""; 

// use for dealing with all tracks instead of just the current event
bool g_all_tracks = false; 
// show those orange lines for tracks?
bool g_show_tracks = true; 
// the line width of the spectrogram selection rectangle
GLint g_rect_width = 1;

//-----------------------------------------------------------------------------
// name: UIAnalysis()
// desc: ...
//-----------------------------------------------------------------------------
UIAnalysis::UIAnalysis( ) : AudicleFace( )
{ 
    if( !this->init( ) )
    {
        fprintf( stderr, "[audicle]: cannot start face...\n" );
        return;
    }
}




//-----------------------------------------------------------------------------
// name: ~UIAnalysis()
// desc: ...
//-----------------------------------------------------------------------------
UIAnalysis::~UIAnalysis( ) 
{
    this->destroy();
}



//-----------------------------------------------------------------------------
// name: init()
// desc: ...
//-----------------------------------------------------------------------------
t_CKBOOL UIAnalysis::init( )
{
    if( !AudicleFace::init() )
        return FALSE;

    int i;

    // log
    BB_log( BB_LOG_SYSTEM, "initializing analysis user interface..." );
    // push log
    BB_pushlog();

    // driver
    driver = NULL;
    // frame
    frame = new Frame;
    biggerframe = new Frame;
	rf_rawframe = new Frame;
	rf_offset = 0; 
	rf_specbounds[0] = -1; // error value
    // transient extractor
    ee = NULL;
	// region comparer
	rc = NULL;
	rc_extractframe = new Frame;
	rc_wholeframe = new Frame;

    // default filename
    m_sndfilename = "a.wav";
    // ui elements
    ui_elements = new UI_Element[NUM_UI_ELEMENTS];

    // set the buffer_size
    m_fft_size = 4096;
    // window size
    m_wnd_size = m_fft_size / 8;
    // set the channels
    m_num_channels = 1;
    // default gain
    m_gain = 1.0f;
    // time scale
    m_time_scale = 1.0f;
    // freq scale
    m_freq_scale = .35f;
    // time view
    m_time_view = 1;
    // freq view
    m_freq_view = 2;
    // spec
    m_spec = TRUE;
    // curr transient
    m_curr_tran = -1;
	// curr similar region
	rc_curr_segment = -1;
	// region size? 
	rc_regionsize = 0;

    // log
    BB_log( BB_LOG_INFO, "(spectrogram) fft size: %d", m_fft_size );
    BB_log( BB_LOG_INFO, "(spectrogram) window size: %d", m_wnd_size );
    BB_log( BB_LOG_INFO, "(spectrogram) num channels: %d", m_num_channels );

    // rendering
    m_waveform_size = m_fft_size > 2048 ? m_fft_size : 2048;
    // zero
    m_waveform = new SAMPLE[m_waveform_size*2];
    m_buffer = new SAMPLE[m_waveform_size*2];
    m_window = new SAMPLE[m_waveform_size*2];
	rf_buffer = new SAMPLE[m_waveform_size*2];
	rf_display = NULL;
    // window;
    hanning( m_window, m_wnd_size );
    // file in
    m_fin= NULL;
    // files
    m_orig = NULL;
    m_res = NULL;

    // set the name for the face
    m_name = "Birdbrain Analysis";

    // analysis mode
    m_analysis_mode = ANA_DET;
    // sliders
    m_which_pane = SINE_PANE;

    // get id for each element
    for( i = 0; i < NUM_UI_ELEMENTS; i++ )
    {
        ui_elements[i].id = IDManager::instance()->getPickID();
        ui_elements[i].name = ui_str[i];
    }

    // slide
    ui_elements[BT_SELECT_LEFT].slide = 0.0f;
    ui_elements[BT_SELECT_RIGHT].slide = 1.0f;
    ui_elements[BT_FREQ_LEFT].slide = 0.0f;
    ui_elements[BT_FREQ_RIGHT].slide = 1.0f;
    ui_elements[BT_SELECT_NOW].slide = 0.5f;
    ui_elements[SL_BRIGHTNESS].slide = .35f;
    ui_elements[SL_CONTRAST].slide = .7f;
    ui_elements[KN_THRESH_TILT].slide = 0.5f;
    // size
    ui_elements[BT_SELECT_LEFT].size_up = .03f;
    ui_elements[BT_SELECT_RIGHT].size_up = .03f;
    ui_elements[BT_FREQ_LEFT].size_up = .03f;
    ui_elements[BT_FREQ_RIGHT].size_up = .03f;
    ui_elements[BT_SELECT_NOW].size_up = .03f;
    ui_elements[BT_SEPARATE].size_up = .07f;
    ui_elements[BT_REWIND].size_up = .02f;
    ui_elements[BT_FF].size_up = .02f;
    ui_elements[BT_CLASSIFIED].size_up = .03f;
    ui_elements[BT_SYN_PLAY].size_up = .04f;
    ui_elements[BT_SYN_SAVE].size_up = .04f;
    ui_elements[BT_NEXT_EVENT].size_up = .03f;
    ui_elements[BT_PREV_EVENT].size_up = .03f;
    ui_elements[BT_ALL].size_up = .035f;
    ui_elements[BT_RES_PLAY].size_up = .04f;
    ui_elements[BT_RES_LOAD].size_up = .04f;
    ui_elements[BT_RES_SAVE].size_up = .04f;
    ui_elements[BT_VIEW_SINE].size_up = .025f;
    ui_elements[BT_VIEW_GROUP].size_up = .025f;
    ui_elements[BT_VIEW_TRAN].size_up = .025f;
    ui_elements[BT_VIEW_RAW].size_up = .025f;
	ui_elements[BT_VIEW_FEATURES].size_up = .025f;
	ui_elements[BT_VIEW_LIB].size_up = .025f;
    // font size
    ui_elements[BT_SELECT_LEFT].font_size = .7f;
    ui_elements[BT_SELECT_RIGHT].font_size = .7f;
    ui_elements[BT_FREQ_LEFT].font_size = .7f;
    ui_elements[BT_FREQ_RIGHT].font_size = .7f;
    ui_elements[BT_SELECT_NOW].font_size = .7f;
    ui_elements[BT_TOGGLE].font_size = .7f;
    ui_elements[BT_REWIND].font_size = .7f;
    ui_elements[BT_FF].font_size = .7f;
    ui_elements[BT_CLASSIFIED].font_size = .55f;
    ui_elements[BT_SYN_PLAY].font_size = .7f;
    ui_elements[BT_SYN_SAVE].font_size = .7f;
    ui_elements[BT_NEXT_EVENT].font_size = .6f;
    ui_elements[BT_PREV_EVENT].font_size = .6f;
    ui_elements[BT_ALL].font_size = .6f;
    ui_elements[BT_RES_PLAY].font_size = .7f;
    ui_elements[BT_RES_LOAD].font_size = .7f; 
    ui_elements[BT_RES_SAVE].font_size = .7f;
    ui_elements[BT_VIEW_SINE].font_size = .6f;
    ui_elements[BT_VIEW_GROUP].font_size = .6f;
    ui_elements[BT_VIEW_TRAN].font_size = .6f;
    ui_elements[BT_VIEW_RAW].font_size = .6f;
	ui_elements[BT_VIEW_FEATURES].font_size = .6f;
	ui_elements[BT_VIEW_LIB].font_size = .6f;
    for( int slider = SL_DET_TRACKS; slider <= SL_TRAN_GAP; slider++ )
        ui_elements[slider].font_size = .85f; 
    ui_elements[BT_TRAN_METHOD].font_size = .85f;

    // slide parameters
    ui_elements[BT_SELECT_LEFT].slide_int = true;
    ui_elements[BT_SELECT_RIGHT].slide_int = true;
    ui_elements[BT_SELECT_NOW].slide_int = true;
    ui_elements[SL_DET_THRESH].slide_1 = ::pow( .075, 1 / THRESH_POW );

    ui_elements[SL_DET_TRACKS].set_bounds( 1.0f, 50.0f, true );
    ui_elements[SL_DET_MINPOINTS].set_bounds( 1.f, 20.0f, true );
    ui_elements[SL_DET_MAXGAP].set_bounds( 0, 20.0f, true );
    ui_elements[SL_DET_ERROR_FREQ].set_bounds(.5f, 1.0f );
    ui_elements[SL_DET_NOISE_RATIO].set_bounds( 0.f, 20.0f );
    ui_elements[BT_FREQ_LEFT].set_bounds( 0, BirdBrain::srate()/2, true );
    ui_elements[BT_FREQ_RIGHT].set_bounds( 0, BirdBrain::srate()/2, true );
    ui_elements[KN_THRESH_TILT].set_bounds( -80.0f, 80.0f );
    ui_elements[SL_TRAN_THRESH].set_bounds( 0.0f, 15.0f, false );
    ui_elements[SL_TRAN_GAP].set_bounds( 1, BirdBrain::srate() / 2, true );
    ui_elements[SL_TRAN_LONG].set_bounds( 1, 2 * BirdBrain::srate(), true );
    ui_elements[SL_TRAN_SHORT].set_bounds( 1, BirdBrain::srate(), true );
    ui_elements[SL_TRAN_MAX_LENGTH].set_bounds( 1, 2 * BirdBrain::srate(), true );
    ui_elements[SL_GROUP_HARM].set_bounds( 0, 1, false );
    ui_elements[SL_GROUP_MOD].set_bounds( 0, 2, false );
    ui_elements[SL_GROUP_OVERLAP].set_bounds( 0, 1, false );
    ui_elements[SL_GROUP_ONSET].set_bounds( 0, 1, false );
    ui_elements[SL_GROUP_OFFSET].set_bounds( 0, 1, false );
    ui_elements[SL_GROUP_MINLEN].set_bounds( 0, 1, false );
	ui_elements[SL_RAW_ROLLOFF].set_bounds( 0, 1, false );
	ui_elements[SL_FEAT_THRESH].set_bounds( 0, 1, false );
    
    ui_elements[SL_DET_TRACKS].set_slide( 4.0f ); // ooh, this was wrong!!!
    ui_elements[SL_DET_MINPOINTS].set_slide( 2 );
    ui_elements[SL_DET_ERROR_FREQ].set_slide( .85f );
    ui_elements[SL_DET_NOISE_RATIO].set_slide( 3.1f );
    ui_elements[SL_TRAN_THRESH].set_slide( 1.0f );
    ui_elements[SL_TRAN_AGEAMT].set_slide( 0.95f );
    ui_elements[SL_TRAN_GAP].set_slide( 2000 );
    ui_elements[SL_TRAN_ATTACK].set_slide( 0.4f );
    ui_elements[SL_TRAN_DECAY].set_slide( 0.9f );
    ui_elements[SL_TRAN_LONG].set_slide( BirdBrain::srate() / 2 );
    ui_elements[SL_TRAN_SHORT].set_slide( BirdBrain::srate() / 16 );
    ui_elements[SL_TRAN_MAX_LENGTH].set_slide( BirdBrain::srate() );
    ui_elements[SL_GROUP_HARM].set_slide( 0.1f );
    ui_elements[SL_GROUP_MOD].set_slide( 0.3f );
    ui_elements[SL_GROUP_OVERLAP].set_slide( 0.88f );
    ui_elements[SL_GROUP_ONSET].set_slide( 0.01f );
    ui_elements[SL_GROUP_OFFSET].set_slide( 0.03f );
    ui_elements[SL_GROUP_MINLEN].set_slide( 0.1f );
	ui_elements[SL_RAW_ROLLOFF].set_slide( 0.2f );
	ui_elements[SL_FEAT_A].set_slide( 1.0f );
	ui_elements[SL_FEAT_B].set_slide( 1.0f );
	ui_elements[SL_FEAT_C].set_slide( 1.0f );
	ui_elements[SL_FEAT_D].set_slide( 1.0f );
	ui_elements[SL_FEAT_E].set_slide( 1.0f );
	ui_elements[SL_FEAT_THRESH].set_slide( 0.9f );
    
    // bad hack (please remove) (no)
    ui_elements[BT_CLASSIFIED].slide = 0.f;

    // specgram
    m_specgram.loc[0] = .1f;
    m_specgram.loc[1] = .25f;

    // set rect
    set_rect( m_specgram, ui_elements );

    // not line
    m_threshold = new notline( m_gain * m_freq_scale * g_butter_height / .5 );
	
    // -... -... --- -..-
    m_nbboxs = 6;
    m_cur_bbox = -1;
    m_bboxs = new BBox[m_nbboxs];
    // sliders
    m_bboxs[0].add2d( Point2D( -0.1f, -1.2f ) ); 
    m_bboxs[0].add2d( Point2D( 10.0f, -0.16f ) );
    // residue
    m_bboxs[1].add2d( Point2D( -10.0f, -1.2f ) );
    m_bboxs[1].add2d( Point2D( -0.1f, -0.47f ) );
    // sine / transient
    m_bboxs[2].add2d( Point2D( -10.0f, -0.47f ) );
    m_bboxs[2].add2d( Point2D( -0.1f, 0.03f ) );
    // spectrogram
    m_bboxs[3].add2d( Point2D( -0.08f, 0.076f ) ); 
    m_bboxs[3].add2d( Point2D( 10.0f, 1.2f ) );
    // waveform
    m_bboxs[4].add2d( Point2D( -10.0f, 0.03f ) );
    m_bboxs[4].add2d( Point2D( -0.04f, 1.2f ) );
    // buttons
    m_bboxs[5].add2d( Point2D( -0.1f, -0.16f ) );
    m_bboxs[5].add2d( Point2D( 10.0f, 0.076f ) );
    // higlight on/off
    m_highlight = false;
    // spectrogram texture on/off
    m_use_tex = true;

    // everything (default)
    m_vrs.push_back( new ViewRegionManual( 0, 0, -1.0f, 1.0f, FALSE, FALSE ) );
    // waveform
    m_vrs.push_back( new ViewRegionManual( 0, 0, 0, .975f, FALSE, TRUE ) );
    // spectrogram + control
    m_vrs.push_back( new ViewRegionManual( -0.07, 0, -0.14, .915f, TRUE, FALSE ) );
    // sliders
    m_vrs.push_back( new ViewRegionManual( -0.09, 0, -1.0, 0.085, TRUE, FALSE ) );
    // deterministic + stochastic
    m_vrs.push_back( new ViewRegionManual( 0, 0, -.9, 0.05, FALSE, TRUE ) );
    // spectrogram
    m_vrs.push_back( new ViewRegionManual( -0.07, 0, 0.05, .975f, TRUE, FALSE ) );

    // get audio
    m_audio = AudioCentral::instance();

    // log
    BB_log( BB_LOG_INFO, "num ui elements: %d", NUM_UI_ELEMENTS );

    // pop log
    BB_poplog();

    return TRUE;
}




//-----------------------------------------------------------------------------
// name: destroy()
// desc: ...
//-----------------------------------------------------------------------------
t_CKBOOL UIAnalysis::destroy( )
{
    // log
    BB_log( BB_LOG_SYSTEM, "shutting down analysis user interface..." );
    this->on_deactivate( 0.0 );
    m_id = Audicle::NO_FACE;
    m_state = INACTIVE;
    SAFE_DELETE_ARRAY( m_bboxs ); 

	// delete things?
	SAFE_DELETE( frame );
	SAFE_DELETE( biggerframe );
	SAFE_DELETE( rf_rawframe );
	SAFE_DELETE( rc_extractframe );
	SAFE_DELETE( rc_wholeframe );
	SAFE_DELETE_ARRAY( ui_elements );
	SAFE_DELETE_ARRAY( m_waveform );
	SAFE_DELETE_ARRAY( m_buffer );
	SAFE_DELETE_ARRAY( m_window );
	SAFE_DELETE_ARRAY( rf_buffer );
	// m_vrs...
	// driver, transient extractor, region comparer, etc?

    return TRUE;
}



const t_CKUINT g_rainbow_resolution = 0x10000;

Spectre g_rainbow[g_rainbow_resolution]; // Spectre defined in ui_element.h
bool g_rainbow_init = false;
void HSVtoRGB( float h, float s, float v, float * rgb );




//-----------------------------------------------------------------------------
// name: draw_rect()
// desc: ...
//-----------------------------------------------------------------------------
void draw_rect( SpectroGram & specgram )
{
    float left = specgram.start[0];
    float top = specgram.start[1];
    float right = specgram.curr[0];
    float bottom = specgram.curr[1];
    float temp;

    if( left > right )
    { temp = left; left = right; right = temp; }
    if( top < bottom )
    { temp = top; top = bottom; bottom = temp; }

    glLineWidth( g_rect_width );
    glBegin( GL_LINE_LOOP );
    glVertex2f( left, top );
    glVertex2f( right, top );
    glVertex2f( right, bottom );
    glVertex2f( left, bottom );
    glEnd();
    glLineWidth( 1 );
}




//-----------------------------------------------------------------------------
// name: clip_rect()
// desc: ...
//-----------------------------------------------------------------------------
void clip_rect( SpectroGram & specgram, UI_Element * ui_elements )
{
    float max_y = g_butter_height;
    float max_x = g_butter_width;

    if( specgram.curr[0] < 0.0f ) specgram.curr[0] = 0.0f;
    else if( specgram.curr[0] > max_x ) specgram.curr[0] = max_x;
    if( specgram.curr[1] < 0.0f ) specgram.curr[1] = 0.0f;
    else if( specgram.curr[1] > max_y ) specgram.curr[1] = max_y;

    // set sliders
    float left = specgram.start[0];
    float top = specgram.start[1];
    float right = specgram.curr[0];
    float bottom = specgram.curr[1];
    float temp;

    if( left > right )
    { temp = left; left = right; right = temp; }
    if( top < bottom )
    { temp = top; top = bottom; bottom = temp; }
    if( left < ui_elements[BT_SELECT_LEFT].slide_0 )
        left = ui_elements[BT_SELECT_LEFT].slide_0; 
    if( right > ui_elements[BT_SELECT_RIGHT].slide_1 )
        right = ui_elements[BT_SELECT_RIGHT].slide_1;
    if( bottom < ui_elements[BT_FREQ_LEFT].slide_0 )
        bottom = ui_elements[BT_FREQ_LEFT].slide_0; 
    if( top > ui_elements[BT_FREQ_RIGHT].slide_1 )
        top = ui_elements[BT_FREQ_RIGHT].slide_1; 

    ui_elements[BT_SELECT_LEFT].slide = left / g_butter_width;
    ui_elements[BT_SELECT_LEFT].slide_last = left / g_butter_width;
    ui_elements[BT_SELECT_RIGHT].slide = right / g_butter_width;
    ui_elements[BT_SELECT_RIGHT].slide_last = right / g_butter_width;
    ui_elements[BT_FREQ_LEFT].slide = bottom / g_butter_height;
    ui_elements[BT_FREQ_LEFT].slide_last = bottom / g_butter_height;
    ui_elements[BT_FREQ_RIGHT].slide = top / g_butter_height;
    ui_elements[BT_FREQ_RIGHT].slide_last = top / g_butter_height;
}




//-----------------------------------------------------------------------------
// name: set_rect
// desc: ...
//-----------------------------------------------------------------------------
void set_rect( SpectroGram & specgram, UI_Element * ui_elements )
{
    specgram.start[0] = ui_elements[BT_SELECT_LEFT].slide * g_butter_width;
    specgram.curr[0] = ui_elements[BT_SELECT_RIGHT].slide * g_butter_width;
    specgram.start[1] = ui_elements[BT_FREQ_LEFT].slide * g_butter_height;
    specgram.curr[1] = ui_elements[BT_FREQ_RIGHT].slide * g_butter_height;
}


//-----------------------------------------------------------------------------
// name: get_rect
// desc: ...
//-----------------------------------------------------------------------------
void get_rect( SpectroGram & specgram, UI_Element * ui_elements )
{
    float start, curr;
    start = specgram.start[0] / g_butter_width;
    curr = specgram.curr[0] / g_butter_width;
    if( start < curr )
    {
        ui_elements[BT_SELECT_LEFT].slide = ui_elements[BT_SELECT_LEFT].slide_last = start;
        ui_elements[BT_SELECT_RIGHT].slide = ui_elements[BT_SELECT_RIGHT].slide_last = curr;
    }
    else if( curr < start )
    {
        ui_elements[BT_SELECT_LEFT].slide = ui_elements[BT_SELECT_LEFT].slide_last = curr;
        ui_elements[BT_SELECT_RIGHT].slide = ui_elements[BT_SELECT_RIGHT].slide_last = start;
    }

    start = specgram.start[1] / g_butter_height;
    curr = specgram.curr[1] / g_butter_height;
    if( start < curr )
    {
        ui_elements[BT_FREQ_LEFT].slide = ui_elements[BT_FREQ_LEFT].slide_last = start;
        ui_elements[BT_FREQ_RIGHT].slide = ui_elements[BT_FREQ_RIGHT].slide_last = curr;
    }
    else if( curr < start )
    {
        ui_elements[BT_FREQ_LEFT].slide = ui_elements[BT_FREQ_LEFT].slide_last = curr;
        ui_elements[BT_FREQ_RIGHT].slide = ui_elements[BT_FREQ_RIGHT].slide_last = start;
    }
    
}


GLuint g_tex_id = 0; 

//-----------------------------------------------------------------------------
// name: spectroinit()
// desc: ...
//-----------------------------------------------------------------------------
void UIAnalysis::spectroinit( )
{
    uint i, j;

    // log
    BB_log( BB_LOG_INFO, "preprocessing spectrogram display..." );
    // push log
    BB_pushlog();

    // rainbow
    if( !g_rainbow_init )
    {
        for( i = 0; i < g_rainbow_resolution; i++ )
        {
            HSVtoRGB( i / (float)g_rainbow_resolution * 300.0f, 1.0f, 1.0f, 
                      (float *)&g_rainbow[i] );
            // fprintf( stderr, "%i  %f %f %f\n", i, g_rainbow[i].r, g_rainbow[i].g, g_rainbow[i].b );
        }
    }


    if( !m_fin )
    {
        // log
        BB_log( BB_LOG_INFO, "no file to process, setting to default color..." );
        for( i = 0; i < SG_WIDTH; i++ )
            for( j = 0; j < SG_HEIGHT; j++ )
            {
                m_spectre[i][j] = 0.1f;
            }
    }
    else
    {
        // log
        // BB_log( BB_LOG_INFO, "processing: %s...", m_fin->filename().c_str() );
        float max = -10, min = 10;
        for( i = 0; i < SG_WIDTH; i++ )
        {
            m_fin->seek( (t_CKUINT)( i / (t_CKSINGLE)SG_WIDTH * (m_fin->info().frames - 1) + .5f), SEEK_SET );
            m_fin->mtick( m_buffer, m_wnd_size );
            // window and zero pad
            apply_window( m_buffer, m_window, m_wnd_size );
            for( j = m_wnd_size; j < m_fft_size; j++ )
                m_buffer[j] = 0.0f;
            rfft( m_buffer, m_fft_size/2, FFT_FORWARD );
            // scale from zero padding
            BirdBrain::scale_fft( m_buffer, m_fft_size, m_fft_size, m_wnd_size );
            complex * cbuf = (complex *)m_buffer;

            // get x
            t_CKINT sx = i;
            t_CKFLOAT hh = m_fft_size / (t_CKFLOAT)m_freq_view / (t_CKFLOAT)SG_HEIGHT;

            // fill
            for( j = 0; j < SG_HEIGHT; j++ )
            {
                m_spectre[sx][j] = //10 * m_gain * m_freq_scale * 
                    .45f + ( 20.0f * log10( cmp_abs(cbuf[(int)(j * hh)]) ) + 60.0f ) / 60.0f;
                // fprintf( stderr, "%f\n", g_spectre[sx][i]);
                if( m_spectre[sx][j] > max ) max = m_spectre[sx][j];
                if( m_spectre[sx][j] < min ) min = m_spectre[sx][j];
            }
            // log
            BB_log( BB_LOG_CRAZY, "column: %d  value max: %.6f  min: %.6f", i, max, min );
        }

        // log
        BB_log( BB_LOG_INFO, "value max: %.6f  min: %.6f", max, min );
        // check bounds
        if( min < -10 )
        {
            min = -1.25;
            // log
            BB_log( BB_LOG_INFO, "reseting min to: %f...", min );
        }

        // log
        BB_log( BB_LOG_INFO, "scaling values according to min and max..." );

        for( i = 0; i < SG_WIDTH; i++ )
            for( j = 0; j < SG_HEIGHT; j++ )
                m_spectre[i][j] = (m_spectre[i][j] - min)/(max - min);
    }

    // try to make a texture
    unsigned char * spec_tex = new unsigned char[SG_WIDTH * SG_HEIGHT * 3]; 
    for( j = 0; j < SG_HEIGHT; j++ )
        for( i = 0; i < SG_WIDTH; i++ )
        {
            uint index = (t_CKINT)((m_spectre[i][j]) * g_rainbow_resolution - .5f );
            if( index >= g_rainbow_resolution ) index = g_rainbow_resolution - 1;
            if( index < 0 ) index = 0;
            Spectre * color = &g_rainbow[index];
            spec_tex[3*(SG_WIDTH*j+i)] = (unsigned char)(255 * color->r);
            spec_tex[3*(SG_WIDTH*j+i)+1] = (unsigned char)(255 * color->g);
            spec_tex[3*(SG_WIDTH*j+i)+2] = (unsigned char)(255 * color->b);
        }
    glGenTextures( 1, &g_tex_id ); 
    glBindTexture( GL_TEXTURE_2D, g_tex_id );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR ); 
    if( g_tex_id )
        glTexImage2D( 
            GL_TEXTURE_2D,
            0,
            GL_RGB,
            SG_WIDTH,
            SG_HEIGHT,
            0,
            GL_RGB,
            GL_UNSIGNED_BYTE,
            spec_tex
        );
    delete [] spec_tex;

    // pop log
    BB_poplog();
}

//-----------------------------------------------------------------------------
// name: spectrogram()
// desc: ...
//-----------------------------------------------------------------------------
void UIAnalysis::spectrogram( t_CKSINGLE x, t_CKSINGLE y )
{
    t_CKSINGLE hinc = g_butter_height / SG_HEIGHT;
    t_CKSINGLE winc = g_butter_width / SG_WIDTH;
    t_CKSINGLE yy = y;
    t_CKSINGLE xx = x;

    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    glDisable( GL_LIGHTING );
    t_CKSINGLE brightness = ui_elements[SL_BRIGHTNESS].slide;
    t_CKSINGLE contrast = ui_elements[SL_CONTRAST].slide;

    glPushName( ui_elements[UI_SPECTROGRAM].id );
    
    // use texture?
    if( !m_use_tex )
    {
        uint i, j;
        for( i = 0; i < SG_WIDTH; i++ )
        {
            glBegin( GL_QUAD_STRIP );
            for( j = 0; j < SG_HEIGHT; j++ )
            {
                //uint index = (t_CKINT)((brightness + g_spectre[i][j].r * contrast ) * g_rainbow_resolution - .5f);
                uint index = (t_CKINT)((m_spectre[i][j]) * g_rainbow_resolution - .5f );
                if( index >= g_rainbow_resolution ) index = g_rainbow_resolution - 1;
                if( index < 0 ) index = 0;
                Spectre * color = &g_rainbow[index];
                //glColor3fv( (float *)color );
                glColor3f(brightness + color->r * contrast, brightness + color->g * contrast, brightness + color->b * contrast);
                //fprintf( stderr, "%f %i - %f %f %f - %f %f %f\n", g_spectre[i][j].r, index, color->r, color->g, color->b,
                //         g_rainbow[index].r, g_rainbow[index].g, g_rainbow[index].b );
    //            glColor3f( brightness + g_spectre[i][j].r * contrast, 0.0f, 0.0f );
                glVertex2d( x, y );
                glVertex2d( x + winc, y );
                y += hinc;
            }
            glEnd();
            y = yy;
            x += winc;
        }
    }
    else // use texture
    {
        // instead
        glPushMatrix();
        glTranslatef( 0.0f, 0.0f, -0.01f );
        glColor4f( brightness, brightness, brightness, contrast ); 
        glBegin( GL_QUADS );
        glVertex2d( x, y );
        glVertex2d( x + g_butter_width, y );
        glVertex2d( x + g_butter_width, y + g_butter_height );
        glVertex2d( x, y + g_butter_height );
        glEnd();
        glPopMatrix();
        if( g_tex_id ) {
            glColor4f( 1.0f, 1.0f, 1.0f, contrast==0 ? 0 : 1/(2*contrast) ); // alpha seems useless but makes me feel better
            // texture
            glBindTexture( GL_TEXTURE_2D, g_tex_id );
            glEnable( GL_TEXTURE_2D );
            // blend
            glEnable( GL_BLEND );
            glBlendFunc( GL_DST_ALPHA, GL_ONE );
        }
        else
            glColor3f( 0.4f, 0.4f, 0.4f );
        glBegin( GL_QUADS );
        if( g_tex_id ) glTexCoord2f( 0.0f, 0.0f );
        glVertex2d( x, y );
        if( g_tex_id ) glTexCoord2f( 1.0f, 0.0f );
        glVertex2d( x + g_butter_width, y );
        if( g_tex_id ) glTexCoord2f( 1.0f, 1.0f );
        glVertex2d( x + g_butter_width, y + g_butter_height );
        if( g_tex_id ) glTexCoord2f( 0.0f, 1.0f );
        glVertex2d( x, y + g_butter_height );
        glEnd(); 
        if( g_tex_id ) {
            glDisable( GL_TEXTURE_2D );
            glDisable( GL_BLEND );
        }
        // end of instead
    }

    // draw rect
    glPushMatrix();
    glColor3f( 1.0f, 1.0f, 1.0f );
    glTranslatef( m_specgram.loc[0], m_specgram.loc[1], .05f );
    draw_rect( m_specgram );
    glPopMatrix();

    glPushMatrix();
    glTranslatef( 0.0f, 0.0f, .05f );
    glColor3f( 1.0f, .8f, 0.0f );
    glBegin( GL_LINES );
    glVertex2f( ui_elements[BT_SELECT_NOW].slide + xx, 0.0f + yy );
    glVertex2f( ui_elements[BT_SELECT_NOW].slide + xx, g_butter_height + yy );
    glEnd();
    glPopMatrix();

    // draw tracks?
    if( g_show_tracks && m_fin && driver && !driver->the_event().empty() ) // if there is at least one track
    {
        Track * tr;
        freqpolar trinfo;
        glColor3f( 1.0f, 0.5f, 0.0f );
        uint size = g_all_tracks ? driver->the_event().size() : driver->cur_event().size();
        for( int e = 0; e < size; e++ )
        {
            tr = g_all_tracks ? driver->the_event().at( e ) : driver->cur_event().at( e );
            if( !tr ) continue; 
            glBegin( GL_LINE_STRIP );
            for( int h = 0; h < tr->history.size(); h++ )
            {
                trinfo = tr->history[h];
                x = (t_CKSINGLE)trinfo.time / m_fin->info().frames * g_butter_width + xx;
                y = (t_CKSINGLE)trinfo.bin / BirdBrain::fft_size() * 2 * g_butter_height + yy; 
                glVertex3f( x, y, 1.0f ); 
            }
            glEnd(); 
        }
        glColor3f( 1.0f, 1.0f, 1.0f ); 
    }

	// draw rolloff? (for fft bandpass filter)
	if( m_fin && m_analysis_mode == ANA_RAW )
	{
		// values from sliders/butters
		t_CKSINGLE nyquist = BirdBrain::srate() / 2.0f;
		t_CKSINGLE start = ui_elements[BT_SELECT_LEFT].fvalue();
		t_CKSINGLE end = ui_elements[BT_SELECT_RIGHT].fvalue();
		t_CKSINGLE low = ui_elements[BT_FREQ_LEFT].fvalue();
		t_CKSINGLE high = ui_elements[BT_FREQ_RIGHT].fvalue();
		t_CKSINGLE rolloff = ui_elements[SL_RAW_ROLLOFF].fvalue();

		// corresponding points to draw on
		t_CKSINGLE x1 = start / m_fin->info().frames * g_butter_width + xx;
		t_CKSINGLE x2 = end / m_fin->info().frames * g_butter_width + xx;
		t_CKSINGLE y2 = high/nyquist + rolloff;
		if( y2 > 1 ) y2 = 1.0f;
		y2 = y2 * g_butter_height + yy; 
		t_CKSINGLE y1 = low/nyquist - rolloff;
		if( y1 < 0 ) y1 = 0.0f;
		y1 = y1 * g_butter_height + yy;

		// now draw
		glLineWidth(2.0);
		glColor3f( 1.0f, 0.8f, 0.8f );
		glBegin( GL_LINE_STRIP );
		glVertex3f( x1, y1, 1.0f );
		glVertex3f( x2, y1, 1.0f );
		glEnd();
		glBegin(GL_LINE_STRIP );
		glVertex3f( x1, y2, 1.0f );
		glVertex3f( x2, y2, 1.0f );
		glEnd();
		glLineWidth(1.0);
	}

    glPopName();
        
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    glEnable( GL_LIGHTING );
}


//-----------------------------------------------------------------------------
// name: render()
// desc: ...
//-----------------------------------------------------------------------------
t_CKUINT UIAnalysis::render( void * data )
{
    // common   
    float c[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLint i;

    if( !m_fin )
        draw_label( "original", -1.1, 0.06, 0.0, 1.0, false, c );
    else
        draw_label( BirdBrain::getbase( m_sndfilename.c_str() ), -1.1, 0.06, 0.0, 1.0, false, c );

	// hack but not bad at all (ahem): change label of separate button
	if( m_analysis_mode == ANA_FEAT )
		ui_elements[BT_SEPARATE].name = "seek";
	else
		ui_elements[BT_SEPARATE].name = ui_str[BT_SEPARATE];

    // draw buttons
    draw_button( ui_elements[BT_LOAD], .10f, 0.0f, 0.0f, .5f, .5f, 1.0f, IMG_LOAD );
    draw_button( ui_elements[BT_ORIG], .25f, 0.0f, 0.0f, .8f, .8f, 1.0f, IMG_PREV );
    draw_button( ui_elements[BT_PLAY], .40f, 0.0f, 0.0f, .5f, 1.0f, .5f, IMG_PLAY );
    draw_button( ui_elements[BT_STOP], .55f, 0.0f, 0.0f, 1.0f, 0.5f, .5, IMG_STOP );
    draw_button( ui_elements[BT_SPEC_SAVE], .7f, 0.0f, 0.0f, 1.0f, 1.0f, .5f, IMG_SAVE );
	draw_button( ui_elements[BT_SEPARATE], 1.05f, 0.0f, 0.0f, .9f, .9f, .9f, IMG_SEP );
    draw_button( ui_elements[BT_TOGGLE], 0, .8f, 0.0f, .5f, .5f, 1.0f, IMG_TOG );
    //draw_button( ui_elements[BT_ZOOM_IN], -1.0, 0.0, 0.0, 1.0, 1.0, .5 );
    //draw_button( ui_elements[BT_ZOOM_OUT], -.75, 0.0, 0.0, .4, 1.0, 1.0 );
    //draw_button( ui_elements[BT_REWIND], -0.63f, 0.07f, 0.0f, .9f, .9f, .9f );
    //draw_button( ui_elements[BT_FF], -0.57f, 0.07f, 0.0f, .5f, .5f, .5f );
    //draw_button( ui_elements[BT_CLASSIFIED], -.6f, -.8f, 0.0f, 0.3f, 0.3f, 0.3f );
    draw_button( ui_elements[BT_SYN_PLAY], -0.25f, -0.37f, 0.0f, .5f, 1.0f, .5f, IMG_PLAY );
    draw_button( ui_elements[BT_SYN_SAVE], -0.40f, -0.37f, 0.0f, 1.0f, 1.0f, .5f, IMG_SAVE );
    draw_button( ui_elements[BT_RES_PLAY], -0.25f, -0.8f, 0.0f, .5f, 1.0f, .5f, IMG_PLAY );
    draw_button( ui_elements[BT_RES_SAVE], -0.40f, -0.8f, 0.0f, 1.0f, 1.0f, .5f, IMG_SAVE );
    draw_button( ui_elements[BT_RES_LOAD], -0.80f, -0.8f, 0.0f, .5f, .5f, 1.0f, IMG_LOAD );
    draw_button( ui_elements[BT_NEXT_EVENT], -0.60f, -0.37f, 0.0f, .5f, .5f, .5f, IMG_NEXT );
    draw_button( ui_elements[BT_PREV_EVENT], -0.70f, -0.37f, 0.0f, .9f, .9f, .9f, IMG_PREV );
    
    // view area
    glBegin( GL_LINE_STRIP );
    glVertex3f( g_view_left + 0.85f, -1.0f, -0.1f );
    glVertex3f( g_view_left + 0.85f, -0.91f, -0.1f );
    glVertex3f( g_view_left, -0.91f, -0.1f );
    glVertex3f( g_view_left, -1.0f, -0.1f );
    glEnd();
    draw_label( "view", g_view_left - 0.1f, -0.99f, 0.0f, 1.0, false, c ); 

     // draw butters
    draw_lr_butter( ui_elements[BT_SELECT_LEFT], &ui_elements[BT_SELECT_NOW], ui_elements[BT_SELECT_RIGHT], 
                    -1.1f, .25f, 0.0f );
    if( !m_spec )
    {
        draw_lr_butter( ui_elements[BT_FREQ_LEFT], NULL, ui_elements[BT_FREQ_RIGHT], 
                        .1, .25, 0.0 );
        draw_label( "frequency", .5f, .187f, -0.5f, 1.0f, false, c );
    }
    else
    {
        // spectrogram
        spectrogram( m_specgram.loc[0], m_specgram.loc[1] );
        // sliders
        draw_slider_h_mini( ui_elements[SL_BRIGHTNESS], .1, .15, 0.0 );
        draw_slider_h_mini( ui_elements[SL_CONTRAST], .7, .15, 0.0 );

        draw_label( "spectrogram", .5, .187, 0.0, 1.0f, false, c );
    }

    if( !m_fin )
    {
        draw_label( "(no files loaded)", -.6, .55, 0.0, 1.0, true, c );
    }

    // drawing waveforms, fft ...
    
    // if file open
    if( m_fin )
    {
        // apply the window
        GLfloat x = -1.1f, inc = g_butter_width/ m_waveform_size, y = .55f;
        glDisable( GL_LIGHTING );
        // color waveform
        glColor3f( 0.4f, 0.4f, 1.0f );

        // draw the time domain waveform
        glBegin( GL_LINE_STRIP );
        GLint ii = ( m_waveform_size - (m_waveform_size/m_time_view) ) / 2;
        for( i = ii; i < ii + m_waveform_size / m_time_view; i++ )
        {
            glVertex3f( x, m_gain * m_time_scale * g_butter_height / .5f * .25f * m_waveform[i] + y, 0.0f );
            x += inc * m_time_view;
        }
        glEnd();

        // fft
        const complex * cbuf = NULL;
        const Frame * f = NULL;
        // if there is a driver, use its frames
        if( driver != NULL )
        {
            uint time_sample = (t_CKUINT)(ui_elements[BT_SELECT_NOW].fvalue());
            f = driver->get_frame( time_sample );
            if( f ) cbuf = f->cmp; 
        }        
        // read from file
        if( cbuf == NULL )
        {
            m_fin->seek( (t_CKUINT)(ui_elements[BT_SELECT_NOW].fvalue() + .5), SEEK_SET );
            //m_fin->seek( (t_CKUINT)(ui_elements[BT_SELECT_NOW].slide * m_fin->info().frames + .5), SEEK_SET );
            m_fin->mtick( m_buffer, m_wnd_size );
            // window and zero pad
            apply_window( m_buffer, m_window, m_wnd_size );
            for( i = m_wnd_size; i < m_fft_size; i++ )
                m_buffer[i] = 0.0f;
            rfft( m_buffer, m_fft_size/2, FFT_FORWARD );
            // scale from zero padding
            BirdBrain::scale_fft( m_buffer, m_fft_size, m_fft_size, m_wnd_size );
            cbuf = (complex *)m_buffer;
        }

        // not NULL - draw
        if( cbuf != NULL )
        {
            // don't draw spectrogram
            if( !m_spec )
            {
                x = .1f;
                y = .28f;
                inc = g_butter_width/ (GLfloat)m_fft_size;

                t_CKSINGLE fval = 1;
                glColor3f( .4f * fval, 1.0f * fval, .4f * fval );

                t_CKSINGLE yy;
                glBegin( GL_LINE_STRIP );
                for( int j = 0; j < m_fft_size/m_freq_view; j++ )
                {
                    yy = m_gain * m_freq_scale * g_butter_height / .5f * 
                            ::pow( (double) 25 * cmp_abs( cbuf[j] ), 0.5 ) + y;
                    glVertex3f( x + j*(inc*m_freq_view), yy, 0.0 );
                }
                glEnd();
            }
        }


        glEnable( GL_LIGHTING );
    }


    // for rotation of sliders
    g_r += 1;


    // specific rendering
    t_CKUINT ret;
    switch( m_which_pane )
    {
    case SINE_PANE:
        ret = render_sine_pane( data );
        break;
    case TRAN_PANE: 
        ret = render_tran_pane( data );
        break;
    case GROUP_PANE:
        ret = render_group_pane( data );
        break;
    case LIB_PANE: 
        ret = render_lib_pane( data );
        break;
	case RAW_PANE:
		ret = render_raw_pane( data );
		break;
	case FEAT_PANE:
		ret = render_features_pane( data );
		break;
    default:
        ret = render_lib_pane( data );
        break;
    }

    switch( m_analysis_mode )
    {
    case ANA_DET:
        ret = render_det_mode( data ) && ret;
        break;
    case ANA_TRAN:
        ret = render_tran_mode( data ) && ret;
        break;
	case ANA_RAW:
		ret = render_raw_mode( data ) && ret;
		break;
	case ANA_FEAT:
		ret = render_features_mode( data ) && ret;
		break;
    default:
        ret = render_det_mode( data ) && ret;
        break;
    }

    // highlighting
    if( m_highlight && m_cur_bbox >= 0 && m_cur_bbox < m_nbboxs )
    {
        Point3D p1 = m_bboxs[m_cur_bbox].pmin();
        Point3D p2 = m_bboxs[m_cur_bbox].pmax();
        glDisable( GL_LIGHTING );
        glPushMatrix();
            glTranslatef( 0.0f, 0.0f, -0.5f );
            glColor3f( 0.075f, 0.075f, 0.075f );
            glBegin( GL_QUADS );
            glVertex2f( p1[0], p1[1] );
            glVertex2f( p2[0], p1[1] );
            glVertex2f( p2[0], p2[1] );
            glVertex2f( p1[0], p2[1] );
            glEnd();
        glPopMatrix();
        glEnable( GL_LIGHTING );
    }

    return ret;
}



//-----------------------------------------------------------------------------
// name: render_tran_mode()
// desc: draw transients and residue and other stuff related to transient mode
//-----------------------------------------------------------------------------
t_CKUINT UIAnalysis::render_tran_mode( void * data )
{
    float c[] = { 1.0f, 1.0f, .5f, 1.0f };

    // labels
    draw_label( "transient", -1.1, -0.39, 0.0, 1.0, false, c );
    draw_label( "residue", -1.1, -.82, 0.0, 1.0, false, c );

    // draw stuff if file is open
    if( m_fin )
    {
        glDisable( GL_LIGHTING );

        GLfloat x = -1.1f, inc, y = .55f, skip, i, y1, y2, x1, x2;

        // envelope
        SAMPLE * env;
        if( ee != NULL && (env = ee->getEnv()) != NULL )
        {   
            // envelope
            // only for envelope based
            if( ui_elements[BT_TRAN_METHOD].slide <= 0.5 /*&& !g_switch_method*/ )
            {
                EnvExtractor * my_ee = (EnvExtractor *) ee;
                glColor3f( 1.0f, 0.4f, 0.4f );
                        
                inc = g_butter_width/ m_waveform_size; 
                skip = (GLfloat)m_fin->info().frames / m_waveform_size;

                glBegin( GL_LINE_STRIP );
                //GLint ii = ( m_waveform_size - (m_waveform_size/m_time_view) ) / 2;
                //for( i = ii; i < ii + m_waveform_size / m_time_view; i++ )
                x += inc * my_ee->estart / skip;
                for( i = 0; i < my_ee->envLen; i += skip )
                {
                    glVertex3f( x, m_gain * m_time_scale * g_butter_height / .5f * .25f * env[(int)i] + y, 0.1f );
                    x += inc;
                }
                glEnd();
            }

            // mark transients
            glColor3f( 1.0f, 1.0f, 0.4f );
            for( int t = 0; t < ee->transients.size() /*&& !g_switch_method*/; t++ )
            {
                // mark( ee->transients[t].start );
                x1 = (GLfloat)( ee->transients[t].start + ee->estart ) / m_fin->info().frames * g_butter_width - 1.1f;
                x2 = (GLfloat)( ee->transients[t].end + ee->estart ) / m_fin->info().frames * g_butter_width - 1.1f;
                y1 = 100000.0f;
                y2 = -100000.0f;
                GLint start = ee->transients[t].start;
                GLint end = ee->transients[t].end;

                // rectangle bounds based on method
                if( ui_elements[BT_TRAN_METHOD].slide <= 0.5 )
                {
                    for( int z = start; z < end; z++ )
                    {
                         if( y1 > env[z] ) y1 = env[z];
                         if( y2 < env[z] ) y2 = env[z];
                    }
                    y1 = y1 * m_gain * m_time_scale * g_butter_height / .5f * .25f + y;
                    y2 = y2 * m_gain * m_time_scale * g_butter_height / .5f * .25f + y;
                }
                else
                {
                    y1 = m_gain * m_time_scale * g_butter_height / .5f * .25f + y;
                    y2 = -m_gain * m_time_scale * g_butter_height / .5 * .25f + y;
                }

                if( t == m_curr_tran )
                {
                    glLineWidth( 2.0 );
                    glColor3f( .4f, 1.0f, .4f );
                }

                glBegin( GL_LINE_LOOP );
                glVertex3f( x1, y1, 0.2f );
                glVertex3f( x2, y1, 0.2f );
                glVertex3f( x2, y2, 0.2f );
                glVertex3f( x1, y2, 0.2f );
                glEnd();

                if( t == m_curr_tran ) // and also otherwise
                {
                    glLineWidth( 1.0 );
                    glColor3f( 1.0f, 1.0f, 0.4f );
                }
            }

            // draw waveform of selected transient?
            if( m_curr_tran != -1 )
            {
                x = -1.1f;
                y = -.20f; //-.30f;

                Frame fr;
                read_transient( &fr );

                inc = g_butter_width / fr.wsize; 

                t_CKSINGLE fval = 1;
                glColor3f( .4f * fval, .4f * fval, 1.0f * fval );

                t_CKSINGLE yy;
                glBegin( GL_LINE_STRIP );
                for( int j = 0; j < fr.wsize / m_time_view; j++ )
                {
                    yy = 0.25 * m_gain * m_time_scale * g_butter_height / .5 *
                        fr.waveform[j] + y;
                    glVertex3f( x + j * inc * m_time_view, yy, 0.0 );
                }
                glEnd();
            }

            // draw residue waveform????
            if( g_tran_suppressed )
            {
                x = -1.1f;
                y = -.65f;
            
                AudioSrcFile * sup = new AudioSrcFile;
                sup->open( g_tran_outfile.c_str(), 0, 0, FALSE );
                int size = sup->info().frames;
                SAMPLE * sup_waveform = new SAMPLE[ size ];
                sup->mtick( sup_waveform, size );

                inc = g_butter_width / size;

                t_CKSINGLE fval = 1;
                glColor3f( .4f * fval, .4f * fval, 1.0f * fval );

                t_CKSINGLE yy;
                glBegin( GL_LINE_STRIP );
                for( int j = 0; j < size / m_time_view; j++ )
                {
                    yy = 0.3 * m_gain * m_freq_scale * g_butter_height / .5f *
                        sup_waveform[j] + y;
                    glVertex3f( x + j * inc * m_time_view, yy, 0.0 );
                }
                glEnd();

                SAFE_DELETE( sup );
                SAFE_DELETE( sup_waveform );
            }       
        }

        glEnable( GL_LIGHTING );
    }

    return 0;
}


//-----------------------------------------------------------------------------
// name: render_tran_pane()
// desc: draw sliders associated with transient analysis
//-----------------------------------------------------------------------------
t_CKUINT UIAnalysis::render_tran_pane( void * data )
{   
    // viewing buttons
    draw_button( ui_elements[BT_VIEW_SINE], g_view_left + 0.05f, -0.92f, 0.0f, .8f, .5f, .5f );
    draw_button( ui_elements[BT_VIEW_GROUP], g_view_left + 0.2f, -0.92f, 0.0f, .8f, .5f, .5f );
    draw_button( ui_elements[BT_VIEW_TRAN], g_view_left + 0.35f, -0.92f, 0.0f, 1.0f, .3f, .3f );
	draw_button( ui_elements[BT_VIEW_RAW], g_view_left + 0.5f, -0.92f, 0.0f, .8f, .5f, .5f );
	draw_button( ui_elements[BT_VIEW_FEATURES], g_view_left + 0.65f, -0.92f, 0.0f, .8f, .5f, .5f );
    draw_button( ui_elements[BT_VIEW_LIB], g_view_left + 0.8f, -0.92f, 0.0f, .8f, .5f, .5f );

    // flipper
    draw_flipper( ui_elements[BT_TRAN_METHOD], .8, -.8, 0.0 );
    
    // sliders
    if( ui_elements[BT_TRAN_METHOD].slide > 0.5 ) // envelope
    {
        draw_slider_h( ui_elements[SL_TRAN_LONG], 0, -.25, 0.0 );
        draw_slider_h( ui_elements[SL_TRAN_THRESH], 0, -.50, 0.0 );
        draw_slider_h( ui_elements[SL_TRAN_MAX_LENGTH], 0, -.75, 0.0 );
        draw_slider_h( ui_elements[SL_TRAN_SHORT], .6, -.25, 0.0 );
        draw_slider_h( ui_elements[SL_TRAN_GAP], .6, -.5, 0.0 );
    }
    else // energy ratio
    {
        draw_slider_h( ui_elements[SL_TRAN_ATTACK], 0, -.25, 0.0 );
        draw_slider_h( ui_elements[SL_TRAN_THRESH], 0, -.50, 0.0 );
        draw_slider_h( ui_elements[SL_TRAN_AGEAMT], 0, -.75, 0.0 );
        draw_slider_h( ui_elements[SL_TRAN_DECAY], .6, -.25, 0.0 );
        draw_slider_h( ui_elements[SL_TRAN_GAP], .6, -.5, 0.0 );
    }

    return 0;
}


//-----------------------------------------------------------------------------
// name: render_det_mode()
// desc: draw information related to deterministic analysis
//-----------------------------------------------------------------------------
t_CKUINT UIAnalysis::render_det_mode( void * data )
{
    GLint i;
    float c[] = { 1.0f, 1.0f, .5f, 1.0f };
    
    const complex * cbuf = NULL;
    const Frame * f = NULL;
    t_CKSINGLE inc; 

    // labels
    draw_label( "sinusoidal", -1.1, -0.39, 0.0, 1.0, false, c );
    draw_label( "stochastic", -1.1, -.82, 0.0, 1.0, false, c );

    // button
    draw_button( ui_elements[BT_ALL], -0.85f, -0.37f, 0.0f, .6f, .6f, 0.8f, IMG_ALL );
    
    // flipper (hopefully temp)
    draw_flipper( ui_elements[BT_CLIP_ONLY], -0.6f, -0.84f, 0.0f );
    draw_flipper( ui_elements[BT_GROUP], .865f, -0.05f, 0.0f );
    
    // IN ALL CASES: compute threshold
    t_CKSINGLE threshold = ::pow( ui_elements[SL_DET_THRESH].fvalue(), THRESH_POW );
    // apply scaling to threshold
    threshold = m_gain * m_freq_scale * g_butter_height / .5f * ::pow( (double) 25 * threshold, 0.5 );
    
    // destination x and y
    float x = 0.0f, y = 0.0f;
    // width of the display
    float a = g_butter_width;
    // theta
    float theta = ui_elements[KN_THRESH_TILT].fvalue() * PIE / 180.0;
    // horizontal displacement from threshold (at right boundary)
    float b = a * ::tan( theta );
    // figure out if clipping if needed
    if( theta < 0.0 && (threshold + b) < 0.0 )
    {
        // under the display floor
        x = -threshold * a / b;
        y = 0.0f;
    }
    else if( theta > 0.0 && fabs( b ) > g_butter_height - threshold )
    {
        // above display ceiling...
        x = (g_butter_height - threshold) * a / b;
        y = g_butter_height;
    }
    else 
    {
        // in between
        x = g_butter_width;
        y = threshold + b;
    }

    double delta_y = b; //y - threshold;
    // delta_y = (delta_y < 0.0f ? -1 : 1) * ::pow( delta_y / (m_gain * m_freq_scale * g_butter_height * 2), 2.0 ) / 25.0;
    double delta_x = m_fft_size / 2.0; //x / (float)g_butter_width * m_fft_size / 2;
    // figure out the actual slope for analysis (bin # on x axis vs. linear magnitude on y axis)
    m_threshold->intercept = threshold;
    m_threshold->slope = delta_y / delta_x;
    // END of threshold computation
	

    // draw the threshold line and the tilt slider
    if( !m_spec )
    {   
        // the tilt
        draw_slider_mini( ui_elements[KN_THRESH_TILT], 0.0f, .3f, 0.0f );

        // the line
        glColor3f( 1.0f, .5f, .5f );
        glPushMatrix();
        glTranslatef( .1f, .28f, 0.0f );
        glDisable( GL_LIGHTING );
        glBegin( GL_LINES );
        glVertex2f( 0.0f, threshold );
        glVertex2f( x, y );
        glEnd();
        glEnable( GL_LIGHTING );
        glPopMatrix();
    }
   

    // if file is open
    if( m_fin )
    {
        glDisable( GL_LIGHTING );
        
        // drawing the resynthesis and residue
        if( driver != NULL )
        {
            // truly bad
            cbuf = NULL;
            SAMPLE * resyn = NULL;
            // the time
            uint time_sample = (t_CKUINT)(ui_elements[BT_SELECT_NOW].fvalue());
            // get fft frame
            f = driver->get_frame( time_sample ); 
            // if time is within range of this synthesized frame
            if( f && time_sample >= frame->time && (time_sample + m_wnd_size) <= (frame->time + frame->wlen) )
            {
                // if it's still within range
                if( f->time >= frame->time ) // used to be only if( f ) but that's now checked above
                {
                    // get the frame from the resynthesis
                    assert( frame != NULL );
                    assert( m_wnd_size == driver->get_window()->wlen );
                    resyn = new SAMPLE[BirdBrain::fft_size()];
                    memset( resyn, 0, BirdBrain::fft_size() * sizeof(SAMPLE) ); 
                    memcpy( resyn, &frame->waveform[(int)(f->time - frame->time)], m_wnd_size * sizeof(SAMPLE) ); // danger comment: timeout
                    apply_window( resyn, driver->get_window()->waveform, driver->get_window()->wlen );
                    // apply_window( resyn, m_window, m_wnd_size );
                    for( i = m_wnd_size; i < m_fft_size; i++ )
                        resyn[i] = 0.0f;
                    // FFT
                    rfft( resyn, BirdBrain::fft_size()/2, FFT_FORWARD );
                    // scale from zero padding
                    BirdBrain::scale_fft( resyn, m_fft_size, m_fft_size, m_wnd_size );
                    cbuf = (complex *)resyn;
                }
            } // (maybe) end truly bad

            // draw resynthesis
            if( cbuf )
            {
                x = -1.1f;
                y = -.30f;
                inc = g_butter_width/ m_fft_size;

                t_CKSINGLE fval = 1;
                glColor3f( .4f * fval, 1.0f * fval, .4f * fval );

                t_CKSINGLE yy;
                glBegin( GL_LINE_STRIP );
                for( int j = 0; j < m_fft_size/m_freq_view; j++ )
                {
                    yy = m_gain * m_freq_scale * g_butter_height / .5f *
                            ::pow( (double) 25 * cmp_abs( cbuf[j] ), 0.5 ) + y;
                    glVertex3f( x + j*(inc*m_freq_view), yy, 0.0 );
                }
                glEnd();
            }

            // very bad [ THIS IS THE CLASSIFIED INFORMATION ]
            cbuf = NULL; 
            Frame resframe;
            if( f && ui_elements[BT_CLASSIFIED].slide < 0.5f ) {
                // make copy
                resframe = *f;
                // get residue
                driver->ana()->get_res( driver->the_event(), resframe );
                resframe.pol2cmp();
                cbuf = resframe.cmp;
            }
            else if( f ) {
                // bad
                uint time_sample = (t_CKUINT)(ui_elements[BT_SELECT_NOW].fvalue());
                f = driver->get_fake_residue( time_sample );
                if( f ) {
                    resframe = *f;
                    resframe.pol2cmp();
                    cbuf = resframe.cmp;
                }
            }

            // draw residue
            if( cbuf )
            {
                x = -1.1f;
                y = -.75f;
                inc = g_butter_width/ m_fft_size;

                t_CKSINGLE fval = 1;
                glColor3f( .4f * fval, 1.0f * fval, .4f * fval );

                t_CKSINGLE yy;
                glBegin( GL_LINE_STRIP );
                for( int j = 0; j < m_fft_size/m_freq_view; j++ )
                {
                    yy = m_gain * m_freq_scale * g_butter_height / .5f *
                            ::pow( (double) 25 * cmp_abs( cbuf[j] ), 0.5 ) + y;
                    glVertex3f( x + j*(inc*m_freq_view), yy, 0.0 );
                }
                glEnd();
            }

            // delete resyn
            SAFE_DELETE_ARRAY( resyn );
        }

        glEnable( GL_LIGHTING );
    }

    return 0;
}



//-----------------------------------------------------------------------------
// name: render_sine_pane()
// desc: draw sliders associated with basic sinusoidal analysis
//-----------------------------------------------------------------------------
t_CKUINT UIAnalysis::render_sine_pane( void * data )
{
    // viewing buttons
    draw_button( ui_elements[BT_VIEW_SINE], g_view_left + 0.05f, -0.92f, 0.0f, 1.0f, .3f, .3f );
    draw_button( ui_elements[BT_VIEW_GROUP], g_view_left + 0.2f, -0.92f, 0.0f, .8f, .5f, .5f );
    draw_button( ui_elements[BT_VIEW_TRAN], g_view_left + 0.35f, -0.92f, 0.0f, .8f, .5f, .5f );
    draw_button( ui_elements[BT_VIEW_RAW], g_view_left + 0.5f, -0.92f, 0.0f, .8f, .5f, .5f );
	draw_button( ui_elements[BT_VIEW_FEATURES], g_view_left + 0.65f, -0.92f, 0.0f, .8f, .5f, .5f );
    draw_button( ui_elements[BT_VIEW_LIB], g_view_left + 0.8f, -0.92f, 0.0f, .8f, .5f, .5f );

    // draw slider
    draw_slider_h( ui_elements[SL_DET_TRACKS], 0, -.25, 0.0 );
    draw_slider_h( ui_elements[SL_DET_MINPOINTS], 0, -.50, 0.0 );
    draw_slider_h( ui_elements[SL_DET_MAXGAP], 0, -.75, 0.0 );
    draw_slider_h( ui_elements[SL_DET_THRESH], .6, -.25, 0.0 );
    draw_slider_h( ui_elements[SL_DET_ERROR_FREQ], .6, -.5, 0.0 );
    draw_slider_h( ui_elements[SL_DET_NOISE_RATIO], .6, -.75, 0.0 );
   
    return 0;
}



//-----------------------------------------------------------------------------
// name: render_group_pane()
// desc: draw sliders associated with grouping
//-----------------------------------------------------------------------------
t_CKUINT UIAnalysis::render_group_pane( void * data )
{
    // viewing buttons
    draw_button( ui_elements[BT_VIEW_SINE], g_view_left + 0.05f, -0.92f, 0.0f, .8f, .5f, .5f );
    draw_button( ui_elements[BT_VIEW_GROUP], g_view_left + 0.2f, -0.92f, 0.0f, 1.0f, .3f, .3f );
    draw_button( ui_elements[BT_VIEW_TRAN], g_view_left + 0.35f, -0.92f, 0.0f, .8f, .5f, .5f );
    draw_button( ui_elements[BT_VIEW_RAW], g_view_left + 0.5f, -0.92f, 0.0f, .8f, .5f, .5f );
	draw_button( ui_elements[BT_VIEW_FEATURES], g_view_left + 0.65f, -0.92f, 0.0f, .8f, .5f, .5f );
    draw_button( ui_elements[BT_VIEW_LIB], g_view_left + 0.8f, -0.92f, 0.0f, .8f, .5f, .5f );

    // draw slider
    draw_slider_h( ui_elements[SL_GROUP_HARM], 0, -.25, 0.0 );
    draw_slider_h( ui_elements[SL_GROUP_MOD], 0, -.50, 0.0 );
    draw_slider_h( ui_elements[SL_GROUP_OVERLAP], 0, -.75, 0.0 );
    draw_slider_h( ui_elements[SL_GROUP_ONSET], .6, -.25, 0.0 );
    draw_slider_h( ui_elements[SL_GROUP_OFFSET], .6, -.5, 0.0 );
    draw_slider_h( ui_elements[SL_GROUP_MINLEN], .6, -.75, 0.0 );

    return 0;
}


//-----------------------------------------------------------------------------
// name: render_raw_mode()
// desc: what to draw on left in raw mode?
//-----------------------------------------------------------------------------
t_CKUINT UIAnalysis::render_raw_mode( void * data )
{
	GLint i;
    float c[] = { 1.0f, 1.0f, .5f, 1.0f };
	float x = 0.0, y = 0.0;
    
    const complex * cbuf = NULL;
    const Frame * f = NULL;
    t_CKSINGLE inc; 

    // labels
    draw_label( "extracted", -1.1, -0.39, 0.0, 1.0, false, c );
    draw_label( "nothing", -1.1, -.82, 0.0, 1.0, false, c );

	// flipper (hopefully temp)
    draw_flipper( ui_elements[BT_CLIP_ONLY], -0.6f, -0.84f, 0.0f );
    
    // IN ALL CASES: show rolloff?
	if( !m_spec )
    {   
		// get values
		t_CKSINGLE nyquist = BirdBrain::srate() / 2.0f;
		t_CKSINGLE low = ui_elements[BT_FREQ_LEFT].fvalue();
		t_CKSINGLE high = ui_elements[BT_FREQ_RIGHT].fvalue();
		t_CKSINGLE rolloff = ui_elements[SL_RAW_ROLLOFF].fvalue();

		// points to draw from
		float x1 = low/nyquist - rolloff;
		if( x1 < 0 ) x1 = 0.0f;
		x1 *= g_butter_width;
		float x2 = high/nyquist + rolloff;
		if( x2 > 1 ) x2 = 1.0f;
		x2 *= g_butter_width;

        // the line
        glColor3f( 1.0f, .8f, .8f );
        glPushMatrix();
        glTranslatef( .1f, .25f, 0.0f );
        glDisable( GL_LIGHTING );
        glBegin( GL_LINES );
        glVertex2f( x1, 0.0f );
        glVertex2f( x1, g_butter_height );
        glEnd();
		glBegin( GL_LINES );
		glVertex2f( x2, 0.0f );
		glVertex2f( x2, g_butter_height );
		glEnd();
        glEnable( GL_LIGHTING );
        glPopMatrix();
    }

    // if file is open
    if( m_fin )
    {
		// maybe show ffts
		complex * cbuf = NULL;
		if( rf_specbounds[0] >= 0 )
		{
			// get samples
			rf_display->seek( (t_CKUINT)(ui_elements[BT_SELECT_NOW].fvalue() - ui_elements[BT_SELECT_LEFT].fvalue() + .5), 
							  SEEK_SET );
			rf_display->mtick( rf_buffer, m_wnd_size );
			// window and zero pad
			apply_window( rf_buffer, m_window, m_wnd_size );
			for( i = m_wnd_size; i < m_fft_size; i++ )
				rf_buffer[i] = 0.0f;
			rfft( rf_buffer, m_fft_size/2, FFT_FORWARD );
			// scale from zero padding
			BirdBrain::scale_fft( rf_buffer, m_fft_size, m_fft_size, m_wnd_size );
			cbuf = (complex *)rf_buffer;
		}		
		// draw them ffts
		glDisable( GL_LIGHTING );
        if( cbuf ) // expected to be true always
        {
            x = -1.1f;
            y = -.30f;
            inc = g_butter_width/ m_fft_size;

            t_CKSINGLE fval = 1;
            glColor3f( .4f * fval, 1.0f * fval, .4f * fval );

            t_CKSINGLE yy;
            glBegin( GL_LINE_STRIP );
            for( int j = 0; j < m_fft_size/m_freq_view; j++ )
            {
                yy = m_gain * m_freq_scale * g_butter_height / .5f *
                        ::pow( (double) 25 * cmp_abs( cbuf[j] ), 0.5 ) + y;
                glVertex3f( x + j*(inc*m_freq_view), yy, 0.0 );
            }
            glEnd();
        }
        glEnable( GL_LIGHTING );
    }

    return 0;
}


//-----------------------------------------------------------------------------
// name: render_raw_pane()
// desc: draw raw extraction rolloff
//-----------------------------------------------------------------------------
t_CKUINT UIAnalysis::render_raw_pane( void * data )
{
    // viewing buttons
    draw_button( ui_elements[BT_VIEW_SINE], g_view_left + 0.05f, -0.92f, 0.0f, .8f, .5f, .5f );
    draw_button( ui_elements[BT_VIEW_GROUP], g_view_left + 0.2f, -0.92f, 0.0f, .8f, .5f, .5f );
    draw_button( ui_elements[BT_VIEW_TRAN], g_view_left + 0.35f, -0.92f, 0.0f, .8f, .5f, .5f );
	draw_button( ui_elements[BT_VIEW_RAW], g_view_left + 0.5f, -0.92f, 0.0f, 1.0f, .3f, .3f );
	draw_button( ui_elements[BT_VIEW_FEATURES], g_view_left + 0.65f, -0.92f, 0.0f, .8f, .5f, .5f );
    draw_button( ui_elements[BT_VIEW_LIB], g_view_left + 0.8f, -0.92f, 0.0f, .8f, .5f, .5f );

	// draw slider
    draw_slider_h( ui_elements[SL_RAW_ROLLOFF], 0, -.25, 0.0 );

    return 0;
}



//-----------------------------------------------------------------------------
// name: render_features_mode()
// desc: draw similar found regions in original sound file
//-----------------------------------------------------------------------------
t_CKUINT UIAnalysis::render_features_mode( void * data )
{
    float c[] = { 1.0f, 1.0f, .5f, 1.0f };

    // labels
    draw_label( "matches", -1.1, -0.39, 0.0, 1.0, false, c );
    draw_label( "nothing", -1.1, -.82, 0.0, 1.0, false, c );

    // draw stuff if file is open
    if( m_fin )
    {
        glDisable( GL_LIGHTING );

        GLfloat x = -1.1f, inc, y = .55f, y1, y2, x1, x2;

        if( rc && !rc_segments.empty() ) // feature extraction stuff's working
        {   
            // mark transients
            glColor3f( 1.0f, 1.0f, 0.4f );
            for( int t = 0; t < rc_segments.size(); t++ )
            {
				//BB_log( BB_LOG_INFO, "%i / %i", rc_segments[t], m_fin->info().frames ); 
				// mark( start );
				GLint start = rc_segments[t];
                GLint end = start + rc_regionsize;
				if( end >= m_fin->info().frames ) end = m_fin->info().frames;
                x1 = (GLfloat)( start ) / m_fin->info().frames * g_butter_width - 1.1f;
                x2 = (GLfloat)( end ) / m_fin->info().frames * g_butter_width - 1.1f;
                y1 = m_gain * m_time_scale * g_butter_height / .5f * .25f + y;
                y2 = -m_gain * m_time_scale * g_butter_height / .5 * .25f + y;

                if( t == rc_curr_segment )
                {
                    glLineWidth( 2.0 );
                    glColor3f( .4f, 1.0f, .4f );
                }

                glBegin( GL_LINE_LOOP );
                glVertex3f( x1, y1, 0.2f );
                glVertex3f( x2, y1, 0.2f );
                glVertex3f( x2, y2, 0.2f );
                glVertex3f( x1, y2, 0.2f );
                glEnd();

                if( t == rc_curr_segment ) // and also otherwise 
                {
                    glLineWidth( 1.0 );
                    glColor3f( 1.0f, 1.0f, 0.4f );
                }
            }

            // draw waveform of selected transient?
            if( rc_curr_segment != -1 )
            {
                x = -1.1f;
                y = -.20f; //-.30f;

                Frame fr;
                read_segment( &fr );

                inc = g_butter_width / fr.wsize; 

                t_CKSINGLE fval = 1;
                glColor3f( .4f * fval, .4f * fval, 1.0f * fval );

                t_CKSINGLE yy;
                glBegin( GL_LINE_STRIP );
                for( int j = 0; j < fr.wsize / m_time_view; j++ )
                {
                    yy = 0.25 * m_gain * m_time_scale * g_butter_height / .5 *
                        fr.waveform[j] + y;
                    glVertex3f( x + j * inc * m_time_view, yy, 0.0 );
                }
                glEnd();
            } 
        }

        glEnable( GL_LIGHTING );
    }

    return 0;
}



//-----------------------------------------------------------------------------
// name: render_features_pane()
// desc: draw feature finding
//-----------------------------------------------------------------------------
t_CKUINT UIAnalysis::render_features_pane( void * data )
{
    // viewing buttons
    draw_button( ui_elements[BT_VIEW_SINE], g_view_left + 0.05f, -0.92f, 0.0f, .8f, .5f, .5f );
    draw_button( ui_elements[BT_VIEW_GROUP], g_view_left + 0.2f, -0.92f, 0.0f, .8f, .5f, .5f );
    draw_button( ui_elements[BT_VIEW_TRAN], g_view_left + 0.35f, -0.92f, 0.0f, .8f, .5f, .5f );
	draw_button( ui_elements[BT_VIEW_RAW], g_view_left + 0.5f, -0.92f, 0.0f, .8f, .5f, .5f );
	draw_button( ui_elements[BT_VIEW_FEATURES], g_view_left + 0.65f, -0.92f, 0.0f, 1.0f, .3f, .3f );
    draw_button( ui_elements[BT_VIEW_LIB], g_view_left + 0.8f, -0.92f, 0.0f, .8f, .5f, .5f );

	// draw slider
	draw_slider_h( ui_elements[SL_FEAT_THRESH], 0, -.25, 0.0 );
    draw_slider_h( ui_elements[SL_FEAT_B], 0, -.50, 0.0 );
    draw_slider_h( ui_elements[SL_FEAT_D], 0, -.75, 0.0 );
    draw_slider_h( ui_elements[SL_FEAT_A], .6, -.25, 0.0 );
    draw_slider_h( ui_elements[SL_FEAT_C], .6, -.5, 0.0 );
    draw_slider_h( ui_elements[SL_FEAT_E], .6, -.75, 0.0 );

    return 0;
}



//-----------------------------------------------------------------------------
// name: render_lib_pane()
// desc: draw library templates or not
//-----------------------------------------------------------------------------
t_CKUINT UIAnalysis::render_lib_pane( void * data )
{
    // viewing buttons
    draw_button( ui_elements[BT_VIEW_SINE], g_view_left + 0.05f, -0.92f, 0.0f, .8f, .5f, .5f );
    draw_button( ui_elements[BT_VIEW_GROUP], g_view_left + 0.2f, -0.92f, 0.0f, .8f, .5f, .5f );
    draw_button( ui_elements[BT_VIEW_TRAN], g_view_left + 0.35f, -0.92f, 0.0f, .8f, .5f, .5f );
	draw_button( ui_elements[BT_VIEW_RAW], g_view_left + 0.5f, -0.92f, 0.0f, .8f, .5f, .5f );
	draw_button( ui_elements[BT_VIEW_FEATURES], g_view_left + 0.65f, -0.92f, 0.0f, .8f, .5f, .5f );
    draw_button( ui_elements[BT_VIEW_LIB], g_view_left + 0.8f, -0.92f, 0.0f, 1.0f, .3f, .3f );

    // draw templates?
    Spectre textcolor;
    textcolor.r = textcolor.g = textcolor.b = 1.0f; 
    draw_library( NULL, NULL, 1.2f-1.05f, 0.0f-0.8f, 2.0f-1.05f, 1.0f-0.8f, &textcolor );

    return 0;
}




//-----------------------------------------------------------------------------
// name: render_pre()
// desc: ...
//-----------------------------------------------------------------------------
void UIAnalysis::render_pre()
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
void UIAnalysis::render_post()
{
    glPopAttrib();

    AudicleFace::render_post();
}




//-----------------------------------------------------------------------------
// name: render_view()
// desc: ...
//-----------------------------------------------------------------------------
void UIAnalysis::render_view( )
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
    
    m_asp = AudicleWindow::main()->m_hsize / AudicleWindow::main()->m_vsize;
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
t_CKUINT UIAnalysis::on_activate()
{
    g_text_color[0] = 1.0f;
    g_text_color[1] = 1.0f;
    g_text_color[2] = 1.0f;
    return AudicleFace::on_activate();
}




//-----------------------------------------------------------------------------
// name: on_deactivate()
// desc: ...
//-----------------------------------------------------------------------------
t_CKUINT UIAnalysis::on_deactivate( t_CKDUR dur )
{
    return AudicleFace::on_deactivate( dur );
}




//-----------------------------------------------------------------------------
// name: play_transient()
// desc: ....
//-----------------------------------------------------------------------------
void UIAnalysis::play_transient()
{
    AudioSrcFile * file = new AudioSrcFile;
    if( ee && !ee->transients.empty() )
    {
        file->open( m_sndfilename.c_str(), ee->estart + ee->transients[m_curr_tran].start, 
            ee->transients[m_curr_tran].end - ee->transients[m_curr_tran].start + 1, 0 );
        m_audio->bus(0)->play( file, TRUE, FALSE );
    }
    else
    {
        msg_box( "bad", "separate first" );
    }
}


//-----------------------------------------------------------------------------
// name: read_transient()
// desc: ....
//-----------------------------------------------------------------------------
void UIAnalysis::read_transient( Frame * fr )
{
    AudioSrcFile * file = new AudioSrcFile;
    uint start = ee->estart + ee->transients[m_curr_tran].start;
    uint len = ee->transients[m_curr_tran].end - ee->transients[m_curr_tran].start + 1;
    file->open( m_sndfilename.c_str(), start, len, 0 );
    
    if( fr != NULL )
    {
        fr->alloc_waveform( len % 2 ? len-1 : len );

        file->mtick( fr->waveform, fr->wlen );
        // KLUDGE-O-MATIC-O-RAMA
        fr->wsize = fr->wlen;
    }

	file->close();

    SAFE_DELETE( file ); 
}


//-----------------------------------------------------------------------------
// name: update_transients()
// desc: calls appropriate transient extraction, with slider parameters
//-----------------------------------------------------------------------------
bool UIAnalysis::update_transients()
{
    if( !m_fin )
        return false;
    
    // get correct time range
    get_rect( m_specgram, ui_elements );

    // update slider values
    if( ui_elements[BT_TRAN_METHOD].slide > 0.5 )
    {
        EngExtractor * my_ee;
        if( ee )
            my_ee = (EngExtractor *)ee;
        else
        {
            my_ee = new EngExtractor( m_sndfilename.c_str() );
            ee = my_ee;
        }
        my_ee->allowableGap = (int)ui_elements[SL_TRAN_GAP].fvalue();
        my_ee->maxTranLen = (int)ui_elements[SL_TRAN_MAX_LENGTH].fvalue();
        my_ee->longLen = (int)ui_elements[SL_TRAN_LONG].fvalue();
        my_ee->longHop = my_ee->longLen / 2;
        my_ee->shortLen = (int)ui_elements[SL_TRAN_SHORT].fvalue();
        my_ee->shortHop = my_ee->shortLen / 2;
        my_ee->thresh = ui_elements[SL_TRAN_THRESH].fvalue();
    }
    else
    {
        EnvExtractor * my_ee;
        if( ee )
            my_ee = (EnvExtractor *)ee;
        else
        {
            my_ee = new EnvExtractor( m_sndfilename.c_str() );
            ee = my_ee;
        }
        my_ee->allowableGap = (int)ui_elements[SL_TRAN_GAP].fvalue();
        my_ee->ageamt = ui_elements[SL_TRAN_AGEAMT].fvalue();
        my_ee->attack = ui_elements[SL_TRAN_ATTACK].fvalue();
        my_ee->decay = ui_elements[SL_TRAN_DECAY].fvalue();
        my_ee->thresh = ui_elements[SL_TRAN_THRESH].fvalue();
    }
    t_CKUINT start = (t_CKUINT)ui_elements[BT_SELECT_LEFT].fvalue();
    t_CKUINT end = (t_CKUINT)ui_elements[BT_SELECT_RIGHT].fvalue();

    // extract
    if( !ee->extract( start, end ) )
    {   
        msg_box( "bad instructions", "couldn't open file for transient extraction" );
    }

    // set current transient to the last one it found
    m_curr_tran = ee->transients.size() - 1;                            
    g_tran_suppressed = false;

    return true;
}


//-----------------------------------------------------------------------------
// name: play_segment()
// desc: ....
//-----------------------------------------------------------------------------
void UIAnalysis::play_segment()
{
    AudioSrcFile * file = new AudioSrcFile;
    if( rc && !rc_segments.empty() )
    {
		if( !m_fin )
			return;
		uint start = rc_segments[rc_curr_segment];
		uint len = (start + rc_regionsize < m_fin->info().frames) ? 
			rc_regionsize : m_fin->info().frames - start;
        file->open( m_sndfilename.c_str(), start, len, 0 );
        m_audio->bus(0)->play( file, TRUE, FALSE );
    }
    else
    {
        msg_box( "bad", "search first" );
    }
}


//-----------------------------------------------------------------------------
// name: read_transient()
// desc: ....
//-----------------------------------------------------------------------------
void UIAnalysis::read_segment( Frame * fr )
{
	if( !m_fin )
		return;

    AudioSrcFile * file = new AudioSrcFile;
    uint start = rc_segments[rc_curr_segment];
	uint len = (start + rc_regionsize < m_fin->info().frames ) ? 
		rc_regionsize : m_fin->info().frames - start;
    file->open( m_sndfilename.c_str(), start, len, 0 );
    
    if( fr != NULL )
    {
        fr->alloc_waveform( len % 2 ? len-1 : len );

        file->mtick( fr->waveform, fr->wlen );
        // KLUDGE-O-MATIC-O-RAMA
        fr->wsize = fr->wlen;
    }

	file->close();

    SAFE_DELETE( file ); 
}


//-----------------------------------------------------------------------------
// name: save_raw_spec()
// desc: save raw range from specgram selection
//-----------------------------------------------------------------------------
bool UIAnalysis::save_raw_spec( Frame * fr, int bounds[] )
{
	// read frame
	AudioSrcFile * file = new AudioSrcFile;
	get_rect( m_specgram, ui_elements ); 
	t_CKUINT start = (t_CKUINT)(ui_elements[BT_SELECT_LEFT].fvalue());
	t_CKUINT len = (t_CKUINT)(ui_elements[BT_SELECT_RIGHT].fvalue()) - start + 1;
	if( file->open( m_sndfilename.c_str(), start, len, 0 ) )
	{
		// actual # samples to read
		fr->wsize = (len % 2) ? len - 1 : len; 
		// zero padding to make it power of 2
		uint padded = fr->wsize * 8; 
		uint log2 = ::log((double)padded) / ::log(2.0); 
		if( padded - ::pow( 2, log2 ) <= ::pow( 2, log2 + 1) - padded )
			padded = ::pow( 2, log2 );
		else
			padded = ::pow( 2, log2 + 1 ); 
		// allocate enough memory
		if( fr->wlen != padded ) // if it's > padded, that's also a problem since it needs to be a power of 2
			fr->alloc_waveform( padded );
		else
			fr->zero();
		// determine where to read in from
		rf_offset = (padded - fr->wsize) / 2;
		// read in
		file->mtick( fr->waveform + rf_offset, fr->wsize );
		// close
		file->close();
	
		// filter according to selected ranges (low and high are fractions of Nyquist)
		double Nyquist = BirdBrain::srate() / 2;
		t_CKFLOAT low = ui_elements[BT_FREQ_LEFT].fvalue() / Nyquist;
		t_CKFLOAT high = ui_elements[BT_FREQ_RIGHT].fvalue() / Nyquist;
		t_CKFLOAT rolloff = ui_elements[SL_RAW_ROLLOFF].fvalue();
		fft_bandpass( fr, low, high, rolloff );
		
		// copy center part left instead of hacking AudioSrcFrame
		//memcpy( fr->waveform, fr->waveform + rf_offset, fr->wsize * sizeof(float) );
		fr->shift_waveform( rf_offset, 0 ); 

		// delete
		SAFE_DELETE( file );

		// remember bounds
		bounds[0] = start;
		bounds[1] = start + len - 1;
		bounds[2] = (int)(low * Nyquist);
		bounds[3] = (int)(high * Nyquist);

		// save display audio source
		SAFE_DELETE( rf_display );
		rf_display = new AudioSrcFrame( fr );

		// return
		return true;
	}
	// delete
	SAFE_DELETE( file );
	SAFE_DELETE( rf_display ); // luckily there is no destructor (otherwise the frame might get deleted)
	// mark bound as invalid
	bounds[0] = -1;
	// error message
	BB_log( BB_LOG_WARNING, "Couldn't open file for raw frame extraction" );
	// return
	return false;
}


//-----------------------------------------------------------------------------
// name: play_raw_spec()
// desc: play raw range from specgram selection
//-----------------------------------------------------------------------------
void UIAnalysis::play_raw_spec( Frame * rawframe )
{
	// check for validity
	if( rf_specbounds[0] < 0 )
		return;
	if( !rawframe )
		return;

	// load bounds
	ui_elements[BT_SELECT_LEFT].set_slide( rf_specbounds[0] );
	ui_elements[BT_SELECT_RIGHT].set_slide( rf_specbounds[1] );
	ui_elements[BT_FREQ_LEFT].set_slide( rf_specbounds[2] );
	ui_elements[BT_FREQ_RIGHT].set_slide( rf_specbounds[3] );
	set_rect( m_specgram, ui_elements );

	// create audio source frame
	AudioSrcFrame * f = new AudioSrcFrame( rawframe );
	f->slide = &ui_elements[BT_SELECT_NOW].slide_local;
	f->slide_locally = &ui_elements[BT_SELECT_NOW].slide_locally;
	f->on = &ui_elements[BT_SYN_PLAY].on; 

	// set position and play without rewinding
	m_audio->bus(0)->play( f, TRUE );
}



//-----------------------------------------------------------------------------
// name: load_file()
// desc: ...
//-----------------------------------------------------------------------------
bool UIAnalysis::load_file( AudioSrcFile * f )
{
    if( f == NULL )
        return false;
    // log
    BB_log( BB_LOG_INFO, "preprocessing waveform display..." );
    // push log
    BB_pushlog();
    // fill waveform
    m_fin = f;
    m_fin->rewind(); 
    GLdouble hop = (GLdouble)m_fin->info().frames / m_waveform_size;
    GLdouble pos = 0;
    SAMPLE sam;

    // log
    BB_log( BB_LOG_INFO, "frames in file: %d", m_fin->info().frames );
    BB_log( BB_LOG_INFO, "waveform size: %d", m_waveform_size );
    BB_log( BB_LOG_INFO, "analysis hop size: %.1f samples", hop );

    for( int j = 0; j < m_waveform_size; j++ )
    {
        m_fin->mtick( &sam, 1 );
        m_waveform[j] = sam;
        pos += hop;
        if( pos < m_fin->info().frames )
            m_fin->seek( (uint)pos, SEEK_SET );
    }

    // pop log
    BB_poplog();

	// reset ranges
    m_fin->seek( 0, SEEK_SET );
    ui_elements[BT_SELECT_LEFT].slide_1 = m_fin->info().frames - 1; // -1s changed 
    ui_elements[BT_SELECT_NOW].slide_1 = m_fin->info().frames - 1;
    ui_elements[BT_SELECT_RIGHT].slide_1 = m_fin->info().frames - 1;
    ui_elements[BT_FREQ_LEFT].slide_1 = m_fin->info().samplerate / 2.0;
    ui_elements[BT_FREQ_RIGHT].slide_1 = m_fin->info().samplerate / 2.0;
    ui_elements[BT_SELECT_LEFT].set_slide( 0 );
    ui_elements[BT_SELECT_RIGHT].set_slide( m_fin->info().frames - 1 );
    ui_elements[BT_FREQ_LEFT].set_slide( 0 );
    ui_elements[BT_FREQ_RIGHT].set_slide( m_fin->info().samplerate / 2.0 ); 
    set_rect( m_specgram, ui_elements );

    // spectro
    spectroinit();

    // no transients found yet
    m_curr_tran = -1;

	// no raw spec sound extracted yet
	rf_specbounds[0] = -1;
	SAFE_DELETE( rf_display );

	// no regions/segments found yet
	rc_curr_segment = -1;

    // keep file name up to date
    m_sndfilename = m_fin->filename(); 

    return true;
}


//-----------------------------------------------------------------------------
// name: create_regioncomparer()
// desc: creates rc
//-----------------------------------------------------------------------------
bool UIAnalysis::create_regioncomparer( int regionsize )
{
	// make sure something is loaded
	if( m_fin == NULL )
		return false;
	// delete rc if it exists
	SAFE_DELETE( rc );
	// allocate space for new frame
	int nsamples = m_fin->info().frames;
	if( !rc_wholeframe )
	{
		rc_wholeframe = new Frame;
		BB_log( BB_LOG_WARNING, "ui_analysis: rc_wholeframe did not exist" );
	}
	if( nsamples > rc_wholeframe->wlen )
	{
		rc_wholeframe->alloc_waveform( nsamples + (nsamples % 2) ); // even number of samples...
	}
	// tick
	m_fin->rewind();
	if( m_fin->mtick( rc_wholeframe->waveform, nsamples ) )
		rc_wholeframe->wsize = nsamples;	
	else
		rc_wholeframe->wsize = 0;
	// create rc
	if( rc_wholeframe->wsize > 0 )
		rc = new RegionComparer( 2 * regionsize / 512, 512, *rc_wholeframe );
	// return
	return (rc != NULL );
}	


//-----------------------------------------------------------------------------
// name: on_event()
// desc: ...
//-----------------------------------------------------------------------------
t_CKUINT UIAnalysis::on_event( const AudicleEvent & event )
{
    static t_CKUINT m_mouse_down = FALSE;
    static t_CKUINT which = 0;
    static Point2D last;
    t_CKBOOL hit = FALSE, change_trans = FALSE;
    Point2D diff;
    int i;

    if( event.type == ae_event_INPUT )
    {
        InputEvent * ie = (InputEvent *)event.data;
        if( ie->type == ae_input_MOUSE )
        {
            ie->popStack();

            for( i = 0; i < NUM_UI_ELEMENTS; i++ )
            {
                if( ie->checkID( ui_elements[i].id ) )
                {
                    if( ie->state == ae_input_DOWN )
                    {
                        hit = TRUE;
                        ui_elements[i].down = TRUE;
                        // slider
                        if( i >= SL_DET_TRACKS && i <= SL_TRAN_AGEAMT )
                            ui_elements[i].slide_last = vr_to_world_x(m_vr,ie->pos[0]) / g_slider_height;
                        if( i >= SL_DET_THRESH && i <= SL_TRAN_GAP )
                            ui_elements[i].slide_last = (vr_to_world_x(m_vr,ie->pos[0]) - .6) / g_slider_height;
                        if( i >= SL_BRIGHTNESS && i <= SL_BRIGHTNESS )
                            ui_elements[i].slide_last = (vr_to_world_x(m_vr,ie->pos[0]) - .1 ) / g_slider_height_mini;
                        if( i >= SL_CONTRAST && i <= SL_CONTRAST )
                            ui_elements[i].slide_last = (vr_to_world_x(m_vr,ie->pos[0]) - .7 ) / g_slider_height_mini;
                        if( i >= BT_SELECT_LEFT && i <= BT_SELECT_RIGHT )
                        {
                            ui_elements[i].slide_last = (vr_to_world_x(m_vr,ie->pos[0]) + 1.1) / g_butter_width;
//                          fprintf( stderr, "------------------------------------------\n" );
//                          fprintf( stderr, "slide: %f last: %f\n", ui_elements[i].slide, ui_elements[i].slide_last );
                        }
                        if( i >= BT_FREQ_LEFT && i <= BT_FREQ_RIGHT )
                            ui_elements[i].slide_last = (vr_to_world_x(m_vr,ie->pos[0]) - .1) / g_butter_width;
                        if( i >= KN_THRESH_TILT && i <= KN_THRESH_TILT )
                            ui_elements[i].slide_last = (vr_to_world_y(m_vr,ie->pos[1]) - .3 ) / g_slider_height_mini;
                        if( i == UI_SPECTROGRAM )
                        {
                            m_specgram.start = vr_to_world(m_vr,ie->pos);
                            m_specgram.start -= m_specgram.loc;
                            m_specgram.curr = m_specgram.start;
                        }
                    }

                    if( ie->state == ae_input_UP && ui_elements[i].down == TRUE )
                    {
                      
                        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ PLAY ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                        if( i == BT_PLAY )
                        {
                            AudioSrcFile * f = new AudioSrcFile;
                            if( m_fin )
                            {
                                // check bounds                                
                                if( (t_CKUINT)(ui_elements[BT_SELECT_LEFT].fvalue()) < 0 )
                                    ui_elements[BT_SELECT_LEFT].set_slide( 0 ); 
                                if( (t_CKUINT)(ui_elements[BT_SELECT_RIGHT].fvalue()) >= m_fin->info().frames )
                                    ui_elements[BT_SELECT_RIGHT].set_slide( m_fin->info().frames - 1 );
                                // play
                                if( f->open( m_sndfilename.c_str(), 
                                    (t_CKUINT)(ui_elements[BT_SELECT_LEFT].fvalue()),
                                    (t_CKUINT)(ui_elements[BT_SELECT_RIGHT].fvalue() - ui_elements[BT_SELECT_LEFT].fvalue() + 1)))
                                {
                                    f->slide = &ui_elements[BT_SELECT_NOW].slide_local;
                                    f->slide_locally = &ui_elements[BT_SELECT_NOW].slide_locally;
                                    f->on = &ui_elements[BT_PLAY].on;
                                    m_audio->bus(0)->play( f, TRUE, FALSE );
                                    ui_elements[i].on = TRUE;
                                    ui_elements[i].on_where = 0.0;
                                }
                                // log
                                BB_log( BB_LOG_FINE, "audio bus 0 num src: %i", m_audio->bus(0)->num_src() );
                            }
                            else
                            {
                                msg_box( "[play button]", "load first!" );
                            }
                        }

                        
                        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ STOP ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                        else if( i == BT_STOP )
                        {
                            m_audio->bus(0)->stop();
                            // set off
                        }
                        
                        
                        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ZOOM IN ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                        else if( i == BT_ZOOM_IN )
                        {
                            //m_vis->zoom_in( ui_elements[BT_SELECT_LEFT].slide, ui_elements[BT_SELECT_RIGHT].slide );
                        }
                        
                        
                        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ZOOM OUT ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                        else if( i == BT_ZOOM_OUT )
                        {
                            //m_vis->zoom_out();
                        }
                        
                        
                        //~~~~~~~~~~~~~~~~~~~~~~~ TOGGLE SPECGRAM AND SPECTRUM ~~~~~~~~~~~~~~~~~~~~~~
                        else if( i == BT_TOGGLE )
                        {
                            m_spec = !m_spec;
                        }

                        
                        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ FAKE RESIDUE ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                        else if( i == BT_CLASSIFIED )
                        {
                            ui_elements[BT_CLASSIFIED].slide = 1.0f - ui_elements[BT_CLASSIFIED].slide;
                            // put in library
                        }

                        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ LOAD ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                        else if( i == BT_LOAD )
                        {
                            DirScanner dScan;
                            dScan.setFileTypes( "Wave Files (*.wav)\0*.wav\0Preprocessed Files (*.pp)\0*.pp\0All Files (*.*)\0*.*\0" );
                            fileData * fD = dScan.openFileDialog();
                            if ( fD )
                            {
                                if( m_orig && m_orig != m_fin )
                                    delete m_orig;
                                m_orig = m_fin;
                                m_fin = NULL;
                                SAFE_DELETE( m_res );
                                SAFE_DELETE( driver );
                                SAFE_DELETE( ee );
								SAFE_DELETE( rc );
                                m_loadfileprev = m_loadfilename; 

                                // log
                                // fprintf(stderr, "opening %s\n", fD->fileName.c_str() );
                                BB_log( BB_LOG_INFO, "loading: %s", fD->fileName.c_str() );
                                // push log
                                BB_pushlog();
                                // check if it's .pp
                                if( fD->fileName.find( ".pp", fD->fileName.length() - 3 ) != string::npos )
                                {
                                    m_loadfilename = fD->fileName;
                                    Driver cabbie;  
                                    m_sndfilename = (BirdBrain::getpath( fD->fileName.c_str() ) + 
                                        BirdBrain::getbase(cabbie.get_filename(fD->fileName).c_str())).c_str();
                                }
                                else
                                    m_loadfilename = m_sndfilename = fD->fileName.c_str();
                                BB_log( BB_LOG_INFO, "(ui_analysis): opening file %s", m_sndfilename.c_str() );

                                if ( fD->next ) 
                                    // fprintf( stderr, "opening first of multiple files...\n");
                                    BB_log( BB_LOG_INFO, "...opening first of multiple files..." );
                                
                                AudioSrcFile * f = new AudioSrcFile;
                                if( !f->open( m_sndfilename.c_str(), 0, 0 ) )
                                {
                                    msg_box( m_sndfilename.c_str(), "cannot open file!" );
                                    delete f;
                                }
                                else
                                {
                                    load_file( f );
                                    g_out_count = 0;
                                    g_res_count = 0;
                                    if( !m_orig ) m_orig = m_fin; 
                                }

                                // pop log
                                BB_poplog();
                            }
                        }
                        
                        
                        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ SEPARATE ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                        else if( i == BT_SEPARATE )
                        {
                            // nothing loaded
                            if( !m_fin )
                            {
                                msg_box( "bad!", "load first!" );
                            }
                            // separate transient
                            else if( m_analysis_mode == ANA_TRAN )
                            {
                                update_transients();
                                
                                // name of output file (figure out in convoluted way)
                                char path[1024];
                                const char * a = m_sndfilename.c_str();
                                const char * b = BirdBrain::getbase( m_sndfilename.c_str() );
                                strcpy( path, a );
                                g_tran_outfile = "res" + BirdBrain::toString( ++g_res_count ) + "_" 
                                                + BirdBrain::getbase( m_sndfilename.c_str() );
                                strcpy( path + (b-a), g_tran_outfile.c_str() );
                                g_tran_outfile = path;

                                // remove transients
                                std::vector<uint> to_suppress; 
                                for( uint i = 0; i < ee->transients.size(); i++ )
                                    to_suppress.push_back( i );
                                
                                g_tran_suppressed = ee->remove_transients( g_tran_outfile.c_str(), to_suppress );

                                // open residue
                                if( m_res )
                                    delete m_res;
                                m_res = new AudioSrcFile; 
                                if( !m_res->open( path, 0, 0, FALSE ) )
                                {
                                    delete m_res;
                                    m_res = NULL;
                                }
                            }
                            // separate sinusoids
                            else if( m_analysis_mode == ANA_DET )
                            {
                                // reload time and frequency boundaries for the analysis
                                get_rect( m_specgram, ui_elements );

                                //t_CKUINT start = (t_CKUINT)(ui_elements[BT_SELECT_LEFT].slide * m_fin->info().frames);
                                //t_CKUINT end = (t_CKUINT)(ui_elements[BT_SELECT_RIGHT].slide * m_fin->info().frames);
                                //t_CKFLOAT low = ui_elements[BT_FREQ_LEFT].slide * m_fin->info().samplerate / 2.0;
                                //t_CKFLOAT high = ui_elements[BT_FREQ_RIGHT].slide * m_fin->info().samplerate / 2.0;
                                t_CKUINT start = (t_CKUINT)(ui_elements[BT_SELECT_LEFT].fvalue());
                                t_CKUINT end = (t_CKUINT)(ui_elements[BT_SELECT_RIGHT].fvalue());
                                t_CKFLOAT low = ui_elements[BT_FREQ_LEFT].fvalue();
                                t_CKFLOAT high = ui_elements[BT_FREQ_RIGHT].fvalue();
                                t_CKUINT num_tracks= atoi(ui_elements[SL_DET_TRACKS].value().c_str());
                                t_CKUINT minpoints = atoi(ui_elements[SL_DET_MINPOINTS].value().c_str());
                                t_CKUINT maxgap = atoi(ui_elements[SL_DET_MAXGAP].value().c_str());
                                t_CKFLOAT error_f = ui_elements[SL_DET_ERROR_FREQ].fvalue();
                                t_CKFLOAT noise_r = ui_elements[SL_DET_NOISE_RATIO].fvalue();
                                // grouping parameters (groan)
                                t_CKFLOAT gr_harm = ui_elements[SL_GROUP_HARM].fvalue();
                                t_CKFLOAT gr_freq = ui_elements[SL_GROUP_MOD].fvalue();
                                t_CKFLOAT gr_amp = ui_elements[SL_GROUP_MOD].fvalue(); 
                                t_CKFLOAT gr_overlap = ui_elements[SL_GROUP_OVERLAP].fvalue();
                                t_CKFLOAT gr_on = ui_elements[SL_GROUP_ONSET].fvalue();
                                t_CKFLOAT gr_off = ui_elements[SL_GROUP_OFFSET].fvalue();
                                t_CKFLOAT gr_minlen = ui_elements[SL_GROUP_MINLEN].fvalue();
                                bool gr = ui_elements[BT_GROUP].slide > 0.5; 

                                // if( driver ) delete driver;
                                if( driver && m_sndfilename != m_loadfilename )
                                {
                                    driver->cruise( start, end, low, high, m_threshold, minpoints, maxgap, 
                                                error_f, noise_r, num_tracks, gr_harm, gr_freq, gr_amp, gr_overlap,
                                                gr_on, gr_off, gr_minlen, gr ); 
                                }
                                else
                                {
                                    SAFE_DELETE( driver ); 
                                    driver = run( (char *)m_loadfilename.c_str(), start, end, low, high, 
                                                m_threshold, minpoints, maxgap, error_f, noise_r, num_tracks, 
                                                m_fft_size, m_wnd_size, gr_harm, gr_freq, gr_amp, gr_overlap,
                                                gr_on, gr_off, gr_minlen, gr );
                                }
                                // test run
                                //driver = run( (char *)m_filename.c_str(), 38397, 233341, 1701, 9612, m_threshold, minpoints, maxgap, error_f, noise_r, num_tracks, m_fft_size, m_wnd_size );

                                AudioSrcFrame * f = NULL;
                                if( !driver->the_event().empty() )
                                {
                                    // honk
                                    driver->honk( driver->the_event(), *frame );
                                    g_all_tracks = true; 
                                    g_show_tracks = true;

                                    int size = driver->m_stop - driver->m_start + 
                                            (driver->m_stop - driver->m_start) % 2 + BirdBrain::wnd_size();
                                    if( biggerframe->wlen < size )
                                        biggerframe->alloc_waveform( size );
                                    else
                                        biggerframe->zero(); 
                                    biggerframe->wsize = size;
                                    int tocopy = (int)(frame->wsize < biggerframe->wsize - (frame->time - driver->m_start) ? 
                                        frame->wsize : biggerframe->wsize - (frame->time - driver->m_start)); 
                                    memcpy( biggerframe->waveform + (int)frame->time - driver->m_start, // danger comment: timeout
                                            frame->waveform, 
                                            tocopy * sizeof(float) );

                                    // minor (ideally) adjustment to start and stop points
                                    ui_elements[BT_SELECT_LEFT].set_slide( driver->m_start );
                                    ui_elements[BT_SELECT_RIGHT].set_slide( driver->m_stop + BirdBrain::wnd_size() );

                                    f = new AudioSrcFrame( biggerframe );
                                    
                                    //f = new AudioSrcFrame( frame );
                                    f->slide = &ui_elements[BT_SELECT_NOW].slide_local;
                                    f->slide_locally = &ui_elements[BT_SELECT_NOW].slide_locally;
                                    f->on = &ui_elements[BT_SYN_PLAY].on;
                                }
                                else
                                {
                                    msg_box( "my precious!", "no tracksssss!" );
                                }

                                // play
                                if( f )
                                {
                                    m_audio->bus(0)->play( f, TRUE );
                                    ui_elements[BT_SYN_PLAY].on = TRUE;
                                    ui_elements[BT_SYN_PLAY].on_where = 0.0;
                                }
                            }
							else if( m_analysis_mode == ANA_RAW )
							{
								if( rf_rawframe )
								{
									if( save_raw_spec( rf_rawframe, rf_specbounds ) ) 
									{
										play_raw_spec( rf_rawframe );
									}
									else
										msg_box( "sorry", "couldn't get raw frame" );
								}
								else 
								{
									BB_log( BB_LOG_WARNING, "ui_analysis: rf_rawframe does not exist" );
								}
							}
							else if( m_analysis_mode == ANA_FEAT )
							{
								bool okay = false;
								// first extract raw frame
								if( !rc_extractframe ) {
									rc_extractframe = new Frame;
									BB_log( BB_LOG_WARNING, "ui_analysis: rc_extractframe did not exist" );
								}
							/*	if( save_raw_spec( rc_extractframe, rf_specbounds ) ) 
								{
									play_raw_spec( rc_extractframe );
									okay = true;
								}
								else
									msg_box( "sorry", "couldn't get raw frame" );
							*/	if( m_fin )
								{
									int start = (int)ui_elements[BT_SELECT_LEFT].fvalue();
									int end = (int)ui_elements[BT_SELECT_RIGHT].fvalue();
									rc_regionsize = end - start;
									m_fin->seek( start, SEEK_SET );
									if( rc_extractframe->wlen < rc_regionsize )
										rc_extractframe->alloc_waveform( rc_regionsize + (rc_regionsize % 2) );
									rc_extractframe->wsize = rc_regionsize;
									okay = (m_fin->mtick( rc_extractframe->waveform, rc_regionsize ) != 0);
									//m_fin->rewind();
								}
								// make regioncomparer
								if( okay )
								{
									// need new region comparer?
									//BB_log( BB_LOG_INFO, "frame start: %i", rf_specbounds[0] );
									if( !rc )
									{
										if( !(okay = create_regioncomparer( rc_regionsize )) )
										{
											BB_log( BB_LOG_WARNING, "ui_analysis: could not create regioncomparer" );
											SAFE_DELETE( rc );
										}
									}
									else
									{
										rc->setRegionSize( rc_regionsize / (512/2) );
									}

								}
								// extract
								if( okay )
								{
									rc_segments.clear();
									// set threshold 
									double threshold = 1 - ui_elements[SL_FEAT_THRESH].fvalue();
									// set weights
									double weights[] = { ui_elements[SL_FEAT_A].fvalue(),
														 ui_elements[SL_FEAT_B].fvalue(), 
														 ui_elements[SL_FEAT_C].fvalue(), 
														 ui_elements[SL_FEAT_D].fvalue(),
														 ui_elements[SL_FEAT_E].fvalue() };
									rc->setWeights( weights );
									rc->findMatchingRegions( *rc_extractframe, rc_segments, threshold, true );
									rc_curr_segment = rc_segments.size() - 1;
									BB_log( BB_LOG_FINE, "threshold: %f", threshold );
									BB_log( BB_LOG_FINE, "found %i matching regions", rc_segments.size() );
									for(int l = 0; l < rc_segments.size(); l++)
										BB_log( BB_LOG_FINE, "segment %d", rc_segments[l] );
								}
							}
                        }
                        

                        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ FORWARD ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                        else if( i == BT_FF )
                        {
                            // move forward by one hop-size (one frame)
                            if( m_fin ) {
                                ui_elements[BT_SELECT_NOW].slide += BirdBrain::hop_size() / (float)m_fin->info().frames;
                                ui_elements[BT_SELECT_NOW].slide_local += (BirdBrain::hop_size() / (float)m_fin->info().frames) /
                                     (ui_elements[BT_SELECT_RIGHT].slide - ui_elements[BT_SELECT_LEFT].slide );
                            }
                        }


                        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ REWIND ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                        else if( i == BT_REWIND )
                        {
                            // back one frame
                            if( m_fin ) {
                                ui_elements[BT_SELECT_NOW].slide -= BirdBrain::hop_size() / (float)m_fin->info().frames;
                                ui_elements[BT_SELECT_NOW].slide_local -= (BirdBrain::hop_size() / (float)m_fin->info().frames) /
                                     (ui_elements[BT_SELECT_RIGHT].slide - ui_elements[BT_SELECT_LEFT].slide );
                            }
                        }


                        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ NEXT EVENT ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                        else if( i == BT_NEXT_EVENT )
                        {
                            if( m_analysis_mode == ANA_TRAN )
                            {
                                if( ee != NULL )
                                {
                                    if( ee->transients.empty() )
                                        m_curr_tran = -1;
                                    else 
                                    {
                                        m_curr_tran++;
                                        if( m_curr_tran >= ee->transients.size() )
                                            m_curr_tran = 0;
                                        play_transient();
                                    }
                                }
                            }
                            else if( m_analysis_mode == ANA_DET )
                            {
                                if( driver != NULL )
                                {
                                    driver->honk( driver->next_event(), *frame );
                                    ui_elements[BT_SELECT_LEFT].set_slide( frame->time );
                                    ui_elements[BT_SELECT_RIGHT].set_slide( frame->time + frame->wsize);
                                    g_all_tracks = false; 
                                    g_show_tracks = true;
                                    
                                    AudioSrcFrame *f = new AudioSrcFrame( frame );
                                    f->slide = &ui_elements[BT_SELECT_NOW].slide_local;
                                    f->slide_locally = &ui_elements[BT_SELECT_NOW].slide_locally;
                                    f->on = &ui_elements[i].on;
                                    m_audio->bus(0)->play( f, TRUE );
                                    ui_elements[i].on = true;
                                    ui_elements[i].on_where = 0.0;
                                }
                            }
							else if( m_analysis_mode == ANA_FEAT )
							{
								if( rc != NULL )
								{
									if( rc_segments.empty() )
										rc_curr_segment = -1;
									else
									{
										rc_curr_segment++;
										if( rc_curr_segment >= rc_segments.size() )
											rc_curr_segment = 0;
										play_segment();
									}
								}
							}
                        }


                        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ PREV EVENT ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                        else if( i == BT_PREV_EVENT )
                        {
                            if( m_analysis_mode == ANA_TRAN )
                            {
                                if( ee != NULL )
                                {
                                    if( ee->transients.empty() )
                                        m_curr_tran = -1;
                                    else 
                                    {
                                        m_curr_tran--;
                                        if( m_curr_tran < 0 )
                                            m_curr_tran = ee->transients.size() - 1;
                                        play_transient();
                                    }
                                }
                            }
                            else if( m_analysis_mode == ANA_DET )
                            {
                                if( driver != NULL )
                                {
                                    driver->honk( driver->prev_event(), *frame );
                                    ui_elements[BT_SELECT_LEFT].set_slide( frame->time );
                                    ui_elements[BT_SELECT_RIGHT].set_slide( frame->time + frame->wsize);
                                    g_all_tracks = false;   
                                    g_show_tracks = true;

                                    AudioSrcFrame *f = new AudioSrcFrame( frame );
                                    f->slide = &ui_elements[BT_SELECT_NOW].slide_local;
                                    f->slide_locally = &ui_elements[BT_SELECT_NOW].slide_locally;
                                    f->on = &ui_elements[i].on;
                                    m_audio->bus(0)->play( f, TRUE );
                                    ui_elements[i].on = true;
                                    ui_elements[i].on_where = 0.0;
                                }
                            }
							else if( m_analysis_mode == ANA_FEAT )
							{
								if( rc != NULL )
								{
									if( rc_segments.empty() )
										rc_curr_segment = -1;
									else
									{
										rc_curr_segment--;
										if( rc_curr_segment < 0 )
											rc_curr_segment = rc_segments.size() - 1;
										play_segment();
									}
								}
							}
                        }

                        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ALL EVENTS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                        else if( i == BT_ALL )
                        {
                            if( m_analysis_mode == ANA_TRAN || m_analysis_mode == ANA_FEAT)
                            {
                                // do nothing, for now (the button shouldn't even be drawn in this case)
                            }
                            else if( m_analysis_mode == ANA_DET )
                            {
                                if( driver != NULL )
                                {
                                    driver->honk( driver->the_event(), *frame );
                                    ui_elements[BT_SELECT_LEFT].set_slide( frame->time );
                                    ui_elements[BT_SELECT_RIGHT].set_slide( frame->time + frame->wsize);
                                    g_all_tracks = true; 
                                    g_show_tracks = true;

                                    AudioSrcFrame *f = new AudioSrcFrame( frame );
                                    f->slide = &ui_elements[BT_SELECT_NOW].slide_local;
                                    f->slide_locally = &ui_elements[BT_SELECT_NOW].slide_locally;
                                    f->on = &ui_elements[i].on;
                                    m_audio->bus(0)->play( f, TRUE );
                                    ui_elements[i].on = true;
                                    ui_elements[i].on_where = 0.0;
                                }
                            }
                        }

                        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ SYN_PLAY ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                        else if( i == BT_SYN_PLAY )
                        {
                            if( m_analysis_mode == ANA_TRAN )
                            {
                                play_transient();
                            }
                            else if( m_analysis_mode == ANA_DET )
                            {
                                AudioSrcFrame * f = NULL;
                                if( driver )
                                {
                                    if( driver && driver->cur_event().size() )
                                    {
                                        driver->honk( driver->cur_event(), *frame ); // this was added post-grouping
                                        ui_elements[BT_SELECT_LEFT].set_slide( frame->time );
                                        ui_elements[BT_SELECT_RIGHT].set_slide( frame->time + frame->wsize);
                                        g_all_tracks = false; 
                                        g_show_tracks = true;

                                        f = new AudioSrcFrame( frame );
                                        f->slide = &ui_elements[BT_SELECT_NOW].slide_local;
                                        f->slide_locally = &ui_elements[BT_SELECT_NOW].slide_locally;
                                        f->on = &ui_elements[BT_SYN_PLAY].on;
                                        m_audio->bus(0)->play( f, TRUE );
                                        ui_elements[i].on = true;
                                        ui_elements[i].on_where = 0.0;
                                    }
                                    else
                                    {
                                        msg_box( "my precious!", "no tracksssss!" );
                                    }
                                }
                                else
                                {
                                    msg_box( "bad!", "separate first!" );
                                }
                            }
							else if( m_analysis_mode == ANA_RAW )
							{
								// check bounds and play if valid
								if( rf_specbounds[0] >= 0 )
								{
									play_raw_spec( rf_rawframe );
								}
								else
								{
									msg_box( "bad!", "separate first!" );
								}
							}
							else if( m_analysis_mode == ANA_FEAT )
							{
								// play found region
								play_segment();
							}
                        }


                        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ SYN_SAVE ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                        else if( i == BT_SYN_SAVE )
                        {
                            if( m_analysis_mode == ANA_TRAN )
                            {
                                if( ee != NULL )
                                {
                                    if( m_curr_tran >= 0 ) // we think we don't need to check  m_curr_tran < transients.size()
                                    {
                                        Frame fr;
                                        read_transient( &fr );
                                        std::string fname; // filename

                                        Transient * temp = new Transient( fr );
                                        if( !save_as_name( fname ) )
                                            temp->name = BirdBrain::getname( m_sndfilename.c_str() ) + '(' 
                                                + BirdBrain::toString(m_curr_tran) + ')';
                                        else
                                            temp->name = BirdBrain::getname( fname.c_str() );
                                        Library::instance()->add(temp);

                                        // write
                                        save_to_file( temp, fname );
                                    }
                                    else
                                    {
                                        msg_box( "not so bad", "no transients found" );
                                    }
                                }
                                else
                                {
                                    msg_box( "bad", "separate first" );
                                }
                            }
                            else if( m_analysis_mode == ANA_DET )
                            {
                                if( driver != NULL )
                                {
                                    std::string fname; 

                                    Deterministic * temp = new Deterministic( g_all_tracks ? driver->the_event() : driver->cur_event() ); 
                                    if( !save_as_name( fname ) )
                                        temp->name = BirdBrain::get_part_of_name( driver->out_file_name.c_str() )
                                                    + '(' + BirdBrain::toString( ++g_out_count ) + ')';
                                    else
                                        temp->name = BirdBrain::getname( fname.c_str() );
                                    Library::instance()->add( temp );

                                    // write
                                    save_to_file( temp, fname );
                                }
                                else
                                {
                                    msg_box( "bad!", "separate first!" );
                                }
                            }
							else if( m_analysis_mode == ANA_RAW )
							{
								// if valid
								if( rf_specbounds[0] >= 0 )
								{
									std::string fname;
									// save as template
									//		get name
									Raw * temp = new Raw( *rf_rawframe );
									if( !save_as_name( fname ) )
										temp->name = BirdBrain::getname( m_sndfilename.c_str() ) + ".raw";
									else
										temp->name = BirdBrain::getname( fname.c_str() );
									Library::instance()->add(temp);
									//		write
									save_to_file( temp, fname );
								}
								else
								{
									msg_box( "bad!", "separate first!" );
								}
							}
							else if( m_analysis_mode == ANA_FEAT )
							{
								if( rc )
								{
									if( !rc_segments.empty() )
									{
										// get selected region
										int bounds[4] = {rc_segments[rc_curr_segment], 
														 rc_segments[rc_curr_segment] + rc_regionsize, 
														 0, BirdBrain::srate() / 2};
										if( save_raw_spec( rc_extractframe, bounds ) )
										{
											std::string fname;
											Raw * temp = new Raw( *rc_extractframe );
											// get name
											if( !save_as_name( fname ) )
												temp->name = BirdBrain::getname( m_sndfilename.c_str() ) + ".reg";
											else
												temp->name = BirdBrain::getname( fname.c_str() );
											Library::instance()->add( temp );
											// write
											save_to_file( temp, fname );
										}
									}
								}
								else
								{
									msg_box( "bad!", "search first!" );
								}
							}
                        }

						//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ SAVE A RAW RANGE ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                        else if( i == BT_SPEC_SAVE )
                        {
							std::string fname; 
							
							if( save_raw_spec( rf_rawframe, rf_specbounds ) )
							{
								// save as template
								//		get name
								Raw * temp = new Raw( *rf_rawframe );
								if( !save_as_name( fname ) )
									temp->name = BirdBrain::getname( m_sndfilename.c_str() ) + ".raw"; 
								else
									temp->name = BirdBrain::getname( fname.c_str() );
								Library::instance()->add(temp);
								//		write
								save_to_file( temp, fname );
							}
                        }
                        
                        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ RES_PLAY ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                        else if( i == BT_RES_PLAY )
                        {
                            AudioSrcFile * f = NULL;
                            char path[1024] = "";
                            const char * a = NULL;
                            const char * b = NULL;

                            if( ui_elements[BT_CLIP_ONLY].slide <= 0.5 || m_analysis_mode == ANA_TRAN )
                            {
                                // if whole file minus selected events is being played (instead of just processed clip)
                                ui_elements[BT_SELECT_LEFT].set_slide( ui_elements[BT_SELECT_LEFT].slide_0 );
                                ui_elements[BT_SELECT_RIGHT].set_slide( ui_elements[BT_SELECT_RIGHT].slide_1 );
                            }
                            else if( m_analysis_mode == ANA_DET ) // used to be just else
                            {
                                // if processed clip is being played for sinusoidal, not transient analysis
                                // because clip_only is not implemented for transients
                                if( driver )
                                {
                                    ui_elements[BT_SELECT_LEFT].set_slide( driver->m_start );
                                    ui_elements[BT_SELECT_RIGHT].set_slide( driver->m_stop );
                                    ui_elements[BT_FREQ_LEFT].set_slide( driver->m_freq_min );
                                    ui_elements[BT_FREQ_RIGHT].set_slide( driver->m_freq_max );
                                }
                            }

                            if( m_analysis_mode == ANA_TRAN )
                            {
                                //if( ee )
                                if( m_res )
                                {
                                    //strcpy( path, g_tran_outfile.c_str() );
                                    f = new AudioSrcFile;
                                    //if( f->open( path, 0, 0, FALSE ) )
                                    if( f->open( m_res->filename().c_str(), 0, 0, FALSE ) )
                                    {
                                        f->slide = &ui_elements[BT_SELECT_NOW].slide_local;
                                        f->slide_locally = &ui_elements[BT_SELECT_NOW].slide_locally;
                                        f->on = &ui_elements[BT_RES_PLAY].on;
                                        m_audio->bus(0)->play( f, TRUE );
                                        ui_elements[i].on = true;
                                        ui_elements[i].on_where = 0.0;
                                    }
                                    else
                                    {
                                        msg_box( path, "separate first!" );
                                        delete f;
                                    }
                                }
                                else
                                {
                                    msg_box( "bad!", "separate first!" );
                                }
                            }
                            else if( m_analysis_mode == ANA_DET )
                            {
                                //if( driver )
                                if( m_res )
                                {
                                    //a = m_fin->filename().c_str();
                                    //b = BirdBrain::getbase( m_fin->filename().c_str() );
                                    //strcpy( path, a );
                                    //strcpy( path + (b-a), driver->res_file_name.c_str() );
                                    f = new AudioSrcFile;
                                    //if( f->open( path, 0, 0, FALSE ) )
                                    if( f->open( m_res->filename().c_str(), 0, 0, FALSE ) )
                                    {
                                        f->slide = &ui_elements[BT_SELECT_NOW].slide_local;
                                        f->slide_locally = &ui_elements[BT_SELECT_NOW].slide_locally;
                                        f->on = &ui_elements[BT_RES_PLAY].on;
                                        m_audio->bus(0)->play( f, TRUE );
                                        ui_elements[i].on = true;
                                        ui_elements[i].on_where = 0.0;
                                    }
                                    else
                                    {
                                        msg_box( path, "save first!" );
                                        delete f;
                                    }
                                }
                                else
                                {
                                    msg_box( "bad!", "separate first!" );
                                }
                            }
                        }

                        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ RES_SAVE ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                        else if( i == BT_RES_SAVE )
                        {
                            char path[1024] = "";
                            const char * a = NULL;
                            const char * b = NULL;
                            bool bad = false;

                            if( m_analysis_mode == ANA_TRAN )
                            {
                                if( ee && strcmp( g_tran_outfile.c_str(), "" ) )
                                {
                                    fprintf( stderr, "saving transient residue...\n" );

                                    // make new Treesynth and TreesynthIO objects for template
                                    Treesynth *ts = new Treesynth;
                                    TreesynthIO *tsio = new TreesynthIO;
                                    strcpy( tsio->ifilename, g_tran_outfile.c_str() );
                                    strcpy( path, g_tran_outfile.c_str() );
                                    // tsio is all set; to use ts, the appropriate face should
                                    //  - make and initialize a new tree of specified total levels
                                    //  - set ts->tree to the new tree
                                    //  - ts->initialize()
                                    //  - change ay ts parameters if needed
                                
                                    ts->tree = new Tree;
                                    ts->tree->initialize( lg(CUTOFF) );
                                    ts->initialize();
                                    ts->resetTreeLevels( 13 );
                                    ts->stoplevel = 9;

                                    // Now, make a new template for the residue
                                    std::string fname;
                                    Residue * temp = new Residue( ts, tsio );
                                    if( !save_as_name( fname ) )
                                        temp->name = "not_tran"; 
                                    else
                                        temp->name = BirdBrain::getname( fname.c_str() );

                                    Library::instance()->add( temp );

                                    // write
                                    save_to_file( temp, fname );
                                }
                                else
                                {
                                    msg_box( "bad!", "separate first!" );
                                    bad = true;
                                }
                            }
                            else if( m_analysis_mode == ANA_DET )
                            {
                                if( driver != NULL )
                                {
                                    fprintf( stderr, "writing residue file...\n" );
                                    // write out residue
                                    driver->write_res( ++g_res_count, ui_elements[BT_CLIP_ONLY].slide > 0.5 );

                                    a = m_sndfilename.c_str(); 
                                    b = BirdBrain::getbase( m_sndfilename.c_str() );
                                    strcpy( path, a );
                                    strcpy( path + (b-a), driver->res_file_name.c_str() );

                                    // make new Treesynth and TreesynthIO objects for template
                                    Treesynth *ts = new Treesynth;
                                    TreesynthIO *tsio = new TreesynthIO;
                                    strcpy( tsio->ifilename, path );

                                    // tsio is all set; to use ts, the appropriate face should
                                    //  - make and initialize a new tree of specified total levels
                                    //  - set ts->tree to the new tree
                                    //  - ts->initialize()
                                    //  - change ay ts parameters if needed
                                
                                    ts->tree = new Tree;
                                    ts->tree->initialize( lg(CUTOFF) );
                                    ts->initialize();
                                    ts->resetTreeLevels( 13 );
                                    ts->stoplevel = 9;

                                    // Now, make a new template for the residue
                                    std::string fname;
                                    Residue * temp = new Residue( ts, tsio );
                                    if( !save_as_name( fname ) )
                                        temp->name = BirdBrain::get_part_of_name( driver->res_file_name.c_str() )
                                                    + '(' + BirdBrain::toString( g_res_count ) + ')';
                                    else
                                        temp->name = BirdBrain::getname( fname.c_str() );
                                    //temp->name = BirdBrain::getname( driver->res_file_name.c_str() )
                                    //           + '(' + BirdBrain::toString( ++g_res_count ) + ')';
                                
                                    Library::instance()->add( temp );

                                    // write
                                    save_to_file( temp, fname );
                                }
                                else
                                {
                                    msg_box( "bad!", "separate first!" );
                                    bad = true;
                                }
                            }

                            if( !bad )
                            {
                                if( m_res )
                                    delete m_res; 
                                m_res = new AudioSrcFile; 
                                if( !m_res->open( path, 0, 0, FALSE ) )
                                {
                                    delete m_res;
                                    m_res = NULL;
                                }
                            }
                        }
        
                        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ RES_LOAD ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                        else if( i == BT_RES_LOAD )
                        {
                            if( m_res != NULL )
                            {
                                m_orig = m_fin;
                                load_file( m_res );
                                m_res = NULL;
                                SAFE_DELETE( driver );
                                SAFE_DELETE( ee );
								SAFE_DELETE( rc );
                                m_loadfileprev = m_loadfilename;
                                m_loadfilename = m_sndfilename;
                            }
                            else
                            {
                                msg_box( "bad!", "save first!" );
                            }
                        }

                        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ LOAD ORIG ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                        else if( i == BT_ORIG )
                        {
                            if( m_orig != NULL )
                            {
                                if( m_orig != m_fin )
                                {
                                    AudioSrcFile * temp = m_fin;
                                    load_file( m_orig );
                                    m_orig = temp;
                                    SAFE_DELETE( driver );
                                    SAFE_DELETE( ee );
                                    SAFE_DELETE( m_res );
									SAFE_DELETE( rc );
                                    std::string tempname = m_loadfilename; 
                                    m_loadfilename = m_loadfileprev;
                                    m_loadfileprev = tempname; 
                                }
                            }
                            else
                            {
                                msg_box( "moron!", "load something first!" );
                            }
                        }

                        // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~ VIEW / RENDERING BUTTONS ~~~~~~~~~~~~~~~~~~~~~~~~~
                        else if( i == BT_VIEW_SINE )
                        {
                            m_which_pane = SINE_PANE;
							m_analysis_mode = ANA_DET;
                        }
                        else if( i == BT_VIEW_TRAN )
                        {
                            m_which_pane = TRAN_PANE;
                            m_analysis_mode = ANA_TRAN;
                        }
                        else if( i == BT_VIEW_GROUP )
                        {
                            m_which_pane = GROUP_PANE;
                            m_analysis_mode = ANA_DET;
                            ui_elements[BT_GROUP].set_slide( 1.0f );
                        }
                        else if( i == BT_VIEW_RAW )
						{
							m_which_pane = RAW_PANE;
							m_analysis_mode = ANA_RAW;
						}
						else if( i == BT_VIEW_FEATURES )
						{
							m_which_pane = FEAT_PANE;
							m_analysis_mode = ANA_FEAT;
						}
						else if( i == BT_VIEW_LIB )
                        {
                            m_which_pane = LIB_PANE;
                        }
                        // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~ CHANGE TRANSIENT MODE ~~~~~~~~~~~~~~~~~~~~~~~~~
                        else if( i == BT_TRAN_METHOD )
                        {
                            ui_elements[i].slide = 1.0f - ui_elements[i].slide;
                            ui_elements[SL_TRAN_THRESH].set_slide( ui_elements[i].slide > 0.5 ? 4.5f : 1.0f );
                            
                            if( m_fin )
                            {
                                SAFE_DELETE( ee );
                                if( ui_elements[i].slide > 0.5 )
                                    ee = new EngExtractor( m_sndfilename.c_str() );
                                else
                                    ee = new EnvExtractor( m_sndfilename.c_str() );                         
                            }

                            m_curr_tran = -1;
                            g_tran_suppressed = false;
                        }
                        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ GROUPING ON/OFF ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                        //~~~~~~~~~~~~~~~ SAVE RESIDUE OF CLIP ONLY VS WHOLE FILE ~~~~~~~~~~~~~~~~~~~
                        else if( i == BT_GROUP || i == BT_CLIP_ONLY )
                        {
                            ui_elements[i].slide = 1.0f - ui_elements[i].slide; 
                        }
                        //~~~~~~~~~~~~~~~~~~~~~~ JUST CLICKING SPECTROGRAM ~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                        else if( i == UI_SPECTROGRAM )
                        {
                            if( m_specgram.curr == m_specgram.start )
                                g_show_tracks = false;
                        }
						//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ else ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                        else
                        {
							//m_audio->play( new AudioSrcTest( 220.0 * (i+1) ) );
                        }
                    }
                } // end of checkid

                // button up
                if( ie->state == ae_input_UP && ui_elements[i].down )
                    ui_elements[i].down = FALSE;
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
        }
        else if( ie->type == ae_input_MOTION )
        {
            t_CKBOOL nothing = TRUE; 
            for( i = SL_DET_TRACKS; i <= SL_TRAN_AGEAMT; i++ )
            {
                if( ui_elements[i].down )
                {
                    ui_elements[i].slide += vr_to_world_x(m_vr,ie->pos[0]) 
                                            / g_slider_height - ui_elements[i].slide_last;
                    ui_elements[i].slide_last = vr_to_world_x(m_vr,ie->pos[0]) / g_slider_height;
                    if( ui_elements[i].slide > 1.0 ) ui_elements[i].slide = 1.0; 
                    if( ui_elements[i].slide_last > 1.0 ) ui_elements[i].slide_last = 1.0;
                    if( ui_elements[i].slide < 0.0 ) ui_elements[i].slide = 0.0; 
                    if( ui_elements[i].slide_last < 0.0 ) ui_elements[i].slide_last = 0;
                    change_trans = TRUE;
                    nothing = FALSE;
                }
            }  

            for( i = SL_DET_THRESH; i <= SL_TRAN_GAP; i++ )
            {
                if( ui_elements[i].down )
                {
                    ui_elements[i].slide += (vr_to_world_x(m_vr,ie->pos[0])- .6 ) 
                                            / g_slider_height - ui_elements[i].slide_last;
                    ui_elements[i].slide_last = (vr_to_world_x(m_vr,ie->pos[0]) - .6 ) / g_slider_height;
                    if( ui_elements[i].slide > 1.0 ) ui_elements[i].slide = 1.0; 
                    if( ui_elements[i].slide_last > 1.0 ) ui_elements[i].slide_last = 1.0;
                    if( ui_elements[i].slide < 0.0 ) ui_elements[i].slide = 0.0; 
                    if( ui_elements[i].slide_last < 0.0 ) ui_elements[i].slide_last = 0;

                    change_trans = TRUE;
                    nothing = FALSE;
                }
            }  
            
            for( i = BT_SELECT_LEFT; i <= BT_SELECT_RIGHT; i++ )
            {
                if( ui_elements[i].down )
                {
                    ui_elements[i].slide += (vr_to_world_x(m_vr,ie->pos[0]) + 1.1) 
                                            / g_butter_width - ui_elements[i].slide_last;
                    if( ui_elements[BT_SELECT_RIGHT].slide - ui_elements[BT_SELECT_LEFT].slide <= .004f )
                    {
                        if( i == BT_SELECT_RIGHT )
                           ui_elements[i].slide = ui_elements[i].slide_last = ui_elements[BT_SELECT_LEFT].slide + .005f;
                        else if( i == BT_SELECT_LEFT )
                            ui_elements[i].slide = ui_elements[i].slide_last = ui_elements[BT_SELECT_RIGHT].slide - .005f;
                    }
                    else
                    {
                        ui_elements[i].slide_last = (vr_to_world_x(m_vr,ie->pos[0]) + 1.1) / g_butter_width;
                        if( ui_elements[i].slide > 1.0 ) ui_elements[i].slide = 1.0; 
                        if( ui_elements[i].slide_last > 1.0 ) ui_elements[i].slide_last = 1.0;
                        if( ui_elements[i].slide < 0.0 ) ui_elements[i].slide = 0.0; 
                        if( ui_elements[i].slide_last < 0.0 ) ui_elements[i].slide_last = 0;
                        ui_elements[i].slide_locally = false;
                    }
                    // set rect
                    set_rect( m_specgram, ui_elements );
                    nothing = FALSE;

//                  fprintf( stderr, "\n" );
//                  fprintf( stderr, "(after) slide: %f last: %f\n", ui_elements[i].slide, ui_elements[i].slide_last );
                }
            }

            for( i = SL_BRIGHTNESS; i <= SL_BRIGHTNESS; i++ )
            {
                if( ui_elements[i].down )
                {
                    ui_elements[i].slide += (vr_to_world_x(m_vr,ie->pos[0]) - .1) 
                                            / g_slider_height_mini - ui_elements[i].slide_last;
                    ui_elements[i].slide_last = (vr_to_world_x(m_vr,ie->pos[0]) - .1) / g_slider_height_mini;
                    if( ui_elements[i].slide > 1.0 ) ui_elements[i].slide = 1.0; 
                    if( ui_elements[i].slide_last > 1.0 ) ui_elements[i].slide_last = 1.0;
                    if( ui_elements[i].slide < 0.0 ) ui_elements[i].slide = 0.0; 
                    if( ui_elements[i].slide_last < 0.0 ) ui_elements[i].slide_last = 0;
                    nothing = FALSE;
                }
            }

            for( i = SL_CONTRAST; i <= SL_CONTRAST; i++ )
            {
                if( ui_elements[i].down )
                {
                    ui_elements[i].slide += (vr_to_world_x(m_vr,ie->pos[0]) - .7) 
                                            / g_slider_height_mini - ui_elements[i].slide_last;
                    ui_elements[i].slide_last = (vr_to_world_x(m_vr,ie->pos[0]) - .7) / g_slider_height_mini;
                    if( ui_elements[i].slide > 1.0 ) ui_elements[i].slide = 1.0; 
                    if( ui_elements[i].slide_last > 1.0 ) ui_elements[i].slide_last = 1.0;
                    if( ui_elements[i].slide < 0.0 ) ui_elements[i].slide = 0.0; 
                    if( ui_elements[i].slide_last < 0.0 ) ui_elements[i].slide_last = 0;
                    nothing = FALSE;
                }
            }
            
            for( i = KN_THRESH_TILT; i <= KN_THRESH_TILT; i++ )
            {
                if( ui_elements[i].down )
                {
                    ui_elements[i].slide += (vr_to_world_y(m_vr,ie->pos[1]) - .3 )
                                            / g_slider_height_mini - ui_elements[i].slide_last;
                    ui_elements[i].slide_last = (vr_to_world_y(m_vr,ie->pos[1]) - .3 ) / g_slider_height_mini;
                    if( ui_elements[i].slide > 1.0 ) ui_elements[i].slide = 1.0; 
                    if( ui_elements[i].slide_last > 1.0 ) ui_elements[i].slide_last = 1.0;
                    if( ui_elements[i].slide < 0.0 ) ui_elements[i].slide = 0.0; 
                    if( ui_elements[i].slide_last < 0.0 ) ui_elements[i].slide_last = 0;
                    nothing = FALSE;
                }
            }
            
            for( i = BT_FREQ_LEFT; i <= BT_FREQ_RIGHT; i++ )
            {
                if( ui_elements[i].down )
                {
                    ui_elements[i].slide += (vr_to_world_x(m_vr,ie->pos[0]) - .1) 
                                            / g_butter_width - ui_elements[i].slide_last;
                    if( ui_elements[BT_FREQ_RIGHT].slide - ui_elements[BT_FREQ_LEFT].slide <= .004f )
                    {
                        if( i == BT_FREQ_LEFT )
                            ui_elements[i].slide = ui_elements[i].slide_last = ui_elements[BT_FREQ_RIGHT].slide - .005f;
                        else if( i == BT_FREQ_RIGHT )
                            ui_elements[i].slide = ui_elements[i].slide_last = ui_elements[BT_FREQ_LEFT].slide + .005f;
                    }
                    else
                    {
                        ui_elements[i].slide_last = (vr_to_world_x(m_vr,ie->pos[0]) - .1) / g_butter_width;
                        if( ui_elements[i].slide > 1.0 ) ui_elements[i].slide = 1.0; 
                        if( ui_elements[i].slide_last > 1.0 ) ui_elements[i].slide_last = 1.0;
                        if( ui_elements[i].slide < 0.0 ) ui_elements[i].slide = 0.0; 
                        if( ui_elements[i].slide_last < 0.0 ) ui_elements[i].slide_last = 0;
                    }
                    // set rect
                    set_rect( m_specgram, ui_elements );
                    nothing = FALSE;
                }
            }

            if( ui_elements[UI_SPECTROGRAM].down )
            {
                m_specgram.curr = vr_to_world(m_vr,ie->pos);
                m_specgram.curr -= m_specgram.loc;
                // fprintf( stderr, "%f %f\n", m_specgram.curr[0], m_specgram.curr[1] );
                clip_rect( m_specgram, ui_elements );
                nothing = FALSE;
            }

            // bounding boxes
            if( nothing ) {
                m_cur_bbox = -1; 
                for( int b = 0; b < m_nbboxs; b++ )
                    if( m_bboxs[b].in( vr_to_world( m_vr, ie->pos ) ) ) {
                        m_cur_bbox = b;
                    }
            }

            // read in values from sliders for real-time transient extraction
            if( m_analysis_mode == ANA_TRAN && change_trans )
            {
                update_transients();
                change_trans = FALSE;
            }
        }
        else if( ie->type == ae_input_KEY )
        {
            switch( ie->key ) {
            case 'h':
            case 'H':
                m_highlight = !m_highlight;
                BB_log( BB_LOG_INFO, "Highlighting %s", m_highlight ? "ON" : "OFF" );
                break;
            case 'T':
                m_use_tex = !m_use_tex;
                BB_log( BB_LOG_INFO, "Spectrogram rendering: %s", m_use_tex ? "TEXTURE" : "QUADS" );
                break;
            case 'r': 
            case 'R':
                g_show_slider_range = !g_show_slider_range; 
                BB_log( BB_LOG_INFO, "%showing ranges of sliders", g_show_slider_range ? "S" : "Not s" ); // :)
                break;
            case 'L':
                g_rect_width %= 2;
                g_rect_width++;
                break;
            case '0':
            case  27: // escape
                m_vr.setDest( *m_vrs[0] );
            break;
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
                {
                    long index = ie->key - '0';
                    m_vr.setDest( m_vr.m_dest == m_vrs[index] ? *m_vrs[0] : *m_vrs[index] );
                }
                break;
            default:
                break;
            }
        }
    }

    return AudicleFace::on_event( event );
}




void HSVtoRGB( float h, float s, float v, float * rgb ) 
{ 
    int i; 
    float f, p, q, t,hTemp;
    float r, g, b;
  
    if( s == 0.0 || h == -1.0) // s==0? Totally unsaturated = grey so R,G and B all equal value 
    { 
      rgb[0] = v;
      rgb[1] = v;
      rgb[2] = v;
      return; 
    }

    hTemp = h/60.0f; 
    i = (int)floor( hTemp );                 // which sector 
    f = hTemp - i;                      // how far through sector 
    p = v * ( 1 - s ); 
    q = v * ( 1 - s * f ); 
    t = v * ( 1 - s * ( 1 - f ) ); 
  
    switch( i )  
    { 
    case 0:{r = v;g = t;b = p;break;} 
    case 1:{r = q;g = v;b = p;break;} 
    case 2:{r = p;g = v;b = t;break;} 
    case 3:{r = p;g = q;b = v;break;}  
    case 4:{r = t;g = p;b = v;break;} 
    case 5: default: {r = v;g = p;b = q;break;} 
    }

    rgb[0] = r;
    rgb[1] = g;
    rgb[2] = b;
} 


// checks and gets name for a template to be saved as if one is provided, otherwise returns false
bool save_as_name( std::string & myname )
{
    DirScanner dScan;
    dScan.setFileTypes( "Tapestrea Template Files (*.tap)\0*.tap\0Wave Files (*.wav)\0*.wav\0All Files (*.*)\0*.*\0" );
    fileData * fD = dScan.saveFileDialog();
    if ( fD )
    {
        if( strstr( fD->fileName.c_str(), ".tap" ) || strstr( fD->fileName.c_str(), ".wav" ) )
        {
            fprintf(stderr, "saving %s\n", fD->fileName.c_str() );
            const char * c = fD->fileName.c_str();
            myname = c;
            return true;
        }
        else
        {
            msg_box( "nice try", "only .tap or .wav files are allowed here" );
        }
    }
    return false;
}


// saves template to given filename
bool save_to_file( Template * temp, std::string fname )
{   
    if( fname.length() == 0 )
        return false; 

	if( strstr( fname.c_str(), ".tap" ) ) 
	{
	    // write .tap file
		Writer w;
		if( w.open( (char *)(fname.c_str()) ) )
		{
			w.write_template( temp ); 
			w.close();
			return true;
		}
		else
		{
			msg_box( "ding!", "cannot open file for writing!" );
		}
	}

	if( strstr( fname.c_str(), ".wav" ) )
	{
		// write sound file (later)
		//return temp->write_sndfile( fname );
	}

    return false;
}


/*t_CKBOOL get_transient( const char * filename, t_CKUINT start, t_CKUINT end )
{
  int buffsize = end - start + 1; // + 1?
  cout << "start: "<< start << endl;
  AudioSrcFile file;
  if( !file.open( filename, start, buffsize, TRUE ) )
      return FALSE;
  if( buffsize % 2 ) buffsize++;

  Frame * frame = new Frame;
  frame->alloc_waveform( buffsize );

  // get samples
  file.mtick( frame->waveform, frame->wlen );
  
  int length = buffsize;
  float rate = 44100;
  int channels = 1;

  float *envelope = new float[buffsize];
  envExtr(buffsize, frame->waveform, envelope, .4, .9);
  int transients[1024];
  int numTransients = transExtr(buffsize, envelope, transients, 1.0, .95, 2000);

  if( numTransients )
  {
    for(int i = 0; i < numTransients; i++)
      cout<<i<<"/"<<transients[i] << "/"<< buffsize <<": "<<envelope[transients[i]]<<endl;
  }
  else
  {
      cout << "no transients!\n" << endl;
  }

  return TRUE;
}


int transExtr(int envLen, float *env, int *transientIndices, float thresh,
          float ageamt, int allowableGap)
{
  float *derivs = new float[envLen];
  int i;

  derivs[0] = 0;
  for(i = 1; i < envLen; i++)
    derivs[i] = fabs(env[i] - env[i - 1]);

  float newamt = 1.0 - ageamt;
  float avgEnergy = env[0];
  int started = 0;
  int numTransients = 0;
  for(i = 0; i < envLen; i++){
    if(derivs[i] > avgEnergy * thresh){
      if(started <= 0){
        transientIndices[numTransients] = i;
        numTransients++;
      }
      started = allowableGap;
    }
    else
      started--;

    avgEnergy *= ageamt;
    avgEnergy += newamt * env[i];
  }

  delete[] derivs;

  return numTransients;
}

void envExtr(int bufLen, float *bufin, float *env, float attack, 
         float decay)
{
  float gainup = 1.0 - attack;
  float gaindn = 1.0 - decay;

  float filtOut = 0;

  for(int i = 0; i < bufLen; i++){
    float datum = fabs(bufin[i]);

    if(datum > filtOut)
      filtOut = (filtOut * attack) + (gainup * datum);
    else
      filtOut = (filtOut * decay) + (gaindn * datum);

    env[i] = filtOut;
  }
}*/
