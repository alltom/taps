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
// name: ui_search.cpp
// desc: birdbrain search
//
// authors: Ananya Misra (amisra@cs.princeton.edu)
//          Ge Wang (gewang@cs.princeton.edu)
//          Matt Hoffman (mdhoffma@cs.princeton.edu)
//          Perry R. Cook (prc@cs.princeton.edu)
// EST. April 20, 2006
//-----------------------------------------------------------------------------
#include "ui_search.h"
#include "taps_sceptre.h"
#include "audicle.h"
#include "audicle_utils.h"
#include "audicle_geometry.h"
#include "taps_regioncomparer.h"
#include "util_readwrite.h"

// UISearch * g_search_face = NULL;

// enumeration for ui elements
enum UI_ELMENTS
{
    BT_LISTEN = 0,
    BT_STOP,
    BT_FIND,
    BT_ADD, // add to library
    BT_LOAD_DB, // load feature database
    BT_ADD_DB, // add template to database
    BT_TOGGLE_VIEW, // toggle matches view
    BT_CENTER, // center matches view on selected
	// face switchers
	BT_ANALYSIS_FACE,
	BT_SYNTHESIS_FACE,
	BT_GROUP_FACE,
	BT_CONTROL_FACE,
	BT_SEARCH_FACE,
	// ctrl
	BT_QUIT,
	BT_FULLSCREEN,
    
    SL_ROTATE_H, // rotate around y-axis
    SL_ZOOM,     // zoom in?
    SL_ROTATE_V, // rotate around x-axis
    SL_NUM_MATCHES, // how many matches to show in 3d view
    
    SL_VAL_A,
    SL_VAL_B,
    SL_VAL_C,
    SL_VAL_D,
    SL_VAL_E,
    SL_WEI_A,
    SL_WEI_B,
    SL_WEI_C,
    SL_WEI_D,
    SL_WEI_E, 

    // keep this as last
    NUM_UI_ELEMENTS
};


// names of ui elements
static char * ui_str[] = { 
    "listen",
    "stop",
    "find",
    "add-lib",
    "load-db",
    "add-db",
    "view",
    "center",
	"analysis",
	"synthesis",
	"group",
	"control",
	"search",
	"exit",
	"fullscreen",
    
    "rotate",
    "zoom",
    "rotate",
    "show N",

    "null",
    "null",
    "null",
    "null",
    "null",
    "null",
    "null",
    "null",
    "null",
    "null",
};




//-----------------------------------------------------------------------------
// name: UISearch()
// desc: ...
//-----------------------------------------------------------------------------
UISearch::UISearch( ) : AudicleFace( )
{ 
    if( !this->init( ) )
    {
        fprintf( stderr, "[audicle]: cannot start face...\n" );
        return;
    }
}


//-----------------------------------------------------------------------------
// name: ~UISearch()
// desc: ...
//-----------------------------------------------------------------------------
UISearch::~UISearch( ) 
{ 
    // delete ui_elements
	for( int i = 0; i < NUM_UI_ELEMENTS; i++ )
	{
		SAFE_DELETE( ui_elements[i] );
	}
	SAFE_DELETE_ARRAY( ui_elements );
    // delete bagels
    // delete library?
    // anything else?
}


//-----------------------------------------------------------------------------
// name: init()
// desc: ...
//-----------------------------------------------------------------------------
t_TAPBOOL UISearch::init( )
{
    if( !AudicleFace::init() )
        return FALSE;

    int i;
   // log
    BB_log( BB_LOG_SYSTEM, "initializing search user interface..." );
    // push log
    BB_pushlog();

    // ui elements
    ui_elements = new UI_Element *[NUM_UI_ELEMENTS];
    memset( ui_elements, 0, sizeof(UI_Element *) * NUM_UI_ELEMENTS );

    // allocate (not in loop if some are UI_Exps)
    for( i = 0; i < NUM_UI_ELEMENTS; i++ )
        ui_elements[i] = new UI_Element; 

    // get id for each element
    for( i = 0; i < NUM_UI_ELEMENTS; i++ )
    {
        ui_elements[i]->id = IDManager::instance()->getPickID();
        ui_elements[i]->name = ui_str[i];
    }

    // font size
    for( i = SL_VAL_A; i <= SL_WEI_E; i++ )
        ui_elements[i]->font_size = 1.3f;
    for( i = BT_LISTEN; i <= BT_ADD_DB; i++ )
        ui_elements[i]->font_size = 0.8f;
    ui_elements[BT_TOGGLE_VIEW]->font_size = 0.7f; 
    ui_elements[BT_CENTER]->font_size = 0.7f;

    // slider values
    for( i = SL_VAL_A; i <= SL_VAL_E; i++ )
    {
        ui_elements[i]->set_bounds( 0, 1 ); 
        ui_elements[i]->set_slide( 0 );
    }
    for( i = SL_WEI_A; i <= SL_WEI_E; i++ )
    {
        ui_elements[i]->set_bounds( 0, 1 ); 
        ui_elements[i]->set_slide( 1 );
    }
    for( i = SL_ROTATE_H; i <= SL_NUM_MATCHES; i++ )
    {
        ui_elements[i]->set_bounds( -180, 180, true );
        ui_elements[i]->set_slide( 0 );
    }
    // overwrite some bounds
    ui_elements[SL_VAL_E]->set_bounds( 0, 2 );
    ui_elements[SL_VAL_E]->set_slide( 0 );
    ui_elements[SL_ZOOM]->set_bounds(0, 5, false);
    ui_elements[SL_ZOOM]->set_slide( 1 );
    ui_elements[SL_NUM_MATCHES]->set_bounds(0, 1, true);
    ui_elements[SL_NUM_MATCHES]->set_slide( 1 );

    // initialize
    down = NULL;
    selected = NULL;
    selected_from_matches = false;
    load_sliders = false;
    m_list_view = true;
    m_center = true;
    xyz_center_init = false;

    // set background color
    m_bg[0] = 0.7; m_bg[1] = 0.8; m_bg[2] = 0.8; m_bg[3] = 1.0;

    // -... -... --- -..- (fix bounds later)
    m_nbboxs = 4;
    m_cur_bbox = -1;
    m_bboxs = new BBox[m_nbboxs];
    // lower sliders
    m_bboxs[0].add2d( Point2D( 0.0f, -1.2f ) ); 
    m_bboxs[0].add2d( Point2D( 10.0f, 0.0f ) );
    // library
    m_bboxs[1].add2d( Point2D( -10.0f, -1.2f ) );
    m_bboxs[1].add2d( Point2D( 0.0f, 0.0f ) );
    // matches
    m_bboxs[3].add2d( Point2D( -10.0f, 0.0f ) ); 
    m_bboxs[3].add2d( Point2D( 0.0f, 1.2f ) );
    // higher sliders
    m_bboxs[2].add2d( Point2D( 0.0, 0.0f ) );
    m_bboxs[2].add2d( Point2D( 10.0f, 1.2f ) );
    // highlight ON/OFF
    m_highlight = false;

    // everything (default)
    m_vrs.push_back( new ViewRegionManual( 0, 0, -1.0f, 1.0f, FALSE, FALSE ) );
    // library
    m_vrs.push_back( new ViewRegionManual( -1.2, 0.0, -1.0, 0.0, TRUE, TRUE ) );
    // lower sliders
    m_vrs.push_back( new ViewRegionManual( 0.0, 1.2, -1.0, 0.0, TRUE, TRUE ) );
    // higher sliders
    m_vrs.push_back( new ViewRegionManual( 0.0, 1.2, 0.0, 1.0, TRUE, TRUE ) );
    // matches
    m_vrs.push_back( new ViewRegionManual( -1.2, 0.0, 0.0, 1.0, TRUE, TRUE ) );
    // set ids according to pushback order and desired location
    m_vr_library_id = 1;
    m_vr_values_id = 2;
    m_vr_weights_id = 3;
    m_vr_matches_id = 4;

    // feature database
    db = NULL;
    db_indices = NULL;
    db_distances = NULL;
    db_path = "";
    db_filename = "";

    // fog
    fog_mode[0] = 0; /*fog_mode[1] = GL_EXP; fog_mode[2] = GL_EXP2;*/ fog_mode[1] = GL_LINEAR;
    fog_filter = 0;
    fog_density = .035f;

    // log
    BB_log( BB_LOG_INFO, "num ui elements: %d", NUM_UI_ELEMENTS );

    // pop log
    BB_poplog();

    // set
    //if( !g_search_face )
    //    g_search_face = this;

    return TRUE;
}




