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
// file: taps_transient.h
// desc: taps transient extractor
//
// author: Perry R. Cook (prc@cs.princeton.edu)
//         Matt Hoffman (mdhoffma@cs.princeton.edu)
//         Ananya Misra (amisra@cs.princeton.edu)
//         Ge Wang (gewang@cs.princeton.edu)
// date: Spring 2005
//-----------------------------------------------------------------------------
#ifndef __TRANSIENT_H__
#define __TRANSIENT_H__


/*
    TransientExtractor

  functions:
    transExtr
    envExtr
    get_transient
info:
    input-      filename
                start sample
                end sample
    
    transient-  frame for reading in from file
                envelope buffer
                transient indices
    
    output-     number of transients
                the transients (indices - start and END)

uh:
    what about residue?
    solution-   later (wavelet tree not so bad...)

    the Verma/Meng paper does transient analysis on residue of sines+noise analysis
*/


#include "taps_birdbrain.h"
#include "taps_sceptre.h"

struct TransLoc
{
    int start;
    int end;
};


class TransientExtractor
{
public:
    virtual bool extract( t_TAPUINT start, t_TAPUINT end ) = 0;
    virtual bool remove_transients( const char *outfilename, std::vector< t_TAPUINT > rmindexs ); 

    virtual SAMPLE * getEnv() = 0; // maybe call this something else

    std::vector< TransLoc > transients;
    
    t_TAPUINT estart;        // samples in the waveform/file
    t_TAPUINT eend;          // where the analysis starts and ends (zooms in)

public:
    int allowableGap;
    int envLen;
    float thresh;

protected:
    std::string filename;
};



class EnvExtractor : public TransientExtractor
{
public:
    EnvExtractor( const char * filename );
    ~EnvExtractor();

    virtual bool extract( t_TAPUINT start, t_TAPUINT end );
    virtual bool remove_transients( const char *outfilename, std::vector< t_TAPUINT > rmindexs ); 

    virtual SAMPLE * getEnv();

public: 
    // int envLen       // in superclass
    // float thresh;    // in superclass    // transExtr
    float ageamt;                           // transExtr
    // int allowableGap;// in superclass    // transExtr
    
    float attack;       // envExtr
    float decay;        // envExtr;

protected: 
    SAMPLE * derivs;
    SAMPLE * envelope;
    int envSize; // size allocated to envelope

    Frame * read_frame;

    int transExtr(); 
    void envExtr( Frame * framein ); 
};      



// energy ratio
class EngExtractor: public TransientExtractor
{
public:
    EngExtractor( const char * filename );
    ~EngExtractor();

    virtual bool extract( t_TAPUINT start, t_TAPUINT end );
    virtual bool remove_transients( const char *outfilename, std::vector< t_TAPUINT > rmindexs );

    virtual SAMPLE * getEnv(); // gets ratio of short frame to long frame energy?

public:
    int longLen;
    int longHop;
    int shortLen;
    int shortHop;
    // float thresh;    // in superclass
    float passFreq; // for high-pass filtering that doesn't take place
    // int envLen;      // in superclass
    // int allowableGap;// in superclass
    int maxTranLen; // max length of a transient

protected: 
    SAMPLE * envelope;
    int * framestarts;
    int envCap; // size allocated to envelope (and framestarts)
    int transExtr();
    SAMPLE energy( Frame * framein );
};




#endif

