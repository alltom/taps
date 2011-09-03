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
// name: ui_library.cpp
// desc: birdbrain ui library
//
// authors: Ananya Misra (amisra@cs.princeton.edu)
//          Ge Wang (gewang@cs.princeton.edu)
//          Perry R. Cook (prc@cs.princeton.edu)
// after EST. January 14, 2005, 8:37 p.m. Friday
//-----------------------------------------------------------------------------
#include "ui_library.h"
#include <algorithm>
#include <limits.h>
using namespace std;


void wait_for( Template * me ); 


//-----------------------------------------------------------------------------
// name: nextFakePoisson()
// desc: new math we invented
//-----------------------------------------------------------------------------
double Poisson::nextFakePoisson( double lambda ) {
    double elambda = ::exp(-1*lambda);
    double product = 1;
    double oldproduct = 1; // no comment
    int count =  0;
    int result=0;

    while (product >= elambda) {
        oldproduct = product;
        product *= rand() / (double)RAND_MAX;
        result = count;
        count++; // keep result one behind
    }

    double d = (oldproduct - elambda) / (oldproduct - product);

    return (double)result + d;
}


//-----------------------------------------------------------------------------
// name: nextPoisson()
// desc: a more boring version of nextFakePoisson()
//-----------------------------------------------------------------------------
int Poisson::nextPoisson( double lambda ) {
    double elambda = ::exp( -1*lambda );
    double product = 1;
    int count =  0;
    int result=0;
    while (product >= elambda) {
        product *= rand() / (double)RAND_MAX;
        result = count;
        count++; // keep result one behind
    }
    return result;
}


//-----------------------------------------------------------------------------
// name: nextExponential()
// desc: generate interval until the next poisson(lambda) event
//-----------------------------------------------------------------------------
double Poisson::nextExponential( double one_over_lambda ) {
    double randx;
    double result;
    randx = rand() / (double)RAND_MAX;
    result = -1 * one_over_lambda * ::log(randx);
    return result;
}


Poisson * Poisson::our_fish = NULL;
Periodic * Periodic::our_periodic = NULL;
FunkyRand * FunkyRand::our_rand = NULL;
PerryRand * PerryRand::our_prand = NULL;
Gaussian * Gaussian::our_gaussian = NULL;


// static stuff
Library * Library::our_instance = NULL;
Library * Library::our_matches = NULL;

Library * Library::instance()
{
    if( !our_instance )
        our_instance = new Library;
    return our_instance;
}


Library * Library::matches()
{
    if( !our_matches )
        our_matches = new Library;

    return our_matches;
}


Library::~Library()
{
    BB_log( BB_LOG_INFO, "Deleting library" ); 
    for( long i = 0; i < templates.size(); i++ ) { SAFE_DELETE( templates[i] ) };

    templates.clear();
}


void Template::play( AudioBus * bus )
{
//	fprintf( stdout, "Adding %x (%s) to bus %x\n", this, this->name.c_str(), bus );
	// stop if needed
	if( lastbus != NULL && lastbus != bus && this->type != TT_SCRIPT ) // not sure why it can't be script, copied from ui_scripting.cpp
		lastbus->remove( this );
	// save
	lastbus = bus;
    // if it successfully rewinds, play
    if( this->rewind() ) 
		bus->play( this, FALSE, FALSE ); // not solo and don't rewind again!
}

void Template::stop()
{
    m_stop_asap = TRUE;
}

void Template::init( time_t myid )
{
    type = 0; 
    name = "[nobody]"; 
    typestr = "[not type]";
    time_stretch = 1.0f; 
    freq_warp = 1.0f; 
    gain = 1.0f; 
    pan = 0.5f; 
    m_delete = FALSE; 
    periodicity = 0.5f; 
    density = 0.5f; 
    polluted = TRUE; 
    mybus = -1; 
	lastbus = NULL;

    features = NULL;

    if( myid == 0 )
        id = time(0); // this is not unique if copying from ChucK script!!
    else
        id = myid; 

    // log
    // fprintf( stderr, "new template with id: %i ; next_id = %i\n", id, next_id );
    BB_log( BB_LOG_FINE, "creating new template with id: %d...", id );
}

void Template::copy_params( const Template & rhs )
{
    // basic params
	this->name = rhs.name; // not adding "Copy" here because then it renames autocopies too.
    this->time_stretch = rhs.time_stretch;
    this->freq_warp = rhs.freq_warp;
    this->gain = rhs.gain;
    this->pan = rhs.pan;
    this->periodicity = rhs.periodicity;
    this->density = rhs.density;

    // pitch/time tables
    this->timetable = rhs.timetable;
    this->pitchtable = rhs.pitchtable;

    // flag for pollution
    this->polluted = TRUE;
}

void Template::set_param( int which, double value )
{ 
    switch( which )
    {
    case GAIN:
        gain = value;
        break;

    case PAN:
        pan = value;
        break;

    case TIME_STRETCH:
        time_stretch = value;
        break;

    case FREQ_WARP:
        freq_warp = quantize_pitch( value );
        break;

    case PERIODICITY:
        //periodicity = value;
        break;

    case DENSITY:
        //density = value;
        break;

    case LOOP_ME: 
        break;

    default:
        // we accompished nothing here
        break;
    }

    polluted = TRUE; 
}


void Template::recompute()
{
    polluted = FALSE;
}


// quantize given value to nearest time on timetable, if there is one
double Template::quantize_time( double value )
{
    if( timetable.size() == 0 )
        return value;

    // binary search
    int left = 0, right = timetable.size() - 1, center;
    while( right - left > 1 ) 
    {
        center = (left + right)/2; 
        if( value < timetable[center] ) 
            if( center > 0 && ::fabs( timetable[center] - value ) < ::fabs( timetable[center - 1] - value ) )
                return timetable[center];
            else
                right = center - 1;
        else if( value > timetable[center] )
            if( center < timetable.size() - 1 && 
            ::fabs( timetable[center] - value ) < ::fabs( timetable[center + 1] - value ) )
                return timetable[center] ; 
            else
                left = center + 1; 
        else
            return value;
    }
    // now left and right are identical or adjacent, pick nearest
    if( ::fabs( timetable[right] - value ) < ::fabs( timetable[left] - value ) )
        return timetable[right];
    else
        return timetable[left];
}


// quantize given value to nearest frequency on pitchtable, if there is one
double Template::quantize_pitch( double value )
{
    if( pitchtable.size() == 0 )
        return value;

    // binary search
    int left = 0, right = pitchtable.size() - 1, center;
    while( right - left > 1 ) 
    {
        center = (left + right)/2; 
        if( value < pitchtable[center] ) 
            if( center > 0 && ::fabs( pitchtable[center] - value ) < ::fabs( pitchtable[center - 1] - value ) )
                return pitchtable[center];
            else
                right = center - 1;
        else if( value > pitchtable[center] )
            if( center < pitchtable.size() - 1 && 
            ::fabs( pitchtable[center] - value ) < ::fabs( pitchtable[center + 1] - value ) )
                return pitchtable[center]; 
            else
                left = center + 1; 
        else
            return value;
    }
    // now left and right are identical or adjacent, pick nearest
    if( ::fabs( pitchtable[right] - value ) < ::fabs( pitchtable[left] - value ) )
        return pitchtable[right];
    else
        return pitchtable[left];
}


// sorting comparison
struct SortAscending
{
    bool operator() ( double lhs, double rhs ) { return lhs < rhs; }
};


// read pitch or time table from file
void Template::read_table( char * filename )
{
    // open
    std::ifstream in; 
    in.open( filename, ios::in );
    if( !in.good() ) 
    {
        BB_log( BB_LOG_WARNING, "Cannot open %s to read quantization table", filename );
        return;
    }
    
    // check pitch or time
    bool readtime, repeat = false; 
    char type;
    char junk[101];
    do {
        repeat = false;
        in >> type; 
        switch( type ) {
        case 'p':
        case 'P':
        case 'f':
        case 'F':
            readtime = false;
            break;
        case 't':
        case 'T':
            readtime = true;
            break;
        case '#':
            in.getline( junk, 100, '\n' );
            repeat = true;
            break;
        default:
            BB_log( BB_LOG_WARNING, "Quantization table %s has the wrong format", filename ); 
            return;
        }
    }
    while( repeat ); 

    // clear appropriate "table" (vector)
    if( readtime )
        timetable.clear(); 
    else
        pitchtable.clear();

    // read number of data points
    int npoints;
    in >> npoints;

    // read in actual info, no control for too few table entries
    double data;
    for( int n = 0; n < npoints; n++ ) {
        in >> data; 
        if( readtime )
            timetable.push_back( (t_TAPTIME)data );
        else
            pitchtable.push_back( data );
    }

    // sort in ascending order
    SortAscending sorter;
    if( readtime )
        std::sort( timetable.begin(), timetable.end(), sorter );
    else
        std::sort( pitchtable.begin(), pitchtable.end(), sorter );

    // close file
    in.close(); 
}


void Template::clear_features()
{
    SAFE_DELETE_ARRAY( features );
}

Deterministic::Deterministic( time_t myid )
    : Template( myid ) 
{
    src = NULL;
    type = TT_DETERMINISTIC;
    typestr = "deterministic";

    syn.setTracks( tracks ); 
    start_time = end_time = 0;
}

