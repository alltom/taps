#include "ui_audio.h"
#include "taps_olar.h"
#include "taps_birdbrain.h"
#include "audicle_def.h"
#include "taps_sceptre.h"

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
	
	m_winsize = 0;

    sfread = NULL;
    sfwrite = NULL;
	samples_read = 0;
	
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
        AudioCentral::instance()->m_cachemanager->closesf( sfread );

    if( sfwrite )
        AudioCentral::instance()->m_cachemanager->closesf( sfwrite );

	if( origsize > 0 )
		SAFE_DELETE_ARRAY(origbuffer);
    origsize = 0;
}


void OlaRandom::initialize( const char filename[], float datasize_secs, float randomness, 
							float mindist_secs, bool scaleamp )
{
	// temp
	BB_setlog( BB_LOG_INFO );

	samples_read = ReadSoundFile( filename );
	set_randomness(randomness);
	set_mindist_secs(mindist_secs);
	set_scaleamp(scaleamp);
	set_segsize_secs(datasize_secs);
	
	BB_log(BB_LOG_INFO, "OlaRandom initialized:");
	BB_pushlog();
	BB_log(BB_LOG_INFO, "filename: %s", filename);
	BB_log(BB_LOG_INFO, "segment size: %d (%f secs)", this->segsize, this->segsize * 1.0 / BirdBrain::srate());
	BB_log(BB_LOG_INFO, "min distance: %d (%f secs)", this->mindist, this->mindist * 1.0 / BirdBrain::srate());
	BB_log(BB_LOG_INFO, "amp scale: %s", this->scale_amp ? "true" : "false");
	BB_poplog();
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
			BB_log(BB_LOG_FINE, "OlaRandom:m_audio_cb: waiting for data");
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


/*//-----------------------------------------------------------------------------
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
}*/


int OlaRandom::ReadSoundFile( const char filename[] )
{
	// open file
	if( sfread )
	{
		AudioCentral::instance()->m_cachemanager->closesf( sfread );
	}
	sfread = AudioCentral::instance()->m_cachemanager->opensf( filename, SFM_READ, &readinfo );
	if( !sfread )
	{
		BB_log( BB_LOG_SEVERE, "[TreesynthIO]: Could not open input file '%s', %s", 
			filename, sf_error_number( sf_error( sfread ) ) );
		return 0;
	}
	strcpy( ifilename, filename );

	// determine number of buffers needed
	origsize = readinfo.frames;
	BB_log(BB_LOG_FINE, "[OlaRandom] frames: %d", origsize);
	SAFE_DELETE_ARRAY(origbuffer);
	origbuffer = new float[origsize];

	// read
	sf_seek( sfread, 0, SEEK_SET );
    int itemsread = sf_read_float( sfread, origbuffer, origsize);
	AudioCentral::instance()->m_cachemanager->closesf( sfread );
	sfread = NULL;

	return itemsread;
}


int OlaRandom::next_segment() 
{
	// start segment (and typically end as well) exactly copied, 1 second's worth
	int start, size;
	int dist = 0;
	if( m_last_buffer_size == 0 ) {
		size = m_next_buffer_size = BirdBrain::srate();
		if(size > segsize) 
			size = m_next_buffer_size = segsize;
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
				BB_log( BB_LOG_FINE, "OlaRandom:next_segment: size: %d", size );
			if( size > OLAR_BIG_BUFFER_SIZE ) {
				BB_log( BB_LOG_FINE, "OlaRandom:next_segment: size too big: %d (%d, %d)\n", size, m_last_buffer_size, m_next_buffer_size );
				size = OLAR_BIG_BUFFER_SIZE;
			}
		} 
		else {
			size = m_next_buffer_size;
		}
		/*do {
			start = rand() % (origsize - size); 
			dist = m_last_start_index - start;
			if( dist < 0 ) dist = -dist;
		} while( dist < mindist ); */
		// try to guarantee dist < mindist on first go
		start = rand() % (origsize - size - (2 * mindist - 1)); 
		// * if start is between 0 and m_last_start_index - mindist, stay the same
		// * if start > m_last_start_index - mindist, shift up so it's > m_last_start_index + mindist
		if( start > m_last_start_index - mindist )
			start += (2 * mindist - 1);
		// * start should still be <= origsize - size
		if( start < 0 || start > origsize - size ) {
			BB_log(BB_LOG_WARNING, "OlaRandom:next_segment: start out of bounds %d", start);
			return 0;
		}
		// * and dist should be >= mindist
		dist = m_last_start_index - start;
		if( dist < 0 ) dist = -dist;
		if( dist < mindist ) {
			BB_log(BB_LOG_WARNING, "OlaRandom:next_segment: mindist not upheld: %d", dist);
		}
	}
	memcpy(m_data_buffer, origbuffer + start, sizeof(float) * size);
	m_last_start_index = start;
	if( scale_amp ) {
		float scale = rand() / (RAND_MAX + 1.0f) * 0.4f + 0.7f; 
		for( int i = 0; i < size; i++ )
			m_data_buffer[i] *= scale;
	}
	return prepare_segment( m_data_buffer, size );
	
//	return WriteSoundFile( ofilename, m_data_buffer, size );
}	

