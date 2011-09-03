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
// name: util_daub.h
// desc: daubechies wavelet transform
//
// authors: Ananya Misra (amisra@cs.princeton.edu)
//          Perry R. Cook (prc@cs.princeton.edu)
//          Numerical Recipes
// date: Autumn 2004
//-----------------------------------------------------------------------------
#ifndef __UTIL_DAUB_H__
#define __UTIL_DAUB_H__


#include <stdlib.h>
#include <stdio.h>
#include <math.h>

float * make_vector(long nl, long nh);

void free_vector(float *v, long nl, long nh);

void wt1(float a[], unsigned long n, int isign,
    void (*wtstep)(float [], unsigned long, int));

void daub4(float a[], unsigned long n, int isign);

void pwtset( int n );

void pwt( float a[], unsigned long n, int isign );


#endif
