/*----------------------------------------------------------------------------
    TAPESTREA: Techniques And Paradigms for Expressive Synthesis, 
               Transformation, and Rendering of Environmental Audio
      Engine and User Interface

    Copyright (c) 2006 Ananya Misra, Perry R. Cook, and Ge Wang.
      http://taps.cs.princeton.edu/
      http://soundlab.cs.princeton.edu/

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
    U.S.A.
-----------------------------------------------------------------------------*/

//-----------------------------------------------------------------------------
// file: taps_driver.cpp
// desc: taps analysis/synthesis driver
//
// author: Ananya Misra (amisra@cs.princeton.edu)
//         Ge Wang (gewang@cs.princeton.edu)
//         Perry R. Cook (prc@cs.princeton.edu)
// date: Autumn 2004
//-----------------------------------------------------------------------------
#include "taps_driver.h"
#include <time.h>
#include <iostream>
#include <limits.h>
using namespace std;

Driver::Driver()
{
    wnd_size = 0;
    fft_size = 0;
    time_now = 0;
    last_hop = 0;
    analysis = NULL;
    synthesis = NULL;
    m_input = NULL;
    sf_out = NULL;
    sf_res = NULL;
    signal_scale = 1.0f;
    sf_buffer = NULL;
    sf_buffer_size = 0;
    event_count = 0;
    m_start = -1;
    m_stop = -1;
    m_freq_min = -1.0f;
    m_freq_max = -1.0f;
    m_times = NULL;
    m_frames = 0;
    pp_framesize = 0; 
    pp_framestart = 0; 
    fft_framesize = 0; 
    fft_framestart = 0; 
    frame_number = 0;
    ppfin = NULL;
    fftfin = NULL; 
}

Driver::~Driver()
{
    this->brake();

    // delete
    long i;

    for( i = 0; i < tracks.size(); i++ ) { SAFE_DELETE( tracks[i] ); }
    for( i = 0; i < realtracks.size(); i++ ) { SAFE_DELETE( realtracks[i] ); }
    for( i = 0; i < fft_frames.size(); i++ ) { SAFE_DELETE( fft_frames[i] ); } 
    for( i = 0; i < fakeresidue.size(); i++ ) { SAFE_DELETE( fakeresidue[i] ); }
    for( i = 0; i < peak_frames.size(); i++ ) { SAFE_DELETE( peak_frames[i] ); }

    tracks.clear();
    realtracks.clear();
    fft_frames.clear();
    fakeresidue.clear();
    peak_frames.clear(); 
    events.clear(); 

    SAFE_DELETE( analysis );
    SAFE_DELETE( synthesis );
    SAFE_DELETE_ARRAY( m_times ); 
}

void Driver::set( t_TAPUINT wnd, t_TAPUINT fft, t_TAPUINT type )
{
    wnd_size = wnd;
    fft_size = fft;

    // allocate 
    f.alloc_waveform( fft_size );
    f_syn.alloc_waveform( wnd_size );
    window.alloc_waveform( wnd_size );
    window_inv.alloc_waveform( wnd_size );
    window_han.alloc_waveform( wnd_size );
    window_big.alloc_waveform( fft_size );
    ola_syn.alloc_waveform( fft_size );
    ola_res.alloc_waveform( fft_size );

    // make the window
    if( type == 1 )
    { hanning( window.waveform, wnd_size ); hanning( window_big.waveform, fft_size ); }
    else if( type == 2 )
    { rectangular( window.waveform, wnd_size ); rectangular( window_big.waveform, fft_size ); }
    else if( type == 3 )
    { blackman( window.waveform, wnd_size ); blackman( window_big.waveform, fft_size ); }
    else
    { hamming( window.waveform, wnd_size ); hamming( window_big.waveform, fft_size ); }

    // calcuate inverse window
    for( t_TAPUINT i = 0; i < window_inv.wlen; i++ )
        window_inv.waveform[i] = window.waveform[i] > 0.0 ? 1.0f / window.waveform[i] : 1.0f;

    hanning( window_han.waveform, wnd_size );

    analysis = new AnaPeaksFFT;
    analysis->wnd_size = wnd;
    analysis->init();

    synthesis = new Syn;
    synthesis->init();

    float B[] = { 1.f, -1.f };
    float A[] = { 1.f, -.98f };

    dcbloke.init( B, 2, A, 2 );
    //dcunbloke.init( A, 2, B, 2 );
}

void Driver::set( Analysis * ana )
{
    this->analysis = ana;
}

Analysis * Driver::ana() const
{
    return this->analysis;
}

bool Driver::open( const string & filename )
{
    assert( this->ana() );
    
    // soundfile or preprocessed? 
    if( filename.find( ".pp", filename.length() - 3 ) != string::npos )
    {
        BB_log( BB_LOG_SYSTEM, "Reading preprocessed file" );
        analysis->processing_mode = PREPROCESS_IN; 
        if( !read_preprocessed( filename ) ) 
            return false; 
    }
    else
    {
        BB_log( BB_LOG_SYSTEM, "Reading sound file" );
        analysis->processing_mode = NORMAL;
        in_file_name = filename;
    }

    // close
	if( m_input && m_input->type() == AudioSrcBuffer::BUFF_FILE ) {
		((AudioSrcFile *)m_input)->close();
	}
	SAFE_DELETE( m_input );
	m_input = new AudioSrcFile;
	// open
	if( !((AudioSrcFile *)m_input)->open( filename.c_str(), 0, 0, TRUE ) ) {
		BB_log( BB_LOG_WARNING, "Driver: could not open file '%s'", filename.c_str() );
		SAFE_DELETE( m_input );
		BB_log( BB_LOG_WARNING, "Driver: trying mic input" );
		return this->open_mic(); 
	}
	// other files
    sf_info_res = sf_info_out = ((AudioSrcFile *)m_input)->info();
    res_file_name = string("res_").append(BirdBrain::getbase(in_file_name.c_str())).c_str();
    out_file_name = string("out_").append(BirdBrain::getbase(in_file_name.c_str())).c_str();
    path = BirdBrain::getpath(filename.c_str());
    out_file_name = path + out_file_name; 
    // Open sf_res later 
    // assert
    assert( ((AudioSrcFile *)m_input)->info().channels < 3 ); 
    assert( BirdBrain::srate() == m_input->srate() );
    // convert to frequency
    f.bins2freqs( m_input->srate() );
    // reset start and endpoints to reasonable times
    if( m_start < 0 || m_start >= m_input->frames() || m_start > m_stop ) m_start = 0;
    if( m_stop < 0 || m_stop >= m_input->frames() || m_stop < m_start ) m_stop = m_input->frames();

    // return
    return TRUE;
}

