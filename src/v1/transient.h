

/*TransientExtractor
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


#include "birdbrain.h"
#include "sceptre.h"

struct TransLoc
{
    int start;
    int end;
};


class TransientExtractor
{
public:
    virtual bool extract( uint start, uint end ) = 0;
    virtual bool remove_transients( const char *outfilename, std::vector< uint > rmindexs ); 

    virtual SAMPLE * getEnv() = 0; // maybe call this something else

    std::vector< TransLoc > transients;
    
    uint estart;        // samples in the waveform/file
    uint eend;          // where the analysis starts and ends (zooms in)

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

    virtual bool extract( uint start, uint end );
    virtual bool remove_transients( const char *outfilename, std::vector< uint > rmindexs ); 

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

    virtual bool extract( uint start, uint end );
    virtual bool remove_transients( const char *outfilename, std::vector< uint > rmindexs );

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

