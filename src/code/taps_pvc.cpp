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
// name: taps_pvc.h
// desc: phase vocoder
//
// authors: Ananya Misra (amisra@cs.princeton.edu)
//          Ge Wang (gewang@cs.princeton.edu)
//          Ahmed Abdallah (aabdalla@princeton.edu)
//          Paul Botelho (pbotelho@princeton.edu)
// date: Spring 2004
//-----------------------------------------------------------------------------
#include "taps_pvc.h"
#include "audicle_def.h"
#include <assert.h>
#include <memory.h>
#include <queue>
using namespace std;


#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif




// internal pvc data
/*struct pvc_data
{
    t_TAPUINT window_size;
    SAMPLE * window[2];
    t_TAPUINT which;
    SAMPLE * the_window;
    queue<polar_window *> windows;
    t_TAPUINT data_size;

    t_TAPUINT io_size;
    t_TAPUINT count;
    SAMPLE * ola[2];
    t_TAPUINT index;
    queue<SAMPLE *> ready;
    float K;

    polar * space;
    SAMPLE * space2;

    t_TAPUINT pool;
    CBuffer polar_pool;
    CBuffer win_pool;
};*/




//-----------------------------------------------------------------------------
// name: pv_create()
// desc: ...
//-----------------------------------------------------------------------------
PVC::PVC( t_TAPUINT window_size, t_TAPUINT io_size, t_TAPUINT pool_size )
{
    _window[0] = new SAMPLE[(window_size + io_size)*2];
    _window[1] = new SAMPLE[(window_size + io_size)*2];
    _the_window = new SAMPLE[window_size];
    _space = new polar[window_size/2];
    _space2 = new SAMPLE[window_size];
    _window_size = window_size;
    _data_size = 0;
    _which = 0;
    _windows = new win_queue;

    _io_size = io_size;
    _count = 0;
    _ola[0] = new SAMPLE[(window_size + io_size)*2];
    _ola[1] = new SAMPLE[(window_size + io_size)*2];
    _index = 0;
    
    memset( _ola[0], 0, (window_size + io_size)*2*sizeof(SAMPLE) );
    memset( _ola[1], 0, (window_size + io_size)*2*sizeof(SAMPLE) );

    _pool = pool_size;
    if( _pool && _pool < 256 )
        _pool = 256;

    if( _pool )
    {
        _polar_pool.initialize( _pool, sizeof(polar_window *) );
        _win_pool.initialize( _pool * 4, sizeof(SAMPLE *) );

        int i;
        polar_window * w;
        SAMPLE * a;
        for( i = 1; i < _pool * 3 / 4; i++ )
        {
            w = new polar_window(window_size/2);
            _polar_pool.put( &w, 1 );
        }
        for( i = 1; i < _pool * 3; i++ )
        {
            a = new SAMPLE[io_size]; // was window_size and crashed when io_size > window_size
            _win_pool.put( &a, 1 );
        }
    }

    // make the window
    pv_set_window( PV_HANNING );

    // phase_inc...?
    _phase_inc = new float[window_size / 2]; 
}


//-----------------------------------------------------------------------------
// name: pv_destroy()
// desc: ...
//-----------------------------------------------------------------------------
PVC::~PVC()
{
    SAFE_DELETE_ARRAY( _window[0] );
    SAFE_DELETE_ARRAY( _window[1] );
    SAFE_DELETE_ARRAY( _ola[0] );
    SAFE_DELETE_ARRAY( _ola[1] );
    SAFE_DELETE_ARRAY( _space );
    SAFE_DELETE_ARRAY( _space2 );

    polar_window * w;
    SAMPLE * a;

    if( _pool )
    {
        while( _polar_pool.get( &w, 1 ) )
        {
            SAFE_DELETE( w );
        }
        while( _win_pool.get( &a, 1 ) )
        {
            SAFE_DELETE_ARRAY( a );
        }
    }

    // could be windows left in queues
    /*while( _windows->size() )
    {
        w = _windows->dequeue();
        SAFE_DELETE( w );
    }*/
    while( _ready.size() )
    {
        a = _ready.front();
        SAFE_DELETE_ARRAY( a );
        _ready.pop();
    }
    
    SAFE_DELETE( _windows );
    SAFE_DELETE_ARRAY( _phase_inc ); 
}

