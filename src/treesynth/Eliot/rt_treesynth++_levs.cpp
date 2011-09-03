/*
	Treesynth
	Includes left tree, randflip and ancestor versus predecessor search precedence
	Real-time
	No "Again?" prompt
	No writing output to file
	Has pruning, but so far parameters are hard coded
	Includes different options for reading the source file	
*/ 

#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <assert.h>
#include <iostream>
#include <sndfile.h>
#include "daub.h"
#include "Stk.h"
#include "RtAudio.h"
using namespace std;

#define OUR_T    float
#define CUTOFF  (1 << 18)
#define MAX_TREE_SIZE	(CUTOFF << 1)  // arbitrary, was 4194304

struct Node {
	int *cs;
	int cslength;
};

// global variables
Node *tnew_data;
OUR_T * tree = new OUR_T[MAX_TREE_SIZE];
OUR_T * lefttree = new OUR_T[MAX_TREE_SIZE]; // optional
OUR_T *tnew = new OUR_T[MAX_TREE_SIZE];
bool leftinit = false; // whether lefttree has been initialized

short levels; // number of levels in the tree
int *lengths; // vector with number of nodes for each level
int *sums; // sum of lengths of previous levels so we know where next level starts

// treesynth knobs
int npredecessors = 3; // number of predecessors checked (dummy value of 3)
float kfactor = 0.3f; // the thing that actually determines npredecessors
bool randflip = false; // whether first 2 coefficients are copied in order or randomly
double percentage = .25; // percentage of nodes to be considered when learning new tree
double epsilon;	// closeness threshold; calculated from percentage
bool ancfirst = true; // whether learning is first done on ancestors or predecessors
int startlevel = 1;
int stoplevel = 10; // changed later in the program
enum TheReadModes
{
    RM_STOP = 0x0,
    RM_WRAP = 0x1,
    RM_BOUNCE = 0x2,
    RM_STATIONARY = 0x4, // maybe this is an action instead of a direction?

    RM_FORWARD = 0x8,
    RM_BACKWARD = 0x10,
    RM_RANDOM = 0x20,

    // don't or this in
    RM_ACTION_MASK = 0x7,
    RM_DIRECTION_MASK = ~RM_ACTION_MASK
};
int rm_mode = RM_FORWARD | RM_STATIONARY; // what direction and what to do at boundary
int rm_next_pos = 0; // the next read position
int rm_next_length = CUTOFF; // the amount of data to read next

// global audio buffer
#define TS_BIG_BUFFER_SIZE  CUTOFF
#define TS_BIG_BUFFER_COUNT 16
OUR_T g_big_buffer[TS_BIG_BUFFER_COUNT][TS_BIG_BUFFER_SIZE];
// shouldn't the "2" be "g_max_data_count"?  yes.  now it's TS_BIG_BUFFER_COUNT
int g_write_index = 0;
int g_read_index = 0;

#define TS_BUFFER_SIZE 2048
OUR_T * g_audio_begin = NULL;
OUR_T * g_audio_end = NULL;
int g_buffer_size = TS_BUFFER_SIZE;
unsigned int g_data_count = 0;
unsigned int g_max_data_count = 2;
int g_ready = 0; // for filling up buffers before anything is played

// functions prototypes
int audio_cb( char * buffer, int buffer_size, void * user_data );
bool audio_initialize( unsigned int srate );
void set_next_pos( const char * filename );

//-----------------------------------------------------------------------------
// name: double pow( int, int )
// desc: HACK: for pow(int, int)
//-----------------------------------------------------------------------------
double pow( int x, int n ) { return pow( (double)x, (double)n ); }

//-----------------------------------------------------------------------------
// name: int our_min( int, int )
// desc: basic minimum function (return minimum of 2 arguments)
//-----------------------------------------------------------------------------
int our_min(int a, int b) {
	// basic minimum function
	if(a < b)
		return a;
	else
		return b;
}

//-----------------------------------------------------------------------------
// name: short maximum( short *, int )
// desc: return value of maximum element in an array
//-----------------------------------------------------------------------------
short maximum(short *array, int size) {
	// return value of maximum element in 'short' array
	if(size <= 0)
		return 0;
	short max = array[0];
	int index = 1;
	while(index < size) {
		if(array[index] > max)
			max = array[index];
		index++;
	}
	return max;
}

