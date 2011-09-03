#include "Eliot.h"
#include <iostream>
using namespace std;
#include "birdbrain.h"
#include "audicle_def.h"

#ifndef NULL
#define NULL 0
#endif


//-----------------------------------------------------------------------------
// Internal Hacks 
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// name: double pow( int, int )
// desc: HACK: for pow(int, int)
//-----------------------------------------------------------------------------
double pow( int x, int n ) { 
	return ( pow( (double)x, (double)n ) ); 
}


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
		std::cerr << "lg : Sorry, I'm a hack and only compute approximate logs of positive integers" << std::endl;
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
// name: int compare( const void *arg1, const void *arg2 )
// desc: compare two values...for use in FindEpsilon
//		 This could have been a member function of Treesynth, but then I 
//		 couldn't figure out the syntax for passing it to FindEpsilon
//-----------------------------------------------------------------------------
int compare( const void *arg1, const void *arg2 )
{
    TS_FLOAT left = *( (TS_FLOAT *)arg1 );
    TS_FLOAT right = *( (TS_FLOAT *)arg2 );

    if( left < right ) return -1;
    else if( left == right ) return 0;
    else return 1;
}


Node::~Node() {SAFE_DELETE_ARRAY( cs );}

//-----------------------------------------------------------------------------
// Tree 
//-----------------------------------------------------------------------------

// Problem: dynamically allocating memory to tree, eg tree = new TS_FLOAT[size], used
//			to screw up dwt, so that's why we used MAX_TREE_SIZE. 

// Notes:	-values[0] stores the approximation
//			-values[1] stores the root or lowest level of detail, denoted by "level 0"
//			-This is confusing but it could be worse (for example without these notes).
//			-The number of nodes in a level is 2^level
//			-The sum of the number of nodes in previous levels, is the point where 
//			the next level starts, and is also 2^level.

//-----------------------------------------------------------------------------
// name: Tree()
// desc: Tree constructor
//-----------------------------------------------------------------------------
Tree::Tree()
{
	m_nodes_ = NULL;
	m_values_ = NULL;
	m_size_ = 0;
	m_levels_ = 0;
}


//-----------------------------------------------------------------------------
// name: ~Tree()
// desc: Tree destructor
//-----------------------------------------------------------------------------
Tree::~Tree()
{
	shutdown();
}


//-----------------------------------------------------------------------------
// name: bool initialize( TS_UINT levels )
// desc: initialize Tree data
//-----------------------------------------------------------------------------
bool Tree::initialize( TS_UINT levels )
{
	if( levels < 1 ) {
		std::cerr << "Tree::initialize : levels (" << levels << ") must be > 1" << std::endl;
		return false;
	}

	if( levels > 20 ) {
		std::cerr << "Tree::intialize : levels = " << levels
				  << " is WAAAAY too big for me. Make it smaller. Retrench!" << std::endl;
		return false;
	}

	m_levels_ = levels;
	m_size_ = 1 << levels; //pow( 2, levels );

	m_nodes_ = new Node[m_size_ + 1];
	m_values_ = new TS_FLOAT[m_size_ + 1]; 
    this->zero();

	// initialize Node data
	m_nodes_[0].value = &m_values_[0];
	for( int i = 1; i < m_size_; i++ ) {
		m_nodes_[i].parent = ( i > 1 ) ? &m_nodes_[i/2] : NULL;
		m_nodes_[i].left =	( i < m_size_ / 2 ) ? &m_nodes_[2*i] : NULL;
		m_nodes_[i].right = ( i < m_size_ / 2 ) ? &m_nodes_[2*i + 1] : NULL;
		m_nodes_[i].value = &m_values_[i];
	}

	return true;
}


//-----------------------------------------------------------------------------
// name: void shutdown()
// desc: delete stuff
//-----------------------------------------------------------------------------
void Tree::shutdown()
{
    /*if( m_nodes_ )
		delete [] m_nodes_;
	m_nodes_ = NULL;

	if( m_values_ )
		delete [] m_values_;
    m_values_ = NULL;*/

    SAFE_DELETE_ARRAY( m_nodes_ );
    SAFE_DELETE_ARRAY( m_values_ );
}


//-----------------------------------------------------------------------------
// name: void zero()
// desc: zero the arrays
//-----------------------------------------------------------------------------
void Tree::zero()
{
    if( m_nodes_ ) 
        for( int i = 0; i <= m_size_; i++ )
            if( m_nodes_[i].value != NULL )
                *(m_nodes_[i].value) = 0;

    if( m_values_ )
        memset( m_values_, 0, (m_size_ + 1) * sizeof( TS_FLOAT ) );
}


