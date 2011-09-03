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
// name: audicle_gfx.cpp
// desc: audicle graphics
//
// authors: Ge Wang (gewang@cs.princeton.edu)
//          Perry R. Cook (prc@cs.princeton.edu)
//          Philip Davidson (philipd@cs.princeton.edu)
//          Ananya Misra (amisra@cs.princeton.edu)
// date: Autumn 2004
//-----------------------------------------------------------------------------
#include "ui_audio.h"
#include "audicle.h"
#include "audicle_gfx.h"
#include <memory.h>
//#ifdef __PLATFORM_LINUX__
#ifndef __PLATFORM_WIN32__
#include <sys/time.h>
#endif

//#define _USE_FTGL_FONTS_

#ifdef _USE_FTGL_FONTS_
#include "FTGLOutlineFont.h"
#include "FTGLTextureFont.h"
#include "FTGLPolygonFont.h"
#endif

//#define AUDICLE_DEBUG_PICK // temp

// global data
t_TAPBOOL AudicleGfx::init_on = FALSE;
t_TAPBOOL AudicleGfx::cursor_on = TRUE;

AudicleWindow * AudicleWindow::our_main = NULL;
t_TAPBOOL AudicleWindow::our_fullscreen = FALSE;


// callback
void g_main_draw( );
void g_main_reshape( int x, int y );
void g_main_keyboard( unsigned char c, int x, int y );
void g_main_special_keys( int c, int x, int y );
void g_main_mouse( int button, int state, int x, int y );
void g_main_depressed_motion( int x, int y );
void g_main_motion( int x, int y );
void g_main_idle( );
void g_main_die( );
void g_main_check_gl_err ();


#ifdef _USE_FTGL_FONTS_
char kernedfontfile[] = "C:\\WINDOWS\\FONTS\\arial.ttf";
char monospacedfontfile[] = "C:\\WINDOWS\\FONTS\\ariblk.ttf";
FTFont * kernedFont = NULL;
FTFont * monospacedFont = NULL;
#endif

void setupFonts() { 
#ifdef _USE_FTGL_FONTS_
    if ( !kernedFont  ) { 
        glEnable(GL_TEXTURE_2D);
        kernedFont = new FTGLTextureFont( kernedfontfile );
        if ( kernedFont->Error() )
            fprintf(stderr, "font open error!\n");
        kernedFont->FaceSize(12);
        kernedFont->Depth(2);
        kernedFont->CharMap(ft_encoding_unicode);
        glDisable(GL_TEXTURE_2D);
    }
    if ( !monospacedFont ) { 
        glEnable(GL_TEXTURE_2D);
        monospacedFont = new FTGLTextureFont( monospacedfontfile );
        if ( monospacedFont->Error() )
            fprintf(stderr, "font open error!\n");
        monospacedFont->FaceSize(12);
        monospacedFont->Depth(2);
        monospacedFont->CharMap(ft_encoding_unicode);
        glDisable(GL_TEXTURE_2D);

    }
#endif
}

// font rendering 
void drawString( const std::string & str ) { 
#ifdef _USE_FTGL_FONTS_
    glEnable(GL_TEXTURE_2D);
  kernedFont->Render( str.c_str() );
    glDisable(GL_TEXTURE_2D);
#else
  int n = str.size();
  for ( int i = 0; i < n ; i++ ) { 
    glutStrokeCharacter(GLUT_STROKE_ROMAN, str[i] );
  }
#endif
}

void drawString_mono( const std::string & str ) { 
#ifdef _USE_FTGL_FONTS_
    glEnable(GL_TEXTURE_2D);
    monospacedFont->Render( str.c_str() );
    glDisable(GL_TEXTURE_2D);
#else
    int n = str.size();
    for ( int i = 0; i < n ; i++ ) {
        glutStrokeCharacter(GLUT_STROKE_MONO_ROMAN, str[i] );
    } 
#endif
} 

double drawString_length ( const std::string & str ) { 
    //this underestimates...
#ifdef _USE_FTGL_FONTS_
    float x1, y1, z1, x2, y2, z2;
    kernedFont->BBox( str.c_str() , x1, y1, z1, x2, y2, z2);
    return 10 * ( x2 - x1 );
#else
    return glutStrokeLength( GLUT_STROKE_ROMAN, (const unsigned char* ) str.c_str() );
#endif
}

double glut_mono_width = 104.76; 

double drawString_length_mono ( const std::string & str ) { 
    //okay, so the characters are 104.76, but glutStrokeLength returns an 
    //int, and it's not even rounded properly, it's just 104 x blah.. stupid!  
#ifdef _USE_FTGL_FONTS_
    float x1, y1, z1, x2, y2, z2;
    monospacedFont->BBox( str.c_str() , x1, y1, z1, x2, y2, z2);
    return 10 * ( x2 - x1 );
#else
    return glut_mono_width * (double)str.size();
#endif
}

