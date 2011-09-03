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
// file: taps_treesynth.h
// desc: taps wavelet tree synthesis
//
// author: Ananya Misra (amisra@cs.princeton.edu)
//         Perry R. Cook (prc@cs.princeton.edu)
//         Ge Wang (gewang@cs.princeton.edu)
// date: Autumn 2004
//-----------------------------------------------------------------------------
#include "ui_audio.h"
#include "ui_treesynth.h"
#include "ui_element.h"
#include "audicle_utils.h"
#include "audicle_gfx.h"
#include "audicle_geometry.h"
#include "audicle.h"
#include "taps_birdbrain.h"
#include "taps_sceptre.h"
#include "ui_library.h"

#include <string>
using namespace std;

// enumeration for ui elements
enum UI_ELMENTS
{
    // sliders
    SL_PERCENTAGE = 0,
    SL_K,
    SL_TOTAL_LEVELS,
    SL_STARTLEVEL,
    SL_STOPLEVEL,

    // flips
    FL_ANCFIRST,
    FL_RANDFLIP,
    FL_WRITE_FILE,

    // buttons
    BT_LOAD_ANALYSIS,
    BT_LOAD_FILE,
    BT_SYNTHESIZE,
    BT_STOP,

    // keep this as last
    NUM_UI_ELEMENTS
};


// names of ui elements
static char * ui_str[] = { 
    "percent",
    "k",
    "total-levels",
    "start-level",
    "stop-level",
    "ancestor first",
    "random flip",
    "write to file",
    "load from analysis",
    "load from file",
    "synthesize",
    "stop"
};


// globals (for now) (but then this would have to include Eliot.h)
Treesynth *g_ts;
TreesynthIO *g_tsio;
Tree * g_tstree;
bool g_shutup = true, g_lib = false, g_lefttree;
char g_ifile[1024], g_ofile[1024];
XThread * g_ts_thread;
XMutex g_mutex;

// init treesynth
void ts_init( UI_Element ** ui_elements )
{
    if( g_lib )
    {
        Template *temp = NULL;
        for(int i = 0; i < Library::instance()->templates.size(); i++ ) {
            temp = Library::instance()->templates[i]->core;
            if( temp->type == TT_RESIDUE ) {
                Residue *res = (Residue *)temp;
                g_ts = res->ts;
                g_tsio = res->tsio;
                
                break;
            }
            else
                temp = NULL;
        }
    }
    else
    {
        g_ts = new Treesynth();
		int datasize = ui_elements[SL_TOTAL_LEVELS]->ivalue();
		g_tsio = g_ts->initialize( g_ifile, datasize, datasize / 4 );
        //g_tsio = new TreesynthIO();

        //strcpy( g_tsio->ifilename, g_ifile );
        //strcpy( g_tsio->ofilename, "" );
        //g_tsio->rm_mode = RM_WRAP | RM_FORWARD;

        //g_tsio->write_to_file = ui_elements[FL_WRITE_FILE]->slide > .5;
        //g_tsio->write_to_buffer = true;
    }

//    g_tstree = new Tree();
//    g_tstree->initialize(  (int)ui_elements[SL_TOTAL_LEVELS]->fvalue() );
        
    //read source sound into g_tstree->values() in some way
    fprintf( stderr, "%s\n", g_tsio->ifilename );
//    int samples = g_tsio->ReadSoundFile( g_tsio->ifilename, g_tstree->values(), g_tstree->getSize() );
//    if( samples <= 0 )
//        msg_box( "bad", "Can't read input file; stop at once." );
    //or memcpy into g_tstree->values()

//  g_ts->tree = g_tstree;
//    g_ts->initialize();

    g_ts->ancfirst = ui_elements[FL_ANCFIRST]->slide > .5;
    g_ts->kfactor = ui_elements[SL_K]->fvalue();
    g_ts->percentage = ui_elements[SL_PERCENTAGE]->fvalue();
    g_ts->randflip = ui_elements[FL_RANDFLIP]->slide >.5;
    g_ts->startlevel = ui_elements[SL_STARTLEVEL]->ivalue();
    g_ts->stoplevel = ui_elements[SL_STOPLEVEL]->ivalue();

    // let's see if you still work. This is a challenge.
}


// just for fun
void ts_end()
{
    if( g_lib ) {
        g_ts = NULL;
        g_tsio = NULL;
        return;
    }
    SAFE_DELETE( g_ts );
    SAFE_DELETE( g_tsio );
}



