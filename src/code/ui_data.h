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
// name: ui_template.h
// desc: revised birdbrain ui library
//
// authors: Ananya Misra (amisra@cs.princeton.edu)
//          Ge Wang (gewang@cs.princeton.edu)
//          Perry R. Cook (prc@cs.princeton.edu)
// after EST. May 9, 2007, 4:38 p.m. Wednesday
//-----------------------------------------------------------------------------
#ifndef __UI_DATA_H__
#define __UI_DATA_H__

#include "ui_audio.h"
#include "ui_element.h"
#include "taps_birdbrain.h"
#include "taps_treesynth.h"
#include "taps_synthesis.h"
#include "taps_pvc.h"
#include "audicle_def.h"
#include "util_thread.h"

#include <map>

//-----------------------------------------------------------------------------
// enumerate data types for use in value
//-----------------------------------------------------------------------------
enum {
	V_INVALID = 0, // invalid
	V_S, // string
	V_TIME, // taptime
	V_FLOAT, // tapfloat
	V_INT, // tapint
	V_UINT, // tapuint
	V_BOOL, // tapbool
	V_C, // char
	V_T, // time_t
} Types;


//-----------------------------------------------------------------------------
// name: struct val_u
// desc: to store miscellaneous data types in one place
//-----------------------------------------------------------------------------
struct val_u 
{
public:
	// data
	std::string s;
	union {
		// limited types
		t_TAPTIME	Time;
		t_TAPFLOAT	Float;
		t_TAPINT	Int;
		t_TAPUINT	Uint;
		t_TAPBOOL	Bool;
		char		c;
		time_t		t;
	};

	// meta-data
protected:
	t_TAPTIME timestamp;
	t_TAPUINT datatype;

public:
	val_u(); // create invalid val_u, like for std::map to use
	val_u( t_TAPUINT type, t_TAPTIME stamp );
	t_TAPTIME get_timestamp();
	t_TAPUINT get_datatype();
	void debug(); // print
};


//-----------------------------------------------------------------------------
// name: class TimeData
// desc: trajectories
//-----------------------------------------------------------------------------
class TimeData
{
public:
	TimeData( t_TAPUINT type );
	t_TAPUINT get_datatype();
	val_u get_val( t_TAPTIME when, val_u (*interp_ptr)( val_u, val_u, t_TAPTIME ) = NULL );
	// val_u get_next_val( t_TAPTIME ___ );
	// val_u get_prev_val( t_TAPTIME ___ );
	void set_val( t_TAPTIME when, void * data );
	void get_vals( std::vector<val_u> & vec ); // pass in vector
	void debug(); // print

protected:
	t_TAPUINT datatype;
//	std::map<t_TAPTIME, val_u> timedata; // vector it
	std::vector<val_u> timedata;
	void get_index( t_TAPTIME when, t_TAPINT &prev, t_TAPINT &next );
};

// maybe better to have the vector / map over time in Data itself
// let timestamp be part of val_u, or just use it as the key in a map
// type conversion in client would be messy anyway, right?
// may have value store its own type

// may be better to pass on entire TimeData to interpolation method?
class Template2;

//-----------------------------------------------------------------------------
// name: class Snapshot
// desc: contains parameter info
//-----------------------------------------------------------------------------
class Snapshot 
{
public:
	Snapshot( Template2 * parent );
	~Snapshot();
	void add_param( std::string name, t_TAPUINT type );
	void set_param( std::string name, t_TAPTIME when, void * value );
	val_u get_param( std::string name, t_TAPTIME when, val_u (*interp_ptr)( val_u, val_u, t_TAPTIME ) = NULL ); 
	TimeData get_param_series( std::string name );
	void debug(); // print
	// include template or audio info

protected:
	TimeData * retrieve_param_data( std::string name );
	std::map<std::string, TimeData> params;
	Template2 * parent;
};


