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
// name: audicle.h
// desc: interface for audicle
//
// authors: Ge Wang (gewang@cs.princeton.edu)
//          Perry R. Cook (prc@cs.princeton.edu)
//          Philip Davidson (philipd@cs.princeton.edu)
//          Ananya Misra (amisra@cs.princeton.edu)
// date: Autumn 2004
//-----------------------------------------------------------------------------
#ifndef __AUDICLE_H__
#define __AUDICLE_H__

#include "audicle_face.h"
#include <vector>




//-----------------------------------------------------------------------------
// name: class Audicle
// desc: ...
//-----------------------------------------------------------------------------
class Audicle
{
public: // how to get the single audicle instance
    static Audicle * instance( );
    static void cleanup( );

public: // initialization and cleanup
    t_TAPBOOL init( );
    t_TAPBOOL shutdown( );

public:
    enum { UP = 0, DOWN = 1, LEFT = 2, RIGHT = 3, BACK = 4 , CUR = 5};
    enum { NO_FACE = 0xffffffff };

public: // look interface
    t_TAPUINT look( t_TAPUINT dir, t_TAPUINT n = 1 );
    t_TAPUINT look_here( );
    t_TAPUINT look_from( t_TAPUINT i, t_TAPUINT dir ) const;     

public: // move interface
    AudicleFace * face( );
    AudicleFace * face( t_TAPUINT index );
    AudicleFace * move( t_TAPUINT dir, t_TAPUINT n = 1 );
    AudicleFace * move_to( t_TAPUINT index );
    AudicleFace * move_to( AudicleFace * face );
    
    
public: // face interface
    t_TAPUINT add( AudicleFace * face );
    t_TAPBOOL swap( t_TAPUINT a, t_TAPUINT b );
    t_TAPBOOL remove( AudicleFace * face );
    t_TAPBOOL remove( t_TAPUINT index );
    
protected: // constructors and destructor
    Audicle( );
    ~Audicle( );
    
protected: // instance
    static Audicle * our_instance;
    
protected: // data
    
    std::vector<AudicleFace *> m_faces;
    std::vector< std::vector<int> > m_matrix;
    t_TAPUINT m_current;
    t_TAPUINT m_last;
    
    t_TAPBOOL m_switching;
    t_TAPUINT m_switch_which;
    t_TAPFLOAT m_switch_start; //time
    t_TAPFLOAT m_switch_last_face;
    
    t_TAPBOOL m_init;
};




#endif