void drawString_centered ( const std::string & str ) { 
#ifdef _USE_FTGL_FONTS_
    glTranslated( -drawString_length(str)*0.5 * 0.1 , 0, 0);
#else
    glTranslated( -drawString_length(str)*0.5, 0, 0);
#endif
    drawString(str);
}

void drawString_centered_mono ( const std::string & str ) { 
    glTranslated( -drawString_length_mono(str)*0.5, 0, 0);
    drawString_mono(str);
}

void scaleFont(double h, double aspect) { 
#ifdef _USE_FTGL_FONTS_
   glScaled( h * 0.1 * aspect, h * 0.1, 1 );
#else
   glScaled( h * .01 * aspect, h * 0.01, 1 );
#endif
}




//-----------------------------------------------------------------------------
// name: init()
// desc: ...
//-----------------------------------------------------------------------------
t_TAPBOOL AudicleGfx::init()
{
    if( init_on )
        return TRUE;

    // log
    BB_log( BB_LOG_SYSTEM, "initializing graphics engine..." );
    // push log
    BB_pushlog();

    // set cursor
    cursor_on = true;

    // log
    if( BirdBrain::frame_rate() > 0 )
        BB_log( BB_LOG_SYSTEM, "maximum frame rate: %u", BirdBrain::frame_rate() );
    else
        BB_log( BB_LOG_SYSTEM, "maximum frame rate: NO LIMIT" );
    BB_log( BB_LOG_SYSTEM, "setting cursor state: %s", cursor_on ? "ON" : "OFF" );
    
#ifdef __MACOSX_CORE__ 
    // log
    BB_log( BB_LOG_INFO, "retrieving current directory..." );
    char * cwd = getcwd( NULL, 0 );
#endif
    
    // glut
    int foo = 1; char * bar[] = { "tapestrea" };
    BB_log( BB_LOG_SYSTEM, "initializing opengl/glut..." );
    glutInit( &foo, bar);
    
#ifdef __MACOSX_CORE__
    // correct the directory..
    chdir( cwd );
    free( cwd );
#endif

    AudicleWindow::main()->get_current_time( TRUE );

    init_on = TRUE;

    // pop log
    BB_poplog();

    return TRUE;
}




//-----------------------------------------------------------------------------
// name: loop()
// desc: ...
//-----------------------------------------------------------------------------
t_TAPBOOL AudicleGfx::loop()
{
    if( !init_on )
        return FALSE;
    
    // loop
    glutMainLoop();
    
    return TRUE;
}




//-----------------------------------------------------------------------------
// name: main()
// desc: ...
//-----------------------------------------------------------------------------
AudicleWindow * AudicleWindow::main( )
{
    if( !our_main )
        our_main = new AudicleWindow;

    assert( our_main != NULL );
    return our_main;
}




//-----------------------------------------------------------------------------
// name: shutdown()
// desc: ...
//-----------------------------------------------------------------------------
t_TAPBOOL AudicleGfx::shutdown()
{
    if( !init_on )
        return TRUE;

    return TRUE;
}




//-----------------------------------------------------------------------------
// name: AudicleWindow()
// desc: ...
//-----------------------------------------------------------------------------
AudicleWindow::AudicleWindow()
{
    m_windowID = 0;
    memset( m_pick_buffer, 0, sizeof(m_pick_buffer) );
    memset( m_cur_vp, 0, sizeof(m_cur_vp) );
    m_hsize = 800;
    m_vsize = 600;
    m_w = 800;
    m_h = 600;
    m_render_mode = GL_RENDER;
    m_antialiased = false;
//    m_console = NULL;
    m_bg = Color4D( 0.0f, 0.0f, 0.0f, 1.0f );
    m_frame_stamp = 0;
    m_bg_alpha = 1.0;
}


