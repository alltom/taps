
//-----------------------------------------------------------------------------
// name: ui_library.cpp
// desc: birdbrain ui library
//
// authors: Ananya Misra (amisra@cs.princeton.edu)
//          Ge Wang (gewang@cs.princeton.edu)
//          Perry R. Cook (prc@cs.princeton.edu)
//          Philip Davidson (philipd@cs.princeton.edu)
// after EST. January 14, 2005, 8:37 p.m. Friday
//-----------------------------------------------------------------------------
#include "ui_library.h"
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
    // rewind
    this->rewind();
    // play
    bus->play( this, FALSE );
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
	
	features = NULL;

    if( myid == 0 )
        id = time(0); 
    else
        id = myid; 

    // log
    // fprintf( stderr, "new template with id: %i ; next_id = %i\n", id, next_id );
    BB_log( BB_LOG_FINE, "creating new template with id: %d...", id );
}

void Template::copy_params( const Template & rhs )
{
    // basic params
    this->name = rhs.name;
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
            timetable.push_back( (TIME)data );
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
        BirdBrain::our_syn_wnd_size = (uint)(BirdBrain::wnd_size() * time_stretch + 0.5);
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


t_CKBOOL Deterministic::stick( SAMPLE * buffer, t_CKUINT num_frames )
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
    // phew
    return cached_sound.wsize > 0;//*/
}


Template * Deterministic::copy( bool copyid ) const
{
    Deterministic * deb = new Deterministic( this->tracks, copyid ? this->id : 0 );
    deb->copy_params( *this );

    return deb;
}


Raw::Raw( const Frame & event, time_t myid ) 
    : Template( myid ), pvc_window_size(1024), pvc_bufsize(512)
{ 
    sound = event; 
    src = new AudioSrcFrame( &sound ); 
    src->m_delete = FALSE;
    type = TT_RAW; 
    typestr = "raw";
   
    // pvc stuff
    pvc_hopsize = pvc_window_size / 8;
    set_pvc_buffers();
    m_pvc = new PVC( pvc_window_size, pvc_bufsize, 2048 /* 0/2048/pool_size */ ); // not bad with no pool (size 0)
    //m_pvc = new PVCtoo( event.waveform, event.wsize, pvc_hopsize, pvc_window_size, pvc_bufsize, 2048 );
}


// copy constructor specifically for use with alternative pvc implementation (in progress (regress))
Raw::Raw( const Raw &rhs ) : Template( rhs.id ) 
{
    // basic stuff
    sound = rhs.sound; 
    src = new AudioSrcFrame( &sound );
    src->m_delete = FALSE;
    type = TT_RAW;
    typestr = "raw"; 

    // pvc stuff
    pvc_window_size = rhs.pvc_window_size; 
    pvc_bufsize = rhs.pvc_bufsize;
    pvc_hopsize = pvc_window_size / 8; 
    set_pvc_buffers(); 
    m_pvc = rhs.m_pvc->pv_copy(); 
    //m_pvc = (PVCtoo *)(rhs.m_pvc)->pv_copy(); 

    // remaining parameters
    this->copy_params( rhs ); 
}


Raw::~Raw()
{
    BB_log( BB_LOG_FINE, "Deleting raw template" ); 
    SAFE_DELETE( src );

    // pvc stuff
    SAFE_DELETE_ARRAY( pvc_buffer ); 
    SAFE_DELETE_ARRAY( pvc_extras ); 
    SAFE_DELETE( m_pvc ); 
    SAFE_DELETE_ARRAY( silence );
}


// initialize pvc buffers (and related variables) after their sizes have been defined
void Raw::set_pvc_buffers()
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


