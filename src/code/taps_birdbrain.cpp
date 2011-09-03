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
// file: taps_birdbrain.cpp
// desc: taps birdbrain utilities
//
// author: Ananya Misra (amisra@cs.princeton.edu)
//         Ge Wang (gewang@cs.princeton.edu)
//         Perry R. Cook (prc@cs.princeton.edu)
// date: Autumn 2004
//-----------------------------------------------------------------------------
#include "taps_birdbrain.h"
#include "util_thread.h"
#include <stdarg.h>

// for getcwd and chdir
#ifdef __PLATFORM_WIN32__
  #include <direct.h>
#else
  #include <unistd.h>
#endif


t_TAPUINT BirdBrain::our_srate = 44100;
t_TAPUINT BirdBrain::our_fft_size = 4096;
t_TAPUINT BirdBrain::our_wnd_size = 512;
t_TAPUINT BirdBrain::our_syn_wnd_size = 512;
t_TAPUINT BirdBrain::our_hop_size = 128;
t_TAPUINT BirdBrain::our_max_tracks = 4;
t_TAPFLOAT BirdBrain::our_over_tracking = 1;
t_TAPBOOL BirdBrain::our_rtaudio_blocking = FALSE;
t_TAPUINT BirdBrain::our_rtaudio_buffer_size = 1024;
t_TAPSINGLE BirdBrain::our_record_buffer_size = 60;
float BirdBrain::our_freq_max = -1;
float BirdBrain::our_freq_min = -1;
t_TAPUINT BirdBrain::our_frame_rate = 30; 
t_TAPBOOL BirdBrain::our_use_gui = TRUE; 
t_TAPBOOL BirdBrain::our_white_bg = FALSE;
std::string BirdBrain::our_start_dir = "";
std::string BirdBrain::our_last_open_dir = "";
std::string BirdBrain::our_last_save_dir = "";

// shift left by "shift"
// put zeros in last "shift" values in buffer
// add to_add to buffer
// don't return first "shift" values in buffer
// size should be the size of buffer and of to_add
void BirdBrain::ola ( SAMPLE *buffer, SAMPLE *to_add, int shift, int size )
{
    int i;
    for( i = 0; i < size; i++ )
    {
        buffer[i] = (i < (size - shift)) ? buffer[i+shift] : 0;
        buffer[i] += to_add[i];
    }
}

// shift left by "shift"
// put zeros in last "shift" values in buffer
// add to_add to buffer
// don't return first "shift" values in buffer (user gets them before ola)
// buffer_size and to_add_size may differ, but to_add_size is assumed to be <= buffer_size
void BirdBrain::ola( SAMPLE *buffer, SAMPLE *to_add, int shift, int buffer_size, int to_add_size)
{
	for(int i = 0; i < buffer_size; i++) {
		buffer[i] = (i < buffer_size - shift) ? buffer[i+shift] : 0;
		if(i < to_add_size)
			buffer[i] += to_add[i];
	}
}

void BirdBrain::goto_dir( const std::string & path )
{
    int error = -1;
#ifdef __PLATFORM_WIN32__
    error = _chdir( path.c_str() ); 
#else
    error = chdir( path.c_str() );
#endif
    if( error != 0 )
        BB_log( BB_LOG_WARNING, "could not change to directory : %s", path.c_str() );
    else
        BB_log( BB_LOG_INFO, "current directory changed to : %s", path.c_str() );
}


void BirdBrain::goto_start_dir()
{
    goto_dir( our_start_dir );
}


void BirdBrain::filter( const SAMPLE *x, const SAMPLE *winxp, SAMPLE *y, int len, float *B, int lenb, float *A, int lena )
{
    int j;

    // normalize by A[0]
    assert( A[0] != 0.0f );
    if( A[0] != 1.0f )
    {
        for( j = 0; j < lenb; j++ )
            B[j] /= A[0];
        for( j = 1; j < lena; j++ )
            A[j] /= A[0];
        A[0] = 1.0f;
    }

    for( int i = 0; i < len; i++ )
    {
        y[i] = 0.0f;
        for( j = 0; j < lenb; j++ )
        {
            if( i-j >= 0 )
                y[i] += B[j] * x[i-j];
                // y[i] += B[j] * ( (i-j) >= 0 ? x[i-j] : 0.0f );
            else
                y[i] += B[j] * winxp[len+i-j];
        }

        for( j = 1; j < lena; j++ )
        {
            if( i-j >= 0 )
                y[i] -= A[j] * y[i-j];
            else
                y[i] -= A[j] * y[len+i-j];
        }
    }
}