//-----------------------------------------------------------------------------
// name: pv_clear()
// desc: clear most things
//-----------------------------------------------------------------------------
void PVC::pv_clear()
{
    memset( _window[0], 0, (_window_size + _io_size)*2*sizeof(SAMPLE) ); 
    memset( _window[1], 0, (_window_size + _io_size)*2*sizeof(SAMPLE) );
    memset( _space, 0, _window_size/2*sizeof(SAMPLE) );
    memset( _space2, 0, _window_size/2*sizeof(SAMPLE) );
    _data_size = 0;
    _which = 0;
    _count = 0;

    memset( _ola[0], 0, (_window_size + _io_size)*2*sizeof(SAMPLE) );
    memset( _ola[1], 0, (_window_size + _io_size)*2*sizeof(SAMPLE) );
    _index = 0;
    
    polar_window * w;
    SAMPLE * a;

    // could be windows left in queues
    while( _windows->size() )
    {
        w = _windows->dequeue();
        SAFE_DELETE( w );
    }
    while( _ready.size() )
    {
        a = _ready.front();
        SAFE_DELETE_ARRAY( a );
        _ready.pop();
    }

    // phase_inc...?
    memset( _phase_inc, 0, _window_size/2*sizeof(float) ); 
}


//-----------------------------------------------------------------------------
// name: pv_analyze()
// desc: ...
//-----------------------------------------------------------------------------
bool PVC::pv_analyze( SAMPLE * buffer, t_TAPUINT hop_size )
{
    assert( hop_size <= _window_size );
    
    t_TAPUINT to_copy = 0;
    complex * cmp = NULL;
    int i;
    t_TAPUINT len = _window_size / 2;
    SAMPLE * w = _window[_which];
    _which = !_which;
    SAMPLE * w2 = _window[_which];

    // copy
    memcpy( w + _data_size, buffer, _io_size * sizeof(SAMPLE) );
    _data_size += _io_size;

    // one window
    while( _data_size >= _window_size )
    {
        polar_window * p = NULL;
        // analyze
        if( _pool )
        {
            if( !_polar_pool.get( &p, 1 ) )
            {
                BB_log( BB_LOG_SYSTEM, "PVC: pool exhausted! (out of memory)" );
                return false;
            }
        }
        else
            p = new polar_window( _window_size / 2 );

        memcpy( w2, w, _window_size * sizeof(SAMPLE) );
        apply_window( w2, _the_window, _window_size );
        signal_swap( w2, _window_size ); // seemingly useless
        rfft( w2, _window_size / 2, FFT_FORWARD );
        cmp = (complex *)w2;
        for( i = 0; i < len; i++ )
        {
            p->array[i].mag = cmp_mag(cmp[i]);
            p->array[i].phase = cmp_phase(cmp[i]);
        }
        // make a copy
        memcpy( p->old, p->array, p->len * sizeof(polar) );

        // queue
        _windows->push( p );
        _data_size -= hop_size;
        w += hop_size;
    }

    memcpy( w2, w, _data_size * sizeof(SAMPLE) );
    _hop_size = hop_size;
    
    return true;
}




//-----------------------------------------------------------------------------
// name: pv_set_window()
// desc: ...
//-----------------------------------------------------------------------------
void PVC::pv_set_window( t_TAPUINT type )
{
    if( type == PV_HAMMING )
        hamming( _the_window, _window_size );
    else if( type == PV_BLACKMAN )
        blackman( _the_window, _window_size );
    else if( type == PV_HANNING )
        hanning( _the_window, _window_size );
    else
        rectangular( _the_window, _window_size );

    // find sum
    _K = 0.0f;
    for( int i = 0; i < _window_size; i++ )
        _K += _the_window[i] * _the_window[i];
}




