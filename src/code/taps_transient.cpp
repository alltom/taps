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
// file: taps_transient.cpp
// desc: taps transient extractor
//
// author: Perry R. Cook (prc@cs.princeton.edu)
//         Matt Hoffman (mdhoffma@cs.princeton.edu)
//         Ananya Misra (amisra@cs.princeton.edu)
//         Ge Wang (gewang@cs.princeton.edu)
// date: Spring 2005
//-----------------------------------------------------------------------------
#include "ui_audio.h"
#include "taps_transient.h"
#include "taps_treesynth.h"

bool TransientExtractor::remove_transients( const char *outfilename, std::vector< t_TAPUINT > rmindexs )
{
    TransLoc tl;
    int tranLen, gapLen, postgapLen, overlap, pad = allowableGap / 4, tstart, mingapLen;
    //float prenoise, postnoise, noise;
    //int count, noiselen = allowableGap; 

    Treesynth ts;
    Tree * ts_tree = NULL; // gets deleted from inside treesynth
    int levels;
    SAMPLE * syn;

    SAMPLE * buffer = NULL;

    // Read original input
	AudioSrcBuffer * orig;
	if( filename.size() > 0 ) {
		AudioSrcFile * f = new AudioSrcFile;
		f->open( filename.c_str(), 0, 0, FALSE );
		orig = f; 
	}
	else {
		AudioSrcMic * m = new AudioSrcMic;
		m->open( 0, 0, FALSE );
		orig = m;
	}
    t_TAPUINT size = orig->frames();
    SAMPLE * working = new SAMPLE[size];
    orig->mtick( working, size );

    // Open output file
    SF_INFO sf_info_write; 
	if( orig->type() == AudioSrcBuffer::BUFF_FILE ) 
		sf_info_write = ((AudioSrcFile *)orig)->info();
	else {
		memset( &sf_info_write, 0, sizeof(sf_info_write) );
		sf_info_write.samplerate = orig->srate();
		sf_info_write.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16; 
	}
    sf_info_write.channels = 1;
//    SNDFILE * out = sf_open( outfilename, SFM_WRITE, &sf_info_write );
	SNDFILE * out = AudioCentral::instance()->m_cachemanager->opensf( outfilename, SFM_WRITE, &sf_info_write );

	// get rid of orig
	if( orig->type() == AudioSrcBuffer::BUFF_FILE )
		((AudioSrcFile *)orig)->close();
	SAFE_DELETE( orig );

    // "bitmap" of transients
    bool * bmp = new bool[size];
    for( int b = 0; b < size; b++ ) 
        bmp[b] = false;
    for( int t = 0; t < transients.size(); t++ )
    {
        tl = transients[t]; 
        for( int b = estart + tl.start; b < estart + tl.end; b++ )
            bmp[b] = true;
    }

    for( int r = 0; r < rmindexs.size(); r++ )
    {
        tl = transients[rmindexs[r]];
        tstart = tl.start - pad; 
        if( tstart < 0 ) tstart = 0;
        tranLen = tl.end - tstart;
        mingapLen = tranLen/4; 
        
        // inefficiently find size of nearest preceding transient-free zone
        int e = estart + tstart;
        gapLen = 0;
        while( gapLen < 2 * tranLen && e > 0 )
        {
            e--;
            if( !bmp[e] )
                gapLen++;
            else if( gapLen <= mingapLen )
                gapLen = 0;
            else
                break;
        }

        // and nearest _next_ free zone
        int f = estart + tl.end;
        postgapLen = 0;
        while( postgapLen < 2 * tranLen && f < size-1 )
        {
            f++;
            if( !bmp[f] )
                postgapLen++;
            else if( postgapLen <= mingapLen )
                postgapLen = 0;
            else 
                break;
        }

        // find final gap length
        bool pre = false, post = false;
        if( postgapLen <= mingapLen )
        {
            pre = true;
        }
        else if( gapLen <= mingapLen )
        {
            post = true;
            gapLen = postgapLen;
        }
        else
            gapLen = postgapLen < gapLen ? postgapLen : gapLen;

        BB_log( BB_LOG_CRAZY, "%s %s %i %i %i", pre ? "true" : "false", post ? "true" : "false", gapLen, 2*tranLen, mingapLen ); 

        if( gapLen <= 0 )
            continue;

        // add non-trans zones
        SAFE_DELETE_ARRAY( buffer );
        buffer = new SAMPLE[gapLen];
        for( int g = 0; g < gapLen; g++ )
        {
            if( pre )
                buffer[g] = working[e + g];
            else if( post )
                buffer[g] = working[f - postgapLen + g];
            else
                buffer[g] = (working[e + g] + working[f - postgapLen + g]) * 0.7;
        }

        levels = lg( gapLen );
        gapLen = (int)::pow(2, levels);
        overlap = gapLen / 2;

        // zero out transient
        memset( working + estart + tstart, 0, tranLen * sizeof( SAMPLE ) );

        // treesynth
        SAFE_DELETE( ts_tree );
        ts_tree = new Tree;
        ts_tree->initialize( levels );
        //memcpy( ts_tree->values(), working + e, gapLen * sizeof( SAMPLE ) );
        memcpy( ts_tree->values(), buffer, gapLen * sizeof( SAMPLE ) );
        ts.tree = ts_tree;
        ts.initialize();
        ts.percentage = 0.8;
        ts.randflip = true;
        ts.stoplevel = 10; // if that's too big, it will stop at the last level automatically

        bool done = false;
        int index = estart + tstart - overlap; 
        if( index < 0 ) index = 0;
        
        if( !ts.setup() ) return false;

        while( !done )
        {
            BB_log( BB_LOG_CRAZY, "a tree" );   
            ts.synth();
            syn = ts.outputSignal();

            // overlap add synthesized signal into working buffer
            for( int i = 0; i < gapLen; i++ )
            {
                if( index + i < size )
                {
                    if( index + i < estart + tstart || index + i >= estart + tl.end )
                        working[index + i] *= 0.5; 
                    working[index + i] += 0.5 * syn[i];
    
                    //working[index + i] += syn[i];
                    //working[index + i] = 0.5 * working[index + i] + 0.5 * syn[i];
                }
                else
                    break;
            }

            index += (gapLen - overlap);
            if( index >= estart + tl.end )
                done = true;
        }
        
        // noise floor?
        /*prenoise = postnoise = noise = 0;
        count = 0;
        int n;
        for( n = estart + tstart <= noiselen ? 0 : estart + tstart - noiselen; 
             n < estart + tstart; 
             n++, count++ )
        {
            prenoise += fabs(working[n]);
        }
        prenoise /= (count > 0 ? count : 1);
        for( n = estart + tl.end, count = 0; 
             n < size && n < estart + tl.end + noiselen; 
             n++, count++ )
        {
            postnoise += fabs(working[n]);
        }
        postnoise /= (count > 0 ? count : 1);
        for( n = estart + tstart, count = 0; n < estart + tl.end; n++, count++ )
        {
            noise += fabs(working[n]);
        }
        noise /= (count > 0 ? count : 1);
        float slope = (postnoise - prenoise)/tranLen;
        if( noise != 0 ) slope /= noise;
        for( n = tstart; n < tl.end; n++ )
        {
            working[estart + n] *= ( slope * n + prenoise );
        }*/

        // sigh of relief
        // fprintf( stderr, "\n\n" );
    }

    // Write out working
    sf_count_t write = sf_write_float( out, working, size );

    // Close stuff
//    sf_close( out );
	AudioCentral::instance()->m_cachemanager->closesf( out );
    SAFE_DELETE_ARRAY( working ); 
    SAFE_DELETE_ARRAY( buffer );

    return true;
}


