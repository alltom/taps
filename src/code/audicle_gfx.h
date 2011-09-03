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
// name: audicle_gfx.h
// desc: audicle graphics
//
// authors: Ge Wang (gewang@cs.princeton.edu)
//          Perry R. Cook (prc@cs.princeton.edu)
//          Philip Davidson (philipd@cs.princeton.edu)
//          Ananya Misra (amisra@cs.princeton.edu)
// date: Autumn 2004
//-----------------------------------------------------------------------------
#ifndef __AUDICLE_GFX_H__
#define __AUDICLE_GFX_H__

#include "audicle_def.h"
#include "audicle_geometry.h"

#include <string>

#ifdef __MACOSX_CORE__
  #include <OpenGL/gl.h>                    // Header File For The OpenGL32 Library
  #include <OpenGL/glu.h>                   // Header File For The GLu32 Library
  #include <GLUT/glut.h>
#else
  #ifdef __PLATFORM_WIN32__
  #include <windows.h>
  #endif
  #include <GL/gl.h>                        // Header File For The OpenGL32 Library
  #include <GL/glu.h>                       // Header File For The GLu32 Library
  #include <GL/glut.h>
#endif
// event.mods & ___
#define ae_input_ALT       GLUT_ACTIVE_ALT
#define ae_input_CTRL      GLUT_ACTIVE_CTRL
#define ae_input_SHIFT     GLUT_ACTIVE_SHIFT
// event.state
#define ae_input_DOWN      GLUT_DOWN    
#define ae_input_UP        GLUT_UP
// event.button
#define ae_input_LEFT_BUTTON   GLUT_LEFT_BUTTON
#define ae_input_MIDDLE_BUTTON  GLUT_MIDDLE_BUTTON
#define ae_input_RIGHT_BUTTON  GLUT_RIGHT_BUTTON


// key value definitions
#define KEY_F1          1
#define KEY_F2          2
#define KEY_F3          3
#define KEY_F4          4
#define KEY_F5          5
#define KEY_F6          6
#define KEY_F7          7
#define KEY_F8          8
#define KEY_F9          9
#define KEY_F10         10
#define KEY_F11         11
#define KEY_F12         12

#define KEY_SPACE    ' '
#define KEY_BQ       '`'
#define KEY_ESCAPE   '\033'
#define KEY_RETURN   '\13'
#define KEY_TAB      '\t'

#define KEY_UPARROW     101
#define KEY_DOWNARROW   103
#define KEY_LEFTARROW   100
#define KEY_RIGHTARROW  102

#define KEY_PGUP     104
#define KEY_PGDN     105
#define KEY_HOME     106
#define KEY_END      107
#define KEY_INS      108

#define KEY_CTRL_A     1
#define KEY_CTRL_B     2
#define KEY_CTRL_C     3
#define KEY_CTRL_D     4
#define KEY_CTRL_E     5
#define KEY_CTRL_F     6
#define KEY_CTRL_G     7
#define KEY_CTRL_H     8
#define KEY_CTRL_I     9
#define KEY_CTRL_J     10
#define KEY_CTRL_K     11
#define KEY_CTRL_L     12
#define KEY_CTRL_M     13
#define KEY_CTRL_N     14
#define KEY_CTRL_O     15
#define KEY_CTRL_P     16
#define KEY_CTRL_Q     17
#define KEY_CTRL_R     18
#define KEY_CTRL_S     19
#define KEY_CTRL_T     20
#define KEY_CTRL_U     21
#define KEY_CTRL_V     22
#define KEY_CTRL_W     23
#define KEY_CTRL_X     24
#define KEY_CTRL_Y     25
#define KEY_CTRL_Z     26


// drawing convenience classes

// gl string shortcuts
void drawString ( const std::string & str );
void drawString_mono ( const std::string & str ) ;
double drawString_length ( const std::string & str ) ;
double drawString_length_mono ( const std::string & str ) ;
void drawString_centered ( const std::string & str ) ;
void drawString_centered_mono ( const std::string & str ) ;
void scaleFont ( double h, double aspect = 1.0 );


class AudicleFace;


//-----------------------------------------------------------------------------
// name: class AudicleGfx
// desc: ...
//-----------------------------------------------------------------------------
class AudicleGfx
{
public:
    // gfx sub system
    static t_TAPBOOL init( );
    // loop
    static t_TAPBOOL loop( );
    // shutdown
    static t_TAPBOOL shutdown( );

public:
    // init
    static t_TAPBOOL init_on;
    // is cursor on
    static t_TAPBOOL cursor_on;
};




#define AG_PICK_BUFFER_SIZE    512
#define AG_PICK_TOLERANCE      4
//-----------------------------------------------------------------------------
// name: class AudicleWindow
// desc: ...
//-----------------------------------------------------------------------------
class ConsoleWindow;