Deterministic::Deterministic( const std::vector<Track *> & realtracks, time_t myid )
    : Template( myid ) 
{ 
    src = NULL;
    type = TT_DETERMINISTIC;
    typestr = "deterministic";

    int i;
    Track * track = NULL;

    // clone pointers
    for( i = 0; i < realtracks.size(); i++ )
    {
        track = new Track;
        *track = *realtracks[i];
        tracks.push_back( track );

        // put copy in working
        //track = new Track;
        //working.push_back( track );
    }

    syn.setTracks( tracks ); 

	// timing info
	if( !tracks.empty() ) {
		start_time = tracks.back()->start;
		end_time = 0;
		for( i = 0; i < tracks.size(); i++ ) {
			if( start_time > tracks[i]->start )
				start_time = tracks[i]->start;
			if( end_time < tracks[i]->end )
				end_time = tracks[i]->end; 
		}
	}
	else {
		start_time = end_time = 0;
	}
}

Deterministic::~Deterministic()
{ 
    BB_log( BB_LOG_FINE, "Deleting deterministic template" ); 
    SAFE_DELETE( src ); 
    /* don't forget to delete tracks */ /* uh... forgot... */
    for( int i = 0; i < tracks.size(); i++ ) // tracks and working should have the same size
    {
        SAFE_DELETE( tracks[i] ); 
        //SAFE_DELETE( working[i] ); 
    }
}

void Deterministic::recompute( )
{
/* IF the synthesis object is of type SYN (synthesizes whole frame in one go)
    // only if poluted
    if( !polluted )
        return;

    // hack, to allow big enough synthesis frame, without making it unnecessarily big
    //  as that messes up the now butter in analysis.
    if( time_stretch > 1 )
    {
        BirdBrain::our_syn_wnd_size = (t_TAPUINT)(BirdBrain::wnd_size() * time_stretch + 0.5);
        BirdBrain::our_syn_wnd_size += (BirdBrain::syn_wnd_size() % 2);
    }

    // fill synthesis cache
    syn.setTracks( tracks );
    syn.time_stretch = this->time_stretch;
    syn.freq_warp = this->freq_warp;
    syn.synthesize( cached_sound );

    // src
    SAFE_DELETE( src );
    src = new AudioSrcFrame( &cached_sound );
    // don't delete automatically
    src->m_delete = FALSE; 
//*/
///* IF the synthesis object is of type SYNFAST (synthesizes requested number of samples)

    // rewind
    rewind();
    // new recompute
    if( !polluted )
        return;
    syn.setTracks( tracks );  
//*/
    // up call (also in old version)
    Template::recompute();
}

t_TAPTIME Deterministic::get_start( bool redo ) {
	if( redo ) {
		// re-calculate as tracks could be changed in group face
		if( !tracks.empty() ) {
			start_time = tracks.back()->start;
			end_time = 0;
			for( int i = 0; i < tracks.size(); i++ ) {
				if( start_time > tracks[i]->start )
					start_time = tracks[i]->start;
				if( end_time < tracks[i]->end )
					end_time = tracks[i]->end; 
			}
		}
		else {
			start_time = end_time = 0;
		}
	}
	return start_time;
}

t_TAPTIME Deterministic::get_end( bool redo ) {
	if( redo ) {
		// re-calculate as tracks could be changed in group face
		if( !tracks.empty() ) {
			start_time = tracks.back()->start;
			end_time = 0;
			for( int i = 0; i < tracks.size(); i++ ) {
				if( start_time > tracks[i]->start )
					start_time = tracks[i]->start;
				if( end_time < tracks[i]->end )
					end_time = tracks[i]->end; 
			}
		}
		else {
			start_time = end_time = 0;
		}
	}
	return end_time;
}

t_TAPBOOL Deterministic::stick( SAMPLE * buffer, t_TAPUINT num_frames )
{
//    assert( m_done == FALSE );
/* IF the synthesis object is of type SYN (synthesizes whole frame in one go)
    // are we done?
    if( m_stop_asap )
        return FALSE;

    assert( src != NULL );
    src->set_gain( gain );
    src->set_pan( pan );
    return src->stick( buffer, num_frames ); 
//*/
///* IF the synthesis object is of type SYNFAST (synthesizes requested number of samples)
    if( m_stop_asap )
        return FALSE;

    // set syn parameters
    syn.time_stretch = this->time_stretch;
    syn.freq_warp = this->freq_warp; 
    // clear frame to stick into (cached_sound is a misnomer)
    cached_sound.zero();
    if( cached_sound.wlen < num_frames )
        cached_sound.alloc_waveform( num_frames + num_frames % 2 ); 
    cached_sound.wsize = num_frames; 
    syn.synthesize( cached_sound ); 
    // copy sound into buffer, converting to stereo
    mono2stereo( cached_sound.waveform, num_frames, buffer ); 
    // gain
    gain_buf( buffer, num_frames * 2, gain );
    // pan
    pan_buf( buffer, num_frames, pan );
	// time
	m_time_elapsed += num_frames;
    // phew
    return cached_sound.wsize > 0;//*/
}


Template * Deterministic::copy( bool copyid ) const
{
    Deterministic * deb = new Deterministic( this->tracks, copyid ? this->id : 0 );
    deb->copy_params( *this );

    return deb;
}


PVCTemp::PVCTemp( time_t myid )
    : Template( myid ), pvc_window_size(1024), pvc_bufsize(512)
{
    // pvc stuff
    pvc_hopsize = pvc_window_size / 8;
    set_pvc_buffers();
    m_pvc = new PVC( pvc_window_size, pvc_bufsize, 2048 /* 0/2048/pool_size */ ); // not bad with no pool (size 0)
    //m_pvc = new PVCtoo( event.waveform, event.wsize, pvc_hopsize, pvc_window_size, pvc_bufsize, 2048 );
}

PVCTemp::PVCTemp( const PVCTemp & rhs, time_t myid )
    : Template( myid )
{
    // pvc stuff
    pvc_window_size = rhs.pvc_window_size; 
    pvc_bufsize = rhs.pvc_bufsize;
    pvc_hopsize = pvc_window_size / 8; 
    set_pvc_buffers(); 
    m_pvc = rhs.m_pvc->pv_copy(); 
    //m_pvc = (PVCtoo *)(rhs.m_pvc)->pv_copy();
}

PVCTemp::~PVCTemp()
{
    BB_log( BB_LOG_FINE, "Deleting pvc template" ); 
    SAFE_DELETE( src );
    // pvc stuff
    SAFE_DELETE_ARRAY( pvc_buffer ); 
    SAFE_DELETE_ARRAY( pvc_extras ); 
    SAFE_DELETE( m_pvc ); 
    SAFE_DELETE_ARRAY( silence );
}


// initialize pvc buffers (and related variables) after their sizes have been defined
void PVCTemp::set_pvc_buffers()
{
    pvc_which = 0;
    pvc_buffer = new SAMPLE[pvc_bufsize];
    pvc_extras = new SAMPLE[pvc_bufsize]; 
    pvc_numextras = 0;
    memset( pvc_extras, 0, pvc_bufsize * sizeof(SAMPLE) );
    memset( pvc_buffer, 0, pvc_bufsize * sizeof(SAMPLE) );
    pvc_win[0] = NULL; pvc_win[1] = NULL; 
    framedone = false;
    silence = new SAMPLE[pvc_bufsize];
    memset( silence, 0, pvc_bufsize * sizeof(SAMPLE) );
}


