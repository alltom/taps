//-----------------------------------------------------------------------------
// name: audicle_elcidua.cpp
// desc: the elcidua
//
// authors: Ge Wang (gewang@cs.princeton.edu)
//          Perry R. Cook (prc@cs.princeton.edu)
//          Philip Davidson (philipd@cs.princeton.edu)
//          Ananya Misra (amisra@cs.princeton.edu)
// date: Autumn 2004
//-----------------------------------------------------------------------------
#include "audicle_elcidua.h"
#include "audicle_gfx.h"
#include "audicle.h"




//-----------------------------------------------------------------------------
// name: ElciduaFace()
// desc: ...
//-----------------------------------------------------------------------------
ElciduaFace::ElciduaFace( ) : AudicleFace( ) { }




//-----------------------------------------------------------------------------
// name: ~ElciduaFace()
// desc: ...
//-----------------------------------------------------------------------------
ElciduaFace::~ElciduaFace( ) { }




//-----------------------------------------------------------------------------
// name: init()
// desc: ...
//-----------------------------------------------------------------------------
t_CKBOOL ElciduaFace::init( )
{
    if( !AudicleFace::init() )
        return FALSE;

    return TRUE;
}




//-----------------------------------------------------------------------------
// name: destroy()
// desc: ...
//-----------------------------------------------------------------------------
t_CKBOOL ElciduaFace::destroy( )
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
t_CKUINT ElciduaFace::render( void * data )
{
    scaleFont( 0.2, 1.2 );
    drawString_centered("devil ElCidua lived!");

    return TRUE;
}




//-----------------------------------------------------------------------------
// name: on_activate()
// desc: ...
//-----------------------------------------------------------------------------
t_CKUINT ElciduaFace::on_activate()
{
    return AudicleFace::on_activate();
}




//-----------------------------------------------------------------------------
// name: on_deactivate()
// desc: ...
//-----------------------------------------------------------------------------
t_CKUINT ElciduaFace::on_deactivate( t_CKDUR dur )
{
    return AudicleFace::on_deactivate( dur );
}




//-----------------------------------------------------------------------------
// name: on_event()
// desc: ...
//-----------------------------------------------------------------------------
t_CKUINT ElciduaFace::on_event( const AudicleEvent & event )
{
 //   Audicle::instance()->move_to( (t_CKUINT)0 );
    return AudicleFace::on_event( event );
}