/* // good
t_CKBOOL Raw::stick( SAMPLE * buffer, t_CKUINT num_frames )
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

// probably not good
/*t_CKBOOL Raw::stick( SAMPLE * buffer, t_CKUINT num_frames )
{
//    assert( m_done == FALSE );

    // are we done?
    if( m_stop_asap )
        return FALSE;

    assert( src != NULL );

    int done = 0;
    int toget = 0; 

    // unused samples computed from previous stick
    if( pvc_numextras > 0 )
    {
        int tocopy = (pvc_numextras < num_frames) ? pvc_numextras : num_frames; 
        for( int p = 0; p < tocopy; p++ )
        {
            buffer[p] = pvc_extras[p]; 
            pvc_extras[p] = (p + tocopy < pvc_bufsize) ? pvc_extras[p + tocopy] : 0; 
            pvc_extras[p + tocopy] = 0; 
            pvc_numextras--;
            done++;
        }
    }

    while( done < num_frames )
    {
        // read one chunk
        memset( pvc_buffer, 0, pvc_bufsize * sizeof(SAMPLE) ); 
        toget = (num_frames - done < pvc_bufsize) ? (num_frames - done) : pvc_bufsize; 
        if( !framedone && !src->mtick( pvc_buffer, pvc_bufsize) )
        {   
            // it may finish reading before it finishes synthesizing
            //                           eg if time_stretch is > 1
            framedone = true; 
        }

        // gulp
        if( time_stretch != 1 || freq_warp != 1 )
        {
            if( !framedone )
            {
                // analyze buffer
                pv_analyze( m_pvc, pvc_buffer, pvc_hopsize );
                // get phase windows out of queue
                while( pvc_win[pvc_which] = pv_deque( m_pvc ) )
                {
                    // unwrap phase
                    pv_unwrap_phase( pvc_win[pvc_which] );
                    // fix phase using last window and expected hop_size
                    if( pvc_win[!pvc_which] )
                        pv_phase_fix( pvc_win[!pvc_which], pvc_win[pvc_which], time_stretch );
                        // freq shift
                    pv_freq_shift( m_pvc, pvc_win[pvc_which], freq_warp );
                    // ifft
                    pv_unwrap_phase( pvc_win[pvc_which] );
                    // overlap/add
                    pv_overlap_add( m_pvc, pvc_win[pvc_which], (int)(pvc_hopsize * time_stretch + .5f) );
                    pvc_which = !pvc_which;
                    // reclaim the window
                    pv_reclaim( m_pvc, pvc_win[pvc_which] );
                }
            }
            // get buffer
            fprintf( stderr, "Delay: %i buffers\n", pv_get_ready_len( m_pvc ) ); 
            if( pv_get_ready_len( m_pvc ) == 0 && framedone )
            {
                done = 0;
                framedone = false;
                break;
            }
            pv_synthesize( m_pvc, pvc_buffer );
        }

        // phew (spoke too soon)
        if( pv_get_ready_len( m_pvc ) != 0 )
        {
            memcpy( buffer + done, pvc_buffer, toget * sizeof(SAMPLE) );  
            memcpy( pvc_extras + pvc_numextras, pvc_buffer + toget, (pvc_bufsize - toget) * sizeof(SAMPLE) ); 
            pvc_numextras += (pvc_bufsize - toget); 
            done += toget;
        }
    }

    mono2stereo( buffer, num_frames ); 
    gain_buf( buffer, num_frames * 2, gain ); 
    pan_buf( buffer, num_frames, pan ); 
    
    pvc_win[0] = pvc_win[1] = NULL; 
    return done; 
}*/

// probably not good
t_CKBOOL Raw::stick( SAMPLE * buffer, t_CKUINT num_frames )
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
    //vocode = false;
    if( !vocode )
    {
        src->set_gain( gain * 1.0 / 8 );
        src->set_pan( pan );
        return src->stick( buffer, num_frames );
    }

    // if you must use phase vocoder:

    // unused samples computed from previous stick
    if( pvc_numextras > 0 )
    {
        int tocopy = (pvc_numextras < num_frames) ? pvc_numextras : num_frames; 
        for( int p = 0; p < tocopy; p++ )
        {
            buffer[p] = pvc_extras[p]; 
            if( p + tocopy < pvc_bufsize )
            {
                pvc_extras[p] = pvc_extras[p + tocopy]; 
                pvc_extras[p + tocopy] = 0;
            }
            else
                pvc_extras[p] = 0;
            pvc_numextras--;
            done++;
        }
    }

    // need to add some silence at the end for complete overlap adding maybe? (clean up later)
    if( !framedone )
    {
        // analyze all
        while( src->mtick( pvc_buffer, pvc_bufsize ) )
        {
            m_pvc->pv_analyze( pvc_buffer, pvc_hopsize ); 
        }
        int h = (int)(pvc_hopsize * time_stretch + .5f);
        for( int p = h; p < pvc_bufsize; p += h )
            m_pvc->pv_analyze( silence, pvc_hopsize ); 
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
            m_pvc->pv_phase_fix_( pvc_win[!pvc_which], pvc_win[pvc_which], time_stretch * pvc_hopsize ); // *hopsize is a bug but has an interesting effect
            // overlap/add
            m_pvc->pv_overlap_add( pvc_win[pvc_which], (int)(pvc_hopsize * time_stretch + .5f) );
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

    mono2stereo( buffer, num_frames ); 
    gain_buf( buffer, num_frames * 2, gain ); 
    pan_buf( buffer, num_frames, pan ); 
    
    pvc_win[0] = pvc_win[1] = NULL; 
    return ongoing; 
}


