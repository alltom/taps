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
// name: audicle_face.cpp
// desc: interface for audicle face
//
// authors: Ge Wang (gewang@cs.princeton.edu)
//          Perry R. Cook (prc@cs.princeton.edu)
//          Philip Davidson (philipd@cs.princeton.edu)
//          Ananya Misra (amisra@cs.princeton.edu)
// date: 2/16/2004
//-----------------------------------------------------------------------------
#include "audicle_face.h"
#include "audicle_gfx.h"
#include "audicle.h"
#include "ui_element.h"
using namespace std;


ViewRegionManual AudicleFace::default_vr( 0, 0, -1.0f, 1.0f, FALSE, FALSE );


//-----------------------------------------------------------------------------
// name: AudicleFace()
// desc: ...
//-----------------------------------------------------------------------------
AudicleFace::AudicleFace( )
    : m_vr( default_vr )
{
    if( !this->init( ) )
    {
        fprintf( stderr, "[audicle]: cannot start face...\n" );
        return;
    }
}




//-----------------------------------------------------------------------------
// name: ~AudicleFace()
// desc: ...
//-----------------------------------------------------------------------------
AudicleFace::~AudicleFace( )
{
    this->destroy( );
}




//-----------------------------------------------------------------------------
// name: init()
// desc: ...
//-----------------------------------------------------------------------------
t_TAPBOOL AudicleFace::init( )
{
    m_id = Audicle::NO_FACE;
    m_state = INACTIVE;

    // light 0 position
    t_TAPSINGLE light0_pos[4] = { 2.0f, 1.2f, 4.0f, 1.0f };

    // light 1 parameters
    t_TAPSINGLE light1_ambient[] = { .2f, .2f, .2f, 1.0f };
    t_TAPSINGLE light1_diffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    t_TAPSINGLE light1_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    t_TAPSINGLE light1_pos[4] = { -2.0f, 0.0f, -4.0f, 1.0f };

    // copy
    memcpy( m_light0_pos, light0_pos, sizeof(m_light0_pos) );
    memcpy( m_light1_ambient, light1_ambient, sizeof(m_light1_ambient) );
    memcpy( m_light1_diffuse, light1_diffuse, sizeof(m_light1_diffuse) );
    memcpy( m_light1_specular, light1_specular, sizeof(m_light1_specular) );
    memcpy( m_light1_pos, light1_pos, sizeof(m_light1_pos) );

    m_angle_y = 0.0f;
    m_eye_y = 0.0f;
    m_bg_alpha = 0.0;
    m_bg_speed = .075;
    sleep_time = 0; 
    m_first = true;
	m_face_button_init = false;
	m_ctrl_button_init = false;

    return TRUE;
}




//-----------------------------------------------------------------------------
// name: destroy()
// desc: ...
//-----------------------------------------------------------------------------
t_TAPBOOL AudicleFace::destroy( )
{
    this->on_deactivate( 0.0 );
    m_id = Audicle::NO_FACE;
    m_state = INACTIVE;

    return TRUE;
}


//-----------------------------------------------------------------------------
// name: render_pre()
// desc: ...
//-----------------------------------------------------------------------------
void AudicleFace::render_pre( ) 
{
    glMatrixMode ( GL_PROJECTION ) ;
    glPushMatrix();
    glMatrixMode ( GL_MODELVIEW );
    glPushMatrix();
    
    xfade_bg();
}




//-----------------------------------------------------------------------------
// name: render()
// desc: ...
//-----------------------------------------------------------------------------
t_TAPUINT AudicleFace::render( void * data )
{
    fprintf( stderr, "[audicle]: face '%i' ('%s') rendering...\n", m_id,
             m_name.c_str() );
	if( AudicleWindow::our_fullscreen )
    {
        glutReshapeWindow( AudicleWindow::main()->m_w, AudicleWindow::main()->m_h );
        glutPostRedisplay();
        AudicleWindow::our_fullscreen = FALSE;
	}
    return TRUE;
}




//-----------------------------------------------------------------------------
// name: render_post()
// desc: ...
//-----------------------------------------------------------------------------
void AudicleFace::render_post()
{
    glMatrixMode ( GL_PROJECTION ) ;
    glPopMatrix();
    glMatrixMode ( GL_MODELVIEW );
    glPopMatrix();
}



