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

#include <string>
using namespace std;


// enumeration for ui elements
enum UI_ELMENTS
{
    BT_LOAD = 0,
    BT_SAVE,
    BT_REWIND,
    BT_FF,
    BT_PLAY,
    BT_STOP,
    BT_ZOOM_IN,
    BT_ZOOM_OUT,
    BT_SEPARATE,
    BT_SELECT_LEFT,
    BT_SELECT_NOW,
    BT_SELECT_RIGHT,
    BT_FREQ_LEFT,
    BT_FREQ_RIGHT,
    BT_TOGGLE,
    BT_SYN_PLAY,
    BT_SYN_SAVE,
    BT_RES_PLAY,
    BT_RES_SAVE,

    SL_DET_TRACKS,
    SL_DET_MINPOINTS,
    SL_DET_MAXGAP,
    SL_DET_THRESH,
    SL_DET_ERROR_FREQ,
    SL_DET_NOISE_RATIO,
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
    "[classified]",
    "<",
    ">",
    "play",
    "stop",
    "zoom in",
    "zoom out",
    "separate",
    "left",
    "now",
    "right",
    "low",
    "high",
    "view ->",
    "play",
    "save",
    "play",
    "save",
    "# sine tracks",
    "min. track length",
    "allowable silence",
    "mag. threshold",
    "freq. sensitivity",
    "peak-to-noise ratio",
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


#define THRESH_POW 4.0
void set_rect( SpectroGram & s, UI_Element * ui_elements );
t_CKUINT g_out_count = 0;
t_CKUINT g_res_count = 0;





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
UIAnalysis::~UIAnalysis( ) { }




//-----------------------------------------------------------------------------
// name: init()
// desc: ...
//-----------------------------------------------------------------------------
t_CKBOOL UIAnalysis::init( )
{
    if( !AudicleFace::init() )
        return FALSE;

    int i;

    // driver
    driver = NULL;
    // frame
    frame = new Frame;
    // default filename
    m_filename = "a.wav";
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

    // rendering
    m_waveform_size = m_fft_size > 2048 ? m_fft_size : 2048;
    // zero
    m_waveform = new SAMPLE[m_waveform_size*2];
    m_buffer = new SAMPLE[m_waveform_size*2];
    m_window = new SAMPLE[m_waveform_size*2];
    // window;
    hanning( m_window, m_wnd_size );
    // file in
    m_fin= NULL;
    
    // set the name for the face
    m_name = "Birdbraind Analysis";

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
    ui_elements[BT_SAVE].size_up = .03f;
    ui_elements[BT_SYN_PLAY].size_up = .04f;
    ui_elements[BT_SYN_SAVE].size_up = .04f;
    ui_elements[BT_RES_PLAY].size_up = .04f;
    ui_elements[BT_RES_SAVE].size_up = .04f;
    // font size
    ui_elements[BT_SELECT_LEFT].font_size = .7f;
    ui_elements[BT_SELECT_RIGHT].font_size = .7f;
    ui_elements[BT_FREQ_LEFT].font_size = .7f;
    ui_elements[BT_FREQ_RIGHT].font_size = .7f;
    ui_elements[BT_SELECT_NOW].font_size = .7f;
    ui_elements[BT_TOGGLE].font_size = .7f;
    ui_elements[BT_REWIND].font_size = .7f;
    ui_elements[BT_FF].font_size = .7f;
    ui_elements[BT_SAVE].font_size = .55f;
    ui_elements[BT_SYN_PLAY].font_size = .7f;
    ui_elements[BT_SYN_SAVE].font_size = .7f;
    ui_elements[BT_RES_PLAY].font_size = .7f;
    ui_elements[BT_RES_SAVE].font_size = .7f;
    // slide parameters
    ui_elements[BT_SELECT_LEFT].slide_int = true;
    ui_elements[BT_SELECT_RIGHT].slide_int = true;
    ui_elements[BT_SELECT_NOW].slide_int = true;
    ui_elements[SL_DET_TRACKS].slide_1 = 50.0f;
    ui_elements[SL_DET_TRACKS].slide_int = true;
    ui_elements[SL_DET_MINPOINTS].slide_0 = 1.f;
    ui_elements[SL_DET_MINPOINTS].slide_1 = 20.0f;
    ui_elements[SL_DET_MINPOINTS].slide_int = true;
    ui_elements[SL_DET_MAXGAP].slide_1 = 20.0f;
    ui_elements[SL_DET_MAXGAP].slide_int = true;
    ui_elements[SL_DET_THRESH].slide_1 = ::pow( .075, 1 / THRESH_POW );
    ui_elements[SL_DET_ERROR_FREQ].slide_0 = .5f;
    ui_elements[SL_DET_ERROR_FREQ].slide_1 = 1.0f;
    ui_elements[SL_DET_NOISE_RATIO].slide_0 = 0.f;
    ui_elements[SL_DET_NOISE_RATIO].slide_1 = 20.0f;
    ui_elements[BT_FREQ_LEFT].slide_1 = 22050;
    ui_elements[BT_FREQ_LEFT].slide_int = true;
    ui_elements[BT_FREQ_RIGHT].slide_1 = 22050;
    ui_elements[BT_FREQ_RIGHT].slide_int = true;
    ui_elements[KN_THRESH_TILT].slide_0 = -80.0f;
    ui_elements[KN_THRESH_TILT].slide_1 = 80.0f;

    // default number of tracks
    ui_elements[SL_DET_TRACKS].slide_0 = 1.0f;
    ui_elements[SL_DET_TRACKS].slide = 4.0f / (ui_elements[SL_DET_TRACKS].slide_1 - ui_elements[SL_DET_TRACKS].slide_0);
    // default freq-sense
    ui_elements[SL_DET_ERROR_FREQ].slide = ( .85f - ui_elements[SL_DET_ERROR_FREQ].slide_0 ) / (ui_elements[SL_DET_ERROR_FREQ].slide_1 - ui_elements[SL_DET_ERROR_FREQ].slide_0);
    // default noise-floor
    ui_elements[SL_DET_NOISE_RATIO].slide = 3.1f / (ui_elements[SL_DET_NOISE_RATIO].slide_1 - ui_elements[SL_DET_NOISE_RATIO].slide_0);

    // bad hack (please remove) (no)
    ui_elements[BT_SAVE].slide = 0.f;

    // specgram
    m_specgram.loc[0] = .1f;
    m_specgram.loc[1] = .25f;
    // set rect
    set_rect( m_specgram, ui_elements );

    // not line
    m_threshold = new notline( m_gain * m_freq_scale * g_butter_height * 2 );

    // get audio
    m_audio = AudioCentral::instance();

    return TRUE;
}




//-----------------------------------------------------------------------------
// name: destroy()
// desc: ...
//-----------------------------------------------------------------------------
t_CKBOOL UIAnalysis::destroy( )
{
    this->on_deactivate( 0.0 );
    m_id = Audicle::NO_FACE;
    m_state = INACTIVE;

    return TRUE;
}




//-----------------------------------------------------------------------------
// name: struct Spectre
// desc: ...
//-----------------------------------------------------------------------------
struct Spectre
{
    t_CKSINGLE r;
    t_CKSINGLE g;
    t_CKSINGLE b;
};



const t_CKUINT g_rainbow_resolution = 0x10000;

Spectre g_rainbow[g_rainbow_resolution];
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

