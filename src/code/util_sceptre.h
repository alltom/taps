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