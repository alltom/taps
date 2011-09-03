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
// file: taps_cmd.h
// desc: on-the-fly programming utilities, adapted from chuck_otf.h
//
// author: Ge Wang (gewang@cs.princeton.edu)
//         Perry R. Cook (prc@cs.princeton.edu)
//		   Ananya Misra (amisra@cs.princeton.edu)
// date: December 2006
//-----------------------------------------------------------------------------
#ifndef __TAPS_CMD_H__
#define __TAPS_CMD_H__

#include "taps_def.h"
#include "util_network.h" // from chuck
#include <memory.h>


// defines
#define TAPS_NET_HEADER      0x8c8cc8c8		// same as chuck
// buffer size
#define TAPS_NET_BUFFER_SIZE 512


enum TapsMsgTypes
{
	TAPS_MSG_ADD = 1,
	TAPS_MSG_REMOVE,
	TAPS_MSG_PLAY,
	TAPS_MSG_STOP,
	TAPS_MSG_KILL,
	TAPS_MSG_TIME,
	TAPS_MSG_STATUS,
	TAPS_MSG_DONE
};


//-----------------------------------------------------------------------------
// name: struct TapsNetMsg
// desc: ...
//-----------------------------------------------------------------------------
struct TapsNetMsg
{
    t_TAPUINT header;
    t_TAPUINT type;
    t_TAPUINT param;
    t_TAPUINT param2;
    t_TAPUINT param3;
    t_TAPUINT length;
    char buffer[TAPS_NET_BUFFER_SIZE];

    TapsNetMsg() { this->clear(); }
    void clear() { header = TAPS_NET_HEADER; type = param = param2 = param3 = length = 0;
                   memset( buffer, 0, sizeof(buffer) ); }
};


// host to network
void otf_hton( TapsNetMsg * msg );
// network to host
void otf_ntoh( TapsNetMsg * msg );

// process incoming message
t_TAPUINT cmd_process_msg( TapsNetMsg * msg, t_CKBOOL immediate, void * data );

// send command
int cmd_send_cmd( int argc, char ** argv, t_TAPINT & i, const char * host, int port, int * is_otf = NULL );
// send file to remote host
int cmd_send_file( const char * filename, TapsNetMsg & msg, const char * op, ck_socket sock );
// connect
ck_socket cmd_send_connect( const char * host, int port );

// callback
void * cmd_cb( void * p );


#endif