//-----------------------------------------------------------------------------
// name: TS_FLOAT getValue( TS_UINT level, TS_UINT offset )
// desc: get the value at a specified node
//-----------------------------------------------------------------------------
inline TS_FLOAT Tree::getValue( TS_UINT level, TS_UINT offset )
{
#if defined(__TSE_DEBUG__)
	if( level > m_levels_ ) {
		std::cerr << "Tree::getValue : invalid level " << level << std::endl;
		return 0;
	}

	if( offset >= ( 1 << level ) ) {	//pow( 2, level )
		std::cerr << "Tree::getValue : invalid offset " << offset << std::endl;
		return 0;
	}
#endif

	return m_values_[( 1 << level ) + offset];	//pow( 2, level )
}


//-----------------------------------------------------------------------------
// name: Node * getNode( TS_UINT level, TS_UINT offset )
// desc: get the specified node itself
//-----------------------------------------------------------------------------
inline Node * Tree::getNode( TS_UINT level, TS_UINT offset )
{
	if( level > m_levels_ ) {
		std::cerr << "Tree::getNode : invalid level " << level << std::endl;
		return 0;
	}

	if( offset >= ( 1 << level ) ) {	// pow( 2, level )
		std::cerr << "Tree::getNode : invalid offset " << offset << std::endl;
		return 0;
	}

	return &( m_nodes_[ (int)( 1 << level ) + offset] );	// pow( 2, level )
}


//-----------------------------------------------------------------------------
// name: TS_FLOAT * values()
// desc: get the list of values (without unnecessary node stuff)
//-----------------------------------------------------------------------------
TS_FLOAT * Tree::values()
{
	return m_values_;
}


//-----------------------------------------------------------------------------
// name: void setValues( TS_FLOAT * values, TS_UINT size )
// desc: writes values into the tree so that they aren't all zero
//-----------------------------------------------------------------------------
void Tree::setValues( TS_FLOAT * values, TS_UINT size )
{
	if( size != m_size_ ) {	
		// can add zero padding or cutoff later - if EVERYTHING ELSE works
		std::cerr << "Tree::setValues : size of data does not match tree size" << std::endl;
		exit(1);
	}

	memcpy( m_values_, values, size * sizeof(TS_FLOAT) );
}


//-----------------------------------------------------------------------------
// name: TS_UINT getSize()
// desc: access m_size_
//-----------------------------------------------------------------------------
TS_UINT Tree::getSize() { return m_size_; }


//-----------------------------------------------------------------------------
// name: TS_UINT getLevels()
// desc: access m_levels_
//-----------------------------------------------------------------------------
TS_UINT Tree::getLevels() { return m_levels_; }


//-----------------------------------------------------------------------------
// name: void resetLevels( TS_UINT levels )
// desc: reset tree size and levels
//       assume tree had maximum size assigned at the beginning, otherwise
//         very bad things will happen very fast.
//-----------------------------------------------------------------------------
void Tree::resetLevels( TS_UINT levels )
{
    if( (1 << levels) <= CUTOFF ) {
        m_levels_ = levels;
        m_size_ = 1 << levels;
    }
}


//-----------------------------------------------------------------------------
// Treesynth (gasp)
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// name: Treesynth()
// desc: Treesynth constructor
//-----------------------------------------------------------------------------
Treesynth::Treesynth()
{
	tree = NULL;
	lefttree = NULL;
	tnew_data = NULL;
	tnew = NULL;
	cands = NULL;
	in = NULL;
	S = NULL;
	L = NULL;
	synsig = NULL;
	srand(time(0));
}


//-----------------------------------------------------------------------------
// name: ~Treesynth()
// desc: Treesynth destructor
//-----------------------------------------------------------------------------
Treesynth::~Treesynth()
{
	shutdown();
}


//-----------------------------------------------------------------------------
// name: bool initialize()
// desc: initialize knobs and other stuff after tree has been set, so it
//		 knows how many levels there are
//-----------------------------------------------------------------------------
bool Treesynth::initialize()
{
	// Check tree
	if( !tree ) {
		std::cerr << "Treesynth::initialize : First set tree, then call me." << std::endl;
		return false;
	}

	// Set knobs
	npredecessors = 3;
	kfactor = 0.3f;
	randflip = false;
	percentage = .25f;
	epsilon = 0;
	ancfirst = true;
	startlevel = 1;

	int levels = tree->getLevels();
	stoplevel = levels - 2;
	if( stoplevel < startlevel )
		stoplevel = levels;

	// Delete stuff before reassigning
	SAFE_DELETE( tnew );
	SAFE_DELETE( tnew_data );
	SAFE_DELETE_ARRAY( synsig );
	SAFE_DELETE_ARRAY( cands );
	SAFE_DELETE_ARRAY( in );
	SAFE_DELETE_ARRAY( L );
	SAFE_DELETE_ARRAY( S );

	// Set remaining trees
	tnew = new Tree;
	tnew->initialize( levels );
	tnew_data = new Tree;
	tnew_data->initialize( levels );

	// Set other stuff
	int s = 1 << ( levels - 1 );	// pow( 2, levels - 1 )
	synsig = new TS_FLOAT[tree->getSize()];
	cands = new int[s];
	in = new bool[s];
	L = new short[s];
	S = new short[s];

	return true;
}


