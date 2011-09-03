// Copyright (c)Victor Lazzarini, 1997-2004
// See License.txt for a disclaimer of all warranties
// and licensing information

#ifdef WIN

#include "SndASIO.h" 
#include <stdio.h>

extern AsioDrivers* asioDrivers;

static float** outsndbuff;
static float** insndbuff;
static ASIOBufferInfo* bufferinfos;
static long buffsize;
static int currentbuffer = 0;
static int encoding; 
static long ochannels;
static long ichannels;
static bool optimise;

SndASIO::SndASIO(int channels, int mode, char* driver, 
				 SndObj** inputs, 
				 int vecsize, float sr) : 
         SndIO(channels,16,inputs,vecsize, sr){

  int i;
  m_mode = mode;
  m_running = false;
  m_driver = driver;
  m_ocurrentbuffer = m_icurrentbuffer = 0;
  m_icount = m_ocount = 0; 

  memset(&m_driverinfo, 0, sizeof(ASIODriverInfo));

  m_asiocallbacks.bufferSwitch = &bufferSwitch;
  m_asiocallbacks.sampleRateDidChange = &sampleRateChanged;
  m_asiocallbacks.asioMessage = &asioMessages;
  m_asiocallbacks.bufferSwitchTimeInfo = &bufferSwitchTimeInfo;

        // Allocate the memory for BufferInfos
   if(!(bufferinfos = new ASIOBufferInfo[(m_channels+2)*2])){
     m_error = 21;
     return;
   }


  if(!asioDrivers) asioDrivers = new AsioDrivers;

  if(asioDrivers->loadDriver(m_driver)){

   if(ASIOInit(&m_driverinfo) == ASE_OK){

   if(ASIOCanSampleRate(m_sr) == ASE_OK) 
	   ASIOSetSampleRate(m_sr);
   else ASIOGetSampleRate((double *)&m_sr);
   // set buffer size
   long dump1, dump2, dump3;
   ASIOGetBufferSize(&dump1, &dump2, &buffsize, &dump3);
   
   // get number of channels
   ASIOGetChannels(&ichannels, &ochannels);
   if(ichannels < m_channels){
	   m_channels = (short) ichannels;
	   m_samples = m_vecsize*m_channels;
   }
   else ichannels = m_channels;
   if(ochannels < m_channels){
	   m_channels = (short) ochannels;
	   m_samples = m_vecsize*m_channels;
   }
   else ochannels = m_channels;
   
   if(m_mode == SND_OUTPUT) ichannels = 0;
   if(m_mode == SND_INPUT) ochannels = 0;
       
  
  // Set the channel infos
   if(!(m_channelinfos = new ASIOChannelInfo[m_channels*2])){
     m_error = 22;
     return;
   }
      
   if((m_mode == SND_IO) || (m_mode == SND_OUTPUT)){           
	    outsndbuff = new float*[2]; 
   if((!(outsndbuff[0] = new float[buffsize*m_channels])) ||
	   (!(outsndbuff[1] = new float[buffsize*m_channels]))){
	   m_error =14;
	   return;
   }
   for(i = 0; i < m_channels; i++){	
   bufferinfos[i].isInput = ASIOFalse;
   bufferinfos[i].channelNum = i;
   bufferinfos[i].buffers[0] =
        bufferinfos[i].buffers[1] = 0;

   m_channelinfos[i].channel = bufferinfos[i].channelNum;
   m_channelinfos[i].isInput = bufferinfos[i].isInput;

   ASIOGetChannelInfo(&m_channelinfos[i]);

  
      switch(m_channelinfos[i].type){

	case ASIOSTInt16LSB:
	encoding = SHORTSAM;
	m_bits = 16;
    break;

	case ASIOSTInt24LSB:
	encoding = LONGSAM;
	m_bits = 24;
    break;

	case ASIOSTInt32LSB:
	encoding = LONGSAM;
	m_bits = 32;
    break;

    default:
	encoding = SHORTSAM;
    break;

	  }
   } 
   }
 	 
   if((m_mode == SND_IO) || (m_mode == SND_INPUT)){
   insndbuff = new float*[2];    
   
   if((!(insndbuff[0] = new float[buffsize*m_channels])) ||
	   (!(insndbuff[1] = new float[buffsize*m_channels]))
	   ){
	   m_error =14;
	   return;
   }

   for(i = 0; i < m_channels; i++){
   bufferinfos[i+ochannels].isInput = ASIOTrue;
   bufferinfos[i+ochannels].channelNum = i;
   bufferinfos[i+ochannels].buffers[0] =
        bufferinfos[i+ochannels].buffers[1] = 0;

   m_channelinfos[i+ochannels].channel = bufferinfos[i+ochannels].channelNum;
   m_channelinfos[i+ochannels].isInput = bufferinfos[i+ochannels].isInput;
 
   ASIOGetChannelInfo(&m_channelinfos[i+ochannels]);
     
      switch(m_channelinfos[i+ochannels].type){

	case ASIOSTInt16LSB:
	encoding = SHORTSAM;
	m_bits = 16;
    break;

	case ASIOSTInt24LSB:
	encoding = LONGSAM;
	m_bits = 24;
    break;

	case ASIOSTInt32LSB:
	encoding = LONGSAM;
	m_bits = 32;
    break;

    default:
	encoding = SHORTSAM;
    break;

	  }

   }
   }

 if(!(ASIOCreateBuffers(bufferinfos, ichannels+ochannels, 
	   buffsize, &m_asiocallbacks)== ASE_OK)){
	   m_error = 25;
	   return;
 }

    
    if(ASIOOutputReady() == ASE_OK) optimise = true;
				else optimise = false;

	m_outsndbuff = outsndbuff;
	m_insndbuff = insndbuff;
	m_encoding = encoding;
	m_bufferinfos = bufferinfos;
	m_ichannels = ichannels;
    m_ochannels  = ochannels;
	m_buffsize = buffsize;
	currentbuffer = 0;
	m_called_read = false;
  
   } 
   else { // could not initialise
   m_error = 24;
   return;
   }
  }   
  else { // if driver could not be loaded

	  m_error = 23;
      return;

  }

}


