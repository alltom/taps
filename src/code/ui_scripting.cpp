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
// name: ui_scripting.cpp
// desc: scripting "support"
//
// authors: Ananya Misra (amisra@cs.princeton.edu)
//          Ge Wang (gewang@cs.princeton.edu)
//          Perry R. Cook (prc@cs.princeton.edu)
// after EST. January 14, 2005, 8:37 p.m. Friday
//-----------------------------------------------------------------------------

#ifndef __TAPS_SCRIPTING_ENABLE__

// nothing

#else // scripting enabled

#include "chuck_def.h"
#include "ui_scripting.h"
#include "ui_synthesis.h"
#include "ui_control.h"
#include "util_thread.h"
#include "util_network.h"
#include "chuck_errmsg.h"
#include "chuck_globals.h"
#include "chuck_vm.h"
#include "chuck_compile.h"
#include "chuck_bbq.h"

#ifndef __PLATFORM_WIN32__
  #define CHUCK_THREAD pthread_t
  #include <pthread.h>
  #include <unistd.h>
#else 
  #define CHUCK_THREAD HANDLE
  #include <direct.h>
#endif

// static variables
ScriptEngine * ScriptCentral::our_engine = NULL;
std::map<t_TAPUINT, Scriptor *> ScriptCentral::shred_to_scriptor;

// thread
static CHUCK_THREAD g_tid_reply;
static CHUCK_THREAD g_tid_vm;
// reply function
static void * reply_sucker( void * a );
// vm function
static void * vm_runner( void * a );
// api
static t_TAPBOOL init_api( Chuck_Env * env );


//-----------------------------------------------------------------------------
// name: startup()
// desc: global scripting startup
//-----------------------------------------------------------------------------
t_TAPBOOL ScriptCentral::startup()
{
    // set chuck log level
    EM_setlog( CK_LOG_SYSTEM );

    // allocate engine
    our_engine = new ScriptEngine();
    // init
    our_engine->init();

    return TRUE;
}


//-----------------------------------------------------------------------------
// name: shutdown()
// desc: global scripting shutdown
//-----------------------------------------------------------------------------
t_TAPBOOL ScriptCentral::shutdown()
{
    // deallocate engine
    SAFE_DELETE( our_engine );

    return TRUE;
}


//-----------------------------------------------------------------------------
// name: engine()
// desc: ...
//-----------------------------------------------------------------------------
ScriptEngine * ScriptCentral::engine()
{
    assert( our_engine != NULL );
    return our_engine;
}


//-----------------------------------------------------------------------------
// name: get_scriptor
// desc: get scriptor associated with given shred id
//-----------------------------------------------------------------------------
Scriptor * ScriptCentral::get_scriptor( t_TAPUINT shred_id )
{
	std::map<t_TAPUINT, Scriptor *>::iterator iter = shred_to_scriptor.find( shred_id );
	if( iter != shred_to_scriptor.end() )
		return iter->second;
	else
	{
		BB_log( BB_LOG_FINE, "ScriptCentral: Could not get script for shred id %i", shred_id );
		return NULL;
	}
}


//-----------------------------------------------------------------------------
// name: find_scriptor
// desc: find scriptor associated with given shred (or its parent, grandparent, etc)
//-----------------------------------------------------------------------------
Scriptor * ScriptCentral::find_scriptor( Chuck_VM_Shred * shred )
{
	Scriptor * script = NULL;
	while( script == NULL && shred != NULL )
	{
		script = get_scriptor( shred->xid );
		shred = shred->parent;
	}
	if( script == NULL )
		BB_log( BB_LOG_WARNING, "ScriptCentral: No script found" );
	return script;
}


//-----------------------------------------------------------------------------
// name: add_scriptor
// desc: add scriptor and shred mapping
//-----------------------------------------------------------------------------
void ScriptCentral::add_scriptor( t_TAPUINT shred_id, Scriptor * script )
{
	std::map<t_TAPUINT, Scriptor *>::iterator iter = shred_to_scriptor.find( shred_id );
	assert( script != NULL );
	assert( iter == shred_to_scriptor.end() );
	// assert doesn't always work so let bad things happen but print a message
	if( iter != shred_to_scriptor.end() ) 
	{
		BB_log( BB_LOG_SYSTEM, "ScriptCentral: Script entry for shred id %i already exists", shred_id );
		return;
	}
	// insert
	shred_to_scriptor[shred_id] = script;
}
	

//-----------------------------------------------------------------------------
// name: remove_scriptor
// desc: remove given scriptor from map (remove map entries by scriptor)
//-----------------------------------------------------------------------------	
void ScriptCentral::remove_scriptor( Scriptor * script )
{
	std::map<t_TAPUINT, Scriptor *>::iterator iter;
	for( iter = shred_to_scriptor.begin(); iter != shred_to_scriptor.end(); )
	{
		if( iter->second == script )
			shred_to_scriptor.erase( iter++ );
		else
			++iter;
	}
}


//-----------------------------------------------------------------------------
// name: remove_shred
// desc: remove map entry by shred
//-----------------------------------------------------------------------------	
void ScriptCentral::remove_shred( t_TAPUINT shred_id )
{
	std::map<t_TAPUINT, Scriptor *>::iterator iter = shred_to_scriptor.find(shred_id);
	if( iter != shred_to_scriptor.end() )
		shred_to_scriptor.erase( iter );
}


//-----------------------------------------------------------------------------
// name: update_shred_to_scriptor
// desc: sometimes shreds are removed in chuck but taps doesn't know
//		 deal with it by going through the list and stopping scriptors as needed
//-----------------------------------------------------------------------------	
void ScriptCentral::update_shred_to_scriptor()
{
	std::map<t_TAPUINT, Scriptor *>::iterator iter;
	for( iter = shred_to_scriptor.begin(); iter != shred_to_scriptor.end(); iter++ )
	{
		t_TAPUINT shred_id = iter->first;
		if( !ScriptCentral::engine()->m_vm->shreduler()->lookup(shred_id) )
			iter->second->m_stop_asap = TRUE;
	}
}


//-----------------------------------------------------------------------------
// name: check_script_running
// desc: sometimes shreds are removed in chuck but taps doesn't know
//		 look up a particular script here (instead of checking all as above)
//		 Assumes each script is matched only to its first shred (not to children)
//-----------------------------------------------------------------------------	
t_TAPBOOL ScriptCentral::check_script_running( Scriptor * script )
{
	// go through map
	std::map<t_TAPUINT, Scriptor *>::iterator iter;
	for( iter = shred_to_scriptor.begin(); iter != shred_to_scriptor.end(); iter++ )
	{
		// if you find the script
		if( iter->second == script )
		{
			// if you can't find its shred in the shreduler, guess it's done
			if( !ScriptCentral::engine()->m_vm->shreduler()->lookup(iter->first) )
				return FALSE;
			// otherwise it's still going
			return TRUE;
		}
	}
	// if you don't find the script at all
	return FALSE;
}


//-----------------------------------------------------------------------------
// name: ScriptEngine()
// desc: ...
//-----------------------------------------------------------------------------
ScriptEngine::ScriptEngine()
    : AudioSrc()
{
    // clear
    m_compiler = NULL;
    m_vm = NULL;
    m_bus = NULL;
    m_current = NULL;

    // flag for audio engine not to delete when done
    m_delete = FALSE;
}


//-----------------------------------------------------------------------------
// name: ~ScriptEngine()
// desc: ...
//-----------------------------------------------------------------------------
ScriptEngine::~ScriptEngine()
{
    // call shutdown
    this->shutdown();
}


//-----------------------------------------------------------------------------
// name: init()
// desc: ...
//-----------------------------------------------------------------------------
t_TAPBOOL ScriptEngine::init()
{
    if( m_vm )
    {
        fprintf( stderr, "[scripting]: already initialized!\n" );
        return FALSE;
    }

    // log
    BB_log( BB_LOG_SYSTEM, "chuck version: %s", CK_VERSION );

    // allocate the vm - needs the type system
    m_vm = g_vm = new Chuck_VM;
    if( !m_vm->initialize(
        FALSE, // no real-time audio internally
        FALSE, // don't halt
        BirdBrain::srate(), // sample rate (doesn't matter here)
        AudioCentral::instance()->m_sig_frames, // buffer size
        8,     // number of buffers (doesn't matter here)
        0,     // dac device (doesn't matter here)
        0,     // adc device (doesn't matter here)
        2,     // output channels
        2,     // input channels
        FALSE  // not blocking
        ) )
    {
        fprintf( stderr, "[chuck]: %s\n", m_vm->last_error() );
        return FALSE;
    }

    // allocate the compiler
    m_compiler = new Chuck_Compiler;
    // initialize the compiler
    if( !m_compiler->initialize( m_vm ) )
    {
        fprintf( stderr, "[chuck]: error initializing compiler...\n" );
        return FALSE;
    }
    // enable dump
    m_compiler->emitter->dump = FALSE;
    // set auto depend
    m_compiler->set_auto_depend( FALSE );
    // objects
    if( !init_api( m_compiler->env ) )
    {
        fprintf( stderr, "[chuck]: unable to initialize pastatree...\n" );
        return FALSE;
    }

    // vm synthesis subsystem - needs the type system
    if( !m_vm->initialize_synthesis( ) )
    {
        fprintf( stderr, "[chuck]: %s\n", m_vm->last_error() );
        return FALSE;
    }
    
    return TRUE;
}