// open mic for reading
bool Driver::open_mic()
{
    assert( this->ana() );
    
    // close
	if( m_input && m_input->type() == AudioSrcBuffer::BUFF_FILE ) {
		((AudioSrcFile *)m_input)->close();
	}
	SAFE_DELETE( m_input );
	m_input = new AudioSrcMic;
	// open
	if( !((AudioSrcMic *)m_input)->open( 0, 0, TRUE ) ) {
		BB_log( BB_LOG_WARNING, "Driver: could not open mic source" );
		SAFE_DELETE( m_input );
		return FALSE; 
	}
	// output files
	memset( &sf_info_out, 0, sizeof(sf_info_out) );
	memset( &sf_info_res, 0, sizeof(sf_info_res) );
	sf_info_out.samplerate = m_input->srate();
	sf_info_out.channels = AudioCentral::instance()->m_mic_buffer->get_channels();
	sf_info_out.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16; 
	sf_info_res = sf_info_out;
	in_file_name = "microphone.wav"; 
	res_file_name = string("res_").append(BirdBrain::getbase(in_file_name.c_str())).c_str();
    out_file_name = string("out_").append(BirdBrain::getbase(in_file_name.c_str())).c_str();
    path = BirdBrain::get_start_dir() + "/";
    out_file_name = path + out_file_name;
	BB_log( BB_LOG_INFO, "Driver: Setting output file path to %s%s", path.c_str(), res_file_name.c_str() );  
    // Open sf_res later 
    // assert
    assert( BirdBrain::srate() == m_input->srate() );
    // convert to frequency
    f.bins2freqs( m_input->srate() );
    // reset start and endpoints to reasonable times
    if( m_start < 0 || m_start >= m_input->frames() || m_start > m_stop ) m_start = 0;
    if( m_stop < 0 || m_stop >= m_input->frames() || m_stop < m_start ) m_stop = m_input->frames();
    // return
    return TRUE;
}


bool Driver::dine()
{
    assert( analysis != NULL );
    assert( m_input != NULL );
    t_TAPUINT i;
    Track * ppin = NULL;

    if( analysis->processing_mode != PREPROCESS_IN )
    {
        if( !read_window( this->f ) )
        {
            BB_log( BB_LOG_WARNING, "Driver: dine() cannot get next window..." );
            return FALSE;
        }

        // block DC
        dcbloke.apply_filter( f );

        // window the data
        apply_window( f.waveform, window.waveform, window.wlen );

        // zero pad the frame
        for( i = wnd_size; i < fft_size; i++ )
            f.waveform[i] = 0.0f;

        // fft - the result is in f.waveform/cmp
        rfft( f.waveform, f.len, FFT_FORWARD );
        // scale from zero padding
        BirdBrain::scale_fft( f.waveform, f.wlen, fft_size, wnd_size );
        // convert to polar
        f.cmp2pol();

        // set the time
        f.time = (t_TAPTIME)time_now;

        fft_frames.push_back( new Frame( f ) );
    }
    else
    {
        if( fft_frames.empty() )
            return false; 
        if( peak_frames.empty() )
            return false;
        if( frame_number >= fft_frames.size() )
        {
            BB_log( BB_LOG_INFO, "(driver): trying to read nonexistent frame info" );
            return false;
        }

        f = *fft_frames[frame_number];      
        ppin = peak_frames[frame_number]; 
        frame_number += 1;
    }
    
    // do the analysis
    analysis->analyze( f, tracks, residue, ppin );
    // add the fake residue
    fakeresidue.push_back( new Frame(residue) );
    
    return true;
}

bool Driver::drink()
{
    analysis->get_tracks( tracks, realtracks );
    if( m_group )
        analysis->get_events( realtracks, events );
    return true;
}

bool Driver::honk( vector<Track *> & rtracks, Frame & out )
{
    synthesis->setTracks( rtracks );
    synthesis->synthesize( out );
    
    sf_info_out.channels = 1; 
//    sf_out = sf_open( out_file_name.c_str(), SFM_WRITE, &sf_info_out ); // 02-08a, 02-04b
//    sf_write_float( sf_out, out.waveform, out.wsize );
//    sf_close( sf_out ); 

    return true;
}

bool Driver::drive( int hop )
{
    assert( m_input != NULL );

    if( hop >= 0 )
    {
        // at the end?
        sf_count_t diff = m_input->frames() - m_input->current();
        if( diff <= 0 || diff > m_input->frames() ) return false;
        // how much to move
        sf_count_t thismuch = hop <= diff ? hop : diff;
        // seek
        m_input->seek( thismuch, SEEK_CUR );
		time_now = m_input->current();
        // remember
        last_hop = thismuch;
        // if end
        if( thismuch < hop ) return false;
    }
    else
    {
        // at the beginning?
        sf_count_t diff = m_input->current();
        if( diff <= 0 || diff > m_input->frames() ) return false;
        // how much to move
        sf_count_t thismuch = -hop <= diff ? hop : -diff;
        // seek
        m_input->seek( thismuch, SEEK_CUR );
        time_now = m_input->current();
		// remember
        last_hop = thismuch;
        // if beginning
        if( thismuch > hop ) return false;
    }

    return true;
}