//-----------------------------------------------------------------------------
// name: class Template2
// desc: perhaps the new Template
//-----------------------------------------------------------------------------
class Template2
{
// functions
public:
	Template2(); // create default snapshot among other things
	~Template2(); // delete snapshots?
	virtual void play( AudioBus * bus ) {} // use default params
	virtual void play( AudioBus * bus, Snapshot * snap ) {} // which snapshot params to use
	virtual void stop() {} // stop default
	virtual void stop( Snapshot * snap ) {} // stop given snapshot
	virtual void capture_still() {} // save current parameters as a new snapshot
	virtual void capture_video() {} // save current parameter timeseries as a new snapvideo

// data
public:
	std::vector<Snapshot *> snaps; // snaps[0] is default
	
protected:
	std::vector<Template2 *> playlist; // copies to play
};


//-----------------------------------------------------------------------------
// name: struct Template
// desc: a template (or a script) for synthesis
//-----------------------------------------------------------------------------

static time_t next_id = 1; // odd values for all templates except those that 
                           // are loaded, to avoid conflicts

struct Template : public AudioSrc
{
    int type;
    std::string name;
    std::string typestr;
    time_t id; 
    
    // transformations (set once per instance for some types)
    double time_stretch;
    double freq_warp;

    // parameters that can be changed dynamically
    double gain;
    double pan;
    double periodicity;
    double density;

    // quantizations
    std::vector<t_TAPTIME> timetable;
    std::vector<double> pitchtable; 

    // which
    enum { TIME_STRETCH = 0, FREQ_WARP, GAIN, PAN, PERIODICITY, DENSITY, LOOP_ME,
           PERCENTAGE, K, STOPLEVEL, STARTLEVEL, TOTAL_LEVELS, RANDFLIP, ANCFIRST, 
           R_FREQ_WARP_LOW, R_FREQ_WARP_HIGH, R_TIME_STRETCH_LOW, R_TIME_STRETCH_HIGH,
           R_PAN_LOW, R_PAN_HIGH, R_GAIN_LOW, R_GAIN_HIGH, RANGE_MODE, RANDOM, 
           DURATION, STARTTIME, STOPTIME, BAG_OFFSET /*must be last*/ };

    // polluted bit
    t_TAPBOOL polluted;

	// which bus to play on (-1 = don't care)
	t_TAPINT mybus;
	
	// last bus used (not necessarily == mybus)
	AudioBus * lastbus;

    // functions
    Template( time_t myid = 0 ) : AudioSrc() { this->init( myid ); }
    virtual ~Template() { SAFE_DELETE_ARRAY( features ); }
    virtual void init( time_t myid = 0 );
    virtual void play( AudioBus * bus );
    virtual void stop();
    virtual const char * type_str() const { return typestr.c_str(); };

    virtual Template * copy( bool copyid = false ) const = 0;
    virtual void copy_params( const Template & rhs );
    virtual void recompute();
    virtual void set_param( int which, double value );

    double quantize_time( double value ); 
    double quantize_pitch( double value );
    void read_table( char * filename );  
    void clear_features(); 

    // features
    float * features;

    // analysis parameters, not used in synthesis (to be added)
	std::string ana_orig_filename;
	t_TAPUINT ana_time_start, ana_time_end;
    double ana_freq_low, ana_freq_high;
    // sines
	t_TAPUINT ana_det_numtracks, ana_det_minpoints, ana_det_maxgap;  
    double ana_det_freqsense, ana_det_peaktonoise, ana_det_threshslope, ana_det_threshintercept;
    // groups
	double ana_gr_harm, ana_gr_freq, ana_gr_amp, ana_gr_overlap, ana_gr_on, ana_gr_off, ana_gr_minlen;
    bool ana_gr_used;
	// transients
    double ana_tran_attack, ana_tran_decay, ana_tran_thresh, ana_tran_aging;
	t_TAPUINT ana_tran_mingap, ana_tran_longframe, ana_tran_shortframe, ana_tran_maxlen;
	bool ana_tran_erused;
	// raw
	double ana_raw_rolloff;
	// res has above plus...
	int ana_res_eventtype; // (det? tran?)
	bool ana_res_cliponlyused; 
	// find? script?
};


enum TemplateTypes
{
    TT_DETERMINISTIC = 1,
    TT_TRANSIENT,
    TT_FILE,
    TT_RESIDUE,
    TT_LOOP,
    TT_TIMELINE,
    TT_BAG,
    TT_SCRIPT,
    TT_RAW,