//-----------------------------------------------------------------------------
// name: init()
// desc: ...
//-----------------------------------------------------------------------------
t_TAPBOOL AudicleWindow::init( t_TAPUINT w, t_TAPUINT h, t_TAPINT xpos, t_TAPINT ypos,
                              const char * name, t_TAPBOOL fullscreen )
{
    if( m_windowID ) return TRUE;

    // log
    BB_log( BB_LOG_SYSTEM, "initializing windowing system..." );
    // push log
    BB_pushlog();
    
    // log
    BB_log( BB_LOG_SYSTEM, "setting window size: %d x %d...", w, h );
    BB_log( BB_LOG_SYSTEM, "setting window position: %d, %d...", xpos, ypos );

    // set modes
    glutInitDisplayMode( GLUT_RGBA | GLUT_ALPHA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_STENCIL );
    glutInitWindowSize( w, h );
    glutInitWindowPosition( xpos, ypos );
    
    m_windowID = glutCreateWindow( name );
    
    // set callback functions
    glutDisplayFunc ( g_main_draw );
    glutReshapeFunc ( g_main_reshape );
    glutKeyboardFunc( g_main_keyboard );
    glutSpecialFunc ( g_main_special_keys );
    glutMouseFunc   ( g_main_mouse );
    glutMotionFunc  ( g_main_depressed_motion );
    glutPassiveMotionFunc ( g_main_motion );
    glutVisibilityFunc ( NULL );
    glutIdleFunc( g_main_idle );
    
    // log
    BB_log( BB_LOG_SYSTEM, "fullscreen mode: %s", fullscreen ? "ON" : "OFF" );
    main()->our_fullscreen = fullscreen;
    if( fullscreen )
        glutFullScreen( );

#if (GLUT_MACOSX_IMPLEMENTATION >= 2 ) 
    glutWMCloseFunc ( g_main_die );
#endif
    
//    glEnable( GL_POINT_SMOOTH );
//    glEnable( GL_LINE_SMOOTH );
//    glEnable( GL_BLEND );
//    glColorMask( true, true, true, true );
//    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    //  glHint (GL_LINE_SMOOTH_HINT, GL_DONT_CARE);

    glPolygonOffset( 1.0, 1.0 );
    glLineWidth( 1 );
    
    // glEnable( GL_POLYGON_SMOOTH );
    glEnable( GL_DEPTH_TEST );
//    glEnable( GL_NORMALIZE );
    
//    glClearDepth( 1900 );
//    glClearColor( 1.0, 1.0, 1.0, 0.f );
//    glClearStencil( 0 );
    
    glSelectBuffer( AG_PICK_BUFFER_SIZE, m_pick_buffer );

    glViewport( 0, 0, w, h );
    glGetIntegerv( GL_VIEWPORT, m_cur_vp );
    
    m_hsize = 1.00;
    m_vsize = 1.33;
    
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity( );

    //perspective?  
    glOrtho( -m_hsize, m_hsize, -m_vsize, m_vsize, -10, 10 );
    
    setupFonts();
    //this is silly as all hell
#if defined(WIN32)
    ShowCursor( TRUE );
#endif

//    m_console = new ConsoleWindow();

    // pop log
    BB_poplog();

    return ( m_windowID != 0 );
}




//-----------------------------------------------------------------------------
// name: set_mouse_coords()
// desc: ...
//-----------------------------------------------------------------------------
void AudicleWindow::set_mouse_coords( int x, int y )
{
     m_mousex = x;
     m_mousey = y;
     pix_to_ndc ( x, y );
}




//-----------------------------------------------------------------------------
// name: set_projection()
// desc: ...
//-----------------------------------------------------------------------------
void AudicleWindow::set_projection()
{
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity( );

    if ( m_render_mode == GL_SELECT )
    {
        gluPickMatrix( m_mousex, m_cur_vp[3] - m_mousey, AG_PICK_TOLERANCE, AG_PICK_TOLERANCE, m_cur_vp );
        glInitNames( );
        glPushName( 0xffffffff ); /* a strange value */
    }

    glMatrixMode( GL_MODELVIEW );
}




//-----------------------------------------------------------------------------
// name: pix_to_ndc()
// desc: ...
//-----------------------------------------------------------------------------
void AudicleWindow::pix_to_ndc( int x, int y )
{
    m_cur_pt[0] = m_hsize * 2.0 * ( (double)x / (double)m_cur_vp[2] - 0.5 );
    m_cur_pt[1] = m_vsize * 2.0 * ( (double)(m_cur_vp[3]-y) / (double)m_cur_vp[3] - 0.5 );
}


#ifdef __PLATFORM_WIN32__
#include <sys/timeb.h>
#else 

#endif 

double AudicleWindow::get_current_time( t_TAPBOOL fresh )
{
#ifdef __PLATFORM_WIN32__
    struct _timeb t;
    _ftime(&t);
    return t.time+t.millitm/1000.0;
#else
    static struct timeval t;
        if( fresh ) gettimeofday(&t,NULL);
        return t.tv_sec+(double)t.tv_usec/1000000;
#endif
        
//    fprintf(stderr, "what the time is it?\n");
    return 0;
}

t_TAPBOOL 
AudicleWindow::check_stamp ( t_TAPUINT * stamp ) { 
    assert ( stamp );
    if ( *stamp == m_frame_stamp ) return false;
    else *stamp = m_frame_stamp;
    return true;
}



//-----------------------------------------------------------------------------
// name: reshape()
// desc: ...
//-----------------------------------------------------------------------------
void AudicleWindow::main_reshape( int w, int h )
{
    assert( this == AudicleWindow::main() );
    
    glViewport( 0, 0, w, h );
    glGetIntegerv( GL_VIEWPORT, m_cur_vp );

    m_vsize = ( m_cur_vp[3] == 0 ) ? 1 : (double)m_cur_vp[3]/480.0;
    double aspect = ( m_cur_vp[3] == 0 ) ? 1 : (double)m_cur_vp[2] / (double)m_cur_vp[3]; 
    m_hsize = aspect * m_vsize;

    // project
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity( ); 
//    glOrtho( -m_hsize, m_hsize, -m_vsize, m_vsize, -10, 10 );

    // view
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity( );
    
    m_w = w;
    m_h = h;


    //XXX haaaaaaaaack!
    //we need an OSD::class instead of using that WindowManager...
    //that will take care of console and messages...
    //and can track current face to determine mesg visibility.

    //reshape
//    if( m_console ) m_console->fit_to_window ( -m_hsize, m_hsize, -m_vsize, m_vsize ) ;

}