//-----------------------------------------------------------------------------
// name: int lg( int )
// desc: floor of base 2 log for positive integers only
//-----------------------------------------------------------------------------
int lg( int n ) { 
	// floor of base 2 log...didn't find in math.h (didn't look for it much, either)
	if( n <= 0 ) {
		cerr << "Sorry, I'm a hack and only compute approximate logs of positive integers" << endl;
		exit(2);
	}
	int log = 0;
	int result = 1;
	while( result <= n ) {
		result *= 2;
		log += 1;
	}
	return log - 1;
}


//-----------------------------------------------------------------------------
// name: int compare( void *, void * )
// desc: compare two values...for use in FindEpsilon
//-----------------------------------------------------------------------------
int compare( const void *arg1, const void *arg2 )
{
    OUR_T left = *( (OUR_T *)arg1 );
    OUR_T right = *( (OUR_T *)arg2 );

    if( left < right ) return -1;
    else if( left == right ) return 0;
    else return 1;
}

//-----------------------------------------------------------------------------
// name: double FindEpsilon( OUR_T *, int, double )
// desc: finds threshold epsilon, based on percentage of values preferred to
//		 lie within epsilon of some given node value (randomness measure)
//-----------------------------------------------------------------------------
double FindEpsilon( OUR_T * tree, int size, double P )
{
    assert( P > 0.0 );

    OUR_T * our_tree = new OUR_T[size];
    memcpy( our_tree, tree, size * sizeof(OUR_T) );

    // find the L1 norm
    for( int i = 1; i < size; i++ )
        our_tree[i] = fabs(our_tree[i]);

    // sort it
    qsort( our_tree, size, sizeof(OUR_T), compare );

    int index = (int)(P * size + .5);
    if( index >= size ) index = size-1;

    cout << size << " " << P << " " << index << endl;

    return 2.0 * (double)our_tree[index];
}

int *cands;
bool *in;
short *S;
short *L;


//-----------------------------------------------------------------------------
// name: Ancestors( int, int, float, int )
// desc: Part 1 of CandidateSet; ancestor search
//-----------------------------------------------------------------------------
short Ancestors( int lev, int nod, float epsilon, int csl )
{
	// Part 1
	// Compare ancestors of given node in tnew_data with ancestors of 
	// selected nodes in tree (AND LEFTTREE?)
	OUR_T ancTnew, ancTree; // float
	
	for(int w = 0; w < csl; w++) {
		OUR_T sum = 0;
		L[w] = 0;
		
		//if(S[w] != M2)
		//	in[w] = false;

		if(in[w]) { // if w is in the existing candidate set
			for(short v = lev; v >= 1; v--) { 
				ancTnew = tnew[sums[v] + (int)(nod/pow(2, lev-v))];
				ancTree = tree[sums[v] + (int)(cands[w]/pow(2, lev-v))];	
																			
				OUR_T s = ancTnew - ancTree; // float
				if(s < 0)		// convoluted way because apparently
					s *= -1;	// abs(_) converts to an integer
				sum += s;

				if(((float)(sum)/(lev-v+1)) <= epsilon/2)	// epsilon, not epsilon/N 
					L[w]++;
				else
					break;
			} // end of for v
		} // end of if
	} // end of for w

	short M1 = maximum(L, csl);
	return M1;
}