void Driver::brake()
{
    if( m_input && m_input->type() == AudioSrcBuffer::BUFF_FILE )
    {
        // close
//        sf_close( sf );
		((AudioSrcFile *)m_input)->close();
		
        // out in honk
        //sf_close( sf_out );
        //sf_out = NULL;

        // res in write_res
        //sf_close( sf_res );
        //sf_res = NULL;
    }
	if( m_input ) {
		SAFE_DELETE( m_input );
	}
    if( ppfin )
    {
        fclose( ppfin ); 
        ppfin = NULL;
    }
    if( fftfin )
    {
        fclose( fftfin ); 
        fftfin = NULL; 
    }
}


void Driver::preprocess()
{
    // clear stuff
    int it; 
    for( it = 0; it < tracks.size(); it++ ) { SAFE_DELETE( tracks[it] ); }
    for( it = 0; it < realtracks.size(); it++ ) { SAFE_DELETE( realtracks[it] ); }
    for( it = 0; it < fft_frames.size(); it++ ) { SAFE_DELETE( fft_frames[it] ); }
    for( it = 0; it < fakeresidue.size(); it++ ) { SAFE_DELETE( fakeresidue[it] ); }
    for( it = 0; it < peak_frames.size(); it++ ) { SAFE_DELETE( peak_frames[it] ); }
    tracks.clear();
    realtracks.clear();
    fft_frames.clear();
    fakeresidue.clear();
    peak_frames.clear(); 
    events.clear();
    analysis->clear(); 

    BB_log( BB_LOG_INFO, "(driver): preprocess" ); 
    BB_pushlog(); 

    // open file for writing
    FILE * ppfile; 
    string ppname = BirdBrain::getpath( in_file_name.c_str() ) + BirdBrain::getname( in_file_name.c_str() ) + ".pp"; 
    string fftname = BirdBrain::getpath( in_file_name.c_str() ) + BirdBrain::getname( in_file_name.c_str() ) + ".fft"; 

    ppfile = fopen( ppname.c_str(), "wb" ); 
    if( !ppfile )
    {
        BB_log( BB_LOG_SYSTEM, "Could not open file %s for writing", ppname.c_str() );
        return; 
    }
    else
        BB_log( BB_LOG_INFO, "Opened file %s for writing", ppname.c_str() );

    // write basics
    int nbytes_pp = 0; 
    int length = in_file_name.length(); 
    nbytes_pp += sizeof(int) * fwrite( &length, sizeof(int), 1, ppfile );
    nbytes_pp += sizeof(char) * fwrite( in_file_name.c_str(), sizeof(char), length, ppfile ); 
    length = fftname.length(); 
    nbytes_pp += sizeof(int) * fwrite( &length, sizeof(int), 1, ppfile ); 
    nbytes_pp += sizeof(char) * fwrite( fftname.c_str(), sizeof(char), length, ppfile ); 
    nbytes_pp += sizeof(t_TAPUINT) * fwrite( &wnd_size, sizeof(t_TAPUINT), 1, ppfile ); 
    nbytes_pp += sizeof(t_TAPUINT) * fwrite( &fft_size, sizeof(t_TAPUINT), 1, ppfile ); 
    t_TAPUINT hop = BirdBrain::hop_size(); 
    nbytes_pp += sizeof(t_TAPUINT) * fwrite( &hop, sizeof(t_TAPUINT), 1, ppfile ); 

    // compute peaks
    BB_log( BB_LOG_INFO, "Preprocessing..." );
    int max_peaks_found = 0; 
    Track * trframe; 
    analysis->processing_mode = PREPROCESS_OUT; 
    do {
        dine(); 
        trframe = tracks.back(); 
        if( trframe->history.size() > max_peaks_found )
            max_peaks_found = trframe->history.size(); 
    } while( drive( BirdBrain::hop_size() ) );
    
    // write info for locating frames
    pp_framesize = sizeof(int) + max_peaks_found * sizeof(freqpolar); // npeaks + each peak
    nbytes_pp += sizeof(int) * fwrite( &pp_framesize, sizeof(int), 1, ppfile ); 
    pp_framestart = nbytes_pp + sizeof(int); // what was written so far plus itself
    nbytes_pp += sizeof(int) * fwrite( &pp_framestart, sizeof(int), 1, ppfile ); 

    // write out each frame
    freqpolar info; 
    int npeaks;
    for( int tr = 0; tr < tracks.size(); tr++ )
    {
        fseek( ppfile, pp_framestart + tr * pp_framesize, SEEK_SET ); 
        trframe = tracks[tr]; 
        npeaks = trframe->history.size();
        nbytes_pp += sizeof(int) * fwrite( &npeaks, sizeof(int), 1, ppfile ); 
        for( int p = 0; p < npeaks; p++ )
        {
            info = trframe->history[p]; 
            nbytes_pp += sizeof(freqpolar) * fwrite( &info, sizeof(freqpolar), 1, ppfile ); 
        }
        BB_log( BB_LOG_FINER, "Wrote out peaks at time %f (samples)", info.time );
    }
    
    fclose( ppfile ); 
    BB_log( BB_LOG_INFO, "%i frames (%i bytes) written to peaks file %s", 
        tracks.size(), nbytes_pp, ppname.c_str() ); 

    // write out fft
    FILE * fftfile = fopen( fftname.c_str(), "wb" );
    if( !fftfile )
    {
        BB_log( BB_LOG_SYSTEM, "Could not open file %s for writing", fftname.c_str() );
        return; 
    }
    else
        BB_log( BB_LOG_INFO, "Opened file %s for writing", fftname.c_str() );
    BB_log( BB_LOG_INFO, "Writing fft frames..." );

    t_TAPTIME * times = new t_TAPTIME[fft_frames.size()]; 
    int q, maxlen = 0; 
    for( q = 0; q < fft_frames.size(); q++ )
    {
        times[q] = fft_frames[q]->time;
        if( fft_frames[q]->len > maxlen )
            maxlen = fft_frames[q]->len;
    }
    // write how many frames are there
    int nbytes = sizeof(int) * fwrite( &q, sizeof(int), 1, fftfile );  
    // write time stamp of each frame
    nbytes += sizeof(t_TAPTIME) * fwrite( times, sizeof(t_TAPTIME), q, fftfile );
    SAFE_DELETE_ARRAY( times );
    // write info on where to find stuff
    fft_framesize = maxlen * sizeof(polar) + sizeof(t_TAPTIME) + 3 * sizeof(t_TAPUINT); // max len of frame + time + len + wlen + wsize 
    fft_framestart = nbytes + 2*sizeof(int); // # frames + times array + framesize + start
    nbytes += sizeof(int) * fwrite( &fft_framesize, sizeof(int), 1, fftfile );
    nbytes += sizeof(int) * fwrite( &fft_framestart, sizeof(int), 1, fftfile ); 
    // write each frame
    for( q = 0; q < fft_frames.size(); q++ )
    {
        fseek( fftfile, fft_framestart + q * fft_framesize, SEEK_SET ); 
        Frame * fr = fft_frames[q]; 
        nbytes += sizeof(t_TAPTIME) * fwrite( &(fr->time), sizeof(t_TAPTIME), 1, fftfile ); 
        nbytes += sizeof(t_TAPUINT) * fwrite( &(fr->len), sizeof(t_TAPUINT), 1, fftfile );
        nbytes += sizeof(t_TAPUINT) * fwrite( &(fr->wlen), sizeof(t_TAPUINT), 1, fftfile );
        nbytes += sizeof(t_TAPUINT) * fwrite( &(fr->wsize), sizeof(t_TAPUINT), 1, fftfile ); 
        nbytes += sizeof(polar) * fwrite( fr->pol, sizeof(polar), fr->len, fftfile );
    }
    // close file
    fclose( fftfile ); 
    BB_log( BB_LOG_INFO, "%i frames (%i bytes) written to fft file %s", q, nbytes, fftname.c_str() ); 

    BB_poplog(); 
}


