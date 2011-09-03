#ifndef __SYNTHESIS_H__
#define __SYNTHESIS_H__

#include "birdbrain.h"


class Synthesis
{
public:
    // do it.  input == FFT frame, output == tracks and residue
    virtual void synthesize( Frame & frame ) = 0;
	virtual void init() { } 
	virtual void reset() { }
	virtual void setTracks( std::vector<Track *> me ) { tracks = me; }; 

public:
    // parameters
    int max_tracks; // number of tracks to synthesize

public:
    // constructor and destructor
    Synthesis();
    virtual ~Synthesis();
    
public: 
	double time_stretch;
	double freq_warp;

protected:
    Frame f;
	std::vector<Track *> tracks;
};


class Syn : public Synthesis
{
public:
	virtual void synthesize( Frame &frame );
	void synhelp( Frame &frame, Track *itrack, int offset );
	
	virtual void init();
	virtual void setTracks( std::vector<Track *> me );

public:
};


struct TrackInfo
{
	TIME start_time;
	TIME end_time;
	uint histind; // last history index  
	double freq; // last frequency value
	double mag; // last magnitude value
	TIME diff_t; // diff_t at end of last step
	double ratio_t; // how far it is between history points
				   // time of history[histind+1] =  diff_t + history[histind] + ratio_t * (history[histind+1]-history[histind])
	bool single_point_track;
};


class SynFast : public Synthesis
{
public:
	virtual void synthesize( Frame &frame ); 
	void synhelp( Frame &frame, int trackind ); 
	virtual void setTracks( std::vector<Track *> me ); 
	virtual void init(); 
	virtual void reset(); 

public:
	std::vector<TrackInfo> tracks_info;
	TIME cur_time; // without stretching
    Frame synframe; // for synhelp
};


class SynSndObj : public Synthesis
{
public:
	virtual void synthesize( Frame &frame );
	virtual void init();

protected:
	int m_tracks;
	float * m_freqs;
	float * m_amps;
	float * m_phases;
};



#endif
