// Copyright (c)Victor Lazzarini, 1997-2004
// See License.txt for a disclaimer of all warranties
// and licensing information

// AudioDefs.h  SndObj Library Defines & Headers
// Base Classes

#ifndef NOPTHREAD
#include "SndThread.h" // needs pthread library
#endif  

#include "SndObj.h"
#include "SndIO.h" 
#include "Table.h"

// SndObj-derived
#include "Oscil.h"    // Oscillators
#include "Oscilt.h"
#include "Oscili.h"
#include "FastOsc.h"
#include "Osc.h"
#include "Osci.h"

#include "SndIn.h"    // Sound input
#include "SndRead.h"  // audio file input

#include "MidiIn.h"   // Midi input
#include  "Bend.h"    // pitch bend
#include "MidiMap.h" 

#include "ADSR.h"     // Envelopes
#include "IADSR.h" 

#include "Buzz.h"     // Buzzer

#include "Balance.h"  // Balance of signals

#include "DelayLine.h" // Delay line
#include  "Tap.h"      // Truncating tap
#include  "Tapi.h"     // Time-varying tap
#include  "Comb.h"     // Comb filter
#include  "Allpass.h"  // Allpass filter
#include  "StringFlt.h" // String filter
#include   "Pluck.h"    // Karplus-Strong
#include   "VDelay.h"   // Variable delay
#include   "Pitch.h"    // Pitch transposer
#include   "Loop.h"     // Looping
#include   "Fir.h"      // direct convolution

#include   "Filter.h"    // Fixed-freq/BW reson
#include   "TpTz.h"      // Two-pole Two-zero
#include    "Reson.h"    // Variable reson
#include    "Lp.h"       // Variable LP with resonance
#include    "ButtBP.h"   // Butterworth filters
#include    "ButtBR.h" 
#include    "ButtHP.h" 
#include    "ButtLP.h" 
#include    "Ap.h"       // 2nd order all-pass
#include    "LowPass.h"  // 1st order LP
#include    "HiPass.h"   // 1st order HP

#include    "Hilb.h"     // Hilbert transformer

#include    "SyncGrain.h" // granular synthesis

#include    "Mix.h"      // Mixer
#include    "Pan.h"     // panning
#include    "Gain.h"    // gain 
#include    "Interp.h"  // curve segments
#include    "Phase.h"    // phase accumulator
#include    "Ring.h"     // general-purpose multiplier
#include    "Unit.h"     // test signals

#include   "Lookup.h"     // table lookup
#include   "Lookupi.h"  

#include  "Rand.h"     // Noise
#include  "Randh.h"    // Band-limited noise
#include  "Randi.h"    // interpolated

// Spectral stuff

#include "FFT.h"        // windowed overlapping FFT
#include "IFFT.h"       // overlap-add IFFT
#include "PVA.h"        // pvoc analysis/synthesis
#include "PVS.h" 
#include "PVRead.h"                
#include "IFGram.h"

#include "SinAnal.h"    // sinusoidal analysis
#include "SinSyn.h"     // sinusoidal resynthesis
#include "AdSyn.h"		// additive resynthesis

#include  "SpecMult.h"    // spectral multiplication
#include  "SpecInterp.h"  // spectral interpolation
#include  "PVMask.h"	  // spectral masking
#include   "PVTransp.h"  // transposition
#include   "PVMix.h"     // mixing
#include   "PVBlur.h"    // blurring
#include   "PVFilter.h"    // mag filtering
#include  "PVMorph.h"     // pvoc morphing 
#include  "SpecPolar.h"  // polar converstion
#include  "SpecSplit.h"  // polar conversion & split
#include  "SpecThresh.h" // threshold filter
#include  "SpecVoc.h"    // impose magnitudes
#include  "SpecCart.h"   // cartesian conversion
#include  "SpecCombine.h" // combine phases & mags
#include   "SpecIn.h"   // spectral file input

#include "Convol.h"     // table-based convolution

// SndIO-derived
#include "SndFIO.h"   // Raw file IO 
#include "SndWave.h"  // RIFF-Wave
#include "SndWaveX.h" // waveformatextensible
#include "SndPVOCEX.h" // pvocex
#include "SndSinIO.h"  // sinusex
#include "SndAiff.h"  // AIFF
#include "SndBuffer.h" // memory buffer 
#include "SndMidi.h"  // midi IO
#include "SndMidiIn.h"
#include "SndRTIO.h" // WinMME/OSS/SGI RT IO
#ifdef _MBCS
#include "SndAsio.h" // ASIO-driver IO
#endif
#ifdef JACK
#include "SndJackIO.h" // Jack IO
#endif
#ifdef MACOSX
#include "SndCoreAudio.h" // Core Audio support
#endif

// Table-derived 
#include "HarmTable.h"    // wavetables
#include "UsrHarmTable.h"
#include "TrisegTable.h"  // envelope
#include  "EnvTable.h"    // envelope curves
#include "SndTable.h"     // soundfile input
#include "PlnTable.h"     // Polynomials
#include "HammingTable.h" // Windows
#include "NoteTable.h"    // midi note conversion
#include "UsrDefTable.h"  // user-definable
#include "LoPassTable.h"    // lowpass impulse response 
#include "SpecEnvTable.h"    // spectral envelope
#include "PVEnvTable.h"      // PV envelope
#include "PVTable.h"         // PV  frame
#include "ImpulseTable.h"    // linear FIR coeffs 