//-----------------------------------------------------------------------------
// name: void shutdown()
// desc: delete stuff
//-----------------------------------------------------------------------------
void Treesynth::shutdown()
{
/*	if( tree )
		delete tree;
	if( tnew )
		delete tnew;
	if( lefttree )
		delete lefttree;
	if( tnew_data )
		delete tnew_data;

	if( cands )
		delete [] cands;
	if( in )
		delete [] in;
	if( L )
		delete [] L;
    if( S )
		delete [] S; */

    SAFE_DELETE( tree );
    SAFE_DELETE( lefttree );
    SAFE_DELETE( tnew );
    SAFE_DELETE( tnew_data );
    
    SAFE_DELETE_ARRAY( cands );
    SAFE_DELETE_ARRAY( in );
    SAFE_DELETE_ARRAY( L );
    SAFE_DELETE_ARRAY( S );

    SAFE_DELETE_ARRAY( synsig );
}


//-----------------------------------------------------------------------------
// name: double FindEpsilon( Tree * the_tree, int size, double P );
// desc: finds threshold epsilon, based on percentage of values preferred to
//		 lie within epsilon of some given node value (randomness measure)
//-----------------------------------------------------------------------------
double Treesynth::FindEpsilon( Tree * thetree, int size, double P )
{
	assert( P > 0.0 ); // delete this later

    TS_FLOAT * ourtree = new TS_FLOAT[size];
    memcpy( ourtree, thetree->values(), size * sizeof(TS_FLOAT) );

    // find the L1 norm
    for( int i = 1; i < size; i++ )
        ourtree[i] = fabs(ourtree[i]);

    // sort it
    qsort( ourtree, size, sizeof(TS_FLOAT), compare );

    int index = (int)(P * size + .5);
    if( index >= size ) index = size-1;

	//std::cout << size << " " << P << " " << index << std::endl;

    double epsilon = 2.0 * (double)ourtree[index];

    SAFE_DELETE_ARRAY( ourtree );
    return epsilon;
}


//-----------------------------------------------------------------------------
// name: short Ancestors( int level, int offset, int csl )
// desc: Part 1 of CandidateSet; ancestor search
//-----------------------------------------------------------------------------
short Treesynth::Ancestors( int level, int offset, int csl )
{
	// Part 1
	// Compare ancestors of given node in tnew_data with ancestors of 
	// selected nodes in tree (AND LEFTTREE?)
	TS_FLOAT ancTnew, ancTree; // float
	
	for( int w = 0; w < csl; w++ ) {
		TS_FLOAT sum = 0;
		L[w] = 0;

		if( in[w] ) { // if w is in the existing candidate set
			for( short v = level; v >= 1; v-- ) { 
				ancTnew = tnew->getValue( v, offset / ( 1 << (level-v) ) );	// pow( 2, level-v )
				ancTree = tree->getValue( v, cands[w] / ( 1 << (level-v) ) );	// pow( 2, level-v )
																			
				TS_FLOAT s = ancTnew - ancTree; // float
				if(s < 0)		// convoluted way because apparently
					s *= -1;	// abs(_) converts to an integer
				sum += s;

				if( (float)( (sum)/(level-v+1) ) <= epsilon)	// epsilon -> epsilon/N?
					L[w]++;
				else
					break;
			} // end of for v
		} // end of if
	} // end of for w

	short M1 = maximum( L, csl );
	return M1;
}


