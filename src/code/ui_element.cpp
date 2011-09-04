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
// name: ui_element.cpp
// desc: birdbrain ui element
//
// authors: Ananya Misra (amisra@cs.princeton.edu)
//          Ge Wang (gewang@cs.princeton.edu)
//          Perry R. Cook (prc@cs.princeton.edu)
//          Philip Davidson (philipd@cs.princeton.edu)
// date: Winter 2004
//-----------------------------------------------------------------------------
#include "ui_library.h"
#include "ui_element.h"
#include "audicle_gfx.h"
using namespace std;

#define WIDTH 64
#define HEIGHT 64

float g_text_color[3] = { 1.0f, 1.0f, 1.0f };
unsigned char g_texture[NUM_IMGS][WIDTH * HEIGHT * 4]; 
GLuint g_img[NUM_IMGS];
GLUquadricObj * g_quadric = NULL; 

#if defined(__MACOSX_CORE__)
#include <Carbon/Carbon.h>
#endif

#if defined(__PLATFORM_LINUX__)
#include <gtk/gtk.h>
#endif

//-----------------------------------------------------------------------------
// name: glu_init()
// desc: big function - initializes quadric object and textures for buttons
//-----------------------------------------------------------------------------
void glu_init()
{
    // initialize quadric
    g_quadric = gluNewQuadric(); 
    gluQuadricNormals( g_quadric, ( GLenum )GLU_SMOOTH );
    gluQuadricTexture( g_quadric, GL_TRUE );

    // set texture state
    glPixelStorei( GL_UNPACK_ALIGNMENT, 4 );
    glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
    glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST );

    // uh... initialize images
    int x, y, i, j;
    for( i = 0; i < NUM_IMGS; i++ )
    {
        switch(i) {
        case IMG_STOP: 
            for( y = 0; y < HEIGHT; y++ ) 
            {
                for( x = 0; x < WIDTH; x++ ) 
                {
                    if( x >= WIDTH/4 && x < 3*WIDTH/4 && y >= HEIGHT/4 && y < 3*HEIGHT/4 ) 
                    {
                        g_texture[i][4*(y*WIDTH + x)] = g_texture[i][4*(y*WIDTH + x) + 1]
                            = g_texture[i][4*(y*WIDTH + x) + 2] = 100;
                        g_texture[i][4*(y*WIDTH + x) + 3] = 255;
                    }
                    else 
                    {
                        g_texture[i][4*(y*WIDTH + x)] = g_texture[i][4*(y*WIDTH + x) + 1]
                            = g_texture[i][4*(y*WIDTH + x) + 2] = 255;
                        g_texture[i][4*(y*WIDTH + x) + 3] = 0;
                    }   
                }
            }
            break;
        case IMG_PLAY: 
            for( y = 0; y < HEIGHT; y++ ) 
            {
                for( x = 0; x < WIDTH; x++ ) 
                {
                    if( x >= WIDTH/4 && x < 3*WIDTH/4 
                        && y >= x*HEIGHT/(2*WIDTH)+HEIGHT/8 && y <= -x*HEIGHT/(2*WIDTH)+7*HEIGHT/8 )  
                    {
                        g_texture[i][4*(y*WIDTH + x)] = g_texture[i][4*(y*WIDTH + x) + 1]
                            = g_texture[i][4*(y*WIDTH + x) + 2] = 100;
                        g_texture[i][4*(y*WIDTH + x) + 3] = 255;
                    }
                    else 
                    {
                        g_texture[i][4*(y*WIDTH + x)] = g_texture[i][4*(y*WIDTH + x) + 1]
                            = g_texture[i][4*(y*WIDTH + x) + 2] = 255;
                        g_texture[i][4*(y*WIDTH + x) + 3] = 0;
                    }   
                }
            }
            break;
        case IMG_LOAD:
            for( y = 0; y < HEIGHT; y++ ) 
            {
                for( x = 0; x < WIDTH; x++ ) 
                {
                    if( (x >= WIDTH/4 && x < 3*WIDTH/4 && y < HEIGHT/2
                        && y >= -x*HEIGHT/WIDTH+3*HEIGHT/4 && y >= x*HEIGHT/WIDTH-HEIGHT/4) 
                        || ( x >= 7*WIDTH/16 && x <= 9*WIDTH/16 && y >= HEIGHT/2 && y < 3*HEIGHT/4 ) )
                    {
                        g_texture[i][4*(y*WIDTH + x)] = g_texture[i][4*(y*WIDTH + x) + 1]
                            = g_texture[i][4*(y*WIDTH + x) + 2] = 100;
                        g_texture[i][4*(y*WIDTH + x) + 3] = 255;
                    }
                    else 
                    {
                        g_texture[i][4*(y*WIDTH + x)] = g_texture[i][4*(y*WIDTH + x) + 1]
                            = g_texture[i][4*(y*WIDTH + x) + 2] = 255;
                        g_texture[i][4*(y*WIDTH + x) + 3] = 0;
                    }   
                }
            }
            break;
        case IMG_PREV:
            for( y = 0; y < HEIGHT; y++ ) 
            {
                for( x = 0; x < WIDTH; x++ ) 
                {
                    if( (y >= HEIGHT/4 && y < 3*HEIGHT/4 && x < WIDTH/2
                        && x >= -y*WIDTH/HEIGHT+3*WIDTH/4 && x >= y*WIDTH/HEIGHT-WIDTH/4) 
                        || ( y >= 7*HEIGHT/16 && y <= 9*HEIGHT/16 && x >= WIDTH/2 && x < 3*WIDTH/4 ) )
                    {
                        g_texture[i][4*(y*WIDTH + x)] = g_texture[i][4*(y*WIDTH + x) + 1]
                            = g_texture[i][4*(y*WIDTH + x) + 2] = 100;
                        g_texture[i][4*(y*WIDTH + x) + 3] = 255;
                    }
                    else 
                    {
                        g_texture[i][4*(y*WIDTH + x)] = g_texture[i][4*(y*WIDTH + x) + 1]
                            = g_texture[i][4*(y*WIDTH + x) + 2] = 255;
                        g_texture[i][4*(y*WIDTH + x) + 3] = 0;
                    }   
                }
            }
            break;
        case IMG_NEXT:
            for( y = 0; y < HEIGHT; y++ ) 
            {
                for( x = 0; x < WIDTH; x++ ) 
                {
                    if( (x >= WIDTH/4 && x < WIDTH/2 && y >= 7*HEIGHT/16 && y <= 9*HEIGHT/16)
                        || (x >= WIDTH/2 && x < 3*WIDTH/4 && y >= x*HEIGHT/WIDTH-HEIGHT/4 
                        && y <= -x*HEIGHT/WIDTH+5*HEIGHT/4) )
                    {
                        g_texture[i][4*(y*WIDTH + x)] = g_texture[i][4*(y*WIDTH + x) + 1]
                            = g_texture[i][4*(y*WIDTH + x) + 2] = 100;
                        g_texture[i][4*(y*WIDTH + x) + 3] = 255;
                    }
                    else 
                    {
                        g_texture[i][4*(y*WIDTH + x)] = g_texture[i][4*(y*WIDTH + x) + 1]
                            = g_texture[i][4*(y*WIDTH + x) + 2] = 255;
                        g_texture[i][4*(y*WIDTH + x) + 3] = 0;
                    }   
                }
            }
            break;
        case IMG_REV:
            for( y = 0; y < HEIGHT; y++ ) 
            {
                for( x = 0; x < WIDTH; x++ ) 
                {
                    if( (y >= x-HEIGHT/8 && y >= -x+5*HEIGHT/8 && y < x+HEIGHT/8 && y < -x+11*HEIGHT/8) 
                        || (y >= -x+7*HEIGHT/8 && y >= x-3*HEIGHT/8 && y < -x+9*HEIGHT/8 && y < x+3*HEIGHT/8) )
                    {
                        g_texture[i][4*(y*WIDTH + x)] = g_texture[i][4*(y*WIDTH + x) + 1]
                            = g_texture[i][4*(y*WIDTH + x) + 2] = 100;
                        g_texture[i][4*(y*WIDTH + x) + 3] = 255;
                    }
                    else 
                    {
                        g_texture[i][4*(y*WIDTH + x)] = g_texture[i][4*(y*WIDTH + x) + 1]
                            = g_texture[i][4*(y*WIDTH + x) + 2] = 255;
                        g_texture[i][4*(y*WIDTH + x) + 3] = 0;
                    }   
                }
            }
            break;
        case IMG_ALL:
            for( y = 0; y < HEIGHT; y++ )
            {
                for( x = 0; x < WIDTH; x++ )
                {
                    if( (x-3*WIDTH/8)*(x-3*WIDTH/8)/(WIDTH*WIDTH*9/256.0) + 
                        (y-3*HEIGHT/8)*(y-3*HEIGHT/8)/(HEIGHT*HEIGHT*9/256.0) <= 1 )
                    {
                        g_texture[i][4*(y*WIDTH + x)] = g_texture[i][4*(y*WIDTH + x) + 1] 
                            = g_texture[i][4*(y*WIDTH + x) + 2] = 100;
                        g_texture[i][4*(y*WIDTH + x) + 3] = 255;
                    }
                    else if( (x-WIDTH/2)*(x-WIDTH/2)/(WIDTH*WIDTH*9/256.0) + 
                        (y-5*HEIGHT/8)*(y-5*HEIGHT/8)/(HEIGHT*HEIGHT*9/256.0) <= 1 )
                    {
                        g_texture[i][4*(y*WIDTH + x)] = g_texture[i][4*(y*WIDTH + x) + 1] 
                            = g_texture[i][4*(y*WIDTH + x) + 2] = 180;
                        g_texture[i][4*(y*WIDTH + x) + 3] = 255;
                    }
                    else if( (x-5*WIDTH/8)*(x-5*WIDTH/8)/(WIDTH*WIDTH*9/256.0) + 
                        (y-3*HEIGHT/8)*(y-3*HEIGHT/8)/(HEIGHT*HEIGHT*9/256.0) <= 1 )
                    {
                        g_texture[i][4*(y*WIDTH + x)] = g_texture[i][4*(y*WIDTH + x) + 1] 
                            = g_texture[i][4*(y*WIDTH + x) + 2] = 150;
                        g_texture[i][4*(y*WIDTH + x) + 3] = 255;
                    }
                    else 
                    {
                        g_texture[i][4*(y*WIDTH + x)] = g_texture[i][4*(y*WIDTH + x) + 1]
                            = g_texture[i][4*(y*WIDTH + x) + 2] = 255;
                        g_texture[i][4*(y*WIDTH + x) + 3] = 0;
                    }
                }
            }
            break;
        case IMG_SAVE:
            for( y = 0; y < HEIGHT; y++ )
            {
                for( x = 0; x < WIDTH; x++ )
                {

                    if( (x-WIDTH/2)*(x-WIDTH/2)/(WIDTH*WIDTH*4/256.0) + 
                        (y-HEIGHT/2)*(y-HEIGHT/2)/(HEIGHT*HEIGHT*4/256.0) <= 1 )
                    {
                        g_texture[i][4*(y*WIDTH + x)] = 180; //200; 
                        g_texture[i][4*(y*WIDTH + x) + 1] = g_texture[i][4*(y*WIDTH + x) + 2] = 180; //100;
                        g_texture[i][4*(y*WIDTH + x) + 3] = 255;
                    }
                    else if( x >= WIDTH/4 && x < 3*WIDTH/4 && y >= HEIGHT/4 && y < 3*HEIGHT/4 )
                    {
                        g_texture[i][4*(y*WIDTH + x)] = g_texture[i][4*(y*WIDTH + x) + 1]
                            = g_texture[i][4*(y*WIDTH + x) + 2] = 100;
                        g_texture[i][4*(y*WIDTH + x) + 3] = 255;
                    }
                    else 
                    {
                        g_texture[i][4*(y*WIDTH + x)] = g_texture[i][4*(y*WIDTH + x) + 1]
                            = g_texture[i][4*(y*WIDTH + x) + 2] = 255;
                        g_texture[i][4*(y*WIDTH + x) + 3] = 0;
                    }
                }
            }
            break;
        case IMG_FIND:
            for( y = 0; y < HEIGHT; y++ )
            {
                for( x = 0; x < WIDTH; x++ )
                {
                    if( (x-3*WIDTH/8)*(x-3*WIDTH/8)/(WIDTH*WIDTH*9/256.0) + 
                        (y-HEIGHT/2)*(y-HEIGHT/2)/(HEIGHT*HEIGHT*9/256.0) <= 1 ) // circle
                    {
                        if( (x-3*WIDTH/8)*(x-3*WIDTH/8)/(WIDTH*WIDTH*4/256.0) + 
                            (y-HEIGHT/2)*(y-HEIGHT/2)/(HEIGHT*HEIGHT*4/256.0) <= 1 ) // inside
                        {
                            g_texture[i][4*(y*WIDTH + x)] = 180; 
                            g_texture[i][4*(y*WIDTH + x) + 1] = g_texture[i][4*(y*WIDTH + x) + 2] = 180;
                            g_texture[i][4*(y*WIDTH + x) + 3] = 255;
                        }
                        else // border
                        {
                            g_texture[i][4*(y*WIDTH + x)] = g_texture[i][4*(y*WIDTH + x) + 1]
                                = g_texture[i][4*(y*WIDTH + x) + 2] = 100;
                            g_texture[i][4*(y*WIDTH + x) + 3] = 255;
                        }
                    }
                    else if( (x >= 9*WIDTH/16 && x < 13*WIDTH/16 && y >= 7*HEIGHT/16.0 && y < 9*HEIGHT/16.0) ) // handle
                    {
                        g_texture[i][4*(y*WIDTH + x)] = g_texture[i][4*(y*WIDTH + x) + 1]
                            = g_texture[i][4*(y*WIDTH + x) + 2] = 100;
                        g_texture[i][4*(y*WIDTH + x) + 3] = 255;
                    }
                    else // nothing
                    {
                        g_texture[i][4*(y*WIDTH + x)] = g_texture[i][4*(y*WIDTH + x) + 1]
                            = g_texture[i][4*(y*WIDTH + x) + 2] = 255;
                        g_texture[i][4*(y*WIDTH + x) + 3] = 0;
                    }
                }
            }
            break;
        case IMG_SEP:
            for( y = 0; y < HEIGHT; y++ ) 
            {
                for( x = 0; x < WIDTH; x++ ) 
                {
                    if( (x >= WIDTH/4 && x < WIDTH/2 && y >= -x*(3.0*HEIGHT)/(4*WIDTH)+3.0*HEIGHT/4 
                        && y <= x*(5.0*HEIGHT)/(4*WIDTH)+HEIGHT/4) 
                        || (x >= WIDTH/2 && x < 3.0*WIDTH/4 && y > x*(5.0*HEIGHT)/(4*WIDTH)-HEIGHT/2
                        && y < -x*(3.0*HEIGHT)/(4*WIDTH)+HEIGHT) )
                    {
                        g_texture[i][4*(y*WIDTH + x)] = g_texture[i][4*(y*WIDTH + x) + 1]
                            = g_texture[i][4*(y*WIDTH + x) + 2] = 100;
                        g_texture[i][4*(y*WIDTH + x) + 3] = 255;
                    }
                    else 
                    {
                        g_texture[i][4*(y*WIDTH + x)] = g_texture[i][4*(y*WIDTH + x) + 1]
                            = g_texture[i][4*(y*WIDTH + x) + 2] = 255;
                        g_texture[i][4*(y*WIDTH + x) + 3] = 0;
                    }   
                }
            }
            break;
        case IMG_COPY:
            for( y = 0; y < HEIGHT; y++ ) 
            {
                for( x = 0; x < WIDTH; x++ ) 
                {
                    if( x >= 13*WIDTH/32 && x < 25*WIDTH/32 && y >= 7*HEIGHT/32 && y < 19*HEIGHT/32 )
                    {
                        g_texture[i][4*(y*WIDTH + x)] = g_texture[i][4*(y*WIDTH + x) + 1]
                            = g_texture[i][4*(y*WIDTH + x) + 2] = 180;
                        g_texture[i][4*(y*WIDTH + x) + 3] = 255;
                    }
                    else if( x >= 7*WIDTH/32 && x < 19*WIDTH/32 && y >= 13*HEIGHT/32 && y < 25*HEIGHT/32 )
                    {
                        g_texture[i][4*(y*WIDTH + x)] = g_texture[i][4*(y*WIDTH + x) + 1]
                            = g_texture[i][4*(y*WIDTH + x) + 2] = 100;
                        g_texture[i][4*(y*WIDTH + x) + 3] = 255;
                    }
                    else 
                    {
                        g_texture[i][4*(y*WIDTH + x)] = g_texture[i][4*(y*WIDTH + x) + 1]
                            = g_texture[i][4*(y*WIDTH + x) + 2] = 255;
                        g_texture[i][4*(y*WIDTH + x) + 3] = 0;
                    }   
                }
            }
            break;
        case IMG_MOD:
            for( y = 0; y < HEIGHT; y++ ) 
            {
                for( x = 0; x < WIDTH; x++ ) 
                {
                    if( y >= -x*HEIGHT/WIDTH+3*HEIGHT/4 && y >= x*HEIGHT/WIDTH-HEIGHT/4 
                        && y <= x*HEIGHT/WIDTH+HEIGHT/4 && y <= -x*HEIGHT/WIDTH+5*HEIGHT/4 ) 
                    {
                        g_texture[i][4*(y*WIDTH + x)] = g_texture[i][4*(y*WIDTH + x) + 1]
                            = g_texture[i][4*(y*WIDTH + x) + 2] = 100;
                        g_texture[i][4*(y*WIDTH + x) + 3] = 255;
                    }
                    else 
                    {
                        g_texture[i][4*(y*WIDTH + x)] = g_texture[i][4*(y*WIDTH + x) + 1]
                            = g_texture[i][4*(y*WIDTH + x) + 2] = 255;
                        g_texture[i][4*(y*WIDTH + x) + 3] = 0;
                    }   
                }
            }
            break;
        case IMG_TOG:
            for( y = 0; y < HEIGHT; y++ ) 
            {
                for( x = 0; x < WIDTH; x++ ) 
                {
                    if( y >= -x*HEIGHT/WIDTH+3*HEIGHT/4 && y <= x*HEIGHT/WIDTH+HEIGHT/4 && x < WIDTH/2 )
                    {
                        g_texture[i][4*(y*WIDTH + x)] = g_texture[i][4*(y*WIDTH + x) + 1]
                            = g_texture[i][4*(y*WIDTH + x) + 2] = 180;
                        g_texture[i][4*(y*WIDTH + x) + 3] = 255;
                    }
                    else if( y >= x*HEIGHT/WIDTH-HEIGHT/4 && y <= -x*HEIGHT/WIDTH+5*HEIGHT/4 && x >= WIDTH/2 ) 
                    {
                        g_texture[i][4*(y*WIDTH + x)] = g_texture[i][4*(y*WIDTH + x) + 1]
                            = g_texture[i][4*(y*WIDTH + x) + 2] = 100;
                        g_texture[i][4*(y*WIDTH + x) + 3] = 255;
                    }
                    else 
                    {
                        g_texture[i][4*(y*WIDTH + x)] = g_texture[i][4*(y*WIDTH + x) + 1]
                            = g_texture[i][4*(y*WIDTH + x) + 2] = 255;
                        g_texture[i][4*(y*WIDTH + x) + 3] = 0;
                    }   
                }
            }
            break;
        case IMG_NEWT:
            for( y = 0; y < HEIGHT; y++ ) 
            {
                for( x = 0; x < WIDTH; x++ ) 
                {
                    if( x >= WIDTH/16 && x < 15*WIDTH/16 && y >= 7*WIDTH/16 && y < 9*WIDTH/16 )
                    {
                        g_texture[i][4*(y*WIDTH + x)] = g_texture[i][4*(y*WIDTH + x) + 1]
                            = g_texture[i][4*(y*WIDTH + x) + 2] = 150;
                        g_texture[i][4*(y*WIDTH + x) + 3] = 255;
                    }
                    else 
                    {
                        g_texture[i][4*(y*WIDTH + x)] = g_texture[i][4*(y*WIDTH + x) + 1]
                            = g_texture[i][4*(y*WIDTH + x) + 2] = 255;
                        g_texture[i][4*(y*WIDTH + x) + 3] = 0;
                    }   
                }
            }
            break;
        case IMG_UPARROW:
            for( y = 0; y < HEIGHT; y++ ) 
            {
                for( x = 0; x < WIDTH; x++ ) 
                {
                    if( (y >= HEIGHT/4 && y < HEIGHT/2 && x >= 7*WIDTH/16 && x <= 9*WIDTH/16)
                        || (y >= HEIGHT/2 && y < 3*HEIGHT/4 && x >= y*WIDTH/HEIGHT-WIDTH/4 
                        && x <= -y*WIDTH/HEIGHT+5*WIDTH/4) )
                    {
                        g_texture[i][4*(y*WIDTH + x)] = g_texture[i][4*(y*WIDTH + x) + 1]
                            = g_texture[i][4*(y*WIDTH + x) + 2] = 100;
                        g_texture[i][4*(y*WIDTH + x) + 3] = 255;
                    }
                    else 
                    {
                        g_texture[i][4*(y*WIDTH + x)] = g_texture[i][4*(y*WIDTH + x) + 1]
                            = g_texture[i][4*(y*WIDTH + x) + 2] = 255;
                        g_texture[i][4*(y*WIDTH + x) + 3] = 0;
                    }   
                }
            }
            break;
        case IMG_CENTER:
            for( y = 0; y < HEIGHT; y++ )
            {
                for( x = 0; x < WIDTH; x++ )
                {
                    if( (x >= 15*WIDTH/32 && x < 17*WIDTH/32 && y >= HEIGHT/4 && y < 3*HEIGHT/4) ||
                             (y >= 15*HEIGHT/32 && y < 17*HEIGHT/32 && x >= WIDTH/4 && x < 3*WIDTH/4) ) // plus
                    {
                        g_texture[i][4*(y*WIDTH + x)] = g_texture[i][4*(y*WIDTH + x) + 1]
                            = g_texture[i][4*(y*WIDTH + x) + 2] = 100;
                        g_texture[i][4*(y*WIDTH + x) + 3] = 255;
                    }
                    else if( (x-WIDTH/2)*(x-WIDTH/2)/(WIDTH*WIDTH*9/256.0) + 
                        (y-HEIGHT/2)*(y-HEIGHT/2)/(HEIGHT*HEIGHT*9/256.0) <= 1 ) // circle
                    {
                        g_texture[i][4*(y*WIDTH + x)] = 180; //200; 
                        g_texture[i][4*(y*WIDTH + x) + 1] = g_texture[i][4*(y*WIDTH + x) + 2] = 180; //100;
                        g_texture[i][4*(y*WIDTH + x) + 3] = 255;
                    }
                    else // nothing
                    {
                        g_texture[i][4*(y*WIDTH + x)] = g_texture[i][4*(y*WIDTH + x) + 1]
                            = g_texture[i][4*(y*WIDTH + x) + 2] = 255;
                        g_texture[i][4*(y*WIDTH + x) + 3] = 0;
                    }
                }
            }
            break;
		case IMG_PLUS:
            for( y = 0; y < HEIGHT; y++ )
            {
                for( x = 0; x < WIDTH; x++ )
                {
                    if( (x >= WIDTH/8 && x < 7*WIDTH/8 && y >= 7*HEIGHT/16 && y < 9*HEIGHT/16) ||
						(y >= HEIGHT/8 && y < 7*HEIGHT/8 && x >= 7*WIDTH/16 && x < 9*WIDTH/16) ) // plus
                    {
                        g_texture[i][4*(y*WIDTH + x)] = g_texture[i][4*(y*WIDTH + x) + 1]
                            = g_texture[i][4*(y*WIDTH + x) + 2] = 100;
                        g_texture[i][4*(y*WIDTH + x) + 3] = 255;
                    }
                    else // nothing
                    {
                        g_texture[i][4*(y*WIDTH + x)] = g_texture[i][4*(y*WIDTH + x) + 1]
                            = g_texture[i][4*(y*WIDTH + x) + 2] = 255;
                        g_texture[i][4*(y*WIDTH + x) + 3] = 0;
                    }
                }
            }
            break;
		case IMG_RECORD:
            for( y = 0; y < HEIGHT; y++ )
            {
                for( x = 0; x < WIDTH; x++ )
                {
					if( (x-WIDTH/2)*(x-WIDTH/2)/(WIDTH*WIDTH/16.0) + 
                        (y-HEIGHT/2)*(y-HEIGHT/2)/(HEIGHT*HEIGHT/16.0) <= 1 ) // circle
                    {
                        g_texture[i][4*(y*WIDTH + x)] = g_texture[i][4*(y*WIDTH + x) + 1] 
							= g_texture[i][4*(y*WIDTH + x) + 2] = 100; //100;
                        g_texture[i][4*(y*WIDTH + x) + 3] = 255;
                    }
                    else // nothing
                    {
                        g_texture[i][4*(y*WIDTH + x)] = g_texture[i][4*(y*WIDTH + x) + 1]
                            = g_texture[i][4*(y*WIDTH + x) + 2] = 255;
                        g_texture[i][4*(y*WIDTH + x) + 3] = 0;
                    }
                }
            }
            break;
        default: 
            for( j = 0; j < WIDTH * HEIGHT; j++ ) 
            {
                g_texture[i][4*j] = g_texture[i][4*j + 1] = g_texture[i][4*j + 2] = 255;
                g_texture[i][4*j + 3] = 0;
            }
            break;
        }
    }

    // generate OpenGL texture
    glGenTextures( NUM_IMGS, &g_img[0] );
    for( i = 0; i < NUM_IMGS; i++ ) {
        glBindTexture( GL_TEXTURE_2D, g_img[i] );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR ); 
        if( g_img[i] )
            glTexImage2D( 
                GL_TEXTURE_2D,
                0,
                GL_RGBA,
                WIDTH,
                HEIGHT,
                0,
                GL_RGBA,
                GL_UNSIGNED_BYTE,
                g_texture[i]
            );
        else
            BB_log( BB_LOG_INFO, "g_img[%i] is %i", i, g_img[i]);
    }
}