//-----------------------------------------------------------------------------
// name: frame_wait()
// desc: ...
//-----------------------------------------------------------------------------
void frame_wait()
{
    static double last_time = 0;
    double now = AudicleWindow::main()->get_current_time( TRUE );
    double diff = now - last_time;
    double dur = 1.0 / BirdBrain::frame_rate();
    // fprintf( stderr, "'%f' '%f' '%f' '%f'\n", now, last_time, diff, dur );

    // too soon
    if( diff < dur )
    {
        // wait for next
        // fprintf( stderr, "sleeping for '%f' seconds\n", (dur - diff) );
        usleep( (t_TAPINT)(1000000 * ( dur - diff )) );
        // get updated now
        now = AudicleWindow::main()->get_current_time( TRUE );
    }

    // save now for next
    last_time = now;
}




//-----------------------------------------------------------------------------
// name: main_draw()
// desc: ...
//-----------------------------------------------------------------------------
void AudicleWindow::main_draw( )
{
    assert( this == AudicleWindow::main() );
    m_frame_stamp++;

    if( BirdBrain::frame_rate() > 0 ) frame_wait();
    else AudicleWindow::main()->get_current_time( TRUE );
    
    // clear buffers
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );

    if( m_antialiased )
    {
        glHint( GL_POINT_SMOOTH_HINT, GL_NICEST );
        glHint( GL_LINE_SMOOTH_HINT, GL_NICEST );
        glEnable( GL_POINT_SMOOTH );
        glEnable( GL_LINE_SMOOTH );
    }
    else
    {
        glDisable( GL_POINT_SMOOTH );
        glDisable( GL_LINE_SMOOTH );
    }

    // set projection ( may be picking )
    set_projection();

    AudicleFace * face = Audicle::instance()->face();

    // push state
    face->render_pre();

    // set view matrices 
    face->render_view();     

    // do yer buzinezz;
    face->render( NULL );
/*
    glEnable(GL_TEXTURE_2D);
    glColor4d ( 0,0,0,1.0);
    glPushMatrix();
    scaleFont( 1.0, 1.0 );

    kernedFont->Render( "hi!" );
    monospacedFont->Render( "hello!" );

    glPopMatrix();    
    glDisable(GL_TEXTURE_2D);
*/
    // restore state
    face->render_post();

    // rendering for constant overlay...
    // we may place a windowmanager here?
    // move messages to a non-static setup....
    // for now put a console at least in here..



    // ui_render_console ();
    
    // flush
    glFlush();
    // swap
    if( m_render_mode == GL_RENDER )
        glutSwapBuffers();

#ifdef __PLATFORM_MACOSX__
    static t_TAPINT never = 0;
    
    never++;
    
    if( never > 1 )
    {
        if( AudicleWindow::our_fullscreen )
        {
            glutReshapeWindow( AudicleWindow::main()->m_w, AudicleWindow::main()->m_h );
            glutPostRedisplay();
            AudicleWindow::our_fullscreen = FALSE;
        }
        never = 0;
    }
#endif
}



//-----------------------------------------------------------------------------
// name: destroy()
// desc: ...
//-----------------------------------------------------------------------------
t_TAPBOOL AudicleWindow::destroy()
{
    assert( this == AudicleWindow::main() );

    if( m_windowID ){ 
        glutDestroyWindow ( m_windowID );
    }
    
    return TRUE;
}




//-----------------------------------------------------------------------------
// name: main_pick()
// desc: ...
//-----------------------------------------------------------------------------