//-----------------------------------------------------------------------------
// name: destroy()
// desc: ...
//-----------------------------------------------------------------------------
t_TAPBOOL UISearch::destroy( )
{
    this->on_deactivate( 0.0 );
    m_id = Audicle::NO_FACE;
    m_state = INACTIVE;

    SAFE_DELETE( db );
    SAFE_DELETE_ARRAY( db_indices );
    SAFE_DELETE_ARRAY( db_distances );

    return TRUE;
}



//-----------------------------------------------------------------------------
// name: render()
// desc: ...
//-----------------------------------------------------------------------------
t_TAPUINT UISearch::render( void * data )
{
    // render each pane
    render_library_pane(); 
    render_matches_pane();
    render_weights_pane();
    render_values_pane();

    // buttons (currently homeless)
    draw_button( *ui_elements[BT_LOAD_DB], 0.2, -0.05, 0.0, .5, .5, 1.0, IMG_LOAD );
    draw_button( *ui_elements[BT_FIND], 0.35, -0.05, 0.0, .8, .8, .8, IMG_FIND );
    draw_button( *ui_elements[BT_ADD_DB], 0.5, -0.05, 0.0, .7, .7, .5, IMG_UPARROW );
    draw_button( *ui_elements[BT_ADD], 0.65, -0.05, 0.0, .7, .6, .5, IMG_LOAD );
    draw_button( *ui_elements[BT_LISTEN], 0.8, -0.05, 0.0, 0.5, 1.0, 0.5, IMG_PLAY );
    draw_button( *ui_elements[BT_STOP], 0.95, -0.05, 0.0, 1.0, 0.5, 0.5, IMG_STOP );
    // bouncing play button
    if( selected != NULL && selected->core != NULL )
        ui_elements[BT_LISTEN]->on = selected->core->playing();
    else
        ui_elements[BT_LISTEN]->on = false;

	// faces / ctrl
	draw_face_buttons( BT_ANALYSIS_FACE, BT_SEARCH_FACE );
	draw_ctrl_buttons( BT_QUIT, BT_FULLSCREEN );

    // rotate
    g_r += 1.0;

    // highlighting
    glDisable( GL_LIGHTING );
    if( m_highlight && m_cur_bbox >= 0 && m_cur_bbox < m_nbboxs )
    {
        Point3D p1 = m_bboxs[m_cur_bbox].pmin();
        Point3D p2 = m_bboxs[m_cur_bbox].pmax();
        glPushMatrix();
            glTranslatef( 0.0f, 0.0f, -0.5f );
            glColor3f( 0.7f, 0.9f, 1.0f );
            glBegin( GL_QUADS );
            glVertex2f( p1[0], p1[1] );
            glVertex2f( p2[0], p1[1] );
            glVertex2f( p2[0], p2[1] );
            glVertex2f( p1[0], p2[1] );
            glEnd();
        glPopMatrix();
    }
    glEnable( GL_LIGHTING );


    return 0;
}


//-----------------------------------------------------------------------------
// name: render_library_pane()
// desc: ...
//-----------------------------------------------------------------------------
void UISearch::render_library_pane()
{
    assert( m_vr_library_id >= 0 && m_vr_library_id < m_vrs.size() ); 
    ViewRegion * lib = m_vrs[m_vr_library_id]; 
    Spectre color; 
    color.r = color.g = color.b = 0.0f;
    float x_offset = 0.15f, y_offset = 0.2f;
    draw_library( selected, NULL, lib->left() + x_offset, lib->down() + y_offset, 
                  lib->right() - x_offset, lib->up() - y_offset, &color );
}