void BirdBrain::scale( SAMPLE *buffer, t_TAPUINT len, float factor )
{
    for( t_TAPUINT i = 0; i < len; i++ )
        buffer[i] *= factor;
}




// scale the gain lost from zero padding
void BirdBrain::scale_fft( SAMPLE *buffer, t_TAPUINT len, t_TAPUINT fft_size, t_TAPUINT wnd_size )
{
    assert( fft_size != 0 && wnd_size != 0 );
    scale( buffer, len, fft_size / (SAMPLE)wnd_size / 2.0f );
}




// undo scale_fft
void BirdBrain::scale_ifft( SAMPLE *buffer, t_TAPUINT len, t_TAPUINT fft_size, t_TAPUINT wnd_size )
{
    assert( fft_size != 0 && wnd_size != 0 );
    scale( buffer, len, 1.0f / (fft_size / (SAMPLE)wnd_size / 2.0f) );
}



// get base name (C:/folder/name.type -> name.type)
const char * BirdBrain::getbase( const char * str )
{
    const char * c = str + strlen(str);
    while( --c >= str )
    {
        if( *c == '/' || *c == '\\' )
            return c+1;
    }

    return str;
}

// get path (all but base name) (C:/folder/name.type -> C:/folder/)
std::string BirdBrain::getpath( const char * str )
{
    std::string s = str;
    char * c = (char *)(s.c_str() + strlen(str));
    const char * start = s.c_str();
    while( --c >= start )
    {
        if( *c == '/' || *c == '\\' )
        {
            *(c+1) = '\0'; 
            return std::string( s.c_str() );  
        }
    }
    //return std::string( s.c_str() );
    *(c+1) = '\0';
    return std::string( s.c_str() );
}


// get name (C:/folder/name.type -> name)
std::string BirdBrain::getname( const char * str )
{
    std::string temp = str;
    std::string base = getbase( temp.c_str() );
    char * c = (char *)base.c_str();
    const char * end = c + base.length() + 1;
    while( c < end )
    {
        if( *c == '.' )
        {
            *c = '\0';
            return std::string( base.c_str() );
        }
        c++;
    }

    return std::string(str);
}


// get part of name (C:/folder/res1_name.type -> name)
std::string BirdBrain::get_part_of_name( const char * str )
{
    static std::string temp;
    temp = getname( str );
    const char * c = (char *)temp.c_str();
    const char * end = c + temp.length() + 1;
    int n;

    while( c < end - 4 )
    {
        if( strncmp( c, "res_", 4 ) == 0 || strncmp( c, "out_", 4 ) == 0 ) 
            return std::string(c + 4);

        // evil
        if( strncmp( c, "res", 3 ) == 0 ) {
            for( n = 4; c + n + 1 < end; n++ )
                if( strncmp( c + n, "_", 1 ) == 0 )
                    return std::string( c + n + 1 );
        }
        
        c++;
    }

    return std::string(str);
}

// to string
std::string BirdBrain::toString( long num )
{
    char buffer[64];
    sprintf( buffer, "%i", num );
    return std::string( buffer );
}


Filter::Filter()
{
    A = B = NULL;
    winxp = ys = NULL;
    lena = lenb = len = 0;
}


Filter::~Filter()
{
    this->clear();
}

void Filter::init( float *B_, int lenb_, float *A_, int lena_ )
{
    assert( lenb_ != 0 && lena_ != 0 );
    this->clear();

    lenb = lenb_;
    lena = lena_;

    B = new float[lenb];
    A = new float[lena];

    memcpy( B, B_, lenb * sizeof(float) );
    memcpy( A, A_, lena * sizeof(float) );

    // normalize by A[0]
    assert( A[0] != 0.0f );
    int j;
    if( A[0] != 1.0f )
    {
        for( j = 0; j < lenb; j++ )
            B[j] /= A[0];
        for( j = 1; j < lena; j++ )
            A[j] /= A[0];
        A[0] = 1.0f;
    }

}

void Filter::clear()
{
    if( A ) delete [] A;
    if( B ) delete [] B;
    if( winxp ) delete [] winxp;
    if( ys ) delete [] ys;

    A = B = NULL;
    winxp = ys = NULL;
    lena = lenb = len = 0;
}

void Filter::clear_sigs()
{
    if( winxp ) delete winxp; 
    winxp = NULL;
    if( ys ) delete ys;
    ys = NULL;
    len = 0;
}

