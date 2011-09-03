#ifndef __BIRDBRAIN_H__
#define __BIRDBRAIN_H__

#include <math.h>
#include <stdlib.h>
#include <assert.h>
#include <memory.h>
#include <vector>
#include <string>

// a pie
#define PIE 3.14159265358979323846

// a sample
#define SAMPLE float
#define uint   unsigned long

// all things dealing with time should maybe use this
#define TIME double

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif


class BirdBrain
{
public:
	static uint srate() { return our_srate; }
    static uint fft_size() { return our_fft_size; }
    static uint wnd_size() { return our_wnd_size; }
	static uint syn_wnd_size() { return our_syn_wnd_size; }
    static uint hop_size() { return our_hop_size; }
    static uint max_tracks() { return our_max_tracks; }
    static uint over_tracking() { return our_over_tracking; }
	static int last_hop() { return our_last_hop; }
	static float freq_min() { return our_freq_min; }
	static float freq_max() { return our_freq_max; }

	static void ola( SAMPLE *buffer, SAMPLE *to_add, int shift, int size );
	static void filter( const SAMPLE *x, const SAMPLE *winxp, SAMPLE *y, int len, float *B, int lenb, float *A, int lena );
    static void scale( SAMPLE *buffer, uint len, float factor );
    static void scale_fft( SAMPLE *buffer, uint len, uint fft_size = fft_size(), uint wnd_size = wnd_size() );
    static void scale_ifft( SAMPLE *buffer, uint len, uint fft_size = fft_size(), uint wnd_size = wnd_size() );

    static const char * getbase( const char * str );
	static std::string getpath( const char * str );
    static std::string getname( const char * str );
    static std::string get_part_of_name( const char * str );
    static std::string toString( long num );

public:
	static uint our_srate;
    static uint our_fft_size;
    static uint our_wnd_size;
	static uint our_syn_wnd_size;
    static uint our_hop_size;
	static uint our_max_tracks;
    static uint our_over_tracking;
	static int our_last_hop;
	static float our_freq_min;
	static float our_freq_max;
};


// complex type
struct complex { float re ; float im ; };
// polar type
struct polar { float mag ; float phase ; };

// complex absolute value
#define cmp_abs(x) ( sqrt( (x).re * (x).re + (x).im * (x).im ) )
// magnitude
#define cmp_mag(x) cmp_abs(x)
// phase
#define cmp_phase(x) ( atan2((double)(x).im, (double)(x).re) )

// function
struct xtoy{ virtual double y( double x ) = 0; };

// line (for threshold, for example)
struct line : public xtoy
{ double slope; double intercept;
  virtual double y( double x ) { return x * slope + intercept; }
};

// freq + polar
struct freqpolar
{
    float freq;
    polar p;
	// questionable
	TIME time; // danger comment: timeout
    // even more questionable
    uint bin;
    // no further comment
    bool isMatched;
};

// Frame in frequency domain
struct Frame
{
    union
    {
        // time-domain
        float * waveform;
        // freq-domain real + imag pairs
        complex * cmp;
    };
    // mag + phase pairs
    polar * pol;
    // corresponding frequencies to bins
    float * freqs;
    // number of elements in each of cmp, pol, freqs
	uint len;
	// capacity of waveform
	uint wlen;
    // how much data is in waveform
	uint wsize;
    // time associated with this frame
    TIME time;	// danger comment: timeout

    // constructor
    Frame() { this->init(); }
    Frame( const Frame & rhs ) { this->init(); *this = rhs; }
    ~Frame() { this->clear(); }
	
	// alloc
	void alloc_waveform( int length )
	{
	     // even numbers
	     assert( length % 2 == 0 );

	     if( waveform ) delete [] waveform;
		 waveform = new float[length];
		 memset( waveform, 0, sizeof(float) * length );
		 wlen = length;
		 len = length / 2;

		 if( pol ) alloc_pol();
		 if( freqs ) alloc_freqs();
	}

	// alloc polar
	void alloc_pol()
	{
		 assert( waveform != NULL );

		 if( pol ) delete [] pol;
		 pol = new polar[len];
		 memset( pol, 0, sizeof(polar) * len );
	}

	// alloc freqs
	void alloc_freqs()
	{
		assert( waveform != NULL );

		if( freqs ) delete [] freqs;
		freqs = new float[len];
		memset( freqs, 0, sizeof(float) * len );
	}

	// convert to polar
	void cmp2pol()
	{
		if( !pol ) alloc_pol();
		for( uint i = 0; i < len; i++ )
		{
			pol[i].mag = cmp_mag(cmp[i]);
			pol[i].phase = cmp_phase(cmp[i]);
		}
	}

	// convert to complex
	void pol2cmp()
	{
		assert( pol != NULL );	
		if( !cmp ) alloc_waveform( 2 * len );
		for( uint i = 0; i < len; i++ )
		{
			cmp[i].re = pol[i].mag * cos(pol[i].phase);
			cmp[i].im = pol[i].mag * sin(pol[i].phase);
		}
	}

	// map bins to frequencies
	void bins2freqs( double srate )
	{
		// use nyquist
		srate /= 2.0;
		if( !freqs ) alloc_freqs();
		for( uint i = 0; i < len; i++ )
		{
			freqs[i] = (double)i/len * srate;
		}
	}

    // clear
    void clear()
    {
        if( cmp ) delete [] cmp;
        if( pol ) delete [] pol;
        if( freqs ) delete [] freqs;
        cmp = NULL; pol = NULL; freqs = NULL;
        wlen = len = 0;
    }
	