//-----------------------------------------------------------------------------
// name: pv_front()
// desc: ...
//-----------------------------------------------------------------------------
polar_window * PVC::pv_front()
{
    if( _windows->size() > 0 )
        return _windows->front();
    
    return NULL;
}




//-----------------------------------------------------------------------------
// name: pv_deque()
// desc: ...
//-----------------------------------------------------------------------------
polar_window * PVC::pv_deque()
{
    polar_window * p = NULL;

    if( _windows->size() > 0 )
    {
        p = _windows->dequeue();
    }

    return p;
}



//-----------------------------------------------------------------------------
// name: pv_unwrap_phase()
// desc: ...
//-----------------------------------------------------------------------------
void PVC::pv_unwrap_phase( polar_window * window )
{
return;
    t_TAPUINT len = window->len;
    polar * p = (polar *)window->array;
    t_TAPUINT i;

#if 1
    float a, b;

    // clip phase in bin 0
    p[0].phase = ::fmod( p[0].phase, PIE );

    // close gap for large discontinuities
    for( i = 1; i < len; i++ )
    {
        a = p[i].phase;
        b = p[i-1].phase;
        
        while( (a - b) > PIE )
        {
            a -= 2 * (float)PIE;
        }
        
        while( (b - a) > PIE )
        {
            a += 2 * (float)PIE;
        }
        
        p[i].phase = a;
    }

#else
    float x;

    for( i = 0; i < len; i++ )
    {
        x = floor( fabs( p[i].phase / PIE ) );
        if( p[i].phase < 0.0f ) x *= -1.0f;
        p[i].phase -= x * PIE;
    }

#endif

}


//------------------------------------------------------------------------------------------------
// name: pv_unwrap_phase_
// desc: yet another attempt, from same source as princarg
//------------------------------------------------------------------------------------------------
void PVC::pv_unwrap_phase_( const polar_window * prev, polar_window * curr )
{
    t_TAPUINT len = curr->len;
    const polar * pold = prev ? prev->old : NULL;
    const polar * cold = curr->old;
    float omega, delta_phi;

    for( int i = 0; i < len; i++ )
    {
        // omega: nominal phase increment for the analysis hop size for each bin
        omega = /*2**/ PIE * _hop_size * i / len; // freq is derivative of phase, so add hopsize * freq to get new phase
        delta_phi = princarg( cold[i].phase - ( pold ? pold[i].phase : 0 ) - omega );
        _phase_inc[i] = (omega + delta_phi); // phase increment for each bin over _hop_size 
    }
}

//------------------------------------------------------------------------------------------------
// name: princarg()
// desc: protected member function
//       from http://www.cs.princeton.edu/courses/archive/spring05/cos325/Bernardini.pdf
//       02 Jan 2006
//       "returns the principal argument of the nominal initial phase of each frame" for each bin.
//       actually probably returns principal argument of the phase passed in as phasein.
//       no builtin function for this? hmm (may also look vaguely familiar from earliest
//          version of unwrap_phase)
//-------------------------------------------------------------------------------------------------
float PVC::princarg( float phasein )
{
// function has been hijacked
/*  while( phasein > PIE )
        phasein -= 2 * (float)PIE;
    while( phasein <= -PIE )
        phasein += 2 * (float)PIE;
    return phasein;
// end of hijacking (and function)
*/  
    // divide by 2 pi
    float a = fabs( phasein / (2 * PIE) );
    // round
    float k = floor(a + 0.5f); 
    if( phasein < 0.0f ) 
        k *= -1.0f;
    // return phasein - round(phasein/2pi)*2pi
    return phasein - k * 2 * PIE; 
}