    glBegin( GL_LINE_LOOP );
    glVertex2f( left, top );
    glVertex2f( right, top );
    glVertex2f( right, bottom );
    glVertex2f( left, bottom );
    glEnd();
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

    ui_elements[BT_SELECT_LEFT].slide = left / g_butter_width;
    ui_elements[BT_SELECT_LEFT].slide_last = left / g_butter_width;
    ui_elements[BT_SELECT_RIGHT].slide = right / g_butter_width;
    ui_elements[BT_FREQ_LEFT].slide_last = bottom / g_butter_height;
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
// name: spectrogram()
// desc: ...
//-----------------------------------------------------------------------------
void UIAnalysis::spectroinit( )
{
    uint i, j;

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
        for( i = 0; i < SG_WIDTH; i++ )
            for( j = 0; j < SG_HEIGHT; j++ )
            {
                m_spectre[i][j] = 0.1f;
            }
    }
    else
    {
        float max = -10, min = 10;
        for( i = 0; i < SG_WIDTH; i++ )
        {
            m_fin->seek( (t_CKUINT)( i / (t_CKSINGLE)SG_WIDTH * m_fin->info().frames + .5f), SEEK_SET );
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
        }

        for( i = 0; i < SG_WIDTH; i++ )
            for( j = 0; j < SG_HEIGHT; j++ )
                m_spectre[i][j] = (m_spectre[i][j] - min)/(max - min);

    }
}