//-----------------------------------------------------------------------------
// name: Predecessors( int, int, float, int )
// desc: Part 2 of CandidateSet; predecessor search
//-----------------------------------------------------------------------------
short Predecessors( int lev, int nod, float epsilon, int csl )
{
	// Part 2
	// similar to part 1 but with predecessors instead of ancestors
	
	// determine number of predecessors to check in general for given node in tnew_data
	int npred = our_min(npredecessors, nod);
	
	OUR_T predTnew, predTree;
	for(int q = 0; q < csl; q++) {
		S[q] = 0;

		// eliminate nodes from candidate set based on whether their L-value is M1
		//if( L[q] != M1 )
		//	in[q] = false;
		
		if(in[q]){
			// determine number of predecessors for given node in tree
			//int np = our_min(npred, cands[q]);
			
			// find and compare np predecessors of tree and tnew
			for(int z = 1; z <= npred; z++) { // npred instead of np
				// figure out where to get stuff from				
				predTnew = tnew[sums[lev] + nod - z];

				if( cands[q] - z >= 0 )
					predTree = tree[sums[lev] + cands[q] - z]; 
				else if( lengths[lev] + cands[q] - z >= 0 && lefttree )
					predTree = lefttree[sums[lev] + lengths[lev] + cands[q] - z];
				else break;

				OUR_T diff = predTnew - predTree;
				if(diff < 0)
					diff *= -1;

				if(diff <= epsilon) 
					S[q]++;
				else
					break;
			} // end of for z
		} // end of else
	} // end of for q

	short M2 = maximum(S, csl);
	return M2;
}

//-----------------------------------------------------------------------------
// name: CandidateSet( int, int, float )
// desc: the very important function for doing the actual learning
//-----------------------------------------------------------------------------
void CandidateSet(int lev, int nod, float epsilon) {
	// Find candidate set and update node in tnew_data

	// Create empty candidate set
	int csl = 2 * tnew_data[sums[lev-1]+(nod/2)].cslength;

	int i;
	for( i = 0; i < csl/2; i++) {
		// insert candidate
		cands[i] = 2*tnew_data[sums[lev-1]+(nod/2)].cs[i];
		cands[i+csl/2] = 2*tnew_data[sums[lev-1]+(nod/2)].cs[i] + 1;
			
		// verify candidate
		if(cands[i] < lengths[lev])	
			in[i] = true;
		else  
			in[i] = false;
		
		if(cands[i+csl/2] < lengths[lev]) 
			in[i+csl/2] = true;
		else 
			in[i+csl/2] = false;
	}
	
	// Do ancestor or predecessor processing
	short M1 = 0, M2 = 0;
	if( ancfirst ) {
		M1 = Ancestors( lev, nod, epsilon, csl );
		for( i = 0; i < csl; i++ )
			if( L[i] != M1 )
				in[i] = false;
		M2 = Predecessors( lev, nod, epsilon, csl );
	}
	else {
		M2 = Predecessors( lev, nod, epsilon, csl );
		for( i = 0; i < csl; i++ )
			if( S[i] != M2 )
				in[i] = false;
		M1 = Ancestors( lev, nod, epsilon, csl );
	}

	// Return all nodes s.t. L = M1 and S = M2
	tnew_data[sums[lev] + nod].cs = new int[csl];
	int index = 0;
	for(int c = 0; c < csl; c++) {
		if( in[c] && L[c] == M1 && S[c] == M2 ) {
			tnew_data[sums[lev] + nod].cs[index++] = cands[c];
		}
	}
	tnew_data[sums[lev] + nod].cslength = index;

	// Back up if cslength is not positive (which should not happen)
	if( index <= 0 ) {
		cerr << "Uh-oh " << lev << "-" << nod << " " << index << endl;
		// copy all candidates regardless of whether they passed the test or not
		// (these are all the children of the nodes in the parent's candidate set)
		index = 0;
		for(int c = 0; c < csl; c++ ) 
			tnew_data[sums[lev] + nod].cs[index++] = cands[c];
		
		tnew_data[sums[lev] + nod].cslength = index;
	}

	// garbage collection
	if(nod % 2 == 1)
		delete [] tnew_data[sums[lev-1]+(nod/2)].cs;
	if(lev == levels - 2 && nod > 0)
		delete [] tnew_data[sums[lev] + (nod-1)].cs;

} // end of CandidateSet


char ifilename[1024] = "orig.wav";
char ofilename[1024] = "syn.wav";
char leftfile[1024] = "";



SF_INFO info;
SNDFILE *sfread; 
SNDFILE *sfwrite;

