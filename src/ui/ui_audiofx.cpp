//-----------------------------------------------------------------------------
// name: ui_audiofx.cpp
// desc: birdbrain ui audio effects
//
// authors: Ananya Misra (amisra@cs.princeton.edu)
//          Ge Wang (gewang@cs.princeton.edu)
//          Perry R. Cook (prc@cs.princeton.edu)
// date: Spring 2006
//-----------------------------------------------------------------------------
#include <math.h>
#include "birdbrain.h"
#include "ui_audiofx.h"
#include <iostream>
using namespace std;

// Start FxStk

const FxStk::STK_FORMAT FxStk :: STK_SINT8 = 1;
const FxStk::STK_FORMAT FxStk :: STK_SINT16 = 2;
const FxStk::STK_FORMAT FxStk :: STK_SINT32 = 8;
const FxStk::STK_FORMAT FxStk :: MY_FLOAT32 = 16;
const FxStk::STK_FORMAT FxStk :: MY_FLOAT64 = 32;

FxStk :: FxStk(void)
{
}

FxStk :: ~FxStk(void)
{
}

void FxStk :: swap16(unsigned char *ptr)
{
  register unsigned char val;

  // Swap 1st and 2nd bytes
  val = *(ptr);
  *(ptr) = *(ptr+1);
  *(ptr+1) = val;
}

void FxStk :: swap32(unsigned char *ptr)
{
  register unsigned char val;

  // Swap 1st and 4th bytes
  val = *(ptr);
  *(ptr) = *(ptr+3);
  *(ptr+3) = val;

  //Swap 2nd and 3rd bytes
  ptr += 1;
  val = *(ptr);
  *(ptr) = *(ptr+1);
  *(ptr+1) = val;
}

void FxStk :: swap64(unsigned char *ptr)
{
  register unsigned char val;

  // Swap 1st and 8th bytes
  val = *(ptr);
  *(ptr) = *(ptr+7);
  *(ptr+7) = val;

  // Swap 2nd and 7th bytes
  ptr += 1;
  val = *(ptr);
  *(ptr) = *(ptr+5);
  *(ptr+5) = val;

  // Swap 3rd and 6th bytes
  ptr += 1;
  val = *(ptr);
  *(ptr) = *(ptr+3);
  *(ptr+3) = val;

  // Swap 4th and 5th bytes
  ptr += 1;
  val = *(ptr);
  *(ptr) = *(ptr+1);
  *(ptr+1) = val;
}

#if (defined(__OS_IRIX__) || defined(__OS_LINUX__) || defined(__OS_MACOSX__))
  #include <unistd.h>
#elif defined(__OS_WINDOWS__)
  #include <windows.h>
#endif

void FxStk :: sleep(unsigned long milliseconds)
{
#if defined(__OS_WINDOWS__)
  Sleep((DWORD) milliseconds);
#elif (defined(__OS_IRIX__) || defined(__OS_LINUX__) || defined(__OS_MACOSX__))
  usleep( (unsigned long) (milliseconds * 1000.0) );
#endif
}

void FxStk :: handleError( const char *message, FxStkError::TYPE type )
{
  if (type == FxStkError::WARNING)
    fprintf(stderr, "\n%s\n\n", message);
  else if (type == FxStkError::DEBUG_WARNING) {
#if defined(_STK_DEBUG_)
    fprintf(stderr, "\n%s\n\n", message);
#endif
  }
  else {
    // Print error message before throwing.
    fprintf(stderr, "\n%s\n\n", message);
    throw FxStkError(message, type);
  }
}

FxStkError :: FxStkError(const char *p, TYPE tipe)
  : type(tipe)
{
  strncpy(message, p, 256);
}

FxStkError :: ~FxStkError(void)
{
}

void FxStkError :: printMessage(void)
{
  printf("\n%s\n\n", message);
}

// end FxStk

// start FxFilter

FxFilter :: FxFilter()
{
  // The default constructor should setup for pass-through.
  gain = 1.0;
  nB = 1;
  nA = 1;
  b = new MY_FLOAT[nB];
  b[0] = 1.0;
  a = new MY_FLOAT[nA];
  a[0] = 1.0;

  inputs = new MY_FLOAT[nB];
  outputs = new MY_FLOAT[nA];
  this->clear();
}