//-----------------------------------------------------------------------------
// name: spectrogram()
// desc: ...
//-----------------------------------------------------------------------------
void UIAnalysis::spectrogram( t_CKSINGLE x, t_CKSINGLE y )
{
    uint i, j;
    t_CKSINGLE hinc = g_butter_height / SG_HEIGHT;
    t_CKSINGLE winc = g_butter_width / SG_WIDTH;
    t_CKSINGLE yy = y;
    t_CKSINGLE xx = x;

    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    glDisable( GL_LIGHTING );
    t_CKSINGLE brightness = ui_elements[SL_BRIGHTNESS].slide;
    t_CKSINGLE contrast = ui_elements[SL_CONTRAST].slide;

    glPushName( ui_elements[UI_SPECTROGRAM].id );
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
    GLint i;
    float c[] = { 1.0f, 1.0f, .5f, 1.0f };

    // draw buttons
    draw_button( ui_elements[BT_LOAD], .15f, 0.0f, 0.0f, .5f, .5f, 1.0f );
    draw_button( ui_elements[BT_PLAY], .35f, 0.0f, 0.0f, .5f, 1.0f, .5f );
    draw_button( ui_elements[BT_STOP], .55f, 0.0f, 0.0f, 1.0f, 0.5f, .5f );
    draw_button( ui_elements[BT_SEPARATE], 1.05f, 0.0f, 0.0f, .9f, .9f, .9f );
    draw_button( ui_elements[BT_TOGGLE], 0, .8f, 0.0f, .5f, .5f, 1.0f );
    //draw_button( ui_elements[BT_ZOOM_IN], -1.0, 0.0, 0.0, 1.0, 1.0, .5 );
    //draw_button( ui_elements[BT_ZOOM_OUT], -.75, 0.0, 0.0, .4, 1.0, 1.0 );
    draw_button( ui_elements[BT_REWIND], -0.63f, 0.07f, 0.0f, .5f, .5f, .5f );
    draw_button( ui_elements[BT_FF], -0.57f, 0.07f, 0.0f, .9f, .9f, .9f );
    draw_button( ui_elements[BT_SAVE], -.6f, -.8f, 0.0f, 0.3f, 0.3f, 0.3f );
    draw_button( ui_elements[BT_SYN_PLAY], -0.25f, -0.37f, 0.0f, .5f, 1.0f, .5f );
    draw_button( ui_elements[BT_SYN_SAVE], -0.40f, -0.37f, 0.0f, 1.0f, 1.0f, .5f );
    draw_button( ui_elements[BT_RES_PLAY], -0.25f, -0.8f, 0.0f, .5f, 1.0f, .5f );
    draw_button( ui_elements[BT_RES_SAVE], -0.40f, -0.8f, 0.0f, 1.0f, 1.0f, .5f );

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

    if( m_fin )
    {
    }
    else
    {
        draw_label( "(no files loaded)", -.6, .55, 0.0, 1.0, true, c );
    }

    // draw slider
    draw_slider_h( ui_elements[SL_DET_TRACKS], 0, -.25, 0.0 );
    draw_slider_h( ui_elements[SL_DET_MINPOINTS], 0, -.50, 0.0 );
    draw_slider_h( ui_elements[SL_DET_MAXGAP], 0, -.75, 0.0 );
    draw_slider_h( ui_elements[SL_DET_THRESH], .6, -.25, 0.0 );
    draw_slider_h( ui_elements[SL_DET_ERROR_FREQ], .6, -.5, 0.0 );
    draw_slider_h( ui_elements[SL_DET_NOISE_RATIO], .6, -.75, 0.0 );
    // for rotation of sliders
    g_r += 1;

    // labels
    draw_label( "original", -1.1, 0.06, 0.0, 1.0, false, c );
    draw_label( "sinusoidal", -1.1, -0.39, 0.0, 1.0, false, c );
    draw_label( "stochastic", -1.1, -.82, 0.0, 1.0, false, c );


/*    if( !m_spec )
    {
        // draw threshold
        t_CKSINGLE threshold = ::pow( ui_elements[SL_DET_THRESH].fvalue(), THRESH_POW );
        // draw knob
        draw_slider_mini( ui_elements[KN_THRESH_TILT], 0, .25f, 0.0, .25, 1.0, .75 );
        // apply scaling to threshold
        threshold = m_gain * m_freq_scale * g_butter_height / .5f * ::pow( (double) 25 * threshold, 0.5 );
        // the line
        glColor3f( 1.0f, .5f, .5f );
        glPushMatrix();
        glTranslatef( .1f, .23f + threshold, 0.0f );
        glRotatef( ui_elements[KN_THRESH_TILT].fvalue(), 0.0f, 0.0f, 1.0f );
        glDisable( GL_LIGHTING );
        glBegin( GL_LINES );
        glVertex2f( 0.0f, 0.0f );
        glVertex2f( g_butter_width, 0.0f );
        glEnd();
        glEnable( GL_LIGHTING );
        glPopMatrix();
    }*/

    if( !m_spec )
    {
        // draw threshold
        t_CKSINGLE threshold = ::pow( ui_elements[SL_DET_THRESH].fvalue(), THRESH_POW );
        // apply scaling to threshold
        threshold = m_gain * m_freq_scale * g_butter_height / .5f * ::pow( (double) 25 * threshold, 0.5 );
        // draw knob
        draw_slider_mini( ui_elements[KN_THRESH_TILT], 0.0f, .3f, 0.0f );
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

        double delta_y = y - threshold;
        // delta_y = (delta_y < 0.0f ? -1 : 1) * ::pow( delta_y / (m_gain * m_freq_scale * g_butter_height * 2), 2.0 ) / 25.0;
        double delta_x = x / (float)g_butter_width * m_fft_size / 2;
        // figure out the actual slope for analysis (bin # on x axis vs. linear magnitude on y axis)
        m_threshold->intercept = ::pow( ui_elements[SL_DET_THRESH].fvalue(), THRESH_POW );
        m_threshold->intercept = m_gain * m_freq_scale * g_butter_height / .5f * ::pow( (double) 25 * m_threshold->intercept, 0.5 );
        m_threshold->slope = delta_y / delta_x;

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
        
        // driver?
        if( driver != NULL )
        {
            uint time_sample = (t_CKUINT)(ui_elements[BT_SELECT_NOW].slide * m_fin->info().frames);
            f = driver->get_frame( time_sample );
            if( f ) cbuf = f->cmp;
        }
        
        // still NULL?
        if( cbuf == NULL )
        {
            m_fin->seek( (t_CKUINT)(ui_elements[BT_SELECT_NOW].slide * m_fin->info().frames + .5), SEEK_SET );
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
            // draw spectrogram
            if( m_spec )
            {
 /*               // get x
                t_CKINT sx = (t_CKINT)(ui_elements[BT_SELECT_NOW].slide * g_width + .5f);
                t_CKINT low = sx - 3;
                t_CKINT high = sx + 3;
                if( low < 0 ) low = 0;
                if( high > g_width - 1 ) high = g_width - 1;
                t_CKFLOAT hh = m_fft_size / (t_CKFLOAT)m_freq_view / (t_CKFLOAT)g_height;

                //for( sx = low; sx < high; sx++ )
                //{
                    // yes?
                    if( g_spectre[sx][0].yes == FALSE )
                    {
                        // fill
                        for( i = 0; i < g_height; i++ )
                        {
                            g_spectre[sx][i].r = //10 * m_gain * m_freq_scale * 
                                .45 + ( 20.0f * log10( cmp_abs(cbuf[(int)(i * hh)]) ) + 60.0f ) / 30.0f;
                            // fprintf( stderr, "%f\n", g_spectre[sx][i].r );
                        }
                        // g_spectre[sx][0].yes = TRUE;
                    }
  */              //}
            }
            else
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

        // drawing the resynthesis and residue
        if( driver != NULL && f )
        {
            // truly bad
            cbuf = NULL;
            SAMPLE * resyn = NULL;
            // the time
            uint time_sample = (t_CKUINT)(ui_elements[BT_SELECT_NOW].slide * m_fin->info().frames);
            // if time is between range
            if( time_sample >= frame->time && (time_sample + m_wnd_size) <= (frame->time + frame->wlen) )
            {
                // get frame
                f = driver->get_frame( time_sample );
                if( f && f->time >= frame->time ) // used to be only if( f )
                {
                    // get the frame from the resynthesis
                    assert( frame != NULL );
                    assert( m_wnd_size == driver->get_window()->wlen );
                    resyn = new SAMPLE[BirdBrain::fft_size()];
                    memcpy( resyn, &frame->waveform[f->time - frame->time], m_wnd_size * sizeof(SAMPLE) );
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

          // very bad
            Frame resframe;
          if( ui_elements[BT_SAVE].slide == 0.f ) {
            // make copy
            resframe = *f;
            // get residue
            driver->ana()->get_res( driver->the_event(), resframe );
            resframe.pol2cmp();
            cbuf = resframe.cmp;
          }
          else {
            // bad
            uint time_sample = (t_CKUINT)(ui_elements[BT_SELECT_NOW].slide * m_fin->info().frames);
            f = driver->get_fake_residue( time_sample );
            resframe = *f;
            resframe.pol2cmp();
            cbuf = resframe.cmp;
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
            if( resyn )
            {
                delete [] resyn;
                resyn = NULL;
            }
        }

        glEnable( GL_LIGHTING );
    }
   
/*
            // draw the frequency domain representation
        for( i = 0; i < m_buffer_size/2; i++ )
        {
            m_spectrums[wf][i].x = x;
            if( !m_usedb )
                m_spectrums[wf][i].y = m_gain * m_freq_scale * 
                    ::pow( (double) 25 * cmp_abs( cbuf[i] ), 0.5 ) + y;
            else
                m_spectrums[wf][i].y = m_gain * m_freq_scale * 
                    ( 20.0f * log10( cmp_abs(cbuf[i])/8.0 ) + 80.0f ) / 80.0f + y + .5f;
            x += inc * m_freq_view;
        }

    m_draw[wf] = true;
    
    glPushMatrix();
    glTranslatef( fp[0], fp[1], fp[2] );

    for( i = 0; i < m_depth; i++ )
    {
        if( m_draw[(wf+i)%m_depth] )
        {
            Pt2D * pt = m_spectrums[(wf+i)%m_depth];
            fval = (m_depth-i)/(float)m_depth;
            
            glColor3f( .4f * fval, 1.0f * fval, .4f * fval );

            x = -1.8f;
            glBegin( GL_LINE_STRIP );
            for( int j = 0; j < m_buffer_size/m_freq_view; j++, pt++ )
                glVertex3f( x + j*(inc*m_freq_view), pt->y, -i * m_space + m_z );
            glEnd();
        }
    }
    
    glPopMatrix();
    
    if( !m_wutrfall )
        m_draw[wf] = false;

    wf--;
    wf %= m_depth;
*/
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
    glOrtho( -m_asp, m_asp, -1.0, 1.0, -10, 10 );
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
// name: on_event()
// desc: ...
//-----------------------------------------------------------------------------
t_CKUINT UIAnalysis::on_event( const AudicleEvent & event )
{
    static t_CKUINT m_mouse_down = FALSE;
    static t_CKUINT which = 0;
    static Point2D last;
    t_CKBOOL hit = FALSE;
    Point2D diff;
    int i, j;

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
                        if( i >= SL_DET_TRACKS && i <= SL_DET_MAXGAP )
                            ui_elements[i].slide_last = (ie->pos[0]/AudicleWindow::main()->m_vsize ) / g_slider_height;
                        if( i >= SL_DET_THRESH && i <= SL_DET_NOISE_RATIO )
                            ui_elements[i].slide_last = (ie->pos[0]/AudicleWindow::main()->m_vsize - .6) / g_slider_height;
                        if( i >= SL_BRIGHTNESS && i <= SL_BRIGHTNESS )
                            ui_elements[i].slide_last = (ie->pos[0]/AudicleWindow::main()->m_vsize - .1 ) / g_slider_height_mini;
                        if( i >= SL_CONTRAST && i <= SL_CONTRAST )
                            ui_elements[i].slide_last = (ie->pos[0]/AudicleWindow::main()->m_vsize - .7 ) / g_slider_height_mini;
                        if( i >= BT_SELECT_LEFT && i <= BT_SELECT_RIGHT )
                            ui_elements[i].slide_last = (ie->pos[0]/AudicleWindow::main()->m_vsize + 1.1) / g_butter_width;
                        if( i >= KN_THRESH_TILT && i <= KN_THRESH_TILT )
                            ui_elements[i].slide_last = (ie->pos[1]/AudicleWindow::main()->m_vsize - .3 ) / g_slider_height_mini;
                        if( i == UI_SPECTROGRAM )
                        {
                            m_specgram.start = ie->pos / AudicleWindow::main()->m_vsize;
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
                                //Template * file = new File( m_fin->filename().c_str() );
                                //LoopTemplate * loop = new LoopTemplate( *file, new Poisson, 80000.0 );
                                if( f->open( m_fin->filename().c_str(), 
                                    (t_CKUINT)( ui_elements[BT_SELECT_LEFT].slide * m_fin->info().frames),
                                    (t_CKUINT)(ui_elements[BT_SELECT_RIGHT].slide > .9999f ? 0 :
                                    ui_elements[BT_SELECT_RIGHT].slide * m_fin->info().frames - 
                                    ui_elements[BT_SELECT_LEFT].slide * m_fin->info().frames - 1) ) )
                                {
                                    f->slide = &ui_elements[BT_SELECT_NOW].slide_local;
                                    f->slide_locally = &ui_elements[BT_SELECT_NOW].slide_locally;
                                    f->on = &ui_elements[BT_PLAY].on;
                                    m_audio->bus(0)->play( f, TRUE, FALSE );
                                    ui_elements[i].on = TRUE;
                                    ui_elements[i].on_where = 0.0;
                                }
                                fprintf( stderr, "num src: %i\n", m_audio->bus(0)->num_src() );
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
                        
                        
                        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ZOOM IN ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                        else if( i == BT_TOGGLE )
                        {
                            m_spec = !m_spec;
                        }

                        
                        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ SAVE ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                        else if( i == BT_SAVE )
                        {
                            ui_elements[BT_SAVE].slide = 1.0f - ui_elements[BT_SAVE].slide;
                            // put in library
                        }


                        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ LOAD ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                        else if( i == BT_LOAD )
                        {

                            DirScanner dScan;
                            fileData * fD = dScan.openFileDialog();
                            if ( fD )
                            {
                                if( m_fin )
                                {
                                    delete m_fin;
                                    m_fin = NULL;
                                }
                                if( driver )
                                {
                                    delete driver;
                                    driver = NULL;
                                }

                                fprintf(stderr, "opening %s\n", fD->fileName.c_str() );
                                m_filename = fD->fileName.c_str();
                                if ( fD->next ) 
                                    fprintf( stderr, "opening first of multiple files...\n");
                                
                                AudioSrcFile * f = new AudioSrcFile;
                                if( !f->open( m_filename.c_str(), 0, 0 ) )
                                {
                                    msg_box( m_filename.c_str(), "cannot open file!" );
                                    delete f;
                                }
                                else
                                {
                                    // fill waveform
                                    m_fin = f;
                                    int hop = m_fin->info().frames / m_waveform_size;
                                    t_CKUINT pos = 0;
                                    SAMPLE sam;

                                    for( j = 0; j < m_waveform_size; j++ )
                                    {
                                        m_fin->mtick( &sam, 1 );
                                        m_waveform[j] = sam;
                                        pos += hop;
                                        m_fin->seek( pos, SEEK_SET );
                                    }

                                    m_fin->seek( 0, SEEK_SET );
                                    ui_elements[BT_SELECT_LEFT].slide_1 = m_fin->info().frames;
                                    ui_elements[BT_SELECT_NOW].slide_1 = m_fin->info().frames;
                                    ui_elements[BT_SELECT_RIGHT].slide_1 = m_fin->info().frames;

                                    // spectro
                                    spectroinit();

                                    g_out_count = 0;
                                    g_res_count = 0;
                                }
                            }
                        }
                        
                        
                        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ SEPARATE ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                        else if( i == BT_SEPARATE )
                        {
                            if( !m_fin )
                            {
                                msg_box( "bad!", "load first!" );
                            }
                            else
                            {
                                t_CKUINT start = (t_CKUINT)(ui_elements[BT_SELECT_LEFT].slide * m_fin->info().frames);
                                t_CKUINT end = (t_CKUINT)(ui_elements[BT_SELECT_RIGHT].slide * m_fin->info().frames);
                                t_CKFLOAT low = ui_elements[BT_FREQ_LEFT].slide * m_fin->info().samplerate / 2.0;
                                t_CKFLOAT high = ui_elements[BT_FREQ_RIGHT].slide * m_fin->info().samplerate / 2.0;
                                t_CKUINT num_tracks= atoi(ui_elements[SL_DET_TRACKS].value().c_str());
                                t_CKUINT minpoints = atoi(ui_elements[SL_DET_MINPOINTS].value().c_str());
                                t_CKUINT maxgap = atoi(ui_elements[SL_DET_MAXGAP].value().c_str());
                                //t_CKFLOAT threshold = ::pow( ui_elements[SL_DET_THRESH].fvalue(), THRESH_POW );
                                //line threshold;
                                //threshold.intercept = ::pow( ui_elements[SL_DET_THRESH].fvalue(), THRESH_POW );
                                //threshold.slope = ::tan( ui_elements[KN_THRESH_TILT].fvalue() * PIE / 180.0 );
                                //threshold.slope = ::pow( threshold.slope, THRESH_POW ); // random
                                //threshold.slope /= (float)(m_fft_size);
                                //fprintf( stderr, "slope: %.12f  intercept: %.12f\n", g_threshold->slope, g_threshold->intercept );
                                //fprintf( stderr, "y: %f\n", g_threshold.slope * m_fft_size / 2 + g_threshold.intercept );
                                t_CKFLOAT error_f = ui_elements[SL_DET_ERROR_FREQ].fvalue();
                                t_CKFLOAT noise_r = ui_elements[SL_DET_NOISE_RATIO].fvalue();

                                if( driver ) delete driver;
                                driver = run( (char *)m_filename.c_str(), start, end, low, high, m_threshold, minpoints, maxgap, error_f, noise_r, num_tracks, m_fft_size, m_wnd_size );

                                AudioSrcFrame * f = NULL;
                                if( driver->the_event().size() )
                                {
                                    // honk
                                    driver->honk( driver->the_event(), *frame );
									
                                    f = new AudioSrcFrame( frame );
                                    f->slide = &ui_elements[BT_SELECT_NOW].slide_local;
                                    f->slide_locally = &ui_elements[BT_SELECT_NOW].slide_locally;
                                    f->on = &ui_elements[BT_SYN_PLAY].on;
                                }
                                else
                                {
                                    msg_box( "my precious!", "no tracksssss!" );
                                }

                                // brake
                                driver->brake();
                                
                                // play
                                if( f )
                                {
                                    m_audio->bus(0)->play( f, TRUE );
                                    ui_elements[BT_SYN_PLAY].on = TRUE;
                                    ui_elements[BT_SYN_PLAY].on_where = 0.0;
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


                        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ SYN_PLAY ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                        else if( i == BT_SYN_PLAY )
                        {
                            AudioSrcFrame * f = NULL;
                            if( driver )
                            {
                                if( driver && driver->the_event().size() )
                                {
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


                        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ SYN_SAVE ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                        else if( i == BT_SYN_SAVE )
                        {
                            if( driver != NULL )
                            {
                                Deterministic * temp = new Deterministic( driver->the_event() );
                                temp->name = BirdBrain::get_part_of_name( driver->out_file_name.c_str() )
                                            + '(' + BirdBrain::toString( ++g_out_count ) + ')';
                                //temp->name = BirdBrain::getname( driver->out_file_name.c_str() )
                                //           + '(' + BirdBrain::toString( ++g_out_count ) + ')';
                                Library::instance()->add( temp );
                            }
                            else
                            {
                                msg_box( "bad!", "separate first!" );
                            }
                        }

                        
                        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ RES_PLAY ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                        else if( i == BT_RES_PLAY )
                        {
                            AudioSrcFile * f = NULL;
                            char path[1024] = "";
                            const char * a = NULL;
                            const char * b = NULL;
                            if( driver )
                            {
                                a = m_fin->filename().c_str();
                                b = BirdBrain::getbase( m_fin->filename().c_str() );
                                strcpy( path, a );
                                strcpy( path + (b-a), "res_" );
                                strcat( path, b ); 
                                f = new AudioSrcFile;
                                if( f->open( path, 0, 0, FALSE ) )
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


                        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ RES_SAVE ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                        else if( i == BT_RES_SAVE )
                        {
                            if( driver != NULL )
                            {
                                fprintf( stderr, "writing residue file...\n" );
                                // write out residue
						        driver->write_res();

								// make new Treesynth and TreesynthIO objects for template
								Treesynth *ts = new Treesynth;
								TreesynthIO *tsio = new TreesynthIO;
								strcpy( tsio->ifilename, driver->res_file_name.c_str() );

								// tsio is all set; to use ts, the appropriate face should
								//	- make and initialize a new tree of specified total levels
								//	- set ts->tree to the new tree
								//  - ts->initialize()
								//  - change ay ts parameters if needed
                                
                                ts->tree = new Tree;
                                ts->tree->initialize( lg(CUTOFF) );
                                ts->initialize();
                                ts->resetTreeLevels( 13 );
                                ts->stoplevel = 9;

								// Now, make a new template for the residue
								Residue * temp = new Residue( ts, tsio );
                                temp->name = BirdBrain::get_part_of_name( driver->res_file_name.c_str() )
                                            + '(' + BirdBrain::toString( ++g_res_count ) + ')';
                                //temp->name = BirdBrain::getname( driver->res_file_name.c_str() )
                                //           + '(' + BirdBrain::toString( ++g_res_count ) + ')';
                                
                                Library::instance()->add( temp );
                            }
                            else
                            {
                                msg_box( "bad!", "separate first!" );
                            }
                        }
                                                
                        
                        //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ else ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                        else
                        {
                            //m_audio->play( new AudioSrcTest( 220.0 * (i+1) ) );
                        }
                    }
                }

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
            for( i = SL_DET_TRACKS; i <= SL_DET_MAXGAP; i++ )
            {
                if( ui_elements[i].down )
                {
                    ui_elements[i].slide += (ie->pos[0]/AudicleWindow::main()->m_vsize ) 
                                            / g_slider_height - ui_elements[i].slide_last;
                    ui_elements[i].slide_last = (ie->pos[0]/AudicleWindow::main()->m_vsize ) / g_slider_height;
                    if( ui_elements[i].slide > 1.0 ) ui_elements[i].slide = ui_elements[i].slide_last = 1.0;
                    if( ui_elements[i].slide < 0.0 ) ui_elements[i].slide = ui_elements[i].slide_last = 0;
                }
            }  

            for( i = SL_DET_THRESH; i <= SL_DET_NOISE_RATIO; i++ )
            {
                if( ui_elements[i].down )
                {
                    ui_elements[i].slide += (ie->pos[0]/AudicleWindow::main()->m_vsize - .6 ) 
                                            / g_slider_height - ui_elements[i].slide_last;
                    ui_elements[i].slide_last = (ie->pos[0]/AudicleWindow::main()->m_vsize - .6 ) / g_slider_height;
                    if( ui_elements[i].slide > 1.0 ) ui_elements[i].slide = ui_elements[i].slide_last = 1.0;
                    if( ui_elements[i].slide < 0.0 ) ui_elements[i].slide = ui_elements[i].slide_last = 0;
                }
            }  
            
            for( i = BT_SELECT_LEFT; i <= BT_SELECT_RIGHT; i++ )
            {
                if( ui_elements[i].down )
                {
                    ui_elements[i].slide += (ie->pos[0]/AudicleWindow::main()->m_vsize + 1.1) 
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
                        ui_elements[i].slide_last = (ie->pos[0]/AudicleWindow::main()->m_vsize + 1.1) / g_butter_width;
                        if( ui_elements[i].slide > 1.0 ) ui_elements[i].slide = ui_elements[i].slide_last = 1.0;
                        if( ui_elements[i].slide < 0.0 ) ui_elements[i].slide = ui_elements[i].slide_last = 0;
                        ui_elements[i].slide_locally = false;
                    }
                    // set rect
                    set_rect( m_specgram, ui_elements );
                }
            }

            for( i = SL_BRIGHTNESS; i <= SL_BRIGHTNESS; i++ )
            {
                if( ui_elements[i].down )
                {
                    ui_elements[i].slide += (ie->pos[0]/AudicleWindow::main()->m_vsize - .1) 
                                            / g_slider_height_mini - ui_elements[i].slide_last;
                    ui_elements[i].slide_last = (ie->pos[0]/AudicleWindow::main()->m_vsize - .1) / g_slider_height_mini;
                    if( ui_elements[i].slide > 1.0 ) ui_elements[i].slide = ui_elements[i].slide_last = 1.0;
                    if( ui_elements[i].slide < 0.0 ) ui_elements[i].slide = ui_elements[i].slide_last = 0;
                }
            }

            for( i = SL_CONTRAST; i <= SL_CONTRAST; i++ )
            {
                if( ui_elements[i].down )
                {
                    ui_elements[i].slide += (ie->pos[0]/AudicleWindow::main()->m_vsize - .7) 
                                            / g_slider_height_mini - ui_elements[i].slide_last;
                    ui_elements[i].slide_last = (ie->pos[0]/AudicleWindow::main()->m_vsize - .7) / g_slider_height_mini;
                    if( ui_elements[i].slide > 1.0 ) ui_elements[i].slide = ui_elements[i].slide_last = 1.0;
                    if( ui_elements[i].slide < 0.0 ) ui_elements[i].slide = ui_elements[i].slide_last = 0;
                }
            }
            
            for( i = KN_THRESH_TILT; i <= KN_THRESH_TILT; i++ )
            {
                if( ui_elements[i].down )
                {
                    ui_elements[i].slide += (ie->pos[1]/AudicleWindow::main()->m_vsize - .3 )
                                            / g_slider_height_mini - ui_elements[i].slide_last;
                    ui_elements[i].slide_last = (ie->pos[1]/AudicleWindow::main()->m_vsize - .3 ) / g_slider_height_mini;
                    if( ui_elements[i].slide > 1.0 ) ui_elements[i].slide = ui_elements[i].slide_last = 1.0;
                    if( ui_elements[i].slide < 0.0 ) ui_elements[i].slide = ui_elements[i].slide_last = 0;
                }
            }
            
            for( i = BT_FREQ_LEFT; i <= BT_FREQ_RIGHT; i++ )
            {
                if( ui_elements[i].down )
                {
                    ui_elements[i].slide += (ie->pos[0]/AudicleWindow::main()->m_vsize - .1) 
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
                        ui_elements[i].slide_last = (ie->pos[0]/AudicleWindow::main()->m_vsize - .1) / g_butter_width;
                        if( ui_elements[i].slide > 1.0 ) ui_elements[i].slide = ui_elements[i].slide_last = 1.0;
                        if( ui_elements[i].slide < 0.0 ) ui_elements[i].slide = ui_elements[i].slide_last = 0;
                    }
                    // set rect
                    set_rect( m_specgram, ui_elements );
                }
            }

            if( ui_elements[UI_SPECTROGRAM].down )
            {
                m_specgram.curr = ie->pos / AudicleWindow::main()->m_vsize;
                m_specgram.curr -= m_specgram.loc;
                // fprintf( stderr, "%f %f\n", m_specgram.curr[0], m_specgram.curr[1] );
                clip_rect( m_specgram, ui_elements );
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