//-----------------------------------------------------------------------------
// name: int ReadSoundFile( char, float *, int )
// desc: Reads given sound file into data array
//-----------------------------------------------------------------------------
int ReadSoundFile( char filename[], float * data, int datasize )
{
	if( !sfread ) {
		sfread = sf_open( filename, SFM_READ, &info );
		if( !sfread )
		{
			cerr << "rt_treesynth++: cannot open file '" << filename << "', quitting" << endl;
            char x[256];
            cin.getline( x, 256 );
			exit(1);
		}
	}

    datasize = rm_next_length;
    sf_seek( sfread, rm_next_pos, SEEK_SET );
	int itemsread = sf_read_float( sfread, data, datasize );
    set_next_pos( filename );

    // rt audio
    if( !audio_initialize( info.samplerate ) ) {	// 44100
        cerr << "rt_treesynth++: cannot open audio interface, quitting" << endl;
		char x[256];
		cin.getline( x, 256 );
		exit(1);
    }

	return itemsread;
}


//-----------------------------------------------------------------------------
// name: int WriteSoundFile( char, float *, int )
// desc: writes synthesized sound to file or to buffer for audio_cb to play
//-----------------------------------------------------------------------------
int WriteSoundFile( char filename[], float * data, int datasize )
{
    if( false )
    {
        if( !sfwrite ) {
			sfwrite = sf_open( filename, SFM_WRITE, &info );

			if( !sfwrite )
			{
				cerr << "rt_treesynth++: cannot open file '" << filename << "', quitting" << endl;
				char x;
				cin >> x;
				exit(1);
			}
		}

		int itemswritten = sf_write_float( sfwrite, data, datasize );
		if( itemswritten <= 0 ) {
            cerr << "rt_treesynth++: cannot write to file '" << filename << "', quitting" << endl;
            char x;
			cin >> x;
			exit(3);
        }
		
        //sf_close( sfwrite );
        return itemswritten;
    }
    else
    {
        while( g_data_count >= g_max_data_count )
#ifdef __WINDOWS_DS__
			Sleep( 10 );
#else
            usleep( 10000 );
#endif
        
        // set the buffer to write
		OUR_T * next_buffer = g_big_buffer[g_write_index++];
        g_write_index %= g_max_data_count;
        
        // copy the data
        memcpy( next_buffer, data, datasize * sizeof(OUR_T) );
        
        // increment data count
        g_data_count++;
	}
    
    return 0;
}


//-----------------------------------------------------------------------------
// name: bool parse_args( int argc, char ** argv )
// desc: Read and decipher arguments
//-----------------------------------------------------------------------------
bool parse_args( int argc, char ** argv )
{
	int index = 1;

	while( argc > index )
    {
        if( !strncmp( argv[index], "-p", 2 ) )
        {
            if( argc > ++index )
            {
                percentage = atof( argv[index++] );
                if( percentage > 1.0 )
                    percentage /= 100.0;
                if( percentage > 1.0 )
                {
                    cout << "uh... percentage needs to be smaller" << endl;
                    return 1;
                }
            }
        }
        else if( !strncmp( argv[index], "-o", 2 ) )
		{
			if( argc > ++index )
				strcpy( ofilename, argv[index++] );
		}
		else if( !strncmp( argv[index], "-k", 2 ) )
		{
			if( argc > ++index ) {
				kfactor = atof( argv[index++] );	// kfactors used to be npredecessors
				if( kfactor > 1 )
					kfactor = 0.5;
			}
		}
		else if( !strncmp( argv[index], "-l", 2 ) )
		{
			if( argc > ++index )
				strcpy( leftfile, argv[index++] );
		}
		else if( !strncmp( argv[index], "-r", 2 ) )
		{
			randflip = true;
			index++;
		}
		else if( !strncmp( argv[index], "-a", 2 ) )
		{
			if( argc > ++index )
				ancfirst = !( !strncmp( argv[index++], "0", 2 ) );
		}
		else {
			strcpy( ifilename, argv[index++] );
		}
    }

    cout << "input file: " << ifilename << endl;
	cout << "left file: " << (strncmp( leftfile, "", 2 ) ? leftfile : ifilename) << endl;

	return true;
}