FxFilter :: FxFilter(int nb, MY_FLOAT *bCoefficients, int na, MY_FLOAT *aCoefficients)
{
  char message[256];

  // Check the arguments.
  if ( nb < 1 || na < 1 ) {
    sprintf(message, "FxFilter: nb (%d) and na (%d) must be >= 1!", nb, na);
    handleError( message, FxStkError::FUNCTION_ARGUMENT );
  }

  if ( aCoefficients[0] == 0.0 ) {
    sprintf(message, "FxFilter: a[0] coefficient cannot == 0!");
    handleError( message, FxStkError::FUNCTION_ARGUMENT );
  }

  gain = 1.0;
  nB = nb;
  nA = na;
  b = new MY_FLOAT[nB];
  a = new MY_FLOAT[nA];

  inputs = new MY_FLOAT[nB];
  outputs = new MY_FLOAT[nA];
  this->clear();

  this->setCoefficients(nB, bCoefficients, nA, aCoefficients);
}

FxFilter :: ~FxFilter()
{
  delete [] b;
  delete [] a;
  delete [] inputs;
  delete [] outputs;
}

void FxFilter :: clear(void)
{
  int i;
  for (i=0; i<nB; i++)
    inputs[i] = 0.0;
  for (i=0; i<nA; i++)
    outputs[i] = 0.0;
}

void FxFilter :: setCoefficients(int nb, MY_FLOAT *bCoefficients, int na, MY_FLOAT *aCoefficients)
{
  int i;
  char message[256];

  // Check the arguments.
  if ( nb < 1 || na < 1 ) {
    sprintf(message, "FxFilter: nb (%d) and na (%d) must be >= 1!", nb, na);
    handleError( message, FxStkError::FUNCTION_ARGUMENT );
  }

  if ( aCoefficients[0] == 0.0 ) {
    sprintf(message, "FxFilter: a[0] coefficient cannot == 0!");
    handleError( message, FxStkError::FUNCTION_ARGUMENT );
  }

  if (nb != nB) {
    delete [] b;
    delete [] inputs;
    nB = nb;
    b = new MY_FLOAT[nB];
    inputs = new MY_FLOAT[nB];
    for (i=0; i<nB; i++) inputs[i] = 0.0;
  }

  if (na != nA) {
    delete [] a;
    delete [] outputs;
    nA = na;
    a = new MY_FLOAT[nA];
    outputs = new MY_FLOAT[nA];
    for (i=0; i<nA; i++) outputs[i] = 0.0;
  }

  for (i=0; i<nB; i++)
    b[i] = bCoefficients[i];
  for (i=0; i<nA; i++)
    a[i] = aCoefficients[i];

  // scale coefficients by a[0] if necessary
  if (a[0] != 1.0) {
    for (i=0; i<nB; i++)
      b[i] /= a[0];
    for (i=0; i<nA; i++)
      a[i] /= a[0];
  }
}

void FxFilter :: setNumerator(int nb, MY_FLOAT *bCoefficients)
{
  int i;
  char message[256];

  // Check the arguments.
  if ( nb < 1 ) {
    sprintf(message, "FxFilter: nb (%d) must be >= 1!", nb);
    handleError( message, FxStkError::FUNCTION_ARGUMENT );
  }

  if (nb != nB) {
    delete [] b;
    delete [] inputs;
    nB = nb;
    b = new MY_FLOAT[nB];
    inputs = new MY_FLOAT[nB];
    for (i=0; i<nB; i++) inputs[i] = 0.0;
  }

  for (i=0; i<nB; i++)
    b[i] = bCoefficients[i];
}