void AudicleWindow::main_pick() { 

    m_render_mode = GL_SELECT;
    glRenderMode( m_render_mode );

    main_draw();

    //return to normal
    m_render_mode = GL_RENDER;
    int nHit = glRenderMode( m_render_mode );

    t_TAPUINT n  = 0 ;
    m_pick_top  = NULL;
    m_pick_size = 0;
    
    // danger danger (I added these two variables; it used just m_*)
    t_TAPUINT pick_size; // (**)
    GLuint * pick_top;  // (**)
    for( int i = 0; i < nHit; i++ )
    {
// more danger
#if 1
        // danger danger
        pick_size = m_pick_buffer[n++];    // danger: used m_pick_size and m_pick_top
        t_TAPUINT zMin = m_pick_buffer[n++];
        t_TAPUINT zMax = m_pick_buffer[n++];
        pick_top = ( m_pick_buffer + n );

        if( nHit <= 1 || pick_size > 1 )   // danger: this part was not there
        {
            m_pick_size = pick_size;
            m_pick_top = pick_top;
        }
#else
        // (**) (= another option?)
        m_pick_size = m_pick_buffer[n++];  // nested danger
        t_TAPUINT zMin = m_pick_buffer[n++];
        t_TAPUINT zMax = m_pick_buffer[n++];
        m_pick_top = ( m_pick_buffer + n );
        //m_pick_size += pick_size;  // nested danger
        pick_size = m_pick_size;
#endif
     
#ifdef AUDICLE_DEBUG_PICK
        fprintf(stderr, "stack %d objs, %d %d\n", pick_size ,zMin, zMax );  // danger: m_pick_size
        for( int j = 0; j < pick_size ; j++ )
        {
         fprintf(stderr, "item %u\n", m_pick_buffer[n++]);
        }        
#else
        n += pick_size; // danger: m_pick_size (**)
#endif
    }

#ifdef AUDICLE_DEBUG_PICK
    fprintf( stderr, "\n" );
#endif
}

//-----------------------------------------------------------------------------
// name: main_mouse()
// desc: ...
//-----------------------------------------------------------------------------
void AudicleWindow::main_mouse( int button, int state, int x, int y)
{

    assert( this == AudicleWindow::main() );
    set_mouse_coords( x, y );


    //fetch pick stack
    main_pick();
    t_TAPUINT * temp_stack = (t_TAPUINT*) malloc ( m_pick_size * sizeof(t_TAPUINT) );
    memcpy ( (void*)temp_stack, (void*)m_pick_top, m_pick_size * sizeof(t_TAPUINT) );

    // create mouse input event
    InputEvent* ev = new InputEvent ( ae_input_MOUSE, m_cur_pt, button, state );
    ev->setMods( glutGetModifiers() );
    ev->setStack( temp_stack , m_pick_size );
    ev->time = get_current_time();

    InputEvent sub = *ev;

    //START AN AUDICLE EVENT 
    //make_mouse_event( m_cur_pt,  button, state );

    AudicleEvent event;
    event.type = ae_event_INPUT;
    event.message = 2;
    event.param1 = button;
    event.param2 = state;
    event.data = (void *) ev;

    Audicle::instance()->face()->on_event( event );

    sub.popStack();

//    if( m_console )
//    {
//        m_console->handleMouse( sub );
//        m_console->handleMouseAlerts( sub ); 
//    }

    free (temp_stack);
    delete ev; //XXX assumes unqueued...

    //    if ( wm ) wm->handleMouse(button, state, cur_pt);

    //    if ( DragManager::instance()->object() ) { } 

}




//-----------------------------------------------------------------------------
// name: main_motion()
// desc: ...
//-----------------------------------------------------------------------------
void AudicleWindow::main_motion( int x, int y )
{
    assert( this == AudicleWindow::main() );
    set_mouse_coords(x,y);

//XXX
#ifdef MACOSX
    if ( 0 < x && x < m_cur_vp[2] && 0 < y && y < m_cur_vp[3] ) { 
        if ( cursorOn ) { 
            HideCursor();
            cursorOn = false;
        }
    }
    else { 
        if ( !cursorOn ) { 
            ShowCursor();
            cursorOn = true;
        }
    }
#endif

    InputEvent * ev = new InputEvent ( ae_input_MOTION, m_cur_pt );
    ev->time = get_current_time();

    AudicleEvent event;
    event.type = ae_event_INPUT;
    event.message = 2;
    event.data = (void *) ev;

    Audicle::instance()->face()->on_event( event );

    delete ev; //XXX assumes unqueued...

}




//-----------------------------------------------------------------------------
// name: main_depressed_motion()
// desc: ...
//-----------------------------------------------------------------------------
void AudicleWindow::main_depressed_motion( int x, int y )
{
    assert( this == AudicleWindow::main() );



    set_mouse_coords(x,y);
//   if ( wm ) wm->handleMotion(cur_pt);

    InputEvent * ev = new InputEvent ( ae_input_MOTION, m_cur_pt );
    ev->time = get_current_time();

    AudicleEvent event;
    event.type = ae_event_INPUT;
    event.message = 2;
    event.data = (void *) ev;


    Audicle::instance()->face()->on_event( event );

    delete ev; //XXX assumes unqueued...

}




