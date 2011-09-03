// Copyright (c)Victor Lazzarini, 1997-2004
// See License.txt for a disclaimer of all warranties
// and licensing information

///////////////////////////////////////////////
// SyncGrain.h: synchronous granular synthesis
//              based on wavetable lookup   
//  VL, 2002
///////////////////////////////////////////////

#ifndef _SyncGrain_H
#define _SyncGrain_H
#include "SndObj.h"
#include "Table.h"


class SyncGrain: public SndObj {


   protected:

   Table* m_table;    // wavetable
   Table* m_envtable; // envelope table

   float m_amp;       // overall amp
   SndObj* m_inputamp;
   float m_fr;        // fundamental freq
   float m_frac;      // fractional part
   SndObj* m_inputfr;
   float m_pitch;     // grain pitch
   SndObj* m_inputpitch;
   float* m_index;    // index into wavetable
   float* m_envindex; // index into envtable

   float m_start;  // grain start index 
   float m_grsize; // size of grains (secs)
   SndObj* m_inputgrsize;
   int m_olaps;   // max number of streams (overlaps)
   float m_point; // grain start offset
   
   int m_count;    // sampling period counter
   int m_numstreams;  // curr num of streams
   int m_firststream; // streams index (first stream) 
   int m_tablesize;   // size of wavetable
   int m_envtablesize; // size of envtable

   short* m_streamon;  // keeps track of the active streams

   public:

   // constructors / destructor
   SyncGrain();
   SyncGrain(Table* wavetable, Table* envtable, float fr, float amp,
	          float pitch, float grsize, float prate=1.f, SndObj* inputfr=0, 
			  SndObj* inputamp=0, SndObj* inputpitch=0, 
			  SndObj* inputgrsize=0, int olaps=100,
			  int vecisize=DEF_VECSIZE, float sr=DEF_SR);

   ~SyncGrain();

   // Set...
   void
   SetWaveTable(Table* wavetable){
      m_table = wavetable;
	  m_tablesize = wavetable->GetLen();
   }
  
   void
   SetEnvelopeTable(Table* envtable){
      m_envtable = envtable;
	  m_envtablesize = envtable->GetLen();
   }   

   void
   SetFreq(float fr, SndObj* inputfr=0){
   m_fr = fr;     // fundamental freq
   m_inputfr = inputfr;

   }

   void
   SetAmp(float amp, SndObj* inputamp=0){
   m_amp = amp;    // overall amp
   m_inputamp = inputamp;

   }

   void 
   SetPitch(float pitch, SndObj* inputpitch=0){
   m_pitch = pitch;  // grain pitch
   m_inputpitch = inputpitch;

   }

   void
   SetGrainSize(float grsize, SndObj* inputgrsize=0){
   m_grsize = grsize; // size of grains (secs)
   m_inputgrsize = inputgrsize;
   }

   void   
   SetPointerRate(float prate){ m_point = prate; }
    // sets the reading
    // pointer rate in relation to grain size:
    // a value of 1 will make the read pointer read
    // the wavetable skiping grainsize positions 
    // along it. A value of 0 will make the table be
    // read always at the start.

   int Set(char* mess, float value);
   int Connect(char* mess, void* input);
 
   short DoProcess();
   char* ErrorMessage();

};
	
#endif

