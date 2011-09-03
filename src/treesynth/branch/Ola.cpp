#include "OLA.h"
#include <iostream>
#include "taps_birdbrain.h"
#include "audicle_def.h"
#include "taps_sceptre.h"

#ifndef NULL
#define NULL 0
#endif


OlaRandom::OlaRandom()
{
	// initialize everything
	strcpy( ifilename, "orig.wav" );
	strcpy( ofilename, "syn.wav" );
    
	m_write_index = 0;
	m_read_index = 0;
	
	m_audio_begin = NULL;
	m_audio_end = NULL;
	
	m_last_buffer_size = 0;
	m_next_buffer_size = 0;
	m_last_start_index = 0;
	m_hopsize = 0;
	m_top = true;
	m_rt_buffer_size = OLAR_BUFFER_SIZE; 

	m_data_count = 0;
	m_max_data_count = OLAR_BIG_BUFFER_COUNT;
	m_ready = 0;
	m_callbacking = 0;
	
	m_srate = 0;
	m_winsize = 0;

    sfread = NULL;
    sfwrite = NULL;

	origbuffer = NULL;
	origsize = 0;

	randomness = 0;
	mindist = 0;
	scale_amp = false;
	segsize = 0;

    // zero
    memset( m_big_buffer, 0, sizeof(float) * OLAR_BIG_BUFFER_COUNT * OLAR_BIG_BUFFER_SIZE );
	memset( m_ola_buffer, 0, sizeof(float) * OLAR_BIG_BUFFER_SIZE );
	memset( m_data_buffer, 0, sizeof(float) * OLAR_BIG_BUFFER_SIZE );
	memset( m_swap_buffer, 0, sizeof(float) * OLAR_BIG_BUFFER_SIZE ); 

    // no samples
    for( int i = 0; i < OLAR_BIG_BUFFER_COUNT; i++ )
        m_buffer_samples[i] = 0;

	write_to_buffer = true;
	write_to_file = false;
}


OlaRandom::~OlaRandom()
{
	while( m_callbacking );

	if( sfread )
        sf_close( sfread );

    if( sfwrite )
        sf_close( sfwrite );

	if( origsize > 0 )
		SAFE_DELETE_ARRAY(origbuffer);
    origsize = 0;

    std::cout << "Goodbye!" << std::endl;    
}


void OlaRandom::initialize( char filename[], int datasize, float randomness, int mindist, bool scale_amp )
{
	// temp
	BB_setlog( BB_LOG_INFO );

	ReadSoundFile( filename );
	this->randomness = randomness;
	this->mindist = mindist;
	this->scale_amp = scale_amp;
	this->segsize = datasize;

	fprintf( stderr, "filename: %s\nsegment size: %d\nrandomness: %.2f\nmindist: %d\namp scale: %s\n", 
		filename, this->segsize, this->randomness, this->mindist, (this->scale_amp ? "true" : "false") );
}


//-----------------------------------------------------------------------------
// name: int m_audio_cb( char * buffer, int buffer_size, void * user_data )
// desc: audio callback
//-----------------------------------------------------------------------------
int OlaRandom::m_audio_cb( char * buf, int buffer_size, void * user_data )
{
	// don't delete me now
	m_callbacking = 1;

	float * buffer = (float *) buf;

    // if not ready, leave (used to play wavelet tree, but that's harder now)
    if( !m_ready ) // waiting for all buffers to be filled before playing any sound
    {
        memset( buffer, 0, buffer_size * sizeof(float) );
        m_ready = 1;
    }

    int remaining = buffer_size;
    int offset = 0;

    while( remaining > 0 )
    {
        while( !m_data_count )
        {
			fprintf( stderr, "oops" );
            usleep( 2000 );
        }
    
		if( !m_audio_begin )
        {
            m_audio_begin = m_big_buffer[m_read_index]; 
            m_audio_end = m_audio_begin + m_buffer_samples[m_read_index]; //nsamples;
			m_buffer_samples[m_read_index] = 0;
            m_read_index++;
            m_read_index %= m_max_data_count;
        }
    
        // we have data, but do we have >= buffer_size? otherwise it's not enough :(
        int tocopy = min( remaining, m_audio_end - m_audio_begin );

        // copy the data out
        memcpy( buffer + offset, m_audio_begin, tocopy * sizeof(float) );
        // increment the read pointer
        m_audio_begin += tocopy;
	
        // check to see if at end
        if( m_audio_begin >= m_audio_end )
        {
            // reset
            m_audio_begin = NULL;
            m_data_count--;
	    }

        // update remaining
        remaining -= tocopy; 
        // update offset ( i bet they could be converged to one variable, but too bad)
        offset += tocopy;
    }

/*	fprintf( stderr, "- " );
	for( int i = 0; i < m_max_data_count; i++ )
		fprintf( stderr, "%d	", m_buffer_samples[i] );
	fprintf( stderr, "(%d)\n", buffer_size );
*/
	// done now
	m_callbacking = 0;

    return 0;
}