SndASIO::~SndASIO()
{

ASIOStop();
m_running = false;
ASIODisposeBuffers();
ASIOExit();
asioDrivers->removeCurrentDriver();
delete asioDrivers;
delete[] m_channelinfos;
delete[] m_outsndbuff;
delete[] m_insndbuff;
delete[] m_bufferinfos;
}

short
SndASIO::Read(){

if((!m_error) &&
   (m_mode != SND_OUTPUT)){ 

if(!m_running)
   if(ASIOStart() == ASE_OK) m_running = true;

int i;
m_called_read = true;

for(i = 0; i < m_samples; i++, m_icount++){
	if(m_icount > m_buffsize*m_channels){
	 m_icurrentbuffer = (m_icurrentbuffer+1)%2;
	 m_icount = 0;
	}
	// thread synchronisation
   while(m_icurrentbuffer == currentbuffer) Sleep(1);
     m_output[i] = m_insndbuff[m_icurrentbuffer][m_icount];
}
return 1;
}
else return 0;

}

short
SndASIO::Write(){

if((!m_error) &&
   (m_mode != SND_INPUT)){ 

if(!m_running)
   if(ASIOStart() == ASE_OK) m_running = true;

for(m_vecpos = 0; m_vecpos < m_vecsize; m_vecpos++){
	if(m_ocount > m_buffsize*m_channels){
	 m_ocurrentbuffer = (m_ocurrentbuffer+1)%2;
	 m_ocount = 0;
	}

	// this one makes sure that we did not called
	// Read() before Write() so that the thread sync
	// is done only once.
if(m_called_read == false)
       while(m_ocurrentbuffer == currentbuffer) Sleep(1);
	 
for(int n=0;n < m_channels; n++){	
	 m_outsndbuff[m_ocurrentbuffer][m_ocount] =
		 (m_IOobjs[n] ? m_IOobjs[n]->Output(m_vecpos):
	                      0.f);
     m_ocount++;		 
	 }
	 
}
m_called_read = false;
return 1;
} else return 0;


}



