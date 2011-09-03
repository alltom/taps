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
// file: taps_sceptre.cpp
// desc: taps fft
//
// author: Ananya Misra (amisra@cs.princeton.edu)
//         Ge Wang (gewang@cs.princeton.edu)
//         Perry R. Cook (prc@cs.princeton.edu)
// date: Autumn 2004
//-----------------------------------------------------------------------------
#include "taps_sceptre.h"




//-----------------------------------------------------------------------------
// name: hanning()
// desc: make window
//-----------------------------------------------------------------------------
void hanning( float * window, unsigned long length )
{
    unsigned long i;
    double phase = 0, delta;

    delta = 2 * PIE / (double) length;

    for( i = 0; i < length; i++ )
    {
        window[i] = (float)(0.5 * (1.0 - cos(phase)));
        phase += delta;
    }
}




//-----------------------------------------------------------------------------
// name: hamming()
// desc: make window
//-----------------------------------------------------------------------------
void hamming( float * window, unsigned long length )
{
    unsigned long i;
    double phase = 0, delta;

    delta = 2 * PIE / (double) length;

    for( i = 0; i < length; i++ )
    {
        window[i] = (float)(0.54 - .46*cos(phase));
        phase += delta;
    }
}



//-----------------------------------------------------------------------------
// name: blackman()
// desc: make window
//-----------------------------------------------------------------------------
void blackman( float * window, unsigned long length )
{
    unsigned long i;
    double phase = 0, delta;

    delta = 2 * PIE / (double) length;

    for( i = 0; i < length; i++ )
    {
        window[i] = (float)(0.42 - .5*cos(phase) + .08*cos(2*phase));
        phase += delta;
    }
}




//-----------------------------------------------------------------------------
// name: rectangular()
// desc: make window
//-----------------------------------------------------------------------------
void rectangular( float * window, unsigned long length )
{
    unsigned long i;
    for( i = 0; i < length; i++ )
        window[i] = 1.0;
}



//-----------------------------------------------------------------------------
// name: triangular()
// desc: make window
//-----------------------------------------------------------------------------
void triangular( float * window, unsigned long length )
{
    unsigned long i;
    float dy, dx;
    dy = 1.0f;
    dx = length/2;
    for( i = 0; i <= length/2; i++ )
        window[i] = i * dy/dx;
    dy = -1.0f;
    dx = length - length/2;
    for( i = 0; i < dx; i++ )
        window[length/2 + i] = window[length/2] + i * dy/dx;
}


//-----------------------------------------------------------------------------
// name: squareroot()
// desc: make window
//-----------------------------------------------------------------------------
void squareroot( float * window, unsigned long length )
{
	unsigned long i;
	float dy, dx;
	dy = 1.0f;
	dx = length/2;
	for( i = 0; i < length/2; i++ )
		window[i] = window[length-1-i] = sqrt(i*dy/dx);
//		window[i] = window[length-1-i] = -4*::pow(1.0*i/length-0.5,2)+1; // parabola
}


//-----------------------------------------------------------------------------
// name: logarithmic()
// desc: make window
//-----------------------------------------------------------------------------
void logarithmic( float * window, unsigned long length )
{
	unsigned long i;
	float dy, dx;
	dy = 1.0f;
	dx = length/2;
	for( i = 0; i < length/2; i++ )
		window[i] = window[length-1-i] = log(i*dy/dx*(exp(1)-1)+1);
//		window[i] = window[length-1-i] = -4*::pow(1.0*i/length-0.5,2)+1; // parabola
}


//-----------------------------------------------------------------------------
// name: hanna() (pierre hanna)
// desc: make window
// http://www2.hsu-hh.de/ant/dafx2002/papers/DAFX02_Hanna_Desainte-Catherine_overlap_add_noise_synthesis.pdf
//-----------------------------------------------------------------------------
void hanna( float * window, unsigned long length )
{
	// sin(pi*n/(N-1))
	unsigned long i;
	for( i = 0; i < length; i++ )
	{
		window[i] = sin( (PIE * i) / (length - 1) );
	}
}


//-----------------------------------------------------------------------------
// name: apply_window()
// desc: apply a window to data
//-----------------------------------------------------------------------------
void apply_window( float * data, float * window, unsigned long length )
{
    unsigned long i;

    for( i = 0; i < length; i++ )
        data[i] *= window[i];
}


