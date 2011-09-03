// Treesynth object
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <assert.h>
#include "util_daub.h"

// libsndfile
#ifndef __USE_SNDFILE_PRECONF__
#include <sndfile.h>
#else
#include "util_sndfile.h"
#endif

//libsamplerate
#include <samplerate.h>

#define TS_FLOAT float
#define TS_UINT  unsigned int

#define CUTOFF (1 << 18)
#define MAX_TREE_SIZE (CUTOFF << 1)
#define TS_BIG_BUFFER_SIZE  CUTOFF
#define TS_BIG_BUFFER_COUNT 8
#define TS_BUFFER_SIZE	2048;


struct Node
{
    TS_FLOAT * value; // Can point to value in m_values_ instead of reproducing, right?
    TS_UINT * cs;
    TS_UINT cslength;

    Node * parent;
    Node * left;
    Node * right;

    Node() {value = NULL; cs = NULL; cslength = 0; parent = NULL; left = NULL; right = NULL;}
    ~Node();
};


class Tree
{
public:
    Tree();
    ~Tree();
    
    bool initialize( TS_UINT levels );	// init by levels => size is power of 2 by default
    void shutdown();

public:
    inline TS_FLOAT getValue( TS_UINT level, TS_UINT offset );
    inline Node * getNode( TS_UINT level, TS_UINT offset );
    TS_FLOAT * values();
	void setValues( TS_FLOAT * values, TS_UINT size ); // just for convenience
	TS_UINT getSize();
	TS_UINT getLevels();
    void resetLevels( TS_UINT levels ); // to change the size dynamically
    void zero(); // zero out m_nodes_ and m_values_, reset m_power_
	TS_FLOAT dist( Tree & t2, int hop = 2 ); // distance going from this to t2
	TS_FLOAT power(); // (RMS^2)/length
	void power_normalize( TS_FLOAT power ); // scale

protected: // dist variations
	TS_FLOAT dEuclidean( Tree & t2 );
	TS_FLOAT dEuclideanHop( Tree &t2, int hop );
	TS_FLOAT dNormalizedPerLevel( Tree & t2 );
	TS_FLOAT dBackwards( Tree & t2 );
	TS_FLOAT dWeightedOverlap( Tree & t2, int hop );
	TS_FLOAT dPowerOnly( Tree & t2 );

protected:
    Node * m_nodes_;
    TS_FLOAT * m_values_;
    TS_UINT m_size_;
    TS_UINT m_levels_;
	TS_FLOAT m_power_;
};


class TreesynthIO; // exists below

class Treesynth
{
public:
    Treesynth();
    ~Treesynth();
    
	TreesynthIO * initialize( char filename[], int datasize, int hopsize );
	void shutdown();

    double FindEpsilon( TS_FLOAT * thevalues, int size, double P );
    short Ancestors( int lev, int nod, int csl );
    short Predecessors( int lev, int nod, int csl );
    void CandidateSet(int lev, int nod );
    bool setup( void );
    void synth( void );
    Tree * outputTree();
	TS_FLOAT * outputSignal();
    void resetTreeLevels( TS_UINT lev );
    void resetTrees();
	void UpdatePercentage( double P );
	void UpdateTreePercentage( double P );

    // the trees and related stuff
    Tree * trees;
	Tree * cur_tree; // only a pointer
	TS_FLOAT * all_epsilons; 
	TS_FLOAT ** dists;

    // treesynth knobs
    
	// If we make these private and implement a bunch of methods to set/get them,
	// they can always check the values before setting and be safe.
    float kfactor; // the thing that actually determines npredecessors
    bool randflip; // whether first 2 coefficients are copied in order or randomly
    double percentage; // percentage of nodes to be considered when learning new tree
    bool ancfirst; // whether learning is first done on ancestors or predecessors
    int startlevel;
    int stoplevel; // changed later in the program
            
	// similar parameters for picking next tree
	float tree_percentage;
	int tree_num_choices; // alternative to percentage

protected:    
    Tree * tnew_data;
    Tree * tnew;
    TS_FLOAT * synsig; // synthesized signal
	TreesynthIO * m_treesynthio_; 