// probably not good
t_TAPBOOL PVCTemp::stick( SAMPLE * buffer, t_TAPUINT num_frames )
{
//  return FALSE;
//    assert( m_done == FALSE );

    // first time
    static bool first = true;

    // are we done?
    if( m_stop_asap )
    {
        first = true;
        return FALSE;
    }

    // re-starting?
    if( first )
    {
        framedone = FALSE;
        memset( pvc_extras, 0, pvc_bufsize * sizeof(SAMPLE) );
        pvc_numextras = 0; 
        m_pvc->pv_clear();
        first = false;
    }

    assert( src != NULL );

    int done = 0;
    int toget = 0; 
    bool ongoing = true;

    // bad hack
    bool vocode = (time_stretch <= 0.956 || time_stretch >= 1.046 || freq_warp <= 0.956 || freq_warp >= 1.046);
    if( !vocode )
    {
        src->set_gain( gain );
        src->set_pan( pan );
        return src->stick( buffer, num_frames );
    }

    // if you must use phase vocoder:

    // unused samples computed from previous stick
    if( pvc_numextras > 0 )
    {
        int tocopy = (pvc_numextras < num_frames) ? pvc_numextras : num_frames; 
        memcpy( buffer, pvc_extras, tocopy * sizeof(SAMPLE) );
        memcpy( pvc_extras, pvc_extras + tocopy, (pvc_bufsize - tocopy) * sizeof(SAMPLE) );
        memset( pvc_extras + pvc_bufsize - tocopy, 0, tocopy * sizeof(SAMPLE) );
        pvc_numextras -= tocopy;
        done += tocopy;
    }

    // need to add some silence at the end for complete overlap adding maybe? (clean up later)
    if( !framedone )
    {
        // analyze all
        while( src->mtick( pvc_buffer, pvc_bufsize ) )
        {
            if( !m_pvc->pv_analyze( pvc_buffer, pvc_hopsize ) ) break; 
        }
        int h = (int)(pvc_hopsize * time_stretch + .5f);
        for( int p = h; p < pvc_bufsize; p += h )
            if( !m_pvc->pv_analyze( silence, pvc_hopsize ) ) break; 
        // watch out for the crash
        framedone = true;
        
        // get phase windows out of queue
        while( pvc_win[pvc_which] = m_pvc->pv_deque() )
        {
            // freq shift ("processing")
            m_pvc->pv_freq_shift( pvc_win[pvc_which], freq_warp );
            // unwrap phase
            //m_pvc->pv_unwrap_phase( pvc_win[pvc_which] );
            //if( pvc_win[!pvc_which] )
            //  m_pvc->pv_phase_fix( pvc_win[!pvc_which], pvc_win[pvc_which], time_stretch * pvc_hopsize ); // *hopsize works in this method too! :| gack
            m_pvc->pv_unwrap_phase_( pvc_win[!pvc_which], pvc_win[pvc_which] ); 
            m_pvc->pv_phase_fix_( pvc_win[!pvc_which], pvc_win[pvc_which], time_stretch /* pvc_hopsize*/ ); // *hopsize is a bug but has an interesting effect
            // overlap/add
            if( !m_pvc->pv_overlap_add( pvc_win[pvc_which], (t_TAPUINT)(time_stretch*pvc_hopsize + .5f) ) ) break;
            pvc_which = !pvc_which;
            // reclaim the window
            m_pvc->pv_reclaim( pvc_win[pvc_which] );
        }
    }

    while( done < num_frames )
    {
        toget = (num_frames - done < pvc_bufsize) ? (num_frames - done) : pvc_bufsize;      

        // gulp
        // get buffer
        if( m_pvc->pv_get_ready_len() == 0 && framedone )
        {
            ongoing = false;
            framedone = false;
            memset( pvc_extras, 0, pvc_bufsize * sizeof(SAMPLE) );
            pvc_numextras = 0; 
            m_pvc->pv_clear(); 
            break;
        }
        m_pvc->pv_synthesize( pvc_buffer );

        // phew (spoke too soon)    
        memcpy( buffer + done, pvc_buffer, toget * sizeof(SAMPLE) );  
        memcpy( pvc_extras + pvc_numextras, pvc_buffer + toget, (pvc_bufsize - toget) * sizeof(SAMPLE) ); 
        pvc_numextras += (pvc_bufsize - toget); 
        done += toget;
    }

//    buffer[num_frames - 1] = 0.1;
    mono2stereo( buffer, num_frames ); 
    gain_buf( buffer, num_frames * 2, gain * 2 ); 
    pan_buf( buffer, num_frames, pan ); 
	// time
	m_time_elapsed += num_frames;
    pvc_win[0] = pvc_win[1] = NULL; 
    return ongoing; 
}



Raw::Raw( const Frame & event, time_t myid ) 
    : PVCTemp( myid )
{ 
    sound = event; 
    src = new AudioSrcFrame( &sound ); 
    src->m_delete = FALSE;
    type = TT_RAW; 
    typestr = "raw";   
}


// copy constructor specifically for use with alternative pvc implementation (in progress (regress))
Raw::Raw( const Raw &rhs, time_t myid ) : PVCTemp( rhs, myid ) 
{
    // basic stuff
    sound = rhs.sound; 
    src = new AudioSrcFrame( &sound );
    src->m_delete = FALSE;
    type = TT_RAW;
    typestr = "raw";  

    // remaining parameters
    this->copy_params( rhs ); 
}


/* // good
t_TAPBOOL Raw::stick( SAMPLE * buffer, t_TAPUINT num_frames )
{
//    assert( m_done == FALSE );

    // are we done?
    if( m_stop_asap )
        return FALSE;

    assert( src != NULL );
    src->set_gain( gain );
    src->set_pan( pan );
    return src->stick( buffer, num_frames );
}
*/


/*
// probably not good (for use with pvctoo)
t_TAPBOOL Raw::stick( SAMPLE * buffer, t_TAPUINT num_frames )
{
//  return FALSE;
//    assert( m_done == FALSE );

    // are we done?
    if( m_stop_asap )
        return FALSE;

    assert( src != NULL );

    int done = 0;
    int toget = 0; 
    bool ongoing = true;

    // bad hack
    bool vocode = (time_stretch <= 0.956 || time_stretch >= 1.046 || freq_warp <= 0.956 || freq_warp >= 1.046);
    if( !vocode && !false )
    {
        src->set_gain( gain );
        src->set_pan( pan );
        return src->stick( buffer, num_frames );
    }

    // if you must use phase vocoder:

    // unused samples computed from previous stick
    if( pvc_numextras > 0 )
    {
        int tocopy = (pvc_numextras < num_frames) ? pvc_numextras : num_frames; 
        memcpy( buffer, pvc_extras, tocopy * sizeof(SAMPLE) );
        memcpy( pvc_extras, pvc_extras + tocopy, (pvc_bufsize - tocopy) * sizeof(SAMPLE) );
        memset( pvc_extras + pvc_bufsize - tocopy, 0, tocopy * sizeof(SAMPLE) );
        pvc_numextras -= tocopy;
        done += tocopy;
    }

    // need to add some silence at the end for complete overlap adding maybe? (clean up later)
    if( !framedone )
    {
        // watch out for the crash
        framedone = true;
        
        // get phase windows out of queue
        while( pvc_win[pvc_which] = m_pvc->pv_deque() )
        {
            // freq shift ("processing")
            m_pvc->pv_freq_shift( pvc_win[pvc_which], freq_warp );
            // unwrap phase
            m_pvc->pv_unwrap_phase_( pvc_win[!pvc_which], pvc_win[pvc_which] ); 
            m_pvc->pv_phase_fix_( pvc_win[!pvc_which], pvc_win[pvc_which], time_stretch * pvc_hopsize ); // *hopsize is a bug but has an interesting effect
            // overlap/add
            m_pvc->pv_overlap_add( pvc_win[pvc_which], (t_TAPUINT)(time_stretch*pvc_hopsize + .5f) );
            pvc_which = !pvc_which;
            // reclaim the window
            m_pvc->pv_reclaim( pvc_win[pvc_which] );
        }
    }

    while( done < num_frames )
    {
        toget = (num_frames - done < pvc_bufsize) ? (num_frames - done) : pvc_bufsize;      

        // gulp
        // get buffer
        if( m_pvc->pv_get_ready_len() == 0 && framedone )
        {
            ongoing = false;
            framedone = false;
            memset( pvc_extras, 0, pvc_bufsize * sizeof(SAMPLE) );
            pvc_numextras = 0; 
            m_pvc->pv_clear(); 
            break;
        }
        m_pvc->pv_synthesize( pvc_buffer );

        // phew (spoke too soon)    
        memcpy( buffer + done, pvc_buffer, toget * sizeof(SAMPLE) );  
        memcpy( pvc_extras + pvc_numextras, pvc_buffer + toget, (pvc_bufsize - toget) * sizeof(SAMPLE) ); 
        pvc_numextras += (pvc_bufsize - toget); 
        done += toget;
    }

//    buffer[num_frames - 1] = 0.1;
    mono2stereo( buffer, num_frames ); 
    gain_buf( buffer, num_frames * 2, gain * 2 ); 
    pan_buf( buffer, num_frames, pan ); 

    pvc_win[0] = pvc_win[1] = NULL; 
    return ongoing; 
}
*/

Template * Raw::copy( bool copyid ) const
{
    Raw * ron = new Raw( *this, copyid ? this->id : 0 ); 
	ron->copy_params(*this);
    return ron;
}


Transient::Transient( const Frame & event, time_t myid )
    : Raw( event, myid )
{
    type = TT_TRANSIENT;
    typestr = "transient";
}
 
Transient::Transient( const Transient & rhs, time_t myid )
    : Raw( rhs, myid )
{
    type = TT_TRANSIENT;
    typestr = "transient"; 
}


Template * Transient::copy( bool copyid ) const
{
//    Transient * tran = new Transient( this->sound, copyid ? this->id : 0 );
//    tran->copy_params( *this );

    Transient * tran = new Transient( *this, copyid ? this->id : 0 );
    return tran;
}


File::File( const std::string & path, time_t myid ) 
    : PVCTemp( myid ) 
{
    filename = path; 
    src = new AudioSrcFile; 
    goodtogo = ((AudioSrcFile *)src)->open( filename.c_str() ); 
    src->m_delete = FALSE;
    type = TT_FILE; 
    typestr = "soundfile"; 
    name = BirdBrain::getname( filename.c_str() ); 
}


/*t_TAPBOOL File::stick( SAMPLE * buffer, t_TAPUINT num_frames )
{
//    assert( m_done == FALSE );

    // are we done?
    if( m_stop_asap )
        return FALSE;

    assert( src != NULL );
    src->set_gain( gain );
    src->set_pan( pan );
    return src->stick( buffer, num_frames );
}
*/

Template * File::copy( bool copyid ) const
{
    File * phil = new File( this->filename, copyid ? this->id : 0 );
    phil->copy_params( *this );

    return phil;
}


