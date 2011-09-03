// Treesynth object
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <assert.h>
#include <iostream>
#include "../lib/sndfile.h"
#include "daub.h"
#include "Stk.h"
#include "RtAudio.h"

#define TS_FLOAT float
#define TS_UINT  unsigned int

#define CUTOFF (1 << 18)
#define MAX_TREE_SIZE (CUTOFF << 1)
#define TS_BIG_BUFFER_SIZE  CUTOFF
#define TS_BIG_BUFFER_COUNT 16
#define TS_BUFFER_SIZE	2048;


struct Node
{
    TS_FLOAT * value; // Can point to value in m_values_ instead of reproducing, right?
    TS_UINT * cs;
    TS_UINT cslength;

    Node * parent;
    Node * left;
    Node * right;
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

protected:
    Node * m_nodes_;
    TS_FLOAT * m_values_;
    TS_UINT m_size_;
    TS_UINT m_levels_;
};


class Treesynth
{
public:
    Treesynth();
    ~Treesynth();
    
	bool initialize();
	void shutdown();

    double FindEpsilon( Tree * the_tree, int size, double P );
    short Ancestors( int lev, int nod, int csl );
    short Predecessors( int lev, int nod, int csl );
    void CandidateSet(int lev, int nod );
    bool setup( void );
    void synth( void );
    Tree * outputTree();
	TS_FLOAT * outputSignal();

    // the trees
    Tree * tree;
    Tree * lefttree;
    
    // treesynth knobs
    
	// If we make these private and implement a bunch of methods to set/get them,
	// they can always check the values before setting and be safe.
    float kfactor; // the thing that actually determines npredecessors
    bool randflip; // whether first 2 coefficients are copied in order or randomly
    double percentage; // percentage of nodes to be considered when learning new tree
    bool ancfirst; // whether learning is first done on ancestors or predecessors
    int startlevel;
    int stoplevel; // changed later in the program
            
protected:    
    Tree * tnew_data;
    Tree * tnew;
    TS_FLOAT * synsig; // synthesized signal

    bool leftinit;

	int npredecessors; // number of predecessors checked
	double epsilon;	// closeness threshold; calculated from percentage
    
    int * cands;
    bool * in;
    short * S; 
    short * L;
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

	int ReadSoundFile( char filename[], TS_FLOAT * data, int datasize );
	int WriteSoundFile( char filename[], TS_FLOAT * data, int datasize );

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

protected:
	// MORE DATA:
	Treesynth * m_treesynth_; // just a harmless pointer...
	bool leftinit;	// delete if not used

	// INPUT AND OUTPUT sound files and info
	SF_INFO info;
	SNDFILE * sfread;
	SNDFILE * sfwrite;

	// INPUT reading position and length
	int rm_next_pos;
	int rm_next_length;

	// OUTPUT real-time stuff
	int g_write_index;
	int g_read_index;
	TS_FLOAT g_big_buffer[TS_BIG_BUFFER_COUNT][TS_BIG_BUFFER_SIZE];

	TS_FLOAT * g_audio_begin;
	TS_FLOAT * g_audio_end;

	int g_buffer_size;
	TS_UINT g_data_count;
	TS_UINT g_max_data_count;
	int g_ready;
	
	RtAudio * g_audio;
	TS_UINT g_srate;
};


// floating functions (aka internal hacks)
double pow( int x, int n );
int our_min(int a, int b);
short maximum(short *array, int size);
int lg( int n );
int compare( const void *arg1, const void *arg2 );
