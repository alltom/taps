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
// name: ui_control.cpp
// desc: control panel
//
// authors: Ananya Misra (amisra@cs.princeton.edu)
//          Ge Wang (gewang@cs.princeton.edu)
//          Perry R. Cook (prc@cs.princeton.edu)
// date: Spring 2006
//-----------------------------------------------------------------------------
#include "ui_control.h"
#include "ui_element.h"
#include "audicle_utils.h"
#include "audicle_gfx.h"
#include "audicle_geometry.h"
#include "audicle.h"
#include "taps_birdbrain.h"

#include <string>
using namespace std;

UIControl * g_control_face;

// enumeration for ui elements
enum UI_ELMENTS
{
    // sliders
    SL_BUS_0 = 0,
    SL_BUS_1,
    SL_BUS_2,
    SL_BUS_3,
    SL_BUS_4,
    SL_BUS_5,
    SL_BUS_6,
    SL_BUS_7,

    SL_REV_0,
    SL_REV_1,
    SL_REV_2,
    SL_REV_3,
    SL_REV_4,
    SL_REV_5,
    SL_REV_6,
    SL_REV_7,

	// buttons
	// face switchers
	BT_ANALYSIS_FACE,
	BT_SYNTHESIS_FACE,
	BT_GROUP_FACE,
	BT_CONTROL_FACE,
	BT_SEARCH_FACE,
	// ctrl
	BT_QUIT,
	BT_FULLSCREEN,
    
    // flips
    FL_REV_0,
    FL_REV_1,
    FL_REV_2,
    FL_REV_3,
    FL_REV_4,
    FL_REV_5,
    FL_REV_6,
    FL_REV_7,

    // keep this as last
    NUM_UI_ELEMENTS
};


// names of ui elements
static char * ui_str[] = { 
    "bus0",
    "bus1",
    "bus2",
    "bus3",
    "bus4",
    "bus5",
    "bus6",
    "bus7",
    "rev0",
    "rev1",
    "rev2",
    "rev3",
    "rev4",
    "rev5",
    "rev6",
    "rev7",
	"analysis",
	"synthesis",
	"group",
	"control",
	"search",
	"exit",
	"fullscreen",
    "on0",
	"on1",
	"on2",
	"on3",
	"on4",
	"on5",
	"on6",
	"on7"
};




//-----------------------------------------------------------------------------
// name: UIControl()
// desc: ...
//-----------------------------------------------------------------------------
UIControl::UIControl( ) : AudicleFace( )
{ 
    if( !this->init( ) )
    {
        fprintf( stderr, "[audicle]: cannot start face...\n" );
        return;
    }
}




//-----------------------------------------------------------------------------
// name: ~UIControl()
// desc: ...
//-----------------------------------------------------------------------------
UIControl::~UIControl( ) 
{ 
	for( int i = 0; i < NUM_UI_ELEMENTS; i++ )
	{
		SAFE_DELETE( ui_elements[i] );
	}
	SAFE_DELETE_ARRAY( ui_elements );
}




//-----------------------------------------------------------------------------
// name: init()
// desc: ...
//-----------------------------------------------------------------------------
t_TAPBOOL UIControl::init( )
{
    if( !AudicleFace::init() )
        return FALSE;

    int i;

    // ui elements
    ui_elements = new UI_Element *[NUM_UI_ELEMENTS];
    for( i = SL_BUS_0; i <= SL_BUS_7; i++ )
    {
        ui_elements[i] = new UI_Element;
        ui_elements[i]->set_bounds( 0.0f, 3.0f );
    }
    for( i = SL_REV_0; i <= SL_REV_7; i++ )
        ui_elements[i] = new UI_Exp( 10 );
    for( i = FL_REV_0; i <= FL_REV_7; i++ )
        ui_elements[i] = new UI_Element;
	for( i = BT_ANALYSIS_FACE; i <= BT_SEARCH_FACE; i++ )
		ui_elements[i] = new UI_Element;
	for( i = BT_QUIT; i <= BT_FULLSCREEN; i++ )
		ui_elements[i] = new UI_Element;

    // get id for each element
    for( i = 0; i < NUM_UI_ELEMENTS; i++ )
    {
        ui_elements[i]->id = IDManager::instance()->getPickID();
        ui_elements[i]->name = ui_str[i];
    }

    // slide
    for( i = SL_BUS_0; i <= SL_BUS_7; i++ )
        ui_elements[i]->set_slide( .5f );
    for( i = SL_REV_0; i <= SL_REV_7; i++ )
        ui_elements[i]->set_slide( 0 );

    // get audio
    m_audio = AudioCentral::instance();

    m_bg[0] = .8; m_bg[1] = .8; m_bg[2] = 1.0; m_bg[3] = 1.0;

    // load sliders
    load_sliders = true;

    // set
    if( !g_control_face )
        g_control_face = this;
    
    return TRUE;
}