/*bool TransientExtractor::remove_transients( const char *outfilename, std::vector< t_TAPUINT > rmindexs )
{
    TransLoc tl;
    int noiselen = allowableGap; // noise floor samples
    double sum = 0, peak = 0;
    int count = 0, peakloc = -1;
    SAMPLE * window = NULL;

    // Read original file (libsndfile? audiosrcfile? other?)
    AudioSrcFile * orig = new AudioSrcFile;
    orig->open( filename.c_str(), 0, 0, FALSE );
    t_TAPUINT size = orig->info().frames;
    SAMPLE * working = new SAMPLE[size];
    orig->mtick( working, size );

    SF_INFO sf_info_write = orig->info();
    sf_info_write.channels = 1;
    SNDFILE * out = sf_open( outfilename, SFM_WRITE, &sf_info_write );

    int tstart, tend, pad = allowableGap / 2; 

    // Suppress selected transients
    for( int r = 0; r < rmindexs.size(); r++ )
    {
        // check bounds
        if( rmindexs[r] < 0 && rmindexs[r] >= transients.size() )
        {
            fprintf( stderr, "bad transient index %i\n", rmindexs[r] );
            break;
        }
        
        // get transient location
        tl = transients[rmindexs[r]]; 

        // compute max peak (or trough) in transient
        peak = 0; 
        for( int m = estart + tl.start; m <= estart + tl.end; m++ )
        {
            if( fabs(working[m]) > peak )
            {
                peak = fabs( working[m] );
                peakloc = m;
            }
        }
        if( peak == 0 ) peak = 1; // :-)
        if( peakloc == -1 ) peakloc = estart + (tl.end - tl.start) / 2;
        fprintf( stderr, "peak: %f, peakloc: %i\n", peak, peakloc );

        // pad location to have peak in center
        int left = peakloc - (estart + tl.start);
        int right = estart + tl.end - peakloc;
        if( left < right )
        {
            tstart = peakloc - right - pad;
            tend = tl.end + pad;
        }
        else if( right < left )
        {
            tstart = tl.start - pad;
            tend = peakloc + left + pad;
        }

        //tstart = estart + tl.start - pad;
        //tend = estart + tl.end + pad;
        if( tstart < 0 ) tstart = 0;
        if( tend >= size ) tend = size - 1;

        // compute noise floor (still a hack)
        sum = 0;
        count = 0;
        double max = 0;
        for( int n = tstart <= noiselen ? 0 : tstart - noiselen; n < tstart; n++, count++ )
        {
            sum += fabs(working[n]);
            if( fabs(working[n]) > max )
                max = fabs(working[n]);
        }
        sum /= count; // sum is now an estimate of the noise floor just before the transient
        max = 0.3 * max + 0.7 * sum; 
        fprintf( stderr, "noise floor: %f\nmax: %f\n", sum, max );

        // attenuate (but right now it's just weird)
        SAFE_DELETE_ARRAY( window );
        window = new SAMPLE[tend - tstart + 1];
        hanning( window, tend - tstart + 1 );
        for( int w = 0; w <= tend - tstart; w++ ) {
            window[w] = max/peak + (1 - window[w]) * max/peak; //1 - 0.3 * window[w];
        }
        apply_window( working + tstart, window, tend - tstart + 1 );
        // change window
    
        // try matching average to average! (this comment does not pertain to existing code)
    }

    // Write out working
    sf_count_t write = sf_write_float( out, working, size );

    // Close stuff
    sf_close( out );

    // Return true, actually.
    return true;
}*/