//-----------------------------------------------------------------------------
// name: bool setup( void )
// desc: read sound files, initialize global values, allocate
//		 memory, set up original wavelet trees, initialize root of new tree
//-----------------------------------------------------------------------------
bool setup( void )
{
	// Read wav files
	static int items, litems;
    int cutoff = CUTOFF;
	cerr << "cutoff " << cutoff << endl;
	float *orig = new float[cutoff], *left = new float[cutoff];
	items = ReadSoundFile( ifilename, orig, cutoff );
	if( !leftinit ) {
		if( strncmp( leftfile, "", 2 ) )
			litems = ReadSoundFile( leftfile, left, cutoff );
		else
			litems = ReadSoundFile( ifilename, left, cutoff );
		leftinit = true;
	}

	// Copy signals into trees
	levels = lg( our_min( items, litems ) );
	cerr << "levels: " << levels << endl; //lengths = new int[levels];
	if(!stoplevel) stoplevel = levels - 2;
	cerr << "startlevel: " << startlevel << "; stoplevel: " << stoplevel << endl;
	sums = new int[levels + 1];
	int i;
	for( i = 0; i <= levels; i++ ) {
		if( i == 0 )
			sums[i] = 1;
		else
			sums[i] = 2 * sums[i-1];
	}
	lengths = sums;
	
	assert( sums[levels] == pow(2, levels) );
	
	int start = ( items - sums[levels] ) / 2;	// take middle values
	int lstart = litems - sums[levels];			// take rightmost values
	for( i = 0; i < sums[levels]; i++ ) {
		tree[i] = (OUR_T)(orig[start + i]);
		if( litems > 0 ) 
			lefttree[i] = (OUR_T)(left[lstart + i]);	
	}

	// Wavelet decompositions
	pwtset( 10 );
	wt1( tree, sums[levels], 1, *pwt );
	wt1( lefttree, sums[levels], 1, *pwt );
	
	// Calculate nearness threshold (epsilon) and print related values
	cout << "p: " << percentage << endl;
    cout << "epsilon: " << (epsilon = FindEpsilon( tree, sums[levels], percentage )) << endl;
	//cout << "k: " << npredecessors << endl;
	cout << "kfactor: " << kfactor << endl;

	// Allocate memory for things needed in CandidateSet
	tnew_data = new Node[(sums[levels])]; 
	cands = new int[lengths[levels-1]];
	in = new bool[lengths[levels-1]];
	L = new short[lengths[levels-1]];
	S = new short[lengths[levels-1]];
	
	// Copy root of tnew, since that's always the same
	tnew[0] = tree[0];	// approximation
	tnew[1] = tree[1];	// root

	// Return
	return true;
}