//-----------------------------------------------------------------------------
// name: start_vm()
// desc: ...
//-----------------------------------------------------------------------------
t_TAPBOOL ScriptEngine::start_vm( AudioBus * which )
{
    if( !m_vm )
    {
        fprintf( stderr, "[scripting]: engine::start - no VM present!\n" );
        return FALSE;
    }

    if( vm_is_running() )
    {
        fprintf( stderr, "[scripting]: engine::start - VM already running!\n" );
        return FALSE;
    }

    // log
    BB_log( BB_LOG_SYSTEM, "starting script engine VM..." );

#ifndef __PLATFORM_WIN32__
    pthread_create( &g_tid_vm, NULL, vm_runner, m_vm );
    pthread_create( &g_tid_reply, NULL, reply_sucker, this );
#else
    g_tid_vm = CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE)vm_runner, m_vm, 0, 0 );
    g_tid_reply = CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE)reply_sucker, this, 0, 0 );
#endif

    // wait a bit
    usleep( 100000 );

    // set stop flag
    m_stop_asap = FALSE;

    // if audio bus specified
    if( which )
    {
        BB_log( BB_LOG_SYSTEM, "adding script engine to audio bus..." );
        assert( m_bus == NULL );
        // assign to bus
//		fprintf( stdout, "playing %x (script engine) on %x\n", this, which );
        which->play( this );
        m_bus = which;
    }

    return TRUE;
}


//-----------------------------------------------------------------------------
// name: stop_vm()
// desc: ...
//-----------------------------------------------------------------------------
t_TAPBOOL ScriptEngine::stop_vm()
{
    if( !m_vm )
    {
        fprintf( stderr, "[scripting]: engine::stop - no VM present!\n" );
        return FALSE;
    }

    if( !vm_is_running() )
    {
        fprintf( stderr, "[scripting]: engine::stop - VM not running!\n" );
        return FALSE;
    }

    // log
    BB_log( BB_LOG_SYSTEM, "removing script engine from audio bus..." );
    // remove from bus
    m_stop_asap = TRUE;

    // if on audio bus 
    if( m_bus )
    {
        // wait done
        while( m_done == FALSE )
            usleep( 10000 );
        // gotten off bus
        m_bus = NULL;
    }

    BB_log( BB_LOG_SYSTEM, "stoping script engine VM..." );
    // stop the VM
    m_vm->stop();
    // wait a bit
    usleep( 50000 );

    BB_log( BB_LOG_SYSTEM, "scripting engine stopped." );
    
    return TRUE;
}


//-----------------------------------------------------------------------------
// name: shutdown()
// desc: ...
//-----------------------------------------------------------------------------
t_TAPBOOL ScriptEngine::shutdown()
{
    // stop first
    if( vm_is_running() )
    {
        stop_vm();
    }

    g_vm = NULL;
    SAFE_DELETE( m_vm );
    SAFE_DELETE( m_compiler );

    // current
    m_current = NULL;

    return TRUE;
}


//-----------------------------------------------------------------------------
// name: reply_sucker()
// desc: ...
//-----------------------------------------------------------------------------
void * reply_sucker( void * a )
{
    Chuck_Msg * msg = NULL;
    ScriptEngine * engine = (ScriptEngine *)a;
    Chuck_VM * vm = engine->m_vm;
    while( engine->m_stop_asap == FALSE )
    {
        msg = vm->get_reply();
        if( !msg ) usleep( 25000 );
        else
        {
            msg->reply( msg );
            delete msg;
        }
    }

    return NULL;
}


//-----------------------------------------------------------------------------
// name: vm_runner()
// desc: ...
//-----------------------------------------------------------------------------
void * vm_runner( void * a )
{
    Chuck_VM * vm = (Chuck_VM *)a;
    // run the vm
    vm->run();

    return NULL;
}


//-----------------------------------------------------------------------------
// name: handle_reply()
// desc: ...
//-----------------------------------------------------------------------------
static void handle_reply( const Chuck_Msg * msg )
{
    Scriptor * scriptor = (Scriptor *)msg->user;
    assert( scriptor->m_waiting_for_id == TRUE && scriptor->m_shred_id == 0 );
    // set id
    scriptor->m_shred_id = msg->replyA;
    scriptor->m_waiting_for_id = FALSE;
}


//-----------------------------------------------------------------------------
// name: vm_is_init()
// desc: ...
//-----------------------------------------------------------------------------
t_TAPBOOL ScriptEngine::vm_is_init()
{
    if( !m_vm ) return FALSE;
    return m_vm->has_init();
}


//-----------------------------------------------------------------------------
// name: vm_is_running()
// desc: ...
//-----------------------------------------------------------------------------
t_TAPBOOL ScriptEngine::vm_is_running()
{
    if( !m_vm ) return FALSE;
    return m_vm->is_running();
}


//-----------------------------------------------------------------------------
// name: stick()
// desc: ...
//-----------------------------------------------------------------------------
t_TAPBOOL ScriptEngine::stick( SAMPLE * buffer, t_TAPUINT num_frames )
{
    if( m_stop_asap )
        return FALSE;

    // compute the samples
    Digitalio::cb2( (char *)buffer, num_frames, m_vm );

    return TRUE;
}


//-----------------------------------------------------------------------------
// name: compile()
// desc: ...
//-----------------------------------------------------------------------------
Scriptor * ScriptEngine::compile( const std::string & filename )
{
    if( !m_compiler )
        return NULL;

    // parse, type-check, and emit
    if( !m_compiler->go( filename.c_str(), NULL ) )
        return NULL;

    // get the code
    Chuck_VM_Code * code = m_compiler->output();
    // name it
    code->name += string(mini(filename.c_str()));

    // make a Scriptor
    Scriptor * scriptor = new Scriptor( code );
    scriptor->m_filename = filename;

    return scriptor;
}


//-----------------------------------------------------------------------------
// name: compile()
// desc: ...
//-----------------------------------------------------------------------------
Chuck_VM_Code * ScriptEngine::recompile( const std::string & filename )
{
    if( !m_compiler ) 
        return NULL; 
    
    // parse, type-check, and emit
    if( !m_compiler->go( filename.c_str(), NULL ) )
        return NULL;

    // get the code
    Chuck_VM_Code * code = m_compiler->output();
    // name it
    code->name += string(mini(filename.c_str()));
    // return it
    return code;
}


//-----------------------------------------------------------------------------
// name: run()
// desc: ...
//-----------------------------------------------------------------------------
t_TAPBOOL ScriptEngine::run( Scriptor * scriptor, t_TAPBOOL immediate )
{
    if( scriptor->m_code == NULL )
    {
        fprintf( stderr, "[scripting]: scriptor has no code to run!\n" );
        return FALSE;
    }

    // make sure scriptor has no id
    assert( scriptor->m_shred_id == 0 && scriptor->m_waiting_for_id == FALSE );
    scriptor->m_waiting_for_id = TRUE;

    // make a new msg - deleted elsewhere (we hope)
    Chuck_Msg * msg = new Chuck_Msg;

    // set the flags for the command
    msg->type = MSG_ADD;
    msg->code = scriptor->m_code;
    msg->user = scriptor;
    msg->reply = handle_reply;
    
    // change directory 
    std::string directory = BirdBrain::getpath( scriptor->m_filename.c_str() );
    if( directory != "" ) BirdBrain::goto_dir( directory );

    // mode
	immediate = TRUE;
    if( immediate ) 
		m_vm->process_msg( msg );
    else 
		m_vm->queue_msg( msg, 1 );

	// wait
	while( scriptor->m_waiting_for_id == TRUE )
		usleep( 10000 );

    return TRUE;
}


//-----------------------------------------------------------------------------
// name: remove()
// desc: ...
//-----------------------------------------------------------------------------
t_TAPBOOL ScriptEngine::remove( Scriptor * script, t_TAPBOOL immediate )
{
    // make sure it's running or waiting for id
    if( script->m_shred_id == 0 && script->m_waiting_for_id == FALSE )
    {
        fprintf( stderr, "[scripting]: scriptor is running; cannot be removed!\n" );
        return FALSE;
    }

    // if waiting
    while( script->m_waiting_for_id )
        usleep( 5000 );

    // make sure has id
    assert( script->m_shred_id != 0 );

    // remove it
    t_TAPBOOL ret = remove( script->m_shred_id, immediate );
    script->m_shred_id = 0;

    return ret;
}


//-----------------------------------------------------------------------------
// name: remove()
// desc: ...
//-----------------------------------------------------------------------------
t_TAPBOOL ScriptEngine::remove( t_TAPUINT id, t_TAPBOOL immediate )
{
    // make a new msg - deleted elsewhere (we hope)
    Chuck_Msg * msg = new Chuck_Msg;

    // set the flags for the command
    msg->type = MSG_REMOVE;
    msg->param = id;

    // mode
    if( immediate ) m_vm->process_msg( msg );
    else m_vm->queue_msg( msg, 1 );

    return TRUE;
}


//-----------------------------------------------------------------------------
// name: remove()
// desc: ...
//-----------------------------------------------------------------------------
t_TAPBOOL ScriptEngine::remove_all( t_TAPBOOL immediate )
{
    // make a new msg - deleted elsewhere (we hope)
    Chuck_Msg * msg = new Chuck_Msg;

    // set the flags for the command
    msg->type = MSG_REMOVEALL;

    // mode
    if( immediate ) m_vm->process_msg( msg );
    else m_vm->queue_msg( msg, 1 );

    return TRUE;
}