//-----------------------------------------------------------------------------
// name: render_weights_pane()
// desc: ...
//-----------------------------------------------------------------------------
void UISearch::render_weights_pane()
{
    assert( m_vr_weights_id >= 0 && m_vr_weights_id < m_vrs.size() ); 
    ViewRegion * wei = m_vrs[m_vr_weights_id]; 
    glPushMatrix();
    glTranslatef( wei->left(), wei->down(), 0.0f );
    // sliders
    ui_elements[SL_WEI_A]->offset = draw_slider_mini( *ui_elements[SL_WEI_A], 0.2f, .25f, 0.0f );
    ui_elements[SL_WEI_B]->offset = draw_slider_mini( *ui_elements[SL_WEI_B], 0.4f, .25f, 0.0f );
    ui_elements[SL_WEI_C]->offset = draw_slider_mini( *ui_elements[SL_WEI_C], 0.6f, .25f, 0.0f );
    ui_elements[SL_WEI_D]->offset = draw_slider_mini( *ui_elements[SL_WEI_D], 0.8f, .25f, 0.0f );
    ui_elements[SL_WEI_E]->offset = draw_slider_mini( *ui_elements[SL_WEI_E], 1.0f, .25f, 0.0f );
    // pane name
        glPushMatrix();
        glTranslatef( (wei->right() - wei->left()) / 2, wei->up() - wei->down() - 0.25f, 0.0f );
        glColor3f( g_text_color[0], g_text_color[1], g_text_color[2] );
        scaleFont( .035 );
        glLineWidth( 2.0f );
        drawString_centered( "feature weights" );
        glLineWidth( 1.0f );
        glPopMatrix();
    glPopMatrix();
}


//-----------------------------------------------------------------------------
// name: render_matches_pane()
// desc: ...
//-----------------------------------------------------------------------------
void UISearch::render_matches_pane()
{
    assert( m_vr_matches_id >= 0 && m_vr_matches_id < m_vrs.size() ); 
    ViewRegion * mat = m_vrs[m_vr_matches_id]; 
    
    float x_offset = 0.15f, y_offset = -0.2f, y_inc = -0.07;
    float x_button = mat->right() - 0.7*x_offset - mat->left(), y_button = mat->down() - 0.5*y_offset - mat->up();
    Spectre color;
    color.r = 0.5; color.g = 0.3; color.b = 0.1;
    char char_buffer[128];

    glPushMatrix();
        glTranslatef( mat->left(), mat->up(), 0.0f );
            
        // button
        draw_button( *ui_elements[BT_TOGGLE_VIEW], x_button, y_button, 0.0f, .5f, .5f, 1.0f, IMG_TOG ); 
        
        // list view    
        if( m_list_view )
        {
            for( int m = 0; mat->up() + y_offset + y_inc * m > mat->down() - y_offset; m++ )
            {
                // load match template if necessary
                if( m >= Library::matches()->size() ) // if it needs loading
                    if( !load_match( m ) || m >= Library::matches()->size() ) // if it couldn't load or is an invalid index
                        break;  // forget it

                // draw template
                draw_template( x_offset, y_offset+y_inc*m, Library::matches()->templates[m], true, 0.0f, &color );
                // draw box
                if( selected == Library::matches()->templates[m] )
                {
                    glPushMatrix();
                        glTranslatef( x_offset, y_offset+y_inc*m, 0.0f );
                        float width = .085f, height = .085f;
                        glLineWidth( 2.0 );
                        glColor3f( color.r, color.g, color.b );
                        glBegin( GL_LINE_LOOP );
                        glVertex2f( -width/2.0f, -height/2.0f );
                        glVertex2f( -width/2.0f, height/2.0f );
                        glVertex2f( width/2.0f, height/2.0f );
                        glVertex2f( width/2.0f, -height/2.0f );
                        glEnd();
                        glLineWidth( 1.0 );
                        glColor3f( g_text_color[0], g_text_color[1], g_text_color[2] ); 
                    glPopMatrix();
                }
                // draw distance
                sprintf( char_buffer, "%.3f", db_distances[m] );
                glPushMatrix();
                    glTranslatef( (mat->right() - mat->left())/2, y_offset+y_inc*m-0.01, 0.0f );
                    scaleFont( 0.025 );
                    drawString( char_buffer );
                glPopMatrix();
            }
        }
        // 3D? hmm.
        else
        {
            int my_name = -1, feature_x, feature_y, feature_z;
            float x, y, z, vr_mid_x, vr_mid_y, vr_mid_z;
            // midpoint of view region
            vr_mid_x = (mat->right() - mat->left()) / 2;
            vr_mid_y = (mat->up() - mat->down()) / 2 - (mat->up() - mat->down());
            vr_mid_z = (10 - 2.3)/2;
            if( !xyz_center_init )
            {
                x_center = vr_mid_x; y_center = vr_mid_y; z_center = vr_mid_z;
                xyz_center_init = true;
            }
            // select features/sliders
            feature_x = 0; feature_y = 1; feature_z = 2;
            // draw matches
            glPushMatrix();
            // center around selected if available
            if( m_center && selected && selected->core->features )
            {
                // find center
                x_center = g_num_features > 0 ? selected->core->features[0] : 0;
                y_center = g_num_features > 1 ? selected->core->features[1] : 0;
                z_center = g_num_features > 2 ? selected->core->features[2] : 0;
                // map to this view region
                x_center = offset_in_range( selected->core->features[feature_x], ui_elements[SL_VAL_A + feature_x]->slide_0, 
                            ui_elements[SL_VAL_A + feature_x]->slide_1, mat->left(), mat->right() );
                y_center = offset_in_range( selected->core->features[feature_y], ui_elements[SL_VAL_A + feature_y]->slide_0,
                            ui_elements[SL_VAL_A + feature_y]->slide_1, mat->down(), mat->up() );
                y_center = y_center - (mat->up() - mat->down()); 
                z_center = offset_in_range( selected->core->features[feature_z], ui_elements[SL_VAL_A + feature_z]->slide_0, 
                            ui_elements[SL_VAL_A + feature_z]->slide_1, 2.3, 10 );
                m_center = false;
            }
            // each match
            glEnable( GL_NORMALIZE );
            // global transformation (to run from clip plane)
            glTranslatef( -.6f, 0.45f, -2.0f );
            glTranslatef( 0.25*vr_mid_x, 0.6*vr_mid_y, -0.25*vr_mid_z );  // multiplied to make up for perspective
            // rotate
            glRotatef( ui_elements[SL_ROTATE_H]->fvalue(), 0.0f, 1.0f, 0.0f );
            glRotatef( ui_elements[SL_ROTATE_V]->fvalue(), -1.0f, 0.0f, 0.0f ); 
            for( int m = 0; m < ui_elements[SL_NUM_MATCHES]->ivalue(); m++ )
            {
                // load match template if necessary
                if( m >= Library::matches()->size() ) // if it needs loading
                    if( !load_match( m ) || m >= Library::matches()->size() ) // if it couldn't load or is an invalid index
                        break;  // forget it
                // draw template
                UI_Template * ui_temp = Library::matches()->templates[m];
                // no, first load features into template                
                if( !ui_temp->core->features )
                {
                    int filenum = db_indices[m];
                    load_features_from_db( ui_temp->core, filenum );    
                }
                // now draw?
                // scale features between left and right
                x = offset_in_range( ui_temp->core->features[feature_x], ui_elements[SL_VAL_A + feature_x]->slide_0, 
                    ui_elements[SL_VAL_A + feature_x]->slide_1, mat->left(), mat->right() );
                // same for y between top and bottom
                y = offset_in_range( ui_temp->core->features[feature_y], ui_elements[SL_VAL_A + feature_y]->slide_0, 
                    ui_elements[SL_VAL_A + feature_y]->slide_1, mat->down(), mat->up() );
                // also move y down to get final value
                y = y - (mat->up() - mat->down());
                // same for z
                z = offset_in_range( ui_temp->core->features[feature_z], ui_elements[SL_VAL_A + feature_z]->slide_0, 
                    ui_elements[SL_VAL_A + feature_z]->slide_1, 2.3, 10 );
                // getting there...
                glPushMatrix();
                // weights
                float weight_x = ui_elements[SL_WEI_A + feature_x]->fvalue();
                float weight_y = ui_elements[SL_WEI_A + feature_y]->fvalue();
                float weight_z = ui_elements[SL_WEI_A + feature_z]->fvalue();
                // zoom
                float zoom = ui_elements[SL_ZOOM]->fvalue();
                glTranslatef( 0, 0, weight_z*zoom*(z_center-z) );
                x = weight_x*(x - x_center); y = weight_y*(y - y_center);
                // scale
                const float constant = 2.0f;
                glScalef( constant, constant, constant );
                // draw template!
                draw_template( zoom*x, 
                               zoom*y, 
                               ui_temp, false, 0.0f, &color ); // don't draw name
                // draw box
                if( selected == Library::matches()->templates[m] )
                {
                    glPushMatrix();
                        glTranslatef( zoom*x, zoom*y, 0.0f );
                        float width = .065f;
                        glLineWidth( 2.0 );
                        glColor3f( color.r, color.g, color.b );
                        glutWireCube( width );
                        glLineWidth( 1.0 );
                        glColor3f( g_text_color[0], g_text_color[1], g_text_color[2] ); 
                    glPopMatrix();
                    my_name = m;
                }
                glPopMatrix();
            }
            glDisable( GL_NORMALIZE );
            glPopMatrix();
            // end of draw matches
            // draw selected's name
            if( my_name != -1 )
            {
                sprintf( char_buffer, "%s (%.3f)", selected->core->name.c_str(), db_distances[my_name] );
                glPushMatrix();
                    glTranslatef( x_offset, y_offset, 0.0f );
                    glLineWidth( 2.0f );
                    scaleFont( 0.025 );
                    glDisable( GL_LIGHTING );
                    glColor3f( color.r, color.g, color.b );
                    drawString( char_buffer );
                    glColor3f( g_text_color[0], g_text_color[1], g_text_color[2] );
                    glEnable( GL_LIGHTING );
                    glLineWidth( 1.0f );
                glPopMatrix();
            }
            // draw additional controls
            draw_button( *ui_elements[BT_CENTER], x_button - .15, y_button, 0.0f, .7f, .5f, .5f, IMG_CENTER );
            ui_elements[SL_ROTATE_H]->offset = draw_slider_h_mini( 
                                *ui_elements[SL_ROTATE_H], 
                                x_button - g_slider_height_mini - .3f, 
                                y_button -.01f, 0.0f );
            ui_elements[SL_ROTATE_V]->offset = draw_slider_mini( *ui_elements[SL_ROTATE_V], 
                                x_button, y_button + .25f, 0.0f );
            ui_elements[SL_ZOOM]->offset = draw_slider_h_mini( 
                                *ui_elements[SL_ZOOM], 
                                x_button - 2 * g_slider_height_mini - .35f, 
                                y_button -.01f, 0.0f );
            ui_elements[SL_NUM_MATCHES]->offset = draw_slider_mini( *ui_elements[SL_NUM_MATCHES], 
                                x_button - .15f, y_button + .25f, 0.0f );
        }
    glPopMatrix();
}


