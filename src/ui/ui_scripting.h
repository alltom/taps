//-----------------------------------------------------------------------------
// name: ui_scripting.h
// desc: scripting "support"
//
// authors: Ananya Misra (amisra@cs.princeton.edu)
//          Ge Wang (gewang@cs.princeton.edu)
//          Perry R. Cook (prc@cs.princeton.edu)
// after EST. January 14, 2005, 8:37 p.m. Friday
//-----------------------------------------------------------------------------
#ifndef __UI_SCRIPTING_H__
#define __UI_SCRIPTING_H__

#include "ui_library.h"
#include "ui_audio.h"

// forward references
struct Chuck_Compiler;
struct Chuck_VM;
struct Chuck_VM_Code;
struct Chuck_VM_Shred;
struct ScriptEngine;
struct Scriptor;


//-----------------------------------------------------------------------------
// name: struct ScriptCentral
// desc: global interface to scripting stuff
//-----------------------------------------------------------------------------
struct ScriptCentral
{
public: // global
    static t_CKBOOL startup();
    static t_CKBOOL shutdown();

public: // interface
    static ScriptEngine * engine(); // get engine

public: // data
    static ScriptEngine * our_engine;
};


//-----------------------------------------------------------------------------
// name: struct Script
// desc: compiler + virtual machine
//-----------------------------------------------------------------------------
struct ScriptEngine : public AudioSrc
{
public:
    ScriptEngine();
    virtual ~ScriptEngine();

    t_CKBOOL init();
    t_CKBOOL start_vm( AudioBus * which );
    t_CKBOOL stop_vm();
    t_CKBOOL shutdown();

    t_CKBOOL vm_is_init();
    t_CKBOOL vm_is_running();
    AudioBus * bus(); // which bus, if any, engine is running on

public:
    Scriptor * compile( const std::string & filename );
    t_CKBOOL run( Scriptor * script, t_CKBOOL immediate = FALSE );
    t_CKBOOL remove( Scriptor * script, t_CKBOOL immediate = FALSE );
    t_CKBOOL remove( t_CKUINT id, t_CKBOOL immediate = FALSE );
    t_CKBOOL remove_all( t_CKBOOL immediate = FALSE );
    Chuck_VM_Code * recompile( const std::string & filename );

public:
    virtual t_CKBOOL stick( SAMPLE * buffer, t_CKUINT num_frames );
    Scriptor * current(); // to be called by script only

public:
    Chuck_Compiler * m_compiler;
    Chuck_VM * m_vm;
    AudioBus * m_bus;
    Scriptor * m_current;
};


//-----------------------------------------------------------------------------
// name: struct Scriptor
// desc: script template
//-----------------------------------------------------------------------------
struct Scriptor : public Template
{
public:
    Scriptor( Chuck_VM_Code * code, time_t myid = 0 );
    virtual ~Scriptor();

    virtual t_CKBOOL stick( SAMPLE * buffer, t_CKUINT num_frames );
    virtual void rewind();
    virtual Template * copy( bool copyid = false ) const;  
    virtual void stop();
    virtual void recompute();

public: // data
    Chuck_VM_Code * m_code;
    t_CKUINT m_shred_id;
    t_CKBOOL m_waiting_for_id;
    t_CKBOOL m_script_init;
    std::string m_filename;
    std::map<Template *, Template *> * m_library_templates;
};


#endif