//-----------------------------------------------------------------------------
// name: current()
// desc: ...
//-----------------------------------------------------------------------------
Scriptor * ScriptEngine::current()
{
    return m_current;
}


//-----------------------------------------------------------------------------
// name: Scriptor()
// desc: ...
//-----------------------------------------------------------------------------
Scriptor::Scriptor( Chuck_VM_Code * code, time_t myid )
    : Template( myid )
{
    // set
    m_code = code;
    // add reference
    m_code->add_ref();
    // set type
    type = TT_SCRIPT;
    // set type string
    typestr = "script";
    // shred it
    m_shred_id = 0;
    // not waiting for id yet
    m_waiting_for_id = FALSE;
    // has not been initialized
    m_script_init = FALSE;
    // library
    m_library_templates = new map<Template *, Template *>;
}


//-----------------------------------------------------------------------------
// name: ~Scriptor()
// desc: ...
//-----------------------------------------------------------------------------
Scriptor::~Scriptor()
{
    // if playing
    if( m_script_init == TRUE )
    {
        // stop
        stop();
    }

    SAFE_RELEASE( m_code );

    // TODO: clean up library templates
    map<Template *, Template *>::iterator iter;
    for( iter = m_library_templates->begin(); iter != m_library_templates->end(); iter++ )
    {
        // do something
    }

    SAFE_DELETE( m_library_templates );

	// clear ui elements
	int i;
	for( i = 0; i < m_elements.size(); i++ )
		IDManager::instance()->freePickID( m_elements[i]->id );
	for( i = 0; i < m_elements.size(); i++ )
	{
		SAFE_DELETE( m_elements[i] );
	}
	m_elements.clear();
}


//-----------------------------------------------------------------------------
// name: stick()
// desc: ...
//-----------------------------------------------------------------------------
t_TAPBOOL Scriptor::stick( SAMPLE * buffer, t_TAPUINT num_frames )
{
    // first time
    if( m_script_init == FALSE )
    {
		// add
        m_script_init = ScriptCentral::engine()->run( this );
        // make sure (assert doesn't always work)
        assert( m_script_init == TRUE );
		if( m_script_init != TRUE )
			BB_log( BB_LOG_WARNING, "Script not initialized!" );
		// add to shred -> scriptor map
		ScriptCentral::add_scriptor( m_shred_id, this );
    }

    // zero it out
    memset( buffer, 0, sizeof(SAMPLE) * 2 * num_frames );

	// is it done?
	if( !ScriptCentral::check_script_running( this ) )
		m_stop_asap = m_done = TRUE;

    // stop
    if( m_stop_asap )
    {
        // other stop code moved to stop() (probably introducing several race condition problems)
        m_script_init = FALSE;
		stop(); // this line may be bad;
        return FALSE;
    }

    return TRUE;
}


//-----------------------------------------------------------------------------
// name: stop()
// desc: ...
//-----------------------------------------------------------------------------
void Scriptor::stop()
{
    // sometimes it needs to be stopped but doesn't get sticked again
    // like if it's on a timeline or something else
    Template::stop();
    // remove
    ScriptCentral::engine()->remove( this );
	// remove from shred -> scriptor map
	ScriptCentral::remove_scriptor( this );
	// clear any chui elements, since they will be constructed again
	while( num_elements() > 0 )
		remove_element( element_at( 0 ) );
}


//-----------------------------------------------------------------------------
// name: rewind()
// desc: ...
//-----------------------------------------------------------------------------
t_TAPBOOL Scriptor::rewind()
{
    // do nothing?
	return TRUE;
}


//-----------------------------------------------------------------------------
// name: recompute()
// desc: recompiles
//-----------------------------------------------------------------------------
void Scriptor::recompute()
{
    return; // no use for now since we can't access them files no mo'

    // compile again
    Chuck_VM_Code * new_code = ScriptCentral::engine()->recompile( m_filename ); 
    // if playing, stop (for safety)
    if( m_script_init )
    { 
        stop();
    }
    // release old code
    SAFE_RELEASE( m_code );
    // add new code
    m_code = new_code;
    // add reference
    m_code->add_ref();
}


//-----------------------------------------------------------------------------
// name: copy()
// desc: ...
//-----------------------------------------------------------------------------
Template * Scriptor::copy( bool copyid ) const
{
    // that should do maybe
    Scriptor * script = new Scriptor( m_code, copyid ? id : 0 );
    script->m_filename = this->m_filename;

    return script;
}


//-----------------------------------------------------------------------------
// name: add_element()
// desc: add taps ui / chui element
//-----------------------------------------------------------------------------
t_TAPBOOL Scriptor::add_element( UI_Element * element )
{
	if( element )
	{
		m_elements.push_back( element );
		return TRUE;
	}
	return FALSE;
}


//-----------------------------------------------------------------------------
// name: remove_element()
// desc: remove ui element from scriptor
//-----------------------------------------------------------------------------
t_TAPBOOL Scriptor::remove_element( UI_Element * element )
{
	t_TAPBOOL removed = FALSE;
	for( int i = 0; i < m_elements.size(); i++ )
	{
		if( m_elements[i] == element )
		{
			m_elements.erase( m_elements.begin() + i-- );
			removed = TRUE;
		}
	}
	if( removed && element )
	{
		IDManager::instance()->freePickID( element->id );
		SAFE_DELETE( element );
	}
	return removed;
}


//-----------------------------------------------------------------------------
// name: num_elements()
// desc: return number of ui elements
//-----------------------------------------------------------------------------
t_TAPUINT Scriptor::num_elements()
{
	return m_elements.size();
}


//-----------------------------------------------------------------------------
// name: element_at()
// desc: return element at index i
//-----------------------------------------------------------------------------
UI_Element * Scriptor::element_at( t_TAPUINT i )
{
	if( i < m_elements.size() )
		return m_elements[i]; 
	else
		return NULL;
}


// TapsSynth API
CK_DLL_SFUN( synth_load );
CK_DLL_SFUN( synth_copy );
CK_DLL_SFUN( synth_count );

// template API
CK_DLL_CTOR( template_ctor );
CK_DLL_DTOR( template_dtor );
CK_DLL_MFUN( template_play );
CK_DLL_MFUN( template_play2 );
CK_DLL_MFUN( template_stop );
CK_DLL_MFUN( template_playing );
CK_DLL_MFUN( template_read ); // to be removed?
CK_DLL_MFUN( template_readFromFile );
CK_DLL_MFUN( template_readFromFile2 );
CK_DLL_MFUN( template_readFromLibrary );
CK_DLL_MFUN( template_readFromLibrary2 );
CK_DLL_MFUN( template_load );
CK_DLL_MFUN( template_rewind );
CK_DLL_MFUN( template_set_bus );
CK_DLL_MFUN( template_get_bus );
CK_DLL_MFUN( template_set_freqWarp );
CK_DLL_MFUN( template_get_freqWarp );
CK_DLL_MFUN( template_set_timeStretch );
CK_DLL_MFUN( template_get_timeStretch );
CK_DLL_MFUN( template_set_pan );
CK_DLL_MFUN( template_get_pan );
CK_DLL_MFUN( template_set_gain );
CK_DLL_MFUN( template_get_gain );
CK_DLL_MFUN( template_set_periodicity );
CK_DLL_MFUN( template_get_periodicity );
CK_DLL_MFUN( template_set_density );
CK_DLL_MFUN( template_get_density );
CK_DLL_MFUN( template_set_random );
CK_DLL_MFUN( template_get_random );
CK_DLL_MFUN( template_get_ready );
CK_DLL_MFUN( template_close ); // to be removed
CK_DLL_MFUN( template_release );

// bus API
CK_DLL_SFUN( bus_volume );
CK_DLL_SFUN( bus_volume2 );
CK_DLL_SFUN( bus_reverb );
CK_DLL_SFUN( bus_reverb2 );
CK_DLL_SFUN( bus_pan );
CK_DLL_SFUN( bus_pan2 );

// small gui API
CK_DLL_CTOR( chui_ctor );
CK_DLL_DTOR( chui_dtor );
CK_DLL_MFUN( chui_setposition );
CK_DLL_MFUN( chui_setvalue );
CK_DLL_MFUN( chui_getvalue );
CK_DLL_MFUN( chui_setsize );
CK_DLL_MFUN( chui_getsize );
CK_DLL_MFUN( chui_setlabel );
CK_DLL_MFUN( chui_getlabel );
CK_DLL_CTOR( chuisl_ctor );
CK_DLL_DTOR( chuisl_dtor );
CK_DLL_MFUN( chuisl_init );
CK_DLL_MFUN( chuisl_setori );
CK_DLL_MFUN( chuisl_getori );
CK_DLL_MFUN( chuisl_setlen );
CK_DLL_MFUN( chuisl_getlen );
CK_DLL_CTOR( chuibt_ctor );
CK_DLL_DTOR( chuibt_dtor );
CK_DLL_CTOR( chuifl_ctor );
CK_DLL_DTOR( chuifl_dtor );


static t_TAPUINT template_offset_data = 0;
static t_TAPUINT template_offset_flag = 0;
static t_TAPUINT chui_offset_data = 0;
static t_TAPUINT chuisl_offset_exp = 0;

//-----------------------------------------------------------------------------
// name: init_api()
// desc: ...
//-----------------------------------------------------------------------------
t_TAPBOOL init_api2( Chuck_Env * env );

