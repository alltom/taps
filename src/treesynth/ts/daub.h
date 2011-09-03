#include <stdlib.h>
#include <stdio.h>
#include <math.h>

float * make_vector(long nl, long nh);

void free_vector(float *v, long nl, long nh);

void wt1(float a[], unsigned long n, int isign,
	void (*wtstep)(float [], unsigned long, int));

void daub4(float a[], unsigned long n, int isign);

void pwtset( int n );

void pwt( float a[], unsigned long n, int isign );