//-----------------------------------------------------------------------------
// name: treesynth( ) [formerly main( )]
// desc: does wavelet tree learning to build the new tree, calls WriteSoundFile
//-----------------------------------------------------------------------------
void treesynth( void )
{
    // Decipher arguments
	// Read sound files
	
	// Make wavelet trees
	// *** What to do with signal lengths that are not powers of 2? 
	// *** What about level sizes?
	// ****** Looks like I have to cut-off the input signal to have power-2 length
	//			otherwise, this doesn't reconstruct signals correctly.
	// ****** Pick "middle" portion if cutting off?
	
	// Book-keeping
	// levels: number of levels in the decomposition
	// lengths[i] = length of level i (i==0 -> root)
	// sums[i] = start index of level i or number of array elements upto but not including level i
	// But in this case, sums[i] = lengths[i] (see /* ... */ in for loop below)
	
	// All of the above is in SETUP.

	// Set up candidate set for root (also always the same, but gets deleted)
	tnew_data[1].cs = new int[1];						
	tnew_data[1].cs[0] = 0;
	tnew_data[1].cslength = 1;

	// Synthesize new tree
	//tnew = OUR_T[(sums[levels])];
	if( startlevel <= 1 ) {
		for(int n = 0; n < lengths[1]; n++)	{		
			// try copying randomly instead of in order
			// (so instead of LR it could be LL or RR or RL)
			if( randflip ) {
				int random = (int)(rand()/(RAND_MAX + 1.0) * lengths[1]);
				tnew[sums[1]+n] = tree[sums[1]+random];
			}
			else
				tnew[sums[1]+n] = tree[sums[1]+n];
		}
	}
	else {
		int l, n; 
		for( l = 1; l <= startlevel; l++ ) {
			for( n = 0; n < lengths[l]; n++ ) {
				tnew[sums[l]+n] = tree[sums[l]+n];
				
				if( l == startlevel - 1 ) {
					tnew_data[sums[l]+n].cs = new int[lengths[l]];
					tnew_data[sums[l]+n].cslength = lengths[l];
					for( int q = 0; q < lengths[l]; q++ )
						tnew_data[sums[l]+n].cs[q] = q;
				}
			}
		}
	}

	// Breadth-first part
    int lev;
	for(lev = startlevel; lev <= stoplevel; lev++) {				
		if( lev >= levels - 1 )
			break;
		cout << "Processing level " << lev << "   ";
		cout << "k is " << (npredecessors = (int)(lengths[lev] * kfactor)) << endl;
		for(int nod = 0; nod < lengths[lev]; nod++) {
			CandidateSet(lev, nod, epsilon); 
			if(tnew_data[sums[lev]+nod].cslength == 0) {
				cerr << "Double Uh-oh " << lev << "-" << nod << endl;
			}
			// Randomly choose a node from candidate set and steal its children
			int randPick = (int)(rand()/(RAND_MAX + 1.0) * tnew_data[sums[lev]+nod].cslength);
			randPick = tnew_data[sums[lev]+nod].cs[randPick];								
			
			// left child
			tnew[sums[lev+1] + 2*nod] = tree[sums[lev+1] + 2*randPick];
			// right child: changed to make sure nodes referred to are within limits
			if(2*nod + 1 < lengths[lev+1]) {
				if( 2*randPick + 1 < lengths[lev+1] )
					tnew[sums[lev+1] + 2*nod + 1] = tree[sums[lev+1] + 2*randPick + 1];	

				else {																						
					tnew[sums[lev+1] + 2*nod + 1] = tree[sums[lev+1] + 2*randPick];				
					
					if( randPick > 0 ) { 
						tnew[sums[lev+1] + 2*nod] = tree[sums[lev+1] + 2*randPick - 1];
					}
				}
			}
			// if it's stoplevel, copy all descendants
			if( lev == stoplevel ) {
				int l, m = 2, p;
				for( l = lev + 2; l < levels; l++ ) {
					m = 2*m;
					for( p = 0; p < m; p++ ) {
						tnew[sums[l] + m*nod + p] = tree[sums[l] + m*randPick + p];
					}
				}
			} // yeah...
		}
	}

	// Reconstruct new tree
	wt1( tnew, sums[levels], -1, *pwt );
	
	// Print
	int written = WriteSoundFile( ofilename, tnew, sums[levels] );
	//cout << written << " samples written to " << ofilename << endl;
}


//-----------------------------------------------------------------------------
// name: MAIN
// desc: the central nervous system
//-----------------------------------------------------------------------------
int main( int argc, char ** argv )
{
	srand(time(0));
	
	parse_args( argc, argv );
	
	while( g_data_count < g_max_data_count ) {
		if( setup() )
			treesynth();
		else {
			cerr << "rt_treesynth++: set up failed, quitting" << endl;
			exit(5);
		}
	}
	g_ready = 1;

	//char again;
	
	while( true ) {
		//cout << "Again? ";	
		//cin >> again;
		//if( again == 'n' )
		//	break;
		//else {
			for( int t = 0; t < sums[levels]; t++ ) {
				lefttree[t] = tnew[t];
				tnew[t] = 0.0;
			}

			if( setup() )
				treesynth();
			else {
				cerr << "rt_treesynth++: set up failed, quitting" << endl;
				exit(5);
			}
		//}
	}
	
	return 0;
}


// audio interface
RtAudio * g_audio = NULL;
unsigned int g_srate = 0;


//-----------------------------------------------------------------------------
// name: audio_cb()
// desc: audio callback
//-----------------------------------------------------------------------------
int audio_cb( char * buffer, int buffer_size, void * user_data )
{
    // if not ready, play wavelet tree? (just for fun)
	if( !g_ready )
	{
		memcpy( buffer, tree, buffer_size * sizeof(OUR_T) );
		return 0;
	}

	// copy out
    if( !g_data_count )
    {
        // no data to playback yet - silence the buffer
        memset( buffer, 0, buffer_size * sizeof(OUR_T) );
    }
    else
    {
        // initialize the read pointers
        if( !g_audio_begin )
        {
            g_audio_begin = g_big_buffer[g_read_index++];
            g_audio_end = g_audio_begin + sums[levels];		// TS_BIG_BUFFER_SIZE
            g_read_index %= g_max_data_count;
        }
        
        // copy the data out
        memcpy( buffer, g_audio_begin, buffer_size * sizeof(OUR_T) );
        // increment the read pointer
        g_audio_begin += buffer_size;
        // check to see if at end
        if( g_audio_begin >= g_audio_end )
        {
            // reset
            g_audio_begin = NULL;
            g_data_count--;
		}
    }

    return 0;
}


