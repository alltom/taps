/*----------------------------------------------------------------------------
    ChucK Concurrent, On-the-fly Audio Programming Language
      Compiler and Virtual Machine

    Copyright (c) 2004 Ge Wang and Perry R. Cook.  All rights reserved.
      http://chuck.cs.princeton.edu/
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
// file: taps_cmd.cpp
// desc: command line programming utilities (adapted from chuck_otf.cpp)
//
// author: Ge Wang (gewang@cs.princeton.edu)
//         Perry R. Cook (prc@cs.princeton.edu)
//		   Ananya Misra (amisra@cs.princeton.edu)
// date: December 2006
//-----------------------------------------------------------------------------
#include "ui_library.h"
#ifdef __TAPS_SCRIPTING_ENABLE__
#include "ui_scripting.h"
#endif
#include "taps_cmd.h"
#include "util_thread.h"
#include "taps_birdbrain.h"
#include "util_readwrite.h"
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>

#ifndef __PLATFORM_WIN32__
#include <unistd.h>
#endif

using namespace std;

// network stuff
ck_socket taps_sock;

//-----------------------------------------------------------------------------
// name: cmd_hton( )
// desc: ...
//-----------------------------------------------------------------------------
void cmd_hton( TapsNetMsg * msg )
{
    msg->header = htonl( msg->header );
    msg->type = htonl( msg->type );
    msg->param = htonl( msg->param );
    msg->param2 = htonl( msg->param2 );
    msg->param3 = htonl( msg->param3 );
    msg->length = htonl( msg->length );
}




//-----------------------------------------------------------------------------
// name: cmd_ntoh( )
// desc: ...
//-----------------------------------------------------------------------------
void cmd_ntoh( TapsNetMsg * msg )
{
    msg->header = ntohl( msg->header );
    msg->type = ntohl( msg->type );
    msg->param = ntohl( msg->param );
    msg->param2 = ntohl( msg->param2 );
    msg->param3 = ntohl( msg->param3 );
    msg->length = ntohl( msg->length );
}




//-----------------------------------------------------------------------------
// name: recv_file()
// desc: ...
//-----------------------------------------------------------------------------
FILE * recv_file( const TapsNetMsg & msg, ck_socket sock )
{
    TapsNetMsg buf;
    
    // what is left
    // t_CKUINT left = msg.param2;
    // make a temp file
    FILE * fd = tmpfile();
	if( fd == NULL ) {
		BB_log( BB_LOG_SYSTEM, "Could not generate temporary file" );
		return NULL;
	}
	do {
		// msg
		if( !ck_recv( sock, (char *)&buf, sizeof(buf) ) )
			goto error;
		cmd_ntoh( &buf );
		// write
		fwrite( buf.buffer, sizeof(char), buf.length, fd );
	} while( buf.param2 );

	return fd; 

error:
    fclose( fd );
    fd = NULL;
    return NULL;
}



//-----------------------------------------------------------------------------
// name: load_template()
// desc: ...
//-----------------------------------------------------------------------------
t_TAPBOOL load_template( std::string name )
{
	t_TAPBOOL ret = TRUE;
	BB_log( BB_LOG_INFO, "opening file : %s", name.c_str() );
    const char * c = name.c_str();
    if( name.rfind( ".tap" ) == name.length()-4 || name.rfind( ".xml" ) == name.length()-4 )
	{
        BB_log( BB_LOG_INFO, "Reading '%s' as template file",
                BirdBrain::getbase(name.c_str()) );
        TemplateReader r;
        if( r.open( (char *)c ) )
        {
            Template * tmp = r.read_template();
            r.close();
            if( tmp != NULL && !Library::instance()->hasID( tmp ) )
            {
                Library::instance()->add( tmp );
            }
            else if( tmp != NULL )
            {
                BB_log( BB_LOG_SYSTEM, "Template '%s' already loaded", name.c_str() );
            }
			else {
				BB_log( BB_LOG_SYSTEM, "Could not load template %s", name.c_str() );
			}
        }
        else
        {
			BB_log( BB_LOG_SYSTEM, "Couldn't find file '%s'", name.c_str() );
			ret = FALSE;
        }
    }
    else if( name.rfind( ".ck" ) == name.length()-3 )
    {
#ifdef __TAPS_SCRIPTING_ENABLE__ 
        BB_log( BB_LOG_INFO, "Reading '%s' as .ck script file",
                BirdBrain::getbase(name.c_str()) );
        Scriptor * script = ScriptCentral::engine()->compile( name );
		if( script != NULL )
        {
            script->name = BirdBrain::getname(name.c_str());
            // replace space with _
            int space = script->name.find( " ", 0 );
            while( space != std::string::npos )
            {
                script->name[space] = '_';
                space = script->name.find( " ", space );
            }
            Library::instance()->add( script );
        }
        else
        {
            BB_log( BB_LOG_SYSTEM, "Couldn't load script '%s'", name.c_str() );
			ret = FALSE;
        }
#else
        BB_log( BB_LOG_SYSTEM, "Scripting was not enabled during compilation" );
#endif
    }
    else if( name.rfind( ".qz" ) == name.length()-3 )
    {
		BB_log( BB_LOG_SYSTEM, "Loading of quantization files is not enabled at this point" );
    }
    else
    {
        BB_log( BB_LOG_INFO, "Reading '%s' as sound file", name.c_str() ); 
        File * file = new File( c );
        if( !file || !file->goodtogo )
		{
			std::string msg = "cannot open file: ";
            AudioSrcFile * f = (AudioSrcFile *)(file->src); 
            msg += f->last_error();
            BB_log( BB_LOG_SYSTEM, msg.c_str() );
			ret = FALSE;
        }
        else
        {
            file->name = BirdBrain::getname( c );
            // replace space with _
            int space = file->name.find( " ", 0 );
            while( space != std::string::npos )
            {
                file->name[space] = '_';
                space = file->name.find( " ", space );
            }
            Library::instance()->add( file );
        }
    }
	return ret;
}



//-----------------------------------------------------------------------------
// name: cmd_process_msg()
// desc: ...
//-----------------------------------------------------------------------------
t_TAPUINT cmd_process_msg( TapsNetMsg * msg, t_TAPBOOL immediate, void * data )
{
//	TapsNetMsg * cmd = new TapsNetMsg;
    FILE * fd = NULL;
	t_TAPUINT ret = 0;
    
    if( msg->type == TAPS_MSG_ADD )
    {
        // get name
		std::string name = std::string(msg->buffer);
		// see if entire file is on the way
        if( msg->param2 )
        {
            fd = recv_file( *msg, (ck_socket)data );
			if( !fd )
            {
                BB_log( BB_LOG_SYSTEM, "incoming source transfer '%s' failed...",
					BirdBrain::getname(msg->buffer).c_str() );
				goto cleanup;
            }
        }
		// load template
		ret = load_template( name ) ? 1 : 0;
	}
	else if( msg->type == TAPS_MSG_REMOVE )
	{
		if( msg->param >= 0 && msg->param < Library::instance()->size() )
		{
			BB_log( BB_LOG_INFO, "Removing template %i", msg->param );
			Library::instance()->remove( Library::instance()->templates[msg->param] );
			ret = 1;
		}
		else
		{
			BB_log( BB_LOG_SYSTEM, "Template index %i is invalid for removal", msg->param );
		}
	}
	else if( msg->type == TAPS_MSG_PLAY )
	{
		// check if it's valid
		if( msg->param >= 0 && msg->param < Library::instance()->size() )
		{
			// get template
			Template * temp = Library::instance()->templates[msg->param]->core; 
			// stop if playing
			if( temp->playing() )
			{
				temp->stop();
				while( temp->playing() )
					usleep( 5000 );
			}
			// recompute
			temp->recompute();
			// select bus
			int busno = temp->mybus;
			if( busno < 0 || busno > AudioCentral::instance()->num_bus() )
				busno = 2;
			// play
			temp->play( AudioCentral::instance()->bus( busno ) );
			BB_log( BB_LOG_INFO, "Playing template %i (%s) on bus %i", msg->param, temp->name.c_str(), busno );
			ret = 1;
		}
		else
		{
			BB_log( BB_LOG_SYSTEM, "Template index %i is invalid for playing", msg->param );
		}
	}
	else if( msg->type == TAPS_MSG_STOP )
	{
		// check if it's valid
		if( msg->param >= 0 && msg->param < Library::instance()->size() )
		{
			Template * temp = Library::instance()->templates[msg->param]->core;
			temp->stop();
			BB_log( BB_LOG_INFO, "Stopping template %i (%s)", msg->param, temp->name.c_str() );
			ret = 1;
		}
		else
		{
			BB_log( BB_LOG_SYSTEM, "Template index %i is invalid for stopping", msg->param );
		}
	}
	else if( msg->type == TAPS_MSG_STATUS )
	{
		// spew out library
		Template * temp;
		BB_log( BB_LOG_INFO, "Printing library" );
		for( int i = 0; i < Library::instance()->size(); i++ )
		{
			temp = Library::instance()->templates[i]->core;
			fprintf( stderr, "%i: %s (%s) -- %s\n", i, temp->name.c_str(), temp->type_str(), temp->playing() ? "playing" : "stopped" );
		}
		BB_log( BB_LOG_INFO, "done" );
		ret = 1;
	}
	else if( msg->type == TAPS_MSG_KILL || msg->type == TAPS_MSG_TIME )
    {
		BB_log( BB_LOG_INFO, "Dealing with message type %i is not implemented yet", msg->type );
		ret = 1;
	}
    else
    {
        BB_log( BB_LOG_SYSTEM, "unrecognized incoming command from network: '%i'", msg->type );
		goto cleanup;
    }
	
cleanup:
	// close file handle
	if( fd ) fclose( fd );

    return ret;
}




//-----------------------------------------------------------------------------
// name: cmd_send_file()
// desc: ...
//-----------------------------------------------------------------------------
int cmd_send_file( const char * filename, TapsNetMsg & msg, const char * op,
                   ck_socket dest )
{
    FILE * fd = NULL;
    struct stat fs;
    
	// copy filename into msg.buffer
    strcpy( msg.buffer, "" );
    strcat( msg.buffer, filename );

    // test it
    fd = fopen( (char *)msg.buffer, "rb" );
    if( !fd )
    {
        BB_log( BB_LOG_SYSTEM, "cannot open file '%s' for [%s]...", filename, op );
        return FALSE;
    }
            
    // stat it
    stat( msg.buffer, &fs );
    fseek( fd, 0, SEEK_SET );

    //fprintf(stderr, "sending TCP file %s\n", msg.buffer );v
    // send the first packet
    msg.param2 = (t_TAPUINT)fs.st_size;
    msg.length = 0;
    cmd_hton( &msg );
    ck_send( dest, (char *)&msg, sizeof(msg) );

    // send the whole thing
    t_TAPUINT left = (t_TAPUINT)fs.st_size;
    while( left )
    {
        //fprintf(stderr,"file %03d bytes left ... ", left);
        // amount to send
        msg.length = left > TAPS_NET_BUFFER_SIZE ? TAPS_NET_BUFFER_SIZE : left;
        // read
        msg.param3 = fread( msg.buffer, sizeof(char), msg.length, fd );
        // amount left
        left -= msg.param3 ? msg.param3 : 0;
        msg.param2 = left;
        //fprintf(stderr, "sending fread %03d length %03d...\n", msg.param3, msg.length );
        // send it
        cmd_hton( &msg );
        ck_send( dest, (char *)&msg, sizeof(msg) );
    }
    
    // close
    fclose( fd );
    //fprintf(stderr, "done.\n", msg.buffer );
    return TRUE;
}



//-----------------------------------------------------------------------------
// name: cmd_send_connect()
// desc: ...
//-----------------------------------------------------------------------------
ck_socket cmd_send_connect( const char * host, int port )
{
    // log
    BB_log( BB_LOG_INFO, "cmd connect: %s:%i", host, port );

    ck_socket sock = ck_tcp_create( 0 );
    if( !sock )
    {
        BB_log( BB_LOG_SYSTEM, "cannot open socket to send command..." );
        return NULL;
    }

    if( strcmp( host, "127.0.0.1" ) )
        BB_log( BB_LOG_INFO, "connecting to %s on port %i via TCP...", host, port );
    
    if( !ck_connect( sock, host, port ) )
    {
        BB_log( BB_LOG_SYSTEM, "cannot open TCP socket on %s:%i...", host, port );
        ck_close( sock );
        return NULL;
    }
    
    ck_send_timeout( sock, 0, 2000000 );

    return sock;
}




//-----------------------------------------------------------------------------
// name: cmd_send_cmd()
// desc: ...
//-----------------------------------------------------------------------------
int cmd_send_cmd( int argc, char ** argv, t_TAPINT & i, const char * host, int port,
                  int * is_otf )
{
    TapsNetMsg msg;
 //   g_sigpipe_mode = 1;
    int tasks_total = 0, tasks_done = 0;
    ck_socket dest = NULL;
    if( is_otf ) *is_otf = TRUE;

    // log
    BB_log( BB_LOG_INFO, "examining cmd command '%s'...", argv[i] );

    if( !strcmp( argv[i], "--add" ) || !strcmp( argv[i], "+" ) )
    {
        if( ++i >= argc )
        {
            BB_log( BB_LOG_SYSTEM, "not enough arguments following [add]..." );
            goto error;
        }

        if( !(dest = cmd_send_connect( host, port )) ) return 0;
        BB_pushlog();
        do {
            // log
            BB_log( BB_LOG_INFO, "sending file '%s' for add...", BirdBrain::getname( argv[i] ).c_str() );
            msg.type = TAPS_MSG_ADD;
            msg.param = 1;
            tasks_done += cmd_send_file( argv[i], msg, "add", dest );
            tasks_total++;
        } while( ++i < argc );
        // log
        BB_poplog();

        if( !tasks_done )
            goto error;
    }
    else if( !strcmp( argv[i], "--remove" ) || !strcmp( argv[i], "-" ) )
    {
        if( ++i >= argc )
        {
            BB_log( BB_LOG_SYSTEM, "not enough arguments following [remove]..." );
            goto error;
        }

        if( !(dest = cmd_send_connect( host, port )) ) return 0;
        BB_pushlog();
        do {
            // log
            BB_log( BB_LOG_INFO, "requesting removal of template '%i'...", atoi(argv[i]) );
            msg.param = atoi( argv[i] );
            msg.type = TAPS_MSG_REMOVE;
            cmd_hton( &msg );
            ck_send( dest, (char *)&msg, sizeof(msg) );
        } while( ++i < argc );
        // log
        BB_poplog();
    }
    else if( !strcmp( argv[i], "--play" ) || !strcmp( argv[i], "_p" ) )
	{
		if( ++i >= argc )
		{
			BB_log( BB_LOG_SYSTEM, "not enough arguments following [play]..." );
			goto error;
		}
		if( !(dest = cmd_send_connect( host, port )) ) return 0;
        BB_pushlog();
        do {
			// log
			BB_log( BB_LOG_INFO, "requesting to play template '%i'...", atoi(argv[i]) );
			msg.param = atoi( argv[i] );
			msg.type = TAPS_MSG_PLAY;
			cmd_hton( &msg );
			ck_send( dest, (char *)&msg, sizeof(msg) );
		} while( ++i < argc );
		// log
        BB_poplog();
    }
    else if( !strcmp( argv[i], "--stop" ) || !strcmp( argv[i], "_s" ) )
    {
        if( ++i >= argc )
        {
			BB_log( BB_LOG_SYSTEM, "not enough arguments following [stop]..." );            
			goto error;
        }
        if( !(dest = cmd_send_connect( host, port )) ) return 0;
        BB_pushlog();
		do { 
			BB_log( BB_LOG_INFO, "requesting to stop template '%i'...", atoi(argv[i]) );
			msg.param = atoi( argv[i] );
			msg.type = TAPS_MSG_STOP;
			cmd_hton( &msg );
			ck_send( dest, (char *)&msg, sizeof(msg) );
		} while( ++i < argc );
        BB_poplog();
    }
    else if( !strcmp( argv[i], "--kill" ) )
    {
        msg.type = TAPS_MSG_KILL;
        msg.param = (i+1)<argc ? atoi(argv[++i]) : 0;
        cmd_hton( &msg );
        ck_send( dest, (char *)&msg, sizeof(msg) );
    }
    else if( !strcmp( argv[i], "--time" ) )
    {
        if( !(dest = cmd_send_connect( host, port )) ) return 0;
        msg.type = TAPS_MSG_TIME;
        msg.param = 0;
        cmd_hton( &msg );
        ck_send( dest, (char *)&msg, sizeof(msg) );
    }
    else if( !strcmp( argv[i], "--status" ) || !strcmp( argv[i], "^" ) )
    {
        if( !(dest = cmd_send_connect( host, port )) ) return 0;
        msg.type = TAPS_MSG_STATUS;
        msg.param = 0;
        cmd_hton( &msg );
        ck_send( dest, (char *)&msg, sizeof(msg) );
    }
    else
    {
        if( is_otf ) *is_otf = FALSE;
        return 0;
    }
        
    // send
    msg.type = TAPS_MSG_DONE;
    cmd_hton( &msg );
    // log
    BB_log( BB_LOG_INFO, "cmd sending request..." );
    ck_send( dest, (char *)&msg, sizeof(msg) );

    // set timeout
    ck_recv_timeout( dest, 0, 2000000 );
    // log
    BB_log( BB_LOG_INFO, "cmd awaiting reply..." );
    // reply
    if( ck_recv( dest, (char *)&msg, sizeof(msg) ) )
    {
        cmd_ntoh( &msg );
        if( !msg.param )
        {
            BB_log( BB_LOG_SYSTEM, "(remote) operation failed (sorry)" );
            BB_log( BB_LOG_SYSTEM, "...(reason: %s)", 
                ( strstr( (char *)msg.buffer, ":" ) ? strstr( (char *)msg.buffer, ":" ) + 1 : (char *)msg.buffer ) );
        }
        else
        {
            BB_log( BB_LOG_INFO, "reply received..." );
        }
    }
    else
    {
        BB_log( BB_LOG_SYSTEM, "remote operation timed out..." );
    }
    // close the sock
    ck_close( dest );
    
    // exit
    // exit( msg.param );

    return 1;
    
error:
    
    // if sock was opened
    if( dest )
    {
        msg.type = TAPS_MSG_DONE;
        cmd_hton( &msg );
        ck_send( dest, (char *)&msg, sizeof(msg) );
        ck_close( dest );
    }
    
    // exit( 1 );

    return 0;
}


//-----------------------------------------------------------------------------
// name: taps_signal_pipe()
// desc: ...
//-----------------------------------------------------------------------------
extern "C" void taps_signal_pipe( int sig_num )
{
    fprintf( stderr, "[taps]: sigpipe handled - broken pipe (no connection)...\n" );
}


//-----------------------------------------------------------------------------
// name: cmd_cb()
// desc: ...
//-----------------------------------------------------------------------------
void * cmd_cb( void * p )
{
    TapsNetMsg msg;
    TapsNetMsg ret;
    ck_socket client;
    int n;

    // catch SIGINT
    // signal( SIGINT, signal_int );
#ifndef __PLATFORM_WIN32__
    // catch SIGPIPE
    signal( SIGPIPE, taps_signal_pipe );
#endif

    while( true )
    {
        client = ck_accept( taps_sock );
        if( !client )
        {
            BB_log( BB_LOG_SYSTEM, "socket error during accept()..." );
            usleep( 40000 );
            ck_close( client );
            continue;
        }
        msg.clear();
        // set time out
        ck_recv_timeout( client, 0, 5000000 );
        n = ck_recv( client, (char *)&msg, sizeof(msg) );
        cmd_ntoh( &msg );
        if( n != sizeof(msg) )
        {
            BB_log( BB_LOG_SYSTEM, "0-length packet..." );
            usleep( 40000 );
            ck_close( client );
            continue;
        }
        
        if( msg.header != TAPS_NET_HEADER )
        {
            BB_log( BB_LOG_SYSTEM, "header mismatch - possible endian lunacy..." );
            ck_close( client );
            continue;
        }

        while( msg.type != TAPS_MSG_DONE )
        {
            if( !cmd_process_msg( &msg, FALSE, client ) )
            {
                ret.param = FALSE;
                strcpy( (char *)ret.buffer, "message process failed" );
                while( msg.type != TAPS_MSG_DONE && n )
                {
                    n = ck_recv( client, (char *)&msg, sizeof(msg) );
                    cmd_ntoh( &msg );
                }
                break;
            }
            else
            {
                ret.param = TRUE;
                strcpy( (char *)ret.buffer, "success" );
                n = ck_recv( client, (char *)&msg, sizeof(msg) );
                cmd_ntoh( &msg );
            }
        }
        
		BB_log( BB_LOG_FINE, "Sending reply" );
        cmd_hton( &ret );
        ck_send( client, (char *)&ret, sizeof(ret) );
        ck_close( client );
    }
    
    return NULL;
}