// read info from preprocessed file
// assumes fft_frames, peak_frames and all other vectors are empty at this point
bool Driver::read_preprocessed( string ppfilename )
{
    BB_log( BB_LOG_INFO, "(driver) reading preprocessed information" ); 
    BB_pushlog(); 
    if( ppfin )
        fclose( ppfin ); 
    ppfin = fopen( ppfilename.c_str(), "rb" );
    if( !ppfin ) 
    {
        BB_log( BB_LOG_SYSTEM, "Could not open file %s", ppfilename.c_str() );
        return false;
    }
    else
        BB_log( BB_LOG_INFO, "Opened preprocessed file %s", ppfilename.c_str() ); 
    
    // get file names
    int length; 
    fread( &length, sizeof(int), 1, ppfin ); 
    fprintf( stderr, "LENGTH IS %i\n", length ); 
    char * buf = new char[length + 1]; 
    fread( buf, sizeof(char), length, ppfin ); 
    buf[length] = '\0'; 
    //in_file_name = std::string( &buf[0], &buf[length-1] ); 
    in_file_name = std::string( (const char *)buf ); 
    in_file_name = BirdBrain::getpath( ppfilename.c_str() ) + BirdBrain::getbase( in_file_name.c_str() );

    std::string fftname; 
    int pp_wnd_size, pp_fft_size, pp_hop_size; 
    fread( &length, sizeof(int), 1, ppfin ); 
    SAFE_DELETE_ARRAY( buf ); 
    buf = new char[length + 1]; 
    fread( buf, sizeof(char), length, ppfin ); 
    buf[length] = '\0'; 
    //fftname = std::string( &buf[0], &buf[length-1] ); 
    fftname = std::string( (const char *)buf ); 
    fftname = BirdBrain::getpath( ppfilename.c_str() ) + BirdBrain::getbase( fftname.c_str() );
    
    fread( &pp_wnd_size, sizeof(t_TAPUINT), 1, ppfin ); 
    fread( &pp_fft_size, sizeof(t_TAPUINT), 1, ppfin ); 
    fread( &pp_hop_size, sizeof(t_TAPUINT), 1, ppfin );

    assert( pp_wnd_size == wnd_size ); 
    assert( pp_fft_size == fft_size ); 
    assert( pp_hop_size == BirdBrain::hop_size() ); 

    // read ffts
    if( fftfin )
        fclose( fftfin ); 
    fftfin = fopen( fftname.c_str(), "rb" ); 
    if( !fftfin )
    {
        BB_log( BB_LOG_SYSTEM, "Could not open fft file %s for reading", fftname.c_str() );
        return false; 
    }
    else
        BB_log( BB_LOG_INFO, "Opened fft file %s for reading", fftname.c_str() );
    BB_log( BB_LOG_INFO, "Reading fft frames..." );

    // read number of fft frames
    int nframes; 
    fread( &nframes, sizeof(int), 1, fftfin ); 
    if( nframes == 0 )
    {
        BB_log( BB_LOG_INFO, "No fft frames to read" ); 
        return false;
    }
    // read time stamps of frames
    //t_TAPTIME * times = new t_TAPTIME[nframes];
    if( !m_times || m_frames < nframes )
    {
        SAFE_DELETE_ARRAY( m_times );
        m_times = new t_TAPTIME[nframes]; 
    }
    fread( m_times, sizeof(t_TAPTIME), nframes, fftfin ); 
    m_frames = nframes; 

    // set m_start and m_stop to nearest frame bounds
    int stopind = 0, startind = 0; 
    for( int n = 0; n < nframes; n++ )
    {
        if( fabs(m_start - m_times[n]) < fabs(m_start - m_times[startind]) )
            startind = n;
        if( fabs(m_stop - m_times[n]) < fabs(m_stop - m_times[stopind]) )
            stopind = n; 
    }
    m_start = (int)m_times[startind]; 
    m_stop = (int)m_times[stopind]; 
    // read framesize etc
    fread( &fft_framesize, sizeof(int), 1, fftfin );
    fread( &fft_framestart, sizeof(int), 1, fftfin ); 

    // read each frame in [startind, stopind]
    // for( int q = startind; q <= stopind; q++ )
    // read ALL frames
    if( !fft_frames.empty() )
        BB_log( BB_LOG_WARNING, "fft_frames is not empty at start of Driver::read_processed()" ); 
    fft_frames.reserve( nframes ); 
    for( int q = startind; q <= stopind; q++ )
//  for( int q = 0; q < nframes; q++ )
    { 
        fseek( fftfin, fft_framestart + q * fft_framesize, SEEK_SET ); 
        Frame * fr = new Frame;
        fread( &(fr->time), sizeof(t_TAPTIME), 1, fftfin ); 
        fread( &(fr->len), sizeof(t_TAPUINT), 1, fftfin ); 
        fread( &(fr->wlen), sizeof(t_TAPUINT), 1, fftfin ); // kind of useless
        fread( &(fr->wsize), sizeof(t_TAPUINT), 1, fftfin ); // kind of useless
        fr->alloc_waveform( 2 * fr->len ); 
        fr->alloc_pol(); 
        fread( fr->pol, sizeof(polar), fr->len, fftfin ); 
        fr->bins2freqs( BirdBrain::srate() ); // BirdBrain::srate was set to sf_info.samplerate in Driver::open() 
        fr->pol2cmp(); 
        fft_frames.push_back( fr ); 
    }

    // read peaks from ppfin
    BB_log( BB_LOG_INFO, "reading peaks" );
    int ppinsize;
    Track * ppin; 
    freqpolar info;
    if( !peak_frames.empty() )
        BB_log( BB_LOG_WARNING, "peak_frames is not empty at start of Driver::read_processed()" ); 
    peak_frames.reserve( nframes ); 
    fread( &pp_framesize, sizeof(int), 1, ppfin ); 
    fread( &pp_framestart, sizeof(int), 1, ppfin ); 
    for( int frameind = startind; frameind <= stopind; frameind++ )
//  for( int frameind = 0; frameind < nframes; frameind++ )
    {
        fseek( ppfin, pp_framestart + frameind * pp_framesize, SEEK_SET ); 
        ppin = new Track;
        fread( &ppinsize, sizeof(int), 1, ppfin ); 
        ppin->history.reserve( ppinsize ); 
        for( int p = 0; p < ppinsize; p++ )
        {
            fread( &info, sizeof(freqpolar), 1, ppfin ); 
            ppin->history.push_back( info ); 
        }   
        peak_frames.push_back( ppin ); 
    }
    BB_log( BB_LOG_INFO, "done" ); 
    assert( peak_frames.size() == fft_frames.size() );

    frame_number = 0;
    // set starting point for this analysis
    //frame_number = startind; 

    BB_poplog(); 

    return true;
}


