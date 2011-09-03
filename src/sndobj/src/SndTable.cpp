// Copyright (c)Victor Lazzarini, 1997-2004
// See License.txt for a disclaimer of all warranties
// and licensing information

//************************************************************//
//  SndTable.cpp:  implementation of the SndTable object      //
//               (soundfile function table)                   //
//                                                            //
//                                                            //
//************************************************************//
#include "SndTable.h"
//////////construction / destruction ///////////////////////
SndTable :: SndTable(){

m_L = 1;
m_channel = 0;
m_input = 0;
m_table = new float[m_L];
MakeTable();

                        }

SndTable :: SndTable(long L, SndFIO* input, short channel){

m_L = L;
m_channel = channel;
m_input = input;
m_table = new float [m_L];
MakeTable();

        }


SndTable :: ~SndTable(){

delete[] m_table;

                         }


///////////// OPERATIONS ////////////////////////////////////
void 
SndTable :: SetInput(long L, SndFIO* input, short channel){
           m_input = input;
           m_channel = channel;
           m_L = L;
           delete[] m_table;
           m_table = new float[m_L];
  }

short
SndTable :: MakeTable(){
  if(!m_input){
     m_error = 1;
     return 0;
  }
  else {
	   int n, i;
	   float max = 0.f;
	   short chan = m_input->GetChannels();
	   long size = m_input->GetVectorSize();
       for(n = 0; n < m_L; n+=size){
       m_input->Read();
	   for(i = 0; (i < size) && (n+i < m_L); i++){
               m_table[n+i] = m_input->Output(i, m_channel);
	  max = (fabs((double)max) < fabs((double)m_table[n+i])) ? m_table[n+i] : max;
	   }
                                   }
       m_input->SetPos(0.f);
	   if(max) for (n = 0; n < m_L; n++) m_table[n] /= max;
       return 1;            
       }
}
///////////////// ERROR HANDLING ///////////////////////////////

char*
SndTable::ErrorMessage(){
  
  char* message;
   
  switch(m_error){

  case 0:
  message = "No error.";
  break; 

  case 1:
  message = "MakeTable() failed. No input obj..";
  break;

  default:
  message = "Undefined error";
  break;
  }

 return message;

}
