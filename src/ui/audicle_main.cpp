//-----------------------------------------------------------------------------
// name: audicle_main.cpp
// desc: entry point for audicle
//
// authors: Ge Wang (gewang@cs.princeton.edu)
//          Perry R. Cook (prc@cs.princeton.edu)
//          Philip Davidson (philipd@cs.princeton.edu)
//          Ananya Misra (amisra@cs.princeton.edu)
// date: 2/16/2004
//-----------------------------------------------------------------------------
#include "audicle_def.h"
#include "audicle_gfx.h"
#include "audicle.h"

#include "ui_audio.h"
#include "ui_analysis.h"
#include "ui_treesynth.h"
#include "ui_synthesis.h"
#include "ui_control.h"
#include "ui_library.h"
#include "ui_search.h"
#include "audicle_elcidua.h"

#include <stdlib.h>
#include <time.h>


// version
#define TAPS_VERSION "1.0.0.0"

// function prototype
void print_usage();

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


//-----------------------------------------------------------------------------
// name: main()
// desc: entry point
//-----------------------------------------------------------------------------
int main( int argc, char ** argv )
{
    // you know
    srand( time( NULL ) );
    // info
    BB_log( BB_LOG_SYSTEM, "Tapestrea version: %s", TAPS_VERSION );
    BB_setlog( BB_LOG_INFO ); 
#ifdef __MACOSX_CORE__
    // limits
    osx_increase_file_handles();
#endif

	// parse arguments
	int index = 1;
	while( index < argc )
	{				
		// preprocess mode (always the last thing)
		if( !strcmp( argv[index], "--preprocess" ) )
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
		// log level
		else 
			BB_setlog( atoi( argv[index] ) ); 

		index++; 
	}

	// normal mode
	// start audio
    AudioCentral::instance()->init( 44100, 1024, 128 );
    // initialize graphics
    AudicleGfx::init();
    // initialize main window
    AudicleWindow::main()->init( 1024 * 2, 768 * 2, 0, 0, "tapestrea", TRUE );

    // launch audicle
    Audicle::instance()->init();
    // add faces to the audicle
    Audicle::instance()->add( new UIAnalysis );
    Audicle::instance()->add( new UISynthesis );
    Audicle::instance()->add( new UISearch );
    Audicle::instance()->add( new UITreesynth );
    Audicle::instance()->add( new ElciduaFace );
    Audicle::instance()->add( new UIControl );
    // move to the initial face
    Audicle::instance()->move_to( (t_CKUINT)0 );

    BB_log( BB_LOG_SYSTEM, "TAPESTREA system ready..." );
    
    // print usage
    print_usage();

    // main loop
    AudicleGfx::loop();

    return 0;
}




//-----------------------------------------------------------------------------
// name: print_usage()
// desc: print usage
//-----------------------------------------------------------------------------
void print_usage()
{
    fprintf( stderr, "----------------------------------------------------\n" );
    fprintf( stderr, "Tapestrea (1.0.0.0)\n" );
    fprintf( stderr, "----------------------------------------------------\n" );
    fprintf( stderr, "'Ctrl + arrow' - switch faces\n" );
    fprintf( stderr, "'Ctrl + g' - toggle full screen\n" );
    fprintf( stderr, "'Ctrl + q' or 'Apple + q' - quit\n" );
    fprintf( stderr, "'up or down arrow' - change volume\n" );
    fprintf( stderr, "'h' - toggle highlighting\n" );
    fprintf( stderr, "'1', '2', '3', ... - toggle zoom on different regions\n" );
    fprintf( stderr, "'0', 'esc' - zoom out\n" );
    fprintf( stderr, "'r' - toggle display of slider ranges\n" );
    fprintf( stderr, "----------------Analysis---------------------------\n" );
    fprintf( stderr, "'L' - toggle spectrogram selection rectangle line width\n" );
    fprintf( stderr, "----------------Synthesis--------------------------\n" );
    fprintf( stderr, "'Backspace' - delete selected template from library\n" );
    fprintf( stderr, "'N' - create new mixed bag\n" );
    fprintf( stderr, "'q' or 'z' - toggle display of quantization markers\n" );
    fprintf( stderr, "Right clicking on a template plays/stops it\n" );
    fprintf( stderr, "Drag to move templates onto/off/within a timeline\n" );
    fprintf( stderr, "-----------------Mixer Face-------------------------\n" );
    fprintf( stderr, "'Ctrl + down arrow' - to access\n" );
    fprintf( stderr, "Bus 0 - analysis\n" );
    fprintf( stderr, "Bus 1 - treesynth face\n" );
    fprintf( stderr, "Bus 2 - synthesis\n" );
    fprintf( stderr, "Bus 7 - ChucK scripts\n" );
    fprintf( stderr, "----------------Treesynth Face---------------------\n" );
    fprintf( stderr, "'Ctrl + any arrow' - leave at once\n" );
	fprintf( stderr, "----------------No face (preprocess)---------------\n" );
	fprintf( stderr, "'taps -pp file1.wav file2.wav ... fileN.wav' - \n" );
	fprintf( stderr, "    preprocess each wav file\n" );
	fprintf( stderr, "    outputs: file1.pp, file1.fft, file2.pp, file2.fft, ...\n" );
	fprintf( stderr, "    later in analysis face, load the .pp file\n" );
    fprintf( stderr, "----------------------------------------------------\n" );
}