//-----------------------------------------------------------------------------
// name: destroy()
// desc: ...
//-----------------------------------------------------------------------------
t_TAPBOOL UIControl::destroy( )
{
    this->on_deactivate( 0.0 );
    m_id = Audicle::NO_FACE;
    m_state = INACTIVE;

    for( int i = 0; i < NUM_UI_ELEMENTS; i++ )
        SAFE_DELETE( ui_elements[i] );

    return TRUE;
}




//-----------------------------------------------------------------------------
// name: render()
// desc: ...
//-----------------------------------------------------------------------------
t_TAPUINT UIControl::render( void * data )
{
    t_TAPUINT i;
    t_TAPFLOAT start = -.8;
    t_TAPFLOAT inc = 2 * fabs( start ) / 7;

    // buses
    for( i = SL_BUS_0; i <= SL_BUS_7; i++ )
    {
        if( load_sliders ) ui_elements[i]->set_slide( AudioCentral::instance()->bus(i-SL_BUS_0)->get_gain() );
        draw_slider( *ui_elements[i], start + (i-SL_BUS_0)*inc, 0.15f, 0.0f );
    }

    // reverb
    for( i = SL_REV_0; i <= SL_REV_7; i++ )
    {
        if( load_sliders ) ui_elements[i]->set_slide( AudioCentral::instance()->bus(i-SL_REV_0)->reverb()->getmix() );
        draw_slider( *ui_elements[i], start + (i-SL_REV_0)*inc, -0.6f, 0.0f );
    }

	// faces / ctrl
	draw_face_buttons( BT_ANALYSIS_FACE, BT_SEARCH_FACE );
	draw_ctrl_buttons( BT_QUIT, BT_FULLSCREEN );

    // load once
    if( load_sliders ) load_sliders = false;

    return 0;
}




//-----------------------------------------------------------------------------
// name: render_pre()
// desc: ...
//-----------------------------------------------------------------------------
void UIControl::render_pre()
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
void UIControl::render_post()
{
    glPopAttrib();

    AudicleFace::render_post();
}




//-----------------------------------------------------------------------------
// name: render_view()
// desc: ...
//-----------------------------------------------------------------------------
void UIControl::render_view( )
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
    float x = AudicleWindow::main()->m_hsize / AudicleWindow::main()->m_vsize;
    glOrtho( -x, x, -1.0, 1.0, -10, 10 );
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
t_TAPUINT UIControl::on_activate()
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
t_TAPUINT UIControl::on_deactivate( t_TAPDUR dur )
{
    return AudicleFace::on_deactivate( dur );
}




//-----------------------------------------------------------------------------
// name: on_event()
// desc: ...
//-----------------------------------------------------------------------------
t_TAPUINT UIControl::on_event( const AudicleEvent & event )
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

                        // bus
                        if( i >= SL_BUS_0 && i <= SL_BUS_7 )
                        {
                            if( ui_elements[i]->down )
                            {
                               ui_elements[i]->slide_last = ( vr_to_world_y(default_vr,ie->pos[1])-.15 ) / g_slider_height;
                            }
                        }

                        // reverb
                        if( i >= SL_REV_0 && i <= SL_REV_7 )
                        {
                            if( ui_elements[i]->down )
                            {
                               ui_elements[i]->slide_last = ( vr_to_world_y(default_vr,ie->pos[1])+.6 ) / g_slider_height;
                            }
                    
                        }
                    }

                    if( ie->state == ae_input_UP && ui_elements[i]->down == TRUE )
                    {
						// switch face
						if( i >= BT_ANALYSIS_FACE && i <= BT_SEARCH_FACE )
						{
							handle_face_button( BT_ANALYSIS_FACE, BT_SEARCH_FACE, i );
						}
						// ctrl
						if( i >= BT_QUIT && i <= BT_FULLSCREEN )
						{
							handle_ctrl_button( BT_QUIT, BT_FULLSCREEN, i );
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
            for( i = SL_BUS_0; i <= SL_BUS_7; i++ )
            {
                if( ui_elements[i]->down )
                {
                    fix_slider_motion( *ui_elements[i], m_vr, ie->pos, .15f, g_slider_height, true );
                    // set the gain
                    m_audio->bus( i - SL_BUS_0 )->set_gain( ui_elements[i]->fvalue() );
                }
            }

            for( i = SL_REV_0; i <= SL_REV_7; i++ )
            {
                if( ui_elements[i]->down )
                {
                    fix_slider_motion( *ui_elements[i], m_vr, ie->pos, -.6f, g_slider_height, true );
                    // set the gain
                    m_audio->bus( i - SL_REV_0 )->reverb()->mix( ui_elements[i]->fvalue() );
                }
            }        
        }
    }

    return AudicleFace::on_event( event );
}