//------------------------------------------------------------------------------------------------
// name: signal_swap()
// desc: protected member function
//       supposedly fft shifting, but just swapping left and right halves of time-domain signal
//-------------------------------------------------------------------------------------------------
void PVC::signal_swap( SAMPLE * sig, t_TAPUINT length )
{
    SAMPLE temp;
    for( int i = 0; i < length/2; i++ )
    {
        temp = sig[i]; 
        sig[i] = sig[i + length/2]; 
        sig[i + length/2] = temp;
    }
}

//-----------------------------------------------------------------------------
// name: pv_phase_fix_()
// desc: yet another attempt, from same source as princarg
//-----------------------------------------------------------------------------
void PVC::pv_phase_fix_( const polar_window * prev, polar_window * curr, float time_stretch )
{
    if( !prev )
    {
        if( curr )
            for( int i = 0; i < curr->len; i++ )
                curr->array[i].phase = curr->old[i].phase; 
        return; 
    }

    for( int i = 0; i < curr->len; i++ )
    {
		t_TAPUINT hop = (t_TAPUINT)(time_stretch * _hop_size + .5f);
		time_stretch = 1.0 * hop / _hop_size;
        curr->array[i].phase = prev->array[i].phase + time_stretch * _phase_inc[i]; 
        // time_stretch * phase_inc gives phase increment over synthesis hop size
    }   
    return;
}


//-----------------------------------------------------------------------------
// name: pv_phase_fix()
// desc: ...
//-----------------------------------------------------------------------------
void PVC::pv_phase_fix( const polar_window * prev, polar_window * curr, float factor )
{
    // make sure not NULL
    if( !prev ) return;

    t_TAPUINT len = curr->len;
    const polar * parr = prev->array;
    const polar * pold = prev->old;
    const polar * cold = curr->old;
    polar * carr = curr->array;

#if 1

    float a, b;
    // close gap for large discontinuities
    for( int i = 0; i < len; i++, parr++, pold++, cold++, carr++ )
    {
        a = cold->phase;
        b = pold->phase;        

        while( (a - b) > PIE )
        {
            a -= 2 * (float)PIE;
        }
        
        while( (b - a) > PIE )
        {
            a += 2 * (float)PIE;
        }
        
        carr->phase = factor * (a - b) + parr->phase;

        a = carr->phase;
        b = parr->phase;        

        while( (a - b) > 2 * PIE )
        {
            a -= 2 * (float)PIE;
        }
        
        while( (b - a) > 2 * PIE )
        {
            a += 2 * (float)PIE;
        }
  
        carr->phase = a;

        carr->phase = ::fmod( carr->phase, 2 * PIE );

        //while( c->phase > PIE ) c->phase -= 2 * PIE;
        //mwhile( c->phase < -PIE ) c->phase += 2 * PIE;
    }
    
#else

    int i;
    fprintf( stderr, "-----------------------------------------\n" );
    for (i = 0; i < len; i++, p++, r++, c++ )
    {
        fprintf( stderr, "diff: %f, %f\n", (c->phase - r->phase), p->phase );
        c->phase = factor * (c->phase - r->phase) + p->phase;
//            if( fabs( c->phase ) > 3.1415 )
//                fprintf( stderr, "%i %f\n", i, c->phase );
    }

#endif
}




//-----------------------------------------------------------------------------
// name: pv_freq_shift()
// desc: ...
//-----------------------------------------------------------------------------
void PVC::pv_freq_shift( polar_window * window, float factor )
{
    t_TAPUINT len = window->len, n, floor, ceiling;
    polar * p = window->old; //window->array;
    polar * parr = window->array;
    polar * q = _space;
    float x; // delta, alpha;

    // resample the frequency domain
    for( n = 0; n < len; n++ )
    {
        // index to sample
        x = (float)n/factor;
        // find the neighbors
        floor = (t_TAPUINT)x;
        ceiling = (t_TAPUINT)(x+.99);
        // interpolate
        if( ceiling < len )
        {
            if( floor == ceiling )
                q[n] = p[floor];
            else
            {
                q[n].mag = p[floor].mag +
                    ((float)x-floor)*(p[ceiling].mag-p[floor].mag);
                q[n].phase = p[floor].phase +
                    ((float)x-floor)*(p[ceiling].phase-p[floor].phase);
            }
        }
        else
        {
            q[n].mag = 0.0f;
            q[n].phase = 0.0f;
        }

        q[n].mag /= (float)sqrt(factor);
        q[n].phase *= factor;
    }

    // copy
    memcpy( parr, q, len * sizeof(polar) );
}