//-----------------------------------------------------------------------------
// name: main_keyboard()
// desc: ...
//-----------------------------------------------------------------------------
void AudicleWindow::main_keyboard( unsigned char c, int x, int y )
{
    assert( this == AudicleWindow::main() );
    set_mouse_coords(x,y);
    static int w = 1024, h = 768;


    InputEvent * ev = new InputEvent ( ae_input_KEY, m_cur_pt, c ); 
    ev->setMods( glutGetModifiers() );
    ev->time = get_current_time();

    AudicleEvent event;
    event.type = ae_event_INPUT;
    event.message = 2;
    event.data = (void *) ev;
    
    bool handled = false;

    if( ev->mods & ae_input_CTRL ) 
    {
        switch( c )
        {   
        case KEY_CTRL_G:
            our_fullscreen = !our_fullscreen;
            if( our_fullscreen )
            {
                w = m_w;
                h = m_h;
                glutFullScreen();
            }
            else
                glutReshapeWindow( w, h );
            handled = true;
            break;
        case KEY_CTRL_Q:
            handled = true;
            exit( 0 );
            break;
        }
    }
    else { 
        switch ( c ) { 
        case '`':
            if ( ev->mods & ae_input_ALT ) {
                m_antialiased = !m_antialiased;
            }
//            else if( m_console ) { 
//                if ( m_console->active() ) m_console->deactivate();
//                else m_console->activate();
//            }
            handled = true;
            break;
        }

    }
    
    if ( !handled ) { 

//        if ( m_console && m_console->active() && m_console->selected() ) { 
//            m_console->handleKey( *ev );
//        }
//        else { 
            Audicle::instance()->face()->on_event( event );
//        }


    }
    delete ev; //XXX assumes unqueued...


//   if ( wm ) wm->handleKey( c, cur_pt );
}




//-----------------------------------------------------------------------------
// name: main_special_keys()
// desc: ...
//-----------------------------------------------------------------------------
void AudicleWindow::main_special_keys( int key, int x, int y )
{
    assert( this == AudicleWindow::main() );
    set_mouse_coords(x,y);
    
    InputEvent * ev = new InputEvent ( ae_input_SPEC, m_cur_pt, key ); 
    ev->setMods( glutGetModifiers() );
    ev->time = get_current_time();

    AudicleEvent event;
    event.type = ae_event_INPUT;
    event.message = 2;
    event.data = (void *) ev;

    bool handled = false;
    if ( ev->mods & ae_input_CTRL ) { 
        handled = true;
        switch ( key ) { 
        case KEY_LEFTARROW:
            Audicle::instance()->move( Audicle::LEFT );
//            if( m_console ) m_console->setCubes();
            break;    
        case KEY_RIGHTARROW:
            Audicle::instance()->move( Audicle::RIGHT );
//            if( m_console ) m_console->setCubes();
            break;
        case KEY_UPARROW:
            Audicle::instance()->move( Audicle::UP );
//            if( m_console ) m_console->setCubes();
            break;
        case KEY_DOWNARROW:
            Audicle::instance()->move( Audicle::DOWN );
//            if( m_console ) m_console->setCubes();
            break;
        default:
            handled = false; 
            break;
        }
    }
    else { 
        static float volume = 1.0f;
        handled = true;
        switch ( key ) { 
        case KEY_LEFTARROW:
            break;    
        case KEY_RIGHTARROW:
            break;
        case KEY_UPARROW:
            volume *= 1.05f;
            AudioCentral::instance()->set_gain( volume );
            fprintf( stderr, "[volume]: %f\n", AudioCentral::instance()->get_gain() );
            break;
        case KEY_DOWNARROW:
            volume *= 1.0f/1.1f;
            AudioCentral::instance()->set_gain( volume );
            fprintf( stderr, "[volume]: %f\n", AudioCentral::instance()->get_gain() );
            break;
        default:
            handled = false; 
            break;
        }
    }
    
    if ( !handled ) { 
//        if ( m_console && m_console->active()  && m_console->selected() ) 
//        { 
//            m_console->handleSpec ( *ev ) ;
//        }
//        else 
//        { 
            Audicle::instance()->face()->on_event( event );
//        }
    }

    delete ev; //XXX assumes unqueued...


//   if ( wm ) wm->handleSpec( key, cur_pt);  
}



//-----------------------------
// name: ui_render_console
// desc: ...
//-----------------------------
void
AudicleWindow::ui_render_console() 
{ 
    //push Attribs

//    if ( m_console->active() ) { 

    t_TAPUINT pflag =  GL_LIGHTING_BIT | GL_COLOR_BUFFER_BIT \
        | GL_DEPTH_BUFFER_BIT ;
    
    pflag |= GL_POINT_BIT | GL_LINE_BIT; 
    
    glPushAttrib( pflag ) ;


    glColorMask(true, true, true, true);
    
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
     glEnable (GL_BLEND);

    if ( m_antialiased ) { 
        glHint( GL_POINT_SMOOTH_HINT, GL_NICEST );
        glHint( GL_LINE_SMOOTH_HINT, GL_NICEST );
        glEnable (GL_POINT_SMOOTH);
        glEnable (GL_LINE_SMOOTH);
    }
    else { 
        glDisable (GL_POINT_SMOOTH);
        glDisable (GL_LINE_SMOOTH);
    }

    glDisable (GL_DEPTH_TEST);
    
    glClearStencil(0);

    //set view
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();

    glOrtho( -m_hsize, m_hsize, -m_vsize, m_vsize, -10, 10 );

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();


    //draw!

//    if( m_console )
//    {
//        m_console->drawWindow();
//        m_console->drawAlerts();
//    }


    //pop view
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    
    //pop attrib
    
    glPopAttrib();
//    }
}


