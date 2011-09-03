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
// name: audicle_def.h
// desc: ...
//
// authors: Ge Wang (gewang@cs.princeton.edu)
//          Perry R. Cook (prc@cs.princeton.edu)
//          Philip Davidson (philipd@cs.princeton.edu)
//          Ananya Misra (amisra@cs.princeton.edu)
// date: Autumn 2004
//-----------------------------------------------------------------------------
#ifndef __AUDICLE_DEF_H__
#define __AUDICLE_DEF_H__

#include "taps_def.h"

#include <stdio.h>
#include <math.h>
#include <assert.h>
#ifndef __PLATFORM_WIN32__
  #include <unistd.h>
#endif

#include <string>
#include <vector>
#include <map>
#include <fstream>

#ifdef __PLATFORM_WIN32__
#define DM '\\'
#define SYS_BACKSPACE 8
#endif

#ifdef __PLATFORM_MACOSX__
#define DM '/'
#define SYS_BACKSPACE 127
#endif

#ifdef __PLATFORM_LINUX__
#define DM '/'
#define SYS_BACKSPACE 8
#endif

#define SYS_RETURN 13

#ifndef NOMINMAX

#ifndef x_max
#define x_max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef x_min
#define x_min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

#endif  /* NOMINMAX */

#endif