Template * Raw::copy( bool copyid ) const
{
//    Transient * tran = new Transient( this->sound, copyid ? this->id : 0 );
//    tran->copy_params( *this );
  
    Raw * ron = new Raw( *this ); 

    return ron;
}


Transient::Transient( const Frame & event, time_t myid )
	: Raw( event, myid )
{
	type = TT_TRANSIENT;
	typestr = "transient";
}
 
Transient::Transient( const Transient & rhs )
	: Raw( rhs )
{
	type = TT_TRANSIENT;
	typestr = "transient"; 
}


Template * Transient::copy( bool copyid ) const
{
	Transient * tran = new Transient( *this );
	return tran;
}


t_CKBOOL File::stick( SAMPLE * buffer, t_CKUINT num_frames )
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
    Residue * res = (Residue *)data;
    res->thread_done = false;

    int samples, write = 1;
    
    BB_log( BB_LOG_INFO, "you are now entering the ts_go() function..." );
    while( !res->shutup ) {
        // Zero the trees
        res->ts->resetTrees();
        // Read file (total_levels will never be > CUTOFF because that's the extent to which the slider goes)
        samples = res->tsio->ReadSoundFile( res->tsio->ifilename, res->ts->tree->values(), 1 << res->requested_levels/*res->ts->tree->getSize()*/ );
        if( samples <= 0 ) 
        {
            BB_log( BB_LOG_SEVERE, "ts_go(): read %i samples from the file %s!", samples, res->tsio->ifilename );
            res->shutup = true;
            break;
        }
        // Change levels accordingly
        res->ts->resetTreeLevels( lg( samples ) );
        res->total_levels = res->ts->tree->getLevels();
        
        res->mutex.acquire();
        if( res->ts->setup() ) {
            res->ts->synth();
            // keep trying to write to file/buffer, until written or told to shut up
            while( !res->shutup && 
                !(write = res->tsio->WriteSoundFile( res->tsio->ofilename, res->ts->outputSignal(), res->ts->tree->getSize() ) ) ) {
                res->mutex.release();
                usleep( 10000 );
                res->mutex.acquire();
            }
        }
 
        res->mutex.release();
        
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


Residue::~Residue()
{
    //fprintf( stderr, "residue, destructor...\n" );
    BB_log( BB_LOG_FINE, "Deleting residue template" ); 
    this->shutup = TRUE;
    usleep( 10000 );
    mutex.acquire();
    SAFE_DELETE( thread );
    mutex.release();
    SAFE_DELETE( ts );
    SAFE_DELETE( tsio );
    SAFE_DELETE( src );
    //fprintf( stderr, "residue, destructor more...\n" );
}

t_CKBOOL Residue::stick( SAMPLE * buffer, t_CKUINT num_frames )
{
//    assert( m_done == FALSE );

    // start the treesynth
    if( thread == NULL )
    {
        thread = new XThread;
        thread->start( ts_go, this );
        usleep( 10000 );
    }
    if( shutup ) // was "else if" but that hung if the file was not found
    {
        //fprintf( stderr, "residue, shutting up...\n" );
        //mutex.acquire();
        //delete thread;
        while( !thread_done )
            usleep( 10000 );
        SAFE_DELETE( thread );
        //mutex.release();
        //fprintf( stderr, "residue, shutting up more...\n" );
        //thread = NULL;
        shutup = FALSE;
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
    return src->stick( buffer, num_frames );
}

// copy from this into a new template
Template * Residue::copy( bool copyid ) const
{
    // The IO
    TreesynthIO * tsio_ross = new TreesynthIO;
    strcpy( tsio_ross->ifilename, tsio->ifilename );
    strcpy( tsio_ross->ofilename, tsio->ofilename );
    tsio_ross->rm_mode = tsio->rm_mode;

    // The TS
    Treesynth * ts_ross = new Treesynth;
    ts_ross->tree = new Tree;
    ts_ross->tree->initialize( lg( CUTOFF ) );
    ts_ross->initialize();
    
    // Create Ross
    Residue * ross = new Residue( ts_ross, tsio_ross, copyid ? this->id : 0 );
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

    Residue *r = (Residue *)(&rhs);
    ts->percentage = r->ts->percentage;
    ts->kfactor = r->ts->kfactor;
    ts->stoplevel = r->ts->stoplevel;
    ts->startlevel = r->ts->startlevel;
    ts->randflip = r->ts->randflip;
    ts->ancfirst = r->ts->ancfirst;
    
    requested_levels = r->requested_levels;
    ts->resetTreeLevels( requested_levels );
    total_levels = ts->tree->getLevels();
    lefttree = r->lefttree;
}


void Residue::set_param( int which, double value )
{

    assert( ts != NULL );
    // call parents

    Template::set_param( which, value );

    mutex.acquire();
    switch( which )
    {
    case PERCENTAGE:
        ts->percentage = value;
        break;
    case K:
        ts->kfactor = value;
        break;
    case STOPLEVEL: 
        ts->stoplevel = (int)value;
        break;
    case STARTLEVEL:
        ts->startlevel = (int)value;
        break;
    case TOTAL_LEVELS:
        requested_levels = (int)value;
        break;
    case RANDFLIP:
        ts->randflip = value > 0.5;
        break;
    case ANCFIRST:
        ts->ancfirst = value > 0.5;
        break;
    default:
        break;
    }
    mutex.release();
}


LoopTemplate::~LoopTemplate()
{
    BB_log( BB_LOG_FINE, "Deleting loop template" ); 
    SAFE_DELETE( temp );
    SAFE_DELETE_ARRAY( acc_buffer );
    SAFE_DELETE_ARRAY( arg_buffer );
}

t_CKBOOL LoopTemplate::stick( SAMPLE * buffer, t_CKUINT num_frames )
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
    t_CKUINT remaining = num_frames;
    t_CKUINT togen = (until_next <= num_frames ? until_next : num_frames);
    t_CKUINT offset = 0;
    t_CKBOOL yes;
        
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
    ((Gaussian *)dist)->periodicity = periodicity;

    while( remaining > 0 )
    {
		// need to figure out time until next event
        if( until_next == 0 )
        {
            // generate the time to wait
            //until_next = (uint)(dist->next_interval( mean ) + .5 );
			double unquantized = dist->next_interval( mean );
			until_next = (uint)(BirdBrain::srate() * quantize_time(unquantized / BirdBrain::srate()) + .5);
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
Timeline::Timeline( TIME dur, time_t myid )
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
    TIME start = (TIME)( where * duration );
    start = (TIME)( BirdBrain::srate() * quantize_time( start / BirdBrain::srate() ) ); 
    instances.push_back( Instance( ui_temp, start, 0, y_offset ) );

    // keep the vector sorted
    for( int i =  instances.size()-2; i >= 0; i-- )
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

    // set the first until_next
    until_next = instances[0].start_time;
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
void Timeline::rewind()
{ 
    AudioSrc::rewind();
    dur_pos = 0;
    curr_index = 0;
    for( int i = 0; i < copies.size(); i++ )
    {
        SAFE_DELETE(copies[i]);
    }
    copies.clear();
    if( instances.size() )
        until_next = instances[0].start_time;
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
    BB_log( BB_LOG_INFO, "timeline waiting for contents to stop playing" );
    while( index < copies.size() )
    {
        // wait for the copy
        wait_for( copies[index] );
        index++;
    }
}

    
t_CKBOOL Timeline::stick( SAMPLE * buffer, t_CKUINT num_frames )
{
//    assert( m_done == FALSE );

    // are we done? (not yet)
    if( m_stop_asap ) // used to be "|| dur_pos >= duration - 1" and no stop_copies
        return FALSE;
    if( dur_pos >= duration - 1 )
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
    t_CKUINT remaining = (t_CKUINT)x_min( (TIME)num_frames, (duration - dur_pos) );
    t_CKUINT togen = (t_CKUINT)x_min( until_next, remaining );
    t_CKUINT offset = 0;//num_frames - remaining;
    t_CKBOOL yes;
    Template * temp = NULL;

    // zero
    memset( acc_buffer, 0, sizeof(SAMPLE) * acc_buffer_size );

    while( remaining > 0 )
    {
        // need to figure out time until next event
        while( (t_CKUINT)until_next == 0 )
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
            togen = (t_CKUINT)(until_next <= remaining ? until_next : remaining);
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
        now_butter->slide = (float)(dur_pos) / duration;
        now_butter->slide_locally = false; // or true, doesn't matter.
    }

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
}



// Marbles
Marble::Marble( UI_Template * u, uint weight, bool norepeat, double randfac )
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


t_CKBOOL BagTemplate::stick( SAMPLE * buffer, t_CKUINT num_frames )
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
    t_CKUINT remaining = num_frames;
    t_CKUINT togen = (t_CKUINT)x_min( until_next, remaining );
    t_CKUINT offset = 0;
    t_CKBOOL yes;
    Template * temp = NULL; 

    // expected/average interval between events (unit: fractional samples)
    double mean = 1.0 / density * BirdBrain::srate(); 

    // zero
    memset( acc_buffer, 0, sizeof(SAMPLE) * acc_buffer_size );

    // determine distribution   
    dist = Gaussian::instance();
    ((Gaussian *)dist)->periodicity = periodicity;

    while( remaining > 0 )
    {
        // need to figure out time until next event
        if( (t_CKUINT)until_next == 0 )
        {
            // generate the time to wait
            //until_next = (TIME)(dist->next_interval( mean ) + .5 );
            double unquantized = dist->next_interval( mean );
            until_next = (TIME)(BirdBrain::srate() * quantize_time(unquantized / BirdBrain::srate()) + .5);
            // set the amount to generate
            // (could be less than until_next because of buffer boundary)
            togen = (t_CKUINT)(until_next <= remaining ? until_next : remaining);
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
    
    // return 
    return TRUE;
}


void BagTemplate::rewind()
{
    AudioSrc::rewind();
    for( int i = 0; i < copies.size(); i++ )
    {
        SAFE_DELETE(copies[i]);
    }
    copies.clear();
    until_next = 0; 
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
    BB_log( BB_LOG_INFO, "bagtemplate waiting for contents to stop playing" );
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
        marbles[marble].likelihood = (uint)value;
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

/*std::vector<uint> new_ids;    // mapping of current even ids to new odd ids in case
                            // they get resaved, so that the above solution works
                            // even when they are reloaded
uint get_id( uint orig_id )
{
    // figure out new id from mapping
    uint myid = 0;
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


//-----------------------------------------------------------------------------
// name: struct Writer 
// desc: write stuff to be read
//-----------------------------------------------------------------------------

bool Writer::open( char * filename )
{
    ofile.open( filename, ios::out );

    return ofile.good();
}

void Writer::close()
{
    ofile.close();
}

// templates
int Writer::write_template( const Template * tempt )
{
    int ret;

    // make sure file is open
    if( ofile.bad() )
    {
        msg_box( "internal errorrr", "cannot write to no file." );
        return 0;
    }

    time_t myid = tempt->id; // get_id( tempt->id ); 

    ofile << tempt->type << " " << myid << std::endl;
    ofile << tempt->name << " " << tempt->type_str() << std::endl;
    ofile << tempt->time_stretch << " " << tempt->freq_warp << " " << tempt->gain << " "
          << tempt->pan << " " << tempt->periodicity << " " << tempt->density << std::endl;

/*  // not sure how this other method works (I mean it doesn't, but stuff should be changed for it to work)
    fprintf( ofile, "%i\n", tempt->type );
    fprintf( ofile, "%s %s\n", tempt->name, tempt->type_str() );
    fprintf( ofile, "%f %f %f %f %f %f\n", tempt->time_stretch, tempt->freq_warp, tempt->gain, 
             tempt->pan, tempt->periodicity, tempt->density );
*/

    switch( tempt->type )
    {
    case TT_DETERMINISTIC:
        ret = write_det( (Deterministic *) tempt );
        break;
	case TT_RAW:
		ret = write_raw( (Raw *) tempt );
		break;
    case TT_TRANSIENT:
        ret = write_trans( (Transient *) tempt );
        break;
    case TT_FILE:
        ret = write_file( (File *) tempt );
        break;
    case TT_RESIDUE:
        ret = write_res( (Residue *) tempt );
        break;
    case TT_LOOP:
        ret = write_loop( (LoopTemplate *) tempt );
        break;
    case TT_TIMELINE:
        ret = write_tl( (Timeline *) tempt );
        break;
    case TT_BAG:
        ret = write_bag( (BagTemplate *) tempt ); 
        break;
    default:
        ret = 0;
    }

    return ret;
}

int Writer::write_det( const Deterministic * debt )
{
    return write_tracks( debt->tracks ); 
}

int Writer::write_trans( const Transient * trance )
{
    return write_frame( trance->sound );
}

int Writer::write_raw( const Raw * war )
{
	return write_frame( war->sound );
}

int Writer::write_file( const File * phial )
{
    ofile << phial->filename << std::endl;
    return 1;
}

int Writer::write_res( const Residue * race )
{
    ofile << race->total_levels << " " << race->requested_levels << " " << race->lefttree << std::endl;
    TreesynthIO * io = race->tsio;
    int s;
    s = strcmp( io->ifilename, "" );
    ofile << (s != 0) << io->ifilename << std::endl;
    s = strcmp( io->leftfile, "" );
    ofile << (s != 0) << io->leftfile << std::endl;
    s = strcmp( io->ofilename, "" );
    ofile << (s != 0) << io->ofilename << std::endl;
    ofile << io->rm_mode << " " << io->write_to_buffer << " " << io->write_to_file << std::endl;
    Treesynth * tsth = race->ts;
    ofile << tsth->percentage << " " << tsth->kfactor << " " << tsth->stoplevel << " "
          << tsth->startlevel << " " << tsth->randflip << " " << tsth->ancfirst << std::endl;
    return 1; 
}

int Writer::write_loop( const LoopTemplate * hole )
{
    ofile << hole->random << std::endl;
    return write_template( hole->temp );
}


// idness is bad. needs to be unique somehow.
int Writer::write_tl( const Timeline * teal )
{
    const Instance * inst;
    int num_instances = teal->instances.size();
    time_t inst_id; 
    
    // write timeline and instance information
    ofile << teal->duration << " " << teal->loop << " " << num_instances << std::endl;
    for( int i = 0; i < num_instances; i++ )
    {
        inst = &(teal->instances[i]); 
        inst_id = inst->ui_temp->core->id; //get_id( inst->ui_temp->core->id ); 
        ofile << inst_id << " " << inst->start_time << " " << inst->end_time << " " 
              << inst->y_offset << std::endl;
    }

    // write instances themselves (as long as they're not timelines, for now) (to prevent infinite writing)
    for( int j = 0; j < num_instances; j++ )
    {
        inst = &(teal->instances[j]);
        if( inst->ui_temp->core->type == TT_TIMELINE )
        {
            ofile << false << std::endl; 
        }
        else
        {
            ofile << true << std::endl;
            write_template( inst->ui_temp->core );
        }
    }

    return 1;
}

int Writer::write_bag( const BagTemplate * bat )
{
    const Marble * mab; 
    int num_marbles = bat->marbles.size();
    time_t mab_id; 
    
    // write marble information
    ofile << num_marbles << std::endl;
    for( int i = 0; i < num_marbles; i++ )
    {
        mab = &(bat->marbles[i]); 
        mab_id = mab->ui_temp->core->id; //get_id( inst->ui_temp->core->id ); 
        ofile << mab_id << " " << mab->playonce << " " << mab->likelihood << " " 
              << mab->random << std::endl;
    }

    // write marbles themselves (as long as they're not bagtemplates, which they shouldn't be)
    for( int j = 0; j < num_marbles; j++ )
    {
        mab = &(bat->marbles[j]); 
        if( mab->ui_temp->core->type == TT_BAG)
        {
            ofile << false << std::endl; 
        }
        else
        {
            ofile << true << std::endl;
            write_template( mab->ui_temp->core );
        }
    }

    return 1;
}


// other things (parts of templates)
int Writer::write_tracks( const std::vector<Track *> &event )
{
    int num_tracks = event.size();
    int track_size;
    Track * t;
    freqpolar * fp; 
    ofile << "num_tracks " << num_tracks << std::endl;
    for( int i = 0; i < num_tracks; i++ )
    {
        t = event[i]; 
        track_size = t->history.size(); 
        ofile << "track " << track_size << " " << t->id << " " << t->state << " "
              << t->start << " " << t->end << " " << t->phase << " " << t->historian << std::endl;
        for( int j = 0; j < track_size; j++ )
        {
            fp = &(t->history[j]); 
            ofile << fp->freq << " " << fp->p.mag << " " << fp->p.phase << " " << fp->time
                  << " " << fp->bin << " " << fp->isMatched << std::endl;
        }
    }
    return num_tracks;
}

// note: this only writes the waveform info, not the cmp/pol/freq stuff; may need to be added later
int Writer::write_frame( const Frame &frame )
{
    ofile << "frame " << frame.wlen << " " << frame.wsize << " " << frame.time << std::endl;
    for( int i = 0; i < frame.wsize; i++ )
    {
        ofile << frame.waveform[i] << " ";
    }
    ofile << std::endl;
    return frame.wsize;
}


//-----------------------------------------------------------------------------
// name: struct Reader 
// desc: read stuff Writer wrote
//-----------------------------------------------------------------------------

bool Reader::open( char * filename )
{
    ifile.open( filename, ios::in );

    return ifile.good();
}

void Reader::close()
{
    ifile.close();
}

// templates
Template * Reader::read_template()
{
    int type;
    time_t id; 
    std::string name, typestr; 
    double time_stretch, freq_warp, gain, pan, periodicity, density; 
    Template * tempt;

    // make sure file is open
    if( ifile.bad() )
    {
        msg_box( "internal errorrr", "cannot read from no file" );
        return 0;
    }
    
    ifile >> type >> id; 
    ifile >> name >> typestr >> time_stretch >> freq_warp >> gain >> pan >> periodicity >> density; 
    //if( id != 0 )id++; // id is now even & so unique

    // tempt gets created (new) in one of these functions
    switch( type )
    {
    case TT_DETERMINISTIC:
        tempt = read_det( id );
        break;
	case TT_RAW:
		tempt = read_raw( id );
		break;
    case TT_TRANSIENT:
        tempt = read_trans( id );
        break;
    case TT_FILE:
        tempt = read_file( id );
        break;
    case TT_RESIDUE:
        tempt = read_res( id );
        break;
    case TT_LOOP:
        tempt = read_loop( id );
        break;
    case TT_TIMELINE:
        tempt = read_tl( id );
        break;
    case TT_BAG:
        tempt = read_bag( id );
        break;
    default:
        tempt = NULL;
    }

    if( tempt != NULL )
    {
        tempt->name = name; 
        tempt->typestr = typestr;
        tempt->time_stretch = time_stretch;
        tempt->freq_warp = freq_warp;
        tempt->gain = gain;
        tempt->pan = pan;
        tempt->periodicity = periodicity;
        tempt->density = density;
    }

    return tempt;
}

Deterministic * Reader::read_det( time_t myid )
{
    std::vector<Track *> temp_tracks; 
    Deterministic * debt;
    
    if( read_tracks( temp_tracks ) )
        debt = new Deterministic( temp_tracks, myid );
    else
        debt = NULL;

    return debt;
}

Transient * Reader::read_trans( time_t myid )
{
    Frame f;
    Transient * trance;
    
    if( read_frame( f ) )
        trance = new Transient( f, myid );
    else
        trance = NULL;
    
    return trance;
}

Raw * Reader::read_raw( time_t myid )
{
    Frame f;
	Raw * war;
    
    if( read_frame( f ) )
        war = new Raw( f, myid );
    else
        war = NULL;
    
    return war;
}

File * Reader::read_file( time_t myid )
{
    std::string path;
    ifile >> path;
    
    File * phial;

    if( path.empty() )
        phial = NULL;
    else
        phial = new File( path, myid );

    return phial;
}

Residue * Reader::read_res( time_t myid )
{
    int total_levels, requested_levels, exists; 
    bool lefttree;

    ifile >> total_levels >> requested_levels >> lefttree; 
    
    TreesynthIO * io = new TreesynthIO;
    // input file name
    ifile >> exists;
    if( exists )
        ifile.getline( io->ifilename, 1024 ); 
    // left file name
    ifile >> exists; 
    if( exists )
        ifile.getline( io->leftfile, 1024 );
    // output file name
    ifile >> exists;
    if( exists )
        ifile.getline( io->ofilename, 1024 ); 
    // other io information
    ifile >> io->rm_mode >> io->write_to_buffer >> io->write_to_file;

    // create and initialize Treesynth object
    Treesynth * tsth = new Treesynth;
    tsth->tree = new Tree;
    tsth->tree->initialize( lg(CUTOFF) );
    tsth->initialize();
    // read in new parameters
    ifile >> tsth->percentage >> tsth->kfactor >> tsth->stoplevel >> tsth->startlevel 
          >> tsth->randflip >> tsth->ancfirst; 
    tsth->tree->resetLevels( total_levels );

    // Now, make a new template for the residue
    Residue * race = new Residue( tsth, io, myid );
    
    return race;
}

LoopTemplate * Reader::read_loop( time_t myid )
{
    LoopTemplate * hole;
    double rand;
    ifile >> rand;
    
    Template * orig = read_template();
    if( orig != NULL )
        hole = new LoopTemplate( *orig, myid );
    
    hole->random = rand;

    return hole;
}


struct pseudoInstance
{
    time_t id;
    TIME start_time;
    TIME end_time;
    float y_offset;
};

Timeline * Reader::read_tl( time_t myid )
{
    TIME duration; 
    int loop, num_instances; 
    bool valid; 
    Template * temp, * poor; 
    UI_Template * uit, * dummy;

    ifile >> duration >> loop >> num_instances; 

    Timeline * teal = new Timeline( duration, myid ); 

    // instances
    pseudoInstance * inst = new pseudoInstance[num_instances];
    
    // read basic instance info
    for( int i = 0; i < num_instances; i++ )
    {
        ifile >> inst[i].id >> inst[i].start_time >> inst[i].end_time >> inst[i].y_offset;
        //inst[i].id += 1;
    }


    // read/match actual instances
    for( int j = 0; j < num_instances; j++ )
    {
        dummy = NULL;
        uit = NULL;

        // match: see if it is already in library
        for( int k = 0; k < Library::instance()->size(); k++ )
        {
            if( Library::instance()->templates[k]->core->id == inst[j].id ) // jackpot
            {   
                uit = Library::instance()->templates[k]; 
                break;
            }
        }

        // read in info if it's in .tap file; 
        ifile >> valid; 
        if( valid )
        {
            // already have a match; ignore this info
            if( uit != NULL )
            {
                poor = read_template();
                SAFE_DELETE( poor );
            }
            // no match found; use this info
            else
            {
                // create main template
                temp = read_template();
                Library::instance()->add( temp );
                uit = Library::instance()->templates.back();
            }
        }

        // if there is a template, place on timeline in two steps
        if( uit != NULL )
        {
            // make dummy copy
            Library::instance()->add( uit->core );
            dummy = Library::instance()->templates.back();
            uit->makedummy( dummy );

            // place dummy on timeline
            teal->place( dummy, 
                         (double) inst[j].start_time / teal->duration, 
                         inst[j].y_offset );
        }
    }

    SAFE_DELETE_ARRAY( inst );

    return teal;
}       

struct pseudoMarble
{
    time_t id; 
    bool playonce;
    uint likelihood;
    double random;
};

BagTemplate * Reader::read_bag( time_t myid )
{
    int num_marbles; 
    bool valid; 
    Template * temp, * poor; 
    UI_Template * uit, * dummy;
    Marble * mabel;

    ifile >> num_marbles; 

    BagTemplate * bat = new BagTemplate( myid ); 

    // marbles
    pseudoMarble * mab = new pseudoMarble[num_marbles];
    
    // read basic marbles info
    for( int i = 0; i < num_marbles; i++ )
    {
        ifile >> mab[i].id >> mab[i].playonce >> mab[i].likelihood >> mab[i].random;
    }

    // read/match actual marbles
    for( int j = 0; j < num_marbles; j++ )
    {
        dummy = NULL;
        uit = NULL;

        // match: see if it is already in library
        for( int k = 0; k < Library::instance()->size(); k++ )
        {
            if( Library::instance()->templates[k]->core->id == mab[j].id )  // jackpot
            {   
                uit = Library::instance()->templates[k]; 
                break;
            }
        }

        // read in info if it's in .tap file; 
        ifile >> valid; 
        if( valid )
        {
            // already have a match; ignore this info
            if( uit != NULL )
            {
                poor = read_template();
                SAFE_DELETE( poor );
            }
            // no match found; use this info
            else
            {
                // create main template
                temp = read_template();
                Library::instance()->add( temp );
                uit = Library::instance()->templates.back();
            }
        }

        // if there is a template, place in bag in two steps
        if( uit != NULL )
        {
            // make dummy copy
            Library::instance()->add( uit->core );
            dummy = Library::instance()->templates.back();
            uit->makedummy( dummy );

            // place dummy in bag
            mabel = bat->insert( dummy );
            if( mabel )
            {
                mabel->playonce = mab[j].playonce;
                mabel->likelihood = mab[j].likelihood;
                mabel->random = mab[j].random;
            }
        }
    }

    SAFE_DELETE_ARRAY( mab );

    return bat;
}

// other things (parts of templates)
int Reader::read_tracks( std::vector<Track *> &event )
{   
    Track * t;
    freqpolar temp_fp;
    std::string junk;
    int num_tracks, track_size;
    
    ifile >> junk >> num_tracks;

    for( int i = 0; i < num_tracks; i++ )
    {
        ifile >> junk >> track_size;
        t = new Track;
        ifile >> t->id >> t->state >> t->start >> t->end >> t->phase >> t->historian;

        for( int j = 0; j < track_size; j++ )
        {
            ifile >> temp_fp.freq >> temp_fp.p.mag >> temp_fp.p.phase >> temp_fp.time
                  >> temp_fp.bin >> temp_fp.isMatched; 
            t->history.push_back( temp_fp );
        }
        event.push_back( t );
    }
    
    return num_tracks;
}

int Reader::read_frame( Frame &frame )
{
    std::string junk;
    uint wlen, wsize;

    ifile >> junk;
    ifile >> wlen >> wsize >> frame.time; // this time is now of type TIME!!

    frame.alloc_waveform( wlen );
    frame.wsize = wsize;

    for( int i = 0; i < frame.wsize; i++ )
    {
        ifile >> frame.waveform[i]; 
    }
    
    return frame.wsize;
}


// template grabber
TemplateGrabber::TemplateGrabber( long hint_num_samples )
{
    m_hint_num_samples = hint_num_samples;
}


// grab
t_CKBOOL TemplateGrabber::grab( Template * temp, Frame * out )
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
t_CKBOOL TemplateGrabber::grab( Template * temp, Frame * out, long max_num_samples )
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
    t_CKBOOL still_going = true;
    // end pointer (for safety)
    SAMPLE * end = NULL;
    // write pointer
    SAMPLE * where = NULL;

    // log
    if( max_num_samples < 0 )
        BB_log( BB_LOG_INFO, "grabbing entire template '%s'...", temp->name.c_str() );
    else
        BB_log( BB_LOG_INFO, "grabbing template '%s' for %.3f seconds...", temp->name.c_str(), (TIME)max_num_samples / BirdBrain::srate() );

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