void Filter::apply_filter( Frame &x )
{
    int j;

    if( !winxp || len != x.wlen )
    {
        clear_sigs();
        len = x.wlen;
        winxp = new SAMPLE[len];
        ys = new SAMPLE[len];
        memset( winxp, 0, len * sizeof(SAMPLE) );
        memset( ys, 0, len * sizeof(SAMPLE) );
    }

    for( int i = 0; i < len; i++ )
    {
        ys[i] = 0.0f;
        for( j = 0; j < lenb; j++ )
        {
            if( i-j >= 0 )
                ys[i] += B[j] * x.waveform[i-j];
                // y[i] += B[j] * ( (i-j) >= 0 ? x[i-j] : 0.0f );
            else
                ys[i] += B[j] * winxp[len+i-j];
        }

        for( j = 1; j < lena; j++ )
        {
            if( i-j >= 0 )
                ys[i] -= A[j] * ys[i-j];
            else
                ys[i] -= A[j] * ys[len+i-j];
        }
    }

    // copy input into winxp for next frame
    memcpy( winxp, x.waveform, len * sizeof(SAMPLE) );
    // copy output into the frame
    memcpy( x.waveform, ys, len * sizeof(SAMPLE) );
}



// homeless utility function
// history indexes for start and end of overlap between two tracks
bool get_overlap_inds( const Track * t1, const Track * t2, int & t1_start, int & t1_end, int & t2_start, int & t2_end )
{
    if( t1 == NULL || t2 == NULL )
        return false;

    int overlap_start, overlap_end;
    
    // determine overlap start
    t1_start = t2_start = 0; 
    if( t1->start > t2->start )
    {
        overlap_start = (int)t1->start;
        //t1_start = 0;
        while( t2_start < t2->history.size() && (int)t2->history[t2_start].time < overlap_start )
            t2_start++;
        if( t2_start == t2->history.size() )
            return false;
    }
    else
    {
        overlap_start = (int)t2->start;
        //t2_start = 0;
        while( t1_start < t1->history.size() && (int)t1->history[t1_start].time < overlap_start )
            t1_start++;
        if( t1_start == t1->history.size() )
            return false;
    }       
    // determine overlap end
    t1_end = t1->history.size() - 1;
    t2_end = t2->history.size() - 1;
    if( t1->end < t2->end )
    {
        overlap_end = (int)t1->end;
        //t1_end = t1->history.size() - 1;
        while( t2_end >= 0 && (int)t2->history[t2_end].time > overlap_end )
            t2_end--;
        if( t2_end < 0 )
            return false;
    }
    else
    {
        overlap_end = (int)t2->end;
        //t2_end = t2->history.size() - 1;
        while( t1_end >= 0 && (int)t1->history[t1_end].time > overlap_end )
            t1_end--;
        if( t1_end < 0 )
            return false;
    }
    // no overlap; in case it got missed before
    if( overlap_start > overlap_end )
        return false;

    return true;
}



// constructor
SinEvent::SinEvent()
{
    average = NULL;
    start = -1;
    end = -1; 
}

// copy constructor (evil; its absence was deadly)
SinEvent::SinEvent( const SinEvent &me )
{
    this->event_tracks = me.event_tracks;
    this->start = me.start;
    this->end = me.end;
    if( me.average )
    {
        this->average = new Track;
        *(this->average) = *(me.average); 
    }
    else
    {
        this->average = NULL;
    }
}


// destructor
SinEvent::~SinEvent()
{
    if( average ) 
        delete average;
    average = NULL;
}



// assignment
SinEvent &SinEvent::operator=( const SinEvent &rhs )
{
    this->event_tracks = rhs.event_tracks;
    this->start = rhs.start;
    this->end = rhs.end;
    if( rhs.average )
    {
        this->average = new Track;
        *(this->average) = *(rhs.average); 
    }
    else
    {
        this->average = NULL;
    }
    return *this;
}


// clear
void SinEvent::clear()
{
    // should tracks in this be deleted? probably not.
    // nope. pointers to tracks used elsewhere. deleted with driver. 
    event_tracks.clear();
    start = -1; 
    end = -1;
    if( average != NULL )
        delete average;
    average = NULL;
}