//-----------------------------------------------------------------------------
// name: short Predecessors( int level, int offset, int csl )
// desc: Part 2 of CandidateSet; predecessor search
//-----------------------------------------------------------------------------
short Treesynth::Predecessors( int level, int offset, int csl )    
{
	// Part 2
	// similar to part 1 but with predecessors instead of ancestors
	
	// determine number of predecessors to check in general for given node in TNEW
	int npred = our_min( npredecessors, offset );
	
	TS_FLOAT predTnew, predTree;
	for( int q = 0; q < csl; q++ ) {
		S[q] = 0;

		if( in[q] ){			
			// find and compare np predecessors of tree and tnew
			for( int z = 1; z <= npred; z++ ) { // npred instead of np
				// figure out where to get stuff from				
				predTnew = tnew->getValue( level, offset - z );

				if( cands[q] - z >= 0 )
					predTree = tree->getValue( level, cands[q] - z );
				else if( (1 << level) + cands[q] - z >= 0 && lefttree ) // pow( 2, level )
					predTree = tree->getValue( level, (1 << level) + cands[q] + z ); // pow( 2, level )
				else break;

				TS_FLOAT diff = predTnew - predTree;
				if( diff < 0 )
					diff *= -1;

				if( diff <= epsilon ) 
					S[q]++;
				else
					break;
			} // end of for z
		} // end of else
	} // end of for q

	short M2 = maximum( S, csl );
	return M2;
}


//-----------------------------------------------------------------------------
// name: void CandidateSet( int lev, int offset );
// desc: the very important function for doing the actual learning
//-----------------------------------------------------------------------------
void Treesynth::CandidateSet( int level, int offset ) {
	// Find candidate set and update node in tnew_data

	// Create empty candidate set
	int csl = 2 * tnew_data->getNode( level, offset )->parent->cslength;

	int i;
	for( i = 0; i < csl/2; i++) {
		// insert candidate
		cands[i] = 2 * tnew_data->getNode( level, offset )->parent->cs[i];
		cands[i+csl/2] = 2 * tnew_data->getNode( level, offset )->parent->cs[i] + 1;
			
		// verify candidate
		if( cands[i] < ( 1 << level ) )	// pow( 2, level )	
			in[i] = true;
		else  
			in[i] = false;
		
		if( cands[i+csl/2] < ( 1 << level ) )	// pow( 2, level ) 
			in[i+csl/2] = true;
		else 
			in[i+csl/2] = false;
	}
	
	// Do ancestor or predecessor processing
	short M1 = 0, M2 = 0;
	if( ancfirst ) {
		M1 = Ancestors( level, offset, csl );
		for( i = 0; i < csl; i++ )
			if( L[i] != M1 )
				in[i] = false;
		M2 = Predecessors( level, offset, csl );
	}
	else {
		M2 = Predecessors( level, offset, csl );
		for( i = 0; i < csl; i++ )
			if( S[i] != M2 )
				in[i] = false;
		M1 = Ancestors( level, offset, csl );
	}

	// Return all nodes s.t. L = M1 and S = M2
    SAFE_DELETE_ARRAY( tnew_data->getNode( level, offset )->cs );
	tnew_data->getNode( level, offset )->cs = new TS_UINT[csl];
	int index = 0;
	for( int c = 0; c < csl; c++ ) {
		if( in[c] && L[c] == M1 && S[c] == M2 ) {
			tnew_data->getNode( level, offset )->cs[index++] = cands[c];
		}
	}
	tnew_data->getNode( level, offset )->cslength = index;

	// Back up if cslength is not positive (which should not happen)
	if( index <= 0 ) {
		std::cerr << "Treesynth::CandidateSet : Uh-oh " << level << "-" 
				  << offset << " " << index << std::endl;
		// copy all candidates regardless of whether they passed the test or not
		// (these are all the children of the nodes in the parent's candidate set)
		index = 0;
		for( int c = 0; c < csl; c++ ) 
			tnew_data->getNode( level, offset )->cs[index++] = cands[c];
		
		tnew_data->getNode( level, offset )->cslength = index;
	}

	// garbage collection
	if(offset % 2 == 1)
		SAFE_DELETE_ARRAY( tnew_data->getNode( level, offset )->parent->cs );
        //delete [] tnew_data->getNode( level, offset )->parent->cs;
	if(level == tree->getLevels() - 2 && offset > 0)
        SAFE_DELETE_ARRAY( tnew_data->getNode( level, offset - 1 )->cs );
		//delete [] tnew_data->getNode( level, offset - 1 )->cs;

} // end of CandidateSet


//-----------------------------------------------------------------------------
// name: bool setup( void )
// desc: set up original wavelet trees, calculate threshold, initialize root
//		 of new tree
//----------------------------------------------------------------------------- 
bool Treesynth::setup( void )
{
	// Get value arrays of trees
	float * tnew_values = tnew->values();
	float * tree_values = tree->values();

	// Wavelet decompositions
	pwtset( 10 );

	wt1( tree_values, tree->getSize(), 1, *pwt );
	if( lefttree ) {
		float * lefttree_values = lefttree->values();
		wt1( lefttree_values, ( 1 << tree->getSize() ), 1, *pwt );	// pow( 2, tree->getSize() )
	}

	// Calculate nearness threshold (epsilon) and print related values
	epsilon = FindEpsilon( tree, tree->getSize(), percentage );
    
    //std::cout << "p: " << percentage << std::endl;
	//std::cout << "epsilon: " << epsilon << std::endl;
	//std::cout << "kfactor: " << kfactor << std::endl;
	
	// Copy root of tnew, since that's always the same

	tnew_values[0] = tree_values[0];	// approximation
	tnew_values[1] = tree_values[1];	// root

	// Return
	return true;
}