	// zero
	void zero()
	{
        if( cmp ) memset( cmp, 0, len * sizeof(complex) );
        if( pol ) memset( pol, 0, len * sizeof(polar) );
        if( freqs ) memset( freqs, 0, len * sizeof(float) );
	}

    // init
    void init()
    {
        cmp = NULL;
        pol = NULL;
        freqs = NULL;
        wlen = wsize = len = 0;
        time = 0;
    }
	
	// deep copy
    Frame & operator =( const Frame & rhs )
    {
        this->clear();
		wlen = rhs.wlen;
		wsize = rhs.wsize;
        len = rhs.len;
        time = rhs.time;
        
        if( rhs.cmp )
        {
			cmp = new complex[len];
            memcpy( cmp, rhs.cmp, len * sizeof(complex) );
        }
        if( rhs.pol )
        {
            pol = new polar[len];
            memcpy( pol, rhs.pol, len * sizeof(polar) );
        }
        if( rhs.freqs )
        {
            freqs = new float[len];
            memcpy( freqs, rhs.freqs, len * sizeof(float) );
        }

        return *this;
    }

	// add two frames
	void add( const Frame &to_add, int to_add_start_pos, int this_start_pos )
	{
		assert( this_start_pos + to_add.wsize <= this->wlen );

		for( int i = to_add_start_pos; i < to_add.wsize; i++ )
		{
			this->waveform[this_start_pos + i] += to_add.waveform[i];
		}
	}

	// shift waveform right or left by some number of samples
	void shift_waveform( int from, int to )
	{
		// check bounds
		assert( from >= 0 && from < wlen && to >= 0 && to < wlen );
		// at least one of from and to must be zero, otherwise it's kind of undefined
		assert( from == 0 || to == 0 );
		// copy / shift
		int i;
		if( from > to ) // shift left
		{
			for( i = 0; i < wlen - from; i++ )
				waveform[to + i] = waveform[from + i];
			memset( waveform + (to + wlen - from), 0, sizeof(float) * (from - to) );
		}
		else if( to > from ) // shift right
		{
			for( i = wlen - to - 1; i >= 0; i-- )
				waveform[to + i] = waveform[from + i];
			memset( waveform, 0, sizeof(float) * to );
		}
	}
};

// sinusoidal track
struct Track
{
    // id of the track
    int id;
    // state
    enum { ACTIVE = 1, INACTIVE = 2, AMBIGUOUS = 3 };
    int state;
    // trajectory of the track
    std::vector<freqpolar> history; // freqpolartime
    // start time (same type of thing as Frame::time)
    TIME start;	// danger comment: timeout
    // end time
    TIME end; // danger comment: timeout
	// phase offset for synthesis
	double phase;
	// marker not used by synthesis to track history
	uint historian;
    
    // constructor
    Track() { id = 0; state = 0; start = end = 0; phase = 0.0; historian = 0; }
	// destructor
	~Track() { return; }

	// invert history
	void invert() 
	{
		std::vector<freqpolar> temp = history;
		freqpolar fpt;
		history.clear();
		
		while ( !temp.empty() ) {
			fpt = temp.back();
			history.push_back( fpt );
			temp.pop_back();
		}

        TIME t = start;
        start = end;
        end = t;

		return;
	}
};


struct TimeGrid
{
	// time of a given frame
	TIME frametime;
	// tracks that have a point in that frame
	std::vector<Track *> tracks;
	// index into history of corresponding track in tracks vector
	std::vector<int> trackinds; 
};


// a sinusoidal event
// what other info would be useful?
class SinEvent
{
public:
	SinEvent();
	SinEvent( const SinEvent &me ); // copy constructor
	~SinEvent();
	
	SinEvent &operator=( const SinEvent &rhs ); // assignment
	void clear(); // yeah

	// add a track to the event
	bool add_track( Track * track );
	// compare harmonics
	bool cmp_harm( Track * track, float error, float overlap_amt );
	// compare common frequency & amplitude modulation
	bool cmp_mod( Track * track, float f_error, float a_error, float overlap_amt );
	// compare common onset & offset
	bool cmp_onoff( Track * track, float on_error, float off_error );

	std::vector< Track * > event_tracks;
	Track * average; 
	TIME start;	// earliest start time from all tracks
	TIME end;   // latest end time from all tracks
};


// filter class
class Filter
{
public:
	Filter();
	~Filter();

	void init( float *B_, int lenb_, float *A_, int lena_ );
	void clear();
	void apply_filter( Frame &x );
	void clear_sigs(); // clears the windows/signals but retains coefficients
	
	// filter coefficients or whatever they're called
	float *A;
	float *B;
	int lena, lenb;

	// signals
	SAMPLE *winxp, *ys;
	int len;
};


// levels
#define BB_LOG_CRAZY            10 // set this to log everything
#define BB_LOG_FINEST           9
#define BB_LOG_FINER            8
#define BB_LOG_FINE             7
#define BB_LOG_CONFIG           6
#define BB_LOG_INFO             5
#define BB_LOG_WARNING          4
#define BB_LOG_SEVERE           3
#define BB_LOG_SYSTEM           2
#define BB_LOG_SYSTEM_ERROR     1
#define BB_LOG_NONE             0  // set this to log nothing

// functions
void BB_log( int, const char *, ... );
void BB_setlog( int );
void BB_pushlog();
void BB_poplog();

// actual level
extern int g_loglevel_bb;
// macro to compare
#define BB_DO_LOG(x) ( x <= g_loglevel_bb )

// number of features in feature vector
// (still need to figure out where this actually belongs)
// (didn't seem to fit in birdbrain class, but needs to be available 
// in ui_search and ui_library and probably other places)
extern int g_num_features;


#endif
