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

#include "chuck_def.h"

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


#define EAT_PI 3.14159265358979
#ifndef TWO_PI
#define TWO_PI ( 2 * EAT_PI )
#endif

#ifndef NOMINMAX

#ifndef x_max
#define x_max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef x_min
#define x_min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

#endif  /* NOMINMAX */

#endif