//-----------------------------------------------------------------------------
// name: pv_cross_synth()
// desc: ...
//-----------------------------------------------------------------------------
void PVC::pv_cross_synth( polar_window * input, const polar_window * filter )
{
    const polar * p = (const polar *)filter->array;
    polar * q = input->array;
    int i;

    assert( input->len == filter->len );
    for( i = 0; i < input->len; i++ )
    {
        q[i].mag *= p[i].mag * input->len;
        q[i].phase += p[i].phase;
    }

    memcpy( input->old, input->array, input->len * sizeof(polar) );
}




//-----------------------------------------------------------------------------
// name: pv_ifft()
// desc: ...
//-----------------------------------------------------------------------------
void PVC::pv_ifft( const polar_window * window, SAMPLE * buffer )
{
    complex * cmp = (complex *)buffer;
    const polar * p = (const polar *)window->array;
    int i;

    for( i = 0; i < window->len; i++ )
    {
        cmp[i].re = p[i].mag * cos( p[i].phase );
        cmp[i].im = p[i].mag * sin( p[i].phase );
    }

    rfft( (float *)cmp, window->len, FFT_INVERSE );
}




//-----------------------------------------------------------------------------
// name: pv_overlap_add()
// desc: ...
//-----------------------------------------------------------------------------
bool PVC::pv_overlap_add( polar_window * the_window, t_TAPUINT hop_size )
{
//    assert( hop_size <= _window_size );
    if( hop_size > _window_size/2 ) hop_size = _window_size/2;

    t_TAPUINT to_ola = _window_size - hop_size;
    SAMPLE * window = _space2;
    SAMPLE * w = _ola[_index]; // length (_window_size + _io_size)*2 samples
    _index = !_index;
    SAMPLE * w2 = _ola[_index];
    int i;

    // ifft
    pv_ifft( the_window, window );
    
    // shift
    signal_swap( window, _window_size ); // seemingly useless

    // window
    apply_window( window, _the_window, _window_size ); // seemingly useless

    // overlap add
    SAMPLE * x = &w[_count];
    for( i = 0; i < to_ola; i++ )
        x[i] += window[i];

    // copy
    memcpy( x+to_ola, window+to_ola, hop_size * sizeof(SAMPLE) );

    // queue
    _count += hop_size;
    while( _count >= _io_size )
    {
        SAMPLE * buffer = NULL;
        if( _pool )
        {
            if( !_win_pool.get( &buffer, 1 ) )
            {
                BB_log( BB_LOG_SYSTEM, "PVC: pool exhausted! (out of memory)" );
                return false;
            }
        }
        else
            buffer = new SAMPLE[_io_size];
        memcpy( buffer, w, _io_size * sizeof(SAMPLE) );
        // scale output
        float R = _K / hop_size;
        for( i = 0; i < _io_size; i++ )
            buffer[i] /= R;
        // push into ready queue
        _ready.push( buffer );

        _count -= _io_size;
        w += _io_size;
    }

    memset( w2, 0, (_window_size + _io_size)*2*sizeof(SAMPLE) );
    memcpy( w2, w, (_window_size + _count) * sizeof(SAMPLE) );
    
    return true;
}


//-----------------------------------------------------------------------------
// name: pv_copy()
// desc: pretend to copy this to a new pvc, but actually just return a new one
//-----------------------------------------------------------------------------
PVC * PVC::pv_copy()
{
    return new PVC( _window_size, _io_size, _pool ); 
}