#ifdef __PLATFORM_WIN32__
unsigned __stdcall ts_go( void * data )
#else
void * ts_go( void * data )
#endif
{
    BB_log( BB_LOG_INFO, "you are now entering the ts_go() function..." );

    Residue * res = (Residue *)data;
    res->thread_done = false;

    int samples, write = 1;
    
    while( !res->shutup ) {
        // Zero the trees
        res->ts->resetTrees();
        // Read file (total_levels will never be > CUTOFF because that's the extent to which the slider goes)
        samples = res->tsio->ReadSoundFile( res->tsio->ifilename, res->ts->tree->values(), 1 << res->requested_levels/*res->ts->tree->getSize()*/ );
		if( samples <= 0 ) 
        {
            BB_log( BB_LOG_SEVERE, "ts_go(): read %i samples from the file %s", samples, res->tsio->ifilename );
            res->shutup = true;
            break;
        }
//fprintf(stderr, "Read %d samples (%d); requested: %d\n", samples, lg(samples), res->requested_levels); 
        // Change levels accordingly
        res->ts->resetTreeLevels( lg( samples ) );
        res->total_levels = res->ts->tree->getLevels();
        res->mutex.acquire();
        BB_log( BB_LOG_FINE, "ts_go : acquired mutex" );
        if( res->ts->setup() ) {
            res->ts->synth();
            // keep trying to write to file/buffer, until written or told to shut up
            while( !res->shutup && 
                !(write = res->tsio->WriteSoundFile( res->tsio->ofilename, res->ts->outputSignal(), res->ts->tree->getSize() ) ) ) {
                res->mutex.release();
                BB_log( BB_LOG_FINE, "ts_go : released mutex" );
                usleep( 10000 );
                res->mutex.acquire();
                BB_log( BB_LOG_FINE, "ts_go : acquired mutex" );
            }
        }
        res->mutex.release();
        BB_log( BB_LOG_FINE, "ts_go : released mutex" );
        
        // may work 
        /*if( res->lefttree ) {
            if( res->ts->lefttree == NULL ) {
                res->ts->lefttree = new Tree();
                res->ts->lefttree->initialize( res->ts->tree->getLevels() );
            }
            res->ts->lefttree->setValues( res->ts->tree->values(), res->ts->tree->getSize() );
        }*/
    }

    BB_log( BB_LOG_INFO, "you are now leaving the ts_go() function..., good luck." );

    res->thread_done = true;

    return 0;
}


#ifdef __PLATFORM_WIN32__
unsigned __stdcall olar_go( void * data )
#else
void * olar_go( void * data )
#endif
{
    BB_log( BB_LOG_INFO, "you are now entering the olar_go() function..." );

    Residue * res = (Residue *)data;
    res->thread_done = false;
	int samples, write = 1;

    while( !res->shutup ) {
		res->mutex.acquire();
		BB_log(BB_LOG_FINE, "olar_go : acquired mutex");
		// get the next segment
		samples = res->olar->next_segment();
		BB_log(BB_LOG_INFO, "olar_go: %d samples", samples);
		if( samples > 0 ) {
			// keep trying to write to file/buffer, until written or told to shut up
			while( !res->shutup &&
				   !(write = res->olar->WriteSoundFile(res->olar->ofilename, res->olar->outputSignal(), samples)) )			{
				res->mutex.release();
				BB_log(BB_LOG_FINE, "olar_go : released mutex");
				usleep(10000);
				res->mutex.acquire();
				BB_log(BB_LOG_FINE, "olar_go : acquired mutex");
			}
		}
		res->mutex.release();
		BB_log(BB_LOG_FINE, "olar_go : released mutex");
	}
	
    BB_log( BB_LOG_INFO, "you are now leaving the olar_go() function..., good luck." );

    res->thread_done = true;

    return 0;
}



Residue::Residue( const char * filepath, time_t myid ) 
	: Template( myid )
{
	// treesynth
	ts = new Treesynth;
	tsio = new TreesynthIO;
	strcpy( tsio->ifilename, filepath );
	ts->tree = new Tree;
	ts->tree->initialize( lg(CUTOFF) );
	ts->initialize();
	ts->resetTreeLevels( 13 );
	ts->stoplevel = 9;
	shutup = false;
	lefttree = false;
	total_levels = ts->tree->getLevels();
	requested_levels = total_levels;
	src_ts = new AudioSrcEliot( tsio );
	src_ts->m_delete = FALSE;
	
	// olar
	olar = new OlaRandom();
	olar->initialize( filepath, 2.0, 0.1, 0, false );
	olar->write_to_file = false;
	src_olar = new AudioSrcOlar( olar );
	src_olar->m_delete = FALSE;
	
	// both
	type = TT_RESIDUE;
	typestr = "background";
	thread = NULL;
	thread_done = true;
	strcpy(filename, filepath);
	m_method = TS;
	src = NULL;
}


Residue::~Residue()
{
    //fprintf( stderr, "residue, destructor...\n" );
    BB_log( BB_LOG_FINE, "Deleting residue template" ); 
    this->shutup = TRUE;
    usleep( 10000 );
    mutex.acquire();
    BB_log( BB_LOG_FINE, "Residue destructor : acquired mutex" );
    SAFE_DELETE( thread );
    mutex.release();
    BB_log( BB_LOG_FINE, "Residue destructor : released mutex" );
    SAFE_DELETE( ts );
    SAFE_DELETE( tsio );
    SAFE_DELETE( src_ts );
	SAFE_DELETE( olar );
	SAFE_DELETE( src_olar );
	//SAFE_DELETE( src );
    //fprintf( stderr, "residue, destructor more...\n" );
}


t_TAPBOOL Residue::stick( SAMPLE * buffer, t_TAPUINT num_frames )
{
//    assert( m_done == FALSE );

    // start the treesynth
    if( thread == NULL )
    {
        thread = new XThread;
		if(m_method == TS) {
			src = src_ts;
			thread->start( ts_go, this ); 
		}
		else if(m_method == OLAR) {
			src = src_olar;
			thread->start( olar_go, this );
		}
		else {
			BB_log(BB_LOG_WARNING, "Residue:stick: unknown method: %d", m_method);
			return FALSE;
		}
        usleep( 10000 );
        BB_log( BB_LOG_INFO, "residue, started new thread %x", thread );
    }
    if( shutup ) // was "else if" but that hung if the file was not found
    {
        //mutex.acquire();
        //delete thread;
		BB_log(BB_LOG_INFO, "shutting up residue");
        while( !thread_done )
            usleep( 10000 );
        SAFE_DELETE( thread );
        shutup = FALSE;
        //mutex.release();
        //thread = NULL;
        BB_log( BB_LOG_INFO, "residue has shut up" );
        return FALSE;
    }

    // are we done?
    if( m_stop_asap )
    {
        BB_log( BB_LOG_INFO, "residue, shutting up asap..." );
        return FALSE;
    }

    assert( src != NULL );
    src->set_gain( gain );
    src->set_pan( pan );
	// time
	m_time_elapsed += num_frames;
    return src->stick( buffer, num_frames );
}


// stop
void Residue::stop() {
	shutup = true; 
	Template::stop(); 
}


// change synthesis method
void Residue::change_method(int m) {
	if(m != TS && m != OLAR) {
		BB_log(BB_LOG_WARNING, "Residue:change_method: invalid method %d", m);
		return;
	}
	if(m != m_method) {
		// stop
		if(this->playing()) {
			BB_log(BB_LOG_INFO, "Residue:change_method: to method %d; stopping...", m);
			this->stop();
			while(!thread_done)
				usleep(10000);
			BB_log(BB_LOG_INFO, "Residue:change_method: stopped");
		}
		// set method
		m_method = m;
		if(m_method == TS)
			src = src_ts;
		else if(m_method == OLAR)
			src = src_olar;
	}
}


// copy from this into a new template
Template * Residue::copy( bool copyid ) const
{
    // Create Ross
    Residue * ross = new Residue( this->filename, copyid ? this->id : 0 );
    ross->copy_params( *this );
    
    // Return Ross
    return ross;
}

// copy from someone else into this
void Residue::copy_params( const Template & rhs )
{
    assert( ts != NULL );
    // call parents
    Template::copy_params( rhs );

	// Treesynth
    Residue *r = (Residue *)(&rhs);
    ts->percentage = r->ts->percentage;
    ts->kfactor = r->ts->kfactor;
    ts->stoplevel = r->ts->stoplevel;
    ts->startlevel = r->ts->startlevel;
    ts->randflip = r->ts->randflip;
    ts->ancfirst = r->ts->ancfirst;
	// tsio->ifilename should already be the same
	strcpy(tsio->ofilename, r->tsio->ofilename);
	tsio->rm_mode = r->tsio->rm_mode;
	tsio->write_to_buffer = r->tsio->write_to_buffer;
	tsio->write_to_file = r->tsio->write_to_file;
    requested_levels = r->requested_levels;
    ts->resetTreeLevels( requested_levels );
    total_levels = ts->tree->getLevels();
    lefttree = r->lefttree;
	
	// Olar
	olar->set_randomness(r->olar->get_randomness());
	olar->set_segsize_secs(r->olar->get_segsize_secs());
	olar->set_mindist_secs(r->olar->get_mindist_secs());
	olar->set_scaleamp(r->olar->get_scaleamp());
	olar->write_to_buffer = r->olar->write_to_buffer;
	olar->write_to_file = r->olar->write_to_file;
	strcpy(olar->ofilename, r->olar->ofilename);
}