//-----------------------------------------------------------------------------
// name: render_values_pane()
// desc: ...
//-----------------------------------------------------------------------------
void UISearch::render_values_pane()
{
    assert( m_vr_values_id >= 0 && m_vr_values_id < m_vrs.size() ); 
    ViewRegion * val = m_vrs[m_vr_values_id]; 
    glPushMatrix();
    glTranslatef( val->left(), val->down(), 0.0f );
    // sliders
    ui_elements[SL_VAL_A]->offset = draw_slider_mini( *ui_elements[SL_VAL_A], 0.2f, .25f, 0.0f );
    ui_elements[SL_VAL_B]->offset = draw_slider_mini( *ui_elements[SL_VAL_B], 0.4f, .25f, 0.0f );
    ui_elements[SL_VAL_C]->offset = draw_slider_mini( *ui_elements[SL_VAL_C], 0.6f, .25f, 0.0f );
    ui_elements[SL_VAL_D]->offset = draw_slider_mini( *ui_elements[SL_VAL_D], 0.8f, .25f, 0.0f );
    ui_elements[SL_VAL_E]->offset = draw_slider_mini( *ui_elements[SL_VAL_E], 1.0f, .25f, 0.0f );
    // pane name
        glPushMatrix();
        glTranslatef( (val->right() - val->left()) / 2, val->up() - val->down() - 0.28f, 0.0f );
        glColor3f( g_text_color[0], g_text_color[1], g_text_color[2] );
        scaleFont( .035 );
        glLineWidth( 2.0f );
        drawString_centered( "feature values" );
        glLineWidth( 1.0f );
        glPopMatrix();
    glPopMatrix();
}


//-----------------------------------------------------------------------------
// name: render_pre()
// desc: ...
//-----------------------------------------------------------------------------
void UISearch::render_pre()
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
void UISearch::render_post()
{
    glPopAttrib();

    AudicleFace::render_post();
}




