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
// name: ui_main.cpp
// desc: entry point for taps
//
// authors: Ananya Misra (amisra@cs.princeton.edu)
//          Philip Davidson (philipd@cs.princeton.edu)
//          Ge Wang (gewang@cs.princeton.edu)
//          Perry R. Cook (prc@cs.princeton.edu)
// date: Spring 2006
//-----------------------------------------------------------------------------
#include "ui_audio.h"
#include "ui_analysis.h"
#include "ui_synthesis.h"
#include "ui_control.h"
#include "ui_group.h"
#include "ui_library.h"
#include "ui_search.h"
#ifdef __TAPS_SCRIPTING_ENABLE__
#include "ui_scripting.h"
#endif

#include "audicle_elcidua.h"
#include "audicle_def.h"
#include "audicle_gfx.h"
#include "audicle.h"

#include "taps_cmd.h"

#include <stdlib.h>
#include <time.h>

#ifdef __PLATFORM_WIN32__
  #include <direct.h>
#else
  #include <unistd.h>
#endif


// version
#define TAPS_VERSION "0.1.0.7rc"

// function prototype
void print_usage();
void version();

#ifdef __MACOSX_CORE__
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>

// free
void osx_increase_file_handles()
{
    struct rlimit limit;
    int err;

    err = getrlimit(RLIMIT_NOFILE, &limit);
    if (err == 0) {
        limit.rlim_cur = RLIM_INFINITY;
        (void) setrlimit(RLIMIT_NOFILE, &limit);
    }
}
#endif

#ifdef __PLATFORM_LINUX__
#include <gtk/gtk.h>

int g_argc;
char ** g_argv;

// gtk thread
void * cb_gtk( void * )
{
    // init
    gtk_init( &g_argc, &g_argv );
    // just run it
    gtk_main();
    return NULL;
}

CHUCK_THREAD g_gtk_id;

#endif

// network stuff
extern ck_socket taps_sock;
t_TAPINT taps_port = 9999;
CHUCK_THREAD taps_tid_cmd = 0;
char taps_host[256] = "127.0.0.1";