EnvExtractor::EnvExtractor( const char * fn )
{
    filename = fn ? fn : "";
    thresh = 1.0;
    ageamt = .95f;
    allowableGap = 2000;

    attack = .4f;
    decay = .9f;

    envLen = 0;

    envelope = NULL;
    derivs = NULL;

    estart = 0;
    eend = 0;

    envSize = 0;

    read_frame = NULL;
}

EnvExtractor::~EnvExtractor()
{
    SAFE_DELETE_ARRAY( envelope );
    SAFE_DELETE_ARRAY( derivs );
    SAFE_DELETE( read_frame );
}

SAMPLE * EnvExtractor::getEnv()
{
    return envelope;
}

bool EnvExtractor::extract( t_TAPUINT start, t_TAPUINT end )
{
    int buffsize = end - start + 1; // + 1?
    bool need_to_read = FALSE;
    envLen = buffsize;
    if( buffsize % 2) buffsize++;

    // set up frame to read into
    if( read_frame == NULL )
    {
        read_frame = new Frame;
        need_to_read = TRUE;
    }
    if( read_frame->wlen < buffsize )
    {
        read_frame->alloc_waveform( buffsize );
        need_to_read = TRUE;
    }
    
    // read clip from original file if needed
    if( estart != start || eend != end || need_to_read )
    {
        AudioSrcBuffer * input;
		AudioSrcFile file;
		AudioSrcMic mic;
		if( filename.size() > 0 ) 
		{
			if( !file.open( filename.c_str(), start, envLen, TRUE ) )
				return FALSE;
			input = &file;
		}
		else
		{
			if( !mic.open(start, envLen, TRUE) )
				return FALSE;
			input = &mic;
		}

        read_frame->zero();

        // get samples
        input->mtick( read_frame->waveform, buffsize );
        read_frame->wsize = buffsize;
		
		if( filename.size() > 0 )
			file.close();
    }

    transients.clear();

    if( envSize < buffsize )
    {
        SAFE_DELETE_ARRAY( envelope );
        envelope = new SAMPLE[buffsize];
        envSize = buffsize;
    }

    envExtr( read_frame );

    int numTransients = transExtr();

    if( numTransients )
    {
        for(int i = 0; i < numTransients; i++)
        {
            //cout<<i<<"/"<<transients[i] << "/"<< buffsize <<": "<<envelope[transients[i]]<<endl;
            BB_log( BB_LOG_FINE, "%i start %i end %i envLen %i envelope %f", i, transients[i].start, 
                transients[i].end, envLen, envelope[transients[i].start] );
        }
    }
    else
    {
        //cout << "no transients!\n" << endl;
        BB_log( BB_LOG_FINE, "yay, no transients. transients evil." );
    }

    estart = start;
    eend = end;

    return TRUE;
}