//-----------------------------------------------------------------------------
// name: UI_Element()
// desc: UI_Element constructor
//-----------------------------------------------------------------------------
UI_Element::UI_Element()
{
    id = 0;
    down = FALSE;
    size_up = -1.0f;
    size_down = -1.0f;
    font_size = 1.0f;
    loc = NULL;
	rel_loc = NULL;
	must_adjust_loc = FALSE;

    slide = 0.0f;
    set_bounds( 0.0f, 1.0f );
    slide_int = false;
    slide_locally = false;

    on = FALSE;
    on_where = 0.0;

    the_height = -1.0f;
    the_width = -1.0f;

    id2 = 0;
    slide2 = 0.0f;
    slide2_last = 0.0f;

    offset = 0.0f;

	element_length = UI_NORMAL;
	element_type = UI_OTHER;
	element_orientation = UI_HORIZONTAL;
	name = "untitled";
}

//-----------------------------------------------------------------------------
// name: fvalue()
// desc: return slider's floating point value
//-----------------------------------------------------------------------------
t_TAPSINGLE UI_Element::fvalue() const
{
    return slide * (slide_1 - slide_0) + slide_0;
}

//-----------------------------------------------------------------------------
// name: ivalue()
// desc: return slider's integer value, rounded off to nearest int 
//-----------------------------------------------------------------------------
t_TAPINT UI_Element::ivalue() const
{
    return (t_TAPINT)(fvalue() + .5f);
}