    // valid ranges should fall below 
    TT_ENUM_CEILING
};


//-----------------------------------------------------------------------------
// name: struct Deterministic
// desc: single track-based event
//-----------------------------------------------------------------------------
struct Deterministic : public Template
{
    std::vector<Track *> tracks;
    std::vector<Track *> working;
    Frame cached_sound;
    SynFast syn; // or could be Syn
    AudioSrcFrame * src;

    Deterministic( const std::vector<Track *> & realtracks, time_t myid = 0  );
    ~Deterministic();
    virtual void recompute();
    virtual t_TAPBOOL stick( SAMPLE * buffer, t_TAPUINT num_frames );
    virtual void rewind() { AudioSrc::rewind(); if( src ) src->rewind(); syn.reset(); }
    virtual Template * copy( bool copyid = false ) const;
};


//-----------------------------------------------------------------------------
// name: struct PVCType
// desc: any template that uses PVC
//-----------------------------------------------------------------------------
struct PVCTemp : public Template
{
	AudioSrc * src;

	// for pvc
	PVC * m_pvc;
	int pvc_window_size;
	int pvc_bufsize;
	int pvc_hopsize;
	polar_window * pvc_win[2];
	unsigned int pvc_which;
	SAMPLE * pvc_buffer;    // IO into pv_analyze/synthesize
    SAMPLE * pvc_extras;    // extra samples that are generated because the amount needed is less than pvc_bufsize
    int pvc_numextras;
    bool framedone; // finished reading frame
    SAMPLE * silence;
	
	// functions
	PVCTemp( time_t myid = 0 );
	PVCTemp( const PVCTemp & rhs );
	virtual ~PVCTemp();
	virtual t_TAPBOOL stick( SAMPLE * buffer, t_TAPUINT num_frames );
	virtual void rewind() { AudioSrc::rewind(); src->rewind(); }
	
	virtual void set_pvc_buffers(); // internal
};


//-----------------------------------------------------------------------------
// name: struct Raw
// desc: raw time and frequency range clip, exactly the same as transient now,
//       but transient may become smarter some time. 
//-----------------------------------------------------------------------------
struct Raw : public PVCTemp
{
    Frame sound;
//    AudioSrcFrame * src;

    // for rt_pvc (probably in vain)
/*    PVC * m_pvc;
    int pvc_window_size;
    int pvc_bufsize;
    int pvc_hopsize;
    polar_window * pvc_win[2]; 
    unsigned int pvc_which; 
    SAMPLE * pvc_buffer;    // IO into pv_analyze/synthesize
    SAMPLE * pvc_extras;    // extra samples that are generated because the amount needed is less than pvc_bufsize
    int pvc_numextras;
    bool framedone; // finished reading frame
    SAMPLE * silence;
*/

    // functions...
    Raw( const Frame & event, time_t myid = 0 ); 
    Raw( const Raw & rhs );
//    virtual ~Raw();
//    virtual t_TAPBOOL stick( SAMPLE * buffer, t_TAPUINT num_frames );
//    virtual void rewind() { AudioSrc::rewind(); src->rewind(); }
    virtual Template * copy( bool copyid = false ) const;

//    virtual void set_pvc_buffers(); // internal
};



//-----------------------------------------------------------------------------
// name: struct Transient
// desc: single sound event (raw waveform)
//       if it gets smarter, it can become public Template
//-----------------------------------------------------------------------------
struct Transient : public Raw
{
    // functions...
    Transient( const Frame & event, time_t myid = 0 ); 
    Transient( const Transient & rhs );
    //~Transient();
    virtual Template * copy( bool copyid = false ) const;
};


//-----------------------------------------------------------------------------
// name: struct File
// desc: any (sound) file!
//-----------------------------------------------------------------------------
struct File : public PVCTemp
{
    std::string filename;
//    AudioSrcFile * src; 
    t_TAPBOOL goodtogo;

    File( const std::string & path, time_t myid = 0 );
	virtual ~File() { BB_log( BB_LOG_FINE, "Deleting file template" ); SAFE_DELETE( src ); }
//    virtual t_TAPBOOL stick( SAMPLE * buffer, t_TAPUINT num_frames );
    virtual void rewind() { AudioSrc::rewind(); src->rewind(); }
    virtual Template * copy( bool copyid = false ) const;
};


