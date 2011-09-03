//-----------------------------------------------------------------------------
// name: ui_scripting.h
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

#include "chuck_compile.h"
#include "chuck_vm.h"
#include "chuck_errmsg.h"
#include "chuck_bbq.h"
#include "chuck_globals.h"
#include "util_thread.h"
#include "util_network.h"
#include "ui_scripting.h"
#include "ui_synthesis.h"
#include "ui_control.h"

#ifndef __PLATFORM_WIN32__
  #define CHUCK_THREAD pthread_t
  #include <pthread.h>
  #include <unistd.h>
#else 
  #define CHUCK_THREAD HANDLE
#endif

// static variables
ScriptEngine * ScriptCentral::our_engine = NULL;

// thread
static CHUCK_THREAD g_tid_reply;
static CHUCK_THREAD g_tid_vm;
// reply function
static void * reply_sucker( void * a );
// vm function
static void * vm_runner( void * a );
// api
static t_CKBOOL init_api( Chuck_Env * env );


//-----------------------------------------------------------------------------
// name: startup()
// desc: global scripting startup
//-----------------------------------------------------------------------------
t_CKBOOL ScriptCentral::startup()
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
t_CKBOOL ScriptCentral::shutdown()
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
t_CKBOOL ScriptEngine::init()
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
t_CKBOOL ScriptEngine::start_vm( AudioBus * which )
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
        which->play( this );
        m_bus = which;
    }

    return TRUE;
}


//-----------------------------------------------------------------------------
// name: stop_vm()
// desc: ...
//-----------------------------------------------------------------------------
t_CKBOOL ScriptEngine::stop_vm()
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
t_CKBOOL ScriptEngine::shutdown()
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
t_CKBOOL ScriptEngine::vm_is_init()
{
    if( !m_vm ) return FALSE;
    return m_vm->has_init();
}


//-----------------------------------------------------------------------------
// name: vm_is_running()
// desc: ...
//-----------------------------------------------------------------------------
t_CKBOOL ScriptEngine::vm_is_running()
{
    if( !m_vm ) return FALSE;
    return m_vm->is_running();
}


//-----------------------------------------------------------------------------
// name: stick()
// desc: ...
//-----------------------------------------------------------------------------
t_CKBOOL ScriptEngine::stick( SAMPLE * buffer, t_CKUINT num_frames )
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
t_CKBOOL ScriptEngine::run( Scriptor * scriptor, t_CKBOOL immediate )
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

    // mode
    if( immediate ) m_vm->process_msg( msg );
    else m_vm->queue_msg( msg, 1 );

    return TRUE;
}


//-----------------------------------------------------------------------------
// name: remove()
// desc: ...
//-----------------------------------------------------------------------------
t_CKBOOL ScriptEngine::remove( Scriptor * script, t_CKBOOL immediate )
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
    t_CKBOOL ret = remove( script->m_shred_id, immediate );
    script->m_shred_id = 0;

    return ret;
}


//-----------------------------------------------------------------------------
// name: remove()
// desc: ...
//-----------------------------------------------------------------------------
t_CKBOOL ScriptEngine::remove( t_CKUINT id, t_CKBOOL immediate )
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
t_CKBOOL ScriptEngine::remove_all( t_CKBOOL immediate )
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
}