//-----------------------------------------------------------------------------
// name: void synth( void ) 
// desc: does wavelet tree learning to build the new tree, then does the 
//		 inverse discrete wavelet transform on the new tree to get synthesized
//		 signal
//-----------------------------------------------------------------------------
void Treesynth::synth( void )
{
	// Set up candidate set for root (also always the same, but gets deleted)
	if( tnew_data->getNode( 0, 0 )->cs == NULL )
		tnew_data->getNode( 0, 0 )->cs = new TS_UINT[1];
	tnew_data->getNode( 0, 0 )->cs[0] = 0;
	tnew_data->getNode( 0, 0 )->cslength = 1;

	// Synthesize new tree
	if( startlevel <= 1 ) {
		for(int n = 0; n < 2; n++)	{	// level 1 (the level next to the root)	
			// try copying randomly instead of in order
			// (so instead of LR it could be LL or RR or RL)
			if( randflip ) {
				int random = (int)( 2 * rand()/( RAND_MAX + 1.0 ) );
				*(tnew->getNode( 1, n )->value) = tree->getValue( 1, random );
			}
			else
				*(tnew->getNode( 1, n )->value) = tree->getValue( 1, n );
		}
	}
	else {
		int l, n; 
		for( l = 1; l <= startlevel; l++ ) {
			int length = ( 1 << l ); // (int)pow( 2, l )
			for( n = 0; n < length; n++ ) {
				*(tnew->getNode( l, n )->value) = tree->getValue( l, n );
				if( l == startlevel - 1 ) {
					SAFE_DELETE_ARRAY( tnew_data->getNode( l, n )->cs ); 
					tnew_data->getNode( l, n )->cs = new TS_UINT[length];
					tnew_data->getNode( l, n )->cslength = length;
					for( int q = 0; q < length; q++ )
						tnew_data->getNode( l, n )->cs[q] = q;
				}
			}
		}
	}

	// Breadth-first part
    int lev;
	for( lev = startlevel; lev <= stoplevel; lev++ ) {				
		if( lev >= tree->getLevels() - 1 )
			break;
		//std::cout << "Processing level " << lev << "   ";
		//std::cout << "k is " << (npredecessors = ( 1 << lev ) * kfactor) << std::endl;	// (int)(pow( 2, lev )
		for( int offset = 0; offset < ( 1 << lev ); offset++ ) {	// (int)(pow( 2, lev )
			CandidateSet( lev, offset ); 
			if( tnew_data->getNode( lev, offset )->cslength == 0 ) 
				std::cerr << "Double Uh-oh " << lev << "-" << offset << std::endl;

			// Randomly choose a node from candidate set and steal its children
			int randPick = (int)( rand()/(RAND_MAX + 1.0) * tnew_data->getNode( lev, offset )->cslength );
			randPick = tnew_data->getNode( lev, offset )->cs[randPick];						
			
			// left child
			*(tnew->getNode( lev, offset )->left->value) = tree->getValue( lev+1, 2*randPick );
			
			// right child: changed to make sure nodes referred to are within limits
			if( 2*offset + 1 < ( 1 << (lev + 1) ) ) {	// pow( 2, lev+1 )
				if( 2*randPick + 1 < ( 1 << (lev + 1) ) )	// pow( 2, lev+1 )
					*(tnew->getNode( lev, offset )->right->value) = tree->getValue( lev+1, 2*randPick + 1 );	

				else {																						
					*(tnew->getNode( lev, offset )->right->value) = tree->getValue( lev+1, 2*randPick );
					
					if( randPick > 0 ) 
						*(tnew->getNode( lev, offset )->left->value) = tree->getValue( lev+1, 2*randPick - 1 );
				}
			}

			// if it's stoplevel, copy all descendants
			if( lev == stoplevel ) {
				int l, m = 2, p;
				for( l = lev + 2; l < tree->getLevels(); l++ ) {
					m = 2*m;
					for( p = 0; p < m; p++ )
						*(tnew->getNode( l, m*offset + p )->value) = tree->getValue( l, m*randPick + p );				
				}
			} // yeah...
		}
	}

	// Reconstruct signal from new wavelet tree
	TS_UINT size = tnew->getSize();
	memcpy( synsig, tnew->values(), size * sizeof(TS_FLOAT) );
    wt1( synsig, size, -1, *pwt );
}


