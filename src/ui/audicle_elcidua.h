//-----------------------------------------------------------------------------
// name: audicle_elcidua.h
// desc: the elciduad
//
// authors: Ge Wang (gewang@cs.princeton.edu)
//          Perry R. Cook (prc@cs.princeton.edu)
//          Philip Davidson (philipd@cs.princeton.edu)
//          Ananya Misra (amisra@cs.princeton.edu)
// date: Autumn 2004
//-----------------------------------------------------------------------------
#ifndef __AUDICLE_ELCIDUA_H__
#define __AUDICLE_ELCIDUA_H__

#include "audicle_def.h"
#include "audicle_face.h"




//-----------------------------------------------------------------------------
// name: class ElciduaFace
// desc: ...
//-----------------------------------------------------------------------------
class ElciduaFace : public AudicleFace
{
public:
    ElciduaFace();
    virtual ~ElciduaFace();

public:
    virtual t_CKBOOL init();
    virtual t_CKBOOL destroy( );

public:
    virtual t_CKUINT render( void * data );
    virtual t_CKUINT on_activate( );
    virtual t_CKUINT on_deactivate( t_CKDUR duration = 0.0 );
    virtual t_CKUINT on_event( const AudicleEvent & event );
};




#endif