void FxFilter :: setDenominator(int na, MY_FLOAT *aCoefficients)
{
  int i;
  char message[256];

  // Check the arguments.
  if ( na < 1 ) {
    sprintf(message, "FxFilter: na (%d) must be >= 1!", na);
    handleError( message, FxStkError::FUNCTION_ARGUMENT );
  }

  if ( aCoefficients[0] == 0.0 ) {
    sprintf(message, "FxFilter: a[0] coefficient cannot == 0!");
    handleError( message, FxStkError::FUNCTION_ARGUMENT );
  }

  if (na != nA) {
    delete [] a;
    delete [] outputs;
    nA = na;
    a = new MY_FLOAT[nA];
    outputs = new MY_FLOAT[nA];
    for (i=0; i<nA; i++) outputs[i] = 0.0;
  }

  for (i=0; i<nA; i++)
    a[i] = aCoefficients[i];

  // scale coefficients by a[0] if necessary
  if (a[0] != 1.0) {
    for (i=0; i<nB; i++)
      b[i] /= a[0];
    for (i=0; i<nA; i++)
      a[i] /= a[0];
  }
}

void FxFilter :: setGain(MY_FLOAT theGain)
{
  gain = theGain;
}

MY_FLOAT FxFilter :: getGain(void) const
{
  return gain;
}

MY_FLOAT FxFilter :: lastOut(void) const
{
  return outputs[0];
}

MY_FLOAT FxFilter :: tick(MY_FLOAT sample)
{
  int i;

  outputs[0] = 0.0;
  inputs[0] = gain * sample;
  for (i=nB-1; i>0; i--) {
    outputs[0] += b[i] * inputs[i];
    inputs[i] = inputs[i-1];
  }
  outputs[0] += b[0] * inputs[0];

  for (i=nA-1; i>0; i--) {
    outputs[0] += -a[i] * outputs[i];
    outputs[i] = outputs[i-1];
  }

  return outputs[0];
}

MY_FLOAT *FxFilter :: tick(MY_FLOAT *vec, unsigned int vectorSize)
{
  for (unsigned int i=0; i<vectorSize; i++)
    vec[i] = tick(vec[i]);

  return vec;
}

// end FxFilter

// start FxDelay

FxDelay :: FxDelay()
{
  // Default max delay length set to 4095.
  length = 4096;
  delete [] inputs;
  inputs = new MY_FLOAT[length];
  this->clear();

  inPoint = 0;
  outPoint = 0;
  delay = 0;
}

FxDelay :: FxDelay(long theDelay, long maxDelay)
{
  // Writing before reading allows delays from 0 to length-1. 
  // If we want to allow a delay of maxDelay, we need a
  // delay-line of length = maxDelay+1.
  length = maxDelay+1;

  // We need to delete the previously allocated inputs.
  delete [] inputs;
  inputs = new MY_FLOAT[length];
  this->clear();

  inPoint = 0;
  this->setDelay(theDelay);
}

FxDelay :: ~FxDelay()
{
}

void FxDelay :: clear(void)
{
  long i;
  for (i=0;i<length;i++) inputs[i] = 0.0;
  outputs[0] = 0.0;
}

void FxDelay :: setDelay(long theDelay)
{
  if (theDelay > length-1) { // The value is too big.
    std::cerr << "FxDelay: setDelay(" << theDelay << ") too big!" << std::endl;
    // Force delay to maxLength.
    outPoint = inPoint + 1;
    delay = length - 1;
  }
  else if (theDelay < 0 ) {
    std::cerr << "FxDelay: setDelay(" << theDelay << ") less than zero!" << std::endl;
    outPoint = inPoint;
    delay = 0;
  }
  else {
    outPoint = inPoint - (long) theDelay;  // read chases write
    delay = theDelay;
  }

  while (outPoint < 0) outPoint += length;  // modulo maximum length
}

long FxDelay :: getDelay(void) const
{
  return (long)delay;
}

MY_FLOAT FxDelay :: energy(void) const
{
  int i;
  register MY_FLOAT e = 0;
  if (inPoint >= outPoint) {
    for (i=outPoint; i<inPoint; i++) {
      register MY_FLOAT t = inputs[i];
      e += t*t;
    }
  } else {
    for (i=outPoint; i<length; i++) {
      register MY_FLOAT t = inputs[i];
      e += t*t;
    }
    for (i=0; i<inPoint; i++) {
      register MY_FLOAT t = inputs[i];
      e += t*t;
    }
  }
  return e;
}