//-----------------------------------------------------------------------------
// name: main()
// desc: entry point
//-----------------------------------------------------------------------------
int main( int argc, char ** argv )
{
    // you know
    srand( time( NULL ) );
    
    // fullscreen
    t_TAPBOOL fullscreen = TRUE;
    // log set by user?
    t_TAPBOOL logset = FALSE;
    // rtaudio channels requested
	t_TAPUINT channels_out = 2;
	t_TAPUINT channels_in = 1;
	// whether the gui is used
	t_TAPBOOL gui = BirdBrain::our_use_gui = TRUE;
	
    // remember
#ifdef __PLATFORM_WIN32__
    char buffer[MAX_PATH];
    DWORD ret = GetEnvironmentVariable( "USERPROFILE", buffer, MAX_PATH );
    if( ret )
    {
        BirdBrain::our_start_dir = buffer; // ISSUE: unicode
        BirdBrain::our_start_dir += "\\Desktop";
    }
    else
        BirdBrain::our_start_dir = "C:\\"; //_getcwd( NULL, 0 );
#else
    BirdBrain::our_start_dir = getcwd( NULL, 0 );
#endif

#ifdef __MACOSX_CORE__
    // limits
    osx_increase_file_handles();
    // sample rate
    BirdBrain::our_srate = 44100;
    // no
    fullscreen = TRUE;
#endif

#ifdef __PLATFORM_WIN32__
    // sample rate
    BirdBrain::our_srate = 44100;
#endif

#ifdef __PLATFORM_LINUX__
    fullscreen = FALSE;
    // sample rate
    BirdBrain::our_srate = 48000;
    // copy
    g_argc = argc;
    g_argv = argv;
    // init
    gtk_init( &g_argc, &g_argv );
    // go
    // pthread_create( &g_gtk_id, NULL, cb_gtk, NULL );
#endif

    // parse arguments
    t_TAPINT index = 1;
    while( index < argc )
    {               
        // sample rate
        if( !strcmp( argv[index], "--srate" ) )
        {
            if( ++index < argc )
            {
                BirdBrain::our_srate = atoi( argv[index] );
                BB_log( BB_LOG_INFO, "(main) Setting sample rate to %i Hz", BirdBrain::srate() );
            }
        }
        // frame rate
        else if( !strcmp( argv[index], "--frate" ) )
        {
            if( ++index < argc )
            {
                BirdBrain::our_frame_rate = atoi( argv[index] );
                BB_log( BB_LOG_INFO, "(main) Setting frame rate to %i frames per second", BirdBrain::frame_rate() );
            }
        }
        // log level
        else if( !strcmp( argv[index], "--loglevel" ) )
        {
            if( ++index < argc )
            {
                BB_setlog( atoi( argv[index] ) );
                logset = TRUE;
            }
        }
		// number of rtaudio output channels
		else if( !strcmp( argv[index], "--outchannels" ) )
		{
			if( ++index < argc )
			{
				channels_out = atoi( argv[index] );
				BB_log( BB_LOG_INFO, "(main) %u real-time audio output channels requested", channels_out );
			}
		}
		// number of rtaudio input channels
		else if( !strcmp( argv[index], "--inchannels" ) )
		{
			if( ++index < argc )
			{
				channels_in = atoi( argv[index] );
				BB_log( BB_LOG_INFO, "(main) %u real-time audio input channels requested", channels_in );
			}
		}
		// use blocking audio (not reliable)
		else if( !strcmp( argv[index], "--blocking" ) )
		{
			BirdBrain::our_rtaudio_blocking = TRUE;
			BB_log( BB_LOG_INFO, "(main) Using blocking audio" );
		}
		// rtaudio buffer size
		else if( !strcmp( argv[index], "--bufsize" ) )
		{
			if( ++index < argc )
			{
				BirdBrain::our_rtaudio_buffer_size = atoi( argv[index] ); 
				BB_log( BB_LOG_INFO, "(main) Setting rtaudio buffer size to %i", BirdBrain::rtaudio_buffer_size() ); 
			}
		}
		// maximum record buffer size in seconds
		else if( !strcmp( argv[index], "--maxrecord" ) )
		{
			if( ++index < argc )
			{
				BirdBrain::our_record_buffer_size = atof( argv[index] );
				BB_log( BB_LOG_INFO, "(main) Setting max record buffer size to %f seconds", 
						BirdBrain::record_buffer_size() );
			}
		}
		// over tracking (sinusoidal analysis)
		else if( !strcmp( argv[index], "--overtracking" ) ) 
		{
			if( ++index < argc )
			{
				t_TAPFLOAT ot = atof( argv[index] ); 
				if( ot > 1 )
					BirdBrain::our_over_tracking = ot; 
				BB_log( BB_LOG_INFO, "(main) Setting over tracking factor to %f", BirdBrain::over_tracking() );
			}
		}
        // preprocess mode (always the last thing)
        else if( !strcmp( argv[index], "--preprocess" ) )
        {
            BB_log( BB_LOG_SYSTEM, "Preprocess mode" ); 
            Driver jill;
            jill.set( 512, 4096, 1 );
            BirdBrain::our_hop_size = 512 / 4;  
            while( ++index < argc )
            {
                if( !jill.open( argv[index] ) )
                {
                    BB_log( BB_LOG_SYSTEM, "(main) Driver could not open file %s", argv[index] );
                    continue;
                }
                
                jill.preprocess(); 
                jill.brake(); 
            }

            return 0;
        }
        // set working directory
		else if( !strcmp( argv[index], "--dir" ) )
        {
            if( ++index < argc )
            {
                BB_log( BB_LOG_SYSTEM, "override working directory: '%s'", argv[index] );
                BirdBrain::our_start_dir = argv[index];
            }
        }
		// no gui
		else if( !strcmp( argv[index], "--nogui" ) )
		{
			BirdBrain::our_use_gui = gui = FALSE;
			BB_log( BB_LOG_SYSTEM, "running in no GUI mode" );
		}
		else if( !strcmp( argv[index], "--whitebg" ) )
		{
			BirdBrain::our_white_bg = TRUE;
			BB_log( BB_LOG_SYSTEM, "setting all background colors to white" );
		}
        // only show version and quit
        else if( !strcmp( argv[index], "--version" ) )
        {
            version();
            exit( 2 );
        }
        // only show usage and quit
        else if( !strcmp( argv[index], "--about" ) || !strcmp( argv[index], "--help" ) )
        {
            print_usage();
            exit( 2 );
        }
		// non
        else
        {
			// otf / command line
			int is_otf = FALSE;
            if( cmd_send_cmd( argc, argv, index, taps_host, taps_port, &is_otf ) )
				exit(0);
			
            // is otf
            if( is_otf ) exit( 1 );

			// nothing
			print_usage();
            BB_log( BB_LOG_SYSTEM, "error: invalid argument '%s'", argv[index] );
            exit( 2 );
        }

        index++; 
    }
    
    // set last to start
    BirdBrain::our_last_open_dir = BirdBrain::our_start_dir;
    BirdBrain::our_last_save_dir = BirdBrain::our_start_dir;

    // normal mode
    // print info
    version();
    // set log
    if( !logset )
        BB_setlog( BB_LOG_INFO );

    // start audio
    if( !AudioCentral::instance()->init( BirdBrain::srate(), BirdBrain::rtaudio_buffer_size(), 128, 8, 
		channels_out, channels_in, BirdBrain::record_buffer_size() ) )
	{
		BB_log( BB_LOG_SYSTEM_ERROR, "Real-time audio initialization error; quitting!" );
		return -1;
	}
	// gui
	if( gui )
	{
		// initialize graphics
		if( !AudicleGfx::init() )
		{
			BB_log( BB_LOG_SYSTEM_ERROR, "Graphics initialization error; quitting!" );
			return -2;
		}
		// initialize main window
		if( !AudicleWindow::main()->init( 1024, 768, 0, 0, "tapestrea", fullscreen ) )
		{
			BB_log( BB_LOG_SYSTEM_ERROR, "Audicle window initialization error; quitting!" );
			return -3;
		}

		// launch audicle
		Audicle::instance()->init();
		// add faces to the audicle
		Audicle::instance()->add( new UIAnalysis );
		Audicle::instance()->add( new UISynthesis );
		Audicle::instance()->add( new UIGroup );
		Audicle::instance()->add( new UIControl );
		Audicle::instance()->add( new UISearch );
		Audicle::instance()->add( new ElciduaFace );
		//Audicle::instance()->add( new ElciduaFace );
		//Audicle::instance()->add( new UITreesynth );

		// move to the initial face
		Audicle::instance()->move_to( (t_TAPUINT)0 );
	}

    BB_log( BB_LOG_SYSTEM, "initial working directory: %s", BirdBrain::our_start_dir == "" ? "NONE" : 
                                                            BirdBrain::our_start_dir.c_str() );  

    BB_log( BB_LOG_SYSTEM, "TAPESTREA system ready..." );
    
	// scripting engine and other inits in nogui mode
	if( !gui )
	{
		// start up script engine (usually happens in synthesis face)
#ifdef __TAPS_SCRIPTING_ENABLE__
		// log
		BB_log( BB_LOG_INFO, "initializing scripting engine..." );
		BB_pushlog();

		// start it
		ScriptCentral::startup();
		// start on last bus
		t_TAPUINT which_bus = AudioCentral::instance()->num_bus() - 2;
		// turn on frankenstein
		ScriptCentral::engine()->start_vm( AudioCentral::instance()->bus( which_bus ) );

		// pop log
		BB_poplog();
		BB_log( BB_LOG_INFO, "all set!" );
#endif
	}

	// command line control
	// start tcp server
	taps_sock = ck_tcp_create( 1 );
	if( !taps_sock || !ck_bind( taps_sock, taps_port ) || !ck_listen( taps_sock, 10 ) )
	{
		BB_log( BB_LOG_SYSTEM, "cannot bind to tcp port %i...", taps_port );
		ck_close( taps_sock );
		taps_sock = NULL;
	}
	else
	{
	// create cb thread
#ifndef __PLATFORM_WIN32__
		pthread_create( &taps_tid_cmd, NULL, cmd_cb, NULL );
#else
		taps_tid_cmd = CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE)cmd_cb, NULL, 0, 0 );