//-----------------------------------------------------------------------------
// name: set_bounds()
// desc: set slider bounds
//-----------------------------------------------------------------------------   
void UI_Element::set_bounds( t_TAPSINGLE s0, t_TAPSINGLE s1, bool integer )
{
    slide_0 = s0;
    slide_1 = s1;
    slide_int = integer;
}

//-----------------------------------------------------------------------------
// name: set_slide()
// desc: set slide to the amount that represents a given value
//-----------------------------------------------------------------------------
void UI_Element::set_slide( t_TAPSINGLE val )
{
    slide = (val - slide_0) / (slide_1 - slide_0);
}

//-----------------------------------------------------------------------------
// name: set_slide2()
// desc: was meant to be for a slider with 2 values
//-----------------------------------------------------------------------------
void UI_Element::set_slide2( t_TAPSINGLE val )
{
    slide = (val - slide_0) / (slide_1 - slide_0);
}

//-----------------------------------------------------------------------------
// name: value()
// desc: return a string version of the fvalue of a slider
//-----------------------------------------------------------------------------
const std::string UI_Element::value() const
{
    char buffer[256];
    std::string val;

    t_TAPSINGLE v = fvalue();
	t_TAPINT w = ivalue();
    if( slide_int )
        sprintf( buffer, "%i", w );
    else
        sprintf( buffer, "%.3f", v );
    val = buffer;

    return val;
}

//-----------------------------------------------------------------------------
// name: draw()
// desc: draw function that calls others based on what info is set within 
//		 the ui_element. dangerous to use since the info is usually NOT set.
//		 use iff you know what you're doing.
//		 (Reassigns location assuming it's given relative to the vr center, 
//		 and between -1 and 1, if you pass it a vr.)
//-----------------------------------------------------------------------------
t_TAPSINGLE UI_Element::draw() const
{
	if( !loc )
		return 0;
	t_TAPSINGLE ret = 0;
	const Flt * p = loc->data();
	t_TAPSINGLE x = p[0], y = p[1], z = p[2];
	
	// slider
	if( element_type == UI_SLIDER ) {
		// length
		if( element_length == UI_NORMAL ) {
			// orientation
			if( element_orientation == UI_VERTICAL )
				ret = draw_slider( *this, x, y, z );
			else if( element_orientation == UI_HORIZONTAL )
				ret = draw_slider_h( *this, x, y, z ); 
		}
		else if( element_length == UI_MINI ) {
			// orientation
			if( element_orientation == UI_VERTICAL )
				ret = draw_slider_mini( *this, x, y, z );
			else if( element_orientation == UI_HORIZONTAL )
				ret = draw_slider_h_mini( *this, x, y, z );
		}
		else if( element_length == UI_MICRO ) {
			// only one orientation available so far
			ret = draw_slider_h_micro( *this, x, y, z );
		}
	}
	// flipper
	else if( element_type == UI_FLIPPER ) {
		// length
		if( element_length == UI_NORMAL )
			draw_flipper( *this, x, y, z );
		else if( element_length == UI_MICRO )
			draw_flipper_micro( *this, x, y, z );
	}
	// button (can't handle)
	else if( element_type == UI_BUTTON ) {
		BB_log( BB_LOG_WARNING, "Use draw_button() to draw button" );
		ret = -1;
	}
	// lr butter (can't handle)
	else if( element_type == UI_BUTTER ) {
		BB_log( BB_LOG_WARNING, "Use draw_lr_butter() to draw lr butter" );
		ret = -1;
	}
	// return
	return ret;
}

//-----------------------------------------------------------------------------
// name: adjust_loc()
// desc: determine "absolute" loc coordinates from "relative ones"
//-----------------------------------------------------------------------------
void UI_Element::adjust_loc( ViewRegion * vr )
{
	if( !rel_loc )
	{
		BB_log( BB_LOG_WARNING, "UI_Element has no loc to adjust" );
		return;
	}
	if( !must_adjust_loc )
	{
		BB_log( BB_LOG_INFO, "UI_Element: no need to adjust loc" );
		return;
	}
	const Flt * p = rel_loc->data();
	t_TAPSINGLE x = p[0], y = p[1], z = p[2];
	// map position based on view region
	if( vr != NULL )
	{
		t_TAPSINGLE vr_length = vr->right() - vr->left();
		t_TAPSINGLE vr_height = vr->up() - vr->down();
		t_TAPSINGLE vr_midx = (vr->right() + vr->left()) / 2;
		t_TAPSINGLE vr_midy = (vr->up() + vr->down()) / 2;
		x = vr_midx + x * vr_length / 2;
		y = vr_midy + y * vr_height / 2;
	}
	if( !loc )
		loc = new Point3D(x, y, z);
	else {
		Flt * q = loc->pdata();
		q[0] = x; q[1] = y; q[2] = z;
	}
	must_adjust_loc = FALSE;
}

#ifdef __TAPS_SCRIPTING_ENABLE__
//-----------------------------------------------------------------------------
// name: set_event()
// desc: hook up with chuck event
//-----------------------------------------------------------------------------
void UI_Element::set_event( Chuck_Event * e )
{
	this->event = e;
}
#endif


//-----------------------------------------------------------------------------
// name: UI_Exp()
// desc: constructor for exponential ui element
//-----------------------------------------------------------------------------
UI_Exp::UI_Exp( double base_ ) : UI_Element() 
{ 
	base = base_; 
	set_bounds( 0.0f, 1.0f ); 
}

//-----------------------------------------------------------------------------
// name: fvalue()
// desc: find value 
//-----------------------------------------------------------------------------
t_TAPSINGLE UI_Exp::fvalue() const
{
    return k * ::pow(base, slide) + c;
}

//-----------------------------------------------------------------------------
// name: set_bounds()
// desc: set up bounds for exponential slider
//-----------------------------------------------------------------------------
void UI_Exp::set_bounds( t_TAPSINGLE s0, t_TAPSINGLE s1, bool integer )
{
    slide_0 = s0;
    slide_1 = s1;
    k = (slide_1 - slide_0) / (base-1);
    c = slide_0 - k;
    slide_int = integer;
}

//-----------------------------------------------------------------------------
// name: set_slide()
// desc: set slide to represent given value
//-----------------------------------------------------------------------------
void UI_Exp::set_slide( t_TAPSINGLE val )
{
    slide = ::log10((val-c)/k) / ::log10(base);
}

//-----------------------------------------------------------------------------
// name: set_slide2()
// desc: was meant to be for a slider with 2 values
//-----------------------------------------------------------------------------
void UI_Exp::set_slide2( t_TAPSINGLE val )
{
    slide = ::log10((val-c)/k) / ::log10(base);
}