MY_FLOAT FxDelay :: contentsAt(unsigned long tapDelay) const
{
  long i = tapDelay;
  if (i < 1) {
    std::cerr << "FxDelay: contentsAt(" << tapDelay << ") too small!" << std::endl;
    i = 1;
  }
  else if (i > delay) {
    std::cerr << "FxDelay: contentsAt(" << tapDelay << ") too big!" << std::endl;
    i = (long) delay;
  }

  long tap = inPoint - i;
  if (tap < 0) // Check for wraparound.
    tap += length;

  return inputs[tap];
}

MY_FLOAT FxDelay :: lastOut(void) const
{
  return FxFilter::lastOut();
}

MY_FLOAT FxDelay :: nextOut(void) const
{
  return inputs[outPoint];
}

MY_FLOAT FxDelay :: tick(MY_FLOAT sample)
{
  inputs[inPoint++] = sample;

  // Check for end condition
  if (inPoint == length)
    inPoint -= length;

  // Read out next value
  outputs[0] = inputs[outPoint++];

  if (outPoint>=length)
    outPoint -= length;

  return outputs[0];
}

MY_FLOAT *FxDelay :: tick(MY_FLOAT *vec, unsigned int vectorSize)
{
  for (unsigned int i=0; i<vectorSize; i++)
    vec[i] = tick(vec[i]);

  return vec;
}

// end FxDelay

// start FxReverb

FxReverb :: FxReverb()
{
}

FxReverb :: ~FxReverb()
{
}

void FxReverb :: setEffectMix(MY_FLOAT mix)
{
  effectMix = mix;
}

MY_FLOAT FxReverb :: lastOut() const
{
  return (lastOutput[0] + lastOutput[1]) * 0.5;
}

MY_FLOAT FxReverb :: lastOutLeft() const
{
  return lastOutput[0];
}

MY_FLOAT FxReverb :: lastOutRight() const
{
  return lastOutput[1];
}

MY_FLOAT *FxReverb :: tick(MY_FLOAT *vec, unsigned int vectorSize)
{
  for (unsigned int i=0; i<vectorSize; i++)
    vec[i] = tick(vec[i]);

  return vec;
}

bool FxReverb :: isPrime(int number)
{
  if (number == 2) return true;
  if (number & 1)   {
      for (int i=3; i<(int)sqrt((double)number)+1; i+=2)
          if ( (number % i) == 0) return false;
      return true; /* prime */
    }
  else return false; /* even */
}

// start FxReverb

// start FxJCRev

FxJCRev :: FxJCRev(MY_FLOAT T60)
{
  // FxDelay lengths for 44100 Hz sample rate.
  int lengths[9] = {1777, 1847, 1993, 2137, 389, 127, 43, 211, 179};
  double scaler = BirdBrain::srate() / 44100.0;

  int delay, i;
  if ( scaler != 1.0 ) {
    for (i=0; i<9; i++) {
      delay = (int) floor(scaler * lengths[i]);
      if ( (delay & 1) == 0) delay++;
      while ( !this->isPrime(delay) ) delay += 2;
      lengths[i] = delay;
    }
  }

  for (i=0; i<3; i++)
      allpassDelays[i] = new FxDelay(lengths[i+4], lengths[i+4]);

  for (i=0; i<4; i++)   {
    combDelays[i] = new FxDelay(lengths[i], lengths[i]);
    combCoefficient[i] = pow(10.0,(double)(-3 * lengths[i] / (T60 * BirdBrain::srate())));
  }

  outLeftDelay = new FxDelay(lengths[7], lengths[7]);
  outRightDelay = new FxDelay(lengths[8], lengths[8]);
  allpassCoefficient = (MY_FLOAT)0.7;
  effectMix = (MY_FLOAT)0.3;
  this->clear();
}

FxJCRev :: ~FxJCRev()
{
  delete allpassDelays[0];
  delete allpassDelays[1];
  delete allpassDelays[2];
  delete combDelays[0];
  delete combDelays[1];
  delete combDelays[2];
  delete combDelays[3];
  delete outLeftDelay;
  delete outRightDelay;
}

