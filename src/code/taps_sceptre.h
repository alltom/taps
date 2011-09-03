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
// file: taps_sceptre.h
// desc: taps fft
//
// author: Ananya Misra (amisra@cs.princeton.edu)
//         Ge Wang (gewang@cs.princeton.edu)
//         Perry R. Cook (prc@cs.princeton.edu)
// date: Autumn 2004
//-----------------------------------------------------------------------------
#ifndef __SCEPTRE_H__
#define __SCEPTRE_H__

#include "taps_birdbrain.h"

#define FFT_FORWARD 1
#define FFT_INVERSE 0
#define make_window hanning

// make the window
void hanning( float * window, unsigned long length );
void hamming( float * window, unsigned long length );
void blackman( float * window, unsigned long length );
void rectangular( float * window, unsigned long length );
void triangular( float * window, unsigned long length );
void hanna( float * window, unsigned long length ); // from dafx-02
void squareroot( float * window, unsigned long length );
void logarithmic( float * window, unsigned long length );
// apply the window
void apply_window( float * data, float * window, unsigned long length );

// real fft, N must be power of 2
void rfft( float * x, long N, unsigned int forward );
// complex fft, NC must be power of 2
void cfft( float * x, long NC, unsigned int forward );

// bandpass filter the samples x - pass through frequencies between
// bandStart * nyquist and bandEnd * nyquist unharmed, smoothly sloping down
// the amplitudes of frequencies outside the passband to 0 over
// rolloff * nyquist hz. (MDH)
void fft_bandpass( Frame * x, float bandStart, float bandEnd, float rolloff );


#endif

