//-----------------------------------------------------------------------------
// name: pvc.h
// desc: phase vocoder
//
// authors: Ge Wang (gewang@cs.princeton.edu)
//          Ahmed Abdallah (aabdalla@princeton.edu)
//          Paul Botelho (pbotelho@princeton.edu)
// date: Spring 2004
//-----------------------------------------------------------------------------
#ifndef __PVC_H__
#define __PVC_H__

#include <stdlib.h>
#include "sceptre.h"

#include <queue>

#ifndef SAMPLE
#define SAMPLE float
#endif

//#define uint   unsigned int
#define PV_WINDOW_DEFAULT	1024
#define PV_HOPSIZE_DEFAULT  ( PV_WINDOW_DEFAULT / 8 )




// polar_array
struct polar_window
{
    polar * array;
    polar * old;
    uint len;

    // constructor
    polar_window( uint size )
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

	virtual int size() { return m_vector.size(); } 
	
	virtual polar_window * get( int index ) { return m_vector.at( index ); }

private:
	std::vector<polar_window *>m_vector;
	int cur_index;
};
// end of code that should be deleted


#define PV_HAMMING          0
#define PV_HANNING          1
#define PV_BLACKMAN         2
#define PV_RECTANGULAR      3

#ifndef BOOL__
#define BOOL__ unsigned int
#define UINT__ unsigned int
#define BYTE__ unsigned char
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
    BOOL__ initialize( UINT__ num_elem, UINT__ width );
    void cleanup();

public:
    UINT__ get( void * data, UINT__ num_elem );
    void put( void * data, UINT__ num_elem );

protected:
    BYTE__ * m_data;
    UINT__   m_data_width;
    UINT__   m_read_offset;
    UINT__   m_write_offset;
    UINT__   m_max_elem;
};




// interface
class PVC
{
public:
	PVC( uint window_size, uint io_size, uint pool_size );
	~PVC();

	virtual void pv_set_window( uint type );
	virtual void pv_analyze( SAMPLE * in, uint hop_size );
	virtual polar_window * pv_front();
	virtual polar_window * pv_deque();
	virtual void pv_unwrap_phase( polar_window * window );
	virtual void pv_phase_fix( const polar_window * prev, polar_window * curr, float factor );
	virtual void pv_freq_shift( polar_window * window, float factor );
	virtual void pv_cross_synth( polar_window * input, const polar_window * filter );
	virtual void pv_overlap_add( polar_window * window, uint hop_size );
	virtual uint pv_synthesize( SAMPLE * out );

	virtual void pv_reclaim( polar_window * window );
	virtual void pv_reclaim2( SAMPLE * window );
	virtual uint pv_get_ready_len();
	virtual void pv_ifft( const polar_window * window, SAMPLE * buffer );

	virtual void pv_clear(); 
	
	virtual PVC * pv_copy(); 

protected:
	uint _window_size;
    SAMPLE * _window[2];
    uint _which;
    SAMPLE * _the_window;
    uint _data_size;

    uint _io_size;
    uint _count;
    SAMPLE * _ola[2];
    uint _index;
	std::queue<SAMPLE *> _ready;
    float _K; // used for rescaling _the_window since its scaling changes based on hopsize/windowsize ratio

    polar * _space;
    SAMPLE * _space2;

    uint _pool;
    CBuffer _polar_pool;
    CBuffer _win_pool;

	float * _phase_inc; // calculated during phase unwrapping :|
	uint _hop_size; // analysis hop size

private:
//	std::queue<polar_window *> _windows;
	win_queue _windows;

protected:
	virtual float princarg( float phasein ); // principal argument of bin
	virtual void signal_swap( SAMPLE * sig, uint length ); 

public:
	virtual void pv_unwrap_phase_( const polar_window * prev, polar_window * curr ); 
	virtual void pv_phase_fix_( const polar_window * prev, polar_window * curr, float time_stretch );
};


// what is this? argh
class PVCtoo : public PVC
{
public:
	PVCtoo( SAMPLE * orig, uint length, uint ana_hop_size, uint window_size, uint io_size, uint pool_size ); 
	//~PVCtoo(); 

/*	virtual void pv_analyze( SAMPLE * in, uint hop_size ); 
	virtual polar_window * pv_front();
	virtual polar_window * pv_deque();
	virtual void pv_reclaim( polar_window * window );
	virtual uint pv_synthesize( SAMPLE * out, uint amount );
*/	virtual PVC * pv_copy(); 	
	void push_window( polar_window * p );

protected:
	uint _in_size;
	uint _out_size;

private:
	win_vector _windows;
//	std::vector<polar_window *> _pwindows; // instead of queue so that they stick around
};


#endif