void FxJCRev :: clear()
{
  allpassDelays[0]->clear();
  allpassDelays[1]->clear();
  allpassDelays[2]->clear();
  combDelays[0]->clear();
  combDelays[1]->clear();
  combDelays[2]->clear();
  combDelays[3]->clear();
  outRightDelay->clear();
  outLeftDelay->clear();
  lastOutput[0] = 0.0;
  lastOutput[1] = 0.0;
}

MY_FLOAT FxJCRev :: tick(MY_FLOAT input)
{
  MY_FLOAT temp, temp0, temp1, temp2, temp3, temp4, temp5, temp6;
  MY_FLOAT filtout;

  temp = allpassDelays[0]->lastOut();
  temp0 = allpassCoefficient * temp;
  temp0 += input;
  allpassDelays[0]->tick(temp0);
  temp0 = -(allpassCoefficient * temp0) + temp;
    
  temp = allpassDelays[1]->lastOut();
  temp1 = allpassCoefficient * temp;
  temp1 += temp0;
  allpassDelays[1]->tick(temp1);
  temp1 = -(allpassCoefficient * temp1) + temp;
    
  temp = allpassDelays[2]->lastOut();
  temp2 = allpassCoefficient * temp;
  temp2 += temp1;
  allpassDelays[2]->tick(temp2);
  temp2 = -(allpassCoefficient * temp2) + temp;
    
  temp3 = temp2 + (combCoefficient[0] * combDelays[0]->lastOut());
  temp4 = temp2 + (combCoefficient[1] * combDelays[1]->lastOut());
  temp5 = temp2 + (combCoefficient[2] * combDelays[2]->lastOut());
  temp6 = temp2 + (combCoefficient[3] * combDelays[3]->lastOut());

  combDelays[0]->tick(temp3);
  combDelays[1]->tick(temp4);
  combDelays[2]->tick(temp5);
  combDelays[3]->tick(temp6);

  filtout = temp3 + temp4 + temp5 + temp6;

  lastOutput[0] = effectMix * (outLeftDelay->tick(filtout));
  lastOutput[1] = effectMix * (outRightDelay->tick(filtout));
  temp = (1.0 - effectMix) * input;
  lastOutput[0] += temp;
  lastOutput[1] += temp;
    
  return (lastOutput[0] + lastOutput[1]) * 0.5;
}

// end JCRev

// start FxNRev

FxNRev :: FxNRev(MY_FLOAT T60)
{
  int lengths[15] = {1433, 1601, 1867, 2053, 2251, 2399, 347, 113, 37, 59, 53, 43, 37, 29, 19};
  double scaler = BirdBrain::srate() / 25641.0;

  int delay, i;
  for (i=0; i<15; i++) {
    delay = (int) floor(scaler * lengths[i]);
    if ( (delay & 1) == 0) delay++;
    while ( !this->isPrime(delay) ) delay += 2;
    lengths[i] = delay;
  }

  for (i=0; i<6; i++) {
    combDelays[i] = new FxDelay( lengths[i], lengths[i]);
    combCoefficient[i] = pow(10.0, (double)(-3 * lengths[i] / (T60 * BirdBrain::srate())));
  }

  for (i=0; i<8; i++)
    allpassDelays[i] = new FxDelay(lengths[i+6], lengths[i+6]);

  allpassCoefficient = (MY_FLOAT)0.7;
  effectMix = (MY_FLOAT)0.3;
  this->clear();
}

FxNRev :: ~FxNRev()
{
  int i;
  for (i=0; i<6; i++)  delete combDelays[i];
  for (i=0; i<8; i++)  delete allpassDelays[i];
}

void FxNRev :: clear()
{
  int i;
  for (i=0; i<6; i++) combDelays[i]->clear();
  for (i=0; i<8; i++) allpassDelays[i]->clear();
  lastOutput[0] = 0.0;
  lastOutput[1] = 0.0;
  lowpassState = 0.0;
}