#ifdef __PLATFORM_WIN32__
unsigned __stdcall ts_start( void * data )
#else
void * ts_start( void * data )
#endif
{
    int samples, write = 1;
    fprintf( stderr, "you are now entering the ts_start() function...\n" );
    while( !g_shutup ) {
        g_mutex.acquire();
        if( g_ts->setup() ) {
            g_ts->synth();
            // keep trying to write to file/buffer, until written or told to shut up
            while( !g_shutup && 
                   !(write = g_tsio->WriteSoundFile( g_tsio->ofilename, g_ts->outputSignal(), g_ts->tree->getSize() ) ) )
            {
                g_mutex.release();
                usleep( 10000 );
                g_mutex.acquire();
            }
        }
        g_mutex.release();
        
        // may work 
        if( g_lefttree ) {
            if( g_ts->lefttree == NULL ) {
                g_ts->lefttree = new Tree();
                g_ts->lefttree->initialize( g_ts->tree->getLevels() );
            }
            g_ts->lefttree->setValues( g_ts->tree->values(), g_ts->tree->getSize() );
        }

        // read input for next iteration
        samples = g_tsio->ReadSoundFile( g_tsio->ifilename, g_ts->tree->values(), g_ts->tree->getSize() );
    }

    g_mutex.acquire();
    ts_end();
    g_mutex.release();

    fprintf( stderr, "you are now leaving the ts_start() function..., good luck.\n" );
    return 0;
}


// run treesynth
void ts_run()
{
    if( g_ts_thread )
        delete g_ts_thread;

    g_shutup = FALSE;
    g_ts_thread = new XThread;
    g_ts_thread->start( ts_start, NULL );
}



//-----------------------------------------------------------------------------
// name: UITreesynth()
// desc: ...
//-----------------------------------------------------------------------------
UITreesynth::UITreesynth( ) : AudicleFace( )
{ 
    if( !this->init( ) )
    {
        fprintf( stderr, "[audicle]: cannot start face...\n" );
        return;
    }
}




//-----------------------------------------------------------------------------
// name: ~UITreesynth()
// desc: ...
//-----------------------------------------------------------------------------
UITreesynth::~UITreesynth( ) { }



//-----------------------------------------------------------------------------
// name: init()
// desc: ...
//-----------------------------------------------------------------------------
t_TAPBOOL UITreesynth::init( )
{
    if( !AudicleFace::init() )
        return FALSE;

    int i;

    // ui elements
	ui_elements = new UI_Element *[NUM_UI_ELEMENTS];

    // get id for each element
    for( i = 0; i < NUM_UI_ELEMENTS; i++ )
    {
		ui_elements[i] = new UI_Element;
        ui_elements[i]->id = IDManager::instance()->getPickID();
        ui_elements[i]->name = ui_str[i];
    }

    // slide
    ui_elements[SL_PERCENTAGE]->slide = .25f;
    ui_elements[SL_K]->slide = .3f;
    // ranges
    ui_elements[SL_STARTLEVEL]->slide_0 = 1;
    ui_elements[SL_STARTLEVEL]->slide_1 = lg(CUTOFF) - 3;
    ui_elements[SL_STARTLEVEL]->slide_int = TRUE;
    ui_elements[SL_STOPLEVEL]->slide_0 = 1;
    ui_elements[SL_STOPLEVEL]->slide_1 = lg(CUTOFF) - 2;
    ui_elements[SL_STOPLEVEL]->slide_int = TRUE;
    ui_elements[SL_TOTAL_LEVELS]->slide_0 = 3;
    ui_elements[SL_TOTAL_LEVELS]->slide_1 = lg(CUTOFF);
    ui_elements[SL_TOTAL_LEVELS]->slide_int = TRUE;
    // slide
    ui_elements[SL_STARTLEVEL]->slide = 0.0f;
    ui_elements[SL_STOPLEVEL]->slide = (9.0f - ui_elements[SL_STOPLEVEL]->slide_0) / (ui_elements[SL_STOPLEVEL]->slide_1 - ui_elements[SL_STOPLEVEL]->slide_0);
    ui_elements[SL_TOTAL_LEVELS]->slide = (11.0f - ui_elements[SL_TOTAL_LEVELS]->slide_0) / (ui_elements[SL_TOTAL_LEVELS]->slide_1 - ui_elements[SL_TOTAL_LEVELS]->slide_0);

    // button font
    ui_elements[BT_LOAD_FILE]->font_size = 0.9f;
    ui_elements[BT_LOAD_ANALYSIS]->font_size = 0.9f;

    // get audio
    m_audio = AudioCentral::instance();

    m_bg[0] = 1.0; m_bg[1] = .75; m_bg[2] = .25; m_bg[3] = 1.0;
    
	// view region
	m_vrs.push_back( new ViewRegionManual( 0, 0, -1.0f, 1.0f, FALSE, FALSE ) );

    return TRUE;
}