//-----------------------------------------------------------------------------
// name: render_view()
// desc: ...
//-----------------------------------------------------------------------------
void UISearch::render_view( )
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
    
    //gluPerspective( 45.0, (GLfloat) AudicleWindow::main()->m_w / 
    //     (GLfloat) AudicleWindow::main()->m_h, 1.0, 300.0 );
    
    if( m_list_view )
        glOrtho( m_vr.left(), m_vr.right(), m_vr.down(), m_vr.up(), -10, 10 );
    else
        glFrustum( m_vr.left(), m_vr.right(), m_vr.down(), m_vr.up(), 2.44f, 10.0f );
    m_vr.next();

    // set the matrix mode to modelview
    glMatrixMode( GL_MODELVIEW );
    // load the identity matrix
    glLoadIdentity( );
    // position the view point
    gluLookAt( 0.0f, 0.0f, 2.5f, 
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
t_TAPUINT UISearch::on_activate()
{
    // set text color
    g_text_color[0] = 0.0f;
    g_text_color[1] = 0.0f;
    g_text_color[2] = 0.0f;
    
    // check if selected has been deleted?
    int i;
    bool found = false;
    for( i = 0; !found && i < Library::instance()->size(); i++ )
        if( Library::instance()->templates[i] == selected )
            found = true;
    for( i = 0; !found && i < Library::matches()->size(); i++ )
        if( Library::matches()->templates[i] == selected )
            found = true;
    if( !found)
        selected = NULL;

    // fog
    GLfloat fogColor[4]= {0.5f, 0.5f, 0.5f, 1.0f};      // Fog Color
    if( fog_filter )
        glFogi(GL_FOG_MODE, fog_mode[fog_filter]);      // Fog Mode
    glFogfv(GL_FOG_COLOR, fogColor);            // Set Fog Color
    glFogf(GL_FOG_DENSITY, fog_density);                // How Dense Will The Fog Be
    glHint(GL_FOG_HINT, GL_DONT_CARE);          // Fog Hint Value
    glFogf(GL_FOG_START, 5.0f);             // Fog Start Depth
    glFogf(GL_FOG_END, 5.5f);               // Fog End Depth
    if( fog_filter )
        glEnable(GL_FOG);

    // audicle stuff
    return AudicleFace::on_activate();
}




//-----------------------------------------------------------------------------
// name: on_deactivate()
// desc: ...
//-----------------------------------------------------------------------------
t_TAPUINT UISearch::on_deactivate( t_TAPDUR dur )
{
    glDisable(GL_FOG);
    return AudicleFace::on_deactivate( dur );
}


//-----------------------------------------------------------------------------
// name: on_event()
// desc: ...
//-----------------------------------------------------------------------------
t_TAPUINT UISearch::on_event( const AudicleEvent & event )
{
    static t_TAPUINT m_mouse_down = FALSE;
    static t_TAPUINT which = 0;
    static Point2D last;
    t_TAPBOOL hit = FALSE;
    t_TAPBOOL somewhere = FALSE; // danger
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
                if( ie->checkID( ui_elements[i]->id ) )
                {
                    if( ie->state == ae_input_DOWN )
                    {
                        hit = TRUE;
                        last_hit_pos = ie->pos; 
                        ui_elements[i]->down = TRUE;

                        // sliders
                        if( i >= SL_VAL_A && i <= SL_VAL_E )
                        {
                            float y_offset = m_vrs[m_vr_values_id]->down() + ui_elements[i]->offset;
                            ui_elements[i]->slide_last = ( vr_to_world_y(m_vr,ie->pos[1]) - y_offset ) / g_slider_height_mini;
                        }
                        if( i >= SL_WEI_A && i <= SL_WEI_E )
                        {
                            float y_offset = m_vrs[m_vr_weights_id]->down() + ui_elements[i]->offset;
                            ui_elements[i]->slide_last = ( vr_to_world_y(m_vr,ie->pos[1]) - y_offset ) / g_slider_height_mini;
                        }
                        if( i >= SL_ROTATE_H && i <= SL_ZOOM )
                        {
                            float x_offset = m_vrs[m_vr_matches_id]->left() + ui_elements[i]->offset;
                            ui_elements[i]->slide_last = ( vr_to_world_x(m_vr,ie->pos[0]) - x_offset ) / g_slider_height_mini; 
                        }
                        if( i >= SL_ROTATE_V && i <= SL_NUM_MATCHES )
                        {
                            float y_offset = m_vrs[m_vr_matches_id]->up() + ui_elements[i]->offset; 
                            ui_elements[i]->slide_last = ( vr_to_world_y(m_vr,ie->pos[1]) - y_offset ) / g_slider_height_mini; 
                        }
                    }

                    if( ie->state == ae_input_UP && ui_elements[i]->down == TRUE )
                    {
                        // flippers, buttons 
                        switch( i )
                        {
                        case BT_FIND: // search based on selected library template 
                        {
                            // this was a templategrabber test, but seems useful
                            TemplateGrabber tg( 3 * BirdBrain::srate() );
                            if( selected )
                            {
                                Frame * f = new Frame;
                                tg.grab( selected->core, f );
                                AudioSrcFrame * matlab = new AudioSrcFrame( f );
                                AudioCentral::instance()->bus(2)->play( matlab );
                            }
                            // clear things to prevent nasty happenings
                            if( selected_from_matches )
                                selected = NULL;
                            if( !Library::matches()->clear() )
                                msg_box( "no", "could not clear" );
                            // do actual finding and draw somewhere
                            if( db )
                            {
                                // load weights from sliders
                                float weights[5]; // hard-coded. boo.
                                for( j = 0; j < 5; j++ )
                                    weights[j] = ui_elements[SL_WEI_A + j]->fvalue();
                                db->setWeights( weights );
                                // get target values
                                float targets[5]; // hard-coded. boo.
                                for( j = 0; j < 5; j++ )
                                    targets[j] = ui_elements[SL_VAL_A + j]->fvalue();
                                // match
                                db->rankClosestFiles( targets, db_indices, db_distances );
                                BB_log( BB_LOG_INFO, "Closest index: %i", db_indices[0] );
                                // enter into matches library? :|
                                for( j = 0; j < 1; j++ )
                                {
                                    load_match( j );
                                }
                                // select top match
                                if( Library::matches()->size() > 0 )
                                {
                                    selected = Library::matches()->templates[0]->orig; 
                                    selected_from_matches = true;
                                    load_features_from_db( selected->core, db_indices[0] );
                                    load_sliders = true;
                                    m_center = true;
                                }
                            }
                            else
                                msg_box( "wait", "first pick a features database");
                            break;
                        }
                        case BT_ADD: // add selected matched sound effect to library
                            if( selected && selected_from_matches && !Library::instance()->hasID( selected->core ) )
                                Library::instance()->add( selected->core->copy() );
                            else if( !selected )
                                msg_box( "Stop confusing me", "Nothing is selected!!" );
                            else if( !selected_from_matches )
                                msg_box( "No no no", "Load from matches (above) to library (below)" );
                            else
                                BB_log( BB_LOG_INFO, "Template already in library; not adding" );
                            break;
                        case BT_ADD_DB: // add selected template to database
                            if( db )
                            {
                                if( selected && !selected_from_matches )
                                {
                                    // log
                                    BB_log( BB_LOG_INFO, "add-db: '%s'...", selected->core->name.c_str() );
                                    string path = db_path + "/tapsfiles/" + selected->core->name + ".tap";
                                    BB_log( BB_LOG_INFO, "add-db path: '%s'...", path.c_str() );

                                    // write
                                    TemplateWriter w;
                                    if( w.open( (char *)path.c_str() ) )
                                    {
                                        w.write_template( selected->core ); 
                                        w.close();
                                    }
                                    else
                                    {
                                        msg_box( "ding!", "cannot open file for writing!" );
                                    }

                                    if( selected->core->features )
                                    {
                                        // adding to database
                                        db->addFile( selected->core->name.c_str(), selected->core->features, true );
                                    }
                                    else
                                    {
                                        msg_box( "But", "Selected template has no features!!" );
                                    }

                                    // path
                                    path = db_path + "/" + db_filename + ".fli";
                                    BB_log( BB_LOG_INFO, "add-db updating FLI file: '%s'...", path.c_str() );
                                    // write the faile
                                    db->writeFLIFile( path.c_str() );
                                }
                                else if( !selected )
                                    msg_box( "Stop confusing me", "Nothing is selected!!" );
                                else if( selected_from_matches )
                                    msg_box( "No no no", "Add-db from library (below) to database (above)" );
                            }
                            else
                            {
                                msg_box( "Okay", "Load a database first" );
                            }
                            break;
                        case BT_LOAD_DB: // load "feature library" / database
                        {
                            DirScanner dScan;
                            dScan.setFileTypes( 
                                "Database files (*.fli)\0*.fli\0"
                            );
                            fileData * fD = dScan.openFileDialog();
                            if( fD )
                            {
                                SAFE_DELETE( db );
                                db = new FeatureLibrary( (char *)(fD->fileName.c_str()) );
                                if( db )
                                {
                                    g_num_features = db->getNumFeats();
                                    BB_log( BB_LOG_INFO, "loaded feature database, apparently; %i features", g_num_features );
                                    // assign space for matching
                                    SAFE_DELETE_ARRAY( db_indices );
                                    db_indices = new int[db->size()];
                                    SAFE_DELETE_ARRAY( db_distances );
                                    db_distances = new float[db->size()];
                                    // set path
                                    db_path = BirdBrain::getpath( fD->fileName.c_str() );
                                    // set filename
                                    db_filename = BirdBrain::getname( fD->fileName.c_str() );
                                    // clear earlier matches
                                    for( j = 0; selected && j < Library::matches()->size(); j++ )
                                        if( selected == Library::matches()->templates[j] )
                                            selected = NULL;
                                    Library::matches()->clear();
                                    // update slider names
                                    for( j = 0; j < g_num_features && j < 5; j++ )
                                    {
                                        ui_elements[SL_VAL_A + j]->name = db->getFeatureName( j );
                                        ui_elements[SL_WEI_A + j]->name = db->getFeatureName( j );
                                    }
                                    for( j = g_num_features; j < 5; j++ )
                                    {
                                        ui_elements[SL_VAL_A + j]->name = "null";
                                        ui_elements[SL_WEI_A + j]->name = "null";
                                        ui_elements[SL_VAL_A + j]->set_slide( 0 );
                                    }
                                    // update slider values
                                    load_sliders = true;
                                    // update max number of matches displayed in 3D view
                                    ui_elements[SL_NUM_MATCHES]->set_bounds(1, db->size(), true );
                                    ui_elements[SL_NUM_MATCHES]->set_slide( db->size() / 2 );
                                    // clear library template features?
                                    // makes limited sense since template features depend on RegionComparer 
                                    // rather than database
                                    // but g_num_features depends on database, so safer this way (in case it increases)
                                    for( j = 0; j < Library::instance()->size(); j++ )
                                    {
                                        Library::instance()->templates[j]->core->clear_features();
                                    }
                                }
                                else
                                {
                                    BB_log( BB_LOG_INFO, "could not open feature database %s", fD->fileName.c_str() ); 
                                }
                            }
                            break;
                        }
                        case BT_TOGGLE_VIEW:
                            m_list_view = !m_list_view;
                            break;
                        case BT_CENTER:
                            m_center = true;
                            break;
                        case BT_LISTEN:
                            if( selected != NULL )
                                play_template( selected ); 
                            else
                                msg_box( "erat e taps!!!", "select something first!" );
                            break;
                        case BT_STOP:
                            if( selected != NULL )
                                selected->core->stop();
                            break;
                        case BT_ANALYSIS_FACE:
						case BT_SYNTHESIS_FACE:
						case BT_GROUP_FACE:
						case BT_CONTROL_FACE:
						case BT_SEARCH_FACE:
							handle_face_button( BT_ANALYSIS_FACE, BT_SEARCH_FACE, i );
							break;
						case BT_QUIT:
						case BT_FULLSCREEN:
							handle_ctrl_button( BT_QUIT, BT_FULLSCREEN, i );
							break;
						default:
                            break;
                        }
                    }
                    
                    if( ie->state == ae_input_UP )
                    {
                        // place events on timeline
                    }
                } // if checkid

                // button up
                if( ie->state == ae_input_UP && ui_elements[i]->down )
                    ui_elements[i]->down = FALSE;
            }
                
            // check templates
            t_TAPBOOL nulldown = FALSE;
            hit = check_library_event( ie, Library::instance(), nulldown ) || hit;
            hit = check_library_event( ie, Library::matches(), nulldown ) || hit;

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

            // mouse up in background (hopefully)
            if( somewhere == FALSE && ie->state != ae_input_DOWN )
            {
                         
            } // END of mouse up in background

            // danger
            if( nulldown ) down = NULL; 
        } // END OF ae_input_MOUSE

        else if( ie->type == ae_input_MOTION )
        {
            handle_motion_event( ie );
        } // end of ae_input_MOTION 

        else if( ie->type == ae_input_KEY )
        {
            handle_keyboard_event( ie->key );
        } // end of ae_input_KEY

        // load sliders?
        if( load_sliders && selected )
        {
            // get features
            /*if( db && !selected->core->features )
            {
                int filenum = db->getFileNum( selected->core->name.c_str() );
                BB_log( BB_LOG_INFO, "File num %i for %s", filenum, selected->core->name.c_str() );
                load_features_from_db( selected->core, filenum );
            }*/
            if( !selected->core->features )
            {
                extract_features( selected->core );
            }
            // load sliders
            if( selected->core->features )
                for( j = 0; j < g_num_features; j++ )
                    ui_elements[SL_VAL_A + j]->set_slide( selected->core->features[j] );
            load_sliders = false;
        }
    }

    return AudicleFace::on_event( event );
}


