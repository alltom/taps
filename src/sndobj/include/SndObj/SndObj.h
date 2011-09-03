// Copyright (c)Victor Lazzarini, 1997-2004
// See License.txt for a disclaimer of all warranties
// and licensing information

//************************************************************//
//  SndObj.h: Interface of the SndObj base class              //
//                                                            //
//                                                            //
//                                                            //
//************************************************************//

#ifndef _SNDOBJ_H 
#define _SNDOBJ_H
#include <stdlib.h>
#include <math.h>
#include <iostream>
#include <string>

using namespace std;

class  SndIO;
const double PI = 4.*atan(1.);
const int DEF_FFTSIZE = 1024;
const int DEF_VECSIZE = 256;
const float DEF_SR = 44100.f;

struct msg_link {
   string msg;
   int  ID;
   msg_link *previous;
};

class SndObj {

                    protected:

 float* m_output; // output samples
 SndObj* m_input; // input object
 float  m_sr;    // sampling rate
 int    m_vecsize; //vector size
 int    m_vecpos; // vector pos counter
 int    m_altvecpos; // secondary counter
 int    m_error;     // error code
 short  m_enable;  // enable object
 
 msg_link *m_msgtable;

 inline int FindMsg(char* mess);
 void AddMsg(const char* mess, int ID);

                   public:

bool IsProcessing() {
if(m_vecpos && m_vecpos != m_vecsize) return true;
else return false;
}

int GetError() { return m_error; }

SndObj operator=(SndObj obj){					   
    if(&obj == this) return *this;
	for(int n = 0; n < m_vecsize; n++) m_output[n] = obj.Output(n);
	return *this;
 }
	 
 SndObj& operator+=(SndObj& obj){
	for(int n = 0; n < m_vecsize; n++) m_output[n] = m_output[n]+obj.Output(n);
	return *this;
 }
 
 SndObj& operator-=(SndObj& obj){
	for(int n = 0; n < m_vecsize; n++) m_output[n] = m_output[n]-obj.Output(n);
	return *this;
 }
  
 SndObj& operator*=(SndObj& obj){
	for(int n = 0; n < m_vecsize; n++) m_output[n] = m_output[n]*obj.Output(n);
	return *this;
 }

  SndObj& operator+=(float val){
	for(int n = 0; n < m_vecsize; n++) m_output[n] = m_output[n]+val;
	return *this;
 }
 
 SndObj& operator-=(float val){
	for(int n = 0; n < m_vecsize; n++) m_output[n] = m_output[n]-val;
	return *this;
 }
  
 SndObj& operator*=(float val){
	for(int n = 0; n < m_vecsize; n++) m_output[n] = m_output[n]*val;
	return *this;
 }

 SndObj operator+(SndObj& obj){
	SndObj temp(0, m_vecsize, m_sr);
	for(int n = 0; n < m_vecsize; n++) temp.m_output[n] = m_output[n]+obj.Output(n);
	return temp;
 }
 
 SndObj operator-(SndObj& obj){
	SndObj temp(0, m_vecsize, m_sr);
	for(int n = 0; n < m_vecsize; n++) temp.m_output[n] = m_output[n]-obj.Output(n);
	return temp;
 }

 SndObj operator*(SndObj& obj){
	SndObj temp(0, m_vecsize, m_sr);
	for(int n = 0; n < m_vecsize; n++) temp.m_output[n] = m_output[n]*obj.Output(n);
	return temp;
 }

  SndObj operator+(float val){
	SndObj temp(0, m_vecsize, m_sr);
	for(int n = 0; n < m_vecsize; n++) temp.m_output[n] = m_output[n]+val;
	return temp;
 }
 
 SndObj operator-(float val){
	SndObj temp(0, m_vecsize, m_sr);
	for(int n = 0; n < m_vecsize; n++) temp.m_output[n] = m_output[n]-val;
	return temp;
 }

 SndObj operator*(float val){
	SndObj temp(0, m_vecsize, m_sr);
	for(int n = 0; n < m_vecsize; n++) temp.m_output[n] = m_output[n]*val;
	return temp;
 } 

 void operator<<(float val){        
	 if(m_vecpos >= m_vecsize) m_vecpos=0;
	 m_output[m_vecpos++] = val;
 }

 void operator<<(float* vector){
   for(m_vecpos=0;m_vecpos<m_vecsize;m_vecpos++)
   m_output[m_vecpos] = vector[m_vecpos];
 }

 void operator>>(SndIO& out);
 void operator<<(SndIO& in);

 int PushIn(float *vector, int size){
	 for(int i = 0; i<size; i++){
      if(m_vecpos >= m_vecsize) m_vecpos = 0;
	  m_output[m_vecpos++] = vector[i];
	 }
	 return m_vecpos;
 }

 int PopOut(float *vector, int size){
  	 for(int i = 0; i<size; i++){
      if(m_altvecpos >= m_vecsize) m_altvecpos = 0;
	  vector[i] = m_output[m_altvecpos++]; 
	 }
	 return m_altvecpos;
 }


 int AddOut(float *vector, int size){
  	 for(int i = 0; i<size; i++){
      if(m_altvecpos >= m_vecsize) m_altvecpos = 0;
	  vector[i] += m_output[m_altvecpos++]; 
	 }
	 return m_altvecpos;
 }

 
 void   GetMsgList(string* list);
 void   Enable(){ m_enable = 1;  }
 void   Disable(){ m_enable = 0; }
 float  Output(int pos){ return m_output[pos%m_vecsize];}

 int GetVectorSize() { return m_vecsize; } 
 void SetVectorSize(int vecsize);
 float  GetSr(){ return m_sr;}
 virtual void SetSr(float sr){ m_sr = sr;}
 virtual int Set(char* mess, float value);
 virtual int Connect(char* mess, void* input);

 
 void SetInput(SndObj* input){
 m_input = input;
 }
 
 SndObj* GetInput(){ return m_input; }

 SndObj(SndObj* input, int vecsize = DEF_VECSIZE, float sr = DEF_SR);
 SndObj();
 SndObj(SndObj& obj);


 virtual ~SndObj();
 virtual char* ErrorMessage();
 virtual short DoProcess();
 
            };

int
SndObj::FindMsg(char* mess){

	msg_link* iter = m_msgtable;
    while(iter->previous && iter->msg.compare(mess))
               iter = iter->previous;
    if(!iter->msg.compare(mess)) return iter->ID;
    else return 0;
}

#endif