t_TAPBOOL init_api( Chuck_Env * env )
{
    // make context
    Chuck_Context * context = type_engine_make_context( NULL, "@[internal-tapestrea]" );
    // reset env
    env->reset();
    // load it
    type_engine_load_context( env, context );

    // load
    t_TAPBOOL ret = init_api2( env );

    // clear context
    type_engine_unload_context( env );

    // commit what is in the type checker at this point
    env->global()->commit();

    return ret;
}

t_TAPBOOL init_api2( Chuck_Env * env )
{
    Chuck_DL_Func * func = NULL;
    Chuck_Value * value = NULL;

    // log
    BB_log( BB_LOG_SYSTEM, "importing Tapestrea + ChucK API... " );

    // import
    if( !type_engine_import_class_begin( env, "TapsSynth", "Object",
                                         env->global(), NULL ) )
        return FALSE;

    // add load()
    func = make_new_sfun( "int", "load", synth_load );
    func->add_arg( "string", "name" );
    if( !type_engine_import_sfun( env, func ) ) goto error;

    // add copy()
    func = make_new_sfun( "int", "copy", synth_copy );
    func->add_arg( "string", "name" );
    func->add_arg( "int", "index" );
    func->add_arg( "int", "copies" );
    if( !type_engine_import_sfun( env, func ) ) goto error;

    // add count()
    func = make_new_sfun( "int", "count", synth_count );
    func->add_arg( "string", "name" );
    if( !type_engine_import_sfun( env, func ) ) goto error;

    // end the class import
    type_engine_import_class_end( env );


    // import
    if( !type_engine_import_class_begin( env, "TapsTemp", "Object",
                                         env->global(), template_ctor ) )
        return FALSE;

    // add member
    template_offset_data = type_engine_import_mvar( env, "int", "@data", FALSE );
    if( template_offset_data == CK_INVALID_OFFSET ) goto error;

    // add member
    template_offset_flag = type_engine_import_mvar( env, "int", "@flag", FALSE );
    if( template_offset_flag == CK_INVALID_OFFSET ) goto error;

    // add play()
    func = make_new_mfun( "void", "play", template_play );
    if( !type_engine_import_mfun( env, func ) ) goto error;

    // add play()
    func = make_new_mfun( "void", "play", template_play2 );
    func->add_arg( "int", "which" );
    if( !type_engine_import_mfun( env, func ) ) goto error;

    // add stop()
    func = make_new_mfun( "void", "stop", template_stop );
    if( !type_engine_import_mfun( env, func ) ) goto error;

    // add rewind()
    func = make_new_mfun( "void", "rewind", template_rewind );
    if( !type_engine_import_mfun( env, func ) ) goto error;

    // add playing()
    func = make_new_mfun( "int", "playing", template_playing );
    if( !type_engine_import_mfun( env, func ) ) goto error;

    // add read()
    func = make_new_mfun( "int", "read", template_read );
    func->add_arg( "string", "path" );
    if( !type_engine_import_mfun( env, func ) ) goto error;

    // add readFromFile()
    func = make_new_mfun( "int", "readFromFile", template_readFromFile );
    func->add_arg( "string", "path" );
	func->add_arg( "int", "show" );
    if( !type_engine_import_mfun( env, func ) ) goto error;

	// add readFromFile()
	func = make_new_mfun( "int", "readFromFile", template_readFromFile2 );
	func->add_arg( "string", "path" );
	if( !type_engine_import_mfun( env, func ) ) goto error;

    // add readFromLibrary()
    func = make_new_mfun( "int", "readFromLibrary", template_readFromLibrary );
    func->add_arg( "string", "name" );
    if( !type_engine_import_mfun( env, func ) ) goto error;

    // add readFromLibrary()
    func = make_new_mfun( "int", "readFromLibrary", template_readFromLibrary2 );
    func->add_arg( "string", "name" );
    func->add_arg( "int", "index" );
    if( !type_engine_import_mfun( env, func ) ) goto error;

    // add close()
    func = make_new_mfun( "int", "close", template_close );
    if( !type_engine_import_mfun( env, func ) ) goto error;

	// add release()
	func = make_new_mfun( "int", "release", template_release );
	if( !type_engine_import_mfun( env, func ) ) goto error;

    // add bus()
    func = make_new_mfun( "int", "bus", template_set_bus );
    func->add_arg( "int", "which" );
    if( !type_engine_import_mfun( env, func ) ) goto error;

    // add bus()
    func = make_new_mfun( "int", "bus", template_get_bus );
    if( !type_engine_import_mfun( env, func ) ) goto error;

    // add gain()
    func = make_new_mfun( "float", "gain", template_set_gain );
    func->add_arg( "float", "value" );
    if( !type_engine_import_mfun( env, func ) ) goto error;
    func = make_new_mfun( "float", "gain", template_get_gain );
    if( !type_engine_import_mfun( env, func ) ) goto error;

    // add pan()
    func = make_new_mfun( "float", "pan", template_set_pan );
    func->add_arg( "float", "value" );
    if( !type_engine_import_mfun( env, func ) ) goto error;
    func = make_new_mfun( "float", "pan", template_get_pan );
    if( !type_engine_import_mfun( env, func ) ) goto error;

    // add freqWarp()
    func = make_new_mfun( "float", "freqWarp", template_set_freqWarp );
    func->add_arg( "float", "value" );
    if( !type_engine_import_mfun( env, func ) ) goto error;
    func = make_new_mfun( "float", "freqWarp", template_get_freqWarp );
    if( !type_engine_import_mfun( env, func ) ) goto error;

    // add timeStretch()
    func = make_new_mfun( "float", "timeStretch", template_set_timeStretch );
    func->add_arg( "float", "value" );
    if( !type_engine_import_mfun( env, func ) ) goto error;
    func = make_new_mfun( "float", "timeStretch", template_get_timeStretch );
    if( !type_engine_import_mfun( env, func ) ) goto error;

    // add periodicity()
    func = make_new_mfun( "float", "periodicity", template_set_periodicity );
    func->add_arg( "float", "value" );
    if( !type_engine_import_mfun( env, func ) ) goto error;
    func = make_new_mfun( "float", "periodicity", template_get_periodicity );
    if( !type_engine_import_mfun( env, func ) ) goto error;

    // add density()
    func = make_new_mfun( "float", "density", template_set_density );
    func->add_arg( "float", "value" );
    if( !type_engine_import_mfun( env, func ) ) goto error;
    func = make_new_mfun( "float", "density", template_get_density );
    if( !type_engine_import_mfun( env, func ) ) goto error;

    // add random()
    func = make_new_mfun( "float", "random", template_set_random );
    func->add_arg( "float", "value" );
    if( !type_engine_import_mfun( env, func ) ) goto error;
    func = make_new_mfun( "float", "random", template_get_random );
    if( !type_engine_import_mfun( env, func ) ) goto error;

    // add ready()
    func = make_new_mfun( "int", "ready", template_get_ready );
    if( !type_engine_import_mfun( env, func ) ) goto error;

    // end the class import
    type_engine_import_class_end( env );


    // import
    if( !type_engine_import_class_begin( env, "TapsBus", "Object",
                                         env->global(), template_ctor ) )
        return FALSE;

    // add reverb()
    func = make_new_sfun( "float", "reverb", bus_reverb );
    func->add_arg( "int", "which" );
    func->add_arg( "float", "value" );
    if( !type_engine_import_sfun( env, func ) ) goto error;

    // add reverb()
    func = make_new_sfun( "float", "reverb", bus_reverb2 );
    func->add_arg( "int", "which" );
    if( !type_engine_import_sfun( env, func ) ) goto error;

    // add volume()
    func = make_new_sfun( "float", "volume", bus_volume );
    func->add_arg( "int", "which" );
    func->add_arg( "float", "value" );
    if( !type_engine_import_sfun( env, func ) ) goto error;

    // add volume()
    func = make_new_sfun( "float", "volume", bus_volume2 );
    func->add_arg( "int", "which" );
    if( !type_engine_import_sfun( env, func ) ) goto error;

    // add pan()
    func = make_new_sfun( "float", "pan", bus_pan );
    func->add_arg( "int", "which" );
    func->add_arg( "float", "value" );
    if( !type_engine_import_sfun( env, func ) ) goto error;

    // add pan()
    func = make_new_sfun( "float", "pan", bus_pan2 );
    func->add_arg( "int", "which" );
    if( !type_engine_import_sfun( env, func ) ) goto error;

    // end the class import
    type_engine_import_class_end( env );


	// import
    if( !type_engine_import_class_begin( env, "TapsUI", "Event",
                                         env->global(), chui_ctor) )
        return FALSE;

	// add member (data -- ui_element)
    chui_offset_data = type_engine_import_mvar( env, "int", "@data", FALSE );
    if( chui_offset_data == CK_INVALID_OFFSET ) goto error;

	// add position()
	func = make_new_mfun( "void", "position", chui_setposition );
	func->add_arg( "float", "x" );
	func->add_arg( "float", "y" );
	func->add_arg( "float", "z" );
	if( !type_engine_import_mfun( env, func ) ) goto error;

	// add value()
	func = make_new_mfun( "float", "value", chui_setvalue );
	func->add_arg( "float", "val" );
	if( !type_engine_import_mfun( env, func ) ) goto error;
	// add value()
	func = make_new_mfun( "float", "value", chui_getvalue );
	if( !type_engine_import_mfun( env, func ) ) goto error;

	// add size()
	func = make_new_mfun( "float", "size", chui_setsize );
	func->add_arg( "float", "scale" );
	if( !type_engine_import_mfun( env, func ) ) goto error;
	// add size()
	func = make_new_mfun( "float", "size", chui_getsize );
	if( !type_engine_import_mfun( env, func ) ) goto error;

	// add label()
	func = make_new_mfun( "string", "label", chui_setlabel );
	func->add_arg( "string", "name" );
	if( !type_engine_import_mfun( env, func ) ) goto error;
	// add label()
	func = make_new_mfun( "string", "label", chui_getlabel );
	if( !type_engine_import_mfun( env, func ) ) goto error;
	
	// end the class import
	type_engine_import_class_end( env );


	// import
    if( !type_engine_import_class_begin( env, "TapsUISlider", "TapsUI",
                                         env->global(), chuisl_ctor) )
        return FALSE;

	// add member (flag for whether it's exponential)
    chuisl_offset_exp = type_engine_import_mvar( env, "int", "@exp", FALSE );
    if( chuisl_offset_exp == CK_INVALID_OFFSET ) goto error;

	// add init()
	func = make_new_mfun( "int", "init", chuisl_init );
	func->add_arg( "int", "isHor" );
	func->add_arg( "int", "isExp" );
	func->add_arg( "int", "isInt" );
	func->add_arg( "int", "length" );
	func->add_arg( "float", "min" );
	func->add_arg( "float", "max" );
	if( !type_engine_import_mfun( env, func ) ) goto error;

	// add orientation()
	func = make_new_mfun( "int", "orientation", chuisl_setori );
	func->add_arg( "int", "hor" );
	if( !type_engine_import_mfun( env, func ) ) goto error; 
	// add orientation()
	func = make_new_mfun( "int", "orientation", chuisl_getori );
	if( !type_engine_import_mfun( env, func ) ) goto error;

	// add length()
	func = make_new_mfun( "int", "length", chuisl_setlen );
	func->add_arg( "int", "len" );
	if( !type_engine_import_mfun( env, func ) ) goto error; 
	// add length()
	func = make_new_mfun( "int", "length", chuisl_getlen );
	if( !type_engine_import_mfun( env, func ) ) goto error;

    // end the class import
    type_engine_import_class_end( env );


	// import
    if( !type_engine_import_class_begin( env, "TapsUIButton", "TapsUI",
                                         env->global(), chuibt_ctor) )
        return FALSE;

	// end the class import
	type_engine_import_class_end( env );


	// import
    if( !type_engine_import_class_begin( env, "TapsUIFlipper", "TapsUI",
                                         env->global(), chuifl_ctor) )
        return FALSE;

	// end the class import
	type_engine_import_class_end( env );

	
	// DONE
    return TRUE;

error:

    // end the class import
    type_engine_import_class_end( env );

    return FALSE;
}