//-----------------------------------------------------------------------------
// name: struct Residue
// desc: background
//-----------------------------------------------------------------------------
struct Residue : public Template
{
    Treesynth * ts;
    TreesynthIO * tsio;
    AudioSrcEliot * src;
    bool lefttree;
    bool shutup;
    int total_levels; // actual number of levels in current tree
    int requested_levels; // number of levels requested by user (file may not have enough data for that many levels)
    XThread * thread;
    XMutex mutex;
    bool thread_done;
    
    Residue( Treesynth * ts_, TreesynthIO * tsio_, time_t myid = 0 ) : Template( myid )
    { ts = ts_; tsio = tsio_; shutup = false; lefttree = false;
      src = new AudioSrcEliot( tsio ); src->m_delete = FALSE; 
      type = TT_RESIDUE; typestr = "background"; thread = NULL; thread_done = true;
      total_levels = ts_->tree->getLevels(); requested_levels = total_levels;  }
    virtual ~Residue();
    virtual t_TAPBOOL stick( SAMPLE * buffer, t_TAPUINT num_frames );
    virtual Template * copy( bool copyid = false ) const;
    void copy_params( const Template & rhs );
    virtual void stop() { shutup = true; Template::stop(); }
    virtual void set_param( int which, double value );
};


//-----------------------------------------------------------------------------
// name: struct LoopTime
// desc: something that returns waiting time using some distribution, 
//       given an average waiting time
//-----------------------------------------------------------------------------
struct LoopTime
{
    virtual ~LoopTime() { }
    virtual double next_interval( double mean ) = 0;
    virtual LoopTime * clone() const = 0;
    XMutex instmutex; 
};


//-----------------------------------------------------------------------------
// name: struct Poisson
// desc: take a guess
//-----------------------------------------------------------------------------
struct Poisson : public LoopTime
{
    static Poisson * instance()
    { if( !our_fish ) our_fish = new Poisson; return our_fish; }
    virtual double next_interval( double mean )
    { return nextExponential( mean ); }

    static Poisson * our_fish;
    static double nextFakePoisson( double lambda );
    static int nextPoisson( double lambda );
    static double nextExponential( double one_over_lambda );
    virtual LoopTime *clone() const { return new Poisson; }
};


//-----------------------------------------------------------------------------
// name: struct Periodic
// desc: identity
//-----------------------------------------------------------------------------
struct Periodic : public LoopTime
{
    static Periodic * instance()
    { if( !our_periodic ) our_periodic = new Periodic; return our_periodic; }

    static Periodic * our_periodic;
    virtual double next_interval( double mean ) { return mean; }
    virtual LoopTime * clone() const { return new Periodic; }
};


//-----------------------------------------------------------------------------
// name: struct Gaussian
// desc: gaussian from http://www.dspguru.com/howto/tech/wgn2.htm (28/9/05)
//-----------------------------------------------------------------------------
struct Gaussian : public LoopTime
{
    static Gaussian * instance()
    { if( !our_gaussian ) our_gaussian = new Gaussian; return our_gaussian; }

    static Gaussian * our_gaussian;
    double periodicity;
    double std; // standard deviation (1 / periodicity)

    virtual double next_interval( double mean ) 
    {  
        // make up some std
        // if periodicity = 0.5, want std =  mean / 4 = mean * 0.5 / 2
        std = mean * (1 - periodicity) / 2;
    
        // generate normal random variable
        double u1, u2, v1, v2, s, x;
        // double y;
        do {
            do {
                u1 = rand() / (double)RAND_MAX;
                u2 = rand() / (double)RAND_MAX;
                v1 = 2 * u1 - 1;
                v2 = 2 * u2 - 1;
                s = v1 * v1 + v2 * v2;
            }
            while( s >= 1 );

            x = sqrt(-2 * log(s) / s) * v1;
            x = mean + std * x;
            // y = sqrt(-2 * log(s) / s) * v2;
            // y = mean + std * y;
        }
        while( x < 0 );

        return x;
    }

    virtual LoopTime * clone() const { return new Gaussian; }
};