// get file name from pp file name
std::string Driver::get_filename( std::string filename )
{
    // soundfile or preprocessed? 
    if( filename.find( ".pp", filename.length() - 3 ) != string::npos )
    {
        FILE * ppf = fopen( filename.c_str(), "rb" ); 
        if( !ppf ) 
            BB_log( BB_LOG_SYSTEM, "Could not open file %s", filename.c_str() );
        else
        {
            int length; 
            fread( &length, sizeof(int), 1, ppf ); 
            char * buf = new char[length + 1]; 
            fread( buf, sizeof(char), length, ppf ); 
            buf[length] = '\0'; 
            fclose( ppf ); 
            //in_file_name = std::string( &buf[0], &buf[length-1] );
            in_file_name = std::string( (const char *)buf );  
        }
    }
    else
    {
        in_file_name = filename;
    }
    return in_file_name; 
}


t_TAPUINT Driver::now() const
{
    return time_now;
}

bool Driver::done( int endpt ) const
{
	if( !m_input ) return true;
    // at the end?
    sf_count_t diff = endpt - m_input->current();
    return diff <= 0 || diff > m_input->frames();
}

int Driver::sndfile_size( ) const
{
	if( !m_input )
		return 0;
	else
		return m_input->frames();
}

vector<Track *> & Driver::the_event( )
{
    return realtracks;
}

vector<Track *> & Driver::next_event()
{
    fprintf( stderr, "Next event: ");
    if( event_count >= events.size() || event_count < 0 )
        return realtracks;
    
    event_count += 1;
    if( event_count >= events.size() )
        event_count = 0;

    fprintf( stderr, "%i of %i : ", event_count + 1, events.size() );
    fprintf( stderr, "start %f end %f size %i\n", events[event_count].start, events[event_count].end, 
        events[event_count].event_tracks.size() );
    return events[event_count].event_tracks;
}

vector<Track *> & Driver::prev_event()
{
    fprintf( stderr, "Previous event: ");
    if( event_count >= events.size() || event_count < 0 )
        return realtracks;
    
    event_count -= 1;
    if( event_count >= events.size() ) // instead of "< 0" since it is a t_TAPUINT
        event_count = events.size() - 1;

    fprintf( stderr, "%i of %i : ", event_count + 1, events.size() );
    fprintf( stderr, "start %f end %f size %i\n", events[event_count].start, events[event_count].end, 
        events[event_count].event_tracks.size() );
    return events[event_count].event_tracks;
}