// make copies
static t_TAPBOOL make_copies( const std::string & name, t_TAPINT index, t_TAPINT copies )
{
    // get list
    vector<UI_Template *> temps = Library::instance()->search_name( name );
    // check index
    if( index < 0 || index >= temps.size() ) return FALSE;
    if( copies <= 0 ) return FALSE;
    // make the copies
    return synth_copy_template( temps[index], copies );
}

// load
CK_DLL_SFUN( synth_load )
{
    // get name
    Chuck_String * name = (Chuck_String *)GET_NEXT_STRING(ARGS);

    // load
    RETURN->v_int = synth_load_template( name->str );
}

// copy
CK_DLL_SFUN( synth_copy )
{
    // get name
    Chuck_String * name = (Chuck_String *)GET_NEXT_STRING(ARGS);
    // get index
    t_TAPINT index = GET_NEXT_INT(ARGS);
    // get copies
    t_TAPINT copies = GET_NEXT_INT(ARGS);
    // copy
    RETURN->v_int = make_copies( name->str, index, copies );
}

// count
CK_DLL_SFUN( synth_count )
{
    // get path
    Chuck_String * path = (Chuck_String *)GET_NEXT_STRING(ARGS);
    // get list
    vector<UI_Template *> temps = Library::instance()->search_name( path->str );

    RETURN->v_int = temps.size();
}

// ctor
CK_DLL_CTOR( template_ctor )
{
}

// dtor
CK_DLL_DTOR( template_dtor )
{
}

// play
CK_DLL_MFUN( template_play )
{
    Template * temp = (Template *)OBJ_MEMBER_INT(SELF,template_offset_data);
    if( !temp )
    {
        fprintf( stderr, "[tapestrea](via chuck): template operation before 'read'...\n" );
        return;
    }

    // chuck audio bus 6, chuck induced on bus 7
    t_TAPUINT which = 0;
    if( temp->mybus < 0 || temp->mybus >= AudioCentral::instance()->num_bus() )
    {
        which = AudioCentral::instance()->num_bus() - 1;
        if( temp->type == TT_SCRIPT ) which--;
    }
    else
    {
        which = temp->mybus;
    }

    if( temp->playing() )
    {
        // stop it
        temp->stop();
    }

    // start over, taking parameters into account
    temp->recompute();
    // TODO
    // if( playme->core->type == TT_TIMELINE )
    //    ((Timeline *)(playme->core))->now_butter = ui_elements[BT_NOW];

//	fprintf( stdout, "Play %x (%s) on bus %i\n", temp, temp->name.c_str(), which );
    // play
    temp->play( AudioCentral::instance()->bus(which) );
}

// play
CK_DLL_MFUN( template_play2 )
{
    Template * temp = (Template *)OBJ_MEMBER_INT(SELF,template_offset_data);
    if( !temp )
    {
        fprintf( stderr, "[tapestrea](via chuck): template operation before 'read'...\n" );
        return;
    }

    t_TAPINT which = GET_NEXT_INT(ARGS);
    if( which < 0 || which >= AudioCentral::instance()->num_bus() )
    {
        fprintf( stderr, "[tapestrea](via chuck): invalid bus '%d' play...\n", which );
        return;
    }

	if( temp->playing() )
    {
        // stop it
        temp->stop();
    }

    // start over, taking parameters into account
    temp->recompute();

//	fprintf( stdout, "Play %x (%s) on bus %i\n", temp, temp->name.c_str(), which );
    temp->play( AudioCentral::instance()->bus(which) );
}

// stop
CK_DLL_MFUN( template_stop )
{
    Template * temp = (Template *)OBJ_MEMBER_INT(SELF,template_offset_data);
    if( !temp )
    {
        fprintf( stderr, "[tapestrea](via chuck): template operation before 'read'...\n" );
        return;
    }

    // stop
    temp->stop();
}

// playing
CK_DLL_MFUN( template_playing )
{
    Template * temp = (Template *)OBJ_MEMBER_INT(SELF,template_offset_data);
    if( !temp )
    {
        fprintf( stderr, "[tapestrea](via chuck): template operation before 'read'...\n" );
        return;
    }

    // playing
    RETURN->v_int = temp->playing();
}

// read
CK_DLL_MFUN( template_read )
{
    Template * temp = (Template *)OBJ_MEMBER_INT(SELF,template_offset_data);
    // scriptor this is running in
    // Scriptor * scriptor = ScriptCentral::engine()->current();
    // assert( scriptor != NULL );
    if( temp )
    {
        // stop the thing
        if( temp->playing() )
            temp->stop();

        /*
        // from library
        if( scriptor->m_library_templates->find(temp) != scriptor->m_library_templates->end() )
        {
            // TODO: clean

            // remove from map
            scriptor->m_library_templates->erase(temp);
        }
        else
        {
            // queue it up for deletions, which needs to take place after 
            // it runs once more and removes itself
        }
        */
    }

    // get path
    Chuck_String * path = (Chuck_String *)GET_NEXT_STRING(ARGS);
    // load the thing
    temp = synth_load_file( path->str );
    // make sure something got loaded
    if( !temp )
    {
        fprintf( stderr, "[tapestrea](via chuck): cannot open template...\n" );
        fprintf( stderr, "    (file: %s)\n", path->str.c_str() );
    }

    // set it
    OBJ_MEMBER_INT(SELF,template_offset_data) = (t_TAPINT)temp;
    // flag it
    OBJ_MEMBER_INT(SELF,template_offset_flag) = 1;

    RETURN->v_int = temp != NULL;
}


// readFromFile: load into library and read from library
CK_DLL_MFUN( template_readFromFile )
{
    Template * temp = (Template *)OBJ_MEMBER_INT(SELF,template_offset_data);
    
	if( temp )
    {
        // stop the thing
        if( temp->playing() )
            temp->stop();
    }

    // get path
    Chuck_String * path = (Chuck_String *)GET_NEXT_STRING(ARGS);
    // get visibility
	t_TAPINT show = (t_TAPINT)GET_NEXT_INT(ARGS);
	// load the thing
    if( !synth_load_template( path->str, show ) )
		fprintf( stderr, "[tapestrea](via chuck): readFromFile() cannot load template '%s'...\n", path->str.c_str() );
    // get list
	std::string name = BirdBrain::getbase( path->str.c_str() );
	vector<UI_Template *> temps = Library::instance()->search_name_exact( name );
	// use last (assuming it's the most recently added)
	if( temps.size() > 0 )
	{
		fprintf( stderr, "[tapestrea](via chuck): readFromFile() reading template '%s'...\n", name.c_str() );
		temp = temps.back()->core;
	}
	else {
        fprintf( stderr, "[tapestrea](via chuck): cannot open template...\n" );
        fprintf( stderr, "    (file: %s)\n", name.c_str() );
		temp = NULL;
	}

    // set it
    OBJ_MEMBER_INT(SELF,template_offset_data) = (t_TAPINT)temp;
    // flag it
    OBJ_MEMBER_INT(SELF,template_offset_flag) = 1;

    RETURN->v_int = temp != NULL;
}


