#include "OLA.h"
#include <iostream>

int olar_audio_cb( char *buffer, int buffer_size, void *user_data );

OlaRandom * olar;

int mainola( int argc, char ** argv ) {
	float datalen = 2; // seconds 
	float looplen = 0; // seconds
	int samplerate = 44100;
	char * ofilename = NULL;
	char * ifilename = "orig.wav"; 
	float randomness = 0;
	bool scale_amp = false;
	float mindist = 0; // seconds

	// read input
	int index = 1;
	while( index < argc ) 
	{
		// randomness
		if( !strncmp( argv[index], "-p", 2 ) )
			randomness = atof( argv[index] + 2 );
		// loop length
		else if( !strncmp( argv[index], "-i", 2 ) )
			looplen = atof( argv[index] + 2 );
		// output file name
		else if( !strncmp( argv[index], "-w", 2 ) ) 
			ofilename = argv[index] + 2;
		// input file name
		else if( !strncmp( argv[index], "-f", 2 ) )
			ifilename = argv[index] + 2; 
		// data length (average)
		else if( !strncmp( argv[index], "-s", 2 ) )
			datalen = atof( argv[index] + 2 ); 
		// min dist
		else if( !strncmp( argv[index], "-d", 2 ) )
			mindist = atof( argv[index] + 2 );
		// sample rate
		else if( !strncmp( argv[index], "-r", 2 ) )
			samplerate = atoi( argv[index] + 2 ); 
		// scale amp randomly
		else if( !strncmp( argv[index], "-a", 2 ) )
			scale_amp = true;
		// increment index
		index++;
	}
	
	// initialize
	olar = new OlaRandom();
	olar->initialize( ifilename, (int)(samplerate * datalen), randomness, (int)(samplerate * mindist), scale_amp );
	olar->audio_initialize( olar_audio_cb, samplerate );

	if( ofilename )
		strcpy( olar->ofilename, ofilename );
	olar->write_to_file = true;

	int count = 0;

	int loopend = (int)(looplen * samplerate);
	fprintf( stderr, "# samples to write: %d\n", loopend );
	while( !loopend || count < loopend ) {
		count += olar->next_segment();
	}
	fprintf( stderr, "# samples written: %d\n", count );

	return 0;
}


int olar_audio_cb( char *buffer, int buffer_size, void *user_data )
{
	return olar->m_audio_cb( buffer, buffer_size, user_data );
}