void Residue::set_param( int which, double value )
{

    assert( ts != NULL );
	assert(olar != NULL);
    // call parents

    Template::set_param( which, value );

    mutex.acquire();
    BB_log( BB_LOG_FINE, "Residue set param : acquired mutex" );
    switch( which )
    {
    case PERCENTAGE:
        ts->percentage = value;
        break;
    case K:
        ts->kfactor = value;
        break;
    case STOPLEVEL: 
        ts->stoplevel = (int)(value + 0.5f);
        break;
    case STARTLEVEL:
        ts->startlevel = (int)(value + 0.5f);
        break;
    case TOTAL_LEVELS:
        requested_levels = (int)(value + 0.5f);
        break;
    case RANDFLIP:
        ts->randflip = value > 0.5;
        break;
    case ANCFIRST:
        ts->ancfirst = value > 0.5;
        break;
	case OLARANDOMNESS:
		olar->set_randomness(value);
		break;
	case SEGSIZE:
		olar->set_segsize_secs(value);
		break;
	case MINDIST:
		olar->set_mindist_secs(value);
		break;
	case SCALE_AMP:
		olar->set_scaleamp(value > 0.5);
		break;
    default:
        break;
    }
    mutex.release();
    BB_log( BB_LOG_FINE, "Residue set param : released mutex" );
}


LoopTemplate::~LoopTemplate()
{
    BB_log( BB_LOG_FINE, "Deleting loop template" ); 
    SAFE_DELETE( temp );
    SAFE_DELETE_ARRAY( acc_buffer );
    SAFE_DELETE_ARRAY( arg_buffer );
}

t_TAPBOOL LoopTemplate::stick( SAMPLE * buffer, t_TAPUINT num_frames )
{
//    assert( m_done == FALSE );

    // are we done? (not yet)
    if( m_stop_asap )
        return FALSE;

    if( acc_buffer_size < num_frames * 2 )
    {
        // delete
        SAFE_DELETE_ARRAY( acc_buffer );
        SAFE_DELETE_ARRAY( arg_buffer );
        // allocate
        acc_buffer = new SAMPLE[num_frames * 2];
        arg_buffer = new SAMPLE[num_frames * 2];
        if( !acc_buffer || !arg_buffer )
        {
            BB_log( BB_LOG_SYSTEM_ERROR, "[looptemplate]: cannot allocate buffer of size '%i'...",
                num_frames * 2 );
            SAFE_DELETE_ARRAY( acc_buffer );
            SAFE_DELETE_ARRAY( arg_buffer );
            return FALSE;
        }
        acc_buffer_size = num_frames * 2;
    }

    int i, j;
    t_TAPUINT remaining = num_frames;
    t_TAPUINT togen = (until_next <= num_frames ? until_next : num_frames);
    t_TAPUINT offset = 0;
    t_TAPBOOL yes;
        
    // expected/average interval between events (unit: fractional samples)
    double mean = 1.0 / density * BirdBrain::srate(); 

    // zero
    memset( acc_buffer, 0, sizeof(SAMPLE) * acc_buffer_size );

    // determine distribution
    /*if( periodicity > .999 )
        dist = Periodic::instance();
    else if( periodicity < .1 )
        dist = Poisson::instance();
    else {
        dist = FunkyRand::instance();
        ((FunkyRand *)dist)->periodicity = periodicity;  
    }*/

    /*if( rand() / (RAND_MAX + 1.0) < periodicity ) 
        dist = Periodic::instance();
    else
        dist = Poisson::instance();*/

    /*dist = PerryRand::instance();
    ((PerryRand *)dist)->periodicity = periodicity;*/
    
    dist = Gaussian::instance();
    
    while( remaining > 0 )
    {
        // need to figure out time until next event
        if( until_next == 0 )
        {
            // generate the time to wait
            // acquire mutex for shared instance (dist)
            dist->instmutex.acquire();
            // reset periodicity
            ((Gaussian *)dist)->periodicity = periodicity;
            // get waiting time
            //until_next = (t_TAPUINT)(dist->next_interval( mean ) + .5 );
            double unquantized = dist->next_interval( mean );
            // release mutex
            dist->instmutex.release();
            // quantize if needed
            until_next = (t_TAPUINT)(BirdBrain::srate() * quantize_time(unquantized / BirdBrain::srate()) + .5);
			//fprintf( stderr, "until_next: %i samples (%f seconds)\n", until_next, (until_next * 1.0)/ BirdBrain::srate() );
            // set the amount to generate
            // (could be less than until_next because of buffer boundary)
            togen = (until_next <= remaining ? until_next : remaining);
            // make new clone
            Template * new_one = temp->copy( true );
            // randomize
            randomize( new_one );
            // recompute
            new_one->recompute();
            // rewind
            new_one->rewind();
            // add it
            copies.push_back( new_one );
            // log
            BB_log( BB_LOG_FINER, "loop: until_next: %d togen: %d", until_next, togen );
            // event happens immediately
            if( togen == 0 )
                continue;
        }

        // get samples
        active.clear();
        for( i = 0; i < copies.size(); i++ )
        {
            // compute samples
            yes = copies[i]->stick( arg_buffer, togen );
            // check for copies that are done
            if( !yes )
            {
                delete copies[i];
            }
            else
            {
                // remember
                active.push_back( copies[i] );
                // accmulate
                for( j = 0; j < togen*2; j++ )
                    acc_buffer[offset*2 + j] += arg_buffer[j];
            }
        }
        // get active
        copies = active;
        // decrement remaining
        remaining -= togen;
        // decrement time until next
        until_next -= togen;
        // find offset
        offset += togen;
    }

    // copy
    memcpy( buffer, acc_buffer, sizeof(SAMPLE) * num_frames * 2 );

    // gain
    gain_buf( buffer, num_frames * 2, gain );
    // pan
    pan_buf( buffer, num_frames, pan );

	// time
	m_time_elapsed += num_frames;

    return TRUE;
}


// better distribution would be, well, better
void LoopTemplate::randomize( Template * victim )
{
    double r; 
    // freq warp
    r = rand() / (double)RAND_MAX; 
    victim->freq_warp = rand_freq_warp[0] + r * (rand_freq_warp[1] - rand_freq_warp[0]);
    victim->freq_warp = quantize_pitch( victim->freq_warp );

    // time stretch
    r = rand() / (double)RAND_MAX; 
    victim->time_stretch = rand_time_stretch[0] + r * (rand_time_stretch[1] - rand_time_stretch[0]);

    // gain
    r = rand() / (double)RAND_MAX; 
    victim->gain = rand_gain[0] + r * (rand_gain[1] - rand_gain[0]);

    // pan
    r = rand() / (double)RAND_MAX; 
    victim->pan = rand_pan[0] + r * (rand_pan[1] - rand_pan[0]);
}


Template * LoopTemplate::copy( bool copyid ) const
{
    LoopTemplate * lou = new LoopTemplate( *this->temp, copyid ? this->id : 0 );
    lou->copy_params( *this );

    return lou;
}


// copy from someone else into this
void LoopTemplate::copy_params( const Template & rhs )
{
    Template::copy_params( rhs );
    
    // loop randomization ranges
    if( rhs.type == TT_LOOP ) {
        LoopTemplate *lt = (LoopTemplate *)(&rhs);
        for( int i = 0; i <= 1; i++ ) {
            this->rand_freq_warp[i] = lt->rand_freq_warp[i];
            this->rand_time_stretch[i] = lt->rand_time_stretch[i];
            this->rand_gain[i] = lt->rand_gain[i];
            this->rand_pan[i] = lt->rand_pan[i];
        }
        this->random = lt->random;
    }
	// if it's a different type of template, add "loop" to its name
	else {
		this->name = rhs.name + "Loop"; 
	}
}


void LoopTemplate::set_param( int which, double value )
{
    Template::set_param( which, value );
    
    bool local_changed = true;

    switch( which )
    {
    case R_FREQ_WARP_LOW:               // not used
        rand_freq_warp[0] = value;
        break;

    case R_FREQ_WARP_HIGH:              // not used
        rand_freq_warp[1] = value;
        break;

    case R_TIME_STRETCH_LOW:            // not used
        rand_time_stretch[0] = value;
        break;

    case R_TIME_STRETCH_HIGH:           // not used
        rand_time_stretch[1] = value;
        break;

    case R_GAIN_LOW:                    // not used
        rand_gain[0] = value;
        break;

    case R_GAIN_HIGH:                   // not used
        rand_gain[1] = value;
        break;

    case R_PAN_LOW:                     // not used
        rand_pan[0] = value;
        break;

    case R_PAN_HIGH:                    // not used
        rand_pan[1] = value;
        break;

    case RANGE_MODE:                    // not used
        //random = value > 0.5;
        break;

    case PERIODICITY:
        periodicity = value;
        break;

    case DENSITY:       
        density = 1 / quantize_time( 1 / value );
        break;

    case RANDOM:
        random = value;
        break;

    default:
        local_changed = true;
        break;
    }

    if( local_changed )
        recompute();
}