//-----------------------------------------------------------------------------
// name: audio_initialize( )
// desc: set up audio capture and playback and initializes any application data
//-----------------------------------------------------------------------------
bool audio_initialize( unsigned int srate )
{
    if( g_audio )
        return TRUE;

    Stk::setSampleRate( g_srate = srate );

    try
    {
        // open the audio device for capture and playback
        g_audio = new RtAudio( 0, 1, 0, 0, RTAUDIO_FLOAT32,
            g_srate, &g_buffer_size, 4 );
    }
    catch( StkError & e )
    {
        // exception
        fprintf( stderr, "%s\n", e.getMessage() );
        fprintf( stderr, "error: cannot open audio device for capture/playback...\n" );
        return false;
    }

    // set the audio callback
    g_audio->setStreamCallback( audio_cb, NULL );
    
    // start the audio
    g_audio->startStream( );
    
    return true;
}




//-----------------------------------------------------------------------------
// name: set_next_pos( const char * )
// desc: set next position to read from in source sound file
//-----------------------------------------------------------------------------
void set_next_pos( const char * filename )
{
    int action = rm_mode & RM_ACTION_MASK;
    int direction = rm_mode & RM_DIRECTION_MASK;
    
    cerr << "reading samples '" << rm_next_pos << "' - '" << (rm_next_pos+rm_next_length) << "'" << endl;

    // don't do anything if stationary
    if( action == RM_STATIONARY )
        return;

    // advance to the next frame
    switch( direction )
    {
        case RM_FORWARD:
            rm_next_pos += rm_next_length;
            break;
        case RM_BACKWARD:
            rm_next_pos -= rm_next_length;
            break;
        case RM_RANDOM:
            rm_next_pos = (int)( rand()/(float)RAND_MAX * (info.frames-rm_next_length) );
            if( rm_next_pos < 0 ) rm_next_pos = 0;
            break;
        default:
            cerr << "no!!!!!!" << endl;
            exit(1);
    }	

    // passed the end
    if( rm_next_pos >= info.frames ) {
        switch( action )
        {
            case RM_STOP:
            {
                cerr << "rt_treesynth++: cannot read no mo' file '" << filename << "', stopping"<< endl;
                char x[256];
                cin.getline( x, 256 );
                exit(1);
                break;
            }
            case RM_WRAP:
                cerr << "warping!" << endl;
                rm_next_pos = 0;
                break;
            case RM_BOUNCE:
                cerr << "bouncing!" << endl;
                assert( direction == RM_FORWARD );
                rm_mode = RM_BACKWARD | action;
                rm_next_pos -= rm_next_length*2;
                break;
            default:
                cerr << "no!!!!" << endl;
                exit(1);
        }
	}
    else if( rm_next_pos < 0 && direction == RM_BACKWARD ) // passed the beginning
    {
        switch( action )
        {
            case RM_STOP:
            {
                cerr << "rt_treesynth++: cannot read no mo' file '" << filename << "', stopping"<< endl;
                char x[256];
                cin.getline( x, 256 );
                exit(1);
                break;
            }
            case RM_WRAP:
                cerr << "warping!!" << endl;
                rm_next_pos = info.frames - ( info.frames % rm_next_length );
                break;
            case RM_BOUNCE:
                cerr << "boucing!!" << endl;
                assert( direction == RM_BACKWARD );
                rm_mode = RM_FORWARD | action;
                rm_next_pos = rm_next_length;
                break;
            default:
                cerr << "no!!!!!" << endl;
                exit(1);
        }
    }

    // make sure we are legal
    if( rm_next_pos < 0 )
        rm_next_pos = 0;
}
