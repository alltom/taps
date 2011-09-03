// Copyright (c)Victor Lazzarini, 1997-2004
// See License.txt for a disclaimer of all warranties
// and licensing information

#include <SndObj/AudioDefs.h>
#include <string.h>
#include <math.h>
#include <time.h>
#ifndef WIN32
#include <unistd.h>
#endif

int
main(int argc, char** argv){

    if(argc != 7) {
	 cout << 
	"sinus infile.wav outfile.wav timestr thresh intracks outracks\n";
     return 1;
	}
    time_t ts, te;
	char* infile = argv[1];
	char* outfile = argv[2];	
	float stratio = atof(argv[3]);  // time-stretch ratio
	float thresh = atof(argv[4]);  // analysis threshold
	int intracks = atoi(argv[5]);  // analysis max number of tracks
	int outracks = atoi(argv[6]);  // synthesis
	int fftsize = 1024;     // FFT analysis size
	int decimation = 256;      // analysis hopsize
	int interpolation = (int)(decimation*stratio); // synthesis hopsize  
	float scale = 2.f;   // scaling factor
	
    time(&ts);

	// SndObj objects set-up  
	
	SndThread thread;    //  processing thread
	
	HarmTable table(10000, 1, 1, 0.75);    // cosine wave
	 
	HammingTable window(fftsize, 0.5); // hanning window
	
	// input sound
	SndWave input(infile,READ,1,16,0,0.f,decimation);
	SndIn   insound(&input, 1, decimation);
	
	// IFD analysis
	IFGram ifgram(&window,&insound,1.f,fftsize,decimation);
	// Sinusoidal analysis
	SinAnal sinus(&ifgram,thresh,intracks, 1, 3);
	// Sinusoidal resynthesis
	SinSyn  synth(&sinus,outracks,&table,scale,interpolation);
	
	// output sound
	SndWave output(outfile, OVERWRITE,1,16,0,0.f,interpolation);
	output.SetOutput(1, &synth);
	
	// sound thread set-up
	
	thread.AddObj(&insound);
	thread.AddObj(&ifgram);
	thread.AddObj(&sinus);
	thread.AddObj(&synth);
	thread.AddObj(&input, SNDIO_IN);
	thread.AddObj(&output, SNDIO_OUT); 
	
	thread.ProcOn();
	
	while(!input.Eof());
	
	thread.ProcOff();
     
	time(&te);
	cout << " process time (secs): " << (te -ts) << "\n";
	return 0;
}