/*

  bool EnvExtractor::extract( t_TAPUINT start, t_TAPUINT end )
{
  int buffsize = end - start + 1; // + 1?
  
  //cout << "start: "<< start << endl;
  AudioSrcFile file;
  if( !file.open( filename.c_str(), start, buffsize, TRUE ) )
      return FALSE;
  envLen = buffsize;
  if( buffsize % 2 ) buffsize++;

  Frame * frame = new Frame;
  frame->alloc_waveform( buffsize );
  transients.clear();

  // get samples
  file.mtick( frame->waveform, frame->wlen );
  
  SAFE_DELETE_ARRAY( envelope );
  envelope = new SAMPLE[buffsize];
  envExtr( frame );
  
  int numTransients = transExtr();

  if( numTransients )
  {
    for(int i = 0; i < numTransients; i++)
    {
        //cout<<i<<"/"<<transients[i] << "/"<< buffsize <<": "<<envelope[transients[i]]<<endl;
        fprintf( stderr, "%i start %i end %i envLen %i envelope %f\n", i, transients[i].start, 
            transients[i].end, envLen, envelope[transients[i].start] );
    }
  }
  else
  {
      //cout << "no transients!\n" << endl;
      fprintf( stderr, "yay, no transients. transients evil.\n" );
  }

  estart = start;
  eend = end;

  SAFE_DELETE( frame ); 

  return TRUE;
}


*/