#endif
	}

	// usage
	print_usage();

	// loop
	if( gui )
	{
		// main loop
		AudicleGfx::loop();
	}
	else
	{
		while( true )
			usleep( 100000 );	
	}

    return 0;
}



//-----------------------------------------------------------------------------
// name: print_usage()
// desc: print usage
//-----------------------------------------------------------------------------
void print_usage()
{
    fprintf( stderr, "\n----------------------------------------------------\n" );
    fprintf( stderr, "'Ctrl + arrow' - switch faces\n" );
    fprintf( stderr, "'Ctrl + g' - toggle full screen\n" );
    fprintf( stderr, "'Ctrl + q' or 'apple + q' - quit\n" );
    fprintf( stderr, "'up or down arrow' - change volume\n" );
    fprintf( stderr, "'h' - toggle highlighting\n" );
    fprintf( stderr, "'1', '2', '3', ... - toggle zoom on different regions\n" );
    fprintf( stderr, "'0', 'esc' - zoom out\n" );
    fprintf( stderr, "'r' - toggle display of slider ranges\n" );
    fprintf( stderr, "-----------------Analysis---------------------------\n" );
    fprintf( stderr, "'L' - toggle spectrogram selection rectangle line width\n" );
    fprintf( stderr, "-----------------Synthesis--------------------------\n" );
    fprintf( stderr, "'Backspace' - delete selected template from library\n" );
    fprintf( stderr, "'N' - create new mixed bag\n" );
    fprintf( stderr, "'q' or 'z' - toggle display of quantization markers\n" );
    fprintf( stderr, "Right clicking on a template plays/stops it\n" );
    fprintf( stderr, "Drag to move templates onto/off/within a timeline\n" );
    fprintf( stderr, "-----------------Control Face-----------------------\n" );
    fprintf( stderr, "Bus 0 - analysis\n" );
    fprintf( stderr, "Bus 2 - synthesis\n" );
    fprintf( stderr, "Bus 3 - group\n" );
#ifdef __TAPS_SCRIPTING_ENABLE__
    fprintf( stderr, "Bus 7 - chuck scripts (default)\n" );
#endif
    fprintf( stderr, "-----------------Search Face-----------------------\n" );
    fprintf( stderr, "All hidden commands\n" );
    fprintf( stderr, "-----------------Command line----------------------\n" );
    fprintf( stderr, "'--srate N'\n" );
    fprintf( stderr, "    set working sample rate to N Hz\n" );
    fprintf( stderr, "'--frate N'\n" );
    fprintf( stderr, "    set maximum frame rate to N Hz\n" );
	fprintf( stderr, "'--outchannels N'\n" );
	fprintf( stderr, "    set number of real-time audio output channels to N\n" );
	fprintf( stderr, "'--inchannels N'\n" );
	fprintf( stderr, "    set number of real-time audio input channels to N\n" );
	//fprintf( stderr, "'--blocking'\n" );
	//fprintf( stderr, "    use blocking audio\n" );
	fprintf( stderr, "'--bufsize N'\n" );
	fprintf( stderr, "    set real-time audio output buffer size to N\n" );
	fprintf( stderr, "'--maxrecord x'\n" );
	fprintf( stderr, "    set maximum number of seconds recorded to x\n" );
	fprintf( stderr, "'--dir XXX'\n" );
	fprintf( stderr, "    set initial working directory to XXX\n" );
	fprintf( stderr, "'--overtracking x'\n" );
	fprintf( stderr, "    set sinusoidal analysis overtracking factor to x (float)\n" );
	fprintf( stderr, "    this finds <#sinetracks> * x peaks, but only saves <#sinetracks> tracks\n"); 
	fprintf( stderr, "'--nogui'\n" );
	fprintf( stderr, "    control synthesis from command line, with no gui\n" );
	fprintf( stderr, "'--whitebg'\n" );
	fprintf( stderr, "'   set background color of every face to white\n" );
    fprintf( stderr, "'--version'\n" );
    fprintf( stderr, "    print version number and target\n" );
    fprintf( stderr, "'--about' or '--help'\n" );
    fprintf( stderr, "    print key mapping, usage, and version information\n" );
    fprintf( stderr, "'--preprocess file1.wav file2.wav ... fileN.wav'\n" );
    fprintf( stderr, "    preprocess each wav file\n" );
    fprintf( stderr, "    outputs: file1.pp, file1.fft, file2.pp, file2.fft, ...\n" );
    fprintf( stderr, "    later in analysis face, load the .pp file\n" );
    fprintf( stderr, "    (this should be specified last, if at all)\n" );
    fprintf( stderr, "----------------------------------------------------\n" );
    version();
    fprintf( stderr, "----------------------------------------------------\n" );
}