//-----------------------------------------------------------------------------
// name: draw_button()
// desc: ...
//-----------------------------------------------------------------------------
void draw_button( UI_Element & e, 
                  float x, float y, float z,
                  float r, float g, float b, 
                  int img_type )
{
    // do once maybe
    if( !g_quadric )
        glu_init();

    if( img_type < 0 || img_type >= NUM_IMGS )
        img_type = IMG_NONE;

    // set the size
    float size_up = e.size_up >= 0.0f ? e.size_up : .05f;
    float size_down = e.size_down >= 0.0f ? e.size_down : size_up * .9f;
    // if loc, use it
    if( e.loc ) { x = (*e.loc)[0]; y = (*e.loc)[1]; z = (*e.loc)[2]; }

    // draw
    glPushMatrix();
        // draw the button
        glPushName( e.id );
        glTranslatef( x, y, z );
        if( e.down )
            glColor3f( r/2, g/2, b/2 );
        else
            glColor3f( r, g, b );
        if( !e.down && e.on )
        {
            glPushMatrix();
            float value = ::sin( e.on_where );
            if( value < 0.0f ) value = -value;
            value = .9f + .25f * value;
            glScalef( value, value, value );
            e.on_where += .10;
        }
        //glutSolidSphere( e.down ? size_down : size_up, 15, 15 );
        //gluSphere( g_quadric, e.down ? size_down : size_up, 15, 15 ); 
        float my_size = e.down ? size_down : size_up;
        gluCylinder( g_quadric, my_size, my_size * .75, my_size , 20, 2 ); 
        glPushMatrix();
            if( g_img[img_type] )
            {
                glBindTexture( GL_TEXTURE_2D, g_img[img_type] );
                glEnable( GL_TEXTURE_2D );              
                //glEnable( GL_BLEND );
                //glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
            }
            glTranslatef( 0, 0, my_size );
            gluDisk( g_quadric, 0, my_size * .75, 20, 2 );
            if( g_img[img_type] )
            {
                glDisable( GL_TEXTURE_2D );
            //  glDisable( GL_BLEND );
            }
        glPopMatrix();
        if( !e.down && e.on )
            glPopMatrix();
        glPushMatrix();
            // draw the name
            glTranslatef( 0.0, -.115 * e.font_size, 0.0 );
            glColor3fv( g_text_color );
            scaleFont( .035 * e.font_size );
            drawString_centered( e.name.c_str() );
        glPopMatrix();
        glPopName();
    glPopMatrix();
}




// slider height
const float g_slider_height = .5f;
const float g_slider_height_mini = .4f;
const float g_slider_height_micro = .3f;
double g_r = 0.0f;
bool g_show_slider_range = true; 
//-----------------------------------------------------------------------------
// name: draw_slider()
// desc: ...
//-----------------------------------------------------------------------------
t_TAPSINGLE draw_slider( const UI_Element & e, 
                  float x, float y, float z )
{
    // set the size
    float size_up = e.size_up >= 0.0f ? e.size_up : .05f;
    float size_down = e.size_down >= 0.0f ? e.size_down : size_up * 1.25f;
    // if loc, use it
    if( e.loc ) { x = (*e.loc)[0]; y = (*e.loc)[1]; z = (*e.loc)[2]; }
    // slider height
    float slider_height = e.the_height <= 0 ? g_slider_height : e.the_height;

    // draw
    glPushMatrix();
        glTranslatef( x, y, z );
        glColor3fv( g_text_color );
        glDisable( GL_LIGHTING );
        glBegin( GL_LINES );
        glVertex2f( 0.0f, 0.0f );
        glVertex2f( 0.0f, slider_height );
        glEnd();
        glEnable( GL_LIGHTING );

        glPushMatrix();
            // draw the name
            glTranslatef( 0.0, -.10 * e.font_size, 0.0 );
            scaleFont( .035 * e.font_size );
            drawString_centered( e.name.c_str() );
        glPopMatrix();
        glPushMatrix();
            // draw the value
            glTranslatef( 0.0, -.10 * e.font_size * 1.5, 0.0 );
            scaleFont( .035 * e.font_size );
            drawString_centered( e.value().c_str() );
        glPopMatrix();
        if( g_show_slider_range )
        {
            glPushMatrix();
                // draw low value
                glTranslatef( .005 * e.font_size, 0.0, 0.0 ); 
                scaleFont( .015 * e.font_size ); 
                char buf[64]; 
                if( e.slide_int )
                    sprintf( buf, "%i", (int)e.slide_0 );
                else
                    sprintf( buf, "%.3f", e.slide_0 );
                drawString( buf ); 
            glPopMatrix();
            glPushMatrix();
                // draw high value
                glTranslatef( .005 * e.font_size, slider_height - .015 * e.font_size, 0.0 ); 
                scaleFont( .015 * e.font_size ); 
                if( e.slide_int )
                    sprintf( buf, "%i", (int)e.slide_1 );
                else
                    sprintf( buf, "%.3f", e.slide_1 );
                drawString( buf ); 
            glPopMatrix();
        }
        // draw the button
        glPushName( e.id );
        glTranslatef( 0.0, e.slide * slider_height, 0.0f );
        glRotatef( 45, 0.0, 1.0, 0.0 );
        //glRotatef( g_r, 0.0, 0.0, 1.0 );
        glColor3f( e.slide * .6f + .6f, (1-e.slide) * .6f + .6f, .6f ); // .2f lighter than actual slider
        glutSolidCube( e.down ? size_down : size_up );
        glColor3f( e.slide * .4f, (1-e.slide) * .4f, 0.0f );
        glDisable( GL_LIGHTING );
        glLineWidth( 2.0 );
        glutWireCube( e.down ? size_down * 1.001 : size_up * 1.001 );
        glLineWidth( 1.0 );
        glEnable( GL_LIGHTING );
        glPopName();
    glPopMatrix();

    return y;
}




//-----------------------------------------------------------------------------
// name: draw_slider_h()
// desc: horizontal
//-----------------------------------------------------------------------------
t_TAPSINGLE draw_slider_h( const UI_Element & e, 
                    float x, float y, float z )
{
    // set the size
    float size_up = e.size_up >= 0.0f ? e.size_up : .05f;
    float size_down = e.size_down >= 0.0f ? e.size_down : size_up * 1.25f;
    // if loc, use it
    if( e.loc ) { x = (*e.loc)[0]; y = (*e.loc)[1]; z = (*e.loc)[2]; }
    // slider height
    float slider_height = e.the_height <= 0 ? g_slider_height : e.the_height;

    // draw
    glPushMatrix();
        glTranslatef( x, y, z );
        glColor3fv( g_text_color );
        glDisable( GL_LIGHTING );
        glBegin( GL_LINES );
        glVertex2f( 0.0f, 0.0f );
        glVertex2f( g_slider_height, 0.0f );
        glEnd();
        glEnable( GL_LIGHTING );

        glPushMatrix();
            // draw the name
            glTranslatef( g_slider_height / 2, -.075 * e.font_size, 0.0 );
            scaleFont( .030 * e.font_size );
            drawString_centered( e.name.c_str() );
        glPopMatrix();
        glPushMatrix();
            // draw the value
            glTranslatef( g_slider_height / 2, -.075 * e.font_size * 1.65, 0.0 );
            scaleFont( .030 * e.font_size );
            drawString_centered( e.value().c_str() );
        glPopMatrix();
        if( g_show_slider_range )
        {
            glPushMatrix();
                // draw low value
                glTranslatef( 0.0, -.03 * e.font_size, 0.0 ); 
                scaleFont( .015 * e.font_size ); 
                char buf[64]; 
                if( e.slide_int )
                    sprintf( buf, "%i", (int)e.slide_0 );
                else
                    sprintf( buf, "%.3f", e.slide_0 );
                drawString( buf ); 
            glPopMatrix();
            glPushMatrix();
                // draw high value
                glTranslatef( slider_height, -.03 * e.font_size, 0.0 ); 
                scaleFont( .015 * e.font_size ); 
                if( e.slide_int )
                    sprintf( buf, "%i", (int)e.slide_1 );
                else
                    sprintf( buf, "%.3f", e.slide_1 ); 
                drawString( buf ); 
            glPopMatrix();
        }

        // draw the button
        glPushName( e.id );
        glTranslatef( e.slide * g_slider_height, 0.0f, 0.0f );
        glRotatef( 45, 0.0, 1.0, 0.0 );
        //glRotatef( g_r, 0.0, 0.0, 1.0 );
        glColor3f( e.slide * .6f + .4f, (1-e.slide) * .6f + .4f, .4f );
        glutSolidCube( e.down ? size_down : size_up );
        glColor3f( e.slide * .4f, (1-e.slide) * .4f, 0.0f );
        glDisable( GL_LIGHTING );
        glLineWidth( 2.0 );
        glutWireCube( e.down ? size_down * 1.001 : size_up * 1.001 );
        glLineWidth( 1.0 );
        glEnable( GL_LIGHTING );
        glPopName();
    glPopMatrix();

    return x;
}




//-----------------------------------------------------------------------------
// name: draw_knob()
// desc: horizontal
//-----------------------------------------------------------------------------
void draw_knob( const UI_Element & e, 
                float x, float y, float z )
{
    // set the size
    float size_up = e.size_up >= 0.0f ? e.size_up : .05f;
    float size_down = e.size_down >= 0.0f ? e.size_down : size_up;
    // if loc, use it
    if( e.loc ) { x = (*e.loc)[0]; y = (*e.loc)[1]; z = (*e.loc)[2]; }

    // draw
    glPushMatrix();
        glTranslatef( x, y, z );
        
        glColor3fv( g_text_color );
        glPushMatrix();
            // draw the name
            glTranslatef( 0.0f, -.09 * e.font_size, 0.0 );
            scaleFont( .025 * e.font_size );
            drawString_centered( e.name.c_str() );
        glPopMatrix();
        glPushMatrix();
            // draw the value
            glTranslatef( 0.0f, -.09 * e.font_size * 1.45, 0.0 );
            scaleFont( .020 * e.font_size );
            drawString_centered( e.value().c_str() );
        glPopMatrix();

        // draw the knob
        glPushName( e.id );
        glColor3f( e.slide * .6f + .4f, (1-e.slide) * .6f + .4f, .4f );
        glRotatef( 180.0f -e.slide * 180.0f, 0.0, 0.0, 1.0 );
        //glutSolidSphere( e.down ? size_down : size_up, 15, 15 );
        glutSolidTorus( size_up * .4f, size_up * .6f, 10, 8 );
        glPopName();

    glPopMatrix();
}