// readFromFile: load into library and read from library, visible by default
CK_DLL_MFUN( template_readFromFile2 )
{
    Template * temp = (Template *)OBJ_MEMBER_INT(SELF,template_offset_data);
    
	if( temp )
    {
        // stop the thing
        if( temp->playing() )
            temp->stop();
    }

    // get path
    Chuck_String * path = (Chuck_String *)GET_NEXT_STRING(ARGS);
    // load the thing
    if( !synth_load_template( path->str ) )
		fprintf( stderr, "[tapestrea](via chuck): cannot load template '%s'...\n", path->str.c_str() );
    // get list
	std::string name = BirdBrain::getbase( path->str.c_str() );
	vector<UI_Template *> temps = Library::instance()->search_name_exact( name );
	// use last (assuming it's the most recently added)
	if( temps.size() > 0 )
	{
		fprintf( stderr, "[tapestrea](via chuck): reading template '%s'...\n", name.c_str() );
		temp = temps.back()->core;
	}
	else {
        fprintf( stderr, "[tapestrea](via chuck): cannot open template...\n" );
        fprintf( stderr, "    (file: %s)\n", name.c_str() );
		temp = NULL;
	}

    // set it
    OBJ_MEMBER_INT(SELF,template_offset_data) = (t_TAPINT)temp;
    // flag it
    OBJ_MEMBER_INT(SELF,template_offset_flag) = 1;

    RETURN->v_int = temp != NULL;
}


// readFromLibrary
CK_DLL_MFUN( template_readFromLibrary )
{
    Template * temp = (Template *)OBJ_MEMBER_INT(SELF,template_offset_data);
    if( temp )
    {
        // stop the thing
        if( temp->playing() )
            temp->stop();

        // TODO: do the above
    }

    // get name
    Chuck_String * name = (Chuck_String *)GET_NEXT_STRING(ARGS);
    // get list
    vector<UI_Template *> temps = Library::instance()->search_name_exact( name->str );
    // use first
    if( temps.size() > 0 )
    {
        fprintf( stderr, "[tapestrea](via chuck): reading template '%s'...\n", name->str.c_str() );
        temp = temps[0]->core;
    }
    else
    {
        fprintf( stderr, "[tapestrea](via chuck): no template matching '%s'...\n", name->str.c_str() );
        temp = NULL;
    }

    // set it
    OBJ_MEMBER_INT(SELF,template_offset_data) = (t_TAPINT)temp;

    RETURN->v_int = temp != NULL;
}

// readFromLibrary
CK_DLL_MFUN( template_readFromLibrary2 )
{
    Template * temp = (Template *)OBJ_MEMBER_INT(SELF,template_offset_data);
    if( temp )
    {
        // stop the thing
        if( temp->playing() )
            temp->stop();

        // TODO: do the above
    }

    // get path
    Chuck_String * name = (Chuck_String *)GET_NEXT_STRING(ARGS);
    // get index
    t_TAPINT index = (t_TAPINT)GET_NEXT_INT(ARGS);
    // get list
    vector<UI_Template *> temps = Library::instance()->search_name_exact( name->str );
    // use first
    if( temps.size() > 0 && index >= 0 && index < temps.size() )
    {
        fprintf( stderr, "[tapestrea](via chuck): reading template '%s' index %d...\n", name->str.c_str(), index );
        temp = temps[index]->core;
    }
    else
    {
        fprintf( stderr, "[tapestrea](via chuck): no template matching '%s' index %d...\n", name->str.c_str(), index );
        temp = NULL;
    }

    // set it
    OBJ_MEMBER_INT(SELF,template_offset_data) = (t_TAPINT)temp;

    RETURN->v_int = temp != NULL;
}

// rewind
CK_DLL_MFUN( template_rewind )
{
    Template * temp = (Template *)OBJ_MEMBER_INT(SELF,template_offset_data);
    if( !temp )
    {
        fprintf( stderr, "[tapestrea](via chuck): template operation before 'read'...\n" );
        return;
    }

    // rewind it
    temp->rewind();
}

// bus
CK_DLL_MFUN( template_set_bus )
{
    Template * temp = (Template *)OBJ_MEMBER_INT(SELF,template_offset_data);
    if( !temp )
    {
        fprintf( stderr, "[tapestrea](via chuck): template operation before 'read'...\n" );
        return;
    }

    // get bus number
    t_TAPINT i = GET_NEXT_INT(ARGS);
    // set it
    if( i >= 0 && i < AudioCentral::instance()->num_bus() )
        temp->mybus = i;
    else
        fprintf( stderr, "[tapestrea](via chuck): cannot set bus to %i\n", i ); 
    
    RETURN->v_int = temp->mybus;
}

// bus
CK_DLL_MFUN( template_get_bus )
{
    Template * temp = (Template *)OBJ_MEMBER_INT(SELF,template_offset_data);
    if( !temp )
    {
        fprintf( stderr, "[tapestrea](via chuck): template operation before 'read'...\n" );
        return;
    }
    
    RETURN->v_int = temp->mybus;
}

// freqWarp
CK_DLL_MFUN( template_set_freqWarp )
{
    Template * temp = (Template *)OBJ_MEMBER_INT(SELF,template_offset_data);
    if( !temp )
    {
        fprintf( stderr, "[tapestrea](via chuck): template operation before 'read'...\n" );
        return;
    }

    // get freq warp
    t_TAPFLOAT f = GET_NEXT_FLOAT(ARGS);
    // set it
    temp->set_param( Template::FREQ_WARP, f );
    // invalidate
    if( g_synth_face ) g_synth_face->load_sliders = true;
    
    RETURN->v_float = temp->freq_warp;
}

// freqWarp
CK_DLL_MFUN( template_get_freqWarp )
{
    Template * temp = (Template *)OBJ_MEMBER_INT(SELF,template_offset_data);
    if( !temp )
    {
        fprintf( stderr, "[tapestrea](via chuck): template operation before 'read'...\n" );
        return;
    }

    RETURN->v_float = temp->freq_warp;
}

// timeStretch
CK_DLL_MFUN( template_set_timeStretch )
{
    Template * temp = (Template *)OBJ_MEMBER_INT(SELF,template_offset_data);
    if( !temp )
    {
        fprintf( stderr, "[tapestrea](via chuck): template operation before 'read'...\n" );
        return;
    }

    // get time stretch
    t_TAPFLOAT f = GET_NEXT_FLOAT(ARGS);
    // set it
    temp->set_param( Template::TIME_STRETCH, f );
    // invalidate
    if( g_synth_face ) g_synth_face->load_sliders = true;

    RETURN->v_float = temp->time_stretch;
}

// timeStretch
CK_DLL_MFUN( template_get_timeStretch )
{
    Template * temp = (Template *)OBJ_MEMBER_INT(SELF,template_offset_data);
    if( !temp )
    {
        fprintf( stderr, "[tapestrea](via chuck): template operation before 'read'...\n" );
        return;
    }

    RETURN->v_float = temp->time_stretch;
}

// pan
CK_DLL_MFUN( template_set_pan )
{
    Template * temp = (Template *)OBJ_MEMBER_INT(SELF,template_offset_data);
    if( !temp )
    {
        fprintf( stderr, "[tapestrea](via chuck): template operation before 'read'...\n" );
        return;
    }

    // get pan
    t_TAPFLOAT f = GET_NEXT_FLOAT(ARGS);
    // set it
    temp->set_param( Template::PAN, f );
    // invalidate
    if( g_synth_face ) g_synth_face->load_sliders = true;

    RETURN->v_float = temp->pan;
}

// pan
CK_DLL_MFUN( template_get_pan )
{
    Template * temp = (Template *)OBJ_MEMBER_INT(SELF,template_offset_data);
    if( !temp )
    {
        fprintf( stderr, "[tapestrea](via chuck): template operation before 'read'...\n" );
        return;
    }

    RETURN->v_float = temp->pan;
}

// gain
CK_DLL_MFUN( template_set_gain )
{
    Template * temp = (Template *)OBJ_MEMBER_INT(SELF,template_offset_data);
    if( !temp )
    {
        fprintf( stderr, "[tapestrea](via chuck): template operation before 'read'...\n" );
        return;
    }

    // get gain
    t_TAPFLOAT f = GET_NEXT_FLOAT(ARGS);
    // set it
    temp->set_param( Template::GAIN, f );
    // invalidate
    if( g_synth_face ) g_synth_face->load_sliders = true;

    RETURN->v_float = temp->gain;
}

// gain
CK_DLL_MFUN( template_get_gain )
{
    Template * temp = (Template *)OBJ_MEMBER_INT(SELF,template_offset_data);
    if( !temp )
    {
        fprintf( stderr, "[tapestrea](via chuck): template operation before 'read'...\n" );
        return;
    }

    RETURN->v_float = temp->gain;
}

// periodicity
CK_DLL_MFUN( template_set_periodicity )
{
    Template * temp = (Template *)OBJ_MEMBER_INT(SELF,template_offset_data);
    if( !temp )
    {
        fprintf( stderr, "[tapestrea](via chuck): template operation before 'read'...\n" );
        return;
    }

    // get periodicity
    t_TAPFLOAT f = GET_NEXT_FLOAT(ARGS);
    // set it
    temp->set_param( Template::PERIODICITY, f );
    // invalidate
    if( g_synth_face ) g_synth_face->load_sliders = true;

    RETURN->v_float = temp->periodicity;
}

