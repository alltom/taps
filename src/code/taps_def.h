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
// file: taps_def.h
// desc: based in part on chuck_def.h
//
// author: Ananya Misra (amisra@cs.princeton.edu)
//         Ge Wang (gewang@cs.princeton.edu)
//         Perry R. Cook (prc@cs.princeton.edu)
// date: Autumn 2004
//-----------------------------------------------------------------------------
#ifndef __TAPS_DEF_H__
#define __TAPS_DEF_H__

#include <stdlib.h>
#include <memory.h>


// types
#define t_TAPTIME                   double
#define t_TAPDUR                    double
#define t_TAPFLOAT                  double
#define t_TAPDOUBLE                 double
#define t_TAPSINGLE                 float
#define t_TAPINT                    long
#define t_TAPDWORD                  unsigned long
#define t_TAPUINT                   t_TAPDWORD
#define t_TAPBOOL                   t_TAPDWORD
#define t_TAPBYTE                   unsigned char

typedef char *                      c_str;
typedef const char *                c_constr;

// sample
#define SAMPLE                      float

// dedenormal procedure
#define TAP_DDN                     TAP_DDN_SINGLE

// bool
#ifndef TRUE
#define TRUE                        1
#define FALSE                       0
#endif

// a pie
#define PIE 3.14159265358979323846
#define TWOPIE (2.0*PIE)

#ifndef SAFE_DELETE
#define SAFE_DELETE(x)              { if(x){ delete x; x = NULL; } }
#define SAFE_DELETE_ARRAY(x)        { if(x){ delete [] x; x = NULL; } }
#define SAFE_RELEASE(x)             { if(x){ x->release(); x = NULL; } }
#define SAFE_ADD_REF(x)             { if(x){ x->add_ref(); } }
#define SAFE_REF_ASSIGN(lhs,rhs)    { SAFE_RELEASE(lhs); (lhs) = (rhs); SAFE_ADD_REF(lhs); }
#endif

// dedenormal
#define TAP_DDN_SINGLE(f)           f = ( f >= 0 ? \
        ( ( f > (t_TAPSINGLE)1e-15 && f < (t_TAPSINGLE)1e15 ) ? f : (t_TAPSINGLE)0.0 ) : \
        ( ( f < (t_TAPSINGLE)-1e-15 && f > (t_TAPSINGLE)-1e15 ) ? f : (t_TAPSINGLE)0.0 ) )
#define TAP_DDN_DOUBLE(f)           f = ( f >= 0 ? \
        ( ( f > (t_TAPDOUBLE)1e-15 && f < (t_TAPDOUBLE)1e15 ) ? f : 0.0 ) : \
        ( ( f < (t_TAPDOUBLE)-1e-15 && f > (t_TAPDOUBLE)-1e15 ) ? f : 0.0 ) )

#ifdef __MACOSX_CORE__
#define __PLATFORM_MACOSX__
#endif

#if defined(__LINUX_ALSA__) || defined(__LINUX_JACK__) || defined(__LINUX_OSS__) 
#define __PLATFORM_LINUX__
#endif

#ifdef __PLATFORM_WIN32__
#include <windows.h>
#ifndef usleep
#define usleep(x) Sleep( x / 1000 )
#endif
#pragma warning (disable : 4996)  //stdio deprecation
#pragma warning (disable : 4312)  //type casts from void*
#pragma warning (disable : 4311)  //type casts to void*
#pragma warning (disable : 4244)  //truncation
#pragma warning (disable : 4786) 
#endif

#endif
