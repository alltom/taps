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
#include <map>

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
    static t_TAPBOOL startup();
    static t_TAPBOOL shutdown();

public: // interface
    static ScriptEngine * engine(); // get engine

public: // data
    static ScriptEngine * our_engine;

public: // mapping shred ID to scriptor
	static Scriptor * get_scriptor( t_TAPUINT shred_id );
	static Scriptor * find_scriptor( Chuck_VM_Shred * shred );
	static void add_scriptor( t_TAPUINT shred_id, Scriptor * script );
	static void remove_scriptor( Scriptor * script );
	static void remove_shred( t_TAPUINT shred_id );
	static void update_shred_to_scriptor();
	static t_TAPBOOL check_script_running( Scriptor * script );
private:
	static std::map<t_TAPUINT, Scriptor *> shred_to_scriptor;
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

    t_TAPBOOL init();
    t_TAPBOOL start_vm( AudioBus * which );
    t_TAPBOOL stop_vm();
    t_TAPBOOL shutdown();

    t_TAPBOOL vm_is_init();
    t_TAPBOOL vm_is_running();
    AudioBus * bus(); // which bus, if any, engine is running on

public:
    Scriptor * compile( const std::string & filename );
    t_TAPBOOL run( Scriptor * script, t_TAPBOOL immediate = FALSE );
    t_TAPBOOL remove( Scriptor * script, t_TAPBOOL immediate = FALSE );
    t_TAPBOOL remove( t_TAPUINT id, t_TAPBOOL immediate = FALSE );
    t_TAPBOOL remove_all( t_TAPBOOL immediate = FALSE );
    Chuck_VM_Code * recompile( const std::string & filename );

public:
    virtual t_TAPBOOL stick( SAMPLE * buffer, t_TAPUINT num_frames );
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

    virtual t_TAPBOOL stick( SAMPLE * buffer, t_TAPUINT num_frames );
    virtual t_TAPBOOL rewind();
    virtual Template * copy( bool copyid = false ) const;  
    virtual void stop();
    virtual void recompute();

public: // additional chui functions
	t_TAPBOOL add_element( UI_Element * element );
	t_TAPBOOL remove_element( UI_Element * element );
	t_TAPUINT num_elements();
	UI_Element * element_at( t_TAPUINT i );

public: // data
    Chuck_VM_Code * m_code;
    t_TAPUINT m_shred_id;
    t_TAPBOOL m_waiting_for_id;
    t_TAPBOOL m_script_init;
    std::string m_filename;
    std::map<Template *, Template *> * m_library_templates;
	std::vector<UI_Element *> m_elements;
};


#endif