//-----------------------------------------------------------------------------
// name: struct PerryRand
// desc: more made up math? (absolutely) (mean is no longer the mean, probably)
//-----------------------------------------------------------------------------
struct PerryRand : public LoopTime
{
    static PerryRand * instance()
    { if( !our_prand ) our_prand = new PerryRand; return our_prand; }

    static PerryRand * our_prand;
    double periodicity;

    LoopTime * poisson;
    LoopTime * periodic;

    virtual double next_interval( double mean ) 
    {  
        assert( periodicity >= 0 && periodicity <= 1 );
        poisson = Poisson::instance();
        periodic = Periodic::instance();

        return periodicity * periodic->next_interval( mean ) + (1 - periodicity) * poisson->next_interval( mean );
    }

    virtual LoopTime * clone() const { return new PerryRand; }
};


//-----------------------------------------------------------------------------
// name: struct FunkyRand
// desc: more made up math? (absolutely) (mean is no longer the mean, probably)
//-----------------------------------------------------------------------------
struct FunkyRand : public LoopTime
{
    static FunkyRand * instance()
    { if( !our_rand ) our_rand = new FunkyRand; return our_rand; }

    static FunkyRand * our_rand;
    double periodicity;

    virtual double next_interval( double mean ) 
    {  
        assert( periodicity > 0 && periodicity < 1 );
        double min, max, range, r, answer; 
        
        // used
        min = mean * periodicity;
        max = mean + (mean - min);
        range = max - min;
        r = rand() / (double)RAND_MAX;
        answer = min + r * range;

        //fprintf( stderr, "%f\n", answer );
        return answer;
    }
    virtual LoopTime * clone() const { return new FunkyRand; }
};


//-----------------------------------------------------------------------------
// name: struct LoopTemplate
// desc: script for looping another template using a probability distribution
//       - lambda for poisson, in this case
//-----------------------------------------------------------------------------
struct LoopTemplate : public Template
{
    LoopTemplate( const Template & orig, time_t myid = 0 ) 
        : Template( myid ) { temp = orig.copy();
                       until_next = 0;
                       acc_buffer = NULL; arg_buffer = NULL; 
                       acc_buffer_size = 0; dist = NULL;
                       type = TT_LOOP; typestr = "loop";
                       rand_freq_warp[0] = orig.freq_warp/2; rand_freq_warp[1] = orig.freq_warp*2;
                       rand_time_stretch[0] = orig.time_stretch/2; rand_time_stretch[1] = orig.time_stretch*2;
                       rand_pan[0] = orig.pan/2; rand_pan[1] = orig.pan*2;
                       rand_gain[0] = orig.gain/2; rand_gain[1] = orig.gain*2; 
                       random = 2.0;}
    virtual ~LoopTemplate();
    Template * temp;
    LoopTime * dist;
    t_TAPUINT until_next;  // samples until next event

    // ranges for randomizing 
    //(rand_<x>[0] has the minimum value for <x> and rand_<x>[1] has the maximum value)
    double rand_freq_warp[2];
    double rand_time_stretch[2];
    double rand_pan[2];
    double rand_gain[2];
    double random; // randomization factor; may not be needed, depending on how RANGE_MODE works

    std::vector<Template *> copies; // still generating
    std::vector<Template *> active; // temp
    SAMPLE * acc_buffer; // to accumulate the audio from copies
    SAMPLE * arg_buffer; // temp
    t_TAPUINT acc_buffer_size; // size of buffer

    virtual t_TAPBOOL stick( SAMPLE * buffer, t_TAPUINT num_frames );
    virtual void rewind() { 
        AudioSrc::rewind();
        until_next = 0; 
        for( int i = 0; i < copies.size(); i++ )
        {
            SAFE_DELETE(copies[i]);
        }
        copies.clear();
    }
    virtual Template * copy( bool copyid = false ) const;
    virtual void copy_params( const Template & rhs );
    virtual void set_param( int which, double value );
    virtual void recompute();
    void replace_dist_with( LoopTime * lt ) { delete dist; dist = lt; }
    void randomize( Template * victim );
};