bool EnvExtractor::remove_transients( const char *outfilename, std::vector< t_TAPUINT > rmindexs )
{
    return TransientExtractor::remove_transients( outfilename, rmindexs );
}


int EnvExtractor::transExtr()
{
  //float *derivs = new float[envLen];
  SAFE_DELETE_ARRAY( derivs );
  derivs = new SAMPLE[envLen];

  int i;

  derivs[0] = 0;
  for(i = 1; i < envLen; i++)
    derivs[i] = fabs(envelope[i] - envelope[i - 1]);

  float newamt = 1.0 - ageamt;
  float avgEnergy = fabs( envelope[0] ); // fabs added
  int started = 0;
  int numTransients = 0;
  TransLoc tl;
  tl.start = 0; tl.end = 0;
  for(i = 0; i < envLen; i++){
    if(derivs[i] > avgEnergy * thresh){
      if(started <= 0){
        tl.start = i;
        tl.end = -1;
        //tl.end = (i + allowableGap) >= envLen ? envLen - 1 : i + allowableGap;
        //transients.push_back( tl );
        //numTransients++;
        started = allowableGap;
      }
      //started = allowableGap;
    }
    else
      started--;

    if( started == 0 && tl.end == -1 )
    {
        tl.end = i;
        transients.push_back( tl );
        numTransients++;
        //started = allowableGap;
    }

    avgEnergy *= ageamt;
    avgEnergy += newamt * fabs( envelope[i] ); // fabs added
  }

  //delete[] derivs;

  return numTransients;
}



void EnvExtractor::envExtr( Frame * framein)
{
  float gainup = 1.0 - attack;
  float gaindn = 1.0 - decay;

  float filtOut = 0;

  for(int i = 0; i < framein->wsize; i++){ // used to be wlen
    float datum = fabs(framein->waveform[i]);

    if(datum > filtOut)
      filtOut = (filtOut * attack) + (gainup * datum);
    else
      filtOut = (filtOut * decay) + (gaindn * datum);

    envelope[i] = filtOut;
  }
}



// ------------------------
// Another option 
// ------------------------

EngExtractor::EngExtractor( const char * fn )
{
    filename = fn ? fn : "";

    estart = 0;
    eend = 0;

    envelope = NULL;
    framestarts = NULL;
    envCap = 0; 

    longLen = BirdBrain::srate() / 2;
    shortLen = BirdBrain::srate() / 16;
    longHop = longLen / 2;
    shortHop = shortLen / 2;

    thresh = 4.5;
    passFreq = 55000; // 5.5 kHz
    envLen = 0;
    allowableGap = 2000;
    maxTranLen = BirdBrain::srate();
}


EngExtractor::~EngExtractor()
{
    SAFE_DELETE_ARRAY( envelope );
}


SAMPLE * EngExtractor::getEnv()
{
    return envelope;
}