void LoopTemplate::recompute( )
{
    // check if polluted
    if( !polluted )
        return; 
    
    // set randomness ranges
    if( random >= 1 )
    {
        rand_freq_warp[0] = freq_warp / random;  rand_freq_warp[1] = freq_warp * random;
        rand_time_stretch[0] = time_stretch / random;  rand_time_stretch[1] = time_stretch * random;
        rand_gain[0] = gain / random;  rand_gain[1] = gain * random;
        rand_pan[0] = pan / random;  rand_pan[1] = pan * random;
    }
    else 
    {
        rand_freq_warp[0] = rand_freq_warp[1] = freq_warp;
        rand_time_stretch[0] = rand_time_stretch[1] = time_stretch;
        rand_gain[0] = rand_gain[1] = gain;
        rand_pan[0] = rand_pan[1] = pan;
    }

    // up call
    Template::recompute();
}


// Timeline
Timeline::Timeline( t_TAPTIME dur, time_t myid )
    : Template( myid )
{
    loop = 0; // no loop by default
    duration = dur;
    dur_pos = 0;
    until_next = 0;
    type = TT_TIMELINE;
    typestr = "timeline";
    curr_index = 0;
    starttime = 0; 
    stoptime = dur;

    acc_buffer = NULL;
    arg_buffer = NULL; 
    acc_buffer_size = 0;

    now_butter = NULL;
}


Timeline::~Timeline()
{
    BB_log( BB_LOG_FINE, "Deleting timeline" ); 
    SAFE_DELETE_ARRAY( acc_buffer );
    SAFE_DELETE_ARRAY( arg_buffer );
    // delete templates still generating
    for( int i = 0; i < copies.size(); i++ )
    {
        SAFE_DELETE( copies[i] );
    }
}

void Timeline::place( UI_Template * ui_temp, float where, float y_offset = 0.0f )
{
    t_TAPTIME start = (t_TAPTIME)( where * duration );
    start = (t_TAPTIME)( BirdBrain::srate() * quantize_time( start / BirdBrain::srate() ) ); 
    instances.push_back( Instance( ui_temp, start, 0, y_offset ) );

    // keep the vector sorted
    int i = 0;
    for( i = instances.size()-2; i >= 0; i-- )
    {
        if( instances[i].start_time > start )
        {
            Instance temp = instances[i];
            instances[i] = instances[i+1];
            instances[i+1] = temp;
        }
        else
            break;
    }

    // update until_next if needed (ie if newly inserted item is the next one to play)? 
    // (but dangerous without locks)
    /*if( i == curr_index )
    {
        until_next = instances[i].start_time - dur_pos;
    }*/
}


// if this is used, we should REALLY remove ui_temp from the library too (we do though) 
void Timeline::remove( UI_Template * ui_temp )
{
    for( int i = 0; i < instances.size(); i++ )
        if( instances[i].ui_temp == ui_temp )
        {
            instances.erase( instances.begin() + i );
            break;
        }
}


// this poor function almost got deleted accidentally.
// IT SHOULD BE HARDER TO DELETE CODE!
t_TAPBOOL Timeline::rewind()
{ 
//	fprintf( stderr, "pre rewind: %s playing\n", this->playing() ? "" : "not" );
	if( !AudioSrc::rewind() )
		return FALSE;
    dur_pos = 0; /*dur_pos = starttime;*/ 
    curr_index = 0;
	if( !instances.empty() )
		until_next = instances[0].start_time; 
/*    for( int j = 0; j < instances.size(); j++ )
    {
        if( instances[j].start_time >= this->starttime )
        {
            curr_index = j;
            until_next = instances[j].start_time - this->starttime;
            break;
        }
    } 
*/    for( int i = 0; i < copies.size(); i++ )
    {
        SAFE_DELETE(copies[i]);
    }
    copies.clear();
//	fprintf( stderr, "post rewind: %s playing\n", this->playing() ? "" : "not" );
	return forward((t_TAPUINT)starttime);
}

void wait_for( Template * me )
{
    int count = 0;
    while( !me->m_done )
    {
        if( (count++ % 10) == 0 ) { fprintf( stderr, "." ); }
        if( count > 100000 ) // half hour or so
        {
            BB_log( BB_LOG_SYSTEM_ERROR, "\nFATAL ERROR!!!" );
            BB_log( BB_LOG_SYSTEM_ERROR, "STILL WAITING FOR COPIES TO STOP PLAYING!!!" );
            BB_log( BB_LOG_SYSTEM_ERROR, "GIVING UP AND DESTROYING SELF. BYE." );
            *(int *)0 = 0; // definite danger: machine.crash();
        }
        usleep( 20000 );
    }
}

void Timeline::stop() 
{ 
    // stop me!
    Template::stop(); 
    // wait for me!
    wait_for( this );
    // stop the copies
    stop_copies();
    // rewind
    rewind(); 
}


void Timeline::stop_copies()
{
    // stop the copies
    for( int i = 0; i < copies.size(); i++ )
        copies[i]->stop();
    // usleep( 20000 );

    int index = 0;
    BB_log( BB_LOG_FINE, "timeline waiting for contents to stop playing" );
    while( index < copies.size() )
    {
        // wait for the copy
        wait_for( copies[index] );
        index++;
    }
}

    
t_TAPBOOL Timeline::stick( SAMPLE * buffer, t_TAPUINT num_frames )
{
//    assert( m_done == FALSE );
    // are we done? (not yet)
    if( m_stop_asap ) // used to be "|| dur_pos >= duration - 1" and no stop_copies
        return FALSE;
    if( dur_pos >= stoptime - 1 ) // used to be duration - 1
    {
        stop_copies();
        return FALSE;
    }

    // warm up...
    if( acc_buffer_size < num_frames * 2 )
    {
        // delete
        SAFE_DELETE_ARRAY( acc_buffer );
        SAFE_DELETE_ARRAY( arg_buffer );
        // allocate
        acc_buffer = new SAMPLE[num_frames * 2];
        arg_buffer = new SAMPLE[num_frames * 2];
        if( !acc_buffer || !arg_buffer )
        {
            BB_log( BB_LOG_SYSTEM_ERROR, "[timeline]: cannot allocate buffer of size '%i'...",
                num_frames * 2 );
            SAFE_DELETE_ARRAY( acc_buffer );
            SAFE_DELETE_ARRAY( arg_buffer );
            return FALSE;
        }
        acc_buffer_size = num_frames * 2;
    }

    int i, j;
    // number of samples until firing the next event - TODO: check bounds
    t_TAPUINT remaining = (t_TAPUINT)x_min( (t_TAPTIME)num_frames, (duration - dur_pos) );
    t_TAPUINT togen = (t_TAPUINT)x_min( until_next, remaining );
    t_TAPUINT offset = 0;//num_frames - remaining;
    t_TAPBOOL yes;
    Template * temp = NULL;

    // zero
    memset( acc_buffer, 0, sizeof(SAMPLE) * acc_buffer_size );

    while( remaining > 0 )
    {
        // need to figure out time until next event
        while( (t_TAPUINT)until_next == 0 )
        {
            // get next template
            if( curr_index < instances.size() ) 
            {
                temp = instances[curr_index].ui_temp->core;
                // make new clone
                Template * new_one = temp->copy( true );
                // quantize clone's pitch according to timeline's pitch table
                // new_one->freq_warp = quantize_pitch( new_one->freq_warp );
                // alternative: set freq_warp directly from table
                if( !pitchtable.empty() )
                    new_one->freq_warp = pitchtable[curr_index % pitchtable.size()]; 
                // recompute
                new_one->recompute();
                // rewind
                new_one->rewind();
                // add it
                copies.push_back( new_one );
                // advance to next event
                curr_index++;
            }

            // see if there are more templates to wait for
            if( curr_index < instances.size() ) 
            {
                until_next = instances[curr_index].start_time - dur_pos;
            }
            else
            {
                temp = NULL;
                until_next = 0xffffffff;
            }

            // set the amount to generate
            // (could be less than until_next because of buffer boundary)
            togen = (t_TAPUINT)(until_next <= remaining ? until_next : remaining);
        }

        // get samples
        active.clear();
        for( i = 0; i < copies.size(); i++ )
        {
            // clear arg_buffer
            // compute samples
            yes = copies[i]->stick( arg_buffer, togen );
            // check for copies that are done
            if( !yes )
            {
                delete copies[i];
            }
            else
            {
                // remember
                active.push_back( copies[i] );
                // accmulate
                for( j = 0; j < togen*2 && offset*2 + j < acc_buffer_size; j++ )
                    acc_buffer[offset*2 + j] += arg_buffer[j];
            }
        }
        // get active
        copies = active;
        // decrement remaining
        remaining -= togen;
        // decrement time until next
        until_next -= togen;
        // advance pos
        dur_pos += togen;
        // find offset
        offset += togen;//num_frames - remaining;
    }

    // copy
    memcpy( buffer, acc_buffer, sizeof(SAMPLE) * num_frames * 2 );

    // gain
    gain_buf( buffer, num_frames * 2, gain ); // why are we using these instead of src->set_gain / src->set_pan?
    // pan
    pan_buf( buffer, num_frames, pan );       // (because we're not stupid) (there is no src) 

    // now, butter!
    if( now_butter != NULL )
    {
        now_butter->slide = ((float)dur_pos - BirdBrain::srate() * now_butter->slide_0) 
                            / (BirdBrain::srate() * (now_butter->slide_1 - now_butter->slide_0));
        now_butter->slide_locally = false; // or true, doesn't matter.
    }

	// time
	m_time_elapsed += num_frames;

    return TRUE;
}