// periodicity
CK_DLL_MFUN( template_get_periodicity )
{
    Template * temp = (Template *)OBJ_MEMBER_INT(SELF,template_offset_data);
    if( !temp )
    {
        fprintf( stderr, "[tapestrea](via chuck): template operation before 'read'...\n" );
        return;
    }

    RETURN->v_float = temp->periodicity;
}

// density
CK_DLL_MFUN( template_set_density )
{
    Template * temp = (Template *)OBJ_MEMBER_INT(SELF,template_offset_data);
    if( !temp )
    {
        fprintf( stderr, "[tapestrea](via chuck): template operation before 'read'...\n" );
        return;
    }

    // get density
    t_TAPFLOAT f = GET_NEXT_FLOAT(ARGS);
    // set it
    temp->set_param( Template::DENSITY, f );
    // invalidate
    if( g_synth_face ) g_synth_face->load_sliders = true;

    RETURN->v_float = temp->density;
}

// density
CK_DLL_MFUN( template_get_density )
{
    Template * temp = (Template *)OBJ_MEMBER_INT(SELF,template_offset_data);
    if( !temp )
    {
        fprintf( stderr, "[tapestrea](via chuck): template operation before 'read'...\n" );
        return;
    }

    RETURN->v_float = temp->density;
}

// random
CK_DLL_MFUN( template_set_random )
{
    Template * temp = (Template *)OBJ_MEMBER_INT(SELF,template_offset_data);
    if( !temp )
    {
        fprintf( stderr, "[tapestrea](via chuck): template operation before 'read'...\n" );
        return;
    }

    // get random
    t_TAPFLOAT f = GET_NEXT_FLOAT(ARGS);
    // set it
    temp->set_param( Template::RANDOM, f );
    // invalidate
    if( g_synth_face ) g_synth_face->load_sliders = true;

    // look at type
    if( temp->type == TT_LOOP )
    {
        // safe to cast, maybe
        LoopTemplate * loop = (LoopTemplate *)temp;
        RETURN->v_float = loop->random;
    }
    else
    {
        // random doesn't apply
        RETURN->v_float = 0.0;
    }
}

// random
CK_DLL_MFUN( template_get_random )
{
    Template * temp = (Template *)OBJ_MEMBER_INT(SELF,template_offset_data);
    if( !temp )
    {
        fprintf( stderr, "[tapestrea](via chuck): template operation before 'read'...\n" );
        return;
    }

    // look at type
    if( temp->type == TT_LOOP )
    {
        // safe to cast, maybe
        LoopTemplate * loop = (LoopTemplate *)temp;
        RETURN->v_float = loop->random;
    }
    else
    {
        // random doesn't apply
        RETURN->v_float = 0.0;
    }
}

// ready
CK_DLL_MFUN( template_get_ready )
{
    Template * temp = (Template *)OBJ_MEMBER_INT(SELF,template_offset_data);
    RETURN->v_int = temp != NULL;
}

// close
CK_DLL_MFUN( template_close )
{
    Template * temp = (Template *)OBJ_MEMBER_INT(SELF,template_offset_data);
    if( !temp ) return;

    // check
    if( OBJ_MEMBER_INT(SELF,template_offset_flag) )
    {
        fprintf( stderr, "[tapestrea](via chuck): closing template...\n" );
        SAFE_DELETE(temp);
    }
    else
    {
        fprintf( stderr, "[tapestrea](via chuck): not closing library template...\n" );
    }

    OBJ_MEMBER_INT(SELF,template_offset_data) = 0;
    RETURN->v_int = 1;
}

// release
CK_DLL_MFUN( template_release )
{
	Template * temp = (Template *)OBJ_MEMBER_INT(SELF,template_offset_data);
	// do something
	// eg decrement some reference count somewhere
	// or use dummification of templates within scripts and update that
	RETURN->v_int = 1;
}

// reverb
CK_DLL_SFUN( bus_reverb )
{
    // get arg
    t_TAPINT which = GET_NEXT_INT(ARGS);
    t_TAPFLOAT value = GET_NEXT_FLOAT(ARGS);
    
    // check
    if( which < 0 || which >= AudioCentral::instance()->num_bus() )
    {
        fprintf( stderr, "[tapestrea](via chuck): invalid bus '%d' reverb...\n", which );
        return;
    }

    AudioCentral::instance()->bus(which)->reverb()->mix( value );
    if( g_control_face ) g_control_face->load_sliders = true;
    RETURN->v_float = AudioCentral::instance()->bus(which)->reverb()->getmix();
}

// reverb
CK_DLL_SFUN( bus_reverb2 )
{
    // get arg
    t_TAPINT which = GET_NEXT_INT(ARGS);

    // check
    if( which < 0 || which >= AudioCentral::instance()->num_bus() )
    {
        fprintf( stderr, "[tapestrea](via chuck): invalid bus '%d' reverb...\n", which );
        return;
    }

    RETURN->v_float = AudioCentral::instance()->bus(which)->reverb()->getmix();
}

// volume
CK_DLL_SFUN( bus_volume )
{
    // get arg
    t_TAPINT which = GET_NEXT_INT(ARGS);
    t_TAPFLOAT value = GET_NEXT_FLOAT(ARGS);

    // check
    if( which < 0 || which >= AudioCentral::instance()->num_bus() )
    {
        fprintf( stderr, "[tapestrea](via chuck): invalid bus '%d' volume...\n", which );
        return;
    }

    AudioCentral::instance()->bus(which)->set_gain( value );
    if( g_control_face ) g_control_face->load_sliders = true;
    RETURN->v_float = AudioCentral::instance()->bus(which)->get_gain();
}

// volume
CK_DLL_SFUN( bus_volume2 )
{
    // get arg
    t_TAPINT which = GET_NEXT_INT(ARGS);
    t_TAPFLOAT value = GET_NEXT_FLOAT(ARGS);

    // check
    if( which < 0 || which >= AudioCentral::instance()->num_bus() )
    {
        fprintf( stderr, "[tapestrea](via chuck): invalid bus '%d' volume...\n", which );
        return;
    }

    RETURN->v_float = AudioCentral::instance()->bus(which)->get_gain();
}

// pan
CK_DLL_SFUN( bus_pan )
{
    // get arg
    t_TAPINT which = GET_NEXT_INT(ARGS);
    t_TAPFLOAT value = GET_NEXT_FLOAT(ARGS);

    // check
    if( which < 0 || which >= AudioCentral::instance()->num_bus() )
    {
        fprintf( stderr, "[tapestrea](via chuck): invalid bus '%d' pan...\n", which );
        return;
    }

    AudioCentral::instance()->bus(which)->set_pan( value );
    if( g_control_face ) g_control_face->load_sliders = true;
    RETURN->v_float = AudioCentral::instance()->bus(which)->get_pan();
}

// pan2
CK_DLL_SFUN( bus_pan2 )
{
    // get arg
    t_TAPINT which = GET_NEXT_INT(ARGS);
    t_TAPFLOAT value = GET_NEXT_FLOAT(ARGS);

    // check
    if( which < 0 || which >= AudioCentral::instance()->num_bus() )
    {
        fprintf( stderr, "[tapestrea](via chuck): invalid bus '%d' pan...\n", which );
        return;
    }

    RETURN->v_float = AudioCentral::instance()->bus(which)->get_pan();
}


// TapsUI constructor
CK_DLL_CTOR( chui_ctor )
{
	// create
	UI_Element * element = new UI_Element;
	element->set_event( (Chuck_Event *)SELF );
    OBJ_MEMBER_INT(SELF, chui_offset_data) = (t_TAPINT)element;
	// find scriptor
	Scriptor * m_scriptor = ScriptCentral::find_scriptor( SHRED );
	// add element to scriptor
	if( m_scriptor )
		m_scriptor->add_element( element );
	else 
		fprintf( stderr, "[tapestrea] (via chuck): No script found for shred %i\n", SHRED->xid );
}

// TapsUI destructor
CK_DLL_DTOR( chui_dtor )
{
	// retrieve element
	UI_Element * element = (UI_Element *)OBJ_MEMBER_INT(SELF, chui_offset_data);
	// remove element from scriptor
	Scriptor * m_scriptor = ScriptCentral::find_scriptor( SHRED );
	if( m_scriptor )
		m_scriptor->remove_element( element );
	// delete element (already done in scriptor, but just in case)
	SAFE_DELETE( element );
	OBJ_MEMBER_INT(SELF, chui_offset_data) = 0;
}

// set position
CK_DLL_MFUN( chui_setposition )
{
	UI_Element * element = (UI_Element *)OBJ_MEMBER_INT(SELF, chui_offset_data);
	if( element ) {
		t_TAPFLOAT x = GET_NEXT_FLOAT(ARGS);
		t_TAPFLOAT y = GET_NEXT_FLOAT(ARGS);
		t_TAPFLOAT z = GET_NEXT_FLOAT(ARGS);
		if( !element->rel_loc )
			element->rel_loc = new Point3D( x, y, z );
		else 
		{
			Flt * p = element->rel_loc->pdata();
			p[0] = x; p[1] = y; p[2] = z;
		}
		element->must_adjust_loc = TRUE;
		//RETURN->v_int = 1;
	}
	else {
		fprintf( stderr, "[tapestrea] (via chuck): position() of non-existent ui element\n" ); 		
		//RETURN->v_int = 0;
	}
}