//-----------------------------------------------------------------------------
// name: pv_synthesize()
// desc: ...
//-----------------------------------------------------------------------------
t_TAPUINT PVC::pv_synthesize( SAMPLE * buffer )
{
    if( _ready.size() == 0 )
        return FALSE;

    // deque
    memcpy( buffer, _ready.front(), _io_size * sizeof(SAMPLE) );
    pv_reclaim2( _ready.front() );
    _ready.pop();

    return TRUE;
}



t_TAPUINT PVC::pv_get_ready_len()
{
    return (t_TAPUINT)_ready.size();
}


void PVC::pv_reclaim( polar_window * window )
{
    if( !window ) return;
    if( _pool )
        _polar_pool.put( &window, 1 );
    else
        delete window;
}


void PVC::pv_reclaim2( SAMPLE * window )
{
    if( !window ) return;
    if( _pool )
        _win_pool.put( &window, 1 );
    else
        delete []  window;
}


//-----------------------------------------------------------------------------
// name: Cbuffer()
// desc: constructor
//-----------------------------------------------------------------------------
CBuffer::CBuffer()
{
    m_data = NULL;
    m_data_width = m_read_offset = m_write_offset = m_max_elem = 0;
}




//-----------------------------------------------------------------------------
// name: ~CBuffer()
// desc: destructor
//-----------------------------------------------------------------------------
CBuffer::~CBuffer()
{
    this->cleanup();
}




//-----------------------------------------------------------------------------
// name: initialize()
// desc: initialize
//-----------------------------------------------------------------------------
PVC_BOOL CBuffer::initialize( PVC_UINT num_elem, PVC_UINT width )
{
    // cleanup
    cleanup();

    BB_log( BB_LOG_CRAZY, "Pool %x initializing", this );

    // allocate
    m_data = (PVC_BYTE *)malloc( num_elem * width );
    if( !m_data )
    {
        BB_log( BB_LOG_SEVERE, "(pvc) : Could not allocate buffers" );
        return false;
    }

    memset( m_data, 0, num_elem * width );
    m_data_width = width;
    m_read_offset = 0;
    m_write_offset = 0;
    m_max_elem = num_elem;

    return true;
}




//-----------------------------------------------------------------------------
// name: cleanup()
// desc: cleanup
//-----------------------------------------------------------------------------
void CBuffer::cleanup()
{
    if( !m_data )
        return;

    free( m_data );

    m_data = NULL;
    m_data_width = m_read_offset = m_write_offset = m_max_elem = 0;
}




//-----------------------------------------------------------------------------
// name: put()
// desc: put
//-----------------------------------------------------------------------------
void CBuffer::put( void * data, PVC_UINT num_elem )
{
    PVC_UINT i, j;
    PVC_BYTE * d = (PVC_BYTE *)data;

    // copy
    for( i = 0; i < num_elem; i++ )
    {
        for( j = 0; j < m_data_width; j++ )
        {
            m_data[m_write_offset*m_data_width+j] = d[i*m_data_width+j];
        }

        // move the write
        m_write_offset++;

        // wrap
        if( m_write_offset >= m_max_elem )
            m_write_offset = 0;
    }
}




//-----------------------------------------------------------------------------
// name: get()
// desc: get
//-----------------------------------------------------------------------------
PVC_UINT CBuffer::get( void * data, PVC_UINT num_elem )
{
    PVC_UINT i, j;
    PVC_BYTE * d = (PVC_BYTE *)data;

    // read catch up with write
    if( m_read_offset == m_write_offset )
    {
        BB_log( BB_LOG_FINE, "(pvc) Cannot get buffer from pool %x; read offset = write offset = %i", this, m_read_offset );
        return 0;
    }

    // copy
    for( i = 0; i < num_elem; i++ )
    {
        for( j = 0; j < m_data_width; j++ )
        {
            d[i*m_data_width+j] = m_data[m_read_offset*m_data_width+j];
            m_data[m_read_offset*m_data_width+j] = 0;
        }

        // move read
        m_read_offset++;

        // catch up
        if( m_read_offset == m_write_offset )
        {
            i++;
            break;
        }

        // wrap
        if( m_read_offset >= m_max_elem )
            m_read_offset = 0;
    }

    // return number of elems
    return i;
}



