// Copyright (c)Victor Lazzarini, 1997-2004
// See License.txt for a disclaimer of all warranties
// and licensing information

////////////////////////////
//  SndMidi.cpp
//
// 
//
#if defined (SGI) || defined (OSS) || defined (WIN)
#include "SndMidi.h"

SndMidi::SndMidi(int buffsize, float sr): SndIO (1, 0, 0, 1, sr)
{

int i;
m_buffsize = buffsize;
m_channels = 16;

delete[] m_output;

m_vel = new unsigned char[128];
m_aft = new unsigned char[128];

for(i = 0; i < 127; i++)
	m_vel[i] = m_aft[i] = 0;

#if defined(OSS) || defined(SGI)
if(!(m_event = new MDevent[m_buffsize])){
m_error =14;
cout << ErrorMessage();
return;
}
#endif

#ifdef WIN
if(!(m_event = new MIDI_event[m_buffsize])){
m_error =14;
cout << ErrorMessage();
return;
}
#endif


if(!(m_output = new float[m_channels])){
m_error = 15;
cout << ErrorMessage();
return;
}


if(!(m_message = new short[m_channels])){
m_error = 15;
cout << ErrorMessage();
return;
}
m_note = 0;
m_noteon = m_noteoff = 0;

m_read = 0;
m_count = 0;

for(i = 0; i < m_channels; i++) {
	m_output[i] = 0.f;
	m_message[i] = 0;
}
}

SndMidi::~SndMidi(){
	delete[] m_message;
	delete[] m_event;
    delete[] m_vel;
}

short
SndMidi::NoteOn(){

if(m_noteon) {
m_noteon = 0;
return m_note;
}
else return -1;
}

short 
SndMidi::NoteOff(){

if(m_noteoff) {
m_noteoff = 0;
return m_note;
}
else return -1;
}
char*
SndMidi::ErrorMessage(){

char* message;
switch(m_error){

  case 14:
  message = "Error allocating buffer memory.";
  break;
  
  case 15:
  message = "Error allocating memory.";
  break;

  default:
  message = SndIO::ErrorMessage();
  break;

}

return message;
}

#ifdef WIN

void
MidiDeviceList(){
MIDIINCAPS  incaps;
unsigned int j;
cout << "MIDI Input devices:\ndevice ID: device name\n";
   for(j = 0; j < midiInGetNumDevs(); j++){
   midiInGetDevCaps(j, &incaps, sizeof(incaps)); 
   cout << j << "        : "<< incaps.szPname << "\n";
   }

}

char* MidiInputDeviceName(int dev, char* name){

MIDIINCAPS   incaps;

if(dev >= 0 && dev < (int) midiInGetNumDevs()){
	midiInGetDevCaps(dev, &incaps, sizeof(incaps));
	return strcpy(name, incaps.szPname);
} else return 0;

}
#endif

#endif