//-----------------------------------------------------------------------------
// name: struct Instance
// desc: instance of a template (on a timeline)
//-----------------------------------------------------------------------------
struct Instance
{
    UI_Template * ui_temp; // the template
    t_TAPTIME start_time;  // local - from start of timeline? (yes.)
    t_TAPTIME end_time; // maybe not
    float y_offset; 

    Instance( UI_Template * u, t_TAPTIME st, t_TAPTIME et = 0, float _ = 0.0f )
    {
        ui_temp = u;
        start_time = st;
        end_time = et;
        y_offset = _; 
    }

    Instance( )
    {
        ui_temp = NULL;
        start_time = 0; 
        end_time = 0; 
        y_offset = 0;
    }
};


//-----------------------------------------------------------------------------
// name: struct Timeline
// desc: time-sorted list of templates over a duration, can be looped
//-----------------------------------------------------------------------------
struct Timeline : public Template
{
    std::vector<Instance> instances;
    int loop;  // loop? -1 -> infinite, 0 -> no, otherwise loop == # of loops
    t_TAPTIME duration;
    t_TAPTIME dur_pos;
    t_TAPTIME until_next;
    unsigned long curr_index;
    t_TAPTIME starttime;
    t_TAPTIME stoptime;
    // unsigned long duration;
    // unsigned long dur_pos;
    // unsigned long until_next;
    
    std::vector<Template *> copies; // still generating
    std::vector<Template *> active; // temp
    SAMPLE * acc_buffer; // to accumulate the audio from copies
    SAMPLE * arg_buffer; // temp
    t_TAPUINT acc_buffer_size; // size of buffer

    // now butter
    UI_Element * now_butter; 

    Timeline( t_TAPTIME dur, time_t myid = 0 );
    virtual ~Timeline();
    virtual void place( UI_Template * ui_temp, float where, float y_offset );
    virtual void remove( UI_Template * ui_temp );
    virtual t_TAPBOOL stick( SAMPLE * buffer, t_TAPUINT num_frames );
    virtual void rewind();
    virtual void stop();
    virtual Template * copy( bool copyid = false ) const;
    virtual void copy_params( const Template & rhs );
    void stop_copies();
};


//-----------------------------------------------------------------------------
// name: struct Marble
// desc: marble in a bag of marbles
//-----------------------------------------------------------------------------
struct Marble 
{
    bool hasplayed;
    bool playonce; 
    t_TAPUINT likelihood;
    double random; 
    bool changeable; // whether controls can be changed

    UI_Template * ui_temp; 

    // ranges for randomizing 
    //(rand_<x>[0] has the minimum value for <x> and rand_<x>[1] has the maximum value)
    double rand_freq_warp[2];
    double rand_time_stretch[2];
    double rand_pan[2];
    double rand_gain[2];
    
    Marble( UI_Template * u, t_TAPUINT weight, bool norepeat, double randfac );
    Marble( UI_Template * u );
    void set_ranges( const Template * victim );
    void randomize( Template * victim );
};


//-----------------------------------------------------------------------------
// name: struct BagTemplate
// desc: bag of marbles
//-----------------------------------------------------------------------------
struct BagTemplate : public Template
{
    std::vector<Marble> marbles; 
    LoopTime * dist; 
    t_TAPUINT weightsum; 
    static int nctrls;  // ui elements / controls needed per marble 
    
    std::vector<Template *> copies; // still generating
    std::vector<Template *> active; // temp
    SAMPLE * acc_buffer; // to accumulate the audio from copies
    SAMPLE * arg_buffer; // temp
    t_TAPUINT acc_buffer_size; // size of buffer
    t_TAPTIME until_next; // samples until next event
    XMutex marblemutex; // mutex for protecting access to marbles in template removal and stick
    XMutex copiesmutex; // mutex for protecting access to copies, in same functions
                        // (currently not used, since copies is deleted only if it stops, which won't
                        //  happen automatically for ongoing templates that remove deals with)
    
    BagTemplate( time_t myid = 0 ); 
    virtual ~BagTemplate(); 
    virtual Marble * insert( UI_Template * ui_temp ); 
    virtual void remove( UI_Template * ui_temp ); 
    virtual t_TAPBOOL stick( SAMPLE * buffer, t_TAPUINT num_frames );
    virtual void rewind(); 
    virtual void stop();
    virtual Template * copy( bool copyid = false ) const;
    virtual void copy_params( const Template & rhs );
    virtual void set_param( int which, double value );
    virtual void recompute();
    void replace_dist_with( LoopTime * lt ) { delete dist; dist = lt; }
};