MY_FLOAT FxNRev :: tick(MY_FLOAT input)
{
  MY_FLOAT temp, temp0, temp1, temp2, temp3;
  int i;

  temp0 = 0.0;
  for (i=0; i<6; i++) {
    temp = input + (combCoefficient[i] * combDelays[i]->lastOut());
    temp0 += combDelays[i]->tick(temp);
  }
  for (i=0; i<3; i++)   {
    temp = allpassDelays[i]->lastOut();
    temp1 = allpassCoefficient * temp;
    temp1 += temp0;
    allpassDelays[i]->tick(temp1);
    temp0 = -(allpassCoefficient * temp1) + temp;
  }

    // One-pole lowpass filter.
  lowpassState = 0.7*lowpassState + 0.3*temp0;
  temp = allpassDelays[3]->lastOut();
  temp1 = allpassCoefficient * temp;
  temp1 += lowpassState;
  allpassDelays[3]->tick(temp1);
  temp1 = -(allpassCoefficient * temp1) + temp;
    
  temp = allpassDelays[4]->lastOut();
  temp2 = allpassCoefficient * temp;
  temp2 += temp1;
  allpassDelays[4]->tick(temp2);
  lastOutput[0] = effectMix*(-(allpassCoefficient * temp2) + temp);
    
  temp = allpassDelays[5]->lastOut();
  temp3 = allpassCoefficient * temp;
  temp3 += temp1;
  allpassDelays[5]->tick(temp3);
  lastOutput[1] = effectMix*(-(allpassCoefficient * temp3) + temp);

  temp = (1.0 - effectMix) * input;
  lastOutput[0] += temp;
  lastOutput[1] += temp;
    
  return (lastOutput[0] + lastOutput[1]) * 0.5;

}

// end FxNRev

// start FxPRCRev

FxPRCRev :: FxPRCRev(MY_FLOAT T60)
{
  // FxDelay lengths for 44100 Hz sample rate.
  int lengths[4]= {353, 1097, 1777, 2137};
  double scaler = BirdBrain::srate() / 44100.0;

  // Scale the delay lengths if necessary.
  int delay, i;
  if ( scaler != 1.0 ) {
    for (i=0; i<4; i++) {
      delay = (int) floor(scaler * lengths[i]);
      if ( (delay & 1) == 0) delay++;
      while ( !this->isPrime(delay) ) delay += 2;
      lengths[i] = delay;
    }
  }

  for (i=0; i<2; i++)   {
    allpassDelays[i] = new FxDelay( lengths[i], lengths[i] );
    combDelays[i] = new FxDelay( lengths[i+2], lengths[i+2] );
    combCoefficient[i] = pow(10.0, (double)(-3 * lengths[i+2] / (T60 * BirdBrain::srate())));
  }

  allpassCoefficient = (MY_FLOAT)0.7;
  effectMix = (MY_FLOAT)0.5;
  this->clear();
}

FxPRCRev :: ~FxPRCRev()
{
  delete allpassDelays[0];
  delete allpassDelays[1];
  delete combDelays[0];
  delete combDelays[1];
}

void FxPRCRev :: clear()
{
  allpassDelays[0]->clear();
  allpassDelays[1]->clear();
  combDelays[0]->clear();
  combDelays[1]->clear();
  lastOutput[0] = 0.0;
  lastOutput[1] = 0.0;
}

MY_FLOAT FxPRCRev :: tick(MY_FLOAT input)
{
  MY_FLOAT temp, temp0, temp1, temp2, temp3;

  temp = allpassDelays[0]->lastOut();
  temp0 = allpassCoefficient * temp;
  temp0 += input;
  allpassDelays[0]->tick(temp0);
  temp0 = -(allpassCoefficient * temp0) + temp;
    
  temp = allpassDelays[1]->lastOut();
  temp1 = allpassCoefficient * temp;
  temp1 += temp0;
  allpassDelays[1]->tick(temp1);
  temp1 = -(allpassCoefficient * temp1) + temp;
    
  temp2 = temp1 + (combCoefficient[0] * combDelays[0]->lastOut());
  temp3 = temp1 + (combCoefficient[1] * combDelays[1]->lastOut());

  lastOutput[0] = effectMix * (combDelays[0]->tick(temp2));
  lastOutput[1] = effectMix * (combDelays[1]->tick(temp3));
  temp = (MY_FLOAT) (1.0 - effectMix) * input;
  lastOutput[0] += temp;
  lastOutput[1] += temp;
    
  return (lastOutput[0] + lastOutput[1]) * (MY_FLOAT) 0.5;

}

// end FxPRCRev