char*
SndASIO::ErrorMessage(){

char* message;

	switch(m_error){

	case 14:
    message = "Memory allocation error. \n";
	break;
    case 21:
	message = "Error allocating memory for bufferinfos\n";
	break;
	case 22:
	message = "Error allocating memory for channelinfos\n";
	break;
	case 23:
	message = "Could not load driver.\n";
	break;
	case 24:
	message = "Could not initialise driver.\n";
	break;
	case 25:
	message = "Could not initialise driver.\n";
	break;
     
	default:
	message = SndIO::ErrorMessage();
	break;

	}
return message;

}
void
bufferSwitch(long index, ASIOBool processNow){
ASIOTime time;
bufferSwitchTimeInfo(&time, index, processNow);
}

ASIOTime*
bufferSwitchTimeInfo(ASIOTime *timeInfo,
							  long index, ASIOBool processNow){

 short* sigshort;
 long* siglong;
 int n,j;

 // we do the channel interleaving here

 for(int i = 0; i < ichannels+ochannels; i++){
  if(bufferinfos[i].isInput == ASIOFalse){


	switch(encoding){

	case SHORTSAM:
    sigshort = (short *)bufferinfos[i].buffers[index];
    for(n = bufferinfos[i].channelNum, j = 0; 
	      n < buffsize*ochannels; n+=ochannels, j++){
	sigshort[j] = (short) outsndbuff[currentbuffer][n];
						 }
	break;
 
    case LONGSAM:
    siglong = (long *)bufferinfos[i].buffers[index];
    for(n = bufferinfos[i].channelNum, j = 0; 
	      n < buffsize*ochannels; n+=ochannels, j++)
		siglong[j] = (long) outsndbuff[currentbuffer][n];
	break;

	}

	 }
	 else{

 
     switch(encoding){

	case SHORTSAM:
    sigshort = (short *)bufferinfos[i].buffers[index];
	for(n = bufferinfos[i].channelNum, j = 0; 
	       n < buffsize*ichannels; n+=ichannels, j++)
		insndbuff[currentbuffer][n] = (float) sigshort[j];
		  
	break;

    case LONGSAM:
    siglong = (long *)bufferinfos[i].buffers[index];
    for(n = bufferinfos[i].channelNum, j = 0; 
	       n < buffsize*ichannels; n+=ichannels, j++)
		insndbuff[currentbuffer][n] = (float) siglong[j];
	break;
   }  
	}

	}
 
 if(optimise)ASIOOutputReady();
	
 currentbuffer= (currentbuffer+1)%2;
 return timeInfo;

}

void
sampleRateChanged(ASIOSampleRate sRate){
}

long
asioMessages(long selector, long value, void *message,
					  double *opt){

long ret = 0;
	switch(selector)
	{
		case kAsioSelectorSupported:
			if(value == kAsioResetRequest
			|| value == kAsioEngineVersion
			|| value == kAsioResyncRequest
			|| value == kAsioLatenciesChanged
			|| value == kAsioSupportsTimeInfo
			|| value == kAsioSupportsTimeCode
			|| value == kAsioSupportsInputMonitor)
				ret = 1L;
			break;
		case kAsioResetRequest:
			ret = 1L;
			break;
		case kAsioResyncRequest:
			ret = 1L;
			break;
		case kAsioLatenciesChanged:
   		     ret = 1L;
			break;
		case kAsioEngineVersion:
			ret = 2L;
			break;
		case kAsioSupportsTimeInfo:
			ret = 1;
			break;
		case kAsioSupportsTimeCode:
			ret = 0;
			break;
	}
	return ret;
}

void DriverList(){
    
	// display driver names 
    char** drivernames = new char*[10];
	int numdrivers;
    for(int i = 0; i < 10; i++)
              drivernames[i] = new char[32];
    if(!asioDrivers)
    asioDrivers = new AsioDrivers;

    numdrivers =  
		asioDrivers->getDriverNames(drivernames, 10); 
    
	cout << "ASIO drivers in this system:\n";
    for(i = 0; i < numdrivers; i++)  
       cout << i << ":" << drivernames[i] << "\n"; 

};


char* DriverName(int num, char* name){

    char** drivernames = new char*[10];
    for(int i = 0; i < 10; i++)
              drivernames[i] = new char[32];

	if(!asioDrivers)
    asioDrivers = new AsioDrivers;
    asioDrivers->getDriverNames(drivernames, 10); 
    if(num && (num < 10)) 
	strcpy(name, drivernames[num]);
	return name;
}

#endif