//-----------------------------------------------------------------------------
// name: struct MultiEvent
// desc: events comprised of other events 
//       (but it only needs templates, not uitemplates. hmm. figure out later.)
//-----------------------------------------------------------------------------
/*struct MultiEvent : public Timeline
{
    MultiEvent( t_TAPTIME dur, time_t myid = 0 ) : Timeline( dur, myid ) {};
    virtual ~MultiEvent();
    virtual void remove( UI_Template * ui_temp ) { /* can't remove */ /*};
};*/



//-----------------------------------------------------------------------------
// name: struct Library
// desc: collection of templates
//-----------------------------------------------------------------------------
struct Library
{
    static Library * instance();
    static Library * matches();
    ~Library();

	// add template to library (and return new vector element)
    UI_Template * add( Template * temp )
    {
        UI_Template * ui_temp = new UI_Template;
        ui_temp->core = temp;
        if( temp->type == TT_TIMELINE || temp->type == TT_BAG )
            ui_temp->backup = temp;
        else
            ui_temp->backup = temp->copy( true );
        ui_temp->id = IDManager::instance()->getPickID();
        templates.push_back( ui_temp );
		return ui_temp;
    }

	// return library size
    t_TAPUINT size()
    {
        return templates.size();
    }

	// remove ui_template from library
    t_TAPBOOL remove( UI_Template * ui_temp )
    {
        if( !ui_temp )
            return FALSE;
        if( !ui_temp->dummies.empty() )
            return FALSE;
        for( int t = 0; t < templates.size(); t++ )
        {
            if( templates[t] == ui_temp )
            {
                templates.erase( templates.begin() + t );
                IDManager::instance()->freePickID( ui_temp->id ); // doesn't do much, but still
                SAFE_DELETE( ui_temp );
                break;
            }
        }
        return TRUE;
    }

	// clear templates
    t_TAPBOOL clear()
    {
        t_TAPBOOL okay = TRUE;
        while( templates.size() > 0 )
            okay = remove( templates[0] ) && okay;
        return okay;
    }

	// check whether library already has at least one template with that id
    // if so, return main associated ui_template, otherwise return NULL
	UI_Template * hasID( Template * tmp ) 
    {
        UI_Template * yes = NULL;
        for( int m = 0; m < templates.size(); m++ )
            if( templates[m]->core->id == tmp->id )
            {
                yes = templates[m]->orig;
                break;
            }
        return yes;
    }

	// list templates of the specified types
    std::vector<UI_Template *> search_type( int ntypes, int types [] )
    {
        std::vector<UI_Template *> tmp;
        UI_Template * ui_temp;

        for( int i = 0; i < templates.size(); i++ )
        {
            ui_temp = templates[i]; // get template
            if( ui_temp->orig == ui_temp ) // not a dummy
            {
                for( int n = 0; n < ntypes; n++ ) // check type
                {
                    if( ui_temp->core->type == types[n] )
                    {
                        tmp.push_back( ui_temp );
                        break;
                    }
                }
            }
        }
        return tmp;
    }

	// return templates whose name includes the given string
    std::vector<UI_Template *> search_name( std::string namelet )
    {
        std::vector<UI_Template *> tmp; 
        UI_Template * ui_temp;
        
        for( int i = 0; i < templates.size(); i++ )
        {
            ui_temp = templates[i];
            if( ui_temp->core->name.find( namelet ) != std::string::npos )
                tmp.push_back( ui_temp );
        }
        return tmp;
    }

	// return template whose name is exactly the given string
    std::vector<UI_Template *> search_name_exact( std::string name )
    {
        std::vector<UI_Template *> tmp; 
        UI_Template * ui_temp;
        
        for( int i = 0; i < templates.size(); i++ )
        {
            ui_temp = templates[i];
            if( ui_temp->core->name == name )
                tmp.push_back( ui_temp );
        }
        return tmp;
    }

