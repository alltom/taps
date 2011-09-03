#include "driver.h"
#include <iostream>
using namespace std;

// ha ha ha ha ha ha
int main( int argc, char ** argv )
{
    if( argc != 11 ) {
        cout << "Usage: birdbrain <filename> <start sample> <end sample> <freq_min> <freq_max> <det thresh> <minpoints> <maxgap>" << endl;
        cout << "       <filename> = name of input file\n";
        cout << "       <start sample> = left / point in input file where processing begins (-1 for default)\n";
        cout << "       <end sample> = right / point in input file where processing ends (-1 for default)\n";
        cout << "       <freq_min> = minimum frequency for peak finding (-1 for default)\n";
        cout << "       <freq_max> = maximum frequency for peak finding (-1 for default)\n";
        cout << "       <det thresh> = deterministic threshold (no default)\n";
        cout << "       <minpoints> = minimum number of times a peak should appear\n";
        cout << "       <maxgap> = maximum frames for which a peak can disappear\n";
        cout << "       <error_f> = minimum ratio for two successive peaks to be considered the same track\n"; 
        cout << "       <noise_r> = minumum ratio of peak to average that is considered to be above the noise floor\n";
        cout << endl;
        return -1;
    }

//    Driver * driver = run( argv[1], atoi( argv[2] ), atoi( argv[3] ), atof( argv[4] ), atoi( argv[5] ), NULL, atoi( argv[7] ), atoi( argv[8] ), atof( argv[9] ), atof( argv[10] ) );
    
    // brake
//    driver->brake();

    // free
//    delete driver;

    return 0;
}