IDManager * IDManager::m_inst = NULL;


//-----------------------------------------------------------------------------
// name: IDManager()
// desc: ...
//-----------------------------------------------------------------------------

IDManager::IDManager( ) { 

    GLint bits = 8;
    glGetIntegerv( GL_STENCIL_BITS, &bits );
    m_sidnum = 1 << bits;

    m_sids = ( t_TAPBOOL *) malloc( m_sidnum * sizeof( t_TAPBOOL ) );

    memset( m_sids, 0, m_sidnum * sizeof( t_TAPBOOL ) ) ;
    m_free = NULL;
    m_pickID = m_sidnum; //pickIDs are separate from stencil ids
}





//-----------------------------------------------------------------------------
// name: instance()
// desc: ...
//-----------------------------------------------------------------------------
IDManager * 
IDManager::instance( ) { 
    if ( !m_inst ) 
    { 
        m_inst = new IDManager( );
        if ( !m_inst ) 
        { 
            fprintf( stderr,"IDManager::could not create _instance;\n");
        }
    }
    return m_inst;
}





//-----------------------------------------------------------------------------
// name: getStencilID()
// desc: ...
//-----------------------------------------------------------------------------
t_TAPUINT
IDManager::getStencilID( ) { 
    for ( int i = 1 ; i < m_sidnum ; i++ ) 
    { 
        if ( m_sids[i] == false ) 
        {
            m_sids[i] = true;
            //fprintf( stderr, "assigned stencil %d\n", i ); 
            return i;
        }
    }
    fprintf( stderr, "IDManager::getStencilID - error - ID array is full!\n");
    return 0;
}




//-----------------------------------------------------------------------------
// name: freeStenciID()
// desc: ...
//-----------------------------------------------------------------------------
void
IDManager::freeStencilID( t_TAPUINT i ) {
    if ( i > 0 && i < m_sidnum ) 
    { 
        if ( !m_sids[i] ) 
            fprintf( stderr, "IDManager::_freeID - error: freeing unbound ID %d\n", i );
        m_sids[i] = false;
    }
    else fprintf( stderr,"IDManager::freeStencilID - ID %d out of range!\n", i);    
}





//-----------------------------------------------------------------------------
// name: getPickID()
// desc: ...
//-----------------------------------------------------------------------------
t_TAPUINT 
IDManager::getPickID( ) {
    return ++m_pickID;
}


//-----
// name: freePickID ( t_TAPUINT id );
// desc: ...
//-----
void
IDManager::freePickID ( t_TAPUINT id ) { 
    freeID * nw = new freeID();
    nw->id = id;
    nw->next = m_free;
    m_free = nw;

}




DragManager * DragManager::m_instance = NULL;

DragManager::DragManager( )  
{ 
    m_type       = ae_drag_None;
    m_object     = NULL;
}

DragManager* 
DragManager::instance( ) 
{ 
    if ( !m_instance ) 
    { 
        m_instance = new DragManager( );
        if ( !m_instance ) { 
            fprintf( stderr, "[chuck] error: DragManager instance creation failed\n" );
        }   
    }
    return m_instance;
}



//-----------------------------------------------------------------------------
// name: g_main_draw()
// desc: ...
//-----------------------------------------------------------------------------
void g_main_draw( void )
{
    AudicleWindow::main()->main_draw();
    g_main_check_gl_err();
}




//-----------------------------------------------------------------------------
// name: g_main_check_gl_err()
// desc: ...
//-----------------------------------------------------------------------------
void g_main_check_gl_err()
{
    GLenum errCode;
    const GLubyte *errString;

    while( (errCode = glGetError()) != GL_NO_ERROR )
    { 
        errString = gluErrorString( errCode );
        fprintf( stderr, "[audicle]: OpenGL error: %d\t%s\n", errCode, errString ); 
    }
}




//-----------------------------------------------------------------------------
// name: g_main_idle()
// desc: ...
//-----------------------------------------------------------------------------
void  g_main_idle( )
{
    // tell glut to render
    glutPostRedisplay();
}




//-----------------------------------------------------------------------------
// name: g_main_reshape()
// desc: ...
//-----------------------------------------------------------------------------
void g_main_reshape( int width, int height )
{
    AudicleWindow::main()->main_reshape( width, height );
}




//-----------------------------------------------------------------------------
// name: g_main_mouse()
// desc: ...
//-----------------------------------------------------------------------------
void g_main_mouse( int button, int state, int x, int y )
{
    AudicleWindow::main()->main_mouse( button, state, x, y );
}




//-----------------------------------------------------------------------------
// name: g_main_motion()
// desc: ...
//-----------------------------------------------------------------------------
void g_main_motion( int x, int y)
{
    AudicleWindow::main()->main_motion( x, y );
}