    std::vector<UI_Template *> templates;

protected:
    Library() { }
    static Library * our_instance;
    static Library * our_matches; 
};


//-----------------------------------------------------------------------------
// name: class Writer
// desc: help you write out a template's information to file
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// name: class Reader 
// desc: read stuff Writer wrote
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// name: struct Writer
// desc: help you write out a template's information to file
//-----------------------------------------------------------------------------
struct Writer
{
    std::ofstream ofile;

    bool open( char * filename );
    void close();

    // templates
    int write_template( const Template * tempt );
    int write_det( const Deterministic * debt );
    int write_trans( const Transient * trance );
    int write_raw( const Raw * war );
    int write_file( const File * phial );
    int write_res( const Residue * race );
    int write_loop( const LoopTemplate * hole );
    int write_tl( const Timeline * teal );
    int write_bag( const BagTemplate * bat ); 

    // other things (parts of templates)
    int write_tracks( const std::vector<Track *> &event );
    int write_frame( const Frame &frame );
};


//-----------------------------------------------------------------------------
// name: struct Reader 
// desc: read stuff Writer wrote
//-----------------------------------------------------------------------------
struct Reader
{
    // internal
protected:
    std::ifstream ifile;

public:
    bool open( char * filename );
    void close();

    // templates
    Template * read_template();
    Deterministic * read_det( time_t myid = 0 );
    Transient * read_trans( time_t myid = 0 );
    Raw * read_raw( time_t myid = 0 );
    File * read_file( time_t myid = 0 );
    Residue * read_res( time_t myid = 0 );
    LoopTemplate * read_loop( time_t myid = 0 );
    Timeline * read_tl( time_t myid = 0 );
    BagTemplate * read_bag( time_t myid = 0 ); 

    // other things (parts of templates)
    int read_tracks( std::vector<Track *> & event );
    int read_frame( Frame & frame );
};


//-----------------------------------------------------------------------------
// name: TemplateGrabber
// desc: ...
//-----------------------------------------------------------------------------
struct TemplateGrabber
{
public:
    TemplateGrabber( long hint_num_samples );
    t_TAPBOOL grab( Template * temp, Frame * out, long max_num_samples );
    t_TAPBOOL grab( Template * temp, Frame * out );

protected:
    long m_hint_num_samples;
};


#endif


/*

  Notes transcribed

  * Analysis: individual templates

  * Synthesis: snapshots are trajectories
			   library can show both (templates and snapshots, different symbols)
			   snapshot button saves trajectory since last record (or beginning if this is the first)

  - Template has snapshots, which have template copies? (just pointers would cause problems in playing multiple snapshots)
  - Template holds audio info + snapshots, snapshots have audio copies?
  - Template handles concurrency + copying?

  In latter case, snapshot points to owner template
  snapshot may have a temporary id while it's in memory
  snapshot's play calls template's play, which determines what to do
	keep track of copies in memory and which snapshot is using which copy
	if there is a free (stopped) copy lying around, assign it to the snapshot
	otherwise make a new copy
	but it's weird for a template to store copies of itself... may be cleaner to separate audio info after all
	but that's one more layer of abstraction too
	oh, but template copy should not copy snapshots or playlist!

  or always make a copy before playing?

  but making copies also complicates the real-time parameter manipulation

  is it better for each snapshot to have its own audio info? but that is the crux of the info in the template, so 
  not memory efficient

  abstraction layers:

  Template : AudioSrc
	name, type
	Audio info : AudioSrc? 
	  Deterministic : audio info
		 AudioSrc whatever
	     relevant data
	  Transient : audio info
	  .
	  .
	  .
	Snapshots
	  map <String, TimeData>
		TimeData maps <t_TAPTIME, val_u> (double check comparisons for t_TAPTIME (double))
		  val_u has data fields
	list of audio info copies <=> snapshots assignments (playlist)

  or ...

  get rid of Audio info layer
  Template : AudioSrc
	audio info directly available here
	Snapshots
	list of copies of self <=> snapshots assignments (playlist)

  Snapshot can point to owner Template + currently playing copy
  Garbage collection?

  Advice: Make interfacing between components loosely coupled, like having mediating classes, for flexibility
  (eg between sliders and parameter values)

*/