void bit_reverse( float * x, long N );

//-----------------------------------------------------------------------------
// name: rfft()
// desc: real value fft
//
//   these routines from the CARL software, spect.c
//   check out the CARL CMusic distribution for more source code
//
//   if forward is true, rfft replaces 2*N real data points in x with N complex 
//   values representing the positive frequency half of their Fourier spectrum,
//   with x[1] replaced with the real part of the Nyquist frequency value.
//
//   if forward is false, rfft expects x to contain a positive frequency 
//   spectrum arranged as before, and replaces it with 2*N real values.
//
//   N MUST be a power of 2.
//
//-----------------------------------------------------------------------------
void rfft( float * x, long N, unsigned int forward )
{
    float c1, c2, h1r, h1i, h2r, h2i, wr, wi, wpr, wpi, temp, theta ;
    float xr, xi ;
    long i, i1, i2, i3, i4, N2p1 ;

    theta = PIE/N ;
    wr = 1. ;
    wi = 0. ;
    c1 = 0.5 ;

    if( forward )
    {
        c2 = -0.5 ;
        cfft( x, N, forward ) ;
        xr = x[0] ;
        xi = x[1] ;
    }
    else
    {
        c2 = 0.5 ;
        theta = -theta ;
        xr = x[1] ;
        xi = 0. ;
        x[1] = 0. ;
    }
    
    wpr = (float) (-2.*pow( sin( 0.5*theta ), 2. )) ;
    wpi = (float) sin( theta ) ;
    N2p1 = (N<<1) + 1 ;
    
    for( i = 0 ; i <= N>>1 ; i++ )
    {
        i1 = i<<1 ;
        i2 = i1 + 1 ;
        i3 = N2p1 - i2 ;
        i4 = i3 + 1 ;
        if( i == 0 )
        {
            h1r =  c1*(x[i1] + xr ) ;
            h1i =  c1*(x[i2] - xi ) ;
            h2r = -c2*(x[i2] + xi ) ;
            h2i =  c2*(x[i1] - xr ) ;
            x[i1] =  h1r + wr*h2r - wi*h2i ;
            x[i2] =  h1i + wr*h2i + wi*h2r ;
            xr =  h1r - wr*h2r + wi*h2i ;
            xi = -h1i + wr*h2i + wi*h2r ;
        }
        else
        {
            h1r =  c1*(x[i1] + x[i3] ) ;
            h1i =  c1*(x[i2] - x[i4] ) ;
            h2r = -c2*(x[i2] + x[i4] ) ;
            h2i =  c2*(x[i1] - x[i3] ) ;
            x[i1] =  h1r + wr*h2r - wi*h2i ;
            x[i2] =  h1i + wr*h2i + wi*h2r ;
            x[i3] =  h1r - wr*h2r + wi*h2i ;
            x[i4] = -h1i + wr*h2i + wi*h2r ;
        }

        wr = (temp = wr)*wpr - wi*wpi + wr ;
        wi = wi*wpr + temp*wpi + wi ;
    }

    if( forward )
        x[1] = xr ;
    else
        cfft( x, N, forward ) ;
}




//-----------------------------------------------------------------------------
// name: cfft()
// desc: complex value fft
//
//   these routines from CARL software, spect.c
//   check out the CARL CMusic distribution for more software
//
//   cfft replaces float array x containing NC complex values (2*NC float 
//   values alternating real, imagininary, etc.) by its Fourier transform 
//   if forward is true, or by its inverse Fourier transform ifforward is 
//   false, using a recursive Fast Fourier transform method due to 
//   Danielson and Lanczos.
//
//   NC MUST be a power of 2.
//
//-----------------------------------------------------------------------------
void cfft( float * x, long NC, unsigned int forward )
{
    float wr, wi, wpr, wpi, theta, scale ;
    long mmax, ND, m, i, j, delta ;
    ND = NC<<1 ;
    bit_reverse( x, ND ) ;
    
    for( mmax = 2 ; mmax < ND ; mmax = delta )
    {
        delta = mmax<<1 ;
        theta = TWOPIE/( forward? mmax : -mmax ) ;
        wpr = (float) (-2.*pow( sin( 0.5*theta ), 2. )) ;
        wpi = (float) sin( theta ) ;
        wr = 1. ;
        wi = 0. ;

        for( m = 0 ; m < mmax ; m += 2 )
        {
            register float rtemp, itemp ;
            for( i = m ; i < ND ; i += delta )
            {
                j = i + mmax ;
                rtemp = wr*x[j] - wi*x[j+1] ;
                itemp = wr*x[j+1] + wi*x[j] ;
                x[j] = x[i] - rtemp ;
                x[j+1] = x[i+1] - itemp ;
                x[i] += rtemp ;
                x[i+1] += itemp ;
            }

            wr = (rtemp = wr)*wpr - wi*wpi + wr ;
            wi = wi*wpr + rtemp*wpi + wi ;
        }
    }

    // scale output
    scale = (float)(forward ? 1./ND : 2.) ;
    {
        register float *xi=x, *xe=x+ND ;
        while( xi < xe )
            *xi++ *= scale ;
    }
}