//-----------------------------------------------------------------------------
// name: Tree * outputTree()
// desc: returns tnew (in wavelet tree form)
//-----------------------------------------------------------------------------
Tree * Treesynth::outputTree()
{
	return tnew;
}


//-----------------------------------------------------------------------------
// name: TS_FLOAT * outputSignal()
// desc: returns the synthesized signal
//-----------------------------------------------------------------------------
TS_FLOAT * Treesynth::outputSignal()
{
    return synsig;
}


//-----------------------------------------------------------------------------
// name: void resetTreeLevels( TS_UINT lev );
// desc: reset levels of all trees in this Treesynth instance
//-----------------------------------------------------------------------------
void Treesynth::resetTreeLevels( TS_UINT lev )
{
    if( lefttree ) {
        std::cerr << "Sorry, cannot reset levels because lefttree will "
                  << "get messed up" << std::endl;
        return;
    }

    if( tree )
        tree->resetLevels( lev );
    if( tnew )
        tnew->resetLevels( lev );
    if( tnew_data )
        tnew_data->resetLevels( lev );
}


//-----------------------------------------------------------------------------
// name: void Treesynth::resetTrees()
// desc: zero the trees
//-----------------------------------------------------------------------------
void Treesynth::resetTrees()
{
    if( lefttree )
        lefttree->zero();
    if( tree )
        tree->zero();
    if( tnew )
        tnew->zero();
    if( tnew_data )
        tnew_data->zero();
}



//-----------------------------------------------------------------------------
// TreesynthIO!!!!!!!!!!
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// name: TreesynthIO()
// desc: TreesynthIO constructor
//-----------------------------------------------------------------------------
TreesynthIO::TreesynthIO()
{
	// initialize everything
	strcpy( ifilename, "orig.wav" );
	strcpy( ofilename, "syn.wav" );
    strcpy( leftfile, "" );

	//rm_mode = RM_FORWARD | RM_WRAP;
	rm_mode = RM_FORWARD | RM_BOUNCE; 
	nsamples = 0; // the size of tree

	leftinit = false;

	rm_next_pos = 0;
	rm_next_length = 0;

	g_write_index = 0;
	g_read_index = 0;
	
	g_audio_begin = NULL;
	g_audio_end = NULL;
	
	g_buffer_size = TS_BUFFER_SIZE;
	g_data_count = 0;
	g_max_data_count = TS_BIG_BUFFER_COUNT;
	g_ready = 0;
	g_callbacking = 0;
	
	g_srate = 0;

    sfread = NULL;
    sfwrite = NULL;

    write_to_file = false;
    write_to_buffer = true;

    // zero
    memset( g_big_buffer, 0, sizeof(TS_FLOAT) * TS_BIG_BUFFER_COUNT * TS_BIG_BUFFER_SIZE );

    // no samples
    for( int i = 0; i < TS_BIG_BUFFER_COUNT; i++ )
        g_buffer_samples[i] = 0;

	std::cout << "TreesynthIO::TreesynthIO : You have created a TreesynthIO (!) "
		<< "object. Some default values are: " << std::endl;
	std::cout << "Input file name - " << ifilename << " ; change by setting ifilename "
		<< "\nOutput file name - " << ofilename << " ; change by setting ofilename "
		<< "\nLeft file name - " << leftfile << " ; change by setting leftfile "
		<< "\nRead mode - " << rm_mode << " (yup) ; change by setting rm_mode (see TheReadModes) "
		<< "\nSamples - " << nsamples << " ; change by setting nsamples " << std::endl;
}


//-----------------------------------------------------------------------------
// name: ~TreesynthIO()
// desc: TreesynthIO constructor
//-----------------------------------------------------------------------------
TreesynthIO::~TreesynthIO()
{
	while( g_callbacking );

	if( sfread )
        sf_close( sfread );

    if( sfwrite )
        sf_close( sfwrite );
        
    std::cout << "Goodbye!" << std::endl;    
}


//-----------------------------------------------------------------------------
// name: clear()
// desc: almost copy of constructor (don't ask)
//-----------------------------------------------------------------------------
/*void TreesynthIO::clear()
{
	strcpy( ifilename, "" );
	strcpy( ofilename, "" );

	rm_mode = RM_FORWARD | RM_STATIONARY;
	nsamples = 0; // the size of tree

	leftinit = false;

	rm_next_pos = 0;
	rm_next_length = 0;

	g_write_index = 0;
	g_read_index = 0;
	
	g_audio_begin = NULL;
	g_audio_end = NULL;
	
	g_buffer_size = TS_BUFFER_SIZE;
	g_data_count = 0;
	g_max_data_count = TS_BIG_BUFFER_COUNT;
	g_ready = 1;
	
	g_srate = 0;
}*/