//-----------------------------------------------------------------------------
// name: draw_slider_mini()
// desc: ...
//-----------------------------------------------------------------------------
t_TAPSINGLE draw_slider_mini( const UI_Element & e, 
                       float x, float y, float z )
{
    // set the size
    float size_up = e.size_up >= 0.0f ? e.size_up : .05f;
    float size_down = e.size_down >= 0.0f ? e.size_down : size_up * 1.25f;
    // if loc, use it
    if( e.loc ) { x = (*e.loc)[0]; y = (*e.loc)[1]; z = (*e.loc)[2]; }
    // slider height
    float slider_height = e.the_height <= 0 ? g_slider_height_mini : e.the_height;

    // draw
    glPushMatrix();
        glTranslatef( x, y, z );
        glColor3fv( g_text_color );
        glDisable( GL_LIGHTING );
        glBegin( GL_LINES );
        glVertex2f( 0.0f, 0.0f );
        glVertex2f( 0.0f, g_slider_height_mini );
        glEnd();
        glEnable( GL_LIGHTING );

        glPushMatrix();
            // draw the name
            glTranslatef( 0.0f, -.06 * e.font_size, 0.0 );
            glColor3fv( g_text_color );
            scaleFont( .02 * e.font_size );
            drawString_centered( e.name.c_str() );
        glPopMatrix();
        glPushMatrix();
            // draw the value
            glTranslatef( 0.0f, -.06 * e.font_size * 1.5, 0.0 );
            glColor3fv( g_text_color );
            scaleFont( .02 * e.font_size );
            drawString_centered( e.value().c_str() );
        glPopMatrix();
        if( g_show_slider_range )
        {
            glPushMatrix();
                // draw low value
                glTranslatef( .005 * e.font_size, 0.0, 0.0 ); 
                scaleFont( .01 * e.font_size ); 
                char buf[64]; 
                if( e.slide_int )
                    sprintf( buf, "%i", (int)e.slide_0 );
                else
                    sprintf( buf, "%.3f", e.slide_0 );
                drawString( buf ); 
            glPopMatrix();
            glPushMatrix();
                // draw high value
                glTranslatef( .005 * e.font_size, slider_height - .01 * e.font_size, 0.0 ); 
                scaleFont( .01 * e.font_size ); 
                if( e.slide_int )
                    sprintf( buf, "%i", (int)e.slide_1 );
                else
                    sprintf( buf, "%.3f", e.slide_1 );
                drawString( buf ); 
            glPopMatrix();
        }

        // draw the button
        glPushName( e.id );
        glTranslatef( 0.0f, e.slide * g_slider_height_mini, 0.0f );
        glRotatef( 45, 0.0, 1.0, 0.0 );
        //glRotatef( g_r, 0.0, 0.0, 1.0 );
        glColor3f( e.slide * .6f + .4f, (1-e.slide) * .6f + .4f, .4f );
        glutSolidCube( e.down ? size_down : size_up );
        glColor3f( e.slide * .4f, (1-e.slide) * .4f, 0.0f );
        glDisable( GL_LIGHTING );
        glLineWidth( 2.0 );
        glutWireCube( e.down ? size_down * 1.001 : size_up * 1.001 );
        glLineWidth( 1.0 );
        glEnable( GL_LIGHTING );
        glPopName();
    glPopMatrix();

    return y;
}




//-----------------------------------------------------------------------------
// name: draw_slider_h_mini()
// desc: ...
//-----------------------------------------------------------------------------
t_TAPSINGLE draw_slider_h_mini( const UI_Element & e, 
                         float x, float y, float z )
{
    // set the size
    float size_up = e.size_up >= 0.0f ? e.size_up : .05f;
    float size_down = e.size_down >= 0.0f ? e.size_down : size_up * 1.25f;
    // if loc, use it
    if( e.loc ) { x = (*e.loc)[0]; y = (*e.loc)[1]; z = (*e.loc)[2]; }
    // slider height
    float slider_height = e.the_height <= 0 ? g_slider_height_mini : e.the_height;

    // draw
    glPushMatrix();
        glTranslatef( x, y, z );
        glColor3fv( g_text_color );
        glDisable( GL_LIGHTING );
        glBegin( GL_LINES );
        glVertex2f( 0.0f, 0.0f );
        glVertex2f( g_slider_height_mini, 0.0f );
        glEnd();
        glEnable( GL_LIGHTING );

        glPushMatrix();
            // draw name and value
		    char buf[64]; 
		    sprintf( buf, "%s (%s)", e.name.c_str(), e.value().c_str() ); 
            glTranslatef( g_slider_height_mini / 2, -.06 * e.font_size, 0.0 );
            scaleFont( .02 * e.font_size );
            drawString_centered( buf );
        glPopMatrix();
        if( g_show_slider_range )
        {
            glPushMatrix();
                // draw low value
                glTranslatef( 0.0, -.02 * e.font_size, 0.0 ); 
                scaleFont( .01 * e.font_size ); 
                char buf[64]; 
                if( e.slide_int )
                    sprintf( buf, "%i", (int)e.slide_0 );
                else
                    sprintf( buf, "%.3f", e.slide_0 );
                drawString( buf ); 
            glPopMatrix();
            glPushMatrix();
                // draw high value
                glTranslatef( slider_height, -.02 * e.font_size, 0.0 ); 
                scaleFont( .01 * e.font_size ); 
                if( e.slide_int )
                    sprintf( buf, "%i", (int)e.slide_1 );
                else
                    sprintf( buf, "%.3f", e.slide_1 ); 
                drawString( buf ); 
            glPopMatrix();
        }

        // draw the button
        glPushName( e.id );
        glTranslatef( e.slide * g_slider_height_mini, 0.0f, 0.0f );
        glRotatef( 45, 0.0, 1.0, 0.0 );
        //glRotatef( g_r, 0.0, 0.0, 1.0 );
        glColor3f( e.slide * .6f + .4f, (1-e.slide) * .6f + .4f, .4f );
        glutSolidCube( e.down ? size_down : size_up );
        glColor3f( e.slide * .4f, (1-e.slide) * .4f, 0.0f );
        glDisable( GL_LIGHTING );
        glLineWidth( 2.0 );
        glutWireCube( e.down ? size_down * 1.001 : size_up * 1.001 );
        glLineWidth( 1.0 );
        glEnable( GL_LIGHTING );        
        glPopName();
    glPopMatrix();

    return x;
}


//-----------------------------------------------------------------------------
// name: draw_slider_h_micro()
// desc: very small horizontal slider
//-----------------------------------------------------------------------------
t_TAPSINGLE draw_slider_h_micro( const UI_Element & e, 
                         float x, float y, float z )
{
    // set the size
    float size_up = e.size_up >= 0.0f ? e.size_up : .0375f;
    float size_down = e.size_down >= 0.0f ? e.size_down : size_up * 1.25f;
    // if loc, use it
    if( e.loc ) { x = (*e.loc)[0]; y = (*e.loc)[1]; z = (*e.loc)[2]; }
    // slider height
    float slider_height = e.the_height <= 0 ? g_slider_height_micro : e.the_height;

    // draw
    glPushMatrix();
        glTranslatef( x, y, z );
        glColor3fv( g_text_color );
        glDisable( GL_LIGHTING );
        glBegin( GL_LINES );
        glVertex2f( 0.0f, 0.0f );
        glVertex2f( slider_height, 0.0f );
        glEnd();
        glEnable( GL_LIGHTING );

        // draw name and value
        char buf[64]; 
        sprintf( buf, "%s (%s)", e.name.c_str(), e.value().c_str() ); 
        glPushMatrix();
            glTranslatef( slider_height / 2, -.042 * e.font_size, 0.0 );
            scaleFont( .02 * e.font_size );
            drawString_centered( buf );
        glPopMatrix();
        // draw range
        if( g_show_slider_range )
        {
            glPushMatrix();
                // draw low value
                glTranslatef( 0.0, -.015 * e.font_size, 0.0 ); 
                scaleFont( .01 * e.font_size ); 
                if( e.slide_int )
                    sprintf( buf, "%i", (int)e.slide_0 );
                else
                    sprintf( buf, "%.3f", e.slide_0 );
                drawString( buf ); 
            glPopMatrix();
            glPushMatrix();
                // draw high value
                glTranslatef( slider_height, -.015 * e.font_size, 0.0 ); 
                scaleFont( .01 * e.font_size ); 
                if( e.slide_int )
                    sprintf( buf, "%i", (int)e.slide_1 );
                else
                    sprintf( buf, "%.3f", e.slide_1 ); 
                drawString( buf ); 
            glPopMatrix();
        }

        // draw the button
        glPushName( e.id );
        glTranslatef( e.slide * slider_height, 0.0f, 0.0f );
        glRotatef( 45, 0.0, 1.0, 0.0 );
        //glRotatef( g_r, 0.0, 0.0, 1.0 );
        glColor3f( e.slide * .6f + .4f, (1-e.slide) * .6f + .4f, .4f );
        glutSolidCube( e.down ? size_down : size_up );
        glColor3f( e.slide * .4f, (1-e.slide) * .4f, 0.0f );
        glDisable( GL_LIGHTING );
        glLineWidth( 2.0 );
        glutWireCube( e.down ? size_down * 1.001 : size_up * 1.001 );
        glLineWidth( 1.0 );
        glEnable( GL_LIGHTING );        
        glPopName();
    glPopMatrix();

    return x;
}


t_TAPSINGLE draw_slider_range( const UI_Element & e, 
                        float x, float y, float z )
{
    // set the size
    float size_up = e.size_up >= 0.0f ? e.size_up : .05f;
    float size_down = e.size_down >= 0.0f ? e.size_down : size_up * 1.25f;
    // if loc, use it
    if( e.loc ) { x = (*e.loc)[0]; y = (*e.loc)[1]; z = (*e.loc)[2]; }
    // slider height
    float slider_height = e.the_height <= 0 ? g_slider_height : e.the_height;

    // draw
    glPushMatrix();
        glTranslatef( x, y, z );
        glColor3fv( g_text_color );
        glDisable( GL_LIGHTING );
        glBegin( GL_LINES );
        glVertex2f( 0.0f, 0.0f );
        glVertex2f( 0.0f, slider_height );
        glEnd();
        glEnable( GL_LIGHTING );

        glPushMatrix();
            // draw the name
            glTranslatef( 0.0, -.10 * e.font_size, 0.0 );
            scaleFont( .035 * e.font_size );
            drawString_centered( e.name.c_str() );
        glPopMatrix();
        glPushMatrix();
            // draw the value
            glTranslatef( 0.0, -.10 * e.font_size * 1.5, 0.0 );
            scaleFont( .035 * e.font_size );
            drawString_centered( e.value().c_str() ); // e.value2().c_str()
        glPopMatrix();

        // draw the button
        glPushMatrix();
        glPushName( e.id );
        glTranslatef( 0.0, e.slide * slider_height, 0.0f );
        glColor3f( e.slide * .6f + .4f, (1-e.slide) * .6f + .4f, .4f );
        glLineWidth( 3.0 );
        glBegin( GL_LINES );
        glVertex2f( -0.02, 0.0f );
        glVertex2f( 0.02, 0.0f );
        glEnd();
        glPopName();
        glPopMatrix();

        // draw the button
        glPushMatrix();
        glPushName( e.id2 );
        glTranslatef( 0.0, e.slide2 * slider_height, 0.0f );
        glColor3f( e.slide2 * .6f + .4f, (1-e.slide2) * .6f + .4f, .4f );
        glLineWidth( 3.0 );
        glBegin( GL_LINES );
        glVertex2f( -0.02, 0.0f );
        glVertex2f( 0.02, 0.0f );
        glEnd();
        glLineWidth( 1.0 );
        glPopName();
        glPopMatrix();
    glPopMatrix();

    return y;
}


//-----------------------------------------------------------------------------
// name: fix_slider_motion()
// desc: fixes slide values after slider motion
//       e = slider to be fixed
//       vr = current view region for that slider (depends on face state)
//       base_pt = start coordinate, from where the line was drawn
//       height = slider height
//       cur_pos = current mouse position
//       vertical = whether slider is vertical or horizontal
//-----------------------------------------------------------------------------
void fix_slider_motion( UI_Element & e, ViewRegion & vr, const Point2D & cur_pos, 
                       t_TAPSINGLE base_pt, t_TAPSINGLE height, t_TAPBOOL vertical )
{   
    if( vertical )
    {
        e.slide += (vr_to_world_y( vr, cur_pos[1] ) - base_pt ) / height - e.slide_last;
        e.slide_last = (vr_to_world_y( vr, cur_pos[1] ) - base_pt ) / height;
    }
    else
    {
        e.slide += (vr_to_world_x( vr, cur_pos[0] ) - base_pt ) / height - e.slide_last;
        e.slide_last = (vr_to_world_x( vr, cur_pos[0] ) - base_pt ) / height;
    }
    if( e.slide > 1.0 ) e.slide = 1.0; 
    if( e.slide_last > 1.0 ) e.slide_last = 1.0;
    if( e.slide < 0.0 ) e.slide = 0.0; 
    if( e.slide_last < 0.0 ) e.slide_last = 0;
}