//-----------------------------------------------------------------------------
// name: on_activate()
// desc: ...
//-----------------------------------------------------------------------------
t_TAPUINT AudicleFace::on_activate()
{
    switch( m_state )
    {
        case INACTIVE:
        case DEACTIVATING:
        {
            //fprintf( stderr, "[audicle]: face '%i' ('%s') activating...\n", m_id,
            //         m_name.c_str() );
            m_state = ACTIVATING;
            AudicleWindow::main()->m_bg_alpha = 1.0 - AudicleWindow::main()->m_bg_alpha;
            m_bg2 = AudicleWindow::main()->m_bg;
            m_bg_speed = -1.0;
            break;
        }

        case ACTIVE:
        {
            fprintf( stderr, "[audicle]: face '%i' ('%s') already active...\n",
                     m_id, m_name.c_str() );
            return FALSE;
        }

        case ACTIVATING:
        {
            fprintf( stderr, "[audicle]: face '%i' ('%s') already activating...\n",
                     m_id, m_name.c_str() );
            return FALSE;
        }

        default:
            fprintf( stderr, "[audicle]: face '%i' ('%s') in illegal state '%i'...\n",
                     m_id, m_name.c_str(), m_state );
            assert( FALSE );
    }

    return TRUE;
}




//-----------------------------------------------------------------------------
// name: on_deactivate()
// desc: ...
//-----------------------------------------------------------------------------
t_TAPUINT AudicleFace::on_deactivate( t_TAPDUR dur )
{
    // gotta deactivate in <= dur (ChucK time)

    switch( m_state )
    {
        case ACTIVE:
        case ACTIVATING:
        {
            //fprintf( stderr, "[audicle]: face '%i' ('%s') deactivating...\n",
            //         m_id, m_name.c_str() );
            m_state = DEACTIVATING;
            break;
        }

        case INACTIVE:
        {
            fprintf( stderr, "[audicle]: face '%i' ('%s') already inactive...\n",
                     m_id, m_name.c_str() );
            return FALSE;
        }

        case DEACTIVATING:
        {
            fprintf( stderr, "[audicle]: face '%i' ('%s') already deactivating...\n",
                     m_id, m_name.c_str() );
            return FALSE;
        }

        default:
        {
            fprintf( stderr, "[audicle]: face '%i' ('%s') in illegal state '%i'...\n",
                     m_id, m_name.c_str() );
            assert( FALSE );
        }
    }

    return TRUE;
}




//-----------------------------------------------------------------------------
// name: on_event()
// desc: ...
//-----------------------------------------------------------------------------
t_TAPUINT AudicleFace::on_event( const AudicleEvent & event )
{
/*
    fprintf( stderr, "[audicle]: input event received...\n" );
    fprintf( stderr, "     type: %i\n", event.type );
    fprintf( stderr, "     mesg: %i\n", event.message );
    fprintf( stderr, "     arg1: %i\n", event.param1 );
    fprintf( stderr, "     arg2: %i\n", event.param2 );
    fprintf( stderr, "     arg3: %i\n", event.param3 );
    fprintf( stderr, "     time: %.4f sec\n", event.timestamp / Digitalio::sampling_rate() );
    fprintf( stderr, "     data: %i\n", event.data );
*/
    return TRUE;
}




//-----------------------------------------------------------------------------
// name: xfade_bg()
// desc: ...
//-----------------------------------------------------------------------------
void AudicleFace::xfade_bg( )
{
    m_bg_alpha = AudicleWindow::main()->m_bg_alpha;
    if( m_bg_alpha > 1.0 )
    {
        AudicleWindow::main()->m_bg_alpha = 1.0;
        glClearColor( m_bg[0], m_bg[1], m_bg[2], m_bg[3] );
        return;
    }

    Color4D c = m_bg2 * ( 1.0 - m_bg_alpha );
    c += m_bg * m_bg_alpha;
    glClearColor( c[0], c[1], c[2], c[3] );

    if( m_bg_speed < 0.0 ) m_bg_speed = AudicleWindow::main()->get_current_time();
    t_TAPFLOAT delta = AudicleWindow::main()->get_current_time() - m_bg_speed;
    AudicleWindow::main()->m_bg = c;
    AudicleWindow::main()->m_bg_alpha += 3.0 * delta;
    m_bg_speed = AudicleWindow::main()->get_current_time();
}


//-----------------------------------------------------------------------------
// name: ndc_to_world_x()
// desc: convert ndc to world coordinates (see render_view)
//-----------------------------------------------------------------------------
double AudicleFace::ndc_to_world_x( double x )
{
    // m_asp = AudicleWindow::main()->m_hsize / AudicleWindow::main()->m_vsize
    // return x * m_asp / hsize
    return x * 1.0 / AudicleWindow::main()->m_vsize;
}

