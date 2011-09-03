//-----------------------------------------------------------------------------
// name: ui_audiofx.h
// desc: birdbrain ui audio effects
//
// authors: Ananya Misra (amisra@cs.princeton.edu)
//          Ge Wang (gewang@cs.princeton.edu)
//          Perry R. Cook (prc@cs.princeton.edu)
// date: Spring 2006
//-----------------------------------------------------------------------------
#ifndef __UI_AUDIOFX_H__
#define __UI_AUDIOFX_H__

#include <string>


// float definition
#define MY_FLOAT SAMPLE


//! FxStk error handling class.
/*!
  This is a fairly abstract exception handling class.  There could
  be sub-classes to take care of more specific error conditions ... or
  not.
*/
class FxStkError
{
public:
  enum TYPE { 
    WARNING,
    DEBUG_WARNING,
    FUNCTION_ARGUMENT,
    FILE_NOT_FOUND,
    FILE_UNKNOWN_FORMAT,
    FILE_ERROR,
    PROCESS_THREAD,
    PROCESS_SOCKET,
    PROCESS_SOCKET_IPADDR,
    AUDIO_SYSTEM,
    MIDI_SYSTEM,
    UNSPECIFIED
  };

protected:
  char message[256];
  TYPE type;

public:
  //! The constructor.
  FxStkError(const char *p, TYPE tipe = FxStkError::UNSPECIFIED);

  //! The destructor.
  virtual ~FxStkError(void);

  //! Prints "thrown" error message to stdout.
  virtual void printMessage(void);

  //! Returns the "thrown" error message TYPE.
  virtual const TYPE& getType(void) { return type; }

  //! Returns the "thrown" error message string.
  virtual const char *getMessage(void) const { return message; }
};


class FxStk
{
public:

  typedef unsigned long STK_FORMAT;
  static const STK_FORMAT STK_SINT8;   /*!< -128 to +127 */
  static const STK_FORMAT STK_SINT16;  /*!< -32768 to +32767 */
  static const STK_FORMAT STK_SINT32;  /*!< -2147483648 to +2147483647. */
  static const STK_FORMAT MY_FLOAT32; /*!< Normalized between plus/minus 1.0. */
  static const STK_FORMAT MY_FLOAT64; /*!< Normalized between plus/minus 1.0. */

  //! Static method which returns the current FxStk sample rate.
  static MY_FLOAT sampleRate(void);

  //! Static method which sets the FxStk sample rate.
  /*!
    The sample rate set using this method is queried by all FxStk
    classes which depend on its value.  It is initialized to the
    default SRATE set in FxStk.h.  Many FxStk classes use the sample rate
    during instantiation.  Therefore, if you wish to use a rate which
    is different from the default rate, it is imperative that it be
    set \e BEFORE FxStk objects are instantiated.
  */
  static void setSampleRate(MY_FLOAT newRate);

  //! Static method which returns the current rawwave path.
  static std::string rawwavePath(void);

  //! Static method which sets the FxStk rawwave path.
  static void setRawwavePath(std::string newPath);

  //! Static method which byte-swaps a 16-bit data type.
  static void swap16(unsigned char *ptr);

  //! Static method which byte-swaps a 32-bit data type.
  static void swap32(unsigned char *ptr);

  //! Static method which byte-swaps a 64-bit data type.
  static void swap64(unsigned char *ptr);

  //! Static cross-platform method to sleep for a number of milliseconds.
  static void sleep(unsigned long milliseconds);

private:
  static MY_FLOAT srate;
  static std::string rawwavepath;

protected:

  //! Default constructor.
  FxStk(void);

  //! Class destructor.
  virtual ~FxStk(void);

  //! Function for error reporting and handling.
  static void handleError( const char *message, FxStkError::TYPE type );

};

// Here are a few other useful typedefs.
typedef signed short SINT16;
typedef signed int SINT32;
typedef float FLOAT32;
typedef double FLOAT64;

#define ONE_OVER_128 (MY_FLOAT) 0.0078125

// end FxStk.h


// FxFilter class
class FxFilter : public FxStk
{
public:
  //! Default constructor creates a zero-order pass-through "filter".
  FxFilter(void);

  //! Overloaded constructor which takes filter coefficients.
  /*!
    An FxStkError can be thrown if either \e nb or \e na is less than
    one, or if the a[0] coefficient is equal to zero.
  */
  FxFilter(int nb, MY_FLOAT *bCoefficients, int na, MY_FLOAT *aCoefficients);

  //! Class destructor.
  virtual ~FxFilter(void);

  //! Clears all internal states of the filter.
  void clear(void);

  //! Set filter coefficients.
  /*!
    An FxStkError can be thrown if either \e nb or \e na is less than
    one, or if the a[0] coefficient is equal to zero.  If a[0] is not
    equal to 1, the filter coeffcients are normalized by a[0].
  */
  void setCoefficients(int nb, MY_FLOAT *bCoefficients, int na, MY_FLOAT *aCoefficients);

  //! Set numerator coefficients.
  /*!
    An FxStkError can be thrown if \e nb is less than one.  Any
    previously set denominator coefficients are left unaffected.
    Note that the default constructor sets the single denominator
    coefficient a[0] to 1.0.
  */
  void setNumerator(int nb, MY_FLOAT *bCoefficients);

  //! Set denominator coefficients.
  /*!
    An FxStkError can be thrown if \e na is less than one or if the
    a[0] coefficient is equal to zero.  Previously set numerator
    coefficients are unaffected unless a[0] is not equal to 1, in
    which case all coeffcients are normalized by a[0].  Note that the
    default constructor sets the single numerator coefficient b[0]
    to 1.0.
  */
  void setDenominator(int na, MY_FLOAT *aCoefficients);

