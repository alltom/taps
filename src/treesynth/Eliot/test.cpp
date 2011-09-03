#include "Eliot.h"


int audio_cb( char *buffer, int buffer_size, void *user_data );

TreesynthIO ts_io;
Treesynth ts;
Tree * ts_tree;

int main( int argc, char ** argv ) {
	ts_tree = new Tree();
	ts_tree->initialize( lg( CUTOFF ) );
	
	// Find better way of parsing arguments. (Meanwhile, just don't do it.)

	// Why did treesynth get so slow?!!!??!?!?!?!?!?!??!?!?
	int samples = ts_io.ReadSoundFile( ts_io.ifilename, ts_tree->values(), CUTOFF );
	std::cerr << "read it\n";
	ts.tree = ts_tree;
	ts.initialize();
	ts_io.audio_initialize( audio_cb );

	while( true ) {
		if( ts.setup() )
			ts.synth();
		ts_io.WriteSoundFile( ts_io.ofilename, ts.outputSignal(), ts.tree->getSize() );

        if( ts.lefttree == NULL ) {
            ts.lefttree = new Tree();
            ts.lefttree->initialize( ts.tree->getLevels() );
        }
        memcpy( ts.lefttree->values(), ts.outputTree()->values(), sizeof(TS_FLOAT) * ts.outputTree()->getSize() );
        
        samples = ts_io.ReadSoundFile( ts_io.ifilename, ts.tree->values(), CUTOFF );
	}

	return 0;
}


int audio_cb( char *buffer, int buffer_size, void *user_data )
{
	return ts_io.m_audio_cb( buffer, buffer_size, user_data );
}