// what to do if something a ui_temp from a library is hit
t_TAPBOOL UISearch::check_library_event( InputEvent *ie, Library *lib, t_TAPBOOL &released )
{
    t_TAPBOOL hit = FALSE;
    for( int j = 0; j < lib->size(); j++ )
    {
        // if a template is selected
        if( ie->checkID( lib->templates[j]->id ) )
        {
            UI_Template * ui_temp_j = lib->templates[j]; 
        
            if( ie->state == ae_input_DOWN )
            {
                hit = TRUE;
                last_hit_pos = ie->pos;
                down = ui_temp_j; 
                orig_pt = curr_pt = vr_to_world(m_vr,ie->pos);
                ui_temp_j->down = TRUE;
            }

            if( ie->state == ae_input_UP && ui_temp_j->down == TRUE )
            {
                // right button: just play
                if( ie->button == ae_input_RIGHT_BUTTON )
                {
                    UI_Template * me = ui_temp_j->orig;                         
                    if( me->core->playing() )
                    {
                        me->core->stop();
                        BB_log( BB_LOG_INFO, "Stopping template" ); 
                    }
                    else
                    {
                        play_template( me );
                        BB_log( BB_LOG_INFO, "Playing template" );
                    }
                }
                // other button: select
                else 
                {
                    selected = ui_temp_j->orig;
                    selected_from_matches = (lib == Library::matches()); 
                    load_sliders = true;
                }
            }
        }

        // button up
        if( ie->state == ae_input_UP && lib->templates[j]->down )
        {
            lib->templates[j]->down = FALSE;
            released = TRUE;
        }
    }

    return hit;
}