	int ntrees;			// number of trees
	int npredecessors;  // number of predecessors checked
	TS_FLOAT epsilon;	// closeness threshold; calculated from percentage
    int cur_tree_index; // cur_tree = trees[cur_tree_index]; both stored for convenience
 	float tree_epsilon; // for picking next tree

    int * cands;
    bool * in;
    short * S; 
    short * L;

	TS_FLOAT * temp_buffer;
	int temp_buffer_size;

	// temp functions
	void next_tree_percentage();
	void next_tree_num();
};


enum TheReadModes
{
	// actions
	RM_STOP = 0x0,
    RM_WRAP = 0x1,
    RM_BOUNCE = 0x2,
    RM_STATIONARY = 0x4,

	// directions
    RM_FORWARD = 0x8,
    RM_BACKWARD = 0x10,
    RM_RANDOM = 0x20,

    // don't or this in
    RM_ACTION_MASK = 0x7,
    RM_DIRECTION_MASK = ~RM_ACTION_MASK
};


class TreesynthIO
{
public:
	// FUNKY THINGS:

	TreesynthIO();
	~TreesynthIO();

	// actual stuff
	int m_audio_cb( char * buffer, int buffer_size, void * user_data );
	bool audio_initialize( int (*audio_cb) (char *, int, void *), TS_UINT srate = 44100 );
	void set_next_pos( const char * filename );

	int OpenToRead( char filename[], int datasize, int hopsize ); // open file and return number of buffers needed
	int ReadSoundFile( TS_FLOAT * data, int datasize, int hopsize );
	int WriteSoundFile( char filename[], TS_FLOAT * data, int datasize );
    
    //void clear();

	// imaginary stuff?
	TS_UINT get_srate();

	// DATA:
	
	// INPUT AND OUTPUT sound file names and info
	char ifilename[1024];
	char ofilename[1024];
	char leftfile[1024];

	// INPUT reading mode
	int rm_mode;

	// OUTPUT synthesized signal size (# of samples)	
	int nsamples;

    // OUTPUT whether to write to file and/or buffer (for real-time playing)
    bool write_to_file;
    bool write_to_buffer;

	// INPUT AND OUTPUT (average power)
	TS_FLOAT avg_power;

	// OUTPUT (randomness factor, possibly to determine window)
	TS_FLOAT tree_percentage;
	
protected:
	// MORE DATA:
	bool leftinit;	// delete if not used

	// INPUT AND OUTPUT sound files and info
	SF_INFO readinfo, writeinfo;
	SNDFILE * sfread;
	SNDFILE * sfwrite;

	// INPUT reading position and hopsize
	int rm_next_pos;
	int rm_next_hop;

	// OUTPUT real-time stuff
	int m_write_index;
	int m_read_index;
	TS_FLOAT m_big_buffer[TS_BIG_BUFFER_COUNT][TS_BIG_BUFFER_SIZE];
    int m_buffer_samples[TS_BIG_BUFFER_COUNT]; // how many samples have been written into each of the big buffers
	TS_FLOAT m_ola_buffer[TS_BIG_BUFFER_SIZE];

	TS_FLOAT * m_audio_begin;
	TS_FLOAT * m_audio_end;

	int m_buffer_size;
	TS_UINT m_data_count;
	TS_UINT m_max_data_count;
	int m_ready;
	int m_callbacking;
	
	TS_UINT m_srate;

	int m_hopsize; // output hop size
	float m_window[TS_BIG_BUFFER_SIZE]; // windowing for overlap add
	int m_winsize;
};


// floating functions (aka internal hacks)
double pow( int x, int n );
int our_min(int a, int b);
short maximum(short *array, int size);
int lg( int n );
int compare( const void *arg1, const void *arg2 );
TS_FLOAT power( TS_FLOAT * samples, int size );
void power_normalize( TS_FLOAT * samples, int size, TS_FLOAT target );
