#include "Eliot.h"
#include <iostream>

int audio_cb( char *buffer, int buffer_size, void *user_data );

TreesynthIO * ts_io;
Treesynth ts;
Tree * ts_tree;

int main( int argc, char ** argv ) {
	int datasize = 1 << 12; 
	int hopsize = 0;
	int loopend = 0;
	int samplerate = 44100;
	float treePercentage = -1; 
	char * ofilename = NULL;
	char * ifilename = "orig.wav"; 

	// read input
	int index = 1;
	while( index < argc ) 
	{
		// tree percentage
		if( !strncmp( argv[index], "-p", 2 ) )
			treePercentage = atof( argv[index] + 2 );
		// loop end
		else if( !strncmp( argv[index], "-i", 2 ) )
			loopend = atoi( argv[index] + 2 );
		// output file name
		else if( !strncmp( argv[index], "-w", 2 ) ) 
			ofilename = argv[index] + 2;
		// input file name
		else if( !strncmp( argv[index], "-f", 2 ) )
			ifilename = argv[index] + 2; 
		// data size
		else if( !strncmp( argv[index], "-s", 2 ) )
			datasize = 1 << (atoi( argv[index] + 2 )); 
		// hop size
		else if( !strncmp( argv[index], "-h", 2 ) )
			hopsize = 1 << (atoi( argv[index] + 2 ));
		// sample rate
		else if( !strncmp( argv[index], "-r", 2 ) )
			samplerate = atoi( argv[index] + 2 ); 
		// increment index
		index++;
	}
	
	// initialize
	if( !hopsize ) 
		hopsize = datasize / 4;
	ts_io = ts.initialize( ifilename, datasize, hopsize );
	if( !ts_io )
		std::cerr << "Why is ts_io null?" << std::endl;
	ts_io->write_to_file = true;

	ts_io->audio_initialize( audio_cb, samplerate );

	if( treePercentage >= 0 )
		ts.UpdateTreePercentage( treePercentage );
	if( ofilename )
		strcpy( ts_io->ofilename, ofilename );
	int count = 0;

	while( !loopend || count++ < loopend ) {
		if( ts.setup() )
			ts.synth();
		ts_io->WriteSoundFile( ts_io->ofilename, ts.outputSignal(), ts.cur_tree->getSize() );
	}

	return 0;

/*	
	ts_tree = new Tree();
	ts_tree->initialize( 13 /*lg( CUTOFF )*/ /*);
	
	// Find better way of parsing arguments. (Meanwhile, just don't do it.)

	// Why did treesynth get so slow?!!!??!?!?!?!?!?!??!?!?
	int samples = ts_io.ReadSoundFile( ts_io.ifilename, ts_tree->values(), /*CUTOFF*//* 1 << 13 );
	std::cerr << "read it\n";
	ts.tree = ts_tree;
	ts.initialize();
	ts_io.audio_initialize( audio_cb );
	
	while( true ) {
		if( ts.setup() )
			ts.synth();
		ts_io.WriteSoundFile( ts_io.ofilename, ts.outputSignal(), ts.tree->getSize() );
		ts_io.ReadSoundFile( ts_io.ifilename, ts_tree->values(), 1 << 13 );
	}

	return 0;
*/

}


int audio_cb( char *buffer, int buffer_size, void *user_data )
{
	return ts_io->m_audio_cb( buffer, buffer_size, user_data );
}


/*

Meta Treesynth

  1. downsample and store in memory (skipping)
  2. decompose each stored chunk (in place)
  3. select a chunk 
	 a. first time, arbitrarily
	 b. otherwise, store a distance between chunks
	 c. randomly select a chunk within a maximum distance (just like selecting a node to copy)
	 d. dist(a, b) need not be equal to dist(b, a) because maybe the end of b is close to the beginning of a and not vice versa
  4. re-synthesize chunk
  5. upsample (hmm, skipping) and play re-synthesized chunk
	 a. may need ola
  6. goto 3

*/
