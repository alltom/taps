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
#ifndef __PVC_H__
#define __PVC_H__

#include <stdlib.h>
#include "taps_sceptre.h"

#include <queue>

#ifndef SAMPLE
#define SAMPLE float
#endif

#define PV_WINDOW_DEFAULT   1024
#define PV_HOPSIZE_DEFAULT  ( PV_WINDOW_DEFAULT / 8 )




// polar_array
struct polar_window
{
    polar * array;
    polar * old;
    t_TAPUINT len;

    // constructor
    polar_window( t_TAPUINT size )
    {
        array = new polar[size];
        old = new polar[size];
        len = size;
    }
    
    // destructor
    ~polar_window()
    {
        if( array )
        {
            delete [] array;
            delete [] old;
            array = NULL;
            old = NULL;
            len = 0;
        }
    }
};

//struct pvc_data


// window collection structures, to allow window queues or vectors or other
// (big mistake. delete.)
struct win_queue
{
public: 
    virtual void push( polar_window * win ) { m_queue.push( win ); }
    
    virtual polar_window * front() const { 
        assert( m_queue.size() > 0 );
        return m_queue.front(); 
    }
    
    virtual polar_window * dequeue() {
        assert( m_queue.size() > 0 ); 
        polar_window * p = m_queue.front();
        m_queue.pop();
        return p;
    }
    
    virtual int size() { return m_queue.size(); }

    ~win_queue() 
    {
        polar_window * w; 
        while( this->size() > 0 )
        {
            w = this->dequeue();
            SAFE_DELETE( w );
        }
    }

private:
    std::queue<polar_window *> m_queue;
};

struct win_vector : win_queue
{
public:
    win_vector() { cur_index = 0; }

    virtual void push( polar_window * win ) { m_vector.push_back( win ); }

    virtual polar_window * front() const {
        assert( m_vector.size() > 0 ); 
        return m_vector[cur_index]; 
    }

    virtual polar_window * dequeue() {
        // don't actually remove
        polar_window * p = front();
        cur_index = (cur_index + 1) % m_vector.size(); 
        return p;
    }

    virtual int size() { return m_vector.size() - 1 - cur_index; } 
    
    polar_window * get( int index ) { return m_vector.at( index ); }

    void rewind() { cur_index = 0; }

    ~win_vector()
    {
        polar_window * w;
        for( int i = 0; i < m_vector.size(); i++ )
        {
            w = m_vector.at( i );
            SAFE_DELETE( w );
        }
    }

private:
    std::vector<polar_window *>m_vector;
    int cur_index;
};
// end of code that should be deleted


#define PV_HAMMING          0
#define PV_HANNING          1
#define PV_BLACKMAN         2
#define PV_RECTANGULAR      3

#ifndef PVC_BOOL
#define PVC_BOOL unsigned int
#define PVC_UINT unsigned int
#define PVC_BYTE unsigned char
#endif




//-----------------------------------------------------------------------------
// name: class CBuffer
// desc: circular buffer
//-----------------------------------------------------------------------------
class CBuffer
{
public:
    CBuffer();
    ~CBuffer();

public:
    PVC_BOOL initialize( PVC_UINT num_elem, PVC_UINT width );
    void cleanup();

public:
    PVC_UINT get( void * data, PVC_UINT num_elem );
    void put( void * data, PVC_UINT num_elem );

protected:
    PVC_BYTE * m_data;
    PVC_UINT   m_data_width;
    PVC_UINT   m_read_offset;
    PVC_UINT   m_write_offset;
    PVC_UINT   m_max_elem;
};




// interface
class PVC
{
public:
    PVC( t_TAPUINT window_size, t_TAPUINT io_size, t_TAPUINT pool_size );
    ~PVC();

    virtual void pv_set_window( t_TAPUINT type );
    virtual bool pv_analyze( SAMPLE * in, t_TAPUINT hop_size );
    virtual polar_window * pv_front();
    virtual polar_window * pv_deque();
    virtual void pv_unwrap_phase( polar_window * window );
    virtual void pv_phase_fix( const polar_window * prev, polar_window * curr, float factor );
    virtual void pv_freq_shift( polar_window * window, float factor );
    virtual void pv_cross_synth( polar_window * input, const polar_window * filter );
    virtual bool pv_overlap_add( polar_window * window, t_TAPUINT hop_size );
    virtual t_TAPUINT pv_synthesize( SAMPLE * out );

    virtual void pv_reclaim( polar_window * window );
    virtual void pv_reclaim2( SAMPLE * window );
    virtual t_TAPUINT pv_get_ready_len();
    virtual void pv_ifft( const polar_window * window, SAMPLE * buffer );

    virtual void pv_clear(); 
    
    virtual PVC * pv_copy(); 

public:
    t_TAPUINT _pool;
    CBuffer _polar_pool;
    CBuffer _win_pool;

protected:
    t_TAPUINT _window_size;
    SAMPLE * _window[2];
    t_TAPUINT _which;
    SAMPLE * _the_window;
    t_TAPUINT _data_size;

    t_TAPUINT _io_size;
    t_TAPUINT _count;
    SAMPLE * _ola[2];
    t_TAPUINT _index;
    std::queue<SAMPLE *> _ready;
    float _K; // used for rescaling _the_window since its scaling changes based on hopsize/windowsize ratio

    polar * _space;
    SAMPLE * _space2;

    float * _phase_inc; // calculated during phase unwrapping :|
    t_TAPUINT _hop_size; // analysis hop size

protected:
//  std::queue<polar_window *> _windows;
    win_queue * _windows;

protected:
    virtual float princarg( float phasein ); // principal argument of bin
    virtual void signal_swap( SAMPLE * sig, t_TAPUINT length ); 

public:
    virtual void pv_unwrap_phase_( const polar_window * prev, polar_window * curr ); 
    virtual void pv_phase_fix_( const polar_window * prev, polar_window * curr, float time_stretch );
};


// what is this? argh
class PVCtoo : public PVC
{
public:
    PVCtoo( SAMPLE * orig, t_TAPUINT length, t_TAPUINT ana_hop_size, t_TAPUINT window_size, t_TAPUINT io_size, t_TAPUINT pool_size ); 

/*  virtual void pv_analyze( SAMPLE * in, t_TAPUINT hop_size ); 
    virtual void pv_reclaim( polar_window * window );
    virtual t_TAPUINT pv_synthesize( SAMPLE * out, t_TAPUINT amount );
    virtual polar_window * pv_front();
    virtual polar_window * pv_deque();  
*/  virtual PVC * pv_copy(); 
    virtual void pv_clear();
    void push_window( polar_window * p );
    
protected:
    t_TAPUINT _in_size;
    t_TAPUINT _out_size;

    // win_vector * _windows;
};


#endif