//-----------------------------------------------------------------------------
// name: ndc_to_world_y()
// desc: convert ndc to world coordinates (see render_view)
//-----------------------------------------------------------------------------
double AudicleFace::ndc_to_world_y( double y )
{
    return y * 1.0 / AudicleWindow::main()->m_vsize;
}

//-----------------------------------------------------------------------------
// name: ndc_to_world_y()
// desc: convert ndc to world coordinates (see render_view)
//-----------------------------------------------------------------------------
Point2D AudicleFace::ndc_to_world( const Point2D & pt )
{
    return Point2D( ndc_to_world_x( pt[0] ), ndc_to_world_y( pt[1] ) ); 
}


// current faces
enum {UIANALYSIS, UISYNTHESIS, UIGROUP, UICONTROL, UISEARCH, ELCIDUA1, NUMFACES};


//-----------------------------------------------------------------------------
// name: draw_face_buttons()
// desc: draw face switching controls
//-----------------------------------------------------------------------------
void AudicleFace::draw_face_buttons( int button_from, int button_to ) 
{
	if( !m_face_button_init )
	{
		// resize
		for( int b = button_from; b <= button_to; b++ )
		{
			ui_elements[b]->size_up = 0.022f;
			ui_elements[b]->font_size = 0.5f;
		}
		m_face_button_init = true;
	}
	
	// draw
	float left = 0.15f; // 0.4
	draw_label( "SWITCH FACE ", left, 0.94f, 0.0, 0.7, false, NULL );
	for( int b = button_from; b <= button_to; b++ )
	{
		float R = 0.75f, G = 0.75f, B = 0.75f;
		AudicleFace * which_face = Audicle::instance()->face( b - button_from );
		
		if( which_face == this )
			R = G = B = 0.5f;
		
		draw_button( *ui_elements[b], left + 0.25 + .12f*(b-button_from), 0.97f, 0.0f, R, G, B );
	}
}


//-----------------------------------------------------------------------------
// name: handle_face_button()
// desc: switch faces
//-----------------------------------------------------------------------------
void AudicleFace::handle_face_button( int button_from, int button_to, int button_hit )
{
	// check bounds
	assert( button_hit - button_from >= UIANALYSIS );
	assert( button_hit - button_from <  NUMFACES );

	// reset button
	ui_elements[button_hit]->down = FALSE;

	// switch face
	Audicle::instance()->move_to( button_hit - button_from );
}


//-----------------------------------------------------------------------------
// name: draw_ctrl_buttons()
// desc: draw control buttons (quit, fullscreen, ...)
//-----------------------------------------------------------------------------
void AudicleFace::draw_ctrl_buttons( int button_quit, int button_fullscreen ) 
{
	if( !m_ctrl_button_init )
	{
		// resize
		ui_elements[button_quit]->size_up = 0.022f;
		ui_elements[button_quit]->font_size = 0.5f;
		ui_elements[button_fullscreen]->size_up = 0.022f;
		ui_elements[button_fullscreen]->font_size = 0.5f;
		m_ctrl_button_init = true;
	}
	
	// draw
	float left = 1.03f;
	draw_button( *ui_elements[button_fullscreen], left, 0.97f, 0.0f, 0.25, 0.75, 0.25, IMG_PLUS );
	draw_button( *ui_elements[button_quit], left + .1, 0.97f, 0.0f, 0.75, 0.25, 0.25, IMG_REV );
}


//-----------------------------------------------------------------------------
// name: handle_ctrl_button()
// desc: handle control
//-----------------------------------------------------------------------------
void AudicleFace::handle_ctrl_button( int button_quit, int button_fullscreen, int button_hit )
{
	static int w = 1024, h = 768;
	if( button_hit == button_quit ) {
		fprintf(stderr, "[AudicleFace] Quitting TAPESTREA\n");
		exit(0);
	}
	else if( button_hit == button_fullscreen ) {
		fprintf(stderr, "[AudicleFace] Toggling fullscreen, if applicable\n");
		AudicleWindow::our_fullscreen = !AudicleWindow::our_fullscreen;
		if( AudicleWindow::our_fullscreen )
		{
			w = AudicleWindow::main()->m_w;
			h = AudicleWindow::main()->m_h;
			glutFullScreen();
		}
		else
			glutReshapeWindow( w, h );
	}
}