vector<Track *> & Driver::cur_event()
{
    if( event_count >= events.size() || event_count < 0 )
        return realtracks;
    
    return events[event_count].event_tracks;
}

const Frame * Driver::get_frame( t_TAPUINT time_sample ) // danger comment: timeout
{
    if( fft_frames.size() == 0 )
        return NULL;

    t_TAPUINT index = fft_frames.size() / 2;
    t_TAPUINT len = fft_frames.size() / 4;

    while( len )
    {
        if( time_sample < fft_frames[index]->time )
            index -= len;
        else
            index += len;

        len /= 2;
    }


    while( time_sample < fft_frames[index]->time && index != 0 )
        index--;

    while( ::abs((int)time_sample - (int)fft_frames[index]->time) > BirdBrain::hop_size() )
    {
        if( index < fft_frames.size() - 1 )
            index++;
        else
            return NULL;
    }

    return fft_frames[index];
}

const Frame * Driver::get_fake_residue( t_TAPUINT time_sample ) // danger comment: timeout
{
    if( fakeresidue.size() == 0 )
        return NULL;

    t_TAPUINT index = fakeresidue.size() / 2;
    t_TAPUINT len = fakeresidue.size() / 4;

    while( len )
    {
        if( time_sample < fakeresidue[index]->time )
            index -= len;
        else
            index += len;

        len /= 2;
    }

    while( time_sample < fakeresidue[index]->time && index != 0 )
        index--;

    while( ::abs((int)time_sample - (int)fakeresidue[index]->time) > BirdBrain::hop_size() )
    {
        if( index < fakeresidue.size() - 1 )
            index++;
        else
            return NULL;
    }

    return fakeresidue[index];
}

const Frame * Driver::get_window( )
{
    return &window;
}

bool Driver::read_window( Frame & frame )
{
	if( !m_input )
		return false;
	// buffer
    if( sf_buffer_size < frame.wlen )
    {
        // delete
        if( sf_buffer ) delete [] sf_buffer;
        // allocate
        sf_buffer = new SAMPLE[frame.wlen];
        if( !sf_buffer )
        {
            fprintf( stderr, "[driver]: cannot allocate buffer of size '%i'...\n",
                frame.wlen );
            return false;
        }
        sf_buffer_size = frame.wlen;
    }

    sf_count_t i;
    sf_count_t diff = m_input->frames() - m_input->current();
    // check to make sure there is enough data from this point
    if( diff <= 0 || diff > m_input->frames() ) return FALSE;

    // read
    sf_count_t thismuch = frame.wlen <= diff ? frame.wlen : diff;
	m_input->mtick( sf_buffer, thismuch );

    // seek back to "now" - only drive() moves read ptr
    m_input->seek( time_now, SEEK_SET );

	// copy
	memcpy( frame.waveform, sf_buffer, sizeof(SAMPLE) * thismuch );
    
    // zero pad if near the end of the file - AHA - is that why the raw residue had zeros at the end?
    for( i = thismuch; i < frame.wlen; i++ )
        frame.waveform[i] = 0.0f;

    // return
    return TRUE;
}


// write residue with given number
void Driver::write_res( t_TAPUINT count, bool clip_only )
{
    if( count != 0 )
        res_file_name = "res" + BirdBrain::toString( count ) + "_" + BirdBrain::getbase( in_file_name.c_str() );
    else
        res_file_name = string("res_").append(BirdBrain::getbase(in_file_name.c_str()));

	res_file_name = path + res_file_name; 

	write_res( clip_only );
}


// write residue to given file name
void Driver::write_res( std::string filename, bool clip_only )
{
	res_file_name = filename; 

    write_res( clip_only );
}