//-----------------------------------------------------------------------------
// name: bit_reverse()
// desc: bitreverse places float array x containing N/2 complex values
//       into bit-reversed order
//-----------------------------------------------------------------------------
void bit_reverse( float * x, long N )
{
    float rtemp, itemp ;
    long i, j, m ;
    for( i = j = 0 ; i < N ; i += 2, j += m )
    {
        if( j > i )
        {
            rtemp = x[j] ; itemp = x[j+1] ; /* complex exchange */
            x[j] = x[i] ; x[j+1] = x[i+1] ;
            x[i] = rtemp ; x[i+1] = itemp ;
        }

        for( m = N>>1 ; m >= 2 && j >= m ; m >>= 1 )
            j -= m ;
    }
}



//-----------------------------------------------------------------------------
// name: fft_bandpass()
// desc: bandpass filter the samples x - pass through frequencies between
//       bandStart * nyquist and bandEnd * nyquist unharmed, smoothly sloping down
//       the amplitudes of frequencies outside the passband to 0 over
//       rolloff * nyquist hz.
//-----------------------------------------------------------------------------
void fft_bandpass( Frame * x, float bandStart, float bandEnd, float rolloff )
{
    float *window = 0;
    int winsize = -1;

    int i;

    // not sure why we have at least 4 "pi" variables floating around
    // (#define PIE, #define PI, float PI earlier in this file, double pi local to functions)
    // got to combine them all some time. 
    // double pi = 4.*atan(1.0); 
    
    // make and apply hanning window
    if( winsize != x->wlen ){
        winsize = x->wlen;
        if(window)
            delete[] window;
  
        window = new float[winsize];
        hanning( window, winsize );
    }
    
    apply_window( x->waveform, window, x->wlen );

    // fft
    rfft( x->waveform, x->len, FFT_FORWARD );
    BirdBrain::scale_fft( x->waveform, x->wlen, x->wlen, x->wsize );
    x->cmp2pol();

    // the core
    int rollWidth = (int)(rolloff * (float)x->len);
    int startIndex = (int)(bandStart * (float)x->len);
    int endIndex = (int)(bandEnd * (float)x->len);
    int lTail = startIndex - rollWidth;
    int rTail = endIndex + rollWidth;
    for(i = 0; i < lTail; i++){
        x->pol[i].mag = 0;
    }
    for(i = lTail > 0 ? lTail : 0; i < startIndex; i++){
        x->pol[i].mag *= 0.5 *
            (1 + cos(((float)(startIndex - i)) * PIE / (float)rollWidth));
    }
    for(i = endIndex; (i < rTail) && (i < x->len); i++){
        x->pol[i].mag *= 0.5 *
            (1 + cos(((float)(i - endIndex)) * PIE / (float)rollWidth));
    } 
    for(i = rTail; i < x->len; i++){
        x->pol[i].mag = 0;
    }
  
    x->pol2cmp();
    
    // inverse fft
    rfft( x->waveform, x->len, FFT_INVERSE);
   
    // inverse window!
    hanning( window, winsize );
    for(i = 0; i < winsize; i++){
        if(window[i] < 0.15)
            window[i] = 0.15f;
            window[i] = 1 / window[i];
    }
    apply_window( x->waveform, window, x->wlen );
    if( window )
        delete[] window;
}