//-----------------------------------------------------------------------------
// name: constructor
// desc: constructs
//-----------------------------------------------------------------------------
PVCtoo::PVCtoo( SAMPLE * orig, t_TAPUINT length, t_TAPUINT ana_hop_size, t_TAPUINT window_size, t_TAPUINT io_size, t_TAPUINT pool_size ) 
    : PVC( window_size, io_size, pool_size ) 
{
    SAFE_DELETE( _windows );
    _windows = new win_vector;
    // pre-analyze
    SAMPLE * ana_start, * ana_buf = NULL;
    int index = 0;
    while( index < length )
    {
        int tocopy = length - index;
        if( tocopy >= _io_size ) // don't bother to copy
            ana_start = orig + index; 
        else // need to add 0s at end, so copy samples into ana_buf
        {
            if(!ana_buf)
                ana_buf = new SAMPLE[_io_size];
            memcpy( ana_buf, orig + index, tocopy * sizeof(SAMPLE) );
            memset( ana_buf + tocopy, 0, (_io_size - tocopy) * sizeof(SAMPLE) );
            ana_start = ana_buf;
        }
        pv_analyze( ana_start, ana_hop_size );
        index += _io_size;
    }
}



//-----------------------------------------------------------------------------
// name: pv_copy()
// desc: copy?
//-----------------------------------------------------------------------------
PVC * PVCtoo::pv_copy()
{
    PVCtoo * copy = new PVCtoo( NULL, 0, 0, _window_size, _io_size, _pool ); 
    
    // copy analyzed stuff
    for( int i = 0; i < _windows->size(); i++ )
    {
        // get new polar window
        polar_window * p = NULL;
        if( copy->_pool )
        {
            if( !copy->_polar_pool.get( &p, 1 ) )
            {
                BB_log( BB_LOG_SYSTEM, "pool %x exhausted!", &_polar_pool );
                assert( FALSE );
            }
        }
        else
            p = new polar_window( _window_size / 2 );
        
        // copy
        polar_window * q = ((win_vector *)_windows)->get(i); 
        p->len = q->len;
        memcpy( p->old, q->old, q->len * sizeof(polar) );
        memcpy( p->array, q->old, p->len * sizeof(polar) );

        // add to queue (shouldn't this give an error message? weird)
        copy->push_window( p );
    }

    // return 
    return copy;
}


//-----------------------------------------------------------------------------
// name: pv_clear()
// desc: clear most things
//-----------------------------------------------------------------------------
void PVCtoo::pv_clear()
{
    memset( _window[0], 0, (_window_size + _io_size)*2*sizeof(SAMPLE) ); 
    memset( _window[1], 0, (_window_size + _io_size)*2*sizeof(SAMPLE) );
    memset( _space, 0, _window_size/2*sizeof(SAMPLE) );
    memset( _space2, 0, _window_size/2*sizeof(SAMPLE) );
    _data_size = 0;
    _which = 0;
    _count = 0;

    memset( _ola[0], 0, (_window_size + _io_size)*2*sizeof(SAMPLE) );
    memset( _ola[1], 0, (_window_size + _io_size)*2*sizeof(SAMPLE) );
    _index = 0;
    
    SAMPLE * a;

    // could be windows left in queues
    while( _ready.size() )
    {
        a = _ready.front();
        SAFE_DELETE_ARRAY( a );
        _ready.pop();
    }

    // reset analyzed queue
    ((win_vector *)_windows)->rewind();

    // phase_inc...?
    memset( _phase_inc, 0, _window_size/2*sizeof(float) ); 
}


//-----------------------------------------------------------------------------
// name: push_window
// desc: add window to win_queue
//-----------------------------------------------------------------------------
void PVCtoo::push_window( polar_window * p )
{
    _windows->push( p );
}
