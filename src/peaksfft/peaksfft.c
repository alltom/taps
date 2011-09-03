/**************************************/
/*****  FFT Hack, by Perry Cook, 2000 */
/*****  This here prints out the N    */
/*****  most significant peaks in     */
/*****  each frame of an fft file     */
/*****  It can also do a quickie      */
/*****  sinusoidal resynthesis at the */
/*****  the same time.                */
/**************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

struct ffthdr {
  char fftc[4];     // .fft
  long fftsize;     // fft size
  long windowsize;  // windowsize (<= fftsize)
  long hopsize;     // hop size (<=fftsize) 
  long dlength;     // original file data length in samples
  long srate;       // original file sampling rate
};

#include "waveio.h"

void main(int argc, char *argv[])   {
    float *fftData,max;
    float *outData;
    float *freqList;
	float *freqMask;
	float *resData;
	float *fftPhases;
    FILE *input, *output, *resoutput;
    int filtering = 0, numFreqs = 0, itemp;
    int notDone,synth=0,verbose = 1, res = 0;
    long *peaks;
    float *magns;
    float temp,temp2,pitch,time,gain=1.0;
    double PI, phase = 0,delta;
    long length,maxloc,winWidth,numPeaks,before_peak,after_peak,temp3;
    long i,j,peak,newHop,total=0;
    short data;
    struct ffthdr fhdr =
        {".fft",256,256,256,0,44100}; // changed from 22050
	struct ffthdr rfhdr;
    struct soundhdr hdr;

    fillheader(&hdr,44100); // changed from 22050

    if (argc < 3)   {
        printf("useage: peaksfft NUMPEAKS filein.fft [OPTIONS]\n");
        printf("        OPTIONS: [-s synthfile.wav] [-q(uiet mode)]\n");
		printf("                 [-g gainMult] [-p pitchMult] [-t timeMult]\n");
                printf("                 [-k# killList or -f# filtList]\n");
                printf("   killList is list of frequencies to disallow\n");
                printf("   filtList is list of frequencies to only allow\n");
		exit(0);
    }
    else {
        input = fopen(argv[2], "rb");
        numPeaks = atoi(argv[1]);
        peaks = (long *) calloc(4,numPeaks+1);
        magns = (float *) calloc(4,numPeaks+1);
        pitch = 1.0;
        time = 1.0;
        i = 3;
        while (i < argc)        {
            if (argv[i][1] == 's')  {
                output = opensoundout(argv[i+1],&hdr);
		        synth = 1;
            }
            if (argv[i][1] == 'p')  {
                pitch = (float) atof(argv[i+1]);
            }
			if (argv[i][1] == 'r')  {
				resoutput = fopen(argv[i+1], "wb"); // writing fft
				res = 1;
			}
            if (argv[i][1] == 't')      {
                time = (float) atof(argv[i+1]);
            }
            if (argv[i][1] == 'q') {
                verbose = 0;
            }
            if (argv[i][1] == 'g') {
                gain = atof(argv[i+1]);
            }
            if (argv[i][1] == 'k')      {
                filtering = -1;
                numFreqs = atoi(&argv[i][2]);
                freqList = (float *) malloc(4*numFreqs);
                for (j=0;j<numFreqs;j++)    {
                    freqList[j] = atof(argv[i+1]);
                    i += 1;
                }
            }
            if (argv[i][1] == 'f')      {
                filtering = 1;
                numFreqs = atoi(&argv[i][2]);
                freqList = (float *) malloc(4*numFreqs);
                for (j=0;j<numFreqs;j++)    {
                    freqList[j] = atof(argv[i+1]);
                    i += 1;
                }
            }
            i += 1;
        }
    }
    if (input)        {
        fread(&fhdr,4,6,input);
        length = fhdr.fftsize;
        fftData = (float *) malloc(4*length);
		fftPhases = (float *) malloc(4*length);
        winWidth = 2 * (fhdr.fftsize / fhdr.windowsize); //window main lobe width
        if (synth)      {
            hdr.dlength = (long) (time * (float) fhdr.dlength * 2.0f);
#ifdef LITTLENDIAN
            hdr.flength = hdr.dlength + 4;
            hdr.bytes_per_sec = fhdr.srate*2;
#endif
            hdr.srate = fhdr.srate;
            PI = 4.*atan(1.0);
            delta = pitch * 2 * PI / (double) fhdr.fftsize;
            outData = (float *) calloc(4,(long) (time * (float) length));
        }
		if (res) {
			resData = (float *) calloc(2, 4*fhdr.fftsize);
			srand(0);
			fwrite(&fhdr, 4, 6, resoutput);
		}
        if (filtering != 0)        {
            length = fhdr.fftsize;
            freqMask = (float *) malloc(length/2 * 4);
			if (filtering == -1)	{
				for (i=0;i<length/2;i++) freqMask[i] = 1.0;
                for (j=0;j<numFreqs;j++)	{
					itemp = freqList[j]*length/fhdr.srate;
					for (i=itemp-winWidth;i<itemp+winWidth;i++)
						freqMask[i] = 0.0;
				}
			}
			else if (filtering == 1)	{
				for (i=0;i<length/2;i++) freqMask[i] = 0.0;
                for (j=0;j<numFreqs;j++)	{
					itemp = freqList[j]*length/fhdr.srate;
					i = itemp;
					//  for (i=itemp-winWidth;i<itemp+winWidth;i++)
						freqMask[i] = 1.0;
				}
			}
		}
        while (fread(fftData,fhdr.fftsize,4,input))       {
            length = fhdr.fftsize;
            for (i=0;i<length/2;i++)   {
                temp = fftData[i*2];
                temp2 = fftData[i*2+1];               
				fftPhases[i] = (float) atan2(temp2, temp);	  // (Don't throw away phase)
                temp2 *= temp2;
                temp *= temp;
				fftData[i] = (float) sqrt(temp + temp2);  // Throw away phase
            }
            length = length / 2;                  // and only look at magnitude
            if (filtering != 0)	{
			    for (i=0;i<length;i++)
					fftData[i] *= freqMask[i];
			}
			for (i=0;i<winWidth;i++)     {
                fftData[i] = 0;                   // null out low frequency terms
            }
            peak = 0;
            while (peak < numPeaks)        {
                max = 0.0;
                maxloc = 0;
                for (i=0;i<length;i++) {
                    if (fabs(fftData[i] > max))   {
                        max = (float) fabs(fftData[i]);	// why fabs? isn't it already +ve due to sqrt?
                        maxloc = i;
                    }
                }
                peaks[peak] = maxloc;
                magns[peak] = max;
                
				// find endpoints or something (figure it out yourself (each time))
				for( temp3 = maxloc, temp2 = max;
				     temp3 > 0 && fftData[temp3 - 1] <= temp2;
					 temp2 = fftData[temp3--] );
				
				before_peak = temp3;
				
				for( temp3 = maxloc, temp2 = max;
					 temp3 < length-1 && fftData[temp3 + 1] <= temp2;
					 temp2 = fftData[temp3++] );

				after_peak = temp3;

				// line interpolation
				temp = (float) (fftData[after_peak] - fftData[before_peak]) / (after_peak - before_peak + 1);
				
				for (i = before_peak; i <= after_peak; i++) {
					fftData[i] = (float) fftData[before_peak] + temp * (i - before_peak);
				}

				/*if (maxloc >= winWidth)  {
					before_peak = (float) fftData[maxloc-winWidth];
					after_peak = (float) fftData[maxloc+winWidth];
                    temp = (after_peak - before_peak) / (2*winWidth); // slope
					/*for (i=maxloc-winWidth;i<maxloc+winWidth;i++) {
						fftData[i] = before_peak + temp*(i-maxloc+winWidth);
						//fftPhases[i] = 2 * PI * rand() / (RAND_MAX + 1.0);
					}*/
				/*	for (i=maxloc-winWidth;i<maxloc+winWidth;i++) {
						fftData[i] = 0;
						//fftPhases[i] = 2 * PI * rand() / (RAND_MAX + 1.0);
					}
                }*/
                peak += 1;
            }
            if (numPeaks > 1)   {
                notDone = 1;
                while (notDone)     {
                    notDone = 0;
                    for (j=0;j<numPeaks-1;j++)  {
                        if (peaks[j] > peaks[j+1])  {
                            if (peaks[j+1] > 0)     {	// sorting in increasing order of LOCATION (index)
                                max = magns[j];			
								maxloc = peaks[j];		
								peaks[j] = peaks[j+1];
								magns[j] = magns[j+1];
								peaks[j+1] = maxloc;
								magns[j+1] = max;
								notDone = 1;
							}
						}
					}
                }
            }
			if (verbose)	{ 
				for (i=0;i<numPeaks;i++)        {
                                        printf("%li,%f ;",fhdr.srate * peaks[i] / fhdr.fftsize,magns[i]);
				}				
				printf("\n\n");
			}
			else printf(".");

            if (synth)  {
                newHop = (long) (time * (float) fhdr.hopsize);
                for (i=0;i<newHop;i++)       {
                    outData[i] = outData[i+newHop];
                }
                for (i=newHop;i<newHop*2;i++) {
                    outData[i] = 0;
                }
                for (i=0;i<newHop*2;i++) {
                    temp = 0.0;
                    for (peak = 0;peak<numPeaks;peak++)     {					// what's going on
                        temp += (float) (magns[peak] * cos(peaks[peak]*phase));	// here ??? 
                    }															// :-(

                    temp *= (1.0 - cos(PI * i / newHop));                       // hanning window?
                    outData[i] += temp;                                         // (overlap) add

                    data = (short) (gain * outData[i]);							// the resynthesis?
                    if (i < newHop)	{											// but how?
                        fwrite(&data,2,1,output);								// oh...i think i get it
                        total += 1;												// :-)
                    }
                    phase += delta;
                    //  temp /= fhdr.fftsize;
                }
                phase -= (delta * newHop);
            }

			if (res)  {
				for (i = 0; i < length; i++)  {
					// go ahead and write (if you can)
					temp = 2 * PI * rand() / (RAND_MAX + 1.0);
					//temp = fftPhases[i];
					resData[2*i] = fftData[i] * cos(temp);
					resData[2*i + 1] = fftData[i] * sin(temp);
				
					//data = (short)resData[2*i];
					//fwrite(&data, 2, 1, resoutput);
					//data = (short)resData[2*i + 1];
					//fwrite(&data, 2, 1, resoutput);
				}

				fwrite(resData, 4, fhdr.fftsize, resoutput);
			}
                
        }
		printf("\n");
        fclose(input);
        free(fftData);
        if (synth)	{
			free(outData);
			closesoundout(output,total);
		}
		if (res)  {
			free(resData);
			fclose(resoutput);
		}
        free(peaks);
        free(magns);
        if (filtering != 0)	{
			free(freqList);
			free(freqMask);
		}
    }
    else    {
		printf("Can't open input (or output) file!!\n");
    }
    
}