  //! Set the filter gain.
  /*!
    The gain is applied at the filter input and does not affect the
    coefficient values.  The default gain value is 1.0.
   */
  virtual void setGain(MY_FLOAT theGain);

  //! Return the current filter gain.
  virtual MY_FLOAT getGain(void) const;

  //! Return the last computed output value.
  virtual MY_FLOAT lastOut(void) const;

  //! Input one sample to the filter and return one output.
  virtual MY_FLOAT tick(MY_FLOAT sample);

  //! Input \e vectorSize samples to the filter and return an equal number of outputs in \e vector.
  virtual MY_FLOAT *tick(MY_FLOAT *vector, unsigned int vectorSize);

protected:
  MY_FLOAT gain;
  int     nB;
  int     nA;
  MY_FLOAT *b;
  MY_FLOAT *a;
  MY_FLOAT *outputs;
  MY_FLOAT *inputs;

};


// FxDelay class
class FxDelay : protected FxFilter
{
public:

  //! Default constructor creates a delay-line with maximum length of 4095 samples and zero delay.
  FxDelay();

  //! Overloaded constructor which specifies the current and maximum delay-line lengths.
  FxDelay(long theDelay, long maxDelay);

  //! Class destructor.
  virtual ~FxDelay();

  //! Clears the internal state of the delay line.
  void clear();

  //! Set the delay-line length.
  /*!
    The valid range for \e theDelay is from 0 to the maximum delay-line length.
  */
  void setDelay(long theDelay);
  void set( long delay, long max );

  //! Return the current delay-line length.
  long getDelay(void) const;

  //! Calculate and return the signal energy in the delay-line.
  MY_FLOAT energy(void) const;

  //! Return the value at \e tapDelay samples from the delay-line input.
  /*!
    The tap point is determined modulo the delay-line length and is
    relative to the last input value (i.e., a tapDelay of zero returns
    the last input value).
  */
  MY_FLOAT contentsAt(unsigned long tapDelay) const;

  //! Return the last computed output value.
  MY_FLOAT lastOut(void) const;

  //! Return the value which will be output by the next call to tick().
  /*!
    This method is valid only for delay settings greater than zero!
   */
  virtual MY_FLOAT nextOut(void) const;

  //! Input one sample to the delay-line and return one output.
  virtual MY_FLOAT tick(MY_FLOAT sample);

  //! Input \e vectorSize samples to the delay-line and return an equal number of outputs in \e vector.
  virtual MY_FLOAT *tick(MY_FLOAT *vector, unsigned int vectorSize);

protected:
  long inPoint;
  long outPoint;
  long length;
  MY_FLOAT delay;
};


// FxReverb class
class FxReverb : public FxStk
{
 public:
  //! Class constructor.
  FxReverb();

  //! Class destructor.
  virtual ~FxReverb();

  //! Reset and clear all internal state.
  virtual void clear() = 0;

  //! Set the mixture of input and "reverberated" levels in the output (0.0 = input only, 1.0 = reverb only). 
  void setEffectMix(MY_FLOAT mix);

  //! Return the last output value.
  MY_FLOAT lastOut() const;

  //! Return the last left output value.
  MY_FLOAT lastOutLeft() const;

  //! Return the last right output value.
  MY_FLOAT lastOutRight() const;

  //! Abstract tick function ... must be implemented in subclasses.
  virtual MY_FLOAT tick(MY_FLOAT input) = 0;

  //! Take \e vectorSize inputs, compute the same number of outputs and return them in \e vector.
  virtual MY_FLOAT *tick(MY_FLOAT *vector, unsigned int vectorSize);

 protected:

  // Returns true if argument value is prime.
  bool isPrime(int number);

  MY_FLOAT lastOutput[2];
  MY_FLOAT effectMix;
};


// FxJCRev class
class FxJCRev : public FxReverb
{
public:
  //! Class constructor taking a T60 decay time argument.
  FxJCRev( MY_FLOAT T60 );

  //! Class destructor.
  ~FxJCRev();

  //! Reset and clear all internal state.
  void clear();

  //! Compute one output sample.
  MY_FLOAT tick( MY_FLOAT input );

protected:
  FxDelay *allpassDelays[3];
  FxDelay *combDelays[4];
  FxDelay *outLeftDelay;
  FxDelay *outRightDelay;
  MY_FLOAT allpassCoefficient;
  MY_FLOAT combCoefficient[4];
};


// FxNRev class
class FxNRev : public FxReverb
{
 public:
  //! Class constructor taking a T60 decay time argument.
  FxNRev(MY_FLOAT T60);

  //! Class destructor.
  ~FxNRev();

  //! Reset and clear all internal state.
  void clear();

  //! Compute one output sample.
  MY_FLOAT tick(MY_FLOAT input);

 protected:  
  FxDelay *allpassDelays[8];
  FxDelay *combDelays[6];
  MY_FLOAT allpassCoefficient;
  MY_FLOAT combCoefficient[6];
  MY_FLOAT lowpassState;
};


// FxPRCRev class
class FxPRCRev : public FxReverb
{
public:
  //! Class constructor taking a T60 decay time argument.
  FxPRCRev(MY_FLOAT T60);

  //! Class destructor.
  ~FxPRCRev();

  //! Reset and clear all internal state.
  void clear();

  //! Compute one output sample.
  MY_FLOAT tick(MY_FLOAT input);

protected:  
  FxDelay *allpassDelays[2];
  FxDelay *combDelays[2];
  MY_FLOAT allpassCoefficient;
  MY_FLOAT combCoefficient[2];

};


#endif