//-----------------------------------------------------------------------------
// name: snap_slider_value()
// desc: snap slider's value to nearest int, if possible
//-----------------------------------------------------------------------------
void snap_slider_value( UI_Element & e )
{
	t_TAPINT ival = e.ivalue();
	if( ival > e.slide_1 )
		ival--;
	else if( ival < e.slide_0 )
		ival++;
	if( ival >= e.slide_0 && ival <= e.slide_1 )
		e.set_slide( ival );
}


// butter width
const float g_butter_width = 1.0f;
const float g_butter_height = .6f; 
//-----------------------------------------------------------------------------
// name: draw_lr_butter()
// desc: ...
//-----------------------------------------------------------------------------
void draw_lr_butter( const UI_Element & left, UI_Element * now, 
                     const UI_Element & right,
                     float x, float y, float z )
{
    glPushMatrix();
    glTranslatef( x, y, z );
    // butter height
    float butter_height = left.the_height <= 0 ? g_butter_height : left.the_height;
    // butter width
    float butter_width = left.the_width <= 0 ? g_butter_width : left.the_width;
    // size up / down
    float size_up, size_down;

    // more danger
    // the line is drawn only during render pass, and not select pass
    if( AudicleWindow::main()->m_render_mode == GL_RENDER )
    {
        glColor3fv( g_text_color );
        glDisable( GL_LIGHTING );
        glBegin( GL_LINES );
        glVertex3f( 0.0f, 0.0f, -0.01f );
        glVertex3f( butter_width, 0.0f, -0.01f );
        glEnd();
        glEnable( GL_LIGHTING );
    }

    // LEFT
    if( left.slide >= -0.001f && left.slide <= 1.001f )
    {
    // set the size
    size_up = left.size_up >= 0.0f ? left.size_up : .06f;
    size_down = left.size_down >= 0.0f ? left.size_down : size_up * .8f;
    
    glPushMatrix();
        glPushName( left.id );
        glTranslatef( left.slide * butter_width, 0.0f, 0.0f );
        glColor3f( (1-left.slide) * .6f + .4f, left.slide * .6f + .4f, .4f );
        //glutSolidSphere( left.down ? size_down : size_up, 15, 15 );
        /*glBegin( GL_POLYGON );
        glVertex2f( -0.0015f, (left.down ? size_down : size_up) );
        glVertex2f( -0.0015f, -(left.down ? size_down : size_up) );
        glVertex2f( (left.down ? size_down : size_up)*sqrt(3.0), 0.0f ); 
        glEnd();*/
        glPushMatrix();
        glTranslatef( -0.0015f, 0.0f, 0.0f );
        glRotatef( 90.0f, 0.0f, 1.0f, 0.0f );
        glutSolidCone( (left.down ? size_down : size_up), (left.down ? size_down : size_up)*sqrt(3.0), 5, 15 );
        glPopMatrix();
        glPushMatrix();
        // draw the name
        glTranslatef( 0.0, -.12 * left.font_size, 0.0 );
        glColor3fv( g_text_color );
        scaleFont( .035 * left.font_size );
        drawString_centered( left.name.c_str() );
        glPopMatrix();
        glDisable( GL_LIGHTING );
        glBegin( GL_LINES );
        glVertex2f( 0.0f, 0.0f );
        glVertex2f( 0.0f, butter_height );
        glEnd();
        glEnable( GL_LIGHTING );
        glPushMatrix();
        // draw the value
        glTranslatef( 0.0, butter_height + .03f, 0.0 );
        scaleFont( .035 * left.font_size );
        drawString_centered( left.value().c_str() );
        glPopMatrix();
        glPopName();
    glPopMatrix();
    }

    // RIGHT
    if( right.slide >= -0.001f && right.slide <= 1.001f )
    {
    // set the size
    size_up = right.size_up >= 0.0f ? right.size_up : .06f;
    size_down = right.size_down >= 0.0f ? right.size_down : size_up * 0.8f;

    glPushMatrix();
        glPushName( right.id );
        glTranslatef( right.slide * butter_width, 0.0f, 0.0f );
        glColor3f( (1-right.slide) * .6f + .4f, right.slide * .6f + .4f, .4f );
        //glutSolidSphere( right.down ? size_down : size_up, 15, 15 );
        /*glBegin( GL_POLYGON );
        glVertex2f( -(right.down ? size_down : size_up)*sqrt(3.0), 0.0f ); 
        glVertex2f( 0.0015f, -(right.down ? size_down : size_up) );
        glVertex2f( 0.0015f, (right.down ? size_down : size_up) );
        glEnd();*/
        glPushMatrix();
        glTranslatef( 0.0015f, 0.0f, 0.0f );
        glRotatef( -90.0f, 0.0f, 1.0f, 0.0f );
        glutSolidCone( (right.down ? size_down : size_up), (right.down ? size_down : size_up)*sqrt(3.0), 4, 15 );
        glPopMatrix();
        glPushMatrix();
        // draw the name
        glTranslatef( 0.0, -.12 * right.font_size, 0.0 );
        glColor3fv( g_text_color );
        scaleFont( .035 * right.font_size );
        drawString_centered( right.name.c_str() );
        glPopMatrix();
        glDisable( GL_LIGHTING );
        glBegin( GL_LINES );
        glVertex2f( 0.0f, 0.0f );
        glVertex2f( 0.0f, butter_height );
        glEnd();
        glEnable( GL_LIGHTING );
        // draw the value
        glPushMatrix();
        glTranslatef( 0.0, butter_height + .03f, 0.0 );
        scaleFont( .035 * right.font_size );
        drawString_centered( right.value().c_str() );
        glPopMatrix();
        glPopName();
    glPopMatrix();
    } 
    
    // NOW
    if( now )
    {
        if( now->slide_locally )
            now->slide = left.slide + now->slide_local * (right.slide-left.slide);

        if( now->slide >= -0.001f &&  now->slide <= 1.001f )
        {
        // set the size
        size_up = now->size_up >= 0.0f ? now->size_up : .06f;
        size_down = now->size_down >= 0.0f ? now->size_down : size_up * 0.8f;
        t_TAPSINGLE hello = .03;

        glPushMatrix();
            glPushName( now->id );
            glTranslatef( now->slide * butter_width, 0, 0.0f );
            glColor3f( (1-now->slide) * .6f + .4f, now->slide * .6f + .4f, .4f );
            glutSolidSphere( now->down ? size_down : size_up, 15, 15 );
            glPushMatrix();
            // draw the name
            glTranslatef( 0.0, -.12 * now->font_size - hello, 0.0 );
            glColor3fv( g_text_color );
            scaleFont( .035 * now->font_size );
            drawString_centered( now->name.c_str() );
            glPopMatrix();
            glColor3f( (1-now->slide) * .6f + .4f, now->slide * .6f + .4f, .4f );
            glDisable( GL_LIGHTING );
            glBegin( GL_LINES );
            glVertex2f( 0.0f, 0.0f );
            glVertex2f( 0.0f, butter_height );
            glEnd();
            glEnable( GL_LIGHTING );
            // draw the value
            glPushMatrix();
            glTranslatef( 0.0, butter_height + .03f, 0.0 );
            glColor3fv( g_text_color );
            scaleFont( .035 * now->font_size );
            drawString_centered( now->value().c_str() );
            glPopMatrix();
            glPopName();
        glPopMatrix();
        }
    }

    glPopMatrix();
}




const float g_flipper_width = .08f;
void draw_flipper( const UI_Element & e, float x, float y, float z )
{
    // set the size
    float size_up = e.size_up >= 0.0f ? e.size_up : .05f;
    float size_down = e.size_down >= 0.0f ? e.size_down : size_up * .75f;
    size_up *= .8;
    size_down *= .8;
    // if loc, use it
    if( e.loc ) { x = (*e.loc)[0]; y = (*e.loc)[1]; z = (*e.loc)[2]; }

    float factor = e.down ? .8 : 1.0;
    float left = -g_flipper_width / 2 * factor;
    float right = g_flipper_width / 2 * factor;
    float top = g_flipper_width * factor;
    float bottom = 0.0f * factor;

    // draw
    glPushMatrix();
        // draw the button
        glPushName( e.id );
        glTranslatef( x, y, z );
        glPushMatrix();
        if( e.down )
            glTranslatef( 0.0f, (g_flipper_width - g_flipper_width * factor) / 2, 0.0f );
        glColor3fv( g_text_color );
        glBegin( GL_LINE_LOOP );
        glVertex2f( left, bottom );
        glVertex2f( left, top );
        glVertex2f( right, top );
        glVertex2f( right, bottom );
        glEnd();
        glColor4f( 1.0f, 1.0f, 1.0f, 0.0f );
        glDisable( GL_LIGHTING );
        glBegin( GL_QUADS );
        glVertex2f( left, bottom );
        glVertex2f( left, top );
        glVertex2f( right, top );
        glVertex2f( right, bottom );
        glEnd();
        glEnable( GL_LIGHTING );
        glPopMatrix();
        glTranslatef( 0.0f, g_flipper_width / 2, 0.0f );
        if( e.slide > .5f )
        {
            glColor3f( .5f, 1.0f, .5f );
            glutSolidSphere( size_down, 15, 15 );
        }
        glPopName();

        glPushMatrix();
            // draw the name
            glTranslatef( 0.0, -.08 * e.font_size, 0.0 );
            glColor3fv( g_text_color );
            scaleFont( .03 * e.font_size );
            drawString_centered( e.name.c_str() );
        glPopMatrix();
    glPopMatrix();
}


void draw_flipper2( const UI_Element & e, float x, float y, float z )
{
    // set the size
    float size_up = e.size_up >= 0.0f ? e.size_up : .05f;
    float size_down = e.size_down >= 0.0f ? e.size_down : size_up * .75f;
    size_up *= .8;
    size_down *= .8;
    // if loc, use it
    if( e.loc ) { x = (*e.loc)[0]; y = (*e.loc)[1]; z = (*e.loc)[2]; }

    float factor = e.down ? .8 : 1.0;
    float left = -g_flipper_width / 2 * factor;
    float right = g_flipper_width / 2 * factor;
    float top = g_flipper_width * factor;
    float bottom = 0.0f * factor;

    // draw
    glPushMatrix();
        // draw the button
        glPushName( e.id );
        glTranslatef( x, y, z );
        glPushMatrix();
        if( e.down )
            glTranslatef( 0.0f, (g_flipper_width - g_flipper_width * factor) / 2, 0.0f );
        glColor3fv( g_text_color );
        glBegin( GL_LINE_LOOP );
        glVertex2f( left, bottom );
        glVertex2f( left, top );
        glVertex2f( right, top );
        glVertex2f( right, bottom );
        glEnd();
        glColor4f( 1.0f, 1.0f, 1.0f, 0.0f );
        glBegin( GL_QUADS );
        glVertex2f( left, bottom );
        glVertex2f( left, top );
        glVertex2f( right, top );
        glVertex2f( right, bottom );
        glEnd();
        glPopMatrix();        
        glTranslatef( 0.0f, g_flipper_width / 2, 0.0f );
        if( e.slide > .5f )
        {
            glColor3f( .5f, 1.0f, .5f );
            glutSolidSphere( size_down, 15, 15 );
        }
        glPopName();

        glPushMatrix();
            // draw the name
            glTranslatef( .06 * e.font_size, -0.015, 0.0 );
            glColor3fv( g_text_color );
            scaleFont( .03 * e.font_size );
            drawString( e.name.c_str() );
        glPopMatrix();
    glPopMatrix();
}