// these were deleted in taps version
#include "Stk.h"
#include "RtAudio.h"
RtAudio * g_olar_audio;

//-----------------------------------------------------------------------------
// name: bool audio_initialize( int (*audio_cb) (char *, int, void *), int srate );
// desc: set up audio capture and playback and initializes any application data
//-----------------------------------------------------------------------------
bool OlaRandom::audio_initialize( int (*audio_cb) (char *, int, void *), int srate )
{
    if( g_olar_audio )
        return TRUE;

	std::cerr << "OlaRandom::audio_initialize : I hope you've passed as audio_cb "
		<< "some function that calls m_audio_cb." << std::endl;

    Stk::setSampleRate( m_srate = srate );

    try
    {
        // open the audio device for capture and playback
        g_olar_audio = new RtAudio( 0, 1, 0, 0, RTAUDIO_FLOAT32, m_srate, &m_rt_buffer_size, 4 );
    }
    catch( StkError & e )
    {
        // exception
        fprintf( stderr, "%s\n", e.getMessage() );
        fprintf( stderr, "error: cannot open audio device for capture/playback...\n" );
        return false;
    }

    // set the audio callback
    g_olar_audio->setStreamCallback( audio_cb, NULL );
    
    // start the audio
    g_olar_audio->startStream( );
    
    return true;
}


int OlaRandom::ReadSoundFile( char filename[] )
{
	// open file
	if( sfread )
	{
		sf_close( sfread );
	}
	sfread = sf_open( filename, SFM_READ, &readinfo );
	if( !sfread )
	{
		BB_log( BB_LOG_SEVERE, "[TreesynthIO]: Could not open input file '%s', %s", 
			filename, sf_error_number( sf_error( sfread ) ) );
		return 0;
	}
	strcpy( ifilename, filename );

	// determine number of buffers needed
	origsize = readinfo.frames;
	std::cerr << "frames: " << origsize << std::endl;
	SAFE_DELETE_ARRAY(origbuffer);
	origbuffer = new float[origsize];

	// read
	sf_seek( sfread, 0, SEEK_SET );
    int itemsread = sf_read_float( sfread, origbuffer, origsize);
	sf_close( sfread );
	sfread = NULL;

	return itemsread;
}


int OlaRandom::next_segment() 
{
	// start segment (and typically end as well) exactly copied, 1 second's worth
	int start, size;
	int dist = 0;
	if( m_last_buffer_size == 0 ) {
		size = m_next_buffer_size = m_srate;
		start = 0;
	}
	else {
		if( !m_top ) {
			float min = segsize / (1 + randomness);
			float max = segsize * (1 + randomness); 
			float randval = rand() / (RAND_MAX + 1.0) * (max - min); 
			m_last_buffer_size = m_next_buffer_size; // no need, it already has this value
			m_next_buffer_size = (int)(min + randval); 
			if( m_next_buffer_size % 2 != 0 ) 
				m_next_buffer_size++;
			if( m_next_buffer_size % 4 != 0 )
				m_next_buffer_size += 2;
			if( m_next_buffer_size > OLAR_BIG_BUFFER_SIZE )
				m_next_buffer_size = OLAR_BIG_BUFFER_SIZE;
			size = (m_last_buffer_size + m_next_buffer_size) / 2;
			if( size % 2 != 0 )
				fprintf( stderr, "size: %d\n", size );
			if( size > OLAR_BIG_BUFFER_SIZE ) {
				fprintf( stderr, "size too big: %d (%d, %d)\n", size, m_last_buffer_size, m_next_buffer_size );
				size = OLAR_BIG_BUFFER_SIZE;
			}
		} 
		else {
			size = m_next_buffer_size;
		}
		do {
			start = rand() % (origsize - size); 
			dist = m_last_start_index - start;
			if( dist < 0 ) dist = -dist;
		} while( dist < mindist );
	}
//	fprintf( stderr, "start: %d, size: %d, dist: %d", start, size, dist );
	memcpy(m_data_buffer, origbuffer + start, sizeof(float) * size);
	m_last_start_index = start;
	if( scale_amp ) {
		float scale = rand() / (RAND_MAX + 1.0f) * 0.4f + 0.7f; 
		for( int i = 0; i < size; i++ )
			m_data_buffer[i] *= scale;
//		fprintf( stderr, ", ampscale = %.2f", scale );
	}
//	fprintf( stderr, "\n" );
	return WriteSoundFile( ofilename, m_data_buffer, size );
}	