// write residue
void Driver::write_res( bool clip_only )
{
	sf_info_res.channels = 1;
//    sf_res = sf_open( res_file_name.c_str(), SFM_WRITE, &sf_info_res );
    sf_res = AudioCentral::instance()->m_cachemanager->opensf( res_file_name.c_str(), SFM_WRITE, &sf_info_res );

    if( !sf_res ) {
        BB_log( BB_LOG_WARNING, "Driver::write_res - Can't write res to %s", res_file_name.c_str() );
        return;
    }

    int h;
    Frame read;
    
	BB_log( BB_LOG_INFO, "Writing residue to %s", res_file_name.c_str() );

    // entire file, rather than just what was selected for analysis
    if( !clip_only )
    {
        // write some of original file:
        // - open original
        if( !m_input ) {
//            sf = AudioCentral::instance()->m_cachemanager->opensf( in_file_name.c_str(), SFM_READ, &sf_info );
			BB_log( BB_LOG_WARNING, "Driver::write_res - Nothing to read from!\n" );
			return;
        }
        // - move read head to beginning
        m_input->seek( 0, SEEK_SET );
        // - read whole file
        read.alloc_waveform( m_input->frames() % 2 ? m_input->frames() + 1 : m_input->frames() );
        read_window( read );
        // - dc block whole file (seems ineffective)
        //dcbloke.clear_sigs();
        //dcbloke.apply_filter( read );
        // - figure out when analysis starts
        int start;
        if( !fft_frames.empty() ) {
            residue = *(fft_frames[0]);
            start = (int)residue.time;
        }
        else
            start = read.wlen;
        // - write till start
        sf_write_float( sf_res, read.waveform, start ); 
        // - put another hop size samples in ola buffer
        // (this DOES NOT get shifted out before ola, because hop is intialized to 0)
        if( fft_frames.size() > 1 )
            h = (int)(fft_frames[1]->time - residue.time);
        else
            h = residue.wlen / 2;
        memcpy( ola_res.waveform, read.waveform + start, h * sizeof( SAMPLE ) );
        // - window (perhaps)
        for( int p = 0; p < h; p++ )
        {
            ola_res.waveform[p] *= (1 - (float)p/h);
        }
    }

    // get residue and write, frame by frame; all cases
    int hop = 0, next_hop = INT_MAX, hack;
    float b = 0.6f;     // onepole lowpass filter : used to be 0.7f
    SAMPLE y = 0, x = 0; 
    for( int i = 0; i < fft_frames.size(); i++ ) {
        residue = *(fft_frames[i]);

        analysis->get_res( realtracks, residue );
        residue.pol2cmp();
        rfft( residue.waveform, residue.len, FFT_INVERSE );
        //dcunbloke.apply_filter( residue ); // AAH

        assert( ola_res.wlen == residue.wlen );
        BirdBrain::ola( ola_res.waveform, residue.waveform, hop, ola_res.wlen );

        hack = residue.wlen > next_hop ? next_hop : residue.wlen;
        next_hop = (i == fft_frames.size() - 1 ? hack : (int)(fft_frames[i+1]->time - residue.time) );
        // scale back
        BirdBrain::scale_ifft( ola_res.waveform, next_hop, fft_size * 2, wnd_size );
        // low-pass filter what is about to be written
        for( int r = 0; r < next_hop; r++ )
        {
            x = ola_res.waveform[r];
            y = (1 - b)*x + b*y; 
            ola_res.waveform[r] = y; 
        }
        // write section to file
        sf_write_float( sf_res, ola_res.waveform, next_hop );
        hop = next_hop;
    }

    if( !clip_only )
    {
        // write remaining amount
        // filter the residue that's about to be written
        for( int r = 0; r < ola_res.wlen; r++ )
        {
            x = ola_res.waveform[r];
            y = (1 - b)*x + b*y; 
            ola_res.waveform[r] = y; 
        }
        // - figure out to where to copy from and copy one ola frame's worth into temporary buffer
        h = (int)residue.time + hop; // is time incremented for each sample or each 'frame'? (sample)
        SAMPLE * copy = new SAMPLE[ola_res.wlen];
        for( int j = 0; j < ola_res.wlen; j++ )
        {
            if( h + j < read.wlen )
                copy[j] = read.waveform[h + j];
            else
                copy[j] = 0.f;

            // - window (perhaps)
            copy[j] *= 0.5f * (float)j/(ola_res.wlen /*- hop*/); 
            ola_res.waveform[j] *= 0.5f * (1.0 - (float)j/(ola_res.wlen /*- hop*/));
        }
        // - OLA
        BirdBrain::ola( ola_res.waveform, copy, hop, ola_res.wlen );
        // - write
        sf_write_float( sf_res, ola_res.waveform, ola_res.wlen );
        // - copy the rest
        h += ola_res.wlen;
        if( h < read.wlen )
            sf_write_float( sf_res, read.waveform + h, read.wlen - h );

        // reset stuff
        // seek back to previous position for original/read file
        m_input->seek( time_now, SEEK_SET );
        SAFE_DELETE_ARRAY( copy );
    }

    // close res file
//    sf_close( sf_res );
	AudioCentral::instance()->m_cachemanager->closesf( sf_res );
    sf_res = NULL;
}