// handle keyboard events
t_TAPBOOL UISearch::handle_keyboard_event( char key )
{
    t_TAPBOOL valid = TRUE;
    switch( key )
    {
    case 8: // backspace
        if( selected != NULL )
        {
            if( selected->core->playing() ) {
                BB_log( BB_LOG_INFO, "Stopping template in preparation for deletion" );
                selected->core->stop(); 
                while( selected->core->playing() )
                    usleep( 5000 );
            }                               
            BB_log( BB_LOG_INFO, "Deleting template %s (0x%x)...", selected->core->name.c_str(), selected );
        }
        break;
    
    case ' ':
        if( selected != NULL )
            play_template( selected );
        break;
    case 'h':
    case 'H':
        m_highlight = !m_highlight;
        BB_log( BB_LOG_INFO, "Highlighting %s", m_highlight ? "ON" : "OFF" );
        break;
    //case 'r': 
    //case 'R':
    //    g_show_slider_range = !g_show_slider_range; 
    //    BB_log( BB_LOG_INFO, "%showing ranges of sliders", g_show_slider_range ? "S" : "Not s" ); // :)
    //    break;
    case 'q':
    case 'z':
        g_show_qz = !g_show_qz;
        BB_log( BB_LOG_INFO, "%showing quantization markers", g_show_qz ? "S" : "Not s" ); 
        break;
    case '0':
    case  27: // escape
        m_vr.setDest( *m_vrs[0] );
    break;
    case '1':
    case '2':
    case '3':
    case '4':
        {
            long index = key - '0';
            m_vr.setDest( m_vr.m_dest == m_vrs[index] ? *m_vrs[0] : *m_vrs[index] );
        }
        break;
    // temp
    case 'r': 
        m_bg[0] -= 0.1;
        if( m_bg[0] < 0 ) m_bg[0] = 0;
        BB_log( BB_LOG_INFO, "rgb: %f %f %f", m_bg[0], m_bg[1], m_bg[2] );
        break;
    case 'R': 
        m_bg[0] += 0.1;
        if( m_bg[0] > 1 ) m_bg[0] = 1;
        BB_log( BB_LOG_INFO, "rgb: %f %f %f", m_bg[0], m_bg[1], m_bg[2] );
        break;              
    case 'g': 
        m_bg[1] -= 0.1;
        if( m_bg[1] < 0 ) m_bg[1] = 0;
        BB_log( BB_LOG_INFO, "rgb: %f %f %f", m_bg[0], m_bg[1], m_bg[2] );
        break;              
    case 'G': 
        m_bg[1] += 0.1;
        if( m_bg[1] > 1 ) m_bg[1] = 1;
        BB_log( BB_LOG_INFO, "rgb: %f %f %f", m_bg[0], m_bg[1], m_bg[2] );
        break;              
    case 'b': 
        m_bg[2] -= 0.1;
        if( m_bg[2] < 0 ) m_bg[2] = 0;
        BB_log( BB_LOG_INFO, "rgb: %f %f %f", m_bg[0], m_bg[1], m_bg[2] );
        break;              
    case 'B': 
        m_bg[2] += 0.1;
        if( m_bg[2] > 1 ) m_bg[2] = 1;
        BB_log( BB_LOG_INFO, "rgb: %f %f %f", m_bg[0], m_bg[1], m_bg[2] );
        break;                
    // fog
    case 'F':
        fog_filter++;
        if( fog_filter > 1 ) fog_filter = 0;
        if( fog_filter )
        {
            // log
            BB_log( BB_LOG_INFO, "fog: ON" );
            glFogi(GL_FOG_MODE, fog_mode[fog_filter]);      // Fog Mode
            glEnable(GL_FOG);
        }
        else
        {
            // log
            BB_log( BB_LOG_INFO, "fog: OFF" );
            glDisable(GL_FOG);
        }
        break;
    // density
    case '<':
        fog_density *= .95f;
        BB_log( BB_LOG_INFO, "fog density: %f", fog_density );
        glFogf(GL_FOG_DENSITY, fog_density);
        break;
    case '>':
        fog_density *= 1.05f;
        BB_log( BB_LOG_INFO, "fog density: %f", fog_density );
        glFogf(GL_FOG_DENSITY, fog_density);
        break;

    default:
        valid = FALSE;
        break;
    }

    return valid;
}