//-----------------------------------------------------------------------------
// name: destroy()
// desc: ...
//-----------------------------------------------------------------------------
t_TAPBOOL UITreesynth::destroy( )
{
    this->on_deactivate( 0.0 );
    m_id = Audicle::NO_FACE;
    m_state = INACTIVE;

    return TRUE;
}




//-----------------------------------------------------------------------------
// name: render()
// desc: ...
//-----------------------------------------------------------------------------
t_TAPUINT UITreesynth::render( void * data )
{
    // slider
    draw_slider( *ui_elements[SL_PERCENTAGE], 0.0f, 0.0f, 0.0f );
    draw_slider( *ui_elements[SL_K], 0.25f, 0.0f, 0.0f );
    draw_slider( *ui_elements[SL_STARTLEVEL], 0.5f, 0.0f, 0.0f );
    draw_slider( *ui_elements[SL_STOPLEVEL], 0.75f, 0.0f, 0.0f );
    draw_slider( *ui_elements[SL_TOTAL_LEVELS], 1.0f, 0.0f, 0.0f );

    // flipper
    draw_flipper( *ui_elements[FL_ANCFIRST], -.75f, 0.0f, 0.0f );
    draw_flipper( *ui_elements[FL_RANDFLIP], -.75f, 0.25f, 0.0f );
    draw_flipper( *ui_elements[FL_WRITE_FILE], -.05f, 0.66f, 0.0f );

    // button
    draw_button( *ui_elements[BT_SYNTHESIZE], -.75f, -.4f, 0.0f, 1.0f, 1.0f, 1.0f );
    draw_button( *ui_elements[BT_STOP], -.55f, -.4f, 0.0f, 1.0f, .5f, .5f );
    draw_button( *ui_elements[BT_LOAD_ANALYSIS], -.85f, 0.7f, 0.0f, .4f, .4f, .4f );
    draw_button( *ui_elements[BT_LOAD_FILE], -.45f, 0.7f, 0.0f, .5f, .5f, .5f );

    // bad
    g_r += 1;

    return 0;
}




//-----------------------------------------------------------------------------
// name: render_pre()
// desc: ...
//-----------------------------------------------------------------------------
void UITreesynth::render_pre()
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
void UITreesynth::render_post()
{
    glPopAttrib();

    AudicleFace::render_post();
}




//-----------------------------------------------------------------------------
// name: render_view()
// desc: ...
//-----------------------------------------------------------------------------
void UITreesynth::render_view( )
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
t_TAPUINT UITreesynth::on_activate()
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
t_TAPUINT UITreesynth::on_deactivate( t_TAPDUR dur )
{
    return AudicleFace::on_deactivate( dur );
}