//-----------------------------------------------------------------------------
// name: version()
// desc: ...
//-----------------------------------------------------------------------------
void version()
{
    fprintf( stderr, "\n" );
    fprintf( stderr, "TAPESTREA version: %s\n", TAPS_VERSION );
#if defined(__PLATFORM_WIN32__)
    fprintf( stderr, "   exe target: microsoft win32\n" );
#elif defined(__WINDOWS_DS__)
    fprintf( stderr, "   exe target: microsoft win32 + cygwin\n" );
#elif defined(__LINUX_ALSA__)
    fprintf( stderr, "   exe target: linux (alsa)\n" );
#elif defined(__LINUX_OSS__)
    fprintf( stderr, "   exe target: linux (oss)\n" );
#elif defined(__LINUX_JACK__)
    fprintf( stderr, "   exe target: linux (jack)\n" );
#elif defined(__MACOSX_UB__)
    fprintf( stderr, "   exe target: mac os x : universal binary\n" );
#elif defined(__MACOSX_CORE__) && defined(__LITTLE_ENDIAN)
    fprintf( stderr, "   exe target: mac os x : intel\n" );
#elif defined(__MACOSX_CORE__)
    fprintf( stderr, "   exe target: mac os x : powerpc\n" );
#else
    fprintf( stderr, "   exe target: uh... unknown\n" );
#endif
    fprintf( stderr, "   http://taps.cs.princeton.edu/\n\n" );
}