// handle motion events
t_TAPBOOL UISearch::handle_motion_event( InputEvent * ie )
{
    t_TAPBOOL nothing = TRUE;
    int i;
    // fix sliders
    for( i = SL_VAL_A; i <= SL_VAL_E; i++ )
    {
        if( ui_elements[i]->down )
        {
            float y_offset = m_vrs[m_vr_values_id]->down() + ui_elements[i]->offset;
            fix_slider_motion( *ui_elements[i], m_vr, ie->pos, y_offset, g_slider_height_mini, true );
            nothing = FALSE;
        }
    }
    // fix more sliders
    for( i = SL_WEI_A; i <= SL_WEI_E; i++ )
    {
        if( ui_elements[i]->down )
        {
            float y_offset = m_vrs[m_vr_weights_id]->down() + ui_elements[i]->offset;
            fix_slider_motion( *ui_elements[i], m_vr, ie->pos, y_offset, g_slider_height_mini, true ); 
            nothing = FALSE;
        }
    }
    // and more
    for( i = SL_ROTATE_H; i <= SL_ZOOM; i++ )
    {
        if( ui_elements[i]->down )
        {
            float x_offset = m_vrs[m_vr_matches_id]->left() + ui_elements[i]->offset;
            fix_slider_motion( *ui_elements[i], m_vr, ie->pos, x_offset, g_slider_height_mini, false );
            nothing = FALSE;
        }
    }
    for( i = SL_ROTATE_V; i <= SL_NUM_MATCHES; i++ )
    {
        if( ui_elements[i]->down )
        {
            float y_offset = m_vrs[m_vr_matches_id]->up() + ui_elements[i]->offset;
            fix_slider_motion( *ui_elements[i], m_vr, ie->pos, y_offset, g_slider_height_mini, true );
            nothing = FALSE;
        }
    }

    // highlighting...
    if( nothing ) {
        m_cur_bbox = -1; 
        for( int b = 0; b < m_nbboxs; b++ )
            if( m_bboxs[b].in( vr_to_world(m_vr, ie->pos ) ) ) {
                m_cur_bbox = b;
                break;
            }
    }

    // ...
    if( down )
    {
        prev_pt = curr_pt;
        curr_pt = vr_to_world(m_vr,ie->pos);
    }

    // why?!?
    return TRUE;
}


void UISearch::play_template( UI_Template * playme )
{
    assert( playme->core != NULL );
    if( playme->core->playing() )
    {
        playme->core->stop();
        while( playme->core->playing() )
            usleep( 5000 );
    }
    // start over, taking parameters into account
    playme->core->recompute();
    
    AudioCentral::instance()->bus(2)->play( playme->core );
}


// load features from given database file index into given template
t_TAPBOOL UISearch::load_features_from_db( Template * temp, int index )
{
    if( temp && db && index >= 0 && index < db->size() )
    {
        const float * tmpfeats = db->getFeatureVec( index );
        temp->clear_features();
        temp->features = new float[g_num_features];
        for( int j = 0; j < g_num_features; j++ )
            temp->features[j] = tmpfeats[j];
        return TRUE;
    }
    else
        return FALSE;
}


// extract features for given template in real-time
t_TAPBOOL UISearch::extract_features( Template * temp )
{
    t_TAPBOOL okay = FALSE;
    if( temp )
    {
        TemplateGrabber tg( 3 * BirdBrain::srate() );
        Frame grabbed;
        okay = tg.grab( selected->core, &grabbed );
        if( okay )
        {
            std::vector<float> featvec;
            RegionComparer::getFeatures( grabbed, featvec );
            temp->clear_features();
            temp->features = new float[g_num_features];
            for( int j = 0; j < g_num_features; j++ ) 
                if( j < featvec.size() )
                    temp->features[j] = featvec[j];
        }
    }
    return okay;
}


// load match from given index
t_TAPBOOL UISearch::load_match( int index )
{
    // initial checks
    if( !db || index < 0 || index >= db->size() || db_indices[index] < 0 || db_indices[index] >= db->size() )
        return FALSE;

    int oldsize = Library::matches()->size();

    if( db->isTemplate( db_indices[index] ) )
    {
        // read taps file
        std::string filename = BirdBrain::getname( db->getFileName( db_indices[index] ) );
        filename = db_path + "/tapsfiles/" + filename + ".tap"; 
        const char * c = filename.c_str();
        TemplateReader r;
        if( r.open( (char *)c ) )
        {
            Template * tmp = r.read_template();
            r.close();
            if( tmp != NULL )
                Library::matches()->add( tmp );
        }
        else
            msg_box( "ding!", "cannot open file for reading!" );
    }
    else
    {
        // read sound file
        std::string filename = db->getFileName( db_indices[index] );
        //BB_log( BB_LOG_INFO, "Reading %s as sound file", filename.c_str() ); 
        File * file = new File( db_path + "/rawfiles/" + filename );
        file->name = BirdBrain::getname( filename.c_str() );
        Library::matches()->add( file );
    }

    return (Library::matches()->size() > oldsize);
}


// map given value in [val_low, val_high] to [low, high]
// return offset from low in [low, high] instead of actual value in [low, high]
float offset_in_range( float value, float val_low, float val_high, float low, float high )
{
    assert( low < high );
    assert( val_low < val_high );
    return (value - val_low) / (val_high - val_low) * (high - low); // + low for actual mapping
}