// add a track to the group of tracks defining the event
bool SinEvent::add_track( Track *track )
{
    if( track != NULL )
    {
        // update average of all the tracks
        Track * new_avg = new Track;
        int av_start, av_end, tr_start, tr_end, av_size, tr_size; 
        if( average == NULL )
        {
            av_start = av_end = tr_start = tr_end = -1;
            av_size = 0;
        }
        else
        {
            av_size = average->history.size();
            if( !get_overlap_inds( average, track, av_start, av_end, tr_start, tr_end ) )
            {
                fprintf( stderr, "Could not add track to group: no overlap\n" );
                delete new_avg;
                return false;
            }
        }
        tr_size = track->history.size();
        freqpolar fp;
        int av_ind = 0, tr_ind = 0;
            // pre-overlap (only one of these while loops should run since av_start or tr_start = 0)
        while( av_ind < av_start )
        {
            new_avg->history.push_back( average->history[av_ind] );
            av_ind++;
        }
        while( tr_ind < tr_start ) 
        {
            new_avg->history.push_back( track->history[tr_ind] );
            tr_ind++;
        }
            // enter overlap zone
        while( av_ind <= av_end && tr_ind <= tr_end )
        {
            if( (int)average->history[av_ind].time != (int)track->history[tr_ind].time )
            {
                fprintf( stderr, "Times don't match. Weird\n" ); 
                return false;
            }
            // compute new average
            fp = average->history[av_ind];
            fp.freq = fp.freq * (event_tracks.size() - 1) + track->history[tr_ind].freq;
            fp.p.mag = fp.p.mag * (event_tracks.size() - 1) + track->history[tr_ind].p.mag;
            fp.freq /= event_tracks.size();
            fp.p.mag /= event_tracks.size();
            new_avg->history.push_back( fp );
            av_ind++; tr_ind++;
        }
            // post-overlap (again at most one while loop should run)
        while( av_ind < av_size )
        {
            new_avg->history.push_back( average->history[av_ind] );
            av_ind++;
        }
        while( tr_ind < tr_size )
        {
            new_avg->history.push_back( track->history[tr_ind] );
            tr_ind++;
        }
            // reassign average to new average
        if( average )
            delete average;
        average = new_avg;

        // add track itself
        event_tracks.push_back( track );
        // update start and end times for full event
        if( start > track->start || start < 0 )
            start = track->start;
        if( end < track->end || end < 0 )
            end = track->end;
        average->start = start;
        average->end = end; 
    }
    else
    {
        fprintf( stderr, "Cannot add track to group. Your turn.\n" );
        return false;
    }

    return true;
}


// check whether given track is harmonically related to tracks in the group
bool SinEvent::cmp_harm( Track * track, float error, float overlap_amt )
{
    bool maybe = false;
    int et_ind, tr_ind, et_time, tr_time;
    int tr_start, tr_end, et_start, et_end;
    int int_ratio;
    float frac_ratio;
    for( int i = 0; i < event_tracks.size(); i++ )
    {
        if( !get_overlap_inds( event_tracks[i], track, et_start, et_end, tr_start, tr_end ) )
            continue;
        // check that at least overlap_amt (fraction) of at least one track overlaps with the other
        assert( et_start <= et_end );
        assert( tr_start <= tr_end );
        int overlap_len = tr_end - tr_start + 1;
        if( (float)overlap_len / track->history.size() < overlap_amt && 
            (float)overlap_len / event_tracks[i]->history.size() < overlap_amt )
            continue; 

        // check for harmonic relation with each track
        et_ind = et_start; tr_ind = tr_start;
        int_ratio = 0;
        frac_ratio = 0.0f;
        
        maybe = true;
        while( et_ind <= et_end && tr_ind <= tr_end )
        {
            if( !maybe ) break;

            et_time = (int)event_tracks[i]->history[et_ind].time;
            tr_time = (int)track->history[tr_ind].time; 
            
            if( et_time != tr_time )
                fprintf( stderr, "darn %i %i\n", et_time, tr_time );
            else
            {
                float r = event_tracks[i]->history[et_ind].freq / track->history[tr_ind].freq;
                float int_r = r<1 ? 1/r : r;
                if( fabs( int_r - (int)(int_r + 0.5f) ) > error ) // not integer ratio
                    maybe = false;
                else if( (frac_ratio < 1 && r > 1) || (frac_ratio > 1 && r < 1) ) // ratio inversed
                    maybe = false;
                else if( int_ratio > 0 && (int)(int_r + 0.5f) != int_ratio ) // ratio varies over track
                    maybe = false;
                else if( int_ratio <= 0 || frac_ratio <= 0 ) // ratio not yet determined for this event track
                {
                    int_ratio = (int)(int_r + 0.5f);
                    frac_ratio = r;
                }

                et_ind++; tr_ind++;
            }
        }

        // if it's still a maybe, it's harmonically matched to at least one track in the event
        if( maybe )
            break;
    }
    
    return maybe;
}