int OlaRandom::WriteSoundFile( char filename[], float * data, int datasize )
{
	if( m_winsize != datasize )
	{
		hanna( m_window, datasize );
		m_winsize = datasize;
	}
	apply_window( data, m_window, datasize );
	/*int hopsize = segsize / 2; //m_last_buffer_size / 2; //m_last_buffer_size - datasize / 2;
	if( hopsize < 0 )
		hopsize = 0;

	BirdBrain::ola( m_ola_buffer, data, hopsize, datasize );
	m_last_buffer_size = datasize;
	
	// reset data and datasize for writing
	memcpy( data, m_ola_buffer, sizeof(float) * hopsize );
	datasize = hopsize;	*/

	if( !m_top )
		m_hopsize = m_last_buffer_size / 2;
	//fprintf( stderr, "hop: %d\n", m_hopsize );
	m_top = !m_top;
	memcpy( m_swap_buffer, m_ola_buffer, sizeof(float) * m_hopsize );	
	BirdBrain::ola( m_ola_buffer, data, m_hopsize, OLAR_BIG_BUFFER_SIZE, datasize );
	m_last_buffer_size = datasize;
	memcpy( data, m_swap_buffer, sizeof(float) * m_hopsize ); 
	datasize = m_hopsize; 

    // data will always fit in one buffer because buffer size = max tree size (CUTOFF)
    if( this->write_to_buffer )
    {
        while( m_data_count >= m_max_data_count ) {
			BB_log( BB_LOG_FINE, "Background: waiting for empty buffer" );
			usleep( 2000 );
		}
		// used to be if ... return 0;

        // set #samples for the write buffer
        if( m_buffer_samples[m_write_index] != 0 )
		{
			BB_log( BB_LOG_WARNING, "Background: overwriting!" );
		}
		m_buffer_samples[m_write_index] = datasize;

		// set the buffer to write
		float * next_buffer = m_big_buffer[m_write_index++];
        m_write_index %= m_max_data_count;

        // copy the data
        memcpy( next_buffer, data, datasize * sizeof(float) );

        // increment data count
        m_data_count++;

/*		fprintf( stderr, "+ " );
		for( int i = 0; i < m_max_data_count; i++ )
			fprintf( stderr, "%d	", m_buffer_samples[i] );
		fprintf( stderr, "(%d)\n", datasize );
*/	}
    
    // if it's also writing to buffer, it doesn't get here until after the write succeeds
    // so the same data should not be written more than once to the file
	int itemswritten = 0;
    if( this->write_to_file )
    {
        if( !sfwrite ) {
            writeinfo = readinfo;
			sfwrite = sf_open( filename, SFM_RDWR, &writeinfo );

			if( !sfwrite )
			{
				std::cerr << "OlaRandom::WriteSoundFile : cannot open file '" << filename << "', quitting" << std::endl;
				//char x[256];
				//std::cin.getline( x, 256 );
				//exit(1);
                return 2;
			}
		}
		sf_seek( sfwrite, writeinfo.frames, SEEK_SET );
		itemswritten = sf_write_float( sfwrite, data, datasize );
		if( itemswritten <= 0 ) {
			std::cerr << "OlaRandom::WriteSoundFile : cannot write to file '" << filename << "'" << std::endl;
			std::cerr << "needed to write " << datasize << " samples" << std::endl;

            //char x[256];
			//std::cin.getline( x, 256 );
			//exit(3);
            return 2;
        }
		sf_close( sfwrite );
		sfwrite = NULL;
    }
    
    // wrote something
	if( this->write_to_file )
		return itemswritten;
	else
		return datasize;
}