Template * Timeline::copy( bool copyid ) const
{
    Timeline * tim = new Timeline( this->duration, copyid ? this->id : 0 );
    tim->copy_params( *this );
    tim->rewind();

    return tim;
}

void Timeline::copy_params( const Template & rhs )
{
    Template::copy_params( rhs );
    
    const Timeline * tim = (Timeline *)&rhs;
    this->instances = tim->instances; 
    this->duration = tim->duration;
    this->starttime = tim->starttime;
    this->stoptime = tim->stoptime;
}



// Marbles
Marble::Marble( UI_Template * u, t_TAPUINT weight, bool norepeat, double randfac )
{
    ui_temp = u;
    likelihood = weight;
    hasplayed = false;
    playonce = norepeat;
    changeable = true;
    // override 'norepeat' and 'changeable' for ongoing templates (and possibly timelines)
    if( u->core->type == TT_RESIDUE || u->core->type == TT_LOOP || u->core->type == TT_BAG )
    {
        playonce = true;
        changeable = false;
    }
    random = randfac;
    set_ranges( u->core );  
}

Marble::Marble( UI_Template * u )
{
    ui_temp = u;
    likelihood = 1;
    hasplayed = false;
    if( u->core->type == TT_RESIDUE || u->core->type == TT_LOOP || u->core->type == TT_BAG )
    {
        playonce = true;
        changeable = false;
    }
    else
    {
        playonce = false;
        changeable = true;
    }
    random = 2.0; 
    set_ranges( u->core ); 
}

void Marble::set_ranges( const Template * victim )
{
    if( !victim ) {
        BB_log( BB_LOG_SYSTEM, "Marble needs a victim to initialize" ); 
        return;
    }
    if( random == 0 )
    {
        rand_freq_warp[0] = rand_freq_warp[1] = victim->freq_warp; 
        rand_time_stretch[0] = rand_time_stretch[1] = victim->time_stretch;
        rand_gain[0] = rand_gain[1] = victim->gain;
        rand_pan[0] = rand_pan[1] = victim->pan; 
    }
    else 
    {
        rand_freq_warp[0] = victim->freq_warp/random; 
        rand_freq_warp[1] = victim->freq_warp*random;
        rand_time_stretch[0] = victim->time_stretch/random; 
        rand_time_stretch[1] = victim->time_stretch*random;
        rand_pan[0] = victim->pan/random; rand_pan[1] = victim->pan*random;
        rand_gain[0] = victim->gain/random; rand_gain[1] = victim->gain*random;
    }
}

void Marble::randomize( Template * victim )
{
    if( !victim ) {
        BB_log( BB_LOG_SYSTEM, "Marble needs a victim to randomize" ); 
        return;
    }
    double r; 
    // freq warp
    r = rand() / (double)RAND_MAX; 
    victim->freq_warp = rand_freq_warp[0] + r * (rand_freq_warp[1] - rand_freq_warp[0]);

    // time stretch
    r = rand() / (double)RAND_MAX; 
    victim->time_stretch = rand_time_stretch[0] + r * (rand_time_stretch[1] - rand_time_stretch[0]);

    // gain
    r = rand() / (double)RAND_MAX; 
    victim->gain = rand_gain[0] + r * (rand_gain[1] - rand_gain[0]);

    // pan
    r = rand() / (double)RAND_MAX; 
    victim->pan = rand_pan[0] + r * (rand_pan[1] - rand_pan[0]);
}


// Bag/Box/Teapot/Other container
int BagTemplate::nctrls = 3;

BagTemplate::BagTemplate( time_t myid ) 
    : Template( myid )
{
    until_next = 0;
    type = TT_BAG;
    typestr = "bag";
    
    acc_buffer = NULL;
    arg_buffer = NULL; 
    acc_buffer_size = 0;

    weightsum = 0; 
}


BagTemplate::~BagTemplate()
{
    BB_log( BB_LOG_FINE, "Deleting bag template" );
    SAFE_DELETE_ARRAY( acc_buffer );
    SAFE_DELETE_ARRAY( arg_buffer );
    // delete templates still generating
    for( int i = 0; i < copies.size(); i++ )
    {
        SAFE_DELETE( copies[i] );
    }
}


Marble * BagTemplate::insert( UI_Template * ui_temp )
{
    if( ui_temp == NULL || ui_temp->core == NULL )
    {
        BB_log( BB_LOG_SYSTEM, "NULL input to BagTemplate::insert" ); 
        return NULL;
    }
    if( ui_temp->core->type == TT_BAG )
    {
        BB_log( BB_LOG_INFO, "Sorry, bags in bags not currently allowed" );
        return NULL;
    }
    marbles.push_back( Marble( ui_temp ) );
    Marble mab = marbles.back();
    if( !mab.playonce )
        weightsum += mab.likelihood;
    return &marbles.back();
}


void BagTemplate::remove( UI_Template * ui_temp )
{
    for( int i = 0; i < marbles.size(); i++ )
        if( marbles[i].ui_temp == ui_temp )
        {
            marblemutex.acquire(); 
            if( !marbles[i].playonce )
                weightsum -= marbles[i].likelihood; 
            // if it's a continous or hard-to-stop template, make its copy stop playing
            if( ui_temp->core->type == TT_RESIDUE || ui_temp->core->type == TT_LOOP || 
                ui_temp->core->type == TT_BAG || ui_temp->core->type == TT_SCRIPT )
            {
                for( int c = 0; c < copies.size(); c++ )
                {
                    if( copies[c]->id == ui_temp->core->id )
                    {
                        copies[c]->stop(); 
                        wait_for( copies[c] ); 
                    }
                }
            }
            marbles.erase( marbles.begin() + i );
            marblemutex.release(); 
            break;
        }
}


t_TAPBOOL BagTemplate::stick( SAMPLE * buffer, t_TAPUINT num_frames )
{
    // are we done? (not yet)
    if( m_stop_asap )
        return FALSE;
    
    if( acc_buffer_size < num_frames * 2 )
    {
        // delete
        SAFE_DELETE_ARRAY( acc_buffer );
        SAFE_DELETE_ARRAY( arg_buffer );
        // allocate
        acc_buffer = new SAMPLE[num_frames * 2];
        arg_buffer = new SAMPLE[num_frames * 2];
        if( !acc_buffer || !arg_buffer )
        {
            BB_log( BB_LOG_SYSTEM_ERROR, "[bagtemplate]: cannot allocate buffer of size '%i'...",
                num_frames * 2 );
            SAFE_DELETE_ARRAY( acc_buffer );
            SAFE_DELETE_ARRAY( arg_buffer );
            return FALSE;
        }
        acc_buffer_size = num_frames * 2;
    }

    int i, j;
    t_TAPUINT remaining = num_frames;
    t_TAPUINT togen = (t_TAPUINT)x_min( until_next, remaining );
    t_TAPUINT offset = 0;
    t_TAPBOOL yes;
    Template * temp = NULL; 

    // expected/average interval between events (unit: fractional samples)
    double mean = 1.0 / density * BirdBrain::srate(); 

    // zero
    memset( acc_buffer, 0, sizeof(SAMPLE) * acc_buffer_size );

    // determine distribution   
    dist = Gaussian::instance();
    
    while( remaining > 0 )
    {
        // need to figure out time until next event
        if( (t_TAPUINT)until_next == 0 )
        {
            // generate the time to wait
            // acquire mutex for shared instance (dist)
            dist->instmutex.acquire();
            // reset periodicity
            ((Gaussian *)dist)->periodicity = periodicity;
            // get waiting time
            double unquantized = dist->next_interval( mean );
            // release mutex
            dist->instmutex.release();
            // quantize if needed
            until_next = (t_TAPTIME)(BirdBrain::srate() * quantize_time(unquantized / BirdBrain::srate()));
            // set the amount to generate
            // (could be less than until_next because of buffer boundary)
            togen = (t_TAPUINT)(until_next <= remaining ? until_next : remaining);
            // pick template
            int pick = (int)(rand()/(RAND_MAX+1.0) * weightsum);
            int sum = 0, m; 
            marblemutex.acquire();
            for( m = 0; m < marbles.size(); m++ )
            {
                // one-time template that has been played or is playing already
                if( marbles[m].playonce && marbles[m].hasplayed )
                    continue;
                // repeating template
                if( !marbles[m].playonce )
                    sum += marbles[m].likelihood;
                // one-time template that has not been played yet
                // or selected repeating template
                if( (marbles[m].playonce && !marbles[m].hasplayed) || sum > pick ) 
                {
                    BB_log( BB_LOG_FINEST, "Marble %i ui_temp 0x%x temp 0x%x", m, marbles[m].ui_temp, marbles[m].ui_temp->core );
                    // get what it picked
                    temp = marbles[m].ui_temp->core;
                    // make new clone
                    Template * new_one = temp->copy( true ); 
                    // randomize
                    if( !marbles[m].playonce ) {
                        marbles[m].set_ranges( new_one ); 
                        marbles[m].randomize( new_one );
                    }
                    // quantize template's pitch according to bag's pitch table
                    new_one->freq_warp = quantize_pitch( new_one->freq_warp );
                    // recompute
                    new_one->recompute(); 
                    // add it
                    copies.push_back( new_one ); 
                    // mark the marble as having been played at least once
                    marbles[m].hasplayed = true;
                    // if it was a selected repeating template
                    // make sure no other repeating ones are selected this time
                    if( sum > pick )
                        sum = -weightsum;
                }
            }               
            marblemutex.release();
            // event happens immediately (what??)
            if( togen == 0 )
                continue;
        }

        // get samples
        active.clear();
        for( i = 0; i < copies.size(); i++ )
        {
            // compute samples
            yes = copies[i]->stick( arg_buffer, togen );
            // check for copies that are done
            if( !yes )
            {
                delete copies[i];
            }
            else
            {
                // remember
                active.push_back( copies[i] );
                // accmulate
                for( j = 0; j < togen*2 && offset*2 + j < acc_buffer_size; j++ )
                    acc_buffer[offset*2 + j] += arg_buffer[j];
            }
        }
        // get active
        copies = active;
        // decrement remaining
        remaining -= togen;
        // decrement time until next
        until_next -= togen;
        // find offset
        offset += togen;
    }

    // copy
    memcpy( buffer, acc_buffer, sizeof(SAMPLE) * num_frames * 2 );

    // gain
    gain_buf( buffer, num_frames * 2, gain );
    // pan
    pan_buf( buffer, num_frames, pan );
    
	// time
	m_time_elapsed += num_frames;

    // return 
    return TRUE;
}