//-----------------------------------------------------------------------------
// name: int audio_cb( char * buffer, int buffer_size, void * user_data )
// desc: audio callback
//-----------------------------------------------------------------------------
int TreesynthIO::m_audio_cb( char * buf, int buffer_size, void * user_data )
{
	// don't delete me now
	g_callbacking = 1;

	TS_FLOAT * buffer = (TS_FLOAT *) buf;

    // if not ready, leave (used to play wavelet tree, but that's harder now)
    if( !g_ready ) // waiting for all buffers to be filled before playing any sound
    {
        memset( buffer, 0, buffer_size * sizeof(TS_FLOAT) );
        g_ready = 1;
    }

    int remaining = buffer_size;
    int offset = 0;

    while( remaining > 0 )
    {
        while( !g_data_count )
        {
            usleep( 2000 );
        }
    
		if( !g_audio_begin )
        {
            g_audio_begin = g_big_buffer[g_read_index]; 
            g_audio_end = g_audio_begin + g_buffer_samples[g_read_index]; //nsamples;
			g_buffer_samples[g_read_index] = 0;
            g_read_index++;
            g_read_index %= g_max_data_count;
        }
    
        // we have data, but do we have >= buffer_size? otherwise it's not enough :(
        int tocopy = min( remaining, g_audio_end - g_audio_begin );

        // copy the data out
        memcpy( buffer + offset, g_audio_begin, tocopy * sizeof(TS_FLOAT) );
        // increment the read pointer
        g_audio_begin += tocopy;

        // check to see if at end
        if( g_audio_begin >= g_audio_end )
        {
            // reset
            g_audio_begin = NULL;
            g_data_count--;
	    }

        // update remaining
        remaining -= tocopy; 
        // update offset ( i bet they could be converged to one variable, but too bad)
        offset += tocopy;
    }

	// done now
	g_callbacking = 0;

    return 0;
}



/*-----------------------------------------------------------------------------
// name: bool audio_initialize( TS_UINT srate, int (*audio_cb) (char *, int, void *) );
// desc: set up audio capture and playback and initializes any application data
//-----------------------------------------------------------------------------
bool TreesynthIO::audio_initialize( int (*audio_cb) (char *, int, void *), TS_UINT srate )
{
    if( g_audio )
        return TRUE;

	std::cerr << "TreesynthIO::audio_initialize : I hope you've passed as audio_cb "
		<< "some function that calls m_audio_cb." << std::endl;

    Stk::setSampleRate( g_srate = srate );

    try
    {
        // open the audio device for capture and playback
        g_audio = new RtAudio( 0, 1, 0, 0, RTAUDIO_FLOAT32, g_srate, &g_buffer_size, 4 );
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
}*/


//-----------------------------------------------------------------------------
// name: void set_next_pos( const char * filename )
// desc: set next position to read from in source sound file
//-----------------------------------------------------------------------------
void TreesynthIO::set_next_pos( const char * filename )
{
    int action = rm_mode & RM_ACTION_MASK;
    int direction = rm_mode & RM_DIRECTION_MASK;
    
	//std::cerr << "TreesynthIO::set_next_pos : reading samples '" << rm_next_pos << "' - '" << (rm_next_pos+rm_next_length) << "'" << std::endl;

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
            rm_next_pos = (int)( rand()/(float)RAND_MAX * (readinfo.frames-rm_next_length) );
            if( rm_next_pos < 0 ) rm_next_pos = 0;
            break;
        default:
			std::cerr << "TreesynthIO::set_next_pos : no!!!!!!" << std::endl;
            exit(1);
    }	

    // passed the end
    if( rm_next_pos >= readinfo.frames ) {
        switch( action )
        {
            case RM_STOP:
            {
                std::cerr << "TreesynthIO::set_next_pos : cannot read no mo' file '" << filename << "', stopping"<< std::endl;
                char x[256];
                cin.getline( x, 256 );
                exit(1);
                break;
            }
            case RM_WRAP:
                std::cerr << "TreesynthIO::set_next_pos : warping!" << std::endl;
                rm_next_pos = 0;
                break;
            case RM_BOUNCE:
                std::cerr << "TreesynthIO::set_next_pos : bouncing!" << std::endl;
                assert( direction == RM_FORWARD );
                rm_mode = RM_BACKWARD | action;
                rm_next_pos -= rm_next_length*2;
                break;
            default:
                std::cerr << "TreesynthIO::set_next_pos : no!!!!" << std::endl;
                exit(1);
        }
	}
    else if( rm_next_pos < 0 && direction == RM_BACKWARD ) // passed the beginning
    {
        switch( action )
        {
            case RM_STOP:
            {
                std::cerr << "TreesynthIO::set_next_pos : cannot read no mo' file '" << filename << "', stopping"<< std::endl;
                char x[256];
                cin.getline( x, 256 );
                exit(1);
                break;
            }
            case RM_WRAP:
                std::cerr << "TreesynthIO::set_next_pos : warping!!" << std::endl;
                rm_next_pos = readinfo.frames - ( readinfo.frames % rm_next_length );
                break;
            case RM_BOUNCE:
                std::cerr << "TreesynthIO::set_next_pos : boucing!!" << std::endl;
                assert( direction == RM_BACKWARD );
                rm_mode = RM_FORWARD | action;
                rm_next_pos = rm_next_length;
                break;
            default:
                std::cerr << "TreesynthIO::set_next_pos : no!!!!!" << std::endl;
                exit(1);
        }
    }

    // make sure we are legal
    if( rm_next_pos < 0 )
        rm_next_pos = 0;
}