class AudicleWindow
{
public:
    AudicleWindow();
    ~AudicleWindow();
    
public:
    t_TAPBOOL init( t_TAPUINT w, t_TAPUINT h, t_TAPINT xpos, t_TAPINT ypos,
                   const char * name, t_TAPBOOL fullscreen );
    t_TAPBOOL destroy( );

public:
    void set_mouse_coords( int x, int y );
    void set_projection( );
    void pix_to_ndc( int x, int y );
    double get_current_time( t_TAPBOOL fresh = FALSE );
    t_TAPBOOL check_stamp( t_TAPUINT * stamp );

public:
    void main_reshape( int w, int h );
    void main_draw( );
    void main_pick( );
    void main_mouse( int button, int state, int x, int y );
    void main_motion( int x, int y );
    void main_depressed_motion( int x, int y );
    void main_keyboard( unsigned char c, int x, int y );
    void main_special_keys( int key, int x, int y );

public:
    void ui_render_console();

public:
    static AudicleWindow * main();
    static AudicleWindow * our_main;
    static t_TAPBOOL our_fullscreen;

public:
    GLint       m_windowID;
    GLuint      m_pick_buffer[AG_PICK_BUFFER_SIZE];
    GLuint *    m_pick_top;
    t_TAPUINT    m_pick_size;
    GLint       m_cur_vp[4];
    Point2D     m_cur_pt;
    t_TAPUINT    m_frame_stamp;

    int m_mousex;
    int m_mousey;
    int m_w;
    int m_h;
    t_TAPFLOAT m_hsize;
    t_TAPFLOAT m_vsize;
    t_TAPUINT m_render_mode;
    t_TAPBOOL m_antialiased;
    Color4D m_bg;
    t_TAPFLOAT m_bg_alpha;

public:
    AudicleFace * m_curr_face;
    AudicleFace * m_last_face;
    ConsoleWindow * m_console;
};

enum draggable_type {   ae_drag_None =0, ae_drag_Unknown, ae_drag_DisplayWindow, \
                        ae_drag_ShredInstance, ae_drag_CodeRevision, \
                        ae_drag_CodeWindow } ;

enum draggable_action { ae_drag_is_Empty =0, ae_drag_is_Picking, \
                        ae_drag_is_Holding, ae_drag_is_Dropping,} ; 

class DragManager {

protected:

    static DragManager * m_instance; 
    draggable_type       m_type;
    void*                m_object;
    draggable_action     m_mode;

public:

    DragManager(); 
    static DragManager * instance();
    draggable_type type()               { return m_type;     }
    void * object()                     { return m_object;   }
    void setobject( void * t,  draggable_type r)
    { m_object = t; m_type = r; m_mode = ( t ) ? ae_drag_is_Holding : ae_drag_is_Empty; } 
    draggable_action mode()             { return m_mode;     }
    void setmode( draggable_action d)   { m_mode = d;        }   
};  


struct freeID { t_TAPUINT id; struct freeID * next; freeID() { id=0; next=NULL; } };

class IDManager { 

protected:

    static IDManager *  m_inst;
    t_TAPBOOL*           m_sids;
    t_TAPINT             m_sidnum;
    t_TAPUINT            m_pickID;
    freeID *            m_free;
    IDManager();

public:
    
    t_TAPUINT            getStencilID();          
    void                freeStencilID(t_TAPUINT i);   
    t_TAPUINT            getPickID();
    void                freePickID(t_TAPUINT id);
    static              IDManager * instance();

};

//-----------------------------------------------------------------------------
// name: struct ViewRegion
// desc: for zooming on faces
//-----------------------------------------------------------------------------
struct ViewRegion
{
public:
    ~ViewRegion() { }

    virtual float left() = 0;
    virtual float right() = 0;
    virtual float down() = 0;
    virtual float up() = 0;
};

struct ViewRegionManual : public ViewRegion
{
public:
    ViewRegionManual( float l = 0, float r = 0, float d = 0, float u = 0,
                      t_TAPBOOL l_ = TRUE, t_TAPBOOL r_ = TRUE )
        : m_left(l), m_right(r), m_down(d), m_up(u), m_lu(l_), m_ru(r_) 
    { }

    ViewRegionManual( ViewRegion & vr )
    {
        m_left = vr.left();
        m_right = vr.right();
        m_down = vr.down();
        m_up = vr.up();
        m_lu = TRUE;
        m_ru = TRUE;
    }

    virtual float left()  { return m_lu ? m_left : 
        -AudicleWindow::main()->m_hsize / AudicleWindow::main()->m_vsize; }
    virtual float right() { return m_ru ? m_right :
         AudicleWindow::main()->m_hsize / AudicleWindow::main()->m_vsize; }
    virtual float down()  { return m_down; }
    virtual float up()    { return m_up; }

public:
    float m_left;
    float m_right;
    float m_down;
    float m_up;

    t_TAPBOOL m_lu, m_ru;
};

// transition time in seconds
#define TAPS_VR_DUR .8
struct ViewRegionInterp : public ViewRegion
{
public:
    ViewRegionInterp( ViewRegion & vr )
    { init( vr ); }

    void init( ViewRegion & vr )
    { m_curr = vr; m_src = &vr; m_dest = &vr; m_alpha = 1.0, m_dur = TAPS_VR_DUR; }

    void setDest( ViewRegion & vr );
    void recalc(); // when view changes
    void next(); // take a step toward destination

    virtual float left();
    virtual float right();
    virtual float down();
    virtual float up();

    ViewRegion * m_dest;

protected:
    ViewRegionManual m_curr;
    ViewRegion * m_src;
    double m_alpha;
    double m_dur;
    double m_start;
};

float vr_to_world_x( ViewRegion & vr, float x );
float vr_to_world_y( ViewRegion & vr, float y );
Point2D vr_to_world( ViewRegion & thevr, const Point2D & vr );


#endif