// alternative to run, for when the filename isn't changed
// assume driver has already been set up once using run, previously
bool Driver::cruise( int start, int stop, float freq_min, float freq_max, xtoy * det_thresh, 
             int min_points, int max_gap, float error_f, float noise_r, int num_tracks,
             float group_harm, float group_freq, float group_amp, float group_overlap, float group_on, 
             float group_off, float group_minlen, bool group)
{
double begin_time = clock();

    if( !this->analysis )
    {
        BB_log( BB_LOG_SEVERE, "(driver): can't cruise; analysis has not been set; first create using 'run'" ); 
        return false;
    }
    if( !this->m_input )
    {
        BB_log( BB_LOG_SEVERE, "(driver): no input!!! :(" );
        return false;
    }
    
    // clear stuff
    int it; 
    for( it = 0; it < tracks.size(); it++ ) { SAFE_DELETE( tracks[it] ); }
    for( it = 0; it < realtracks.size(); it++ ) { SAFE_DELETE( realtracks[it] ); }
    for( it = 0; it < fakeresidue.size(); it++ ) { SAFE_DELETE( fakeresidue[it] ); }
    events.clear();
    tracks.clear();
    realtracks.clear();
    fakeresidue.clear();
    residue.zero();
    ola_syn.zero();
    ola_res.zero();
    m_input->seek( 0, SEEK_SET );
    event_count = 0; 

    // set variables
    BirdBrain::our_max_tracks = num_tracks;
    BirdBrain::our_freq_min = (freq_min >= 0.0f ? freq_min : 0.0f);
    BirdBrain::our_freq_max = (freq_max >= 0.0f ? freq_max : 0.0f);
    m_start = start;
    m_stop = stop;
    m_freq_min = freq_min;
    m_freq_max = freq_max;

    // analysis stuff
    analysis->max_tracks = num_tracks; // analysis uses this variable, set to BB::our_max_tracks upon instantiation
    analysis->threshold = det_thresh;
    analysis->cache_thresh( fft_size / 2 );
    analysis->minpoints = min_points;
    analysis->maxgap = max_gap;
    analysis->error_f = error_f;
    analysis->noise_ratio = noise_r;
    
    // grouping stuff
    m_group = group;
    analysis->harm_error = group_harm;
    analysis->freq_mod_error = group_freq;
    analysis->amp_mod_error = group_amp;
    analysis->min_overlap_frac = group_overlap; 
    analysis->onset_error = group_on;
    analysis->offset_error = group_off;
    analysis->min_event_length = group_minlen;

    // update m_start and m_stop
    int stopind = 0, startind = 0; 
    for( int n = 0; n < m_frames; n++ )
    {
        if( fabs(m_start - m_times[n]) < fabs(m_start - m_times[startind]) )
            startind = n;
        if( fabs(m_stop - m_times[n]) < fabs(m_stop - m_times[stopind]) )
            stopind = n; 
    }
    m_start = (int)m_times[startind]; 
    m_stop = (int)m_times[stopind]; 
    frame_number = startind;
    
// is it better to read each time? 
    // clear old stuff 
    for( it = 0; it < fft_frames.size(); it++ ) { SAFE_DELETE( fft_frames[it] ); }
    for( it = 0; it < peak_frames.size(); it++ ) { SAFE_DELETE( peak_frames[it] ); }
    fft_frames.clear(); 
    peak_frames.clear(); 

    // read
    Track * ppin; int ppinsize; freqpolar info;
    BB_log( BB_LOG_INFO, "Reading frames" ); 
    for( it = startind; it <= stopind; it++ )
    {
        // read fft frame
        fseek( fftfin, fft_framestart + it * fft_framesize, SEEK_SET ); 
        Frame * fr = new Frame;
        fread( &(fr->time), sizeof(t_TAPTIME), 1, fftfin ); 
        fread( &(fr->len), sizeof(t_TAPUINT), 1, fftfin ); 
        fread( &(fr->wlen), sizeof(t_TAPUINT), 1, fftfin ); // kind of useless
        fread( &(fr->wsize), sizeof(t_TAPUINT), 1, fftfin ); // kind of useless
        fr->alloc_waveform( 2 * fr->len ); 
        fr->alloc_pol(); 
        fread( fr->pol, sizeof(polar), fr->len, fftfin ); 
        fr->bins2freqs( BirdBrain::srate() ); // BirdBrain::srate() was set to sf_info.samplerate in Driver::open() 
        fr->pol2cmp(); 
        fft_frames.push_back( fr ); 

        // read peaks frame
        fseek( ppfin, pp_framestart + it * pp_framesize, SEEK_SET ); 
        ppin = new Track;
        fread( &ppinsize, sizeof(int), 1, ppfin ); 
        ppin->history.reserve( ppinsize ); 
        for( int p = 0; p < ppinsize; p++ )
        {
            fread( &info, sizeof(freqpolar), 1, ppfin ); 
            ppin->history.push_back( info ); 
        }   
        peak_frames.push_back( ppin ); 
    }
    BB_log( BB_LOG_INFO, "done" ); 
    // set frame number
    frame_number = 0; 
// end of "is it better..."
    
    // drive and dine and stuff
    drive( m_start );
    last_hop = 0;

    while( !done( m_stop ) )
    {
        dine();
        drive( BirdBrain::hop_size() );
        // cout << jack.now() << endl;
    }

    // get the tracks
    drink();
    
double end_time = clock(); 

    BB_log( BB_LOG_INFO, "Total processing time: %f seconds, cruise", (end_time - begin_time)/CLOCKS_PER_SEC ); 

    return true;
}


Driver * run( char * filename, int start, int stop, float freq_min, float freq_max, xtoy * det_thresh, 
             int min_points, int max_gap, float error_f, float noise_r, int num_tracks, int fftsize, int wndsize,
             float group_harm, float group_freq, float group_amp, float group_overlap, float group_on,  
             float group_off, float group_minlen, bool group )
//int main( int argc, char ** argv )
{
double begin_time = clock();    

    Driver * jack = new Driver;

    // set the max numbers of tracks (for both analysis and synthesis)
    // this must be done before calling set() for any Driver...
    BirdBrain::our_max_tracks = num_tracks;
    // set fft size
    BirdBrain::our_fft_size = fftsize;
    // set wnd size
    BirdBrain::our_wnd_size = wndsize;
    BirdBrain::our_syn_wnd_size = wndsize;
    // set hop size
    BirdBrain::our_hop_size = wndsize / 4;
    // set endpoints
    BirdBrain::our_freq_min = (freq_min >= 0.0f ? freq_min : 0.0f);
    BirdBrain::our_freq_max = (freq_max >= 0.0f ? freq_max : 0.0f);
    jack->m_start = start;
    jack->m_stop = stop;
    jack->m_freq_min = freq_min;
    jack->m_freq_max = freq_max;

    // open driver; set up analysis
    jack->set( wndsize, fftsize, 1 );
	if( filename != NULL )
		jack->open( filename );
	else
		jack->open_mic();
    jack->ana()->threshold = det_thresh;
    jack->ana()->cache_thresh( fftsize / 2 );
    jack->ana()->minpoints = min_points;
    jack->ana()->maxgap = max_gap;
    jack->ana()->error_f = error_f;
    jack->ana()->noise_ratio = noise_r;
    // grouping stuff
    jack->m_group = group;
    jack->ana()->harm_error = group_harm;
    jack->ana()->freq_mod_error = group_freq;
    jack->ana()->amp_mod_error = group_amp;
    jack->ana()->min_overlap_frac = group_overlap; 
    jack->ana()->onset_error = group_on;
    jack->ana()->offset_error = group_off;
    jack->ana()->min_event_length = group_minlen;
    jack->drive( jack->m_start );
    jack->last_hop = 0;
    while( !jack->done( jack->m_stop ) )
    {
        jack->dine();
        jack->drive( BirdBrain::hop_size() );
		// cout << jack.now() << endl;
    }

    // get the tracks
    jack->drink();

    // don't brake
    // jack->brake();

double end_time = clock(); 
    
    BB_log( BB_LOG_INFO, "Total processing time: %f seconds, run", (end_time - begin_time)/CLOCKS_PER_SEC );  

    return jack;
}