t_TAPBOOL BagTemplate::rewind()
{
    if( !AudioSrc::rewind() )
		return FALSE;
    for( int i = 0; i < copies.size(); i++ )
    {
        SAFE_DELETE(copies[i]);
    }
    copies.clear();
    until_next = 0; 
	return TRUE;
}


void BagTemplate::stop()
{
    // stop me!
    Template::stop(); 
    // wait for me!
    wait_for( this );
    // stop the copies
    for( int i = 0; i < copies.size(); i++ )
        copies[i]->stop();
    // usleep( 20000 );
    int index = 0;
    BB_log( BB_LOG_FINE, "bagtemplate waiting for contents to stop playing" );
    while( index < copies.size() )
    {
        // wait for the copy
        wait_for( copies[index] );
        index++;
    }
    BB_pushlog();
    BB_log( BB_LOG_INFO, "done" );
    BB_poplog(); 
    rewind(); 
}


Template * BagTemplate::copy( bool copyid ) const
{
    BagTemplate * bach = new BagTemplate( copyid ? this->id : 0 );
    bach->copy_params( *this );
    bach->rewind();

    return bach;
}


void BagTemplate::copy_params( const Template & rhs )
{
    Template::copy_params( rhs );
    
    const BagTemplate * bach = (BagTemplate *)&rhs;
    this->marbles = bach->marbles;
    this->weightsum = bach->weightsum; 
}


void BagTemplate::set_param( int which, double value )
{
    Template::set_param( which, value ); 
    
    // basic params that template::set_param didn't handle for personal reasons
    switch( which )
    {
    case PERIODICITY:
        periodicity = value;
        break;
    case DENSITY:
        density = 1 / quantize_time( 1 / value );
        break;
    default: 
        break;
    }

    // marble params
    if( which < BAG_OFFSET || which >= BAG_OFFSET + nctrls * marbles.size() )
        return;

    int marble = (which - BAG_OFFSET) / nctrls;
    int param = (which - BAG_OFFSET) % nctrls;

    if( !marbles[marble].changeable )
        return; 

    switch( param )
    {
    case 0: 
    {
        // change whether this is played once or repeats
        bool old = marbles[marble].playonce; 
        bool curr = value > 0.5; 
        if( !old && curr )
            weightsum -= marbles[marble].likelihood; 
        else if( old && !curr )
            weightsum += marbles[marble].likelihood; 
        marbles[marble].playonce = curr; 
        break;
    }
    case 1:
        // change the likelihood of playing this marble
        if( !marbles[marble].playonce )
            weightsum -= marbles[marble].likelihood;
        marbles[marble].likelihood = (t_TAPUINT)value;
        if( !marbles[marble].playonce )
            weightsum += marbles[marble].likelihood; 
        break;
    case 2:
        // change randomness / ranges
        marbles[marble].random = value;
        marbles[marble].set_ranges( marbles[marble].ui_temp->core ); 
        break;
    default:
        break;
    }
}


void BagTemplate::recompute()
{
    if( !polluted )
        return; 

    weightsum = 0;
    for( int m = 0; m < marbles.size(); m++ )
    {
        if( !marbles[m].playonce )
            weightsum += marbles[m].likelihood; 
        marbles[m].hasplayed = false;
    }

    Template::recompute();
}



//-----------------------------------------------------------------------------
// junk
//-----------------------------------------------------------------------------
/*
MultiEvent::~MultiEvent()
{
    SAFE_DELETE_ARRAY( acc_buffer );
    SAFE_DELETE_ARRAY( arg_buffer );
    // delete templates still generating
    for( int i = 0; i < copies.size(); i++ )
    {
        SAFE_DELETE( copies[i] );
    }
}
*/

//-----------------------------------------------------------------------------
// id mismanagement
//-----------------------------------------------------------------------------

/*std::vector<t_TAPUINT> new_ids;    // mapping of current even ids to new odd ids in case
                            // they get resaved, so that the above solution works
                            // even when they are reloaded
t_TAPUINT get_id( t_TAPUINT orig_id )
{
    // figure out new id from mapping
    t_TAPUINT myid = 0;
    if( orig_id % 2 )
        myid = orig_id; 
    else
    {
        if( orig_id / 2 < new_ids.size() )
            myid = new_ids[orig_id / 2];
        else
            new_ids.resize( orig_id, 0 );
        
        if( myid == 0 )
        {
            myid = next_id; 
            next_id += 2;
            new_ids[orig_id / 2] = myid; 
        }
    }
    return myid; 
}*/

// template grabber
TemplateGrabber::TemplateGrabber( long hint_num_samples )
{
    m_hint_num_samples = hint_num_samples;
}


// grab
t_TAPBOOL TemplateGrabber::grab( Template * temp, Frame * out )
{
    long max_num_samples = 0;

    // figure out which kind of template
    switch( temp->type )
    {
    case TT_DETERMINISTIC:
    case TT_TRANSIENT:
    case TT_FILE:
    case TT_RAW:
        max_num_samples = -1;
    break;

    case TT_RESIDUE:
    case TT_LOOP:
    case TT_TIMELINE:
    case TT_BAG:
    case TT_SCRIPT:
        max_num_samples = m_hint_num_samples;
    break;

    default:
        // shouldn't get here
        assert( FALSE );
    break;
    }

    return grab( temp, out, max_num_samples );
}


// grab
t_TAPBOOL TemplateGrabber::grab( Template * temp, Frame * out, long max_num_samples )
{
    // vector of frames
    std::vector<Frame *> frames;
    // frame
    Frame * it = NULL;
    // mono frame
    Frame mono;
    // frame size
    long size = 2048;
    // number of channels
    long num_channels = 2;
    // flag
    t_TAPBOOL still_going = true;
    // end pointer (for safety)
    SAMPLE * end = NULL;
    // write pointer
    SAMPLE * where = NULL;

    // log
    if( max_num_samples < 0 )
        BB_log( BB_LOG_INFO, "grabbing entire template '%s'...", temp->name.c_str() );
    else
        BB_log( BB_LOG_INFO, "grabbing template '%s' for %.3f seconds...", temp->name.c_str(), (t_TAPTIME)max_num_samples / BirdBrain::srate() );

    // max
    if( max_num_samples < 0 ) max_num_samples = LONG_MAX;

    // stop, if currently playing
    if( temp->playing() )
    {
        temp->stop();
        while( temp->playing() )
            usleep( 5000 );
    }

    // rewind the template
    temp->reset();
    temp->rewind();
    temp->recompute();

    // get audio
    while( still_going && max_num_samples > 0 )
    {
        // make new frame
        it = new Frame;
        // alloc it
        it->alloc_waveform( size );
        // get the audio
        still_going = temp->stick( it->waveform, size / num_channels );
        // decrement max_num_samples
        if( max_num_samples >= (size / num_channels) ) max_num_samples -= size / num_channels;
        else max_num_samples = 0;
        // append
        frames.push_back( it );
    }

    // stop
    temp->m_done = TRUE; // usually audio engine does this but we are hijacking. 
    temp->stop(); // magic. (?) for scripts to stop, maybe

    // clear out
    out->clear();
    // alloc
    out->alloc_waveform( frames.size() * size / num_channels );
    out->wsize = out->wlen;
    // mono frame
    mono.alloc_waveform( size / num_channels );
    
    // get it out
    where = out->waveform;
    // end
    end = out->waveform + out->wlen;

    // get frames
    for( long i = 0; i < frames.size(); i++ )
    {
        // the frame
        it = frames[i];
        // convert to mono
        AudioSrc::stereo2mono( it->waveform, it->wlen / num_channels, mono.waveform );
        // copy to out
        memcpy( where, mono.waveform, size / num_channels * sizeof(SAMPLE) );
        // free it
        delete it;
        // move where
        where += size / num_channels;
    }

    // verify
    assert( where == end );

    return TRUE;
}