//-----------------------------------------------------------------------------
// name: int ReadSoundFile( char filename[], TS_FLOAT * data, int datasize )
// desc: Reads given sound file into data array
//-----------------------------------------------------------------------------
int TreesynthIO::ReadSoundFile( char filename[], TS_FLOAT * data, int datasize )
{
	if( !sfread ) {
		sfread = sf_open( filename, SFM_READ, &readinfo );
        if( !sfread )
        {
        	std::cerr << "TreesynthIO::ReadSoundFile : cannot open file '" << filename << "', quitting" << std::endl;
            //char x[256];
			//std::cin.getline( x, 256 );
			//exit(1);
            return 0;
		}
	}

    // this is probably right
    rm_next_length = datasize;
	sf_seek( sfread, rm_next_pos, SEEK_SET );
	
    int itemsread = sf_read_float( sfread, data, datasize );
	set_next_pos( filename );
    // rt audio
    /*if( !audio_initialize( readinfo.samplerate ) ) {	// 44100
        std::cerr << "TreesynthIO::ReadSoundFile : cannot open audio interface, quitting" << std::endl;
		char x[256];
		std::cin.getline( x, 256 );
		exit(1);
    }*/
    return itemsread;
}


//-----------------------------------------------------------------------------
// name: TS_UINT get_srate()
// desc: return sample rate
//-----------------------------------------------------------------------------
TS_UINT TreesynthIO::get_srate()
{
	return readinfo.samplerate;
}


//-----------------------------------------------------------------------------
// name: int WriteSoundFile( char filename[], TS_FLOAT * data, int datasize );
// desc: writes synthesized sound to file or to buffer for audio_cb to play
//-----------------------------------------------------------------------------
int TreesynthIO::WriteSoundFile( char filename[], TS_FLOAT * data, int datasize )
{
    // bad - replace by right solution
    nsamples = datasize;

    // data will always fit in one buffer because buffer size = max tree size (CUTOFF)

    if( this->write_to_buffer )
    {
        if( g_data_count >= g_max_data_count )
            return 0;

        // set #samples for the write buffer
        g_buffer_samples[g_write_index] = nsamples;

		// set the buffer to write
		TS_FLOAT * next_buffer = g_big_buffer[g_write_index++];
        g_write_index %= g_max_data_count;

        // copy the data
        memcpy( next_buffer, data, datasize * sizeof(TS_FLOAT) );
    
        // increment data count
        g_data_count++;
	}
    
    // if it's also writing to buffer, it doesn't get here until after the write succeeds
    // so the same data should not be written more than once to the file
    if( this->write_to_file )
    {
        if( !sfwrite ) {
            writeinfo = readinfo;
			sfwrite = sf_open( filename, SFM_WRITE, &writeinfo );

			if( !sfwrite )
			{
				std::cerr << "TreesynthIO::WriteSoundFile : cannot open file '" << filename << "', quitting" << std::endl;
				//char x[256];
				//std::cin.getline( x, 256 );
				//exit(1);
                return 2;
			}
		}

		int itemswritten = sf_write_float( sfwrite, data, datasize );
		if( itemswritten <= 0 ) {
			std::cerr << "TreesynthIO::WriteSoundFile : cannot write to file '" << filename << "', quitting" << std::endl;
            //char x[256];
			//std::cin.getline( x, 256 );
			//exit(3);
            return 2;
        }
    }
    
    // wrote something
    return 1;
}