//-----------------------------------------------------------------------------
// name: g_main_depressed_motion()
// desc: ...
//-----------------------------------------------------------------------------
void g_main_depressed_motion( int x, int y )
{
    AudicleWindow::main()->main_depressed_motion(x,y);
}




//-----------------------------------------------------------------------------
// name: g_main_keyboard()
// desc: ...
//-----------------------------------------------------------------------------
void g_main_keyboard( unsigned char c, int x, int y )
{
    AudicleWindow::main()->main_keyboard( c, x, y );
}




//-----------------------------------------------------------------------------
// name: g_main_special_keys()
// desc: ...
//-----------------------------------------------------------------------------
void g_main_special_keys( int key, int x, int y )
{
    AudicleWindow::main()->main_special_keys( key, x, y );
}




//-----------------------------------------------------------------------------
// name: g_main_die()
// desc: ...
//-----------------------------------------------------------------------------
void g_main_die( )
{
    fprintf( stderr, "[audicle]: exiting...\n" );
    exit( 0 );
}




//-----------------------------------------------------------------------------
// name: setDest()
// desc: set view region to zoom to
//-----------------------------------------------------------------------------
void ViewRegionInterp::setDest( ViewRegion & vr )
{
    // do nothing
    if( &vr == m_dest ) return;

    // at rest
    // if( m_alpha >= 1.0 )
    {
        // make dest src
        m_src = &m_curr;
        // copy curr
        m_curr = *m_src;
        // set dest
        m_dest = &vr;
        // reset alpha
        m_alpha = 0;
        // reset dur
        m_dur = TAPS_VR_DUR;
        // set start
        m_start = AudicleWindow::main()->get_current_time();
    }
    // else // in transition already
    // {

    // }
}

//-----------------------------------------------------------------------------
// name: next()
// desc: move step towards dest
//-----------------------------------------------------------------------------
void ViewRegionInterp::next()
{
    // do nothing
    if( m_alpha >= 1.0 ) return;

    // compute the alpha
    m_alpha = (AudicleWindow::main()->get_current_time() - m_start) / m_dur;
    // alpha
    recalc();
}

//-----------------------------------------------------------------------------
// name: recalc()
// desc: use alpha
//-----------------------------------------------------------------------------
void ViewRegionInterp::recalc()
{
    if( m_alpha > 1.0 ) m_alpha = 1.0;

    m_curr.m_left = m_src->left() + m_alpha * (m_dest->left() - m_src->left());
    m_curr.m_right = m_src->right() + m_alpha * (m_dest->right() - m_src->right());
    m_curr.m_down = m_src->down() + m_alpha * (m_dest->down() - m_src->down());
    m_curr.m_up = m_src->up() + m_alpha * (m_dest->up() - m_src->up());
}

//-----------------------------------------------------------------------------
// name: left()
// desc: ...
//-----------------------------------------------------------------------------
float ViewRegionInterp::left()
{
    if( m_alpha >= 1.0 ) return m_dest->left();
    else return m_curr.left();
}

//-----------------------------------------------------------------------------
// name: right()
// desc: ...
//-----------------------------------------------------------------------------
float ViewRegionInterp::right()
{
    if( m_alpha >= 1.0 ) return m_dest->right();
    else return m_curr.right();
}

//-----------------------------------------------------------------------------
// name: down()
// desc: ...
//-----------------------------------------------------------------------------
float ViewRegionInterp::down()
{
    if( m_alpha >= 1.0 ) return m_dest->down();
    else return m_curr.down();
}

//-----------------------------------------------------------------------------
// name: up()
// desc: ...
//-----------------------------------------------------------------------------
float ViewRegionInterp::up()
{
    if( m_alpha >= 1.0 ) return m_dest->up();
    else return m_curr.up();
}

//-----------------------------------------------------------------------------
// name: vr_to_world_x()
// desc: converts view region coordinates to world coordinates
//-----------------------------------------------------------------------------
float vr_to_world_x( ViewRegion & vr, float x )
{
    float right = AudicleWindow::main()->m_hsize;
    float left = -right;
    float diff = (vr.right() - vr.left());
    float answer = (x - left) / (right-left) * diff + vr.left();
    return answer;
}

//-----------------------------------------------------------------------------
// name: vr_to_world_y()
// desc: ...
//-----------------------------------------------------------------------------
float vr_to_world_y( ViewRegion & vr, float y )
{
    float up = AudicleWindow::main()->m_vsize;
    float down = -up;
    float diff = (vr.up() - vr.down());
    float answer = (y - down) / (up-down) * diff + vr.down();
    return answer;
}

//-----------------------------------------------------------------------------
// name: vr_to_world()
// desc: ...
//-----------------------------------------------------------------------------
Point2D vr_to_world( ViewRegion & thevr, const Point2D & vr )
{
    return Point2D( vr_to_world_x(thevr,vr[0]), vr_to_world_y(thevr,vr[1]) );
}