const float g_flipper_width_micro = .04f;
void draw_flipper_micro( const UI_Element & e, float x, float y, float z )
{
    // set the size
    float size_up = e.size_up >= 0.0f ? e.size_up : .025;
    float size_down = e.size_down >= 0.0f ? e.size_down : size_up * .75f;
    size_up *= .8;
    size_down *= .8;
    // if loc, use it
    if( e.loc ) { x = (*e.loc)[0]; y = (*e.loc)[1]; z = (*e.loc)[2]; }

    float factor = e.down ? .8 : 1.0;
    float left = -g_flipper_width_micro / 2 * factor;
    float right = g_flipper_width_micro / 2 * factor;
    float top = g_flipper_width_micro * factor;
    float bottom = 0.0f * factor;

    // draw
    glPushMatrix();
        // draw the button
        glPushName( e.id );
        glTranslatef( x, y, z );
        glPushMatrix();
        if( e.down )
            glTranslatef( 0.0f, (g_flipper_width_micro - g_flipper_width_micro * factor) / 2, 0.0f );
        glColor3fv( g_text_color );
        glBegin( GL_LINE_LOOP );
        glVertex2f( left, bottom );
        glVertex2f( left, top );
        glVertex2f( right, top );
        glVertex2f( right, bottom );
        glEnd();
        glColor4f( 1.0f, 1.0f, 1.0f, 0.0f );
        glDisable( GL_LIGHTING );
        glBegin( GL_QUADS );
        glVertex2f( left, bottom );
        glVertex2f( left, top );
        glVertex2f( right, top );
        glVertex2f( right, bottom );
        glEnd();
        glEnable( GL_LIGHTING );
        glPopMatrix();
        glTranslatef( 0.0f, g_flipper_width_micro / 2, 0.0f );
        if( e.slide > .5f )
        {
            glColor3f( .5f, 1.0f, .5f );
            glutSolidSphere( size_down, 15, 15 );
        }
        glPopName();

        glPushMatrix();
            // draw the name
            glTranslatef( 0.0, -0.043 * e.font_size, 0.0 );
            glColor3fv( g_text_color );
            scaleFont( .01875 * e.font_size );
            drawString_centered( e.name.c_str() );
        glPopMatrix();
    glPopMatrix();   
}


void draw_label( const string & text, float x, float y, float z, float font_size, bool centered, float * c )
{
    glPushMatrix();
    glTranslatef( x, y, z );
    glLineWidth( 2 );
    if( c )
        glColor3fv( c );
    else
        glColor3fv( g_text_color );
    scaleFont( .03 * font_size );
    if( centered )
        drawString_centered( text.c_str() );
    else
        drawString( text.c_str() );
    glLineWidth( 1 );
    glPopMatrix();
}

bool g_show_qz = true; // show whether it is quantized to a table
void draw_template( float x, float y, UI_Template * ui_temp, bool draw_name, float y_offset, Spectre * color )
{
    Template * temp = ui_temp->core;
    t_TAPBOOL playing = ui_temp->core ? ui_temp->core->playing() : FALSE;

    // shape
    glPushMatrix();
    glTranslatef( x, y + y_offset, 0.0f );
    if( color )
        glColor3f( color->r, color->g, color->b );
    else
        glColor3f( 0.0f, 0.0f, 0.0f );
    glBegin( GL_LINES );
    glVertex2f( 0, 0 );
    if( y_offset < .5f )
        glVertex2f( 0, -y_offset );
    else
        glVertex2f( 0, y_offset );
    glEnd();
    glPushName( ui_temp->id );
    if( playing )
    {
        glPushMatrix();
        double val = ::sin( ui_temp->on_where );
        if( val < 0 ) val = -val;
        val = .8 + .4 * val;
        glScalef( val, val, val );
        ui_temp->on_where += .04;
    }

    switch( temp->type )
    {
    case TT_DETERMINISTIC:
        glPushMatrix();
        glColor3f( .75, 1.0, .5 );
        glRotatef( 45.0f, 1.0f, 1.0f, 1.0f );
        glutSolidSphere( .03, 10, 10 );
        glColor3f( 0, .5, 0 );
        glRotatef( 90.0f, 1.0f, 0.0f, 0.0f );
        glDisable( GL_LIGHTING );
        glutWireSphere( .032, 10, 10 );
        glEnable( GL_LIGHTING );
        glPopMatrix();
        break;

    case TT_TRANSIENT:
        glPushMatrix();
        glColor3f( .25, .25, 1.0 );
        glTranslatef( 0.0f, -.0275, 0.0f );
        glRotatef( -90.0f, 1.0f, 0.0f, 0.0f );
        glutSolidCone( .02, 0.055, 10, 10 );
        glPopMatrix();
        break;

    case TT_FILE:
        glPushMatrix();
        glColor3f( 1.0, .25, .25 );
        glTranslatef( 0.0f, .0275f, 0.0f );
        glRotatef( 90.0f, 1.0f, 0.0f, 0.0f );
        glutSolidCone( .02, 0.055, 10, 10 );
        glPopMatrix();
        break;

    case TT_RESIDUE:
        glPushMatrix();
        glColor3f( 1.00, .5, .5 );
        glRotatef( 45.0f, ::sin(.45), .45, ::cos(.45) );
        glutSolidCube( .04 );
        glColor3f( .5, 0, 0 );
        glDisable( GL_LIGHTING );
        glLineWidth( 2.0 );
        glutWireCube( .0415 );
        glLineWidth( 1.0 );
        glEnable( GL_LIGHTING );
        glPopMatrix();
        break;

    case TT_LOOP:
        glPushMatrix();
        glColor3f( 1.00, .75, .5 );
        glRotatef( 45.0f, 1.0f, 1.0f, 0.0f );
        glutSolidTorus( .02, .0197, 12, 12 );
        /*glColor3f( .7, .25, 0 );
        glDisable( GL_LIGHTING );
        glLineWidth( 2.0 );
        glutWireTorus( .026, .015, 20, 20 );
        glLineWidth( 1.0 );
        */glEnable( GL_LIGHTING );
        glPopMatrix();
        break;

    case TT_TIMELINE:
    {
        float width = .08f;
        float factor = 1.0f;
        float left = -width / 2 * factor;
        float right = width / 2 * factor;
        float top = width * .1f * factor;
        float bottom = -width * .1f * factor;
    
        glPushMatrix();
        glColor3f( .5, .5, .5 );
        glDisable( GL_LIGHTING );
        glBegin( GL_QUADS );
        glVertex2f( left, bottom );
        glVertex2f( left, top );
        glVertex2f( right, top );
        glVertex2f( right, bottom );
        glEnd();
        glEnable( GL_LIGHTING );
        glPopMatrix();
    }
        break;

    case TT_BAG:
        glPushMatrix();
            glPushMatrix();
            glTranslatef( -.015, .005f, 0.0f );
            glColor3f( .75, 1.0, .5 );
            glRotatef( 45.0f, 1.0f, 1.0f, 1.0f );
            glutSolidSphere( .009, 8, 8 );
            glColor3f( 0, .5, 0 );
            glRotatef( 90.0f, 1.0f, 0.0f, 0.0f );
            glDisable( GL_LIGHTING );
            glutWireSphere( .0096, 8, 8 );
            glPopMatrix();
            glPushMatrix();
            glColor3f( .25, .25, 1.0 );
            glTranslatef( 0.001f, -.0125f, 0.0f );
            glRotatef( -90.0f, 1.0f, 0.0f, 0.0f );
            glutSolidCone( .01, 0.02255, 5, 5 );
            glPopMatrix();
            glPushMatrix();
            glColor3f( 1.00, .5, .5 );
            glRotatef( 45.0f, ::sin(.45), .45, ::cos(.45) );
            glTranslatef( 0.015f, -0.005f, 0.0f ); 
            glutSolidCube( .01 );
            glColor3f( .5, 0, 0 );
            glDisable( GL_LIGHTING );
            glLineWidth( 2.0 );
            glutWireCube( .010375 );
            glLineWidth( 1.0 );
            glEnable( GL_LIGHTING );
            glPopMatrix();
        glPopMatrix(); 
        break;

    case TT_SCRIPT:
        glPushMatrix();
            glColor3f( .9f, .6f, .3f );
            glTranslatef( 0.0f, -0.013, 0.0f );
            glRotatef( -45.0f, 1.0f, 0.0f, 0.0f );
            glPushMatrix();
                gluCylinder( g_quadric, .025, .025, .035, 8, 2 );
            glPopMatrix();
            glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
            glColor3f( .6f, .4f, .2f );
            glPushMatrix();
                gluCylinder( g_quadric, .026, .026, .036, 8, 2 ); 
            glPopMatrix();
            glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
        glPopMatrix();
        break;

    case TT_RAW:
        glPushMatrix();
            glDisable( GL_LIGHTING );
            glBegin( GL_QUADS );
            glColor3f( 1.00, .5, .5 );
            glVertex2f( -0.03, -0.03 );
            glColor3f( .5, 1.00, .5 );
            glVertex2f( 0.03, -0.03 );
            glColor3f( .5, .5, 1.00 );
            glVertex2f( 0.03, 0.03 );
            glColor3f( .75, .75, .75 );
            glVertex2f( -0.03, 0.03 );
            glEnd();
            glColor3f( 0.5, 0.5, 0.5 );
            glBegin( GL_LINE_LOOP );
            glVertex3f( -0.03, -0.03, 0.001 );
            glVertex3f( 0.03, -0.03, 0.001 );
            glVertex3f( 0.03, 0.03, 0.001 );
            glVertex3f( -0.03, 0.03, 0.001 );
            glEnd();
            glEnable( GL_LIGHTING );
        glPopMatrix();
        break;

    default:
        assert( FALSE );
        break;
    }

    if( playing )
    {
        glPopMatrix();
    }

    if( draw_name )
    {
        glDisable( GL_LIGHTING ); 
        glPushMatrix();
        if( color )
            glColor3f( color->r, color->g, color->b );
        else
            glColor3f( 0.0f, 0.0f, 0.0f );
        glTranslatef( .05, -0.01f, 0.0f );
        scaleFont( .02 );
        drawString( temp->name );
        glPopMatrix();
        glEnable( GL_LIGHTING );
    }

    // show if it has pitch/time tables
    if( g_show_qz && ui_temp->core->timetable.size() > 0 )
    {
        float total_width = .08f;
        float total_height = .08f;
        float radius = 0.008f; 
        glLineWidth( 0.5 );
        glColor3f( 0.0f, 0.0f, 0.5f );
        glPushMatrix();
            glTranslatef( total_width/2+radius, total_height/2-radius, 0 );
            glutWireSphere( radius, 4, 4 ); 
        glPopMatrix();
        glLineWidth( 1.0 );
        glColor3f( 0.0f, 0.0f, 0.0f );
    }
    if( g_show_qz && ui_temp->core->pitchtable.size() > 0 )
    {
        float total_width = .08f;
        float total_height = .08f;
        float radius = 0.008f; 
        glLineWidth( 0.5 );
        glColor3f( 0.0f, 0.5f, 0.0f );
        glPushMatrix();
            glTranslatef( total_width/2+radius*3, total_height/2-radius, 0 );
            glutWireSphere( radius, 4, 4 ); 
        glPopMatrix();
        glLineWidth( 1.0 );
        glColor3f( 0.0f, 0.0f, 0.0f );
    }
    
    glPopName();
    glPopMatrix();

    glColor3f( 0.0f, 0.0f, 0.0f );
}