bool EngExtractor::extract( t_TAPUINT start, t_TAPUINT end )
{
	int buffsize = end - start + 1; // + 1?
	estart = start;
	eend = end;

	AudioSrcBuffer * input;
	AudioSrcFile file;
	AudioSrcMic mic;

	if( filename.size() > 0 ) {
		if( !file.open( filename.c_str(), start, buffsize, TRUE ) )
			return FALSE;
		input = &file;
	}
	else {
		if( !mic.open( start, buffsize, TRUE ) )
			return FALSE;
		input = &mic;
	}

	Frame * frame = new Frame;
	frame->wsize = buffsize;
	if( buffsize % 2 ) buffsize++;
	frame->alloc_waveform( buffsize );

	transients.clear();

	// get samples (WHAT ABOUT STEREO FILES? converted to mono by mtick)
	input->mtick( frame->waveform, frame->wlen );
	if( filename.size() > 0 )
		file.close();

	if( envCap < buffsize )
	{
		SAFE_DELETE_ARRAY( envelope );
		envelope = new SAMPLE[buffsize];
		SAFE_DELETE_ARRAY( framestarts ); 
		framestarts = new int[buffsize]; 
		envCap = buffsize;
	}

	// get ratios
	Frame * longf = new Frame;
	longf->alloc_waveform( longLen % 2 ? longLen + 1 : longLen );
	Frame * shortf = new Frame;
	shortf->alloc_waveform( shortLen % 2 ? shortLen + 1 : shortLen );
	SAMPLE longEng, shortEng;
	envLen = 0;

	for( int a = 0; a < buffsize; a += longHop )
	{
		longf->wsize = a + longLen > buffsize ? buffsize - a : longLen; 
		memcpy( longf->waveform, frame->waveform + a, longf->wsize * sizeof( SAMPLE ) );

		longEng = energy( longf );
		for( int b = 0; b < longf->wsize; b+= shortHop )
		{
			shortf->wsize = b + shortLen > longf->wsize ? longf->wsize - b : shortLen;
			memcpy( shortf->waveform, frame->waveform + a + b, shortf->wsize * sizeof( SAMPLE ) );
			shortEng = energy( shortf );
    
			framestarts[envLen] = a + b;
			envelope[envLen] = shortEng / longEng; 
			envLen++;
		}
	}

	int numTransients = transExtr();

	if( numTransients )
	{
		for(int i = 0; i < numTransients; i++)
		{
			BB_log( BB_LOG_FINE, "%i start %i end %i buffsize %i", i, transients[i].start, 
				transients[i].end, buffsize );
		}
	}
	else
	{
		BB_log( BB_LOG_FINE, "yay, no transients. transients evil." );
	}

	SAFE_DELETE( frame ); 
	SAFE_DELETE( longf );
	SAFE_DELETE( shortf );

	return TRUE;
}


int EngExtractor::transExtr()
{
    //int sperl = longLen / shortHop; // short frames per long frame
    //if( longLen % shortHop ) sperl++;
    TransLoc tl;
    tl.start = -1;
    tl.end = -allowableGap;
    int sample, llength, buffsize = eend - estart + 1;

    for( int a = 0; a < envLen; a++ )
    {
        //sample = (a / sperl) * longHop + (a % sperl) * shortHop; 
        sample = framestarts[a]; 
        //llength = eend - sample + 1 > longLen ? longLen : eend - sample + 1;
        llength = buffsize - sample > longLen ? longLen : buffsize - sample; 
        
        if( envelope[a] > thresh )
        {
            // AAAAAAAAAAAAAAAAAAAAAAAAH!!!
            // What am I trying to do?
            if( tl.start == -1 || sample - tl.end >= allowableGap )
                tl.start = sample;
            // ends at end of this short frame, but don't want to go into next long frame
            //if( (a % sperl) * shortHop + shortLen > llength ) 
            if( sample + shortLen > llength ) // i.e. sample + shortLen - 1 >= llength
                tl.end = sample + llength - 1;
            else
                tl.end = sample + shortLen - 1; 
            if( tl.end - tl.start > maxTranLen ) // too long
                tl.start = -1;
        }
        else if( sample - tl.end > allowableGap && tl.start != -1 )
        {
            if( tl.start < tl.end )
                transients.push_back( tl );
            tl.start = -1;
        }
    }

    return transients.size();
}


bool EngExtractor::remove_transients( const char * outfilename, std::vector<t_TAPUINT>rmindexs )
{
    return TransientExtractor::remove_transients( outfilename, rmindexs );
}


// compute energy of given frame
SAMPLE EngExtractor::energy( Frame * framein )
{
    SAMPLE total = 0;
    
    if( framein->wsize <= 0 )
        return 0;

    for( int i = 0; i < framein->wsize; i++ )
        total += (framein->waveform[i] * framein->waveform[i]); 

    total /= framein->wsize;

    return total;
}