// set value
CK_DLL_MFUN( chui_setvalue )
{
	UI_Element * element = (UI_Element *)OBJ_MEMBER_INT(SELF, chui_offset_data);
	if( element ) {
		t_TAPFLOAT val = GET_NEXT_FLOAT(ARGS);
		element->set_slide( val );
		RETURN->v_float = val;
	}
	else {
		fprintf( stderr, "[tapestrea] (via chuck): value() of non-existent ui element\n" );
		RETURN->v_float = 0;
	}
}

// get value
CK_DLL_MFUN( chui_getvalue )
{
	UI_Element * element = (UI_Element *)OBJ_MEMBER_INT(SELF, chui_offset_data);
	if( element ) {
		RETURN->v_float = element->fvalue();
	}
	else {
		fprintf( stderr, "[tapestrea] (via chuck): value() of non-existent ui element\n" );
		RETURN->v_float = 0;
	}
}


// set size
CK_DLL_MFUN( chui_setsize )
{
	UI_Element * element = (UI_Element *)OBJ_MEMBER_INT(SELF, chui_offset_data);
	if( element ) {
		t_TAPFLOAT scale = GET_NEXT_FLOAT(ARGS);
		// todo: save this info for later use, maybe as an object 
		fprintf( stderr, "[tapestrea] (via chuck): size() not implemented yet\n" );
		RETURN->v_float = scale;
	}
	else {
		fprintf( stderr, "[tapestrea] (via chuck): size() of non-existent ui element\n" );
		RETURN->v_float = 0;
	}
}

// get size
CK_DLL_MFUN( chui_getsize )
{
	UI_Element * element = (UI_Element *)OBJ_MEMBER_INT(SELF, chui_offset_data);
	if( element ) {
		fprintf( stderr, "[tapestrea] (via chuck): size() not implemented yet\n" );
		RETURN->v_float = 0;
	}
	else {
		fprintf( stderr, "[tapestrea] (via chuck): size() of non-existent ui element\n" );
		RETURN->v_float = 0;
	}
}

// set name
CK_DLL_MFUN( chui_setlabel )
{
	UI_Element * element = (UI_Element *)OBJ_MEMBER_INT(SELF, chui_offset_data);
	if( element ) {
		Chuck_String * name = (Chuck_String *)GET_NEXT_STRING(ARGS);
		element->name = name->str;
		RETURN->v_string = name;
	}
	else {
		fprintf( stderr, "[tapestrea] (via chuck): label() of non-existent ui element\n" );
		RETURN->v_string = NULL;
	}
}

// get name
CK_DLL_MFUN( chui_getlabel )
{
	UI_Element * element = (UI_Element *)OBJ_MEMBER_INT(SELF, chui_offset_data);
	if( element ) {
		RETURN->v_string = (Chuck_String *)element->name.c_str();
	}
	else {
		fprintf( stderr, "[tapestrea] (via chuck): label() of non-existent ui element\n" );
		RETURN->v_string = NULL;
	}
}


// TapsUISlider constructor
CK_DLL_CTOR( chuisl_ctor )
{ 
	UI_Element * element = (UI_Element *)OBJ_MEMBER_INT(SELF, chui_offset_data);
	element->element_type = UI_Element::UI_SLIDER;
}

// TapsUISlider destructor
CK_DLL_DTOR( chuisl_dtor ) { }

// slider init
CK_DLL_MFUN( chuisl_init )
{
	// delete whatever existed
	UI_Element * element = (UI_Element *)OBJ_MEMBER_INT(SELF, chui_offset_data);
	Scriptor * m_scriptor = ScriptCentral::find_scriptor( SHRED );
	if( element ) {
		if( m_scriptor )
			m_scriptor->remove_element( element );
		else {
			fprintf( stderr, "[tapestrea] (via chuck): Slider init: no scriptor found!\n" );
			SAFE_DELETE( element );
		}
	}

	// read params
	t_TAPINT isHor = GET_NEXT_INT(ARGS);
	t_TAPINT isExp = GET_NEXT_INT(ARGS);
	t_TAPINT isInt = GET_NEXT_INT(ARGS);
	t_TAPINT length = GET_NEXT_INT(ARGS);
	t_TAPFLOAT min = GET_NEXT_FLOAT(ARGS);
	t_TAPFLOAT max = GET_NEXT_FLOAT(ARGS);

	// exponential or not, create new element accordingly
	if( isExp )
		element = new UI_Exp( 10000 ); // base is arbitrary now
	else 
		element = new UI_Element;
	OBJ_MEMBER_INT(SELF, chuisl_offset_exp) = isExp;
	
	// set drawing params of element
	if( isHor )
		element->element_orientation = UI_Element::UI_HORIZONTAL;
	else
		element->element_orientation = UI_Element::UI_VERTICAL;
	if( length == 2 )
		element->element_length = UI_Element::UI_MICRO;
	else if( length == 1 )
		element->element_length = UI_Element::UI_MINI;
	else
		element->element_length = UI_Element::UI_NORMAL;
	
	// set range params
	element->set_bounds( min, max, isInt > 0 );

	// set type and event
	element->element_type = UI_Element::UI_SLIDER;
	element->set_event( (Chuck_Event *)SELF );

	// add element
	OBJ_MEMBER_INT(SELF, chui_offset_data) = (t_TAPINT)element;
	if( m_scriptor )
		m_scriptor->add_element( element );

	RETURN->v_int = 1;
}

// set orientation
CK_DLL_MFUN( chuisl_setori )
{
	UI_Element * element = (UI_Element *)OBJ_MEMBER_INT(SELF, chui_offset_data);
	if( element ) {
		t_TAPINT ori = GET_NEXT_INT(ARGS);
		if( ori != 0 )
			element->element_orientation = UI_Element::UI_HORIZONTAL;
		else
			element->element_orientation = UI_Element::UI_VERTICAL;
		RETURN->v_int = ori;
	}
	else {
		fprintf( stderr, "[tapestrea] (via chuck): orientation() of non-existent ui element\n" );
		RETURN->v_int = 0;
	}
}

// get orientation
CK_DLL_MFUN( chuisl_getori )
{
	UI_Element * element = (UI_Element *)OBJ_MEMBER_INT(SELF, chui_offset_data);
	if( element ) {
		RETURN->v_int = (element->element_orientation == UI_Element::UI_HORIZONTAL);
	}
	else {
		fprintf( stderr, "[tapestrea] (via chuck): orientation() of non-existent ui element\n" );
		RETURN->v_int = 0;
	}
}

// set length
CK_DLL_MFUN( chuisl_setlen )
{
	UI_Element * element = (UI_Element *)OBJ_MEMBER_INT(SELF, chui_offset_data);
	if( element ) {
		t_TAPINT length = GET_NEXT_INT(ARGS);
		if( length == 2 )
			element->element_length = UI_Element::UI_MICRO;
		else if( length == 1 )
			element->element_length = UI_Element::UI_MINI;
		else
			element->element_length = UI_Element::UI_NORMAL;
		RETURN->v_int = length;
	}
	else {
		fprintf( stderr, "[tapestrea] (via chuck): length() of non-existent ui element\n" );
		RETURN->v_int = 0;
	}
}

// get length
CK_DLL_MFUN( chuisl_getlen )
{
	UI_Element * element = (UI_Element *)OBJ_MEMBER_INT(SELF, chui_offset_data);
	if( element ) {
		t_TAPINT length;
		if( element->element_length == UI_Element::UI_MICRO )
			length = 2;
		else if( element->element_length == UI_Element::UI_MINI )
			length = 1;
		else
			length = 0;
		RETURN->v_int = length;
	}
	else {
		fprintf( stderr, "[tapestrea] (via chuck): length() of non-existent ui element\n" );
		RETURN->v_int = 0;
	}
}

// TapsUIButton constructor
CK_DLL_CTOR( chuibt_ctor )
{ 
//	chui_ctor();
	UI_Element * element = (UI_Element *)OBJ_MEMBER_INT(SELF, chui_offset_data);
	element->element_type = UI_Element::UI_BUTTON;
}

// TapsUIButton destructor
CK_DLL_DTOR( chuibt_dtor ) { }

// TapsUIFlipper constructor
CK_DLL_CTOR( chuifl_ctor )
{ 
//	chui_ctor();
	UI_Element * element = (UI_Element *)OBJ_MEMBER_INT(SELF, chui_offset_data);
	element->element_type = UI_Element::UI_FLIPPER;
}

// TapsUIFlipper destructor
CK_DLL_DTOR( chuifl_dtor ) { }


#endif


/*
(look at maui for polling / event-based options)
(seems to have a Chuck_Event * e, which can be accessed / modified using set_event() and get_event())

base:
  TapsUI 
    int setPosition( float x, float y, float z );
    int setValue( float value );
	int setSize( float scale );
	int setLabel( string label );
	float getValue();
    float xpos, ypos, zpos, scale;
	string label;

derived:
  TapsUISlider
    int isHorizontal;
	int isExponential;
	int isInt;
    int size (normal, mini, micro)
	int init(int isHor, int isExp, int isInt, int size, float min, float max)
    int setOrientation(int hor);
	int setExp(int exp); // no -- bad! set exp in init only
	int setLength(int x);
 
  TapsUIButton
    
  TapsUIFlipper
    
*/