void draw_library( UI_Template * selected, Template * timeline, float x_start, float y_start, 
                  float x_max, float y_max, Spectre * color )
{
    float x = x_start;
    float y = y_start;
    float x_inc = .25f;
    float y_inc = .08f;
    int i;

    glPushMatrix();
    //glTranslatef( -1.05f, -.8f, 0.0f );
    UI_Template * ui_temp;
    Template * temp;
    // draw library
    for( i = 0; i < Library::instance()->templates.size(); i++ )
    {
        // get the template
        ui_temp = Library::instance()->templates[i];
        temp = ui_temp->core;

        // danger
        // make sure it's not a dummy
        if( ui_temp->orig != ui_temp )
            continue;

        // draw template
		if( ui_temp->show ) {
			if( ui_temp->core == timeline ) {
				Spectre colour; 
				colour.r = 0.4f; colour.g = 0.0f; colour.b = 0.0f; 
				draw_template( x, y, ui_temp, true, 0.0f, &colour );
			}
			else
				draw_template( x, y, ui_temp, true, 0.0f, color );
		}

        // show selection
        glPushMatrix();
        glTranslatef( x, y, 0.0f );
        if( selected == ui_temp && ui_temp->show )
        {
            float width = .085f;
            float height = .085f;
            glLineWidth( 2.0 );
            if( color ) glColor3f( color->r, color->g, color->b );
            glBegin( GL_LINE_LOOP );
            glVertex2f( -width/2.0f, -height/2.0f );
            glVertex2f( -width/2.0f, height/2.0f );
            glVertex2f( width/2.0f, height/2.0f );
            glVertex2f( width/2.0f, -height/2.0f );
            glEnd();
            glLineWidth( 1.0 );
            if( color ) glColor3f( 0.0f, 0.0f, 0.0f );
        }
        glPopMatrix();

        x += x_inc;
        if( x >= x_max )
        {
            x = x_start;;
            y += y_inc;
        }
        if( y >= y_max )
        {
            // bad
            break;
        }
    }
    glPopMatrix();
}

// "library" for group face with multiple selections
void draw_group_library( std::vector<bool>& selected, std::vector<UI_Template *>& all, 
						float x_start, float y_start, 
						float x_max, float y_max, Spectre * color )
{
    float x = x_start;
    float y = y_start;
    float x_inc = .25f;
    float y_inc = .08f;
    int i;

    glPushMatrix();
    //glTranslatef( -1.05f, -.8f, 0.0f );
    UI_Template * ui_temp;
    Template * temp;
    // draw library
    for( i = 0; i < all.size(); i++ )
    {
        // get the template
        ui_temp = all[i];
        temp = ui_temp->core;

        // make sure it's not a dummy
        if( ui_temp->orig != ui_temp )
            continue;

        // draw template
		if( ui_temp->show ) {
			draw_template( x, y, ui_temp, true, 0.0f, color );
		}

        // show selection
        glPushMatrix();
        glTranslatef( x, y, 0.0f );
        if( selected[i] && ui_temp->show )
        {
            float width = .085f;
            float height = .085f;
            glLineWidth( 2.0 );
            if( color ) glColor3f( color->r, color->g, color->b );
            glBegin( GL_LINE_LOOP );
            glVertex2f( -width/2.0f, -height/2.0f );
            glVertex2f( -width/2.0f, height/2.0f );
            glVertex2f( width/2.0f, height/2.0f );
            glVertex2f( width/2.0f, -height/2.0f );
            glEnd();
            glLineWidth( 1.0 );
            if( color ) glColor3f( 0.0f, 0.0f, 0.0f );
        }
        glPopMatrix();

        x += x_inc;
        if( x >= x_max )
        {
            x = x_start;;
            y += y_inc;
        }
        if( y >= y_max )
        {
            // bad
            break;
        }
    }
    glPopMatrix();
}



void draw_arrow( Point3D cur_pt, Point3D prev_pt, 
                 Point3D orig_pt, Color4D linecol,
                 Color4D highlight )
{
     // cur_pt  is the current position
     // prev_pt is the previous position.
     // orig_pt is origin of the line;

     static Point3D p1 = Point3D(0,0);
     static Point3D p2 = Point3D(0,0);
     static Point3D p3 = Point3D(0,0);
     static Point3D * pv[3] = { &p1, &p2, &p3 };

     // dont draw arrow if we aren't going anywhere.
     if( !( cur_pt == orig_pt ) )
     {
         Vec2D dir = ( orig_pt - cur_pt ).unit() * 0.08;
         Vec2D orth = Point2D( - dir[1], dir[0] ) * 0.3;

         p1 = cur_pt + dir;
         p2 = p1 + ( prev_pt - cur_pt ) * 2.0 ;
         p3 = orig_pt;

         glLineWidth (2.0);

         // color
         glColor4dv ( linecol.data() );

         glBegin(GL_LINE_STRIP); //0.025
         //draw bezier...
         for ( int ri = 0 ; ri <= 40 ; ri++) 
             glVertex2dv( p1.bezier_interp_pt( (Point3D **)&pv, 3, 0.025 * ri ).data() );
         glEnd();

         glPushMatrix();
         // more danger below
         // glTranslatef( -0.004f, -.01f, 0.0f );
         glBegin( GL_TRIANGLE_FAN );
            glVertex2dv( cur_pt.data());
            glVertex2dv( (p1+orth).data() );

            // highlight
            glColor4dv( highlight.data() );
            glVertex2dv( (p1+ dir*0.25).data() );
            glColor4dv( linecol.data() );
            glVertex2dv( (p1-orth).data() );
         glEnd();
         glPopMatrix();

         glLineWidth( 1.0 );
    }
}

void draw_thing( float x, float y, float spin, UI_Element * ui_temp )
{
    t_TAPBOOL playing = TRUE;

    // shape
    glPushMatrix();
    glTranslatef( x, y, 0.0f );
    glColor3f( 0.5f, 0.1f, 0.3f );
    glBegin( GL_LINES );
    glVertex2f( 0, 0 );
    glEnd();
    glPushName( ui_temp->id );
    if( playing )
    {
        glPushMatrix();
        double val = ::sin( ui_temp->on_where );
        if( val < 0 ) val = -val;
        val = .3 + .4 * val;
        glScalef( val, val, val );
        ui_temp->on_where += .01;
    }

    glPushMatrix();
    glColor3f( .75, 1.0, .5 );
    glRotatef( 45.0f, 1.0f, 1.0f, 1.0f );
    //glutSolidSphere( .03, 10, 10 );
    glutSolidSphere( .13, 10, 10 );
    glColor3f( 0, .5, 0 );
    glRotatef( 90.0f+spin, 1.0f, 0.0f, 0.0f );
    glDisable( GL_LIGHTING );
    //glutWireSphere( .032, 10, 10 );
    glutWireSphere( .132, 10, 10 );
    glEnable( GL_LIGHTING );
    glPopMatrix();

    if( playing )
    {
        glPopMatrix();
    }
    
    glPopName();
    glPopMatrix();

    glColor3f( 0.0f, 0.0f, 0.0f );
}


#ifdef __PLATFORM_LINUX__

t_TAPBOOL g_ok;
// callback
static void cb_ok( GtkDialog * dialog, gint response, gpointer user_data )
{
    g_ok = FALSE;
    gtk_main_quit();
}

#endif


//-----------------------------------------------------------------------------
// name: msg_box()
// desc: ...
//-----------------------------------------------------------------------------
void msg_box( const char * title, const char * msg )
{
    BB_log( BB_LOG_SEVERE, "--ALERT-- %s: %s\n", title, msg );
    
#ifdef __PLATFORM_WIN32__
    MessageBox( NULL, msg, title, MB_OK | MB_TASKMODAL );
#elif defined( __MACOSX_CORE__ )
    if( AudicleWindow::our_fullscreen )
    {
        glutReshapeWindow( AudicleWindow::main()->m_w, AudicleWindow::main()->m_h );
        glutPostRedisplay();
        AudicleWindow::our_fullscreen = FALSE;
        return;
    }

    // from http://www.gamedev.net/reference/articles/article2281.asp
    // TODO: rewrite
//    Str255 strTitle;
//    Str255 strMessage;
//    CopyCStringToPascal( title, strTitle );
//    CopyCStringToPascal( msg, strMessage );
//    StandardAlert( kAlertNoteAlert, strTitle, strMessage, NULL, 0 );
#else    
    if( g_ok ) return;
    g_ok = TRUE;
    
    GtkWidget *dialog, *label;

    dialog = gtk_dialog_new_with_buttons( title, 
                                          NULL,
                                          GTK_DIALOG_DESTROY_WITH_PARENT,
                                          GTK_STOCK_OK,
                                          GTK_RESPONSE_NONE,
                                          NULL );

    label = gtk_label_new( msg );
    gtk_label_set_selectable( GTK_LABEL( label ), TRUE );

    gtk_container_add( GTK_CONTAINER( GTK_DIALOG( dialog )->vbox ), label );
    gtk_widget_show( label );

    g_signal_connect_swapped( dialog, "response", G_CALLBACK( cb_ok ), NULL );
    g_signal_connect_swapped( dialog, "response", G_CALLBACK( gtk_widget_destroy ), (GtkWidget *)dialog );
    gtk_window_set_position( (GtkWindow *)dialog, GTK_WIN_POS_CENTER );
    gtk_widget_show_all( dialog );
    gtk_main();
    
    // wait
    while( g_ok )
        usleep( 30000 );
        
    // gtk_widget_hide( dialog );
    // gtk_widget_destroy( dialog ); // why is this commented out? shouldn't we be destroying this and label too? like:
	gtk_widget_destroy( dialog );
	gtk_widget_destroy( label );

#endif
}



//-----------------------------------------------------------------------------
// name: UI_Template()
// desc: constructor
//-----------------------------------------------------------------------------
UI_Template::UI_Template() : UI_Element() 
{ 
    backup = NULL; 
    core = NULL; 
    orig = this; 
	show = TRUE;
}

//-----------------------------------------------------------------------------
// name: makedummy()
// desc: danger - make dummy
//-----------------------------------------------------------------------------
void UI_Template::makedummy( UI_Template * dummy )
{
    dummy->core = core;
    dummy->backup = backup;
    dummy->orig = orig;
    orig->dummies.push_back( dummy );
    // LOG
    BB_log( BB_LOG_FINE, "makedummy - dummies of %x = %i", orig, orig->dummies.size() ); 
    BB_pushlog(); 
    for( int d = 0; d < orig->dummies.size(); d++ )
        BB_log( BB_LOG_FINEST, "%x", orig->dummies[d] ); 
    BB_poplog();
}

//-----------------------------------------------------------------------------
// name: removedummy()
// desc: danger - remove dummy
//-----------------------------------------------------------------------------
void UI_Template::removedummy( UI_Template * dummy )
{
    assert( dummy->orig == this );
    for( int i = 0; i < dummies.size(); i++ )
    {
        if( dummies[i] == dummy )
        {
            dummies.erase( dummies.begin() + i );
            break;
        }
    }
    // LOG
    BB_log( BB_LOG_FINE, "removedummy - dummies of %x (%x) = %i", this, orig, dummies.size() ); 
    BB_pushlog(); 
    for( int d = 0; d < this->dummies.size(); d++ )
        BB_log( BB_LOG_FINEST, "%x", this->dummies[d] ); 
    BB_poplog();
}

//-----------------------------------------------------------------------------
// name: ~UI_Template()
// desc: danger - destructor. 
//-----------------------------------------------------------------------------
UI_Template::~UI_Template()
{
    // only delete if there are no dummies using its data
    if( orig == this && dummies.empty() )
    {
        if( core != backup )
        {
            SAFE_DELETE( core );
        }
        else
        {
            core = NULL;
        }
        
        SAFE_DELETE( backup );
    }
}