// check whether given track is related to group by common frequency & amplitude modulation
// maybe the modulation should be computed differently (try to understand that icmc paper)
bool SinEvent::cmp_mod( Track * track, float f_error, float a_error, float overlap_amt )
{
    int av_start, av_end, av_ind, tr_start, tr_end, tr_ind; 
    if( !get_overlap_inds( average, track, av_start, av_end, tr_start, tr_end ) )
        return false;
    // check that at least overlap_amt (fraction) of at least one track overlaps with the other
    assert( av_start <= av_end );
    assert( tr_start <= tr_end );
    int overlap_len = tr_end - tr_start + 1;
    if( (float)overlap_len / track->history.size() < overlap_amt && 
        (float)overlap_len / average->history.size() < overlap_amt )
        return false;

    av_ind = av_start; 
    tr_ind = tr_start;
    
    float f_ratio = 0, a_ratio = 0, fr, ar;
    bool maybe = true;

    while( av_ind <= av_end && tr_ind <= tr_end )
    {
        fr = average->history[av_ind].freq / track->history[tr_ind].freq;
        ar = average->history[av_ind].p.mag / track->history[tr_ind].p.mag;
        
        if( f_ratio > 0 && fabs(f_ratio - fr) > f_error )
            maybe = false;
        if( a_ratio > 0 && fabs(a_ratio - ar) > a_error )
            maybe = false;
        if( f_ratio <= 0 )
            f_ratio = fr;
        if( a_ratio < 0 )
            a_ratio = ar; 

        av_ind++; tr_ind++;
    }
    
    return maybe;   
}

// check whether given track shares onset and offset with rest of the group
bool SinEvent::cmp_onoff( Track * track, float on_error, float off_error )
{
    if( average == NULL )
        return false;

    return (fabs( track->start - average->start ) < on_error * BirdBrain::srate() && 
            fabs( track->end - average->end ) < off_error * BirdBrain::srate() && 
            track->end > average->start && average->end > track->end );
}


// Track
// constructor
Track::Track() { 
	id = 0; 
	state = 0; 
	start = end = 0; 
	phase = 0.0; 
	historian = 0; 
	current_syn_index = 0; 
}

// destructor
Track::~Track() { 
	return; 
}

// invert history
void Track::invert() 
{
	std::vector<freqpolar> temp = history;
	freqpolar fpt;
	history.clear();
        
	while ( !temp.empty() ) {
		fpt = temp.back();
		history.push_back( fpt );
		temp.pop_back();
	}

	t_TAPTIME t = start;
	start = end;
	end = t;

	return;
}



// log globals
int g_loglevel_bb = BB_LOG_INFO;
int g_logstack_bb = 0;
XMutex g_logmutex_bb;

// name
static const char * g_logstr[] = {
    "NONE",         // 0
    "SYSTEM_ERROR", // 1
    "SYSTEM",       // 2
    "SEVERE",       // 3
    "WARN!!",       // 4
    "INFORM",       // 5
    "CONFIG",       // 6
    "FINE!!",       // 7
    "FINER!",       // 8
    "FINEST",       // 9
    "ALL!!"           // 10
};

// log
void BB_log( int level, const char * message, ... )
{
    va_list ap;

    if( level > BB_LOG_CRAZY ) level = BB_LOG_CRAZY;
    else if( level <= BB_LOG_NONE ) level = BB_LOG_NONE + 1;

    // check level
    if( level > g_loglevel_bb ) return;

    g_logmutex_bb.acquire();
#ifdef __TAPS_SCRIPTING_ENABLE__
    fprintf( stderr, "[taps^]:" );
#else
    fprintf( stderr, "[taps]: " );
#endif
    fprintf( stderr, "(%i:%s): ", level, g_logstr[level] );

    // if( g_logstack_bb ) fprintf( stderr, " " );
    for( int i = 0; i < g_logstack_bb; i++ )
        fprintf( stderr, " | " );

    va_start( ap, message );
    vfprintf( stderr, message, ap );
    va_end( ap );

    fprintf( stderr, "\n" );
    g_logmutex_bb.release();
}

// set log level
void BB_setlog( int level )
{
    if( level > BB_LOG_CRAZY ) level = BB_LOG_CRAZY;
    else if( level < BB_LOG_NONE ) level = BB_LOG_NONE;
    g_loglevel_bb = level;

    // log this
    BB_log( BB_LOG_SYSTEM, "setting log level to: %i (%s)...", level, g_logstr[level] );
}

// push log
void BB_pushlog()
{
    g_logstack_bb++;
}

// pop log
void BB_poplog()
{
    g_logstack_bb--;
    if( g_logstack_bb < 0 ) g_logstack_bb = 0;
}


// number of features in feature vector
int g_num_features = 0;