//-----------------------------------------------------------------------------
// name: stick()
// desc: ...
//-----------------------------------------------------------------------------
t_CKBOOL Scriptor::stick( SAMPLE * buffer, t_CKUINT num_frames )
{
    // first time
    if( m_script_init == FALSE )
    {
        // add
        m_script_init = ScriptCentral::engine()->run( this );
        // make sure
        assert( m_script_init == TRUE );
    }

    // zero it out
    memset( buffer, 0, sizeof(SAMPLE) * 2 * num_frames );

    // stop
    if( m_stop_asap )
    {
        // other stop code moved to stop() (probably introducing several race condition problems)
        m_script_init = FALSE;
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
}


//-----------------------------------------------------------------------------
// name: rewind()
// desc: ...
//-----------------------------------------------------------------------------
void Scriptor::rewind()
{
    // do nothing?
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
CK_DLL_MFUN( template_read );
CK_DLL_MFUN( template_readFromLibrary );
CK_DLL_MFUN( template_readFromLibrary2 );
CK_DLL_MFUN( template_load );
CK_DLL_MFUN( template_rewind );
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
CK_DLL_MFUN( template_close );

// bus API
CK_DLL_SFUN( bus_volume );
CK_DLL_SFUN( bus_volume2 );
CK_DLL_SFUN( bus_reverb );
CK_DLL_SFUN( bus_reverb2 );
CK_DLL_SFUN( bus_pan );
CK_DLL_SFUN( bus_pan2 );


static t_CKUINT template_offset_data = 0;
static t_CKUINT template_offset_flag = 0;
//-----------------------------------------------------------------------------
// name: init_api()
// desc: ...
//-----------------------------------------------------------------------------
t_CKBOOL init_api2( Chuck_Env * env );

t_CKBOOL init_api( Chuck_Env * env )
{
    // make context
    Chuck_Context * context = type_engine_make_context( NULL, "@[internal-tapestrea]" );
    // reset env
    env->reset();
    // load it
    type_engine_load_context( env, context );

    // load
    t_CKBOOL ret = init_api2( env );

    // clear context
    type_engine_unload_context( env );

    // commit what is in the type checker at this point
    env->global()->commit();

    return ret;
}

t_CKBOOL init_api2( Chuck_Env * env )
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

    // add readFromLibrary()
    func = make_new_mfun( "int", "readFromLibrary", template_readFromLibrary );
    func->add_arg( "string", "path" );
    if( !type_engine_import_mfun( env, func ) ) goto error;

    // add readFromLibrary()
    func = make_new_mfun( "int", "readFromLibrary", template_readFromLibrary2 );
    func->add_arg( "string", "path" );
    func->add_arg( "int", "index" );
    if( !type_engine_import_mfun( env, func ) ) goto error;

    // add close()
    func = make_new_mfun( "int", "close", template_close );
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

    return TRUE;

error:

    // end the class import
    type_engine_import_class_end( env );

    return FALSE;
}


// make copies
static t_CKBOOL make_copies( const string & name, t_CKINT index, t_CKINT copies )
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
    t_CKINT index = GET_NEXT_INT(ARGS);
    // get copies
    t_CKINT copies = GET_NEXT_INT(ARGS);
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
    t_CKUINT which = AudioCentral::instance()->num_bus() - 1;
    if( temp->type == TT_SCRIPT ) which--;

    if( temp->playing() )
    {
        // stop it
        temp->stop();
        // remove explicitly, if not script
        if( temp->type != TT_SCRIPT )
            AudioCentral::instance()->bus(which)->remove( temp );
    }

    // start over, taking parameters into account
    temp->recompute();
    // TODO
    // if( playme->core->type == TT_TIMELINE )
    //    ((Timeline *)(playme->core))->now_butter = ui_elements[BT_NOW];

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

    t_CKINT which = GET_NEXT_INT(ARGS);
    if( which < 0 || which >= AudioCentral::instance()->num_bus() )
    {
        fprintf( stderr, "[tapestrea](via chuck): invalid bus '%d' play...\n", which );
        return;
    }

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
    Scriptor * scriptor = ScriptCentral::engine()->current();
    assert( scriptor != NULL );
    if( temp )
    {
        // stop the thing
        if( temp->playing() )
            temp->stop();

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
    OBJ_MEMBER_INT(SELF,template_offset_data) = (t_CKINT)temp;
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

    // get path
    Chuck_String * path = (Chuck_String *)GET_NEXT_STRING(ARGS);
    // get list
    vector<UI_Template *> temps = Library::instance()->search_name_exact( path->str );
    // use first
    if( temps.size() > 0 )
    {
        fprintf( stderr, "[tapestrea](via chuck): reading template '%s'...\n", path->str.c_str() );
        temp = temps[0]->core;
    }
    else
    {
        fprintf( stderr, "[tapestrea](via chuck): no template matching '%s'...\n", path->str.c_str() );
        temp = NULL;
    }

    // set it
    OBJ_MEMBER_INT(SELF,template_offset_data) = (t_CKINT)temp;

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
    Chuck_String * path = (Chuck_String *)GET_NEXT_STRING(ARGS);
    // get index
    t_CKINT index = (t_CKINT)GET_NEXT_INT(ARGS);
    // get list
    vector<UI_Template *> temps = Library::instance()->search_name_exact( path->str );
    // use first
    if( temps.size() > 0 && index >= 0 && index < temps.size() )
    {
        fprintf( stderr, "[tapestrea](via chuck): reading template '%s' index %d...\n", path->str.c_str(), index );
        temp = temps[index]->core;
    }
    else
    {
        fprintf( stderr, "[tapestrea](via chuck): no template matching '%s' index %d...\n", path->str.c_str(), index );
        temp = NULL;
    }

    // set it
    OBJ_MEMBER_INT(SELF,template_offset_data) = (t_CKINT)temp;

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
    t_CKFLOAT f = GET_NEXT_FLOAT(ARGS);
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
    t_CKFLOAT f = GET_NEXT_FLOAT(ARGS);
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
    t_CKFLOAT f = GET_NEXT_FLOAT(ARGS);
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
    t_CKFLOAT f = GET_NEXT_FLOAT(ARGS);
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
    t_CKFLOAT f = GET_NEXT_FLOAT(ARGS);
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
    t_CKFLOAT f = GET_NEXT_FLOAT(ARGS);
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
    t_CKFLOAT f = GET_NEXT_FLOAT(ARGS);
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

// reverb
CK_DLL_SFUN( bus_reverb )
{
    // get arg
    t_CKINT which = GET_NEXT_INT(ARGS);
    t_CKFLOAT value = GET_NEXT_FLOAT(ARGS);
    
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
    t_CKINT which = GET_NEXT_INT(ARGS);

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
    t_CKINT which = GET_NEXT_INT(ARGS);
    t_CKFLOAT value = GET_NEXT_FLOAT(ARGS);

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
    t_CKINT which = GET_NEXT_INT(ARGS);
    t_CKFLOAT value = GET_NEXT_FLOAT(ARGS);

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
    t_CKINT which = GET_NEXT_INT(ARGS);
    t_CKFLOAT value = GET_NEXT_FLOAT(ARGS);

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
    t_CKINT which = GET_NEXT_INT(ARGS);
    t_CKFLOAT value = GET_NEXT_FLOAT(ARGS);

    // check
    if( which < 0 || which >= AudioCentral::instance()->num_bus() )
    {
        fprintf( stderr, "[tapestrea](via chuck): invalid bus '%d' pan...\n", which );
        return;
    }

    RETURN->v_float = AudioCentral::instance()->bus(which)->get_pan();
}


#endif