//-----------------------------------------------------------------------------
// name: on_event()
// desc: ...
//-----------------------------------------------------------------------------
t_TAPUINT UITreesynth::on_event( const AudicleEvent & event )
{
    static t_TAPUINT m_mouse_down = FALSE;
    static t_TAPUINT which = 0;
    static Point2D last;
    t_TAPBOOL hit = FALSE;
    Point2D diff;
    t_TAPUINT i;

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
                    if( ie->state == ae_input_DOWN )
                    {
                        hit = TRUE;
                        ui_elements[i]->down = TRUE;
                        
                        if( i >= SL_PERCENTAGE && i <= SL_STOPLEVEL )
                            ui_elements[i]->slide_last = (ie->pos[1]/AudicleWindow::main()->m_vsize ) / g_slider_height;
                    
                        
                        if( i == FL_ANCFIRST )
                        {
                            ui_elements[i]->slide = 1.0f - ui_elements[i]->slide;
                            if( g_ts )
                            {
                                g_mutex.acquire();
                                g_ts->ancfirst = ui_elements[FL_ANCFIRST]->fvalue() > 0.5;
                                g_mutex.release();
                            }
                        }

                        if( i == FL_RANDFLIP )
                        {
                            ui_elements[i]->slide = 1.0f - ui_elements[i]->slide;
                            if( g_ts )
                            {
                                g_mutex.acquire();
                                g_ts->randflip = ui_elements[FL_RANDFLIP]->fvalue() > 0.5;
                                g_mutex.release();
                            }
                        }

                        if( i == FL_WRITE_FILE )
                        {
                            if( g_shutup )
                            {
                                ui_elements[i]->slide = 1.0f - ui_elements[i]->slide;
                            }
                            else
                            {
                                msg_box( "bad!", "wavelet-tree already synthesizing!" );
                            } 
                        }
                    }

                    if( ie->state == ae_input_UP && ui_elements[i]->down == TRUE )
                    {

                        // ~~~~~~~~~~~~~~~~~~~~~~~~ LOAD FROM FILE ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                        if( i == BT_LOAD_FILE )
                        {
                            m_audio->bus(1)->stop();
                            usleep( 100000 );
                            g_shutup = TRUE;
                            g_lib = FALSE;
                            DirScanner dScan;
                            fileData * fD = dScan.openFileDialog();
                            if ( fD )
                            {
                                fprintf(stderr, "residue file %s\n", fD->fileName.c_str() );
                                if ( fD->next ) 
                                    fprintf( stderr, "may not be opening first of multiple files...\n");
                                
                                strcpy( g_ifile, fD->fileName.c_str() );
                            }
                        }

                        
                        // ~~~~~~~~~~~~~~~~~~~~~~~~ LOAD FROM ANALYSIS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                        if( i == BT_LOAD_ANALYSIS )
                        {
                            m_audio->bus(1)->stop();
                            usleep( 100000 );
                            g_shutup = TRUE;
                            g_lib = TRUE;
                        }


                        // ~~~~~~~~~~~~~~~~~~~~~~~~ SYNTHESIZE ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                        if( i == BT_SYNTHESIZE )
                        {
                            if( g_shutup )
                            {
                                ts_init( ui_elements );
                                ts_run();
                                AudioSrcEliot * ts = new AudioSrcEliot( g_tsio );
                                ts->on = &ui_elements[BT_SYNTHESIZE]->on;
                                m_audio->bus(1)->play( ts, FALSE );
                                ui_elements[BT_SYNTHESIZE]->on = TRUE;
                                ui_elements[BT_SYNTHESIZE]->on_where = 0.0;
                            }
                            else
                            {
                                msg_box( "bad!", "wavelet-tree already synthesizing!" );
                            }
                        }


                        // ~~~~~~~~~~~~~~~~~~~~~~~~ STOP ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                        if( i == BT_STOP )
                        {
                            m_audio->bus(1)->stop();
                            usleep( 100000 );
                            g_shutup = TRUE;
                        }

                    }
                }

                // button up
                if( ie->state == ae_input_UP && ui_elements[i]->down )
                    ui_elements[i]->down = FALSE;
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
            for( i = SL_PERCENTAGE; i <= SL_STOPLEVEL; i++ )
            {
                if( ui_elements[i]->down )
                {
                    ui_elements[i]->slide += (ie->pos[1]/AudicleWindow::main()->m_vsize ) 
                                            / g_slider_height - ui_elements[i]->slide_last;
                    ui_elements[i]->slide_last = (ie->pos[1]/AudicleWindow::main()->m_vsize ) / g_slider_height;
                    if( ui_elements[i]->slide > 1.0 ) ui_elements[i]->slide = ui_elements[i]->slide_last = 1.0;
                    if( ui_elements[i]->slide < 0.0 ) ui_elements[i]->slide = ui_elements[i]->slide_last = 0;
                }
            }

            if( ui_elements[SL_STARTLEVEL]->down )
            {
                if( g_ts )
                {
                    g_mutex.acquire();
                    g_ts->startlevel = ui_elements[SL_STARTLEVEL]->ivalue();
                    g_mutex.release();
                }
            }

            if( ui_elements[SL_STOPLEVEL]->down )
            {
                if( g_ts )
                {
                    g_mutex.acquire();
                    g_ts->stoplevel = ui_elements[SL_STOPLEVEL]->ivalue();
                    g_mutex.release();
                }
            }

            if( ui_elements[SL_PERCENTAGE]->down )
            {
                if( g_ts )
                {
                    g_mutex.acquire();
                    g_ts->percentage = ui_elements[SL_PERCENTAGE]->fvalue();
                    g_mutex.release();
                }
            }

            if( ui_elements[SL_K]->down )
            {
                if( g_ts )
                {
                    g_mutex.acquire();
                    g_ts->kfactor = ui_elements[SL_K]->fvalue();
                    g_mutex.release();
                }
            }

            if( ui_elements[FL_ANCFIRST]->down )
            {
                if( g_ts )
                {
                    g_mutex.acquire();
                    g_ts->ancfirst = ui_elements[FL_ANCFIRST]->fvalue() > 0.5;
                    g_mutex.release();
                }
            }

            if( ui_elements[FL_RANDFLIP]->down )
            {
                if( g_ts )
                {
                    g_mutex.acquire();
                    g_ts->randflip = ui_elements[FL_RANDFLIP]->fvalue() > 0.5;
                    g_mutex.release();
                }
            }
        }
    }

    return AudicleFace::on_event( event );
}
