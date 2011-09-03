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
  #include <OpenGL/gl.h>				    // Header File For The OpenGL32 Library
  #include <OpenGL/glu.h>					// Header File For The GLu32 Library
  #include <GLUT/glut.h>
#else
  #ifdef __PLATFORM_WIN32__
  #include <windows.h>
  #endif
  #include <GL/gl.h>						// Header File For The OpenGL32 Library
  #include <GL/glu.h>						// Header File For The GLu32 Library
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
#define KEY_F1			1
#define KEY_F2			2
#define KEY_F3			3
#define KEY_F4			4
#define KEY_F5			5
#define KEY_F6			6
#define KEY_F7			7
#define KEY_F8			8
#define KEY_F9			9
#define KEY_F10			10
#define KEY_F11			11
#define KEY_F12			12

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
    static t_CKBOOL init( );
    // loop
    static t_CKBOOL loop( );
    // shutdown
    static t_CKBOOL shutdown( );

public:
    // init
    static t_CKBOOL init_on;
    // is cursor on
    static t_CKBOOL cursor_on;
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
    t_CKBOOL init( t_CKUINT w, t_CKUINT h, t_CKINT xpos, t_CKINT ypos,
                   const char * name, t_CKBOOL fullscreen );
    t_CKBOOL destroy( );

public:
    void set_mouse_coords( int x, int y );
    void set_projection( );
    void pix_to_ndc( int x, int y );
    double get_current_time( t_CKBOOL fresh = FALSE );
    t_CKBOOL check_stamp( t_CKUINT * stamp );

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
    static t_CKBOOL our_fullscreen;

public:
    GLint       m_windowID;
    GLuint      m_pick_buffer[AG_PICK_BUFFER_SIZE];
    GLuint *    m_pick_top;
    t_CKUINT    m_pick_size;
    GLint       m_cur_vp[4];
    Point2D     m_cur_pt;
    t_CKUINT    m_frame_stamp;

    int m_mousex;
    int m_mousey;
    int m_w;
    int m_h;
    t_CKFLOAT m_hsize;
    t_CKFLOAT m_vsize;
    t_CKUINT m_render_mode;
    t_CKBOOL m_antialiased;
    Color4D m_bg;
    t_CKFLOAT m_bg_alpha;

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


struct freeID { t_CKUINT id; struct freeID * next; freeID() { id=0; next=NULL; } };

class IDManager { 

protected:

    static IDManager *  m_inst;
    t_CKBOOL*           m_sids;
    t_CKINT             m_sidnum;
    t_CKUINT            m_pickID;
    freeID *            m_free;
    IDManager();

public:
    
    t_CKUINT            getStencilID();          
    void                freeStencilID(t_CKUINT i);   
    t_CKUINT            getPickID();
    void                freePickID(t_CKUINT id);
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
                      t_CKBOOL l_ = TRUE, t_CKBOOL r_ = TRUE )
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

    t_CKBOOL m_lu, m_ru;
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