// prepare next segment (data) by windowing and overlap-adding, and write next 
// hopsize overlap-added samples back to data
int OlaRandom::prepare_segment( float * data, int datasize ) {
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
	BB_log(BB_LOG_FINE, "OlaRandom:prepare_segment: datasize: %d hopsize: %d", datasize, m_hopsize);
	datasize = m_hopsize; 
	return datasize;
}

float * OlaRandom::outputSignal() {
	return m_data_buffer;
}

int OlaRandom::WriteSoundFile( const char filename[], float * data, int datasize )
{
    // data will always fit in one buffer because buffer size = max tree size (CUTOFF)
    if( this->write_to_buffer )
    {
        /*while( m_data_count >= m_max_data_count ) {
			BB_log( BB_LOG_FINE, "Background: waiting for empty buffer" );
			usleep( 2000 );
		}*/
		if( m_data_count >= m_max_data_count ) {
			BB_log( BB_LOG_FINE, "OlaRandom:WriteSoundFile: No empty buffer" );
			return 0;
		}

        // set #samples for the write buffer
        if( m_buffer_samples[m_write_index] != 0 )
		{
			BB_log( BB_LOG_WARNING, "OlaRandom:WriteSoundFile: overwriting!" );
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
			sfwrite = AudioCentral::instance()->m_cachemanager->opensf( filename, SFM_RDWR, &writeinfo );

			if( !sfwrite )
			{
				BB_log( BB_LOG_SEVERE, "OlaRandom::WriteSoundFile : cannot open file '%s', quitting", 
					filename );
				return 2;
			}
		}
		sf_seek( sfwrite, writeinfo.frames, SEEK_SET );
		itemswritten = sf_write_float( sfwrite, data, datasize );
		if( itemswritten <= 0 ) {
			BB_log( BB_LOG_SEVERE, "OlaRandom::WriteSoundFile : cannot write to file '%s', needed to write %d samples",
				filename, datasize );
			return 2;
        }
		AudioCentral::instance()->m_cachemanager->closesf( sfwrite );
		sfwrite = NULL;
    }
    
    // wrote something
	if( this->write_to_file )
		return itemswritten;
	else
		return datasize;
}

// params
float OlaRandom::get_randomness() {
	return randomness;
}

// set randomness: independent
float OlaRandom::set_randomness(float r) {
	if(r >= 0 && r <= 1) {
		randomness = r;
		BB_log(BB_LOG_FINE, "OlaRandom:set_randomness: set to %f", randomness);
	}
	else
		BB_log(BB_LOG_INFO, "OlaRandom:set_randomness: value %f is not in [0,1]; not set", r);
	return randomness;
}

bool OlaRandom::get_scaleamp() {
	return scale_amp;
}

// set scale_amp: independent
bool OlaRandom::set_scaleamp(bool s) {
	scale_amp = s;
	BB_log(BB_LOG_INFO, "OlaRandom:set_scaleamp: set to %s", scale_amp ? "true" : "false");
	return scale_amp;
}

float OlaRandom::get_mindist_secs() {
	return mindist * 1.0 / BirdBrain::srate();
}

// set mindist: depends on randomness and segsize
float OlaRandom::set_mindist_secs(float m) {
	if(m < 0)
		BB_log(BB_LOG_INFO, "OlaRandom:set_mindist_secs: %f < 0; not set", m);
	else {
		mindist = (int)(m * BirdBrain::srate());
		int maxsamples = samples_read;
		if(maxsamples > origsize) // shouldn't be
			maxsamples = origsize;
		int maxsize = segsize * (1 + randomness) + 1;
		// want maxsamples - maxsize - (2 * mindist - 1) > 0
		// i.e. mindist < (maxsamples - maxsize + 1) / 2 
		int mindist_cutoff = (maxsamples - maxsize + 1) / 2;
		if(mindist > mindist_cutoff)
			mindist = mindist_cutoff;
		BB_log( mindist == (int)(m * BirdBrain::srate()) ? BB_LOG_FINE : BB_LOG_INFO, 
			    "OlaRandom:set_mindist_secs: set to %f", mindist * 1.0 / BirdBrain::srate() );
	}
	return mindist * 1.0 / BirdBrain::srate();
}

float OlaRandom::get_segsize_secs() {
	return segsize * 1.0 / BirdBrain::srate();
}

// set segsize: depends on randomness
float OlaRandom::set_segsize_secs(float s) {
	if(s < 0)
		BB_log(BB_LOG_INFO, "OlaRandom:set_segsize_secs: %f < 0; not set", s);
	else {
		segsize = (int)(s * BirdBrain::srate());
		int maxsamples = samples_read;
		if(maxsamples > OLAR_BIG_BUFFER_SIZE)
			maxsamples = OLAR_BIG_BUFFER_SIZE;
		if((segsize * (1 + randomness) > maxsamples))
			segsize = (int)(maxsamples / (1 + randomness));
		BB_log( segsize == (int)(s * BirdBrain::srate()) ? BB_LOG_FINE : BB_LOG_INFO,  
				"OlaRandom:set_segsize_secs: set to %f", segsize * 1.0 / BirdBrain::srate() );
	}
	return segsize * 1.0 / BirdBrain::srate